/*=========================================================================
*
*  Copyright Insight Software Consortium
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*         http://www.apache.org/licenses/LICENSE-2.0.txt
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
*
*=========================================================================*/
#ifndef itkMorphologicalContourInterpolator_hxx
#define itkMorphologicalContourInterpolator_hxx

#include "itkAndImageFilter.h"
#include "itkBinaryBallStructuringElement.h"
#include "itkBinaryCrossStructuringElement.h"
#include "itkBinaryDilateImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkImageAlgorithm.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkMorphologicalContourInterpolator.h"
#include "itkMultiThreader.h"
#include "itkObjectFactory.h"
#include "itkOrImageFilter.h"
#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkSimpleFastMutexLock.h"
#include "itkThreadedIndexedContainerPartitioner.h"
#include "itkUnaryFunctorImageFilter.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <queue>
#include <utility>
#include <vector>

namespace itk
{
template< typename TImage >
struct SegmentBetweenTwo
{
  int axis;
  TImage* out;
  int label, i, j;
  typename MorphologicalContourInterpolator< TImage >::SliceType::Pointer iconn, jconn;
};

template< typename TImage >
class MorphologicalContourInterpolatorParallelInvoker:
  public itk::DomainThreader< itk::ThreadedIndexedContainerPartitioner, MorphologicalContourInterpolator< TImage > >
{
public:
  // Standard ITK typedefs.
  typedef MorphologicalContourInterpolatorParallelInvoker Self;
  typedef itk::DomainThreader< itk::ThreadedIndexedContainerPartitioner,
    MorphologicalContourInterpolator< TImage > >          Superclass;
  typedef itk::SmartPointer< Self >                       Pointer;
  typedef itk::SmartPointer< const Self >                 ConstPointer;

  // The domain is an index range.
  typedef typename Superclass::DomainType DomainType;

  // This creates the ::New() method for instantiating the class.
  itkNewMacro( Self );

  /** Array of segments which need to be interpolated. */
  void
  SetWorkArray( std::vector< SegmentBetweenTwo< TImage > >& workArray )
  {
    m_WorkArray = workArray;
  }

  /** Array of segments which need to be interpolated. */
  void
  ClearWorkArray()
  {
    m_WorkArray.clear();
  }

protected:
  // We need a constructor for the itkNewMacro.
  MorphologicalContourInterpolatorParallelInvoker() {}

private:
  virtual void
  ThreadedExecution( const DomainType& subDomain, const ThreadIdType threadId )
  {
    // Look only at the range of cells by the set of indices in the subDomain.
    for ( itk::IndexValueType ii = subDomain[0]; ii <= subDomain[1] && ii < IndexValueType( m_WorkArray.size() ); ++ii )
      {
      this->m_Associate->InterpolateBetweenTwo(
        m_WorkArray[ii].axis,
        m_WorkArray[ii].out,
        m_WorkArray[ii].label,
        m_WorkArray[ii].i,
        m_WorkArray[ii].j,
        m_WorkArray[ii].iconn,
        m_WorkArray[ii].jconn,
        threadId );
      }
  } // ThreadedExecution

  std::vector< SegmentBetweenTwo< TImage > > m_WorkArray;
};

template< typename TImage >
bool
MorphologicalContourInterpolator< TImage >
::ImagesEqual( typename BoolSliceType::Pointer& a, typename BoolSliceType::Pointer& b )
{
  ImageRegionConstIterator< BoolSliceType > ita( a, a->GetLargestPossibleRegion() );
  ImageRegionConstIterator< BoolSliceType > itb( b, b->GetLargestPossibleRegion() );

  while ( !ita.IsAtEnd() )
    {
    if ( ita.Get() != itb.Get() )
      {
      break;
      }
    ++ita;
    ++itb;
    }

  if ( ita.IsAtEnd() )
    {
    return true;
    }
  else
    {
    return false;
    }
} // >::ImagesEqual

template< typename TImage >
MorphologicalContourInterpolator< TImage >
::MorphologicalContourInterpolator()
  : m_Label( 0 ),
  m_Axis( -1 ),
  m_HeuristicAlignment( true ),
  m_UseDistanceTransform( true ),
  m_UseBallStructuringElement( false ),
  m_UseCustomSlicePositions( false ),
  m_MinAlignIters( pow( 2, TImage::ImageDimension ) ), // smaller of this and pixel count of the search image
  m_MaxAlignIters( pow( 6, TImage::ImageDimension ) ), // bigger of this and root of pixel count of the search image
  m_ThreadCount( MultiThreader::GetGlobalDefaultNumberOfThreads() ),
  m_LabeledSlices( TImage::ImageDimension ) // initialize with empty sets
{
  // set up pipeline for regioned connected components
  m_RoI = RoiType::New();
  m_RoI->SetDirectionCollapseToIdentity();
  m_Binarizer = BinarizerType::New();
  m_Binarizer->SetInput( m_RoI->GetOutput() );
  m_ConnectedComponents = ConnectedComponentsType::New();
  m_ConnectedComponents->SetInput( m_Binarizer->GetOutput() );
  // FullyConnected is related to structuring element used
  // true for ball, false for cross
  m_ConnectedComponents->SetFullyConnected( m_UseBallStructuringElement );
}

template< typename TImage >
template< typename T2 >
void
MorphologicalContourInterpolator< TImage >
::ExpandRegion( typename T2::RegionType& region, const typename T2::IndexType& index )
{
  for ( unsigned int a = 0; a < T2::ImageDimension; ++a )
    {
    if ( region.GetIndex( a ) > index[a] )
      {
      region.SetSize( a, region.GetSize( a ) + region.GetIndex( a ) - index[a] );
      region.SetIndex( a, index[a] );
      }
    else if ( region.GetIndex( a ) + (typename T2::IndexValueType)region.GetSize( a ) <= index[a] )
      {
      region.SetSize( a, index[a] - region.GetIndex( a ) + 1 );
      }
    // else it is already within
    }
}

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::DetermineSliceOrientations()
{
  m_LabeledSlices.clear();
  m_LabeledSlices.resize( TImage::ImageDimension ); // initialize with empty sets
  m_BoundingBoxes.clear();

  typename TImage::ConstPointer m_Input = this->GetInput();
  typename TImage::Pointer m_Output = this->GetOutput();

  typename TImage::RegionType region = m_Output->GetRequestedRegion();
  ImageRegionConstIteratorWithIndex< TImage > it( m_Input, region );
  while ( !it.IsAtEnd() )
    {
    typename TImage::IndexType indPrev, indNext;
    const typename TImage::IndexType ind = it.GetIndex();
    const typename TImage::PixelType val = m_Input->GetPixel( ind );
    if ( val != 0 )
      {
      typename TImage::RegionType boundingBox1;
      boundingBox1.SetIndex( ind );
      for ( unsigned int a = 0; a < TImage::ImageDimension; ++a )
        {
        boundingBox1.SetSize( a, 1 );
        }
      std::pair< typename BoundingBoxesType::iterator, bool > resBB
        = m_BoundingBoxes.insert( std::make_pair( val, boundingBox1 ) );
      if ( !resBB.second ) // include this index in existing BB
        {
        ExpandRegion< TImage >( resBB.first->second, ind );
        }

      unsigned int cTrue = 0;
      unsigned int cAdjacent = 0;
      unsigned int axis = 0;
      for ( unsigned int a = 0; a < TImage::ImageDimension; ++a )
        {
        indPrev = ind;
        indPrev[a]--;
        indNext = ind;
        indNext[a]++;
        typename TImage::PixelType prev = 0;
        if ( region.IsInside( indPrev ) )
          {
          prev = m_Input->GetPixel( indPrev );
          }
        typename TImage::PixelType next = 0;
        if ( region.IsInside( indNext ) )
          {
          next = m_Input->GetPixel( indNext );
          }
        if ( prev == 0 && next == 0 ) // && - isolated slices only, || - flat edges too
          {
          axis = a;
          ++cTrue;
          }
        else if ( prev == val && next == val )
          {
          ++cAdjacent;
          }
        }
      if ( cTrue == 1 && cAdjacent == TImage::ImageDimension - 1 )
      // slice has empty adjacent space only along one axis
        {
        if ( m_Axis == -1 || m_Axis == int(axis) )
          {
          m_LabeledSlices[axis][val].insert( ind[axis] );
          }
        }
      }
    ++it;
    }
} // >::DetermineSliceOrientations

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::Extrapolate( int axis,
  TImage* out,
  typename TImage::PixelType label,
  typename TImage::IndexValueType i,
  typename TImage::IndexValueType j,
  typename SliceType::Pointer& iConn,
  typename TImage::PixelType iRegionId,
  ThreadIdType threadId )
{
  PixelList jRegionIds;

  jRegionIds.push_back( iRegionId );
  typename SliceType::IndexType centroid = Centroid( iConn, jRegionIds );

  typename SliceType::RegionType reg3;
  typename SliceType::SizeType size;
  size.Fill( 3 );
  reg3.SetSize( size );

  typename SliceType::IndexType phIndex;
  for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
    {
    phIndex[d] = centroid.GetIndex()[d] - 1;
    }
  reg3.SetIndex( phIndex );

  // create a phantom small slice centered around centroid
  typename SliceType::Pointer phSlice = SliceType::New();
  phSlice->CopyInformation( iConn );
  phSlice->SetRegions( reg3 );
  phSlice->Allocate( true );

  // add a phantom point to the center of a newly constructed slice
  phSlice->SetPixel( centroid, iRegionId );

  // do alignment in case the iShape is concave and centroid is not within the iShape
  typename SliceType::IndexType translation = Align( iConn, iRegionId, phSlice, jRegionIds );
  // now translate the phantom slice for best alignment
  for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
    {
    phIndex[d] -= translation[d];
    }
  reg3.SetIndex( phIndex );
  phSlice->SetRegions( reg3 );
  typename SliceType::IndexType t0;
  t0.Fill( 0 );
  Interpolate1to1( axis, out, label, i, j, iConn, iRegionId, phSlice, iRegionId, t0, false, threadId );
} // >::Extrapolate

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::BoolSliceType::Pointer
MorphologicalContourInterpolator< TImage >
::Dilate1( typename BoolSliceType::Pointer& seed, typename BoolSliceType::Pointer& mask, ThreadIdType threadId )
{
  // set up structuring element for dilation
  typedef BinaryCrossStructuringElement< typename BoolSliceType::PixelType,
    BoolSliceType::ImageDimension > CrossStructuringElementType;
  typedef BinaryBallStructuringElement< typename BoolSliceType::PixelType,
    BoolSliceType::ImageDimension > BallStructuringElementType;
  typedef BinaryDilateImageFilter< BoolSliceType, BoolSliceType,
    CrossStructuringElementType >   CrossDilateType;
  typedef BinaryDilateImageFilter< BoolSliceType, BoolSliceType,
    BallStructuringElementType >    BallDilateType;

  static std::vector< bool > initialized( m_ThreadCount ); // default: false
  static std::vector< typename CrossDilateType::Pointer > m_CrossDilator( m_ThreadCount );
  static std::vector< typename BallDilateType::Pointer > m_BallDilator( m_ThreadCount );
  static std::vector< CrossStructuringElementType > m_CrossStructuringElement( m_ThreadCount );
  static std::vector< BallStructuringElementType > m_BallStructuringElement( m_ThreadCount );
  typedef AndImageFilter< BoolSliceType, BoolSliceType, BoolSliceType > AndFilterType;
  static std::vector< typename AndFilterType::Pointer > m_And( m_ThreadCount );

  if ( !initialized[threadId] ) // make sure these non-trivial operations are executed only once per thread
    {
    m_CrossDilator[threadId] = CrossDilateType::New();
    m_BallDilator[threadId] = BallDilateType::New();
    m_And[threadId] = AndFilterType::New();
    m_And[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    typedef Size< BoolSliceType::ImageDimension > SizeType;
    SizeType size;
    size.Fill( 1 );

    m_CrossDilator[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    m_CrossStructuringElement[threadId].SetRadius( size );
    m_CrossStructuringElement[threadId].CreateStructuringElement();
    m_CrossDilator[threadId]->SetKernel( m_CrossStructuringElement[threadId] );

    m_BallDilator[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    m_BallStructuringElement[threadId].SetRadius( size );
    m_BallStructuringElement[threadId].CreateStructuringElement();
    m_BallDilator[threadId]->SetKernel( m_BallStructuringElement[threadId] );

    initialized[threadId] = true;
    }

  typename BoolSliceType::Pointer temp;
  if ( m_UseBallStructuringElement )
    {
    m_BallDilator[threadId]->SetInput( seed );
    m_BallDilator[threadId]->GetOutput()->SetRegions( seed->GetRequestedRegion() );
    m_BallDilator[threadId]->Update();
    temp = m_BallDilator[threadId]->GetOutput();
    }
  else
    {
    m_CrossDilator[threadId]->SetInput( seed );
    m_CrossDilator[threadId]->GetOutput()->SetRegions( seed->GetRequestedRegion() );
    m_CrossDilator[threadId]->Update();
    temp = m_CrossDilator[threadId]->GetOutput();
    }
  temp->DisconnectPipeline();
  // temp->SetRegions(mask->GetLargestPossibleRegion()); //not needed when seed and mask have same regions

  m_And[threadId]->SetInput( 0, mask );
  m_And[threadId]->SetInput( 1, temp );
  m_And[threadId]->GetOutput()->SetRegions( seed->GetRequestedRegion() );
  m_And[threadId]->Update();
  typename BoolSliceType::Pointer result = m_And[threadId]->GetOutput();
  result->DisconnectPipeline();
  return result;
} // >::Dilate1

template< typename TImage >
std::vector< typename MorphologicalContourInterpolator< TImage >::BoolSliceType::Pointer >
MorphologicalContourInterpolator< TImage >
::GenerateDilationSequence( typename BoolSliceType::Pointer& begin,
  typename BoolSliceType::Pointer& end,
  ThreadIdType threadId )
{
  std::vector< typename BoolSliceType::Pointer > seq;
  seq.push_back( Dilate1( begin, end, threadId ) );

  do
    {
    seq.back()->DisconnectPipeline();
    seq.push_back( Dilate1( seq.back(), end, threadId ) );
    }
  while ( !ImagesEqual( seq.back(), seq[seq.size() - 2] ) );

  seq.pop_back(); // remove duplicate image
  return seq;
}

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::BoolSliceType::Pointer
MorphologicalContourInterpolator< TImage >
::FindMedianImageDilations( typename BoolSliceType::Pointer& intersection,
  typename BoolSliceType::Pointer& iMask,
  typename BoolSliceType::Pointer& jMask,
  ThreadIdType threadId )
{
  std::vector< typename BoolSliceType::Pointer > iSeq = GenerateDilationSequence( intersection, iMask, threadId );
  std::vector< typename BoolSliceType::Pointer > jSeq = GenerateDilationSequence( intersection, jMask, threadId );
  std::reverse( iSeq.begin(), iSeq.end() ); // we want to start from i and end at intersection
  if ( iSeq.size() < jSeq.size() )
    {
    iSeq.swap( jSeq ); // swap so iSeq.size() >= jSeq.size()
    }
  float ratio = float( jSeq.size() ) / iSeq.size();

  // generate union of transition sequences
  typedef OrImageFilter< BoolSliceType > OrType;
  static std::vector< bool > initialized( m_ThreadCount ); // default: false
  static std::vector< typename OrType::Pointer > m_Or( m_ThreadCount );
  if ( !initialized[threadId] )
    {
    m_Or[threadId] = OrType::New();
    m_Or[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    initialized[threadId] = true;
    }

  std::vector< typename BoolSliceType::Pointer > seq;
  for ( unsigned x = 0; x < iSeq.size(); x++ )
    {
    m_Or[threadId]->SetInput( 0, iSeq[x] );
    unsigned xj = ratio * x;
    m_Or[threadId]->SetInput( 1, jSeq[xj] );
    m_Or[threadId]->GetOutput()->SetRegions( iMask->GetRequestedRegion() );
    m_Or[threadId]->Update();
    seq.push_back( m_Or[threadId]->GetOutput() );
    seq.back()->DisconnectPipeline();
    }

  // find median
  unsigned minIndex = 0;
  IdentifierType min = iMask->GetRequestedRegion().GetNumberOfPixels();
  for ( unsigned x = 0; x < iSeq.size(); x++ )
    {
    IdentifierType iS = CardSymDifference( seq[x], iMask );
    IdentifierType jS = CardSymDifference( seq[x], jMask );
    IdentifierType xScore = iS >= jS ? iS - jS : jS - iS; // std::abs(iS-jS)
    if ( xScore < min )
      {
      min = xScore;
      minIndex = x;
      }
    }
  return seq[minIndex];
} // >::FindMedianImageDilations

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::FloatSliceType::Pointer
MorphologicalContourInterpolator< TImage >
::MaurerDM( typename BoolSliceType::Pointer& mask, ThreadIdType threadId )
{
  typedef itk::SignedMaurerDistanceMapImageFilter< BoolSliceType, FloatSliceType > FilterType;
  static std::vector< bool > initialized( m_ThreadCount ); // default: false
  static std::vector< typename FilterType::Pointer > filter( m_ThreadCount );
  if ( !initialized[threadId] )
    {
    filter[threadId] = FilterType::New();
    filter[threadId]->SetUseImageSpacing( false ); // interpolation algorithm calls for working in index space
    filter[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    initialized[threadId] = true;
    }
  filter[threadId]->SetInput( mask );
  filter[threadId]->GetOutput()->SetRequestedRegion( mask->GetRequestedRegion() );
  filter[threadId]->Update();
  return filter[threadId]->GetOutput();
}

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::BoolSliceType::Pointer
MorphologicalContourInterpolator< TImage >
::FindMedianImageDistances( typename BoolSliceType::Pointer& intersection,
  typename BoolSliceType::Pointer& iMask,
  typename BoolSliceType::Pointer& jMask,
  ThreadIdType threadId )
{
  // calculate distance field
  typename FloatSliceType::Pointer sdf = MaurerDM( intersection, threadId );

  // create histograms of distances and union
  typename BoolSliceType::Pointer orImage = BoolSliceType::New();
  orImage->CopyInformation( intersection );
  orImage->SetRegions( iMask->GetRequestedRegion() );
  orImage->Allocate( true );
  std::vector< long long > iHist;
  std::vector< long long > jHist;
  ImageRegionConstIterator< BoolSliceType > iti( iMask, iMask->GetRequestedRegion() );
  ImageRegionConstIterator< BoolSliceType > itj( jMask, iMask->GetRequestedRegion() );
  ImageRegionIterator< BoolSliceType > ito( orImage, iMask->GetRequestedRegion() );
  ImageRegionConstIterator< FloatSliceType > itsdf( sdf, iMask->GetRequestedRegion() );
  const short fractioning = 10; // how many times more precise distance than rounding to int
  while ( !itsdf.IsAtEnd() )
    {
    bool iM = iti.Get();
    bool jM = itj.Get();
    typename TImage::PixelType dist = fractioning * itsdf.Get();
    if ( iM && !jM )
      {
      if ( size_t(dist) >= iHist.size() )
        {
        iHist.resize( dist + 1, 0 );
        }
      iHist[dist]++;
      ito.Set( true );
      }
    else if ( jM && !iM )
      {
      if ( size_t(dist) >= jHist.size() )
        {
        jHist.resize( dist + 1, 0 );
        }
      jHist[dist]++;
      ito.Set( true );
      }
    else if ( iM && jM )
      {
      ito.Set( true );
      }

    ++iti;
    ++itj;
    ++ito;
    ++itsdf;
    }

  // sum of histogram bins for i and j and
  std::vector< long long >::size_type maxSize = std::max( iHist.size(), jHist.size() );
  if ( maxSize == 0 )
    {
    return intersection;
    }
  iHist.resize( maxSize, 0 );
  jHist.resize( maxSize, 0 );
  assert( iHist[0] == 0 );
  assert( jHist[0] == 0 );
  std::vector< long long > iSum( maxSize, 0 );
  std::vector< long long > jSum( maxSize, 0 );
  for ( unsigned b = 1; b < maxSize; b++ )
    {
    iSum[b] = iSum[b - 1] + iHist[b];
    jSum[b] = jSum[b - 1] + jHist[b];
    }
  long long iTotal = iSum[maxSize - 1];
  long long jTotal = jSum[maxSize - 1];

  // find minimum of differences of sums
  int bestBin = 0;
  long long bestDiff = LLONG_MAX;
  for ( unsigned b = 0; b < maxSize; b++ )
    {
    long long iS = std::abs( iTotal - iSum[b] + jSum[b] );
    long long jS = std::abs( jTotal - jSum[b] + iSum[b] );
    long long diff = std::abs( iS - jS );
    if ( diff < bestDiff )
      {
      bestDiff = diff;
      bestBin = b;
      }
    }

  // threshold at distance bestBin is the median intersection
  typedef BinaryThresholdImageFilter< FloatSliceType, BoolSliceType >   FloatBinarizerType;
  typedef AndImageFilter< BoolSliceType, BoolSliceType, BoolSliceType > AndFilterType;
  static std::vector< bool > initialized( m_ThreadCount ); // default: false
  static std::vector< typename FloatBinarizerType::Pointer > threshold( m_ThreadCount );
  static std::vector< typename AndFilterType::Pointer > m_And( m_ThreadCount );
  if ( !initialized[threadId] )
    {
    threshold[threadId] = FloatBinarizerType::New();
    threshold[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    m_And[threadId] = AndFilterType::New();
    m_And[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    initialized[threadId] = true;
    }
  threshold[threadId]->SetInput( sdf );
  threshold[threadId]->SetUpperThreshold( float( bestBin ) / fractioning );
  threshold[threadId]->GetOutput()->SetRequestedRegion( sdf->GetRequestedRegion() );
  threshold[threadId]->Update();

  m_And[threadId]->SetInput( threshold[threadId]->GetOutput() );
  m_And[threadId]->SetInput( 1, orImage );
  m_And[threadId]->GetOutput()->SetRequestedRegion( orImage->GetRequestedRegion() );
  m_And[threadId]->Update();
  typename BoolSliceType::Pointer median = m_And[threadId]->GetOutput();
  return median;
} // >::FindMedianImageDistances

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::SliceType::RegionType
MorphologicalContourInterpolator< TImage >
::BoundingBox( itk::SmartPointer< SliceType > image )
{
  typename SliceType::RegionType newRegion = image->GetLargestPossibleRegion();
  typename SliceType::IndexType minInd = newRegion.GetIndex() + newRegion.GetSize();
  typename SliceType::IndexType maxInd = newRegion.GetIndex();
  ImageRegionConstIteratorWithIndex< SliceType > iIt( image, newRegion );

  while ( !iIt.IsAtEnd() )
    {
    if ( iIt.Get() )
      {
      typename SliceType::IndexType ind = iIt.GetIndex();
      for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
        {
        if ( ind[d] < minInd[d] )
          {
          minInd[d] = ind[d];
          }
        if ( ind[d] > maxInd[d] )
          {
          maxInd[d] = ind[d];
          }
        }
      }
    ++iIt;
    }

  newRegion.SetIndex( minInd );
  for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
    {
    newRegion.SetSize( d, maxInd[d] - minInd[d] + 1 );
    }
  return newRegion;
} // >::BoundingBox

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::Interpolate1to1( int axis,
  TImage* out,
  typename TImage::PixelType label,
  typename TImage::IndexValueType i,
  typename TImage::IndexValueType j,
  typename SliceType::Pointer& iConn,
  typename TImage::PixelType iRegionId,
  typename SliceType::Pointer& jConn,
  typename TImage::PixelType jRegionId,
  const typename SliceType::IndexType& translation,
  bool recursive,
  ThreadIdType threadId )
{
  // translate iConn by t/2 and jConn by -t/2
  typename SliceType::IndexType iTrans;
  typename SliceType::IndexType jTrans;
  typename SliceType::RegionType iRegion = iConn->GetLargestPossibleRegion();
  typename SliceType::RegionType jRegion = jConn->GetLargestPossibleRegion();
  typename SliceType::IndexType jBottom = jRegion.GetIndex();
  bool carry = false;
  for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
    {
    if ( !carry )
      {
      iTrans[d] = translation[d] / 2;
      carry = translation[d] % 2;
      }
    else if ( translation[d] % 2 == 0 )
      {
      iTrans[d] = translation[d] / 2;
      }
    else // use carry
      {
      if ( translation[d] > 0 )
        {
        iTrans[d] = translation[d] / 2 + 1;
        }
      else
        {
        iTrans[d] = translation[d] / 2 - 1;
        }
      carry = false;
      }
    jTrans[d] = iTrans[d] - translation[d];
    iRegion.SetIndex( d, iRegion.GetIndex()[d] + iTrans[d] );
    jRegion.SetIndex( d, jRegion.GetIndex()[d] + jTrans[d] );
    jBottom[d] = jRegion.GetIndex()[d] + jRegion.GetSize( d ) - 1;
    }
  typename SliceType::RegionType newRegion = iRegion;
  ExpandRegion< SliceType >( newRegion, jRegion.GetIndex() );
  ExpandRegion< SliceType >( newRegion, jBottom );
  typename SliceType::IndexValueType mid = ( i + j + ( carry ? 1 : 0 ) ) / 2; // index of middle slice

  typename SliceType::Pointer iConnT = TranslateImage( iConn, iTrans, newRegion );
  typename SliceType::Pointer jConnT = TranslateImage( jConn, jTrans, newRegion );

  if ( !recursive ) // reduce newRegion to bounding box so we deal with less pixels
    {
    newRegion = BoundingBox( iConnT );
    typename SliceType::RegionType jBB = BoundingBox( jConnT );
    typename SliceType::IndexType i2 = jBB.GetIndex();
    ExpandRegion< SliceType >( newRegion, i2 );
    for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
      {
      i2[d] += jBB.GetSize( d ) - 1;
      }
    ExpandRegion< SliceType >( newRegion, i2 );
    }

  // create and allocate slice binary masks
  typename BoolSliceType::Pointer iSlice = BoolSliceType::New();
  iSlice->CopyInformation( iConnT );
  iSlice->SetRegions( newRegion );
  iSlice->Allocate( true );
  typename BoolSliceType::Pointer jSlice = BoolSliceType::New();
  jSlice->CopyInformation( jConnT );
  jSlice->SetRegions( newRegion );
  jSlice->Allocate( true );

  // convert to binary by iteration
  ImageRegionConstIterator< SliceType > iIt( iConnT, newRegion );
  ImageRegionConstIterator< SliceType > jIt( jConnT, newRegion );
  ImageRegionIterator< BoolSliceType > ibIt( iSlice, newRegion );
  ImageRegionIterator< BoolSliceType > jbIt( jSlice, newRegion );
  while ( !iIt.IsAtEnd() )
    {
    if ( iIt.Get() == iRegionId )
      {
      ibIt.Set( true );
      }
    if ( jIt.Get() == jRegionId )
      {
      jbIt.Set( true );
      }
    ++iIt;
    ++jIt;
    ++ibIt;
    ++jbIt;
    }

  // create intersection
  typedef AndImageFilter< BoolSliceType > AndSliceType;
  static std::vector< bool > initialized( m_ThreadCount ); // default: false
  static std::vector< typename AndSliceType::Pointer > sAnd( m_ThreadCount );
  if ( !initialized[threadId] )
    {
    sAnd[threadId] = AndSliceType::New();
    sAnd[threadId]->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
    initialized[threadId] = true;
    }
  sAnd[threadId]->SetInput( 0, iSlice );
  sAnd[threadId]->SetInput( 1, jSlice );
  sAnd[threadId]->GetOutput()->SetRegions( iSlice->GetRequestedRegion() );
  sAnd[threadId]->Update();
  typename BoolSliceType::Pointer intersection = sAnd[threadId]->GetOutput();
  intersection->DisconnectPipeline();

  typename BoolSliceType::Pointer median;
  if ( m_UseDistanceTransform )
    {
    median = FindMedianImageDistances( intersection, iSlice, jSlice, threadId );
    }
  else
    {
    median = FindMedianImageDilations( intersection, iSlice, jSlice, threadId );
    }

  // finally write it out into the output image pointer
  typename TImage::RegionType outRegion = this->GetOutput()->GetRequestedRegion();
  typename SliceType::RegionType sliceRegion;
  for ( int d = 0; d < int(TImage::ImageDimension) - 1; d++ )
    {
    if ( d < axis )
      {
      sliceRegion.SetIndex( d, outRegion.GetIndex( d ) );
      sliceRegion.SetSize( d, outRegion.GetSize( d ) );
      }
    else
      {
      sliceRegion.SetIndex( d, outRegion.GetIndex( d + 1 ) );
      sliceRegion.SetSize( d, outRegion.GetSize( d + 1 ) );
      }
    }
  typename SliceType::IndexType t0;
  t0.Fill( 0 );
  IntersectionRegions( t0, sliceRegion, newRegion ); // clips new region to output region
  // sliceRegion possibly shrunk, copy it into outRegion
  for ( int d = 0; d < int(TImage::ImageDimension) - 1; d++ )
    {
    if ( d < axis )
      {
      outRegion.SetIndex( d, sliceRegion.GetIndex( d ) );
      outRegion.SetSize( d, sliceRegion.GetSize( d ) );
      }
    else
      {
      outRegion.SetIndex( d + 1, sliceRegion.GetIndex( d ) );
      outRegion.SetSize( d + 1, sliceRegion.GetSize( d ) );
      }
    }
  outRegion.SetIndex( axis, mid );
  outRegion.SetSize( axis, 1 );

  typename SliceType::Pointer midConn = SliceType::New();
  midConn->CopyInformation( iConnT );
  midConn->SetRegions( newRegion );
  midConn->Allocate( true );

  ImageRegionConstIterator< BoolSliceType > seqIt( median, newRegion );
  ImageRegionIterator< SliceType > midIt( midConn, newRegion );
  while ( !seqIt.IsAtEnd() )
    {
    if ( seqIt.Get() )
      {
      midIt.Set( 1 );
      }
    ++seqIt;
    ++midIt;
    }

  bool withinReq = true;
  typename TImage::RegionType reqRegion = this->GetOutput()->GetRequestedRegion();
  for ( unsigned d = 0; d < TImage::ImageDimension; d++ )
    {
    if ( outRegion.GetIndex( d ) < reqRegion.GetIndex( d ) ||
         outRegion.GetIndex( d ) + outRegion.GetSize( d ) >
         reqRegion.GetIndex( d ) + reqRegion.GetSize( d ) )
      {
      withinReq = false;
      break;
      }
    }

  static SimpleFastMutexLock mutex;
  if ( withinReq ) // else we should not write it
    {
    seqIt.GoToBegin();
    // writing through one RLEImage iterator invalidates all the others
    // so this whole writing loop needs to be serialized
    mutex.Lock();
    ImageRegionIterator< TImage > outIt( out, outRegion );
    while ( !outIt.IsAtEnd() )
      {
      if ( seqIt.Get() && outIt.Get() < label )
        {
        outIt.Set( label );
        }
      ++seqIt;
      ++outIt;
      }

    mutex.Unlock();
    } // iterator destroyed here

  // recurse if needed
  if ( std::abs( i - j ) > 2 )
    {
    PixelList regionIDs;
    regionIDs.push_back( 1 );

    int iReq = i < reqRegion.GetIndex( axis ) ? -1 :
      ( i > reqRegion.GetIndex( axis ) + IndexValueType( reqRegion.GetSize( axis ) ) ? +1 : 0 );
    int jReq = j < reqRegion.GetIndex( axis ) ? -1 :
      ( j > reqRegion.GetIndex( axis ) + IndexValueType( reqRegion.GetSize( axis ) ) ? +1 : 0 );
    int mReq = mid < reqRegion.GetIndex( axis ) ? -1 :
      ( mid > reqRegion.GetIndex( axis ) + IndexValueType( reqRegion.GetSize( axis ) ) ? +1 : 0 );
    bool first = std::abs( i - mid ) > 1 && std::abs( iReq + mReq ) <= 1; // i-mid?
    bool second = std::abs( j - mid ) > 1 && std::abs( jReq + mReq ) <= 1; // j-mid?

    if ( first )
      {
      Interpolate1to1( axis, out, label, i, mid, iConn, iRegionId, midConn, 1, iTrans, true, threadId );
      }
    if ( second )
      {
      Interpolate1to1( axis, out, label, j, mid, jConn, jRegionId, midConn, 1, jTrans, true, threadId );
      }
    }
} // >::Interpolate1to1

template< typename TImage >
class MatchesID
{
  typename TImage::PixelType m_ID;

public:
  MatchesID() {}
  MatchesID( typename TImage::PixelType id )
    : m_ID( id )
  {}
  bool
  operator!=( const MatchesID& other )
  {
    return m_ID != other.m_ID;
  }

  bool
  operator==( const MatchesID& other )
  {
    return m_ID == other.m_ID;
  }

  inline bool
  operator()( const typename TImage::PixelType& val ) const
  {
    return val == m_ID;
  }
};

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::Interpolate1toN( int axis,
  TImage* out,
  typename TImage::PixelType label,
  typename TImage::IndexValueType i,
  typename TImage::IndexValueType j,
  typename SliceType::Pointer& iConn,
  typename TImage::PixelType iRegionId,
  typename SliceType::Pointer& jConn,
  const PixelList& jRegionIds,
  const typename SliceType::IndexType& translation,
  ThreadIdType threadId )
{
  // first convert iConn into binary mask
  MatchesID< TImage > matchesID( iRegionId );

  typedef UnaryFunctorImageFilter< SliceType, BoolSliceType, MatchesID< TImage > > CastType;
  typename CastType::Pointer caster = CastType::New();
  caster->SetNumberOfThreads( 1 ); // excessive threading is counterproductive
  caster->SetFunctor( matchesID );
  caster->SetInput( iConn );
  caster->Update();
  typename BoolSliceType::Pointer mask = caster->GetOutput();

  typename SliceType::RegionType iRegion, jRegion, newjRegion;
  iRegion = iConn->GetLargestPossibleRegion();
  jRegion = jConn->GetLargestPossibleRegion();
  newjRegion = jRegion;
  newjRegion.SetSize( iRegion.GetSize() );

  // construct n empty images
  std::vector< typename BoolSliceType::Pointer > blobs;
  for ( unsigned x = 0; x < jRegionIds.size(); x++ )
    {
    typename BoolSliceType::Pointer temp = BoolSliceType::New();
    temp->CopyInformation( jConn );
    temp->SetRegions( iRegion );
    temp->Allocate( true );
    blobs.push_back( temp );
    }

  // fill the n images with intersections - these are seeds
  typename SliceType::Pointer belongs = SliceType::New();
  belongs->CopyInformation( mask );
  belongs->SetRegions( iRegion );
  belongs->Allocate( true ); // initialize to zero (false)
  ImageRegionIterator< SliceType > belongIt( belongs, iRegion );
  IntersectionRegions( translation, iRegion, jRegion );
  ImageRegionConstIterator< BoolSliceType > maskIt( mask, iRegion );
  ImageRegionConstIteratorWithIndex< SliceType > jIt( jConn, jRegion );
  ImageRegionIterator< SliceType > belongInit( belongs, iRegion );

  // convert jConn into n blobs, translating them into the index space of iConn
  while ( !maskIt.IsAtEnd() )
    {
    if ( maskIt.Get() )
      {
      typename TImage::PixelType jVal = jIt.Get();
      typename PixelList::const_iterator res = std::find( jRegionIds.begin(), jRegionIds.end(), jVal );
      if ( res != jRegionIds.end() )
        {
        blobs[res - jRegionIds.begin()]->SetPixel( maskIt.GetIndex(), true );
        belongInit.Set( res - jRegionIds.begin() + 1 );
        }
      }
    ++maskIt;
    ++jIt;
    ++belongInit;
    }

  // prepare dilation filter
  iRegion = iConn->GetLargestPossibleRegion(); // expand to full i image
  for ( unsigned x = 0; x < jRegionIds.size(); x++ )
    {
    blobs[x]->SetRegions( iRegion );
    }
  ImageRegionConstIterator< BoolSliceType > maskIt2( mask, iRegion );
  ImageRegionConstIteratorWithIndex< BoolSliceType > jIt2( blobs[0], iRegion );

  bool hollowedMaskEmpty;

  do // while hollowed mask is not empty
    {
    for ( unsigned x = 0; x < jRegionIds.size(); x++ )
      {
      blobs[x] = Dilate1( blobs[x], mask, threadId );
      blobs[x]->DisconnectPipeline();
      }

    hollowedMaskEmpty = true;
    maskIt2.GoToBegin();
    jIt2.GoToBegin();
    belongIt.GoToBegin();
    while ( !maskIt2.IsAtEnd() ) // hollow out the big mask with dilated seeds while avoiding conflicts
      {
      if ( maskIt2.Get() )
        {
        if ( !belongIt.Get() )
          {
          unsigned x = 0;
          while ( x < jRegionIds.size() )
            {
            if ( blobs[x]->GetPixel( jIt2.GetIndex() ) )
              {
              break;
              }
            ++x;
            }

          if ( x < jRegionIds.size() ) // covered by a blob, hollow it out
            {
            belongIt.Set( x + 1 );
            for ( x++; x < jRegionIds.size(); x++ )
              {
              // pixel does not belong to this blob
              blobs[x]->SetPixel( jIt2.GetIndex(), false );
              }
            }
          else // keep it
            {
            hollowedMaskEmpty = false;
            }
          }
        else // the pixel already belongs to some blob
          {
          for ( unsigned x = 0; x < jRegionIds.size(); x++ )
            {
            if ( unsigned(belongIt.Get()) != x + 1 )
              {
              // pixel does not belong to this blob
              blobs[x]->SetPixel( jIt2.GetIndex(), false );
              }
            }
          }
        }
      ++maskIt2;
      ++jIt2;
      ++belongIt;
      }
    }
  while ( !hollowedMaskEmpty );

  blobs.clear(); // deallocates the images

  // convert the belongs into n Conn-style images
  std::vector< typename SliceType::Pointer > conns;
  for ( unsigned x = 0; x < jRegionIds.size(); x++ )
    {
    typename SliceType::Pointer temp2 = SliceType::New();
    temp2->CopyInformation( iConn );
    temp2->SetRegions( iConn->GetLargestPossibleRegion() );
    temp2->Allocate( true );
    conns.push_back( temp2 );
    }
  ImageRegionConstIteratorWithIndex< SliceType > belongIt2( belongs, iRegion );
  while ( !belongIt2.IsAtEnd() )
    {
    const typename SliceType::PixelType belong = belongIt2.Get();
    if ( belong > 0 )
      {
      conns[belong - 1]->SetPixel( belongIt2.GetIndex(), iRegionId );
      }
    ++belongIt2;
    }
  // make n 1-to-1 interpolations
  for ( unsigned x = 0; x < jRegionIds.size(); x++ )
    {
    Interpolate1to1( axis, out, label, i, j, conns[x], iRegionId, jConn, jRegionIds[x], translation, false, threadId );
    }
} // >::Interpolate1toN

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::SliceType::Pointer
MorphologicalContourInterpolator< TImage >
::TranslateImage( typename SliceType::Pointer& image,
  const typename SliceType::IndexType& translation,
  typename SliceType::RegionType newRegion )
{
  typename SliceType::Pointer result = SliceType::New();
  result->CopyInformation( image );
  result->SetRegions( newRegion );
  result->Allocate( true ); // initialize to zero (false)
  typename SliceType::RegionType inRegion = image->GetLargestPossibleRegion();
  IntersectionRegions( translation, inRegion, newRegion );
  ImageAlgorithm::Copy< SliceType, SliceType >( image.GetPointer(), result.GetPointer(), inRegion, newRegion );
  return result;
}

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::IntersectionRegions( const typename SliceType::IndexType& translation,
  typename SliceType::RegionType& iRegion,
  typename SliceType::RegionType& jRegion )
{
  typename SliceType::IndexType iBegin = iRegion.GetIndex();
  typename SliceType::IndexType jBegin = jRegion.GetIndex();
  for ( IdentifierType d = 0; d < SliceType::ImageDimension; d++ )
    {
    IndexValueType iSize = iRegion.GetSize( d );
    IndexValueType jSize = jRegion.GetSize( d );
    iBegin[d] += translation[d];
    IndexValueType t = std::max( iBegin[d], jBegin[d] );
    iRegion.SetSize( d, std::min( IndexValueType( iSize ) - ( t - iBegin[d] ),
        IndexValueType( jSize ) - ( t - jBegin[d] ) ) );
    iRegion.SetIndex( d, t - translation[d] );
    jRegion.SetIndex( d, t );
    }
  jRegion.SetSize( iRegion.GetSize() ); // size is the same
}

template< typename TImage >
IdentifierType
MorphologicalContourInterpolator< TImage >
::Intersection( typename SliceType::Pointer& iConn,
  typename TImage::PixelType iRegionId,
  typename SliceType::Pointer& jConn,
  const PixelList& jRegionIds,
  const typename SliceType::IndexType& translation )
{
  typename SliceType::RegionType iRegion, jRegion;
  iRegion = iConn->GetLargestPossibleRegion();
  jRegion = jConn->GetLargestPossibleRegion();
  IntersectionRegions( translation, iRegion, jRegion );

  std::vector< IdentifierType > counts( jRegionIds.size() );
  for ( unsigned x = 0; x < jRegionIds.size(); x++ )
    {
    counts[x] = 0;
    }
  ImageRegionConstIterator< SliceType > iIt( iConn, iRegion );
  ImageRegionConstIterator< SliceType > jIt( jConn, jRegion );
  while ( !iIt.IsAtEnd() )
    {
    if ( iIt.Get() == iRegionId )
      {
      typename TImage::PixelType jVal = jIt.Get();
      typename PixelList::const_iterator res = std::find( jRegionIds.begin(), jRegionIds.end(), jVal );
      if ( res != jRegionIds.end() )
        {
        ++counts[res - jRegionIds.begin()];
        }
      }
    ++iIt;
    ++jIt;
    }

  IdentifierType sum = 0;
  for ( unsigned x = 0; x < jRegionIds.size(); x++ )
    {
    if ( counts[x] == 0 )
      {
      return 0; // iConn must intersect all subregions of jConn
      }
    sum += counts[x];
    }
  return sum;
} // >::Intersection

template< typename TImage >
IdentifierType
MorphologicalContourInterpolator< TImage >
::CardSymDifference( typename BoolSliceType::Pointer& iShape, typename BoolSliceType::Pointer& jShape )
{
  typename BoolSliceType::RegionType region = iShape->GetLargestPossibleRegion();
  IdentifierType count = 0;
  ImageRegionConstIterator< BoolSliceType > iIt( iShape, region );
  ImageRegionConstIterator< BoolSliceType > jIt( jShape, region );
  while ( !iIt.IsAtEnd() )
    {
    if ( iIt.Get() != jIt.Get() )
      {
      count++;
      }
    ++iIt;
    ++jIt;
    }

  return count;
}

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::SliceType::IndexType
MorphologicalContourInterpolator< TImage >
::Centroid( typename SliceType::Pointer& conn, const PixelList& regionIds )
{
  ImageRegionConstIteratorWithIndex< SliceType > it( conn, conn->GetLargestPossibleRegion() );
  IndexValueType ind[SliceType::ImageDimension] = { 0 }; // all components are initialized to zero
  IdentifierType pixelCount = 0;
  while ( !it.IsAtEnd() )
    {
    typename TImage::PixelType val = it.Get();
    if ( val )
      {
      typename PixelList::const_iterator res = std::find( regionIds.begin(), regionIds.end(), val );
      if ( res != regionIds.end() )
        {
        ++pixelCount;
        typename SliceType::IndexType pInd = it.GetIndex();
        for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
          {
          ind[d] += pInd[d];
          }
        }
      }
    ++it;
    }

  typename SliceType::IndexType retVal;
  for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
    {
    retVal[d] = ind[d] / pixelCount;
    }
  return retVal;
} // >::Centroid

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::SliceType::IndexType
MorphologicalContourInterpolator< TImage >
::Align( typename SliceType::Pointer& iConn,
  typename TImage::PixelType iRegionId,
  typename SliceType::Pointer& jConn,
  const PixelList& jRegionIds )
{
  // calculate centroids
  PixelList iRegionIds;

  iRegionIds.push_back( iRegionId );
  typename SliceType::IndexType iCentroid = Centroid( iConn, iRegionIds );
  typename SliceType::IndexType jCentroid = Centroid( jConn, jRegionIds );

  typename SliceType::IndexType ind;
  for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
    {
    ind[d] = jCentroid[d] - iCentroid[d];
    }

  // construct an image with all possible translations
  typename SliceType::RegionType searchRegion;
  typename SliceType::RegionType iLPR = iConn->GetLargestPossibleRegion();
  typename SliceType::RegionType jLPR = jConn->GetLargestPossibleRegion();
  for ( IdentifierType d = 0; d < SliceType::ImageDimension; d++ )
    {
    searchRegion.SetIndex( d, jLPR.GetIndex()[d] - iLPR.GetIndex()[d] - iLPR.GetSize( d ) + 1 );
    searchRegion.SetSize( d, iLPR.GetSize( d ) + jLPR.GetSize( d ) - 1 );
    }

  typename BoolSliceType::Pointer searched = BoolSliceType::New();
  searched->SetRegions( searchRegion );
  searched->Allocate( true ); // initialize to zero (false)

  // breadth first search starting from centroid, implicitly:
  // when intersection scores are equal, chooses the one closer to centroid
  std::queue< typename SliceType::IndexType > uncomputed;
  typename SliceType::IndexType t0;
  t0.Fill( 0 );
  uncomputed.push( t0 ); // no translation - guaranteed to find a non-zero intersection
  uncomputed.push( ind ); // this introduces movement, and possibly has the same score
  searched->SetPixel( t0, true );
  searched->SetPixel( ind, true );
  IdentifierType score, maxScore = 0;
  typename SliceType::IndexType bestIndex;
  IdentifierType iter = 0;
  IdentifierType minIter = std::min( m_MinAlignIters, searchRegion.GetNumberOfPixels() );
  IdentifierType maxIter = std::max( m_MaxAlignIters, (IdentifierType)sqrt( searchRegion.GetNumberOfPixels() ) );

  while ( !uncomputed.empty() )
    {
    ind = uncomputed.front();
    uncomputed.pop();
    score = Intersection( iConn, iRegionId, jConn, jRegionIds, ind );
    if ( score > maxScore )
      {
      maxScore = score;
      bestIndex = ind;
      }

    // we breadth this search
    if ( !m_HeuristicAlignment || maxScore == 0 || iter <= minIter || (score > maxScore * 0.9 && iter <= maxIter) )
      {
      for ( unsigned d = 0; d < SliceType::ImageDimension; d++ )
        {
        ind[d] -= 1; // "left"
        if ( searchRegion.IsInside( ind ) && !searched->GetPixel( ind ) )
          {
          uncomputed.push( ind );
          searched->SetPixel( ind, true );
          ++iter;
          }
        ind[d] += 2; // "right"
        if ( searchRegion.IsInside( ind ) && !searched->GetPixel( ind ) )
          {
          uncomputed.push( ind );
          searched->SetPixel( ind, true );
          ++iter;
          }
        ind[d] -= 1; // return to initial
        }
      }
    }

  return bestIndex;
} // >::Align

template< typename TImage >
typename MorphologicalContourInterpolator< TImage >::SliceType::Pointer
MorphologicalContourInterpolator< TImage >
::RegionedConnectedComponents( const typename TImage::RegionType& region,
  typename TImage::PixelType label,
  IdentifierType& objectCount )
{
  m_RoI->SetExtractionRegion( region );
  m_RoI->SetInput( this->GetInput() );
  m_Binarizer->SetLowerThreshold( label );
  m_Binarizer->SetUpperThreshold( label );
  m_ConnectedComponents->Update();
  objectCount = m_ConnectedComponents->GetObjectCount();
  return m_ConnectedComponents->GetOutput();
}

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::InterpolateBetweenTwo( int axis,
  TImage* out,
  typename TImage::PixelType label,
  typename TImage::IndexValueType i,
  typename TImage::IndexValueType j,
  typename SliceType::Pointer& iconn,
  typename SliceType::Pointer& jconn,
  ThreadIdType threadId )
{
  // go through comparison image and create correspondence pairs
  typedef std::set< std::pair< typename TImage::PixelType, typename TImage::PixelType > > PairSet;
  PairSet pairs, unwantedPairs, uncleanPairs;
  typename SliceType::RegionType ri = iconn->GetRequestedRegion();
  typename SliceType::RegionType rj = jconn->GetRequestedRegion();
  ImageRegionConstIterator< SliceType > iti( iconn, ri );
  ImageRegionConstIterator< SliceType > itj( jconn, rj );
  while ( !iti.IsAtEnd() )
    {
    if ( iti.Get() != 0 || itj.Get() != 0 )
      {
      uncleanPairs.insert( std::make_pair( iti.Get(), itj.Get() ) );
      // std::cout << " iti:" << iti.GetIndex() << iti.Get() <<
      //  " itj:" << itj.GetIndex() << itj.Get() << std::endl;
      if ( iti.Get() != 0 && itj.Get() != 0 )
        {
        unwantedPairs.insert( std::make_pair( 0, itj.Get() ) );
        unwantedPairs.insert( std::make_pair( iti.Get(), 0 ) );
        }
      }
    ++iti;
    ++itj;
    }

  std::set_difference( uncleanPairs.begin(), uncleanPairs.end(), unwantedPairs.begin(),
    unwantedPairs.end(), std::inserter( pairs, pairs.end() ) );

  // first do extrapolation for components without overlaps
  typename PairSet::iterator p = pairs.begin();
  while ( p != pairs.end() )
    {
    if ( p->second == 0 )
      {
      Extrapolate( axis, out, label, i, j, iconn, p->first, threadId );
      pairs.erase( p++ );
      }
    else if ( p->first == 0 )
      {
      Extrapolate( axis, out, label, j, i, jconn, p->second, threadId );
      pairs.erase( p++ );
      }
    else
      {
      ++p;
      }
    }

  // count ocurrances of each component
  typedef std::map< typename TImage::PixelType, IdentifierType > CountMap;
  CountMap iCounts, jCounts;
  for ( p = pairs.begin(); p != pairs.end(); ++p )
    {
    iCounts[p->first]++;
    jCounts[p->second]++;
    }

  // now handle 1 to 1 correspondences
  p = pairs.begin();
  while ( p != pairs.end() )
    {
    if ( iCounts[p->first] == 1 && jCounts[p->second] == 1 )
      {
      PixelList regionIDs;
      regionIDs.push_back( p->second );
      typename SliceType::IndexType translation = Align( iconn, p->first, jconn, regionIDs );
      Interpolate1to1( axis, out, label, i, j, iconn, p->first, jconn, p->second, translation, false, threadId );
      iCounts.erase( p->first );
      jCounts.erase( p->second );
      pairs.erase( p++ );
      }
    else
      {
      ++p;
      }
    }

  PixelList regionIDs( pairs.size() ); // preallocate
  // now do 1-to-N and M-to-1 cases
  p = pairs.begin();
  while ( p != pairs.end() )
    {
    regionIDs.clear();

    if ( iCounts[p->first] == 1 ) // M-to-1
      {
      for ( typename PairSet::iterator rest = pairs.begin(); rest != pairs.end(); ++rest )
        {
        if ( rest->second == p->second )
          {
          regionIDs.push_back( rest->first );
          }
        }

      typename SliceType::IndexType translation = Align( jconn, p->second, iconn, regionIDs );
      Interpolate1toN( axis, out, label, j, i, jconn, p->second, iconn, regionIDs, translation, threadId );

      typename PairSet::iterator rest = pairs.begin();
      while ( rest != pairs.end() )
        {
        if ( rest != p && rest->second == p->second )
          {
          --iCounts[rest->first];
          --jCounts[rest->second];
          pairs.erase( rest++ );
          }
        else
          {
          ++rest;
          }
        }

      --iCounts[p->first];
      --jCounts[p->second];
      pairs.erase( p++ );
      } // M-to-1
    else if ( jCounts[p->second] == 1 ) // 1-to-N
      {
      for ( typename PairSet::iterator rest = pairs.begin(); rest != pairs.end(); ++rest )
        {
        if ( rest->first == p->first )
          {
          regionIDs.push_back( rest->second );
          }
        }

      typename SliceType::IndexType translation = Align( iconn, p->first, jconn, regionIDs );
      Interpolate1toN( axis, out, label, i, j, iconn, p->first, jconn, regionIDs, translation, threadId );

      typename PairSet::iterator rest = pairs.begin();
      ++rest;
      while ( rest != pairs.end() )
        {
        if ( rest != p && rest->first == p->first )
          {
          --iCounts[rest->first];
          --jCounts[rest->second];
          pairs.erase( rest++ );
          }
        else
          {
          ++rest;
          }
        }

      --iCounts[p->first];
      --jCounts[p->second];
      pairs.erase( p++ );
      } // 1-to-N
    else
      {
      ++p;
      }
    } // 1-to-N and M-to-1

  // only M-to-N correspondences remain
  // we turn each M-to-N case into m 1-to-N cases
  p = pairs.begin();
  while ( p != pairs.end() )
    {
    regionIDs.clear();
    for ( typename PairSet::iterator rest = p; rest != pairs.end(); ++rest )
      {
      if ( rest->first == p->first )
        {
        regionIDs.push_back( rest->second );
        }
      }

    typename SliceType::IndexType translation = Align( iconn, p->first, jconn, regionIDs );
    Interpolate1toN( axis, out, label, i, j, iconn, p->first, jconn, regionIDs, translation, threadId );

    typename PairSet::iterator rest = p;
    ++rest;
    while ( rest != pairs.end() )
      {
      if ( rest->first == p->first )
        {
        pairs.erase( rest++ );
        }
      else
        {
        ++rest;
        }
      }

    // counts no longer matter, do not waste time deleting them
    pairs.erase( p++ );
    } // M-to-N
} // void MorphologicalContourInterpolator::InterpolateBetweenTwo()

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::InterpolateAlong( int axis, TImage* out )
{
  // a list of segments which need to be interpolated
  std::vector< SegmentBetweenTwo< TImage > > segments;
  typename TImage::RegionType reqRegion = this->GetOutput()->GetRequestedRegion();
  for ( typename LabeledSlicesType::iterator it = m_LabeledSlices[axis].begin();
        it != m_LabeledSlices[axis].end();
        ++it )
    {
    if ( m_Label == 0 || m_Label == it->first ) // label needs to be interpolated
      {
      typename SliceSetType::iterator prev = it->second.begin();
      if ( prev == it->second.end() )
        {
        continue; // nothing to do for this label
        }

      typename TImage::RegionType ri = reqRegion;
      if ( !m_UseCustomSlicePositions )
        {
        typename BoundingBoxesType::iterator iBB = m_BoundingBoxes.find( it->first );
        if ( iBB == m_BoundingBoxes.end() )
          {
          continue; // this label not present in requested region
          }
        else
          {
          ri = iBB->second;
          }
        }
      ri.SetSize( axis, 0 );
      ri.SetIndex( axis, *prev );
      IdentifierType xCount;
      typename SliceType::Pointer iconn = this->RegionedConnectedComponents( ri, it->first, xCount );
      iconn->DisconnectPipeline();
      int iReq = *prev < reqRegion.GetIndex( axis ) ? -1 :
        ( *prev > reqRegion.GetIndex( axis ) + IndexValueType( reqRegion.GetSize( axis ) ) ? +1 : 0 );

      typename SliceSetType::iterator next = it->second.begin();
      for ( ++next; next != it->second.end(); ++next )
        {
        typename TImage::RegionType rj = ri;
        rj.SetIndex( axis, *next );
        typename SliceType::Pointer jconn = this->RegionedConnectedComponents( rj, it->first, xCount );
        jconn->DisconnectPipeline();
        int jReq = *next < reqRegion.GetIndex( axis ) ? -1 :
          ( *next > reqRegion.GetIndex( axis ) + IndexValueType( reqRegion.GetSize( axis ) ) ? +1 : 0 );

        if ( *prev + 1 < *next // only if they are not adjacent slices
             && std::abs(iReq + jReq) <= 1 ) // and not out of the requested region
        // unless they are on opposite ends
          {
          SegmentBetweenTwo< TImage > s;
          s.axis = axis;
          s.out = out;
          s.label = it->first;
          s.i = *prev;
          s.j = *next;
          s.iconn = iconn;
          s.jconn = jconn;
          segments.push_back( s );
          }
        iconn = jconn;
        iReq = jReq;
        prev = next;
        }
      }
    }

  typedef MorphologicalContourInterpolatorParallelInvoker< TImage > Parallelizer;
  typename Parallelizer::Pointer parallelizer = Parallelizer::New();
  parallelizer->SetWorkArray( segments );
  typename Parallelizer::DomainType completeDomain;
  completeDomain[0] = 0;
  completeDomain[1] = std::max( 0, int( segments.size() ) - 1 );
  parallelizer->Execute( this, completeDomain );
  parallelizer->ClearWorkArray();
} // >::InterpolateAlong

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::AllocateOutputs()
{
  typedef ImageBase< TImage::ImageDimension > ImageBaseType;
  typename ImageBaseType::Pointer outputPtr;

  for ( OutputDataObjectIterator it( this ); !it.IsAtEnd(); it++ )
    {
    outputPtr = dynamic_cast< ImageBaseType * >( it.GetOutput() );
    if ( outputPtr )
      {
      outputPtr->SetBufferedRegion( outputPtr->GetRequestedRegion() );
      outputPtr->Allocate( true );
      }
    }
}

template< typename TImage >
void
MorphologicalContourInterpolator< TImage >
::GenerateData()
{
  typename TImage::ConstPointer m_Input = this->GetInput();
  typename TImage::Pointer m_Output = this->GetOutput();
  this->AllocateOutputs();

  if ( m_UseCustomSlicePositions )
    {
    SliceIndicesType t = m_LabeledSlices;
    this->DetermineSliceOrientations(); // calculates bounding boxes
    m_LabeledSlices = t; // restore custom positions
    }
  else
    {
    this->DetermineSliceOrientations();
    }

  if ( m_BoundingBoxes.size() == 0 && !m_UseCustomSlicePositions )
    {
    ImageAlgorithm::Copy< TImage, TImage >( m_Input.GetPointer(), m_Output.GetPointer(),
      m_Output->GetRequestedRegion(), m_Output->GetRequestedRegion() );
    return; // no contours detected
    }

  if ( m_Axis == -1 ) // interpolate along all axes
    {
    FixedArray< bool, TImage::ImageDimension > aggregate;
    aggregate.Fill( false );
    for ( unsigned i = 0; i < TImage::ImageDimension; i++ )
      {
      if ( this->m_Label == 0 ) // examine all labels
        {
        for ( unsigned l = 0; l < m_LabeledSlices[i].size(); l++ )
          {
          if ( m_LabeledSlices[i][l].size() > 1 )
            {
            aggregate[i] = true;
            }
          }
        }
      else // we only care about this label
        {
        if ( m_LabeledSlices[i][m_Label].size() > 1 )
          {
          aggregate[i] = true;
          }
        }
      }

    for ( unsigned int a = 0; a < TImage::ImageDimension; ++a )
      {
      if ( aggregate[a] )
        {
        this->InterpolateAlong( a, m_Output );
        }
      }
    } // interpolate along all axes
  else // interpolate along the specified axis
    {
    this->InterpolateAlong( m_Axis, m_Output );
    }

  // Overwrites m_Output with non non-zeroes from m_Input
  ImageRegionIterator< TImage > itO( this->GetOutput(), this->GetOutput()->GetBufferedRegion() );
  ImageRegionConstIterator< TImage > itI( this->GetInput(), this->GetOutput()->GetBufferedRegion() );
  while ( !itI.IsAtEnd() )
    {
    typename TImage::PixelType val = itI.Get();
    if ( val != 0 )
      {
      itO.Set( val );
      }

    ++itI;
    ++itO;
    }
} // >::GenerateData
} // end namespace itk

#endif // itkMorphologicalContourInterpolator_hxx
