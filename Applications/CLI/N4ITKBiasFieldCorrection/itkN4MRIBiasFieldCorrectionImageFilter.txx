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
#ifndef __itkN4MRIBiasFieldCorrectionImageFilter_txx
#define __itkN4MRIBiasFieldCorrectionImageFilter_txx

#include "itkN4MRIBiasFieldCorrectionImageFilter.h"

#include "itkAddImageFilter.h"
#include "itkBSplineControlPointImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkIterationReporter.h"
#include "itkSubtractImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "itkProgressReporter.h"

#include "vnl/algo/vnl_fft_1d.h"
#include "vnl/vnl_complex_traits.h"
#include "vxl/vcl/vcl_complex.h"

namespace itk {

template <class TInputImage, class TMaskImage, class TOutputImage>
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::N4MRIBiasFieldCorrectionImageFilter() :
  m_MaskLabel( NumericTraits<MaskPixelType>::One ),
  m_NumberOfHistogramBins( 200 ),
  m_WeinerFilterNoise( 0.01 ),
  m_BiasFieldFullWidthAtHalfMaximum( 0.15 ),
  m_ConvergenceThreshold( 0.001 ),
  m_SplineOrder( 3 )
{
  this->SetNumberOfRequiredInputs( 1 );

  this->m_LogBiasFieldControlPointLattice = NULL;

  this->m_NumberOfFittingLevels.Fill( 1 );
  this->m_NumberOfControlPoints.Fill( 4 );

  this->m_MaximumNumberOfIterations.SetSize( 1 );
  this->m_MaximumNumberOfIterations.Fill( 50 );
}

template<class TInputImage, class TMaskImage, class TOutputImage>
void
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::GenerateData()
{
  this->AllocateOutputs();

  const InputImageType * inputImage = this->GetInput();
  typedef typename InputImageType::RegionType RegionType;
  const RegionType inputRegion = inputImage->GetBufferedRegion();


  // Calculate the log of the input image.
  RealImagePointer logInputImage = RealImageType::New();
  logInputImage->CopyInformation( inputImage );
  logInputImage->SetRegions( inputRegion );
  logInputImage->Allocate();

  ImageRegionConstIterator<InputImageType> inpItr( inputImage, inputRegion );
  ImageRegionIterator<RealImageType> outItr( logInputImage, inputRegion );

  inpItr.GoToBegin();
  outItr.GoToBegin();

  while( !inpItr.IsAtEnd() )
    {
    outItr.Set( static_cast< RealType >( inpItr.Get() ) );
    ++inpItr;
    ++outItr;
    }

  ImageRegionIteratorWithIndex<RealImageType> It( logInputImage, inputRegion );

  for( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    if( ( !this->GetMaskImage() ||
          this->GetMaskImage()->GetPixel( It.GetIndex() ) == this->m_MaskLabel )
        && ( !this->GetConfidenceImage() ||
             this->GetConfidenceImage()->GetPixel( It.GetIndex() ) > 0.0 ) )
      {
      if( It.Get() > NumericTraits<typename InputImageType::PixelType>::Zero )
        {
        It.Set( vcl_log( static_cast< RealType >( It.Get() ) ) );
        }
      }
    }

  // Duplicate logInputImage since we reuse the original at each iteration.

  typedef ImageDuplicator<RealImageType> DuplicatorType;
  typename DuplicatorType::Pointer duplicator = DuplicatorType::New();
  duplicator->SetInputImage( logInputImage );
  duplicator->Update();

  RealImagePointer logUncorrectedImage = duplicator->GetOutput();

  // Provide an initial log bias field of zeros

  RealImagePointer logBiasField = RealImageType::New();
  logBiasField->CopyInformation( inputImage );
  logBiasField->SetRegions( inputImage->GetLargestPossibleRegion() );
  logBiasField->Allocate();
  logBiasField->FillBuffer( 0.0 );


  // Iterate until convergence or iterative exhaustion.
  unsigned int maximumNumberOfLevels = 1;
  for( unsigned int d = 0; d < this->m_NumberOfFittingLevels.Size(); d++ )
    {
    if( this->m_NumberOfFittingLevels[d] > maximumNumberOfLevels )
      {
      maximumNumberOfLevels = this->m_NumberOfFittingLevels[d];
      }
    }
  if( this->m_MaximumNumberOfIterations.Size() != maximumNumberOfLevels )
    {
    itkExceptionMacro(
      "Number of iteration levels is not equal to the max number of levels." );
    }

  unsigned totalMaxNumberOfIterations = 0;
  for(unsigned level=0;level<maximumNumberOfLevels;level++)
    totalMaxNumberOfIterations += this->m_MaximumNumberOfIterations[level];  
  ProgressReporter progress(this, 0, totalMaxNumberOfIterations);

  for( this->m_CurrentLevel = 0; this->m_CurrentLevel < maximumNumberOfLevels;
       this->m_CurrentLevel++ )
    {
    IterationReporter reporter( this, 0, 1 );

    this->m_ElapsedIterations = 0;
    this->m_CurrentConvergenceMeasurement = NumericTraits<RealType>::max();
    while( this->m_ElapsedIterations++ <
           this->m_MaximumNumberOfIterations[this->m_CurrentLevel] &&
           this->m_CurrentConvergenceMeasurement > this->m_ConvergenceThreshold )
      {

      // Sharpen the current estimate of the uncorrected image.

      RealImagePointer logSharpenedImage = this->SharpenImage( logUncorrectedImage );

      typedef SubtractImageFilter<RealImageType, RealImageType, RealImageType>
      SubtracterType;
      typename SubtracterType::Pointer subtracter1 = SubtracterType::New();
      subtracter1->SetInput1( logUncorrectedImage );
      subtracter1->SetInput2( logSharpenedImage );
      subtracter1->Update();

      // Smooth the residual bias field estimate and add the resulting
      // control point grid to get the new total bias field estimate.

      RealImagePointer newLogBiasField = this->UpdateBiasFieldEstimate( subtracter1->GetOutput() );

      this->m_CurrentConvergenceMeasurement =
        this->CalculateConvergenceMeasurement( logBiasField, newLogBiasField );
      logBiasField = newLogBiasField;

      typename SubtracterType::Pointer subtracter2 = SubtracterType::New();
      subtracter2->SetInput1( logInputImage );
      subtracter2->SetInput2( logBiasField );
      subtracter2->Update();
      logUncorrectedImage = subtracter2->GetOutput();

      reporter.CompletedStep();
      progress.CompletedPixel();
      }

    typedef BSplineControlPointImageFilter
    <BiasFieldControlPointLatticeType, ScalarImageType>
    BSplineReconstructerType;
    typename BSplineReconstructerType::Pointer reconstructer =
      BSplineReconstructerType::New();
    reconstructer->SetInput( this->m_LogBiasFieldControlPointLattice );
    reconstructer->SetOrigin( logBiasField->GetOrigin() );
    reconstructer->SetSpacing( logBiasField->GetSpacing() );
    reconstructer->SetDirection( logBiasField->GetDirection() );
    reconstructer->SetSize( logBiasField->GetLargestPossibleRegion().GetSize() );
    reconstructer->Update();

    typename BSplineReconstructerType::ArrayType numberOfLevels;
    numberOfLevels.Fill( 1 );
    for( unsigned int d = 0; d < ImageDimension; d++ )
      {
      if( this->m_NumberOfFittingLevels[d] + 1 >= this->m_CurrentLevel &&
          this->m_CurrentLevel != maximumNumberOfLevels-1 )
        {
        numberOfLevels[d] = 2;
        }
      }
    this->m_LogBiasFieldControlPointLattice = reconstructer->
      RefineControlPointLattice( numberOfLevels );
    }

  typedef ExpImageFilter<RealImageType, RealImageType> ExpImageFilterType;
  typename ExpImageFilterType::Pointer expFilter = ExpImageFilterType::New();
  expFilter->SetInput( logBiasField );
  expFilter->Update();


  // Divide the input image by the bias field to get the final image.

  typedef DivideImageFilter<InputImageType, RealImageType, OutputImageType>
  DividerType;
  typename DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( inputImage );
  divider->SetInput2( expFilter->GetOutput() );
  divider->GraftOutput( this->GetOutput() );
  divider->Update();

  this->GraftOutput( divider->GetOutput() );
}

template<class TInputImage, class TMaskImage, class TOutputImage>
typename
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>::RealImagePointer
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::SharpenImage( const RealImageType *unsharpenedImage ) const
{
  // Build the histogram for the uncorrected image.  Store copy
  // in a vnl_vector to utilize vnl FFT routines.  Note that variables
  // in real space are denoted by a single uppercase letter whereas their
  // frequency counterparts are indicated by a trailing lowercase 'f'.

  RealType binMaximum = NumericTraits<RealType>::NonpositiveMin();
  RealType binMinimum = NumericTraits<RealType>::max();

  ImageRegionConstIterator<RealImageType> ItU(
    unsharpenedImage, unsharpenedImage->GetLargestPossibleRegion() );

  for( ItU.GoToBegin(); !ItU.IsAtEnd(); ++ItU )
    {
    if( ( !this->GetMaskImage() ||
          this->GetMaskImage()->GetPixel( ItU.GetIndex() ) == this->m_MaskLabel )
        && ( !this->GetConfidenceImage() ||
             this->GetConfidenceImage()->GetPixel( ItU.GetIndex() ) > 0.0 ) )
      {
      RealType pixel = ItU.Get();
      if( pixel > binMaximum )
        {
        binMaximum = pixel;
        }
      else if( pixel < binMinimum )
        {
        binMinimum = pixel;
        }
      }
    }
  RealType histogramSlope = ( binMaximum - binMinimum ) /
    static_cast<RealType>( this->m_NumberOfHistogramBins - 1 );

  // Create the intensity profile (within the masked region, if applicable)
  // using a triangular parzen windowing scheme.

  vnl_vector<RealType> H( this->m_NumberOfHistogramBins, 0.0 );

  for( ItU.GoToBegin(); !ItU.IsAtEnd(); ++ItU )
    {
    if( ( !this->GetMaskImage() ||
          this->GetMaskImage()->GetPixel( ItU.GetIndex() ) == this->m_MaskLabel )
        && ( !this->GetConfidenceImage() ||
             this->GetConfidenceImage()->GetPixel( ItU.GetIndex() ) > 0.0 ) )
      {
      RealType pixel = ItU.Get();

      RealType cidx = ( static_cast<RealType>( pixel ) - binMinimum ) /
        histogramSlope;
      unsigned int idx = vnl_math_floor( cidx );
      RealType     offset = cidx - static_cast<RealType>( idx );

      if( offset == 0.0 )
        {
        H[idx] += 1.0;
        }
      else if( idx < this->m_NumberOfHistogramBins - 1 )
        {
        H[idx] += 1.0 - offset;
        H[idx+1] += offset;
        }
      }
    }

  // Determine information about the intensity histogram and zero-pad
  // histogram to a power of 2.

  RealType exponent =
    vcl_ceil( vcl_log( static_cast<RealType>( this->m_NumberOfHistogramBins ) ) /
              vcl_log( 2.0 ) ) + 1;
  unsigned int paddedHistogramSize = static_cast<unsigned int>(
    vcl_pow( static_cast<RealType>( 2.0 ), exponent ) + 0.5 );
  unsigned int histogramOffset = static_cast<unsigned int>( 0.5 *
    ( paddedHistogramSize - this->m_NumberOfHistogramBins ) );

  vnl_vector< vcl_complex<RealType> > V( paddedHistogramSize,
                                         vcl_complex<RealType>( 0.0, 0.0 ) );

  for( unsigned int n = 0; n < this->m_NumberOfHistogramBins; n++ )
    {
    V[n+histogramOffset] = H[n];
    }

  // Instantiate the 1-d vnl fft routine.

  vnl_fft_1d<RealType> fft( paddedHistogramSize );

  vnl_vector< vcl_complex<RealType> > Vf( V );

  fft.fwd_transform( Vf );

  // Create the Gaussian filter.

  RealType scaledFWHM = this->m_BiasFieldFullWidthAtHalfMaximum / histogramSlope;
  RealType expFactor = 4.0 * vcl_log( 2.0 ) / vnl_math_sqr( scaledFWHM );
  RealType scaleFactor = 2.0 * vcl_sqrt( vcl_log( 2.0 )
                                         / vnl_math::pi ) / scaledFWHM;

  vnl_vector< vcl_complex<RealType> > F( paddedHistogramSize,
                                         vcl_complex<RealType>( 0.0, 0.0 ) );

  F[0] = vcl_complex<RealType>( scaleFactor, 0.0 );
  unsigned int halfSize = static_cast<unsigned int>(
      0.5 * paddedHistogramSize );
  for( unsigned int n = 1; n <= halfSize; n++ )
    {
    F[n] = F[paddedHistogramSize - n] = vcl_complex<RealType>( scaleFactor *
      vcl_exp( -vnl_math_sqr( static_cast<RealType>( n ) ) * expFactor ), 0.0 );
    }
  if( paddedHistogramSize % 2 == 0 )
    {
    F[halfSize] = vcl_complex<RealType>( scaleFactor * vcl_exp( 0.25 *
      -vnl_math_sqr( static_cast<RealType>( paddedHistogramSize ) ) *
      expFactor ), 0.0 );
    }

  vnl_vector< vcl_complex<RealType> > Ff( F );

  fft.fwd_transform( Ff );

  // Create the Weiner deconvolution filter.

  vnl_vector< vcl_complex<RealType> > Gf( paddedHistogramSize );

  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    vcl_complex<RealType> c =
      vnl_complex_traits< vcl_complex<RealType> >::conjugate( Ff[n] );
    Gf[n] = c / ( c * Ff[n] + this->m_WeinerFilterNoise );
    }

  vnl_vector< vcl_complex<RealType> > Uf( paddedHistogramSize );

  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    Uf[n] = Vf[n] * Gf[n].real();
    }

  vnl_vector< vcl_complex<RealType> > U( Uf );

  fft.bwd_transform( U );
  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    U[n] = vcl_complex<RealType>( vnl_math_max( U[n].real(),
      static_cast<RealType>( 0.0 ) ), 0.0 );
    }

  // Compute mapping E(u|v).

  vnl_vector< vcl_complex<RealType> > numerator( paddedHistogramSize );

  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    numerator[n] = vcl_complex<RealType>(
        ( binMinimum + ( static_cast<RealType>( n ) - histogramOffset )
          * histogramSlope ) * U[n].real(), 0.0 );
    }
  fft.fwd_transform( numerator );
  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    numerator[n] *= Ff[n];
    }
  fft.bwd_transform( numerator );

  vnl_vector< vcl_complex<RealType> > denominator( U );

  fft.fwd_transform( denominator );
  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    denominator[n] *= Ff[n];
    }
  fft.bwd_transform( denominator );

  vnl_vector<RealType> E( paddedHistogramSize );

  for( unsigned int n = 0; n < paddedHistogramSize; n++ )
    {
    if( denominator[n].real() != 0.0 )
      {
      E[n] = numerator[n].real() / denominator[n].real();
      }
    else
      {
      E[n] = 0.0;
      }
    }

  // Remove the zero-padding from the mapping.

  E = E.extract( this->m_NumberOfHistogramBins, histogramOffset );

  const InputImageType * inputImage = const_cast<Self*>(this)->GetInput();

  // Sharpen the image with the new mapping, E(u|v)
  RealImagePointer sharpenedImage = RealImageType::New();
  sharpenedImage->CopyInformation( inputImage );
  sharpenedImage->SetRegions( inputImage->GetLargestPossibleRegion() );
  sharpenedImage->Allocate();
  sharpenedImage->FillBuffer( 0.0 );

  ImageRegionIterator<RealImageType> ItC(
    sharpenedImage, sharpenedImage->GetLargestPossibleRegion() );

  for( ItU.GoToBegin(), ItC.GoToBegin(); !ItU.IsAtEnd(); ++ItU, ++ItC )
    {
    if( ( !this->GetMaskImage() ||
          this->GetMaskImage()->GetPixel( ItU.GetIndex() ) == this->m_MaskLabel )
        && ( !this->GetConfidenceImage() ||
             this->GetConfidenceImage()->GetPixel( ItU.GetIndex() ) > 0.0 ) )
      {
      RealType     cidx = ( ItU.Get() - binMinimum ) / histogramSlope;
      unsigned int idx = vnl_math_floor( cidx );

      RealType correctedPixel = 0;
      if( idx < E.size() - 1 )
        {
        correctedPixel = E[idx] + ( E[idx + 1] - E[idx] )
          * ( cidx - static_cast<RealType>( idx ) );
        }
      else
        {
        correctedPixel = E[E.size() - 1];
        }
      ItC.Set( correctedPixel );
      }
    }

  return sharpenedImage;
}

template<class TInputImage, class TMaskImage, class TOutputImage>
typename
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>::RealImagePointer
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::UpdateBiasFieldEstimate( RealImageType* fieldEstimate )
{
  // Get original direction and change to identity temporarily for the
  // b-spline fitting.
  typedef typename RealImageType::DirectionType   DirectionType;
  DirectionType direction = fieldEstimate->GetDirection();
  DirectionType identity;

  identity.SetIdentity();
  fieldEstimate->SetDirection( identity );

  PointSetPointer fieldPoints = PointSetType::New();
  fieldPoints->Initialize();

  typename BSplineFilterType::WeightsContainerType::Pointer weights =
    BSplineFilterType::WeightsContainerType::New();
  weights->Initialize();
  ImageRegionConstIteratorWithIndex<RealImageType>
  It( fieldEstimate, fieldEstimate->GetRequestedRegion() );

  unsigned int index = 0;
  for ( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    if( ( !this->GetMaskImage() ||
          this->GetMaskImage()->GetPixel( It.GetIndex() ) == this->m_MaskLabel )
        && ( !this->GetConfidenceImage() ||
             this->GetConfidenceImage()->GetPixel( It.GetIndex() ) > 0.0 ) )
      {
      PointType point;
      fieldEstimate->TransformIndexToPhysicalPoint( It.GetIndex(), point );

      ScalarType scalar;
      scalar[0] = It.Get();

      fieldPoints->SetPointData( index, scalar );
      fieldPoints->SetPoint( index, point );

      RealType confidenceWeight = 1.0;
      if( this->GetConfidenceImage() )
        {
        confidenceWeight =
          this->GetConfidenceImage()->GetPixel( It.GetIndex() );
        }
      weights->InsertElement( index, confidenceWeight );
      index++;
      }
    }
  fieldEstimate->SetDirection( direction );

  typename BSplineFilterType::Pointer bspliner = BSplineFilterType::New();

  typename BSplineFilterType::ArrayType numberOfControlPoints;
  typename BSplineFilterType::ArrayType numberOfFittingLevels;
  numberOfFittingLevels.Fill( 1 );
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    if( !this->m_LogBiasFieldControlPointLattice )
      {
      numberOfControlPoints[d] = this->m_NumberOfControlPoints[d];
      }
    else
      {
      numberOfControlPoints[d] = this->m_LogBiasFieldControlPointLattice->
        GetLargestPossibleRegion().GetSize()[d];
      }
    }

  typename ScalarImageType::PointType parametricOrigin =
    fieldEstimate->GetOrigin();
  for( unsigned int d = 0; d < ImageDimension; d++ )
    {
    parametricOrigin[d] += (
        fieldEstimate->GetSpacing()[d] *
        fieldEstimate->GetLargestPossibleRegion().GetIndex()[d] );
    }
  bspliner->SetOrigin( parametricOrigin );
  bspliner->SetSpacing( fieldEstimate->GetSpacing() );
  bspliner->SetSize( fieldEstimate->GetLargestPossibleRegion().GetSize() );
  bspliner->SetDirection( fieldEstimate->GetDirection() );
  bspliner->SetGenerateOutputImage( false );
  bspliner->SetNumberOfLevels( numberOfFittingLevels );
  bspliner->SetSplineOrder( this->m_SplineOrder );
  bspliner->SetNumberOfControlPoints( numberOfControlPoints );
  bspliner->SetInput( fieldPoints );
  bspliner->SetPointWeights( weights );
  bspliner->Update();

  // Add the bias field control points to the current estimate.

  if( !this->m_LogBiasFieldControlPointLattice )
    {
    this->m_LogBiasFieldControlPointLattice = bspliner->GetPhiLattice();
    }
  else
    {
    typedef AddImageFilter<BiasFieldControlPointLatticeType,
                           BiasFieldControlPointLatticeType,
                           BiasFieldControlPointLatticeType>
    AdderType;
    typename AdderType::Pointer adder = AdderType::New();
    adder->SetInput1( this->m_LogBiasFieldControlPointLattice );
    adder->SetInput2( bspliner->GetPhiLattice() );
    adder->Update();

    this->m_LogBiasFieldControlPointLattice = adder->GetOutput();
    }

  typedef BSplineControlPointImageFilter
  <BiasFieldControlPointLatticeType, ScalarImageType> BSplineReconstructerType;
  typename BSplineReconstructerType::Pointer reconstructer =
    BSplineReconstructerType::New();
  reconstructer->SetInput( this->m_LogBiasFieldControlPointLattice );
  reconstructer->SetOrigin( fieldEstimate->GetOrigin() );
  reconstructer->SetSpacing( fieldEstimate->GetSpacing() );
  reconstructer->SetDirection( direction );
  reconstructer->SetSize( fieldEstimate->GetLargestPossibleRegion().GetSize() );
  reconstructer->Update();

  const InputImageType * inputImage = this->GetInput();

  typedef VectorIndexSelectionCastImageFilter<ScalarImageType, RealImageType>
  SelectorType;
  typename SelectorType::Pointer selector = SelectorType::New();
  selector->SetInput( reconstructer->GetOutput() );
  selector->SetIndex( 0 );
  selector->Update();
  selector->GetOutput()->SetRegions( inputImage->GetRequestedRegion() );

  RealImagePointer smoothField = selector->GetOutput();
  smoothField->Update();
  smoothField->DisconnectPipeline();
  smoothField->SetRegions( inputImage->GetRequestedRegion() );

  return smoothField;
}

template<class TInputImage, class TMaskImage, class TOutputImage>
typename
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>::RealType
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::CalculateConvergenceMeasurement( const RealImageType *fieldEstimate1,
                                   const RealImageType *fieldEstimate2 ) const
{
  typedef SubtractImageFilter<RealImageType, RealImageType, RealImageType>
  SubtracterType;
  typename SubtracterType::Pointer subtracter = SubtracterType::New();
  subtracter->SetInput1( fieldEstimate1 );
  subtracter->SetInput2( fieldEstimate2 );
  subtracter->Update();


  // Calculate statistics over the mask region

  RealType mu = 0.0;
  RealType sigma = 0.0;
  RealType N = 0.0;

  ImageRegionConstIteratorWithIndex<RealImageType> It(
    subtracter->GetOutput(),
    subtracter->GetOutput()->GetLargestPossibleRegion() );

  for( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    if( ( !this->GetMaskImage() ||
          this->GetMaskImage()->GetPixel( It.GetIndex() ) == this->m_MaskLabel )
        && ( !this->GetConfidenceImage() ||
             this->GetConfidenceImage()->GetPixel( It.GetIndex() ) > 0.0 ) )
      {
      RealType pixel = vcl_exp( It.Get() );
      N += 1.0;

      if( N > 1.0 )
        {
        sigma = sigma + vnl_math_sqr( pixel - mu ) * ( N - 1.0 ) / N;
        }
      mu = mu * ( 1.0 - 1.0 / N ) + pixel / N;
      }
    }
  sigma = vcl_sqrt( sigma / ( N - 1.0 ) );

  return ( sigma / mu );
}

template<class TInputImage, class TMaskImage, class TOutputImage>
void
N4MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::PrintSelf(std::ostream &os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "Mask label: " << this->m_MaskLabel << std::endl;
  os << indent << "Number of histogram bins: "
     << this->m_NumberOfHistogramBins << std::endl;
  os << indent << "Weiner filter noise: "
     << this->m_WeinerFilterNoise << std::endl;
  os << indent << "Bias field FWHM: "
     << this->m_BiasFieldFullWidthAtHalfMaximum << std::endl;
  os << indent << "Maximum number of iterations: "
     << this->m_MaximumNumberOfIterations << std::endl;
  os << indent << "Convergence threshold: "
     << this->m_ConvergenceThreshold << std::endl;
  os << indent << "Spline order: " << this->m_SplineOrder << std::endl;
  os << indent << "Number of fitting levels: "
     << this->m_NumberOfFittingLevels << std::endl;
  os << indent << "Number of control points: "
     << this->m_NumberOfControlPoints << std::endl;
  os << indent << "CurrentConvergenceMeasurement: "
     << this->m_CurrentConvergenceMeasurement << std::endl;
  os << indent << "CurrentLevel: " << this->m_CurrentLevel << std::endl;
  os << indent << "ElapsedIterations: "
     << this->m_ElapsedIterations << std::endl;
  os << indent << "LogBiasFieldControlPointLattice: "
     << this->m_LogBiasFieldControlPointLattice << std::endl;
}

} // end namespace itk

#endif
