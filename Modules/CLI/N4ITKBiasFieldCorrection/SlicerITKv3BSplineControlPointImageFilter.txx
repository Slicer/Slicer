#ifndef __itkBSplineControlPointImageFilter_txx
#define __itkBSplineControlPointImageFilter_txx

#include "SlicerITKv3BSplineControlPointImageFilter.h"

#include "itkImageDuplicator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkLBFGSBOptimizer.h"

namespace itk
{

/**
 * ParameterCostFunction class definitions
 */
template <class TControlPointLattice>
ParameterCostFunction<TControlPointLattice>
::ParameterCostFunction()
{
  this->m_ControlPointLattice = NULL;
}

template <class TControlPointLattice>
ParameterCostFunction<TControlPointLattice>
::~ParameterCostFunction()
{
}

template <class TControlPointLattice>
typename ParameterCostFunction<TControlPointLattice>::MeasureType
ParameterCostFunction<TControlPointLattice>
::GetValue( const ParametersType & parameters ) const
{
  typename TControlPointLattice::PointType point;
  for( unsigned int d = 0; d < ParametricDimension; d++ )
    {
    point[d] = parameters[d];
    if( this->m_CloseDimension[d] )
      {
      MeasureType sign = -1.0;
      if( point[d] < 0.0 )
        {
        sign = 1.0;
        }
      while( point[d] < 0 || point[d] >= 1.0 )
        {
        point[d] += sign;
        }
      }
    }

  typedef BSplineControlPointImageFilter<TControlPointLattice> BSplineFilterType;
  typename BSplineFilterType::Pointer bspliner = BSplineFilterType::New();
  bspliner->SetOrigin( this->m_Origin );
  bspliner->SetSpacing( this->m_Spacing );
  bspliner->SetSize( this->m_Size );
  bspliner->SetDirection( this->m_Direction );
  bspliner->SetSplineOrder( this->m_SplineOrder );
  bspliner->SetCloseDimension( this->m_CloseDimension );
  bspliner->SetInput( this->m_ControlPointLattice );

  typename TControlPointLattice::PixelType value;
  bspliner->Evaluate( point, value );

  MeasureType metric = 0.0;
  for( unsigned int d = 0; d < value.Size(); d++ )
    {
    metric += vnl_math_sqr( value[d] - this->m_DataPoint[d] );
    }

  return metric;
}

template <class TControlPointLattice>
void
ParameterCostFunction<TControlPointLattice>
::GetDerivative( const ParametersType & parameters,
                 DerivativeType & derivative ) const
{
  typename TControlPointLattice::PointType point;
  for( unsigned int d = 0; d < ParametricDimension; d++ )
    {
    point[d] = parameters[d];
    if( this->m_CloseDimension[d] )
      {
      MeasureType sign = -1.0;
      if( point[d] < 0.0 )
        {
        sign = 1.0;
        }
      while( point[d] < 0 || point[d] >= 1.0 )
        {
        point[d] += sign;
        }
      }
    }

  typedef BSplineControlPointImageFilter<TControlPointLattice> BSplineFilterType;
  typename BSplineFilterType::Pointer bspliner = BSplineFilterType::New();
  bspliner->SetOrigin( this->m_Origin );
  bspliner->SetSpacing( this->m_Spacing );
  bspliner->SetSize( this->m_Size );
  bspliner->SetDirection( this->m_Direction );
  bspliner->SetSplineOrder( this->m_SplineOrder );
  bspliner->SetCloseDimension( this->m_CloseDimension );
  bspliner->SetInput( this->m_ControlPointLattice );

  typename TControlPointLattice::PixelType value;
  bspliner->Evaluate( point, value );
  typename BSplineFilterType::GradientType gradient;
  bspliner->EvaluateGradient( point, gradient );

  derivative.SetSize( this->GetNumberOfParameters() );
  derivative.Fill( 0.0 );
  for( unsigned int i = 0; i < gradient.Rows(); i++ )
    {
    for( unsigned int j = 0; j < gradient.Cols(); j++ )
      {
      derivative[j] += ( 2.0 * ( value[i] - this->m_DataPoint[i] )
                         * gradient(i, j) );
      }
    }
}

template <class TControlPointLattice>
unsigned int
ParameterCostFunction<TControlPointLattice>
::GetNumberOfParameters() const
{
  return ParametricDimension;
}

/**
 * BSplineControlPointImageFilter class definitions
 */

template <class InputImage, class TOutputImage>
BSplineControlPointImageFilter<InputImage, TOutputImage>
::BSplineControlPointImageFilter()
{
  this->m_SplineOrder.Fill( 3 );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    this->m_NumberOfControlPoints[i] = ( this->m_SplineOrder[i] + 1 );
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

  this->m_Spacing.Fill(1.0);
  this->m_Size.Fill(1);
  this->m_Origin.Fill(0.);
  this->m_Direction.SetIdentity();
  this->m_DoMultilevel = false;
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

  itkDebugMacro( "Setting m_NumberOfLevels to "
                 << this->m_NumberOfLevels );
  itkDebugMacro( "Setting m_MaximumNumberOfLevels to "
                 << this->m_MaximumNumberOfLevels );

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
        RealType c = vcl_pow( static_cast<RealType>( 2.0 ),
                              static_cast<RealType>( C.cols() - j - 1 ) );
        for( unsigned int k = 0; k < C.rows(); k++ )
          {
          R(k, j) *= c;
          }
        }
      R = R.transpose();
      R.flipud();
      S = S.transpose();
      S.flipud();

      this->m_RefinedLatticeCoefficients[i] =
        ( vnl_svd<RealType>( R ).solve( S ) ).extract( 2, S.cols() );
      }
    }
  this->Modified();
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::GenerateData()
{
  for( unsigned int i = 0; i < ImageDimension; i++ )
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
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    this->m_NumberOfControlPoints[i] =
      this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
    }

  this->GenerateOutputImageFast();
}

template <class TInputPointImage, class TOutputImage>
typename BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
::ControlPointLatticeType::Pointer
BSplineControlPointImageFilter<TInputPointImage, TOutputImage>
::RefineControlPointLattice( ArrayType numberOfLevels )
{
  this->SetNumberOfLevels( numberOfLevels );

  typedef ImageDuplicator<ControlPointLatticeType> ImageDuplicatorType;
  typename ImageDuplicatorType::Pointer Duplicator = ImageDuplicatorType::New();
  Duplicator->SetInputImage( this->GetInput() );
  Duplicator->Update();

  typename ControlPointLatticeType::Pointer psiLattice =
    ControlPointLatticeType::New();
  psiLattice = Duplicator->GetOutput();
  for( unsigned int m = 1; m < this->m_MaximumNumberOfLevels; m++ )
    {
    ArrayType numberOfNewControlPoints;
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      numberOfNewControlPoints[i] =
        psiLattice->GetLargestPossibleRegion().GetSize()[i];
      }
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      if( m < this->m_NumberOfLevels[i] )
        {
        numberOfNewControlPoints[i] =
          2 * numberOfNewControlPoints[i] - this->m_SplineOrder[i];
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

    typename ControlPointLatticeType::Pointer refinedLattice =
      ControlPointLatticeType::New();
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
          idx_Psi[i] = static_cast<unsigned int>( 0.5 * idx[i] );
          }
        else
          {
          idx_Psi[i] = static_cast<unsigned int>( idx[i] );
          }
        }
      for( unsigned int i = 0; i < ( 2 << (ImageDimension - 1) ); i++ )
        {
        PixelType sum( 0.0 );
        PixelType val;
        off = this->NumberToIndex( i, size );

        bool outOfBoundary = false;
        for( unsigned int j = 0; j < ImageDimension; j++ )
          {
          tmp[j] = idx[j] + off[j];
          if( tmp[j] >= static_cast<int>( numberOfNewControlPoints[j] ) &&
              !this->m_CloseDimension[j] )
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
                  this->GetInput()->GetLargestPossibleRegion().GetSize()[k] ) &&
                !this->m_CloseDimension[k] )
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
              this->m_RefinedLatticeCoefficients[k] ( off[k], off_Psi[k] );
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
  collapsedPhiLattices[ImageDimension + 1];
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
      totalNumberOfSpans[i] =
        this->GetInput()->GetLargestPossibleRegion().GetSize()[i];
      }
    else
      {
      totalNumberOfSpans[i] =
        this->GetInput()->GetLargestPossibleRegion().GetSize()[i]
        - this->m_SplineOrder[i];
      }
    }
  FixedArray<RealType, ImageDimension> U;
  FixedArray<RealType, ImageDimension> currentU;
  currentU.Fill( -1 );
  typename ImageType::IndexType startIndex =
    this->GetOutput()->GetRequestedRegion().GetIndex();
  typename ControlPointLatticeType::IndexType startPhiIndex =
    this->GetInput()->GetLargestPossibleRegion().GetIndex();

  ImageRegionIteratorWithIndex<ImageType>
  It( this->GetOutput(), this->GetOutput()->GetRequestedRegion() );
  for( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    typename ImageType::IndexType idx = It.GetIndex();
    for( unsigned int i = 0; i < ImageDimension; i++ )
      {
      U[i] = static_cast<RealType>( totalNumberOfSpans[i] )
        * static_cast<RealType>( idx[i] - startIndex[i] )
        / static_cast<RealType>( this->m_Size[i] - 1 );
      if( vnl_math_abs( U[i] - static_cast<RealType>(
                          totalNumberOfSpans[i] ) ) <= this->m_BSplineEpsilon )
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
    for( int i = ImageDimension - 1; i >= 0; i-- )
      {
      if( U[i] != currentU[i] )
        {
        for( int j = i; j >= 0; j-- )
          {
          this->CollapsePhiLattice( collapsedPhiLattices[j + 1],
                                    collapsedPhiLattices[j], U[j], j );
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
                      ControlPointLatticeType *collapsedLattice, RealType u, unsigned int dimension )
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
        + 0.5 * static_cast<RealType>( this->m_SplineOrder[dimension] - 1 );
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
::EvaluateAtPoint( PointType point, PixelType & data )
{
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    point[i] -= this->m_Origin[i];
    point[i] /=
      ( static_cast<RealType>( this->m_Size[i] - 1 ) * this->m_Spacing[i] );
    }
  this->Evaluate( point, data );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateAtIndex( IndexType idx, PixelType & data )
{
  PointType point;

  this->GetOutput()->TransformIndexToPhysicalPoint( idx, point );
  this->EvaluateAtPoint( point, data );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateAtContinuousIndex( ContinuousIndexType idx, PixelType & data )
{
  PointType point;

  this->GetOutput()->TransformContinuousIndexToPhysicalPoint( idx, point );
  this->EvaluateAtPoint( point, data );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::Evaluate( PointType params, PixelType & data )
{
  vnl_vector<RealType> p( ImageDimension );
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    if( params[i] == NumericTraits<RealType>::OneValue() )
      {
      params[i] = NumericTraits<RealType>::OneValue() - this->m_BSplineEpsilon;
      }
    if( params[i] < 0.0 || params[i] >= 1.0 )
      {
      itkExceptionMacro( "The specified point " << params
                                                << " is outside the reparameterized domain [0, 1)." );
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
                                                 + idx[i] ) + 0.5 * static_cast<RealType>( this->m_SplineOrder[i] - 1 );

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
::EvaluateGradientAtPoint( PointType point, GradientType & gradient )
{
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    point[i] -= this->m_Origin[i];
    point[i] /= ( static_cast<RealType>( this->m_Size[i] - 1 )
                  * this->m_Spacing[i] );
    }
  this->EvaluateGradient( point, gradient );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradientAtIndex( IndexType idx, GradientType & gradient )
{
  PointType point;

  this->GetOutput()->TransformIndexToPhysicalPoint( idx, point );
  this->EvaluateGradientAtPoint( point, gradient );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradientAtContinuousIndex( ContinuousIndexType idx,
                                     GradientType & gradient )
{
  PointType point;

  this->GetOutput()->TransformContinuousIndexToPhysicalPoint( idx, gradient );
  this->EvaluateGradientAtPoint( point, gradient );
}

template <class InputImage, class TOutputImage>
void
BSplineControlPointImageFilter<InputImage, TOutputImage>
::EvaluateGradient( PointType params, GradientType & gradient )
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
      params[i] -= this->m_BSplineEpsilon;
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
                                                   + idx[i] ) + 0.5 * static_cast<RealType>( this->m_SplineOrder[i] - 1 );
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
::EvaluateHessian( PointType params, GradientType & hessian,
                   unsigned int component )
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
      params[i] = 1.0 - this->m_BSplineEpsilon;
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
              static_cast<unsigned>( p[i] ) + idx[i] ) + 0.5
            * static_cast<RealType>( this->m_SplineOrder[i] - 1 );
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
::CalculateParametersClosestToDataPoint( PointDataType point, PointType & params )
{
  typedef ParameterCostFunction<ControlPointLatticeType> CostFunctionType;
  typename CostFunctionType::Pointer costFunction = CostFunctionType::New();

  costFunction->SetControlPointLattice(
    const_cast<ControlPointLatticeType *>( this->GetInput() ) );
  costFunction->SetOrigin( this->m_Origin );
  costFunction->SetSpacing( this->m_Spacing );
  costFunction->SetSize( this->m_Size );
  costFunction->SetDirection( this->m_Direction );
  costFunction->SetSplineOrder( this->m_SplineOrder );
  costFunction->SetCloseDimension( this->m_CloseDimension );
  costFunction->SetDataPoint( point );

  /**
   * Rescale parameters between [0, 1)
   */
  typename LBFGSBOptimizer::ParametersType initialParameters;
  initialParameters.SetSize( ImageDimension );
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    initialParameters[d] = ( params[d] - this->m_Origin[d] )
      / ( static_cast<RealType>( this->m_Size[d] - 1 ) * this->m_Spacing[d] );
    }
  typename LBFGSBOptimizer::BoundSelectionType boundSelection;
  boundSelection.SetSize( ImageDimension );
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    if( this->m_CloseDimension[d] )
      {
      boundSelection[d] = 0;
      }
    else
      {
      boundSelection[d] = 2;
      }
    }
  typename LBFGSBOptimizer::BoundValueType lowerBounds;
  typename LBFGSBOptimizer::BoundValueType upperBounds;
  lowerBounds.SetSize( ImageDimension );
  upperBounds.SetSize( ImageDimension );
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    lowerBounds[d] = 0.0;
    upperBounds[d] = 1.0;
    }

  typename LBFGSBOptimizer::Pointer optimizer = LBFGSBOptimizer::New();
  optimizer->SetMinimize( true );
  optimizer->SetCostFunction( costFunction );
  optimizer->SetInitialPosition( initialParameters );
  optimizer->SetCostFunctionConvergenceFactor( 1e1 );
  optimizer->SetLowerBound( lowerBounds );
  optimizer->SetUpperBound( upperBounds );
  optimizer->SetBoundSelection( boundSelection );
  optimizer->SetProjectedGradientTolerance( 1e-5 );

  optimizer->StartOptimization();

  typename LBFGSBOptimizer::ParametersType finalParameters =
    optimizer->GetCurrentPosition();
  for( unsigned int i = 0; i < ImageDimension; i++ )
    {
    point[i] -= this->m_Origin[i];
    point[i] /= ( static_cast<RealType>( this->m_Size[i] - 1 )
                  * this->m_Spacing[i] );
    }
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    params[d] = finalParameters[d] * this->m_Spacing[d]
      * static_cast<RealType>( this->m_Size[d] - 1 ) + this->m_Origin[d];
    }
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
  os << indent << "Spline order: " << this->m_SplineOrder << std::endl;
  os << indent << "Close dimension: " << this->m_CloseDimension << std::endl;
  os << indent << "Parametric domain" << std::endl;
  os << indent << "  Origin:    " << this->m_Origin << std::endl;
  os << indent << "  Spacing:   " << this->m_Spacing << std::endl;
  os << indent << "  Size:      " << this->m_Size << std::endl;
  os << indent << "  Direction: " << this->m_Direction << std::endl;
}

}  // end namespace itk

#endif
