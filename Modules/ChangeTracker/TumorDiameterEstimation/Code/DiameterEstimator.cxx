// Estimate the max diameter of a binary segmentation following this approach:
// 1) for each direction of the image, find the largest area slice in the
// segmentation
// 2) identify the points located on the boundary of the segmented slice
// 3) order all possible pairs of points by distance
// 4) starting from the pair of points that are most far apart, do the
// following, until either the distance between the next pair of points is
// less than the current estimate of the diameter, or we run out of points
//   4.0) set the current estimate of the max diameter to the distance between
//   the current pair of points
//   4.1) follow the line between the points, and subtract those pixels that
//   are located outside the segmented region from the current diameter
//   estimate
//   4.2) update the current estimate
//
//   Author: Andriy Fedorov
//   Date: 12 Dec 2008
//
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkNeighborhoodIterator.h"
#include "itkExtractImageFilter.h"
#include "itkLineIterator.h"
#include "itkLookAtTransformInitializer.h"
#include "itkResampleImageFilter.h"
#include "itkRigid3DTransform.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkPadImageFilter.h"
#include "itkPasteImageFilter.h"
#include "itkExceptionObject.h"
//#include "itkKdTree.h"

#include <iostream>

using namespace std;
//#include "itkSimpleFilterWatcher.h"

//#include "itkMedianImageFilter.h"
//#include "itkSliceBySliceImageFilter.h"
//#include "itkCommand.h"


//#include "itkImageFileReader.h"
//#include "itkSliceBySliceImageFilter.h"

typedef unsigned char PixelType; 
typedef itk::Image<PixelType,3> ImageType;
typedef itk::Image<PixelType,2> ImageType2D;
typedef ImageType2D::PointType Point2DType;
typedef itk::ImageFileReader<ImageType> ReaderType;
typedef itk::ImageFileWriter<ImageType2D> WriterType2D;
typedef itk::ImageFileWriter<ImageType> WriterType;
typedef itk::ImageRegionConstIteratorWithIndex<ImageType> IteratorType;
typedef itk::ImageRegionIterator<ImageType> CopyIteratorType;
typedef itk::NeighborhoodIterator<ImageType2D> NeiIteratorType;
typedef itk::ExtractImageFilter<ImageType,ImageType2D> ExtractorType;
typedef itk::LineIterator<ImageType2D> LineIteratorType2D;
typedef itk::LineIterator<ImageType> LineIteratorType;

typedef itk::Rigid3DTransform<double> TransformType;
typedef itk::LookAtTransformInitializer<TransformType,ImageType> 
  TransformInitializerType;
typedef itk::ResampleImageFilter<ImageType,ImageType>
  ResampleFilterType;
typedef itk::NearestNeighborInterpolateImageFunction<ImageType>
  InterpolatorType;
typedef itk::PadImageFilter<ImageType,ImageType> PadderType;
typedef itk::PasteImageFilter<ImageType> PasterType;

//typedef itk::Statistics::KdTree<ImageType2D::PointType> KdTreeType;
//typedef itk::SliceBySliceImageFilter<ImageType,ImageType> SliceFilterType;
typedef pair<ImageType2D::PointType,ImageType2D::PointType> PtPairType;
typedef pair<ImageType::PointType,ImageType::PointType> PtPairType3D;
typedef pair<double,PtPairType> DistPtPairType;
typedef pair<double,PtPairType3D> DistPtPairType3D;

struct DistPtPairSort
{
     bool operator()(DistPtPairType p0, DistPtPairType p1) { 
       return p0.first>p1.first;
     }
};

ImageType::IndexType FindLargestAreaSlice(ImageType::Pointer image);
ImageType::IndexType FindLargestAreaSliceAlongDirection(ImageType::Pointer image,int);
double PtDistanceSquared(Point2DType, Point2DType);
double PtDistance(Point2DType p0, Point2DType p1);
double EstimateDiameterBresenham(ImageType2D::Pointer, ImageType2D::IndexType, 
  ImageType2D::IndexType, double);
//ImageType::Pointer RealignVolume(ImageType::Pointer, ImageType::IndexType,
//  ImageType::IndexType, ImageType::IndexType);
ImageType::Pointer RealignVolume(ImageType::Pointer, ImageType::PointType,
  ImageType::PointType, TransformType::Pointer&);
ImageType::Pointer PadImage(ImageType::Pointer);
void FindBoundaryPoints(ImageType2D::Pointer,vector<ImageType2D::PointType>&);
double FindMaxDiameterTuple(ImageType2D::Pointer, vector<ImageType2D::PointType>&, DistPtPairType&);
double GetMaxDiameterEstimate(ImageType::Pointer, DistPtPairType3D&);
void UpdateImageWithDiameter(ImageType::Pointer, ImageType::PointType, ImageType::PointType);
void SaveImage(ImageType::Pointer image, const char* name);
double FindMaxDiameterTupleThruPoint(ImageType2D::Pointer, 
  vector<ImageType2D::PointType>&, ImageType2D::PointType,
  DistPtPairType &maxDiameterTuple);
ImageType2D::PointType FindProjectionPoint2D(ImageType2D::PointType,
  ImageType2D::PointType, ImageType2D::PointType);

int main( int argc, char ** argv )
{
  // Verify the number of parameters in the command line
  if( argc < 2 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputBinaryImage outputSliceWithDiameterVisualization(optional)" << std::endl;
    return EXIT_FAILURE;
    }

  const char * inputFilename  = argv[1];
  char* outputFilename = NULL;
  if(argc>2)
    outputFilename = argv[2];

  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( inputFilename  );
  reader->Update();

  ImageType::Pointer image = reader->GetOutput();

  // Note: padding changes the estimate of the diameter
  // I believe the reason is in the handling of the boundary during nei
  // iterator traversal
  image = PadImage(image);
    {
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName("padded.nhdr");
      writer->SetInput(image);
      writer->Update();
    }
   
  DistPtPairType3D maxDiameterTuple;
  double maxDiameterEstimate;

  maxDiameterEstimate = GetMaxDiameterEstimate(image, maxDiameterTuple);

  // Now look at all the slices perpendicular to the largest diameter
  // (initially, along the orthogonal image directions)
  
  // TODO: possible solutions
  //
  // I: resampling
  //  1. get the angle between the largest diameter and some axis
  //  2. resample the image by rotating it by this angle
  //  3. follow the same technique as before, but examine only one axis along
  //  the diameter-aligned direction
  //
  // II: oblique slice extraction (more time-consuming)
  //  1. for each slice orthogonal to the largest diameter:
  //       extract the slice
  //       extract the contour
  //       follow the procedure as before for each contour

  // Following Approach I, realign the input volume
  //
  /*
  ImageType2D::IndexType idx0, idx1;
  slice->TransformPhysicalPointToIndex(maxDiameterTuple.second.first, idx0);
  slice->TransformPhysicalPointToIndex(maxDiameterTuple.second.second, idx1);

  
  ImageType::Pointer realignedImage;
  for(i=0,j=0;i<3;i++){
    if(!extractionIndex[i]){
      volIdx0[i] = idx0[j];
      volIdx1[i] = idx1[j];
      j++;
    }
    else {
      volIdx0[i] = extractionIndex[i];
      volIdx1[i] = extractionIndex[i];
    }
  }
  */

  TransformType::Pointer transform = TransformType::New();
  ImageType::Pointer realignedImage = RealignVolume(image, maxDiameterTuple.second.first,
    maxDiameterTuple.second.second, transform);
  ImageType::PointType mdPt0, mdPt1;
  mdPt0 = transform->TransformPoint(maxDiameterTuple.second.first);
  mdPt1 = transform->TransformPoint(maxDiameterTuple.second.second);

  maxDiameterTuple.second.first = mdPt0;
  maxDiameterTuple.second.second = mdPt1;

  UpdateImageWithDiameter(realignedImage, mdPt0, mdPt1);

  // Search for the 2nd largest diameter
  ImageType::IndexType extractionIndex = FindLargestAreaSliceAlongDirection(realignedImage, 2);
  ImageType::SizeType extractionSize = image->GetLargestPossibleRegion().GetSize();
  ImageType::RegionType extractionRegion;
  ImageType::PointType extractionPoint;
  extractionSize[2] = 0;
  extractionRegion.SetSize(extractionSize);
  extractionRegion.SetIndex(extractionIndex);
  image->TransformIndexToPhysicalPoint(extractionIndex, extractionPoint);
  ExtractorType::Pointer extractor2 = ExtractorType::New();
  extractor2->SetInput(realignedImage);
  extractor2->SetExtractionRegion(extractionRegion);
  try{
    extractor2->Update();
  }catch(itk::ExceptionObject &e){
    cout << "Exception in extractor2: " << e << endl;
    abort();
  }
  
  ImageType2D::Pointer slice2 = extractor2->GetOutput();
  vector<ImageType2D::PointType> boundaryPts;
  FindBoundaryPoints(slice2, boundaryPts);

  // Here search for the max diameter is a bit different, because the diameter
  // segment has to intersect the first diameter we have already estimated
  ImageType2D::PointType intersectionPt;
  ImageType2D::IndexType intersectionIdx;

  intersectionPt[0] = mdPt0[0];
  intersectionPt[1] = mdPt0[1];

  DistPtPairType maxDiameter2d2;
  DistPtPairType3D diameterTuple3D2;
  ImageType::PointType intersectionPoint3d; // approximate intersection of the two diameters
  double maxDiameter2nd = FindMaxDiameterTupleThruPoint(slice2, boundaryPts, intersectionPt, maxDiameter2d2);
  intersectionPoint3d[0] = mdPt0[0];
  intersectionPoint3d[1] = mdPt0[1];
  intersectionPoint3d[2] = extractionPoint[2];

  ImageType::PointType dPt0, dPt1;
  dPt0[0] = maxDiameter2d2.second.first[0];
  dPt0[1] = maxDiameter2d2.second.first[1];
  dPt0[2] = extractionPoint[2];
  dPt1[0] = maxDiameter2d2.second.second[0];
  dPt1[1] = maxDiameter2d2.second.second[1];
  dPt1[2] = extractionPoint[2];
  cout << "Second diameter: " << dPt0 << ", " << dPt1 << endl;

  diameterTuple3D2.first = maxDiameter2nd;
  diameterTuple3D2.second.first = dPt0;
  diameterTuple3D2.second.second = dPt1;
  UpdateImageWithDiameter(realignedImage, dPt0, dPt1);
  SaveImage(realignedImage, "after_t2.nhdr");

  // now find the last diameter, which goes through the point of intersection
  // of the first two
  DistPtPairType3D diameterTuple3D3;
  ImageType::PointType d0pt0, d0pt1, d1pt0, d1pt1, d2pt0, d2pt1, dPt2;
  d0pt0 = maxDiameterTuple.second.first;
  d0pt0 = maxDiameterTuple.second.second;
  d1pt0 = diameterTuple3D2.second.first;
  d1pt1 = diameterTuple3D2.second.second;

  // compute cross product of the vectors formed by the first two diameters
  itk::Vector<double,3> cross, d0, d1;

  d0[0] = d0pt0[0]-d0pt1[0];
  d0[1] = d0pt0[1]-d0pt1[1];
  d0[2] = d0pt0[2]-d0pt1[2];

  d1[0] = d1pt0[0]-d1pt1[0];
  d1[1] = d1pt0[1]-d1pt1[1];
  d1[2] = d1pt0[2]-d1pt1[2];

  //cross[0] = (a2b3 − a3b2, a3b1 − a1b3, a1b2 − a2b1).
  cross[0] = d0[1]*d1[2]-d0[2]*d1[1];
  cross[1] = d0[2]*d1[0]-d0[0]*d1[2];
  cross[2] = d0[0]*d1[1]-d0[1]*d1[0];
  cross.Normalize();
  cross *= 10.;
  
  // scale the line segment to make sure it intersects the image bounding box,
  // find the intersection points with each of the bbox planes, and keep those
  // that are inside the image
  // plane/line intersection:
  // http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/
  dPt0[0] = intersectionPoint3d[0]+cross[0];
  dPt0[1] = intersectionPoint3d[1]+cross[1];
  dPt0[2] = intersectionPoint3d[2]+cross[2];
  dPt1[0] = intersectionPoint3d[0]-cross[0];
  dPt1[1] = intersectionPoint3d[1]-cross[1];
  dPt1[2] = intersectionPoint3d[2]-cross[2];

  UpdateImageWithDiameter(realignedImage, dPt0, dPt1);
  SaveImage(realignedImage, "after_t3.nhdr");

  double t; // line equation parameter
  t = (dPt0[0]-intersectionPoint3d[0])/(dPt1[0]-intersectionPoint3d[0]);
  
  

  // the line containing the last diameter is defined by the intersection of
  // the first two diameters and the cross product
  ImageType::IndexType bboxId0, bboxId1;
  ImageType::PointType bboxPt0, bboxPt1;
  ImageType::SizeType bboxIdxSize = realignedImage->GetLargestPossibleRegion().GetSize();
  bboxId0[0] = 0;
  bboxId0[1] = 0;
  bboxId0[2] = 0;
  bboxId1[0] = bboxIdxSize[0]-1;
  bboxId1[1] = bboxIdxSize[1]-1;
  bboxId1[2] = bboxIdxSize[2]-1;
  realignedImage->TransformIndexToPhysicalPoint(bboxId0, bboxPt0);
  realignedImage->TransformIndexToPhysicalPoint(bboxId1, bboxPt1);

//  slice->TransformIndexToPhysicalPoint(extractionIndex, 
//  FindMaxDiameterTupleOrtho(slice, boundaryPoints, maxDiameterTuple2, 
//    intersectionPt);

    /*
//  if(outputFilename){
    WriterType::Pointer writer2 = WriterType::New();
    writer2->SetInput(realignedImage);
    writer2->SetFileName("after_t.nhdr");
    writer2->Update();
//  }*/
 
  return 0;
}

ImageType::IndexType FindLargestAreaSlice(ImageType::Pointer image){
  ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
  unsigned i, j;
  double* sliceAcc[3];
  ImageType::SpacingType spacing = image->GetSpacing();

  // These will store the mask area in each slice
  sliceAcc[0] = new double[size[0]];
  sliceAcc[1] = new double[size[1]];
  sliceAcc[2] = new double[size[2]];

  memset(sliceAcc[0], 0, sizeof(double)*size[0]);
  memset(sliceAcc[1], 0, sizeof(double)*size[1]);
  memset(sliceAcc[2], 0, sizeof(double)*size[2]);

  IteratorType it(image, image->GetLargestPossibleRegion());
  for(it.GoToBegin();!it.IsAtEnd();++it){
    if(it.Get()){
      ImageType::IndexType idx = it.GetIndex();
      sliceAcc[0][idx[0]]++;
      sliceAcc[1][idx[1]]++;
      sliceAcc[2][idx[2]]++;
    }
  }

  ImageType::IndexType maxAreaIdx;
  double maxArea = 0;

  for(i=0;i<size[0];i++)
    sliceAcc[0][i] *= spacing[1]*spacing[2];
  for(i=0;i<size[1];i++)
    sliceAcc[1][i] *= spacing[0]*spacing[2];
  for(i=0;i<size[2];i++)
    sliceAcc[2][i] *= spacing[0]*spacing[1];

  for(i=0;i<3;i++){
    for(j=0;j<size[i];j++){
      if(sliceAcc[i][j]>maxArea){
        maxAreaIdx[0] = 0;
        maxAreaIdx[1] = 0;
        maxAreaIdx[2] = 0;
        maxAreaIdx[i] = j;
        maxArea = sliceAcc[i][j];
      }
    }
  }

  delete [] sliceAcc[0];
  delete [] sliceAcc[1];
  delete [] sliceAcc[2];

  // TODO: convert this in physical units
  cout << "Max segmentation slice area is " << maxArea 
    << ", slice extraction index: " << maxAreaIdx << endl;
  cout << "(non-zero component of slice extraction index defines \
    the direction and slice within this direction)" << endl;
  return maxAreaIdx;
}

ImageType::IndexType FindLargestAreaSliceAlongDirection(ImageType::Pointer image, int dir){
  ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
  unsigned i, j;
  double* sliceAcc;
  ImageType::SpacingType spacing = image->GetSpacing();

  // These will store the mask area in each slice
  sliceAcc = new double[size[dir]];

  memset(sliceAcc, 0, sizeof(double)*size[dir]);

  IteratorType it(image, image->GetLargestPossibleRegion());
  for(it.GoToBegin();!it.IsAtEnd();++it){
    if(it.Get()){
      ImageType::IndexType idx = it.GetIndex();
      sliceAcc[idx[dir]]++;
    }
  }

  ImageType::IndexType maxAreaIdx;
  double maxArea = 0;

  // find pixel area in the direction of interest
  double pixelArea;
  switch(dir){
  case 0: pixelArea = spacing[1]*spacing[2]; break;
  case 1: pixelArea = spacing[0]*spacing[2]; break;
  case 2: pixelArea = spacing[0]*spacing[1]; break;
  }

  maxAreaIdx[0] = 0;
  maxAreaIdx[1] = 0;
  maxAreaIdx[2] = 0;

  for(j=0;j<size[dir];j++){
    if(sliceAcc[j]>maxArea){
      maxAreaIdx[dir] = j;
      maxArea = sliceAcc[j];
    }
  }

  maxArea *= pixelArea;
  delete [] sliceAcc;

  // TODO: convert this in physical units
  cout << "Max segmentation slice area is " << maxArea 
    << ", slice extraction index: " << maxAreaIdx << endl;
  cout << "(non-zero component of slice extraction index defines \
    the direction and slice within this direction)" << endl;
  return maxAreaIdx;
}

double PtDistanceSquared(Point2DType p0, Point2DType p1){
  return (p0[0]-p1[0])*(p0[0]-p1[0])+
    (p0[1]-p1[1])*(p0[1]-p1[1]);
}

double PtDistance(Point2DType p0, Point2DType p1){
  return sqrt((p0[0]-p1[0])*(p0[0]-p1[0])+
    (p0[1]-p1[1])*(p0[1]-p1[1]));
}

// Use Bresenham line iterator to find the diameter segments that are outside
// the binary segmentation, and update the initial estimate accodingly
double EstimateDiameterBresenham(ImageType2D::Pointer slice, 
  ImageType2D::IndexType idx0, ImageType2D::IndexType idx1, 
  double currentDiameter){
  bool outside = false;
  LineIteratorType2D lit(slice, idx0, idx1);
  ImageType2D::IndexType outsideIdx0, outsideIdx1;
  for(lit.GoToBegin();!lit.IsAtEnd();++lit){
    if(!lit.Get()){
      if(!outside){
        outside = true;
        outsideIdx0 = lit.GetIndex();
      }
    }
    if(lit.Get()){
      if(outside){
        ImageType2D::PointType pt0, pt1;
        outsideIdx1 = lit.GetIndex();
        slice->TransformIndexToPhysicalPoint(outsideIdx0, pt0);
        slice->TransformIndexToPhysicalPoint(outsideIdx1, pt1);
        currentDiameter -= PtDistance(pt0, pt1);
        outside = false;
      }
    }
  }
  return currentDiameter;
}

ImageType::Pointer RealignVolume(ImageType::Pointer inputImage, 
  ImageType::PointType pt0, ImageType::PointType pt1,
  TransformType::Pointer &transform){

  TransformInitializerType::Pointer tInit =
    TransformInitializerType::New();
  TransformInitializerType::PointType center;
  TransformInitializerType::VectorType direction, up;
  TransformType::OutputVectorType translation;
  ImageType::SizeType size;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  ImageType::Pointer paddedImage;

  center[0] = (pt0[0]+pt1[0])/2.0;
  center[1] = (pt0[1]+pt1[1])/2.0;
  center[2] = (pt0[2]+pt1[2])/2.0;

  direction[0] = pt0[0]-center[0];
  direction[1] = pt0[1]-center[1];
  direction[2] = pt0[2]-center[2];
  direction.Normalize();
  
  tInit->SetImage(inputImage);
  tInit->SetTransform(transform);
  tInit->SetPlane(center, direction, size);
  tInit->InitializeTransform();

  cout << "Transform: " << endl << transform << endl;

  ResampleFilterType::Pointer resampler = 
    ResampleFilterType::New();
  resampler->SetInput(inputImage);
  resampler->SetTransform(transform);
  resampler->SetOutputParametersFromImage(inputImage);
  resampler->SetInterpolator(interpolator);
  resampler->Update();

  return resampler->GetOutput();
}

ImageType::Pointer RealignVolume(ImageType::Pointer inputImage, 
  ImageType::IndexType extractionIndex, 
  ImageType::IndexType idx0, ImageType::IndexType idx1,
  TransformType::Pointer &transform){

  TransformInitializerType::Pointer tInit =
    TransformInitializerType::New();
  TransformInitializerType::PointType center;
  TransformInitializerType::VectorType direction, up;
  TransformType::OutputVectorType translation;
  ImageType::PointType pt0, pt1;
  ImageType::SizeType size;
  InterpolatorType::Pointer interpolator = InterpolatorType::New();
  ImageType::Pointer paddedImage;

  inputImage->TransformIndexToPhysicalPoint(idx0, pt0);
  inputImage->TransformIndexToPhysicalPoint(idx1, pt1);
  center[0] = (pt0[0]+pt1[0])/2.0;
  center[1] = (pt0[1]+pt1[1])/2.0;
  center[2] = (pt0[2]+pt1[2])/2.0;

  direction[0] = pt0[0]-center[0];
  direction[1] = pt0[1]-center[1];
  direction[2] = pt0[2]-center[2];
  direction.Normalize();
  
  tInit->SetImage(inputImage);
  tInit->SetTransform(transform);
  tInit->SetPlane(center, direction, size);
  tInit->InitializeTransform();



  /*
  translation[0] = 0;
  translation[1] = 0;
  translation[2] = 0;
  VectorType translationT = transform->TransformVector(translation);
  transform->SetTranslation(translationT);
  
  TransformType::MatrixType rotX, rotY, rotZ;
  TransformType::MatrixType matrix = transform->GetMatrix();
  double sinX, cosX, sinY, cosY, sinZ, cosZ;
  sinX = direction[0];
  cosX = sqrt(1.-sinX*sinX);
  sinY = direction[1];
  cosY = sqrt(1.-sinY*sinY);
  sinZ = direction[2];
  cosZ = sqrt(1.-sinZ*sinZ);

  rotX[0][0] = 1.;  rotX[0][1] = 0.;    rotX[0][2] = 0.;
  rotX[1][0] = 0.;  rotX[1][1] = cosX;  rotX[1][2] = -sinX;
  rotX[2][0] = 0.;  rotX[2][1] = sinX;  rotX[2][2] = cosX;

  rotY[0][0] = cosY;  rotY[0][1] = 0.;  rotY[0][2] = sinY;
  rotY[1][0] = 0.;    rotY[1][1] = 1;   rotY[1][2] = 0;
  rotY[2][0] = -sinY; rotY[2][1] = 0;   rotY[2][2] = cosY;

  rotZ[0][0] = cosZ;  rotZ[0][1] = -sinZ; rotZ[0][2] = 0.;
  rotZ[1][0] = sinZ;  rotZ[1][1] = cosZ;  rotZ[1][2] = 0;
  rotZ[2][0] = 0.;    rotZ[2][1] = 0;     rotZ[2][2] = 1;

  matrix = rotX*rotY*rotZ;
  transform->SetMatrix(matrix);
  transform->SetCenter(center);
  */
  /*
  TransformType::InputVectorType offset;
  offset[0] = 0;
  offset[1] = 0;
  offset[2] = 0;
  TransformType::InputVectorType offsetT = transform->TransformVector(offset);
  transform->SetTranslation(offsetT);*/
//  transform->SetTranslation(translation);

  cout << "Transform: " << endl << transform << endl;

  ResampleFilterType::Pointer resampler = 
    ResampleFilterType::New();
  resampler->SetInput(inputImage);
  resampler->SetTransform(transform);
  resampler->SetOutputParametersFromImage(inputImage);
  /*
  resampler->SetSize(size);
  resampler->SetOutputSpacing(inputImage->GetSpacing());
  resampler->SetOrigin(inputImage->GetOrigin());
  */
  resampler->SetInterpolator(interpolator);
  resampler->Update();

  return resampler->GetOutput();
}

ImageType::Pointer PadImage(ImageType::Pointer image){

  ImageType::Pointer paddedImage = ImageType::New();
  ImageType::RegionType region = image->GetLargestPossibleRegion();
  ImageType::SizeType size = region.GetSize(), newSize;
  ImageType::IndexType index = region.GetIndex(), newIndex;
  PasterType::Pointer paster = PasterType::New();

  newSize[0] = size[0]+size[0]/2;
  newSize[1] = size[1]+size[1]/2;
  newSize[2] = size[2]+size[2]/2;

  newIndex[0] = index[0]-size[0]/4;
  newIndex[1] = index[1]-size[1]/4;
  newIndex[2] = index[2]-size[2]/4;

  cout << "Initial image size: " << size << endl;
  cout << "Image has been padded to have new size " << newSize << endl;

  region.SetSize(newSize);
  region.SetIndex(index);

  paddedImage->SetRegions(region);
  paddedImage->Allocate();
  paddedImage->FillBuffer(0);
  paddedImage->SetSpacing(image->GetSpacing());

  for(index[0];index[0]<size[0];index[0]++){
    index[1] = 0;
    for(index[1];index[1]<size[1];index[1]++){
      index[2] = 0;
      for(index[2];index[2]<size[2];index[2]++){
        ImageType::IndexType newIndex;
        newIndex[0] = index[0]+size[0]/4;
        newIndex[1] = index[1]+size[1]/4;
        newIndex[2] = index[2]+size[2]/4;
        paddedImage->SetPixel(newIndex, image->GetPixel(index));
      }
    }
  }

  return paddedImage;

  /*
//  paddedImage->SetOrigin(image->GetOrigin());
  cout << "Input image: ";
  image->Print(cout);

  paster->SetInput(paddedImage);
  paster->SetSourceImage(image);
  paster->SetDestinationImage(paddedImage);
  paster->SetSourceRegion(image->GetLargestPossibleRegion());
  paster->SetDestinationIndex(index);
  cout << "Destination index: " << index << endl;
  cout << "Destination size: " << newSize << endl;
  cout << "Source size: " << size << endl;
  paster->Update();

  return paster->GetOutput();

  
  CopyIteratorType inputIt(image, image->GetLargestPossibleRegion());
  CopyIteratorType outputIt(paddedImage, image->GetLargestPossibleRegion());
  for(inputIt.GoToBegin(),outputIt.GoToBegin();!inputIt.IsAtEnd();++inputIt,++outputIt)
    inputIt.Set(outputIt.Get());

  return paddedImage;
  
   * PadImageFilter doesn't work in the release of ITK used 
   * in Slicer right now
   *
  PadderType::Pointer padder = PadderType::New();
  ImageType::SizeType padSize, size;
  long unsigned int padBound[3];
  
  size = image->GetLargestPossibleRegion().GetSize();
  padSize[0] = size[0]/4;
  padSize[1] = size[1]/4;
  padSize[2] = size[2]/4;

  padBound[0] = size[0]/4;
  padBound[1] = size[0]/4;
  padBound[2] = size[0]/4;

  padder->SetPadLowerBound(&padBound[0]);
  padder->SetPadUpperBound(&padBound[0]);
  padder->SetInput(image);
  padder->Update();
  // TODO follow the diameter segment, and update the image data to fix the
  // resampling issues
  return padder->GetOutput();
  */
}

// Get the boundary pixels
void FindBoundaryPoints(ImageType2D::Pointer slice,vector<ImageType2D::PointType>& boundaryPts){
  boundaryPts.clear();
  int i;
  NeiIteratorType::RadiusType neiRadius;
  neiRadius.Fill(1);
  NeiIteratorType neiIt(neiRadius, slice, slice->GetLargestPossibleRegion());
  for(neiIt.GoToBegin();!neiIt.IsAtEnd();++neiIt){
    if(neiIt.GetPixel(4)==0)
      continue;
    for(i=0;i<9;i++)
      if(neiIt.GetPixel(i)==0){
        ImageType2D::PointType boundaryPt;
        ImageType2D::IndexType boundaryIdx;
        neiIt.SetPixel(4, neiIt.GetPixel(4)*4);
        slice->TransformIndexToPhysicalPoint(neiIt.GetIndex(), boundaryPt);
        slice->TransformPhysicalPointToIndex(boundaryPt, boundaryIdx);
        /*
        if(neiIt.GetIndex() != boundaryIdx){
          cout << "Conversion failure: " << endl;
          cout << "Input index: " << neiIt.GetIndex() << endl;
          cout << "Output point: " << boundaryPt << endl;
          cout << "Output index: " << boundaryIdx << endl;
          abort();
        }*/
        boundaryPts.push_back(boundaryPt);
        break;
      }
  }
}

double FindMaxDiameterTuple(ImageType2D::Pointer slice, 
  vector<ImageType2D::PointType> &boundaryPts,
  DistPtPairType &maxDiameterTuple){
  int numBoundaryPts = boundaryPts.size();
  int i, j;
  vector<DistPtPairType> distPtVector;
  
  cout << "Initially " << distPtVector.size() << " pairs" << endl;

  // Find all distance combinations, excluding duplicates
  double maxDist = 0;
  PtPairType maxDiameterPair;
  for(i=0;i<numBoundaryPts;i++){
    for(j=i+1;j<numBoundaryPts;j++){
      double dist;
      dist = PtDistance(boundaryPts[i],boundaryPts[j]);
      distPtVector.push_back(
        DistPtPairType(dist,PtPairType(boundaryPts[i],boundaryPts[j])));
      if(dist>maxDist){
        maxDiameterPair = PtPairType(boundaryPts[i],boundaryPts[j]);
        maxDist = dist;
      }
    }
  }

  // sort them by the estimated diameter
  sort(distPtVector.begin(), distPtVector.end(), DistPtPairSort());
  cout << "Total number of tuples to evaluate: " << distPtVector.size() << endl;

  cout << "Initial diameter estimate is " << maxDist << endl;
  ImageType2D::IndexType idx0, idx1;
  if(!slice->TransformPhysicalPointToIndex(maxDiameterPair.first, idx0))
    cout << "Point " << maxDiameterPair.first << " out of bounds!" << endl;
  if(!slice->TransformPhysicalPointToIndex(maxDiameterPair.second, idx1))
    cout << "Point " << maxDiameterPair.second << " out of bounds!" << endl;

  // Follow the line s for each pair to find intersection points, exclude 
  // outside segments from the diameter estimate
  maxDiameterTuple = distPtVector[0];
  maxDist = maxDiameterTuple.first;
  double diameterEstimate = 0;
  i = 0;
  int tupleCnt = distPtVector.size();
  for(vector<DistPtPairType>::const_iterator pIt = distPtVector.begin();
    pIt!=distPtVector.end();++pIt,++i){

    double currentDiameter;

    slice->TransformPhysicalPointToIndex((*pIt).second.first, idx0);
    slice->TransformPhysicalPointToIndex((*pIt).second.second, idx1);
    currentDiameter = (*pIt).first;
    currentDiameter = EstimateDiameterBresenham(slice, idx0, idx1, currentDiameter);
    
    // if the current estimate is larger than the distance between the next
    // tuple of points, then we have the largest diameter possible
    if(currentDiameter > diameterEstimate){
      diameterEstimate = currentDiameter;
      maxDiameterTuple = *pIt;
    }

    if(i>tupleCnt-1)
      if(diameterEstimate > (*(pIt+1)).first)
        break;
  }
  
  return diameterEstimate;
}

double FindMaxDiameterTupleThruPoint(ImageType2D::Pointer slice, 
  vector<ImageType2D::PointType> &boundaryPts, ImageType2D::PointType centerPt,
  DistPtPairType &maxDiameterTuple){
  int numBoundaryPts = boundaryPts.size();
  int i, j;
  vector<DistPtPairType> distPtVector;

  // Find all distance combinations, excluding duplicates
  double maxDist = 0;
  PtPairType maxDiameterPair;
  for(i=0;i<numBoundaryPts;i++){
    for(j=i+1;j<numBoundaryPts;j++){
      double dist;

      ImageType2D::PointType projectionPt;
      projectionPt = FindProjectionPoint2D(boundaryPts[i], boundaryPts[j], centerPt);

      // take only those diameter measurements that are within the spacing
      // size from the largest diameter
      if(PtDistance(projectionPt, centerPt)>.1)
        continue;
      dist = PtDistance(boundaryPts[i],boundaryPts[j]);
      distPtVector.push_back(
        DistPtPairType(dist,PtPairType(boundaryPts[i],boundaryPts[j])));
      if(dist>maxDist){
        maxDiameterPair = PtPairType(boundaryPts[i],boundaryPts[j]);
        maxDist = dist;
      }
    }
  }

  // sort them by the estimated diameter
  sort(distPtVector.begin(), distPtVector.end(), DistPtPairSort());
  cout << "Total number of tuples to evaluate: " << distPtVector.size() << endl;

  cout << "Initial diameter estimate is " << maxDist << endl;
  ImageType2D::IndexType idx0, idx1;
  if(!slice->TransformPhysicalPointToIndex(maxDiameterPair.first, idx0))
    cout << "Point " << maxDiameterPair.first << " out of bounds!" << endl;
  if(!slice->TransformPhysicalPointToIndex(maxDiameterPair.second, idx1))
    cout << "Point " << maxDiameterPair.second << " out of bounds!" << endl;

  // Follow the line s for each pair to find intersection points, exclude 
  // outside segments from the diameter estimate
  maxDiameterTuple = distPtVector[0];
  maxDist = maxDiameterTuple.first;
  double diameterEstimate = 0;
  i = 0;
  int tupleCnt = distPtVector.size();
  for(vector<DistPtPairType>::const_iterator pIt = distPtVector.begin();
    pIt!=distPtVector.end();++pIt,++i){

    double currentDiameter;

    slice->TransformPhysicalPointToIndex((*pIt).second.first, idx0);
    slice->TransformPhysicalPointToIndex((*pIt).second.second, idx1);
    currentDiameter = (*pIt).first;
    currentDiameter = EstimateDiameterBresenham(slice, idx0, idx1, currentDiameter);
    
    // if the current estimate is larger than the distance between the next
    // tuple of points, then we have the largest diameter possible
    if(currentDiameter > diameterEstimate){
      diameterEstimate = currentDiameter;
      maxDiameterTuple = *pIt;
    }

    if(i>tupleCnt-1)
      if(diameterEstimate > (*(pIt+1)).first)
        break;
  }
  
  return diameterEstimate;
}

double GetMaxDiameterEstimate(ImageType::Pointer image, DistPtPairType3D& maxDiameterTuple3D){
  ImageType::IndexType extractionIndex = FindLargestAreaSlice(image);
  ImageType::RegionType extractionRegion;
  ImageType::SizeType extractionSize = image->GetLargestPossibleRegion().GetSize();
  ImageType::PointType extractionPoint;
  int i, j;
  for(i=0;i<3;i++)
    if(extractionIndex[i])
      extractionSize[i] = 0;
  // size is 0 for the dimension that collapses
  // index specifies which slice to select, and should be non-zero for the
  // same component where size is 0
  extractionRegion.SetSize(extractionSize);
  extractionRegion.SetIndex(extractionIndex);
  image->TransformIndexToPhysicalPoint(extractionIndex, extractionPoint);

  ExtractorType::Pointer extractor = ExtractorType::New();
  extractor->SetInput(image);
  extractor->SetExtractionRegion(extractionRegion);
  try{
    extractor->Update();
  }catch(itk::ExceptionObject &e){
    cout << "Exception in extractor: " << e << endl;
    abort();
  }

  ImageType2D::Pointer slice = extractor->GetOutput();
    if(1){
      WriterType2D::Pointer writer = WriterType2D::New();
      writer->SetFileName("slice.mha");
      writer->SetInput(slice);
      writer->Update();
    }
  

  vector<ImageType2D::PointType> boundaryPts;

  FindBoundaryPoints(slice, boundaryPts);
  cout << boundaryPts.size() << " boundary points found" << endl;

  DistPtPairType maxDiameterTuple;
  double diameterEstimate;
  diameterEstimate = FindMaxDiameterTuple(slice, boundaryPts, maxDiameterTuple);

  cout << "Updated max diameter estimate: " << diameterEstimate << endl;
  
  ImageType::PointType maxDiameterPt0, maxDiameterPt1;
  for(i=0,j=0;i<3;i++){
    if(extractionIndex[i]){
      maxDiameterPt0[i] = extractionPoint[i];
      maxDiameterPt1[i] = extractionPoint[i];
    } else {
      maxDiameterPt0[i] = maxDiameterTuple.second.first[j];
      maxDiameterPt1[i] = maxDiameterTuple.second.second[j];
      j++;
    }
  }

  maxDiameterTuple3D.first = diameterEstimate;
  maxDiameterTuple3D.second.first = maxDiameterPt0;
  maxDiameterTuple3D.second.second = maxDiameterPt1;

  return diameterEstimate;
}

void UpdateImageWithDiameter(ImageType::Pointer image, ImageType::PointType pt0, ImageType::PointType pt1){
  ImageType::IndexType idx0, idx1;

  image->TransformPhysicalPointToIndex(pt0, idx0);
  image->TransformPhysicalPointToIndex(pt1, idx1);

  LineIteratorType lit(image, idx0, idx1);
  for(lit.GoToBegin();!lit.IsAtEnd();++lit)
    if(lit.Get())
      lit.Set(lit.Get()+80);
}

void SaveImage(ImageType::Pointer image, const char* name){
  WriterType::Pointer writer1 = WriterType::New();
  writer1->SetInput(image);
  writer1->SetFileName(name);
  writer1->Update();
}
      
// Project point p2 on the (p0,p1) line
// from http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
ImageType2D::PointType FindProjectionPoint2D(ImageType2D::PointType p0,
  ImageType2D::PointType p1, ImageType2D::PointType p2){
  ImageType2D::PointType projectionPt;
  double u;

  u = (p2[0]-p0[0])*(p1[0]-p0[0])+(p2[1]-p0[1])*(p1[1]-p0[1]);
  u /= PtDistanceSquared(p0,p1);

  projectionPt[0] = p0[0]+u*(p1[0]-p0[0]);
  projectionPt[1] = p0[1]+u*(p1[1]-p0[1]);

  return projectionPt;
}
