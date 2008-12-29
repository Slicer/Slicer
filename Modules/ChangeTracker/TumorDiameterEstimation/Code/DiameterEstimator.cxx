//
// Tool for estimating binary segmentation diameter and diameter-based
// volume.
//
// I. Estimate the max diameter of a binary segmentation following this approach:
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
// II. Estimate the second largest diameter by finding the largest area slice
// orthogonal to the first diameter, and finding the largest diameter of that
// slice that intersects the first diameter.
//
// III. Find the third diameter, which is orthogonal to the first two.
//
// IV. Estimate the tumor volume as ABC/2 (add reference here). Report the
// estimated volume together with the true volume based on voxel count.
//
//   Author: Andriy Fedorov
//           Surgical Planning Lab, BWH
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

#include <iostream>

using namespace std;
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

typedef ImageType::PointType Point3D;
typedef ImageType2D::PointType Point2D;
typedef ImageType2D::IndexType Index2D;
typedef ImageType::IndexType Index3D;
typedef pair<Point2D,Point2D> Point2DPair;
typedef pair<Point3D,Point3D> Point3DPair;
typedef pair<double,Point2DPair> DistPointPair2D;
typedef pair<double,Point3DPair> DistPointPair3D;

struct DistPtPairSort
{
  bool operator()(DistPointPair2D p0, DistPointPair2D p1) { 
    return p0.first>p1.first;
  }
};

double FindMaskVolume(ImageType::Pointer);
ImageType::IndexType FindLargestAreaSlice(ImageType::Pointer image);
ImageType::IndexType FindLargestAreaSliceAlongDirection(ImageType::Pointer image,int);
double PtDistanceSquared(Point2D, Point2D);
double PtDistance(Point2D p0, Point2D p1);
double PtDistance3D(Point3D p0, Point3D p1);
double EstimateDiameterBresenham(ImageType2D::Pointer, Point2D, Point2D, double);
double EstimateDiameterBresenham3D(ImageType::Pointer, Point3D, Point3D, double);
ImageType::Pointer RealignVolume(ImageType::Pointer, ImageType::PointType,
                                 Point3D, TransformType::Pointer&);
ImageType::Pointer PadImage(ImageType::Pointer);
void FindBoundaryPoints(ImageType2D::Pointer,vector<Point2D>&);
double FindMaxDiameterTuple(ImageType2D::Pointer, vector<Point2D>&, DistPointPair3D&);
double GetMaxDiameterEstimate(ImageType::Pointer, DistPointPair3D&);
void UpdateImageWithDiameter(ImageType::Pointer, Point3D, Point3D);
void SaveImage(ImageType::Pointer image, const char* name);
double FindMaxDiameterTupleThruPoint(ImageType2D::Pointer, 
                                     vector<Point2D>&, Point2D, DistPointPair2D&);
ImageType2D::PointType FindProjectionPoint2D(Point2D, Point2D, Point2D);
int FindLineBBoxIntersectionPoints(ImageType::Pointer, Point3D, 
  itk::Vector<double,3>, ImageType::PointType&, Point3D&);

int main( int argc, char ** argv )
{
  DistPointPair3D diameterA, diameterB, diameterC;

  // Verify the number of parameters in the command line
  if( argc < 2 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputBinaryImage outputVolumeWithDiameterVisualization(optional)" << std::endl;
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
  // I believe the reason is in the handling of the boundary during
  // neighborhood iterator traversal
  image = PadImage(image); 

  GetMaxDiameterEstimate(image, diameterA);

  // Now look at all the slices perpendicular to the largest diameter
  // (initially, along the orthogonal image directions)
  
  //  1. get the angle between the largest diameter and some axis
  //  2. resample the image by rotating it by this angle
  //  3. follow the same technique as before, but examine only one axis along
  //  the diameter-aligned direction

  TransformType::Pointer diameterA_transform = TransformType::New();
  TransformType::Pointer diameterA_inverseTransform = TransformType::New();
  ImageType::Pointer realignedImage = RealignVolume(image, diameterA.second.first,
                                                    diameterA.second.second, 
                                                    diameterA_transform);
  diameterA_transform->GetInverse(diameterA_inverseTransform);
  Point3D mdPt0, mdPt1;
  mdPt0 = diameterA_transform->TransformPoint(diameterA.second.first);
  mdPt1 = diameterA_transform->TransformPoint(diameterA.second.second);

  diameterA.second.first = mdPt0;
  diameterA.second.second = mdPt1;

  // Search for the 2nd largest diameter
  Index3D extractionIndex = 
    FindLargestAreaSliceAlongDirection(realignedImage, 2);
  ImageType::SizeType extractionSize = 
    image->GetLargestPossibleRegion().GetSize();
  ImageType::RegionType extractionRegion;
  Point3D extractionPoint;
  extractionSize[2] = 0;
  extractionRegion.SetSize(extractionSize);
  extractionRegion.SetIndex(extractionIndex);
  realignedImage->TransformIndexToPhysicalPoint(extractionIndex, extractionPoint);
  ExtractorType::Pointer extractor = ExtractorType::New();
  extractor->SetInput(realignedImage);
  extractor->SetExtractionRegion(extractionRegion);
  try
    {
    extractor->Update();
    }
  catch(itk::ExceptionObject &e)
    {
    cout << "Exception in extractor: " << e << endl;
    abort();
    }
  
  ImageType2D::Pointer diameterB_slice = extractor->GetOutput();
  vector<ImageType2D::PointType> boundaryPts;
  FindBoundaryPoints(diameterB_slice, boundaryPts);

  // Here search for the max diameter is a bit different, because the diameter
  // segment has to intersect the first diameter we have already estimated
  Point2D intersectionPt;

  intersectionPt[0] = mdPt0[0];
  intersectionPt[1] = mdPt0[1];

  DistPointPair2D diameterB_2d;
  Point3D intersectionPoint3d; // approximate intersection of the two diameters
  FindMaxDiameterTupleThruPoint(diameterB_slice, boundaryPts, intersectionPt, diameterB_2d);
  intersectionPoint3d[0] = mdPt0[0];
  intersectionPoint3d[1] = mdPt0[1];
  intersectionPoint3d[2] = extractionPoint[2];

  ImageType::PointType dPt0, dPt1;
  dPt0[0] = diameterB_2d.second.first[0];
  dPt0[1] = diameterB_2d.second.first[1];
  dPt0[2] = extractionPoint[2];
  dPt1[0] = diameterB_2d.second.second[0];
  dPt1[1] = diameterB_2d.second.second[1];
  dPt1[2] = extractionPoint[2];

  diameterB.first = diameterB_2d.first;
  diameterB.second.first = dPt0;
  diameterB.second.second = dPt1;

  // now find the last diameter, which goes through the point of intersection
  // of the first two
  DistPointPair3D diameterTuple3D3;
  Point3D d0pt0, d0pt1, d1pt0, d1pt1, d2pt0, d2pt1, dPt2;
  d0pt0 = diameterA.second.first;
  d0pt1 = diameterA.second.second;
  d1pt0 = diameterB.second.first;
  d1pt1 = diameterB.second.second;

  // compute cross product of the vectors formed by the first two diameters
  itk::Vector<double,3> cross, d0, d1;

  d0[0] = d0pt0[0]-d0pt1[0];
  d0[1] = d0pt0[1]-d0pt1[1];
  d0[2] = d0pt0[2]-d0pt1[2];

  d1[0] = d1pt0[0]-d1pt1[0];
  d1[1] = d1pt0[1]-d1pt1[1];
  d1[2] = d1pt0[2]-d1pt1[2];

  cross[0] = d0[1]*d1[2]-d0[2]*d1[1];
  cross[1] = d0[2]*d1[0]-d0[0]*d1[2];
  cross[2] = d0[0]*d1[1]-d0[1]*d1[0];

  ImageType::PointType is0, is1;
  int nIntersections;
  nIntersections = FindLineBBoxIntersectionPoints(realignedImage,
    intersectionPoint3d, cross, is0, is1);

  assert(nIntersections == 2);

  diameterC.second.first = is0;
  diameterC.second.second = is1;
  diameterC.first = EstimateDiameterBresenham3D(realignedImage, is0, is1, 
    PtDistance3D(is0, is1));
  diameterA.second.first = 
    diameterA_inverseTransform->TransformPoint(diameterA.second.first);
  diameterA.second.second = 
    diameterA_inverseTransform->TransformPoint(diameterA.second.second);  
  diameterB.second.first = 
    diameterA_inverseTransform->TransformPoint(diameterB.second.first);
  diameterB.second.second = 
    diameterA_inverseTransform->TransformPoint(diameterB.second.second);
  diameterC.second.first = 
    diameterA_inverseTransform->TransformPoint(diameterC.second.first);
  diameterC.second.second = 
    diameterA_inverseTransform->TransformPoint(diameterC.second.second);

  UpdateImageWithDiameter(image, diameterA.second.first, diameterA.second.second);
  UpdateImageWithDiameter(image, diameterB.second.first, diameterB.second.second);
  UpdateImageWithDiameter(image, diameterC.second.first, diameterC.second.second);

  if(outputFilename)
    SaveImage(image, outputFilename);

  cout << "Estimated diameters (mm): " << diameterA.first << ", " <<
    diameterB.first << ", " << diameterC.first << endl;
  cout << "Tumor volume estimation based on diameters (A*B*C/2) (mm^3): " <<
    diameterA.first*diameterB.first*diameterC.first/2. << endl;
  cout << "True volume (mm^3): " << FindMaskVolume(image) << endl;
    return 0;
}

ImageType::IndexType FindLargestAreaSlice(ImageType::Pointer image)
{
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
  for(it.GoToBegin();!it.IsAtEnd();++it)
    {
    if(it.Get())
      {
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

  for(i=0;i<3;i++)
    {
    for(j=0;j<size[i];j++)
      {
      if(sliceAcc[i][j]>maxArea)
        {
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

  return maxAreaIdx;
}

double FindMaskVolume(ImageType::Pointer image)
{
  double volume = 0.0;
  ImageType::SpacingType spacing = image->GetSpacing();

  IteratorType it(image, image->GetLargestPossibleRegion());
  for(it.GoToBegin();!it.IsAtEnd();++it){
    if(it.Get())
      volume++;
  }
  volume *= spacing[0]*spacing[1]*spacing[2];
  return volume;
}

ImageType::IndexType FindLargestAreaSliceAlongDirection(ImageType::Pointer image, int dir)
{
  ImageType::SizeType size = image->GetLargestPossibleRegion().GetSize();
  unsigned j;
  double* sliceAcc;
  ImageType::SpacingType spacing = image->GetSpacing();

  // These will store the mask area in each slice
  sliceAcc = new double[size[dir]];

  memset(sliceAcc, 0, sizeof(double)*size[dir]);

  IteratorType it(image, image->GetLargestPossibleRegion());
  for(it.GoToBegin();!it.IsAtEnd();++it)
    {
    if(it.Get())
      {
      ImageType::IndexType idx = it.GetIndex();
      sliceAcc[idx[dir]]++;
      }
    }

  ImageType::IndexType maxAreaIdx;
  double maxArea = 0;

  // find pixel area in the direction of interest
  double pixelArea = 0.0;
  switch(dir)
    {
    case 0: pixelArea = spacing[1]*spacing[2]; break;
    case 1: pixelArea = spacing[0]*spacing[2]; break;
    case 2: pixelArea = spacing[0]*spacing[1]; break;
    }

  maxAreaIdx[0] = 0;
  maxAreaIdx[1] = 0;
  maxAreaIdx[2] = 0;

  for(j=0;j<size[dir];j++)
    {
    if(sliceAcc[j]>maxArea)
      {
      maxAreaIdx[dir] = j;
      maxArea = sliceAcc[j];
      }
    }

  maxArea *= pixelArea;
  delete [] sliceAcc;

  return maxAreaIdx;
}

double PtDistanceSquared(Point2D p0, Point2D p1)
{
  return (p0[0]-p1[0])*(p0[0]-p1[0])+
    (p0[1]-p1[1])*(p0[1]-p1[1]);
}

double PtDistance(Point2D p0, Point2D p1)
{
  return sqrt((p0[0]-p1[0])*(p0[0]-p1[0])+
              (p0[1]-p1[1])*(p0[1]-p1[1]));
}

double PtDistance3D(Point3D p0, Point3D p1){
  return sqrt((p0[0]-p1[0])*(p0[0]-p1[0])+
    (p0[1]-p1[1])*(p0[1]-p1[1])+
    (p0[2]-p1[2])*(p0[2]-p1[2]));
}

// Use Bresenham line iterator to find the diameter segments that are outside
// the binary segmentation, and update the initial estimate accodingly
double EstimateDiameterBresenham(ImageType2D::Pointer slice, 
                                 Point2D pt0, Point2D pt1, double currentDiameter)
{
  bool outside = false;
  Index2D idx0, idx1;

  slice->TransformPhysicalPointToIndex(pt0, idx0);
  slice->TransformPhysicalPointToIndex(pt1, idx1);

  LineIteratorType2D lit(slice, idx0, idx1);
  ImageType2D::IndexType outsideIdx0, outsideIdx1;
  for(lit.GoToBegin();!lit.IsAtEnd();++lit)
    {
    if(!lit.Get())
      {
      if(!outside)
        {
        outside = true;
        outsideIdx0 = lit.GetIndex();
        }
      }
    if(lit.Get())
      {
      if(outside)
        {
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

double EstimateDiameterBresenham3D(ImageType::Pointer image, 
  Point3D pt0, Point3D pt1,
  double currentDiameter){
  bool outside = false;
  Index3D idx0, idx1;

  image->TransformPhysicalPointToIndex(pt0, idx0);
  image->TransformPhysicalPointToIndex(pt1, idx1);

  LineIteratorType lit(image, idx0, idx1);
  ImageType::IndexType outsideIdx0, outsideIdx1;
  for(lit.GoToBegin();!lit.IsAtEnd();++lit){
    if(!lit.Get()){
      if(!outside){
        outside = true;
        outsideIdx0 = lit.GetIndex();
      }
    }
    if(lit.Get()){
      if(outside){
        ImageType::PointType pt0, pt1;
        outsideIdx1 = lit.GetIndex();
        image->TransformIndexToPhysicalPoint(outsideIdx0, pt0);
        image->TransformIndexToPhysicalPoint(outsideIdx1, pt1);
        currentDiameter -= PtDistance3D(pt0, pt1);
        outside = false;
      }
    }
  }
  return currentDiameter;
}

ImageType::Pointer RealignVolume(ImageType::Pointer inputImage, 
                                 ImageType::PointType pt0, ImageType::PointType pt1,
                                 TransformType::Pointer &transform)
{

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
                                 TransformType::Pointer &transform)
{

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

  ResampleFilterType::Pointer resampler = 
    ResampleFilterType::New();
  resampler->SetInput(inputImage);
  resampler->SetTransform(transform);
  resampler->SetOutputParametersFromImage(inputImage);
  resampler->SetInterpolator(interpolator);
  resampler->Update();

  return resampler->GetOutput();
}

ImageType::Pointer PadImage(ImageType::Pointer image)
{

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

  region.SetSize(newSize);
  region.SetIndex(index);

  paddedImage->SetRegions(region);
  paddedImage->Allocate();
  paddedImage->FillBuffer(0);
  paddedImage->SetSpacing(image->GetSpacing());

  for(index[0];index[0]<static_cast<long>(size[0]);index[0]++)
    {
    index[1] = 0;
    for(index[1];index[1]<static_cast<long>(size[1]);index[1]++)
      {
      index[2] = 0;
      for(index[2];index[2]<static_cast<long>(size[2]);index[2]++)
        {
        ImageType::IndexType newIndex;
        newIndex[0] = index[0]+size[0]/4;
        newIndex[1] = index[1]+size[1]/4;
        newIndex[2] = index[2]+size[2]/4;
        paddedImage->SetPixel(newIndex, image->GetPixel(index));
        }
      }
    }

  return paddedImage;
}

// Get the boundary pixels
void FindBoundaryPoints(ImageType2D::Pointer slice,vector<ImageType2D::PointType>& boundaryPts)
{
  boundaryPts.clear();
  int i;
  NeiIteratorType::RadiusType neiRadius;
  neiRadius.Fill(1);
  NeiIteratorType neiIt(neiRadius, slice, slice->GetLargestPossibleRegion());
  for(neiIt.GoToBegin();!neiIt.IsAtEnd();++neiIt)
    {
    if(neiIt.GetPixel(4)==0)
      continue;
    for(i=0;i<9;i++)
      if(neiIt.GetPixel(i)==0)
        {
        ImageType2D::PointType boundaryPt;
        ImageType2D::IndexType boundaryIdx;
        neiIt.SetPixel(4, neiIt.GetPixel(4)*4);
        slice->TransformIndexToPhysicalPoint(neiIt.GetIndex(), boundaryPt);
        slice->TransformPhysicalPointToIndex(boundaryPt, boundaryIdx);
           boundaryPts.push_back(boundaryPt);
           break;
        }
    }
}

double FindMaxDiameterTuple(ImageType2D::Pointer slice, 
                            vector<ImageType2D::PointType> &boundaryPts,
                            DistPointPair2D &maxDiameterTuple)
{
  int numBoundaryPts = boundaryPts.size();
  int i, j;
  vector<DistPointPair2D> distPtVector;
  
  // Find all distance combinations, excluding duplicates
  double maxDist = 0;
  Point2DPair maxDiameterPair;
  for(i=0;i<numBoundaryPts;i++)
    {
    for(j=i+1;j<numBoundaryPts;j++)
      {
      double dist;
      dist = PtDistance(boundaryPts[i],boundaryPts[j]);
      distPtVector.push_back(
        DistPointPair2D(dist,Point2DPair(boundaryPts[i],boundaryPts[j])));
      if(dist>maxDist)
        {
        maxDiameterPair = Point2DPair(boundaryPts[i],boundaryPts[j]);
        maxDist = dist;
        }
      }
    }

  // sort them by the estimated diameter
  sort(distPtVector.begin(), distPtVector.end(), DistPtPairSort());

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
  for(vector<DistPointPair2D>::const_iterator pIt = distPtVector.begin();
      pIt!=distPtVector.end();++pIt,++i)
    {

    double currentDiameter;

    currentDiameter = (*pIt).first;
    currentDiameter = EstimateDiameterBresenham(slice, 
      (*pIt).second.first, (*pIt).second.second, currentDiameter);
    
    // if the current estimate is larger than the distance between the next
    // tuple of points, then we have the largest diameter possible
    if(currentDiameter > diameterEstimate)
      {
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
                                     vector<ImageType2D::PointType> &boundaryPts, 
                                     ImageType2D::PointType centerPt,
                                     DistPointPair2D &maxDiameterTuple)
{
  int numBoundaryPts = boundaryPts.size();
  int i, j;
  vector<DistPointPair2D> distPtVector;

  // Find all distance combinations, excluding duplicates
  double maxDist = 0;
  Point2DPair maxDiameterPair;
  for(i=0;i<numBoundaryPts;i++)
    {
    for(j=i+1;j<numBoundaryPts;j++)
      {
      double dist;

      ImageType2D::PointType projectionPt;
      projectionPt = FindProjectionPoint2D(boundaryPts[i], boundaryPts[j], centerPt);

      // take only those diameter measurements that are within the spacing
      // size from the largest diameter
      if(PtDistance(projectionPt, centerPt)>.1)
        continue;
      dist = PtDistance(boundaryPts[i],boundaryPts[j]);
      distPtVector.push_back(
        DistPointPair2D(dist,Point2DPair(boundaryPts[i],boundaryPts[j])));
      if(dist>maxDist)
        {
        maxDiameterPair = Point2DPair(boundaryPts[i],boundaryPts[j]);
        maxDist = dist;
        }
      }
    }

  // sort them by the estimated diameter
  sort(distPtVector.begin(), distPtVector.end(), DistPtPairSort());

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
  for(vector<DistPointPair2D>::const_iterator pIt = distPtVector.begin();
      pIt!=distPtVector.end();++pIt,++i)
    {

    double currentDiameter;

    currentDiameter = (*pIt).first;
    currentDiameter = EstimateDiameterBresenham(slice, 
      (*pIt).second.first, (*pIt).second.second, currentDiameter);
    
    // if the current estimate is larger than the distance between the next
    // tuple of points, then we have the largest diameter possible
    if(currentDiameter > diameterEstimate)
      {
      diameterEstimate = currentDiameter;
      maxDiameterTuple = *pIt;
      }

    if(i>tupleCnt-1)
      if(diameterEstimate > (*(pIt+1)).first)
        break;
    }
  
  return diameterEstimate;
}

double GetMaxDiameterEstimate(ImageType::Pointer image, DistPointPair3D& maxDiameterTuple3D)
{
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
  try
    {
    extractor->Update();
    }
  catch(itk::ExceptionObject &e)
    {
    cout << "Exception in extractor: " << e << endl;
    abort();
    }

  ImageType2D::Pointer slice = extractor->GetOutput();
  if(1)
    {
    WriterType2D::Pointer writer = WriterType2D::New();
    writer->SetFileName("slice.mha");
    writer->SetInput(slice);
    writer->Update();
    }
  

  vector<ImageType2D::PointType> boundaryPts;

  FindBoundaryPoints(slice, boundaryPts);

  DistPointPair2D maxDiameterTuple;
  double diameterEstimate;
  diameterEstimate = FindMaxDiameterTuple(slice, boundaryPts, maxDiameterTuple);

  ImageType::PointType maxDiameterPt0, maxDiameterPt1;
  for(i=0,j=0;i<3;i++)
    {
    if(extractionIndex[i])
      {
      maxDiameterPt0[i] = extractionPoint[i];
      maxDiameterPt1[i] = extractionPoint[i];
      }
    else
      {
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

void UpdateImageWithDiameter(ImageType::Pointer image, ImageType::PointType pt0, ImageType::PointType pt1)
{
  ImageType::IndexType idx0, idx1;

  image->TransformPhysicalPointToIndex(pt0, idx0);
  image->TransformPhysicalPointToIndex(pt1, idx1);

  LineIteratorType lit(image, idx0, idx1);
  for(lit.GoToBegin();!lit.IsAtEnd();++lit)
    if(lit.Get())
      lit.Set(80);
}

void SaveImage(ImageType::Pointer image, const char* name)
{
  WriterType::Pointer writer1 = WriterType::New();
  writer1->SetInput(image);
  writer1->SetFileName(name);
  writer1->Update();
}
      
// Project point p2 on the (p0,p1) line
// from http://local.wasp.uwa.edu.au/~pbourke/geometry/pointline/
ImageType2D::PointType FindProjectionPoint2D(ImageType2D::PointType p0,
                                             ImageType2D::PointType p1, ImageType2D::PointType p2)
{
  ImageType2D::PointType projectionPt;
  double u;

  u = (p2[0]-p0[0])*(p1[0]-p0[0])+(p2[1]-p0[1])*(p1[1]-p0[1]);
  u /= PtDistanceSquared(p0,p1);

  projectionPt[0] = p0[0]+u*(p1[0]-p0[0]);
  projectionPt[1] = p0[1]+u*(p1[1]-p0[1]);

  return projectionPt;
}

// given an image and a linse defined by a point and a vector, find the points of 
// intersection between the line and the bounding box
// The function returns the number of intersection points, and initializes the
// values of the intersection points passed as input accordingly
// Math from http://local.wasp.uwa.edu.au/~pbourke/geometry/planeline/
int FindLineBBoxIntersectionPoints(ImageType::Pointer image, 
  ImageType::PointType p, itk::Vector<double,3> v,
  ImageType::PointType &is0, ImageType::PointType &is1){
  ImageType::SizeType imageSize;
  ImageType::SpacingType imageSpacing;
  itk::Vector<double,3> sizePhys, sp0, sp1;
  itk::Vector<double,3> bbox[2];
  int i, j, iPointCount;
 
  imageSize = image->GetLargestPossibleRegion().GetSize();
  imageSpacing = image->GetSpacing();
  
  sizePhys[0] = imageSpacing[0]*(double)imageSize[0];
  sizePhys[1] = imageSpacing[1]*(double)imageSize[1];
  sizePhys[2] = imageSpacing[2]*(double)imageSize[2];
  
  bbox[0][0] = 0;
  bbox[0][1] = 0;
  bbox[0][2] = 0;
  bbox[1][0] = sizePhys[0];
  bbox[1][1] = sizePhys[1];
  bbox[1][2] = sizePhys[2];

  // scale the vector to make sure segment intersects the bounding box
  v.Normalize();
  v *= sqrt(sizePhys[0]*sizePhys[0]+sizePhys[1]*sizePhys[1]+sizePhys[2]*sizePhys[2]);
  
  
  sp0[0] = p[0]+v[0];
  sp0[1] = p[1]+v[1];
  sp0[2] = p[2]+v[2];
  sp1[0] = p[0]-v[0];
  sp1[1] = p[1]-v[1];
  sp1[2] = p[2]-v[2];

  iPointCount = 0;
  for(i=0;i<2;i++){
    itk::Vector<double, 3> planePoint = bbox[i];
    for(j=0;j<3;j++){
      itk::Vector<double, 3> planeNormal;
      double u;
      planeNormal.Fill(0);
      planeNormal[j] = bbox[1][j];

      if(fabs(planeNormal*(sp1-sp0))<1e-8)
        continue; // line is parallel to the plane
      u = planeNormal*(planePoint-sp0);
      u /= planeNormal*(sp1-sp0);
      if(u>0. && u<1.){
        if(iPointCount==0){
          is0[0] = sp0[0]+(sp1[0]-sp0[0])*u;
          is0[1] = sp0[1]+(sp1[1]-sp0[1])*u;
          is0[2] = sp0[2]+(sp1[2]-sp0[2])*u;
        } else {
          is1[0] = sp0[0]+(sp1[0]-sp0[0])*u;
          is1[1] = sp0[1]+(sp1[1]-sp0[1])*u;
          is1[2] = sp0[2]+(sp1[2]-sp0[2])*u;
        }
        iPointCount++;
        if(iPointCount==2)
          break;
      }
    }
    if(iPointCount==2)
      break;
  }
  return iPointCount;
}
