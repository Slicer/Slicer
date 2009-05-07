#ifndef __itkBSplineControlPointImageFilter_txx
#define __itkBSplineControlPointImageFilter_txx

#include "itkBSplineControlPointImageFilter.h"

#include "itkImageDuplicator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"

namespace itk
{

template <class InputImage, class TOutputImage>
BSplineControlPointImageFilter<InputImage, TOutputImage>
::BSplineControlPointImageFilter()
{
  this->m_SplineOrder.Fill( 3 );
  for ( unsigned int i = 0; i < ImageDimension; i++ )
    {
    this->m_NumberOfControlPoints[i] = ( this->m_SplineOrder[i]+1 );
    this->m_Kernel[i] = KernelType::New();
    this->m_Kernel[i]->SetSplineOrder( this->m_SplineOrder[i] );
    }
  this->m_KernelOrder0 = KernelOrder0Type::New();
  this->m_KernelOrder1 = KernelOrder1Type::New();
  this->m_KernelOrder2 = KernelOrder2Type::New();
  this->m_KernelOrder3 = KernelOrder3Type::New();

  this->m_NumberOfLevels.Fill( 1 );
  this->m_CloseDimension.Fill( 0 );

  this->m_BSplineEpsilon = vcl_numeric_limits<RealType>::epsilon();
}

template <class InputImage, class TOutputImage>
BSplineControlPointImageFilter<InputImage, TOutputImage>
::~BSplineControlPointImageFilter()
{
}

template <class TInputPointImage, class TOutputImage>
void
BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
::SetNumberOfLevels( ArrayType levels )
{
  this->m_NumberOfLevels = levels;
  this->m_MaximumNumberOfLevels = 1;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( this->m_NumberOfLevels[i] == 0 )
      {
      itkExceptionMacro(
        "The number of levels in each dimension must be greater than 0" );
      }
    if( this->m_NumberOfLevels[i] > this->m_MaximumNumberOfLevels )
      {
      this->m_MaximumNumberOfLevels = this->m_NumberOfLevels[i];
      }
    }

  itkDebugMacro( "Setting m_NumberOfLevels to " <<
                 this->m_NumberOfLevels );
  itkDebugMacro( "Setting m_MaximumNumberOfLevels to " <<
                 this->m_MaximumNumberOfLevels );

  if( this->m_MaximumNumberOfLevels > 1 )
    {
    this->m_DoMultilevel = true;
    }
  else
    {
    this->m_DoMultilevel = false;
    }
  this->SetSplineOrder( this->m_SplineOrder );
  this->Modified();
}

template <class TInputPointImage, class TOutputImage>
void
BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
::SetSplineOrder( unsigned int order )
{
  this->m_SplineOrder.Fill( order );
  this->SetSplineOrder( this->m_SplineOrder );
}

template <class TInputPointImage, class TOutputImage>
void
BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
::SetSplineOrder( ArrayType order )
{
  itkDebugMacro( "Setting m_SplineOrder to " << order );

  this->m_SplineOrder = order;
  for( int i = 0; i < ImageDimension; i++ )
    {
    if( this->m_SplineOrder[i] == 0 )
      {
      itkExceptionMacro(
        "The spline order in each dimension must be greater than 0" );
      }

    this->m_Kernel[i] = KernelType::New();
    this->m_Kernel[i]->SetSplineOrder( this->m_SplineOrder[i] );

    if( this->m_DoMultilevel )
      {
      typename KernelType::MatrixType C;
      C = this->m_Kernel[i]->GetShapeFunctionsInZeroToOneInterval();

      vnl_matrix<RealType> R;
      vnl_matrix<RealType> S;
      R.set_size( C.rows(), C.cols() );
      S.set_size( C.rows(), C.cols() );
      for( unsigned int j = 0; j < C.rows(); j++ )
        {
        for( unsigned int k = 0; k < C.cols(); k++ )
          {
          R(j, k) = S(j, k) = static_cast<RealType>( C(j, k) );
          }
        }
      for( unsigned int j = 0; j < C.cols(); j++ )
        {
        RealType c = pow( 2.0, static_cast<RealType>( C.cols()-j-1 ) );
        for( unsigned int k = 0; k < C.rows(); k++)
          {
          R(k, j) *= c;
          }
        }
      R = R.transpose();
      R.flipud();
      S = S.transpose();
      S.flipud();

      this->m_RefinedLatticeCoefficients[i]
          = ( vnl_svd<RealType>( R ).solve( S ) ).extract( 2, S.cols() );
      }
    }
  this->Modified();
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::GenerateData()
{
  for( unsigned int i = 0; i < ImageDimension; i++)
    {
    if( this->m_Size[i] == 0 )
      {
      itkExceptionMacro( "Size must be specified." );
      }
    }
  this->GetOutput()->SetOrigin( this->m_Origin );
  this->GetOutput()->SetSpacing( this->m_Spacing );
  this->GetOutput()->SetRegions( this->m_Size );
  this->GetOutput()->SetDirection( this->m_Direction );
  this->GetOutput()->Allocate();

  /**
   * Calculate the appropriate epsilon value.
   */
  unsigned int maximumNumberOfSpans = 0;
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    unsigned int numberOfSpans = this->m_NumberOfControlPoints[d]
      - this->m_SplineOrder[d];
    numberOfSpans <<= ( this->m_NumberOfLevels[d] - 1 );
    if( numberOfSpans > maximumNumberOfSpans )
      {
      maximumNumberOfSpans = numberOfSpans;
      }
    }
  this->m_BSplineEpsilon = 10.0 * vcl_numeric_limits<RealType>::epsilon();
  while( static_cast<RealType>( maximumNumberOfSpans ) ==
    static_cast<RealType>( maximumNumberOfSpans ) - this->m_BSplineEpsilon )
    {
    this->m_BSplineEpsilon *= 10.0;
    }
  this->m_BSplineEpsilon = 0.0001;

  for( unsigned int i = 0; i < ImageDimension; i++)
    {
    this->m_NumberOfControlPoints[i]
      = this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
    }

  this->GenerateOutputImageFast();
}

template <class TInputPointImage, class TOutputImage>
typename BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
  ::ControlPointLatticeType::Pointer
BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
::RefineControlLattice( ArrayType NumberOfLevels )
{
  this->SetNumberOfLevels( NumberOfLevels );

  typedef ImageDuplicator<ControlPointLatticeType> ImageDuplicatorType;
  typename ImageDuplicatorType::Pointer Duplicator
    = ImageDuplicatorType::New();
  Duplicator->SetInputImage( this->GetInput() );
  Duplicator->Update();

  typename ControlPointLatticeType::Pointer psiLattice
    = ControlPointLatticeType::New();
  psiLattice = Duplicator->GetOutput();

  for( unsigned int m = 1; m < this->m_MaximumNumberOfLevels; m++ )
    {
    ArrayType numberOfNewControlPoints;
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      numberOfNewControlPoints[i]
        = psiLattice->GetLargestPossibleRegion().GetSize()[i];
      }
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      if( m < this->m_NumberOfLevels[i] )
        {
        numberOfNewControlPoints[i]
          = 2*numberOfNewControlPoints[i]-this->m_SplineOrder[i];
        }
      }
    typename RealImageType::RegionType::SizeType size;
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      if( this->m_CloseDimension[i] )
        {
        size[i] = numberOfNewControlPoints[i] - this->m_SplineOrder[i];
        }
      else
        {
        size[i] = numberOfNewControlPoints[i];
        }
      }

    typename ControlPointLatticeType::Pointer refinedLattice
      = ControlPointLatticeType::New();
    refinedLattice->SetRegions( size );
    refinedLattice->Allocate();
    PixelType data;
    data.Fill( 0.0 );
    refinedLattice->FillBuffer( data );

    typename ControlPointLatticeType::IndexType idx;
    typename ControlPointLatticeType::IndexType idx_Psi;
    typename ControlPointLatticeType::IndexType tmp;
    typename ControlPointLatticeType::IndexType tmp_Psi;
    typename ControlPointLatticeType::IndexType off;
    typename ControlPointLatticeType::IndexType off_Psi;
    typename ControlPointLatticeType::RegionType::SizeType size_Psi;

    size.Fill( 2 );
    unsigned int N = 1;
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      N *= ( this->m_SplineOrder[i] + 1 );
      size_Psi[i] = this->m_SplineOrder[i] + 1;
      }

    ImageRegionIteratorWithIndex<ControlPointLatticeType>
      It( refinedLattice, refinedLattice->GetLargestPossibleRegion() );

    It.GoToBegin();
    while( !It.IsAtEnd() )
      {
      idx = It.GetIndex();
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        if( m < this->m_NumberOfLevels[i] )
          {
          idx_Psi[i] = static_cast<unsigned int>( 0.5*idx[i] );
          }
        else
          {
          idx_Psi[i] = static_cast<unsigned int>( idx[i] );
          }
        }
      for( unsigned int i = 0; i < ( 2 << ImageDimension - 1 ); i++ )
        {
        PixelType sum( 0.0 );
        PixelType val;
        off = this->NumberToIndex( i, size );

        bool outOfBoundary = false;
        for( unsigned int j = 0; j < ImageDimension; j++ )
          {
          tmp[j] = idx[j] + off[j];
          if( tmp[j] >= static_cast<int>( numberOfNewControlPoints[j] )
                 && !this->m_CloseDimension[j] )
            {
            outOfBoundary = true;
            break;
            }
          if( this->m_CloseDimension[j] )
            {
            tmp[j] %= refinedLattice->GetLargestPossibleRegion().GetSize()[j];
            }
          }
        if( outOfBoundary )
          {
          continue;
          }

        for( unsigned int j = 0; j < N; j++ )
          {
          off_Psi = this->NumberToIndex( j, size_Psi );

          bool outOfBoundary = false;
          for( unsigned int k = 0; k < ImageDimension; k++ )
            {
            tmp_Psi[k] = idx_Psi[k] + off_Psi[k];
            if( tmp_Psi[k] >= static_cast<int>(
                  this->GetInput()->GetLargestPossibleRegion().GetSize()[k] )
                  && !this->m_CloseDimension[k] )
              {
              outOfBoundary = true;
              break;
              }
            if( this->m_CloseDimension[k] )
              {
             tmp_Psi[k] %= psiLattice->GetLargestPossibleRegion().GetSize()[k];
              }
            }
            if( outOfBoundary )
               {
               continue;
               }
            RealType coeff = 1.0;
            for( unsigned int k = 0; k < ImageDimension; k++ )
              {
              coeff *=
                this->m_RefinedLatticeCoefficients[k]( off[k], off_Psi[k] );
              }
            val = psiLattice->GetPixel( tmp_Psi );
            val *= coeff;
            sum += val;
            }
          refinedLattice->SetPixel( tmp, sum );
          }

      bool IsEvenIndex = false;
      while( !IsEvenIndex && !It.IsAtEnd() )
        {
        ++It;
        idx = It.GetIndex();
        IsEvenIndex = true;
        for( unsigned int i = 0; i < ImageDimension; i++ )
          {
          if( idx[i] % 2 )
            {
            IsEvenIndex = false;
            }
          }
        }
      }

    typedef ImageDuplicator<ControlPointLatticeType> ImageDuplicatorType;
    typename ImageDuplicatorType::Pointer Duplicator
      = ImageDuplicatorType::New();
    Duplicator->SetInputImage( refinedLattice );
    Duplicator->Update();
    psiLattice = Duplicator->GetOutput();
    }
  return psiLattice;
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::GenerateOutputImageFast()
{
  typename ControlPointLatticeType::Pointer
    collapsedPhiLattices[ImageDimension+1];
  for( int i = 0; i < ImageDimension; i++ )
    {
    collapsedPhiLattices[i] = ControlPointLatticeType::New();
    collapsedPhiLattices[i]->SetOrigin( this->GetInput()->GetOrigin() );
    collapsedPhiLattices[i]->SetSpacing( this->GetInput()->GetSpacing() );
    typename ControlPointLatticeType::SizeType size;
    size.Fill( 1 );
    for( int j = 0; j < i; j++ )
      {
      size[j] = this->GetInput()->GetLargestPossibleRegion().GetSize()[j];
      }
    collapsedPhiLattices[i]->SetRegions( size );
    collapsedPhiLattices[i]->Allocate();
    }
  typedef ImageDuplicator<ControlPointLatticeType> DuplicatorType;
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage( this->GetInput() );
  duplicator->Update();

  collapsedPhiLattices[ImageDimension] = duplicator->GetOutput();
  ArrayType totalNumberOfSpans;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( this->m_CloseDimension[i] )
      {
      totalNumberOfSpans[i]
        = this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
      }
    else
      {
      totalNumberOfSpans[i]
        = this->GetInput()->GetLargestPossibleRegion().GetSize()[i]
          - this->m_SplineOrder[i];
      }
    }
  FixedArray<RealType, ImageDimension> U;
  FixedArray<RealType, ImageDimension> currentU;
  currentU.Fill( -1 );
  typename ImageType::IndexType startIndex
    = this->GetOutput()->GetRequestedRegion().GetIndex();
  typename ControlPointLatticeType::IndexType startPhiIndex
    = this->GetInput()->GetLargestPossibleRegion().GetIndex();

  ImageRegionIteratorWithIndex<ImageType>
     It( this->GetOutput(), this->GetOutput()->GetRequestedRegion() );
  for( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    typename ImageType::IndexType idx = It.GetIndex();
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      U[i] = static_cast<RealType>( totalNumberOfSpans[i] ) *
             static_cast<RealType>( idx[i] - startIndex[i] ) /
             static_cast<RealType>( this->m_Size[i] - 1 );
      if( vnl_math_abs( U[i] - static_cast<RealType>( totalNumberOfSpans[i] ) )
            <= this->m_BSplineEpsilon )
        {
        U[i] = static_cast<RealType>( totalNumberOfSpans[i] )
          - this->m_BSplineEpsilon;
        }
      if( U[i] >= static_cast<RealType>( totalNumberOfSpans[i] ) )
        {
        itkExceptionMacro( "The collapse point component " << U[i]
          << " is outside the corresponding parametric domain of [0, "
          << totalNumberOfSpans[i] << "]." );
        }
      }
    for( int i = ImageDimension-1; i >= 0; i-- )
      {
      if( U[i] != currentU[i] )
        {
        for( int j = i; j >= 0; j-- )
          {
          this->CollapsePhiLattice(
            collapsedPhiLattices[j+1], collapsedPhiLattices[j], U[j], j );
          currentU[j] = U[j];
          }
        break;
        }
      }
    It.Set( collapsedPhiLattices[0]->GetPixel( startPhiIndex ) );
    }
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::CollapsePhiLattice( ControlPointLatticeType *lattice,
                      ControlPointLatticeType *collapsedLattice,
                      RealType u, unsigned int dimension )
{
  ImageRegionIteratorWithIndex<ControlPointLatticeType> It
    ( collapsedLattice, collapsedLattice->GetLargestPossibleRegion() );

  for( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    PointDataType data;
    data.Fill( 0.0 );
    typename ControlPointLatticeType::IndexType idx = It.GetIndex();
    for( unsigned int i = 0; i < this->m_SplineOrder[dimension] + 1; i++ )
      {
      idx[dimension] = static_cast<unsigned int>( u ) + i;
      RealType v = u - idx[dimension]
        + 0.5*static_cast<RealType>( this->m_SplineOrder[dimension] - 1 );
      RealType B = 0.0;
      switch( this->m_SplineOrder[dimension] )
        {
        case 0:
          {
          B = this->m_KernelOrder0->Evaluate( v );
          break;
          }
        case 1:
          {
          B = this->m_KernelOrder1->Evaluate( v );
          break;
          }
        case 2:
          {
          B = this->m_KernelOrder2->Evaluate( v );
          break;
          }
        case 3:
          {
          B = this->m_KernelOrder3->Evaluate( v );
          break;
          }
        default:
          {
          B = this->m_Kernel[dimension]->Evaluate( v );
          break;
          }
        }
      if( this->m_CloseDimension[dimension] )
        {
        idx[dimension] %=
          lattice->GetLargestPossibleRegion().GetSize()[dimension];
        }
      data += ( lattice->GetPixel( idx ) * B );
      }
    It.Set( data );
    }
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateAtPoint( PointType point, PixelType &data )
{
  for( unsigned int i = 0; i < ImageDimension; i++)
    {
    point[i] -= this->m_Origin[i];
    point[i] /= ( static_cast<RealType>( this->m_Size[i]-1 ) * this->m_Spacing[i] );
    }
  this->Evaluate( point, data );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateAtIndex( IndexType idx, PixelType &data )
{
  PointType point;
  this->GetOutput()->TransformIndexToPhysicalPoint( idx, point );
  this->EvaluateAtPoint( point, data );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateAtContinuousIndex( ContinuousIndexType idx, PixelType &data )
{
  PointType point;
  this->GetOutput()->TransformContinuousIndexToPhysicalPoint( idx, point );
  this->EvaluateAtPoint( point, data );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::Evaluate( PointType params, PixelType &data )
{
  vnl_vector<RealType> p( ImageDimension );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( params[i] == NumericTraits<RealType>::One )
      {
      params[i] = NumericTraits<RealType>::One - this->m_BSplineEpsilon;
      }
    if( params[i] < 0.0 || params[i] >= 1.0 )
      {
      itkExceptionMacro( "The specified point " << params
        << " is outside the reparameterized domain [0, 1]." );
      }
    p[i] = static_cast<RealType>( params[i] )
         * static_cast<RealType>(
         this->GetInput()->GetLargestPossibleRegion().GetSize()[i]
         - this->m_SplineOrder[i] );
    }

  typename RealImageType::RegionType::SizeType size;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    size[i] = this->m_SplineOrder[i] + 1;
    }
  typename RealImageType::Pointer w;
  w = RealImageType::New();
  w->SetRegions( size );
  w->Allocate();

  PointDataType val;
  data.Fill( 0.0 );

  ImageRegionIteratorWithIndex<RealImageType>
    Itw( w, w->GetLargestPossibleRegion() );

  for( Itw.GoToBegin(); !Itw.IsAtEnd(); ++Itw )
    {
    RealType B = 1.0;
    typename RealImageType::IndexType idx = Itw.GetIndex();
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      RealType u = p[i] - static_cast<RealType>( static_cast<unsigned>( p[i] )
        + idx[i] ) + 0.5*static_cast<RealType>( this->m_SplineOrder[i] - 1 );
      switch( this->m_SplineOrder[i] )
        {
        case 0:
          {
          B *= this->m_KernelOrder0->Evaluate( u );
          break;
          }
        case 1:
          {
          B *= this->m_KernelOrder1->Evaluate( u );
          break;
          }
        case 2:
          {
          B *= this->m_KernelOrder2->Evaluate( u );
          break;
          }
        case 3:
          {
          B *= this->m_KernelOrder3->Evaluate( u );
          break;
          }
        default:
          {
          B *= this->m_Kernel[i]->Evaluate( u );
          break;
          }
        }
      }
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      idx[i] += static_cast<unsigned int>( p[i] );
      if( this->m_CloseDimension[i] )
        {
        idx[i] %= this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
        }
      }
    if( this->GetInput()->GetLargestPossibleRegion().IsInside( idx ) )
      {
      val = this->GetInput()->GetPixel( idx );
      val *= B;
      data += val;
      }
    }
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradientAtPoint( PointType point, GradientType &gradient )
{
  for( unsigned int i = 0; i < ImageDimension; i++)
    {
    point[i] -= this->m_Origin[i];
    point[i] /=
      ( static_cast<RealType>( this->m_Size[i] - 1 ) * this->m_Spacing[i] );
    }
  this->EvaluateGradient( point, gradient );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradientAtIndex( IndexType idx, GradientType &gradient )
{
  PointType point;
  this->GetOutput()->TransformIndexToPhysicalPoint( idx, point );
  this->EvaluateGradientAtPoint( point, gradient );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradientAtContinuousIndex(
  ContinuousIndexType idx, GradientType &gradient )
{
  PointType point;
  this->GetOutput()->TransformContinuousIndexToPhysicalPoint( idx, gradient );
  this->EvaluateGradientAtPoint( point, gradient );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradient( PointType params, GradientType &gradient )
{
  vnl_vector<RealType> p( ImageDimension );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( params[i] < 0.0 || params[i] > 1.0 )
      {
      itkExceptionMacro( "The specified point " << params
        << " is outside the reparameterized image domain [0, 1)." );
      }
    if( params[i] == 1.0 )
      {
      params[i] -= vnl_math::float_eps;
      }
    p[i] = static_cast<RealType>( params[i] ) * static_cast<RealType>(
      this->GetInput()->GetLargestPossibleRegion().GetSize()[i]
      - this->m_SplineOrder[i] );
    }

  typename RealImageType::RegionType::SizeType size;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    size[i] = this->m_SplineOrder[i] + 1;
    }
  typename RealImageType::Pointer w;
  w = RealImageType::New();
  w->SetRegions( size );
  w->Allocate();

  PixelType val;
  gradient.SetSize( val.Size(), ImageDimension );
  gradient.Fill( 0.0 );

  ImageRegionIteratorWithIndex<RealImageType>
     Itw( w, w->GetLargestPossibleRegion() );

  for( unsigned int j = 0; j < gradient.Cols(); j++ )
    {
    for( Itw.GoToBegin(); !Itw.IsAtEnd(); ++Itw )
      {
      RealType B = 1.0;
      typename RealImageType::IndexType idx = Itw.GetIndex();
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        RealType u = p[i] - static_cast<RealType>( static_cast<unsigned>( p[i] )
          + idx[i] ) + 0.5*static_cast<RealType>( this->m_SplineOrder[i] - 1 );
        if( j == i )
          {
          B *= this->m_Kernel[i]->EvaluateDerivative( u );
          }
        else
          {
          B *= this->m_Kernel[i]->Evaluate( u );
          }
        }
      for( unsigned int i = 0; i < ImageDimension; i++ )
        {
        idx[i] += static_cast<unsigned int>( p[i] );
        if( this->m_CloseDimension[i] )
          {
          idx[i] %= this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
          }
        }
      if( this->GetInput()->GetLargestPossibleRegion().IsInside( idx ) )
        {
        val = this->GetInput()->GetPixel( idx );
        val *= B;
        for( unsigned int i = 0; i < val.Size(); i++ )
          {
          gradient( i, j ) += val[i];
          }
        }
      }
    }
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateHessian( PointType params, GradientType &hessian,
  unsigned int component = 0 )
{
  vnl_vector<RealType> p( ImageDimension );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( params[i] < 0.0 || params[i] > 1.0 )
      {
      itkExceptionMacro( "The specified point " << params
        << " is outside the reparameterized image domain [0, 1)." );
      }
    if( params[i] == 1.0 )
      {
      params[i] = 1.0 - vnl_math::float_eps;
      }
    p[i] = static_cast<RealType>( params[i] ) * static_cast<RealType>(
      this->GetInput()->GetLargestPossibleRegion().GetSize()[i]
      - this->m_SplineOrder[i] );
    }

  typename RealImageType::RegionType::SizeType size;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    size[i] = this->m_SplineOrder[i] + 1;
    }
  typename RealImageType::Pointer w;
  w = RealImageType::New();
  w->SetRegions( size );
  w->Allocate();

  RealType val;
  hessian.SetSize( ImageDimension, ImageDimension );
  hessian.Fill( 0.0 );

  ImageRegionIteratorWithIndex<RealImageType>
     Itw( w, w->GetLargestPossibleRegion() );

  for( unsigned int j = 0; j < hessian.Rows(); j++ )
    {
    for( unsigned int k = j; k < hessian.Cols(); k++ )
      {
      for( Itw.GoToBegin(); !Itw.IsAtEnd(); ++Itw )
        {
        RealType B = 1.0;
        typename RealImageType::IndexType idx = Itw.GetIndex();
        for( unsigned int i = 0; i < ImageDimension; i++ )
          {
          RealType u = p[i] - static_cast<RealType>(
            static_cast<unsigned>( p[i] ) + idx[i] )
            + 0.5*static_cast<RealType>( this->m_SplineOrder[i] - 1 );
          if( i == j && j == k )
            {
            B *= this->m_Kernel[i]->EvaluateNthDerivative( u, 2 );
            }
          else if( ( i == j || i == k ) && j != k )
            {
            B *= this->m_Kernel[i]->EvaluateDerivative( u );
            }
          else
            {
            B *= this->m_Kernel[i]->Evaluate( u );
            }
          }
        for( unsigned int i = 0; i < ImageDimension; i++ )
          {
          idx[i] += static_cast<unsigned int>( p[i] );
          if( this->m_CloseDimension[i] )
            {
            idx[i] %= this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
            }
          }
        if( this->GetInput()->GetLargestPossibleRegion().IsInside( idx ) )
          {
          val = this->GetInput()->GetPixel( idx )[component];
          val *= B;
          hessian( j, k ) += val;
          hessian( k, j ) += val;
          }
        }
      }
    }
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateParametersAtPoint( PointDataType point, PointType &params )
{
//  std::cout << "Evaluate Parameters At Point" << std::endl;
//  std::cout << " Point: " << point << std::endl;
//  std::cout << " Initial guess " << params << std::endl;
  RealType gg;
  RealType gam;
  RealType fp;
  RealType dgg;
  RealType fret;
  RealType step;

  const RealType ftol = 1.0e-10;
  const RealType eps = 1.0e-20;

  PointDataType grad = this->EvaluateGradientForCGD( params, point );
  fp = this->EvaluateMetricForCGD( params, point );

  PointDataType g = -grad;
  PointDataType gh = g;
  grad = g;

  for( unsigned int its = 1; its <= 100; its++ )
    {
//    std::cout << "   Iteration " << its << std::endl;
    this->LineMinimizationForCGD( &step, &fret, params, grad, point );
    params += ( step * grad );

    if( 2.0 * vnl_math_abs( fret - fp ) <= ftol * ( vnl_math_abs( fret )
           + vnl_math_abs( fp ) + eps ) )
      {
      return;
      }

    fp = fret;
    grad = this->EvaluateGradientForCGD( params, point );

    dgg = gg = 0.0;

    gg = g * g;
    //dgg = grad * grad;
    dgg = ( grad + g ) * grad;

    if( gg == 0.0 )
      {
      return;
      }

    gam = vnl_math_max( static_cast<RealType>( 0.0 ),
      static_cast<RealType>( dgg / gg ) );

    g = -grad;
    gh = g + gam * gh;
    grad = gh;
    }
//  std::cout << "      Number of iterations exceed maximum number of iterations (" << 100
//      << ")." << std::endl;
}

template<class TInputImage, class TOutputImage>
typename BSplineControlPointImageFilter<TInputImage, TOutputImage>::RealType
BSplineControlPointImageFilter<TInputImage, TOutputImage>
::EvaluateMetricForCGD( PointType point, PointDataType data )
{
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( point[i] < 0.0 || point[i] > 1.0 )
      {
      return NumericTraits<RealType>::max();
      }
    }

  PointDataType value;
  this->Evaluate( point, value );
  PointDataType metric;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    metric[i] = value[i] + ( point[i] * this->m_Spacing[i]
      * ( this->m_Size[i] - 1 ) + this->m_Origin[i] ) - data[i] ;
    }

  return metric.GetSquaredNorm();
}

template<class TInputImage, class TOutputImage>
typename BSplineControlPointImageFilter<TInputImage, TOutputImage>::PointDataType
BSplineControlPointImageFilter<TInputImage, TOutputImage>
::EvaluateGradientForCGD( PointType point, PointDataType data )
{
  PointDataType value;
  this->Evaluate( point, value );
  PointDataType metric;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    metric[i] = value[i] + ( point[i] * this->m_Spacing[i]
      * ( this->m_Size[i] - 1 ) + this->m_Origin[i] ) - data[i];
    }

  GradientType grad;
  this->EvaluateGradientAtPoint( point, grad );
  PointDataType metricGrad;
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    metricGrad[i] = 0.0;
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      if( i == j )
        {
        metricGrad[i] += 2 * metric[i] * ( grad(i, i)
          + this->m_Spacing[i]*( this->m_Size[i] - 1 ) );
        }
      else
        {
        metricGrad[i] += 2 * metric[j] * grad(j, i);
        }
      }
    }
  return metricGrad;
}

template<class TInputImage, class TOutputImage>
typename BSplineControlPointImageFilter<TInputImage, TOutputImage>::RealType
BSplineControlPointImageFilter<TInputImage, TOutputImage>
::EvaluateEnergyForLineSearch( RealType lambda, PointType point,
  PointDataType grad, PointDataType data )
{
  return this->EvaluateMetricForCGD( point + lambda * grad, data );
}

template<class TInputImage, class TOutputImage>
void
BSplineControlPointImageFilter<TInputImage, TOutputImage>
::LineMinimizationForCGD( RealType *step, RealType *fret, PointType point,
  PointDataType grad, PointDataType data )
{
//  // std::cout << "    Begin line search..." << std::endl;

  // We should now have a, b and c, as well as f(a), f(b), f(c),
  // where b gives the minimum energy position;
  RealType ax, bx, cx;
  this->FindBracketingTriplet( &ax, &bx, &cx, point, grad, data );

  this->BrentSearch( ax, bx, cx, step, fret, point, grad, data );
//  this->GoldenSectionSearch( ax, bx, cx, step, fret );
}

template<class TInputImage, class TOutputImage>
void
BSplineControlPointImageFilter<TInputImage, TOutputImage>
::FindBracketingTriplet( RealType *ax, RealType *bx, RealType *cx,
  PointType point, PointDataType grad, PointDataType data )
{
  const RealType Gold = 1.618034;
  const RealType Glimit = 100.0;
  const RealType Tiny = 1e-20;
  *ax = 0.0;
  *bx = 1.0;
  RealType fa = this->EvaluateEnergyForLineSearch( *ax, point, grad, data );
  RealType fb = this->EvaluateEnergyForLineSearch( *bx, point, grad, data );

  RealType dum;
  if( fb > fa )
    {
    dum = *ax;
    *ax = *bx;
    *bx = dum;
    dum = fb;
    fb = fa;
    fa = dum;
    }
  *cx = *bx + Gold*( *bx-*ax );
  RealType fc = this->EvaluateEnergyForLineSearch( *cx, point, grad, data );
  if( *cx < *ax )
    {
    // std::cout << "      Bracket triple: "
    //          << "f(" << *cx << ") = " << fc << ", "
    //          << "f(" << *bx << ") = " << fb << ", "
    //          << "f(" << *ax << ") = " << fa << std::endl;
    }
  else
    {
    // std::cout << "      Bracket triple: "
    //          << "f(" << *ax << ") = " << fa << ", "
    //          << "f(" << *bx << ") = " << fb << ", "
    //          << "f(" << *cx << ") = " << fc << std::endl;
    }
  RealType ulim, u, r, q, fu;
  unsigned int iter = 0;
  while( fb > fc )
    {
    r = ( *bx-*ax )*( fb-fc );
    q = ( *bx-*cx )*( fb-fa );
    RealType denom = 2.0*vnl_math_max( vnl_math_abs( q-r ), Tiny );
    if( q-r < 0.0 )
      {
      denom = -denom;
      }
    u = *bx - ( ( *bx-*cx )*q - ( *bx-*ax )*r )/denom;
    ulim = *bx + Glimit*( *cx-*bx );
    if( ( *bx-u )*( u-*cx ) > 0.0 )
      {
      fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
      if( fu < fc )
        {
        *ax = *bx;
        *bx = u;
        fa = fb;
        fb = fu;
        if( *cx < *ax )
          {
          // std::cout // <<  "      Bracket triple: "
                    // <<  "f(" // <<  *cx // <<  ") = " // <<  fc // <<  ", "
                    // <<  "f(" // <<  *bx // <<  ") = " // <<  fb // <<  ", "
                    // <<  "f(" // <<  *ax // <<  ") = " // <<  fa // <<  std::endl;
          }
        else
          {
          // std::cout // <<  "      Bracket triple: "
                    // <<  "f(" // <<  *ax // <<  ") = " // <<  fa // <<  ", "
                    // <<  "f(" // <<  *bx // <<  ") = " // <<  fb // <<  ", "
                    // <<  "f(" // <<  *cx // <<  ") = " // <<  fc // <<  std::endl;
          }
        return;
        }
      else if( fu > fb )
        {
        *cx = u;
        fc = fu;
        if( *cx < *ax )
          {
          // std::cout // <<  "      Bracket triple: "
                    // <<  "f(" // <<  *cx // <<  ") = " // <<  fc // <<  ", "
                    // <<  "f(" // <<  *bx // <<  ") = " // <<  fb // <<  ", "
                    // <<  "f(" // <<  *ax // <<  ") = " // <<  fa // <<  std::endl;
          }
        else
          {
          // std::cout // <<  "      Bracket triple: "
                    // <<  "f(" // <<  *ax // <<  ") = " // <<  fa // <<  ", "
                    // <<  "f(" // <<  *bx // <<  ") = " // <<  fb // <<  ", "
                    // <<  "f(" // <<  *cx // <<  ") = " // <<  fc // <<  std::endl;
          }
        return;
        }
      u = *cx + Gold*( *cx-*bx );
      fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
      }
    else if( ( *cx-u )*( u-ulim ) > 0.0 )
      {
      fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
      if( fu < fc )
        {
        *bx = *cx;
        *cx = u;
        u = *cx + Gold*( *cx-*bx );
        fb = fc;
        fc = fu;
        fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
        }
      }
    else if( ( u-ulim )*( ulim-*cx ) >=  0.0 )
      {
      u = ulim;
      fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
      }
    else
      {
      u = *cx + Gold*( *cx-*bx );
      fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
      }

    *ax = *bx;
    *bx = *cx;
    *cx = u;
    fa = fb;
    fb = fc;
    fc = fu;
    if( *cx < *ax )
      {
      // std::cout // <<  "      Bracket triple: "
                // <<  "f(" // <<  *cx // <<  ") = " // <<  fc // <<  ", "
                // <<  "f(" // <<  *bx // <<  ") = " // <<  fb // <<  ", "
                // <<  "f(" // <<  *ax // <<  ") = " // <<  fa // <<  std::endl;
      }
    else
      {
      // std::cout // <<  "      Bracket triple: "
                // <<  "f(" // <<  *ax // <<  ") = " // <<  fa // <<  ", "
                // <<  "f(" // <<  *bx // <<  ") = " // <<  fb // <<  ", "
                // <<  "f(" // <<  *cx // <<  ") = " // <<  fc // <<  std::endl;
      }
    }
}

template<class TInputImage, class TOutputImage>
void
BSplineControlPointImageFilter<TInputImage, TOutputImage>
::BrentSearch( RealType ax, RealType bx, RealType cx, RealType *step,
  RealType *fret, PointType point, PointDataType grad, PointDataType data )
{
  const unsigned int ITMAX = 100;
  const RealType R = 0.6180339;
  const RealType CGOLD = 1.0 - R;
  const RealType tol = 1e-4;
  const RealType ZEPS = 1.0e-10;

  RealType a;
  RealType b;
  RealType d;
  RealType p;
  RealType q;
  RealType r;
  RealType etemp;
  RealType e = 0.0;

  if( ax < cx )
    {
    a = ax;
    b = cx;
    }
  else
    {
    a = cx;
    b = ax;
    }
  RealType x, fx;
  RealType w, fw;
  RealType v, fv;
  RealType u, fu;
  x = w = v = bx;
  fx = fw = fv = this->EvaluateEnergyForLineSearch( x, point, grad, data );

  for( unsigned int iter = 1; iter <= 100; iter++ )
    {
    // std::cout // <<  "        Iteration (Brent's) " // <<  iter
              // <<  ": f(x = " // <<  x // <<  ") = " // <<  fx // <<  ", x in ["
              // <<  a // <<  ", " // <<  b // <<  "] " // <<  std::endl;

    RealType xm = 0.5 * ( a + b );
    RealType tol1 = tol * vnl_math_abs( x ) + ZEPS;
    RealType tol2 = 2.0 * tol1;
    if( vnl_math_abs( x - xm ) <= ( tol2 - 0.5 * ( b - a ) ) )
      {
      // std::cout // <<   "    Results of line search: E(" // <<  x // <<  ") = " // <<  fx // <<  "." // <<  std::endl;
      *step = x;
      *fret = fx;
      return;
      }
    if( vnl_math_abs( e ) > tol1 )
      {
      r = ( x - w ) * ( fx - fv );
      q = ( x - v ) * ( fx - fw );
      p = ( x - v ) * q - ( x - w ) * r;
      q = 2.0 * ( q - r );
      if( q > 0.0 )
        {
        p = -p;
        }
      q = vnl_math_abs( q );
      etemp = e;
      e = d;
      if( vnl_math_abs( p ) >= vnl_math_abs( 0.5 * q * etemp )
           || p <= q * ( a - x ) || p >= q * ( b - x ) )
        {
        if( x >= xm )
          {
          e = a - x;
          }
        else
          {
          e = b - x;
          }
        d = CGOLD * e;
        }
      else
        {
        d = p / q;
        u = x + d;
        if( u - a < tol2 || b - u < tol2 )
          {
          d = vnl_math_abs( tol1 );
          if( xm - x <= 0 )
            {
            d = -d;
            }
          }
        }
      }
    else
      {
      if( x >= xm )
        {
        e = a - x;
        }
      else
        {
        e = b - x;
        }
      d = CGOLD * e;
      }
    if( vnl_math_abs( d ) >= tol1 )
      {
      u = x + d;
      }
    else
      {
      u = x + vnl_math_abs( tol1 );
      if( d <= 0 )
        {
        u = x - vnl_math_abs( tol1 );
        }
      }
    fu = this->EvaluateEnergyForLineSearch( u, point, grad, data );
    if( fu <= fx )
      {
      if( u >= x )
        {
        a = x;
        }
      else
        {
        b = x;
        }
      v = w;
      w = x;
      x = u;
      fv = fw;
      fw = fx;
      fx = fu;
      }
    else
      {
      if( u < x )
        {
        a = u;
        }
      else
        {
        b = u;
        }
      if( fu <= fw || w == x )
        {
        v = w;
        w = u;
        fv = fw;
        fw = fu;
        }
      else if( fu <= fv || v == x || v == w )
        {
        v = u;
        fv = fu;
        }
      }
    }
  *step = x;
  *fret = fx;
  // std::cout <<  "    Results of line search: E(" << x << ") = " << fx << "." << std::endl;
}

/**
 * Standard "PrintSelf" method
 */
template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::PrintSelf(
  std::ostream& os,
  Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    this->m_Kernel[i]->Print( os, indent );
    }
  os << indent << "B-spline order: "
     << this->m_SplineOrder << std::endl;
  os << indent << "Close dimension: "
     << this->m_CloseDimension << std::endl;
}



}  //end namespace itk

#endif
