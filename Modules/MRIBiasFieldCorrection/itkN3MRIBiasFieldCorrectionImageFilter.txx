/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkN3MRIBiasFieldCorrectionImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2009/04/29 19:05:28 $
  Version:   $Revision: 1.1 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkN3MRIBiasFieldCorrectionImageFilter_txx
#define __itkN3MRIBiasFieldCorrectionImageFilter_txx

#include "itkN3MRIBiasFieldCorrectionImageFilter.h"

#include "itkDivideImageFilter.h"
#include "itkExpImageFilter.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkImageRegionIterator.h"
#include "itkLabelStatisticsImageFilter.h"
#include "itkLogImageFilter.h"
#include "itkOtsuThresholdImageFilter.h"
#include "itkSubtractImageFilter.h"

#include "vnl/algo/vnl_fft_1d.h"
#include "vnl/vnl_complex_traits.h"
#include "vxl/vcl/vcl_complex.h"

namespace itk {

template <class TInputImage, class TMaskImage, class TOutputImage>
N3MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::N3MRIBiasFieldCorrectionImageFilter()
{
  this->SetNumberOfRequiredInputs( 2 );
 
  this->m_MaskLabel = NumericTraits<MaskPixelType>::One;
  
  this->m_NumberOfHistogramBins = 200;
  this->m_WeinerFilterNoise = 0.01;
  this->m_BiasFieldFullWidthAtHalfMaximum = 0.15;
  
  this->m_MaximumNumberOfIterations = 50;
  this->m_ConvergenceThreshold = 0.001;

  this->m_SplineOrder = 3;
  this->m_NumberOfFittingLevels.Fill( 4 );
  this->m_NumberOfControlPoints.Fill( 4 );
}

template<class TInputImage, class TMaskImage, class TOutputImage>
void
N3MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::GenerateData()
{
  /**
   * Calculate the log of the input image.
   */
  typename RealImageType::Pointer logInputImage = RealImageType::New();
   
  typedef LogImageFilter<InputImageType, RealImageType> LogFilterType;
  typename LogFilterType::Pointer logFilter = LogFilterType::New();
  logFilter->SetInput( this->GetInput() );
  logFilter->Update();
  logInputImage = logFilter->GetOutput();

  /**
   * Remove possible nans/infs from the log input image.
   */
  ImageRegionIterator<RealImageType> It( logInputImage,
    logInputImage->GetLargestPossibleRegion() );
  ImageRegionConstIterator<MaskImageType> ItM( this->GetMaskImage(),
    this->GetMaskImage()->GetLargestPossibleRegion() );
  for( It.GoToBegin(), ItM.GoToBegin(); !It.IsAtEnd(); ++It, ++ItM )
    {
    if( ItM.Get() == this->m_MaskLabel )
      { 
      if( vnl_math_isnan( It.Get() ) || vnl_math_isinf( It.Get() ) 
        || It.Get() < 0.0 )
        {
        It.Set( 0.0 ); 
        }
      }
    else
      {
      It.Set( 0.0 );
      }    
    }

  /**
   * Provide an initial log bias field of zeros 
   */  
  typename RealImageType::Pointer logBiasField = RealImageType::New();
  logBiasField->SetOrigin( this->GetInput()->GetOrigin() );
  logBiasField->SetRegions( this->GetInput()->GetRequestedRegion() );
  logBiasField->SetSpacing( this->GetInput()->GetSpacing() );
  logBiasField->SetDirection( this->GetInput()->GetDirection() );
  logBiasField->Allocate();
  logBiasField->FillBuffer( 0.0 );

  /**
   * Iterate until convergence or iterative exhaustion.
   */ 
  bool isConverged = false;
  unsigned int iteration = 0;  
  while( !isConverged && iteration++ < this->m_MaximumNumberOfIterations )
    {
    typedef SubtractImageFilter<RealImageType, RealImageType, RealImageType>
      SubtracterType; 
 
    typename SubtracterType::Pointer subtracter1 = SubtracterType::New();
    subtracter1->SetInput1( logInputImage );
    subtracter1->SetInput2( logBiasField );
    subtracter1->Update();

    typename RealImageType::Pointer sharpenedImage
      = this->SharpenImage( subtracter1->GetOutput() );

    typename SubtracterType::Pointer subtracter2 = SubtracterType::New();
    subtracter2->SetInput1( logInputImage );
    subtracter2->SetInput2( sharpenedImage );
    subtracter2->Update();
    
    typename RealImageType::Pointer newLogBiasField 
      = this->SmoothField( subtracter2->GetOutput() );

    RealType cv = this->CalculateConvergenceMeasurement( 
      logBiasField, newLogBiasField );
    isConverged = ( cv < this->m_ConvergenceThreshold );

    itkDebugMacro( "Iteration " << iteration << ": " 
      << " convergence criterion = " << cv );

    logBiasField = newLogBiasField;
    } 

  typedef ExpImageFilter<RealImageType, RealImageType> ExpImageFilterType;
  typename ExpImageFilterType::Pointer expFilter = ExpImageFilterType::New();
  expFilter->SetInput( logBiasField );
  expFilter->Update();

  /**
   * Divide the input image by the bias field to get the final image.
   */
  typedef DivideImageFilter<InputImageType, RealImageType, OutputImageType>
    DividerType;
  typename DividerType::Pointer divider = DividerType::New();
  divider->SetInput1( this->GetInput() );
  divider->SetInput2( expFilter->GetOutput() );
  divider->Update();
  
  this->SetNthOutput( 0, divider->GetOutput() );
}

template<class TInputImage, class TMaskImage, class TOutputImage>
typename N3MRIBiasFieldCorrectionImageFilter
  <TInputImage, TMaskImage, TOutputImage>::RealImageType::Pointer
N3MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::SharpenImage( typename RealImageType::Pointer unsharpenedImage )
{
  /**
   * Build the histogram for the uncorrected image.  Store copy 
   * in a vnl_vector to utilize vnl FFT routines.  Note that variables
   * in real space are denoted by a single uppercase letter whereas their
   * frequency counterparts are indicated by a trailing lowercase 'f'.
   */   
  RealType binMaximum = NumericTraits<RealType>::NonpositiveMin();
  RealType binMinimum = NumericTraits<RealType>::max();
    
  ImageRegionIterator<RealImageType> ItU( unsharpenedImage,
    unsharpenedImage->GetLargestPossibleRegion() );
  ImageRegionConstIterator<MaskImageType> ItM( this->GetMaskImage(),
    this->GetMaskImage()->GetLargestPossibleRegion() );
  for( ItU.GoToBegin(), ItM.GoToBegin(); !ItU.IsAtEnd(); ++ItU, ++ItM )
    {
    if( ItM.Get() == this->m_MaskLabel )
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

  typedef LabelStatisticsImageFilter<RealImageType, MaskImageType> 
    StatisticsFilterType;
  typename StatisticsFilterType::Pointer stats = StatisticsFilterType::New();
  stats->SetInput( unsharpenedImage );
  stats->SetLabelInput( const_cast<MaskImageType*>( 
    static_cast<const MaskImageType*>( this->ProcessObject::GetInput( 1 ) ) ) );
  stats->SetUseHistograms( true );
  stats->SetHistogramParameters( this->m_NumberOfHistogramBins, binMinimum,
    binMaximum );
  stats->Update();

  /**
   * Determine information about the intensity histogram and zero-pad 
   * histogram to a power of 2.
   */

  RealType exponent = vcl_ceil( vcl_log( static_cast<RealType>(
    this->m_NumberOfHistogramBins ) ) / vcl_log( 2.0 ) ) + 1;
  unsigned int paddedHistogramSize = static_cast<unsigned int>( 
    vcl_pow( static_cast<RealType>( 2.0 ), exponent ) + 0.5 );
  unsigned int histogramOffset = static_cast<unsigned int>( 0.5 * 
    ( paddedHistogramSize - this->m_NumberOfHistogramBins ) );
  RealType histogramSlope = ( binMaximum - binMinimum ) / 
    static_cast<RealType>( this->m_NumberOfHistogramBins );

  vnl_vector< vcl_complex<RealType> > V( paddedHistogramSize, 
    vcl_complex<RealType>( 0.0, 0.0 ) );
  for( unsigned int n = 0; n < this->m_NumberOfHistogramBins; n++ )
    {
    V[n+histogramOffset] = vcl_complex<RealType>( 
      stats->GetHistogram( this->m_MaskLabel )->GetFrequency( n, 0 ), 0.0 );    
    }

  /**
   * Instantiate the 1-d vnl fft routine
   */ 
  vnl_fft_1d<RealType> fft( paddedHistogramSize );
    
  vnl_vector< vcl_complex<RealType> > Vf( V );
  fft.fwd_transform( Vf );

  /**
   * Create the Gaussian filter.
   */
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
    F[n] = F[paddedHistogramSize - n] = vcl_complex<RealType>( 
      scaleFactor * vcl_exp( -vnl_math_sqr( static_cast<RealType>( n ) ) 
      * expFactor ), 0.0 );
    }
  if( paddedHistogramSize % 2 == 0 )
    {
    F[halfSize] = vcl_complex<RealType>( scaleFactor * vcl_exp( 0.25 *
      -vnl_math_sqr( static_cast<RealType>( paddedHistogramSize ) ) 
      * expFactor ), 0.0 );
    }

  vnl_vector< vcl_complex<RealType> > Ff( F );
  fft.fwd_transform( Ff );

  /**
   * Create the Weiner deconvolution filter.
   */  
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
    U[n] = vcl_complex<RealType>( vnl_math_max( 
      U[n].real(), static_cast<RealType>( 0.0 ) ), 0.0 );
    }

  /**
   * Compute mapping E(u|v)
   */
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
    E[n] = numerator[n].real() / denominator[n].real();
    if( vnl_math_isinf( E[n] ) || vnl_math_isnan( E[n] ) )
      {
      E[n] = 0.0;
      }
    }
    
  /**
   * Remove the zero-padding from the mapping
   */  
  E = E.extract( this->m_NumberOfHistogramBins, histogramOffset );

  /**
   * Sharpen the image with the new mapping, E(u|v)
   */
  typename RealImageType::Pointer sharpenedImage = RealImageType::New();
  sharpenedImage->SetOrigin( unsharpenedImage->GetOrigin() );
  sharpenedImage->SetSpacing( unsharpenedImage->GetSpacing() );
  sharpenedImage->SetRegions( unsharpenedImage->GetLargestPossibleRegion() );
  sharpenedImage->SetDirection( unsharpenedImage->GetDirection() );
  sharpenedImage->Allocate();
  sharpenedImage->FillBuffer( 0.0 );
  
  ImageRegionIterator<RealImageType> ItC( sharpenedImage,
    sharpenedImage->GetLargestPossibleRegion() );
  
  ItU.GoToBegin();
  ItC.GoToBegin();
  ItM.GoToBegin();
  while( !ItM.IsAtEnd() )
    {
    if( ItM.Get() == this->m_MaskLabel )
      {
      RealType cidx = ( ItU.Get() - binMinimum ) / histogramSlope;
      unsigned int idx = static_cast<unsigned int>( cidx );

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
    ++ItU;
    ++ItC;
    ++ItM;  
    }
    
  return sharpenedImage;  
}

template<class TInputImage, class TMaskImage, class TOutputImage>
typename N3MRIBiasFieldCorrectionImageFilter
  <TInputImage, TMaskImage, TOutputImage>::RealImageType::Pointer
N3MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::SmoothField( typename RealImageType::Pointer fieldEstimate )
{
  /**
   * Get original direction and change to identity temporarily for the 
   * b-spline fitting.  
   */
  typename RealImageType::DirectionType direction 
    = fieldEstimate->GetDirection();
  typename RealImageType::DirectionType identity;
  identity.SetIdentity();
  fieldEstimate->SetDirection( identity );

  typename PointSetType::Pointer fieldPoints = PointSetType::New();
  fieldPoints->Initialize();

  ImageRegionConstIteratorWithIndex<RealImageType>
    It( fieldEstimate, fieldEstimate->GetRequestedRegion() );
  ImageRegionConstIteratorWithIndex<MaskImageType>
    ItM( this->GetMaskImage(), this->GetMaskImage()->GetRequestedRegion() );

  unsigned int N = 0;
  for ( It.GoToBegin(), ItM.GoToBegin(); !It.IsAtEnd(); ++It, ++ItM )
    {
    if( ItM.Get() == this->m_MaskLabel )
      { 
      typename PointSetType::PointType point;
      fieldEstimate->TransformIndexToPhysicalPoint( It.GetIndex(), point );
  
      ScalarType scalar;
      scalar[0] = It.Get();
  
      fieldPoints->SetPointData( N, scalar );
      fieldPoints->SetPoint( N, point );
      N++;
      }
    }
  fieldEstimate->SetDirection( direction );
  
  typename BSplineFilterType::Pointer bspliner = BSplineFilterType::New();
  bspliner->SetOrigin( fieldEstimate->GetOrigin() );
  bspliner->SetSpacing( fieldEstimate->GetSpacing() );
  bspliner->SetSize( fieldEstimate->GetLargestPossibleRegion().GetSize() );
  bspliner->SetDirection( fieldEstimate->GetDirection() );
  bspliner->SetGenerateOutputImage( true );
  bspliner->SetNumberOfLevels( this->m_NumberOfFittingLevels );
  bspliner->SetSplineOrder( this->m_SplineOrder );
  bspliner->SetNumberOfControlPoints( this->m_NumberOfControlPoints );
  bspliner->SetInput( fieldPoints );
  bspliner->Update();

  /**
   * Save the bias field control points in case the user wants to 
   * reconstruct the bias field.
   */
  this->m_BiasFieldControlPointLattice = bspliner->GetPhiLattice();

  typename RealImageType::Pointer smoothField = RealImageType::New();
  smoothField->SetOrigin( fieldEstimate->GetOrigin() );
  smoothField->SetSpacing( fieldEstimate->GetSpacing() );
  smoothField->SetRegions( 
    fieldEstimate->GetLargestPossibleRegion().GetSize() );
  smoothField->SetDirection( direction );
  smoothField->Allocate();
  
  ImageRegionIterator<ScalarImageType> ItB( bspliner->GetOutput(),
    bspliner->GetOutput()->GetLargestPossibleRegion() );
  ImageRegionIterator<RealImageType> ItF( smoothField,
    smoothField->GetLargestPossibleRegion() );
  for( ItB.GoToBegin(), ItF.GoToBegin(); !ItB.IsAtEnd(); ++ItB, ++ItF )
    {
    ItF.Set( ItB.Get()[0] );
    }
    
  return smoothField;
}

template<class TInputImage, class TMaskImage, class TOutputImage>
typename N3MRIBiasFieldCorrectionImageFilter
  <TInputImage, TMaskImage, TOutputImage>::RealType
N3MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::CalculateConvergenceMeasurement( typename RealImageType::Pointer 
  fieldEstimate1, typename RealImageType::Pointer fieldEstimate2 )
{
  typedef SubtractImageFilter<RealImageType, RealImageType, RealImageType>
    SubtracterType;
  typename SubtracterType::Pointer subtracter = SubtracterType::New();
  subtracter->SetInput1( fieldEstimate1 );
  subtracter->SetInput2( fieldEstimate2 );
  subtracter->Update();

  /**
   * Calculate statistics over the mask region
   */
  typedef LabelStatisticsImageFilter<RealImageType, MaskImageType> 
    StatisticsFilterType;
  typename StatisticsFilterType::Pointer stats = StatisticsFilterType::New();
  stats->SetInput( subtracter->GetOutput() );
  stats->SetLabelInput( const_cast<MaskImageType*>( 
    static_cast<const MaskImageType*>( this->ProcessObject::GetInput( 1 ) ) ) );
  stats->SetUseHistograms( false );
  stats->Update();

  /**
   * Although Sled's paper proposes convergence determination via 
   * the coefficient of variation, the actual mnc implementation 
   * utilizes the standard deviation as the convergence measurement. 
   */
  return stats->GetSigma( this->m_MaskLabel );
}

template<class TInputImage, class TMaskImage, class TOutputImage>
void
N3MRIBiasFieldCorrectionImageFilter<TInputImage, TMaskImage, TOutputImage>
::PrintSelf(std::ostream &os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  
  os << indent << "Mask label: " 
     << this->m_MaskLabel << std::endl;
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
  os << indent << "Spline order: " 
     << this->m_SplineOrder << std::endl;
  os << indent << "Number of fitting levels: " 
     << this->m_NumberOfFittingLevels << std::endl;
  os << indent << "Number of control points: " 
     << this->m_NumberOfControlPoints << std::endl;
}

}// end namespace itk
#endif
