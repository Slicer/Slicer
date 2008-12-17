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
typedef itk::ImageRegionConstIteratorWithIndex<ImageType> IteratorType;
typedef itk::NeighborhoodIterator<ImageType2D> NeiIteratorType;
typedef itk::ExtractImageFilter<ImageType,ImageType2D> ExtractorType;
typedef itk::LineIterator<ImageType2D> LineIteratorType2D;
//typedef itk::Statistics::KdTree<ImageType2D::PointType> KdTreeType;
//typedef itk::SliceBySliceImageFilter<ImageType,ImageType> SliceFilterType;
typedef pair<ImageType2D::PointType,ImageType2D::PointType> PtPairType;
typedef pair<double,PtPairType> DistPtPairType;

struct DistPtPairSort
{
     bool operator()(DistPtPairType p0, DistPtPairType p1) { 
       return p0.first>p1.first;
     }
};

ImageType::IndexType FindLargestAreaSlice(ImageType::Pointer image);
double PtDistanceSquared(Point2DType, Point2DType);
double PtDistance(Point2DType p0, Point2DType p1);

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
   
  ImageType::IndexType extractionIndex = FindLargestAreaSlice(image);
  ImageType::RegionType extractionRegion;
  ImageType::SizeType extractionSize = image->GetLargestPossibleRegion().GetSize();
  int i, j;
  for(i=0;i<3;i++)
    if(extractionIndex[i])
      extractionSize[i] = 0;
  extractionRegion.SetSize(extractionSize);
  extractionRegion.SetIndex(extractionIndex);

  ExtractorType::Pointer extractor = ExtractorType::New();
  extractor->SetInput(image);
  extractor->SetExtractionRegion(extractionRegion);
  extractor->Update();

  ImageType2D::Pointer slice = extractor->GetOutput();


  vector<ImageType2D::PointType> boundaryPts;

  // Get the boundary pixels
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
  
  cout << boundaryPts.size() << " boundary points found" << endl;

  int numBoundaryPts = boundaryPts.size();
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

  // Follow the lines for each pair to find intersection points, exclude 
  // outside segments from the diameter estimate
  DistPtPairType maxDiameterTuple = distPtVector[0];
  maxDist = maxDiameterTuple.first;
  double diameterEstimate = 0;
  i = 0;
  int tupleCnt = distPtVector.size();
  for(vector<DistPtPairType>::const_iterator pIt = distPtVector.begin();
    pIt!=distPtVector.end();++pIt,++i){

    double currentDiameter;

    slice->TransformPhysicalPointToIndex((*pIt).second.first, idx0);
    slice->TransformPhysicalPointToIndex((*pIt).second.second, idx1);
    LineIteratorType2D lit(slice, idx0, idx1);
    bool outside = false;
    currentDiameter = (*pIt).first;
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
 
  cout << "Updated max diameter estimate: " << diameterEstimate << endl;

  slice->TransformPhysicalPointToIndex(maxDiameterTuple.second.first, idx0);
  slice->TransformPhysicalPointToIndex(maxDiameterTuple.second.second, idx1);
  LineIteratorType2D lit(slice, idx0, idx1);
  i = 0;
  for(lit.GoToBegin();!lit.IsAtEnd();++lit,++i)
    if(lit.Get())
      lit.Set(lit.Get()+80);
    
  if(outputFilename){
    WriterType2D::Pointer writer = WriterType2D::New();
    writer->SetInput(slice);
    writer->SetFileName(outputFilename);
    writer->Update();
  }
 
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

  bzero(sliceAcc[0], sizeof(double)*size[0]);
  bzero(sliceAcc[1], sizeof(double)*size[1]);
  bzero(sliceAcc[2], sizeof(double)*size[2]);

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

double PtDistanceSquared(Point2DType p0, Point2DType p1){
  return (p0[0]-p1[0])*(p0[0]-p1[0])+
    (p0[1]-p1[1])*(p0[1]-p1[1]);
}

double PtDistance(Point2DType p0, Point2DType p1){
  return sqrt((p0[0]-p1[0])*(p0[0]-p1[0])+
    (p0[1]-p1[1])*(p0[1]-p1[1]));
}
