/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkLMMSEVectorImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkLMMSEVectorImageFilter_txx
#define _itkLMMSEVectorImageFilter_txx
#include "itkLMMSEVectorImageFilter.h"
#ifndef M_PI
#define M_PI 3.141516
#endif

namespace itk
{

template <class TInputImage, class TOutputImage>
LMMSEVectorImageFilter<TInputImage, TOutputImage>
::LMMSEVectorImageFilter()
{

  m_RadiusEstimation.Fill( 1 );
  m_RadiusFiltering.Fill( 1 );
  m_Iterations = 5;
  m_MinimumNumberOfUsedVoxelsEstimation = 1;
  m_MinimumNumberOfUsedVoxelsFiltering = 1;
  m_FirstBaseline = 0;
  m_UseAbsoluteValue = false;
  m_KeepValue = false;
  m_MinimumNoiseSTD = 0;
  m_MaximumNoiseSTD = 10000;
  m_HistogramResolutionFactor = 2.0;
  m_MaximumNumberOfBins = 200000;
}

template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion()
throw (InvalidRequestedRegionError)
{
  // Call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  // Get pointer to the input
  InputImagePointer inputPtr = const_cast<TInputImage *>( this->GetInput() );
  inputPtr->SetRequestedRegion( inputPtr->GetLargestPossibleRegion() );
}

template <class TInputImage, class TOutputImage>
void LMMSEVectorImageFilter<TInputImage, TOutputImage>
::GenerateData()
{
  // Auxiliar filters:
  ExtractPointer extract   = ExtractType::New();;

  LocalMeanPointer  mean      = LocalMeanType::New();
  StatsPointer      stats     = StatsType::New();
  HistogramPointer  histogram = HistogramType::New();
  LMMSEStepPointer  lmmse     = LMMSEStepType::New();
  InputCastPointer  icast     = InputCastType::New();
  OutputCastPointer ocast     = OutputCastType::New();
  icast->SetInput( this->GetInput() );
  // For each iteration:
  for( unsigned int iter = 0; iter < m_Iterations; ++iter )
    {
    // Create a minipieline to compute local means and the noise:
    if( iter == 0 )
      {
      extract->SetInput( icast->GetOutput() );
      }
    else
      {
      extract->SetInput( lmmse->GetOutput() );
      }
    mean->SetInput( extract->GetOutput() );
    stats->SetInput( mean->GetOutput() );
    // Fix the corresponding parameters:
    extract->SetVolumeNr( m_FirstBaseline );
    mean->SetMinimumNumberOfUsedVoxels( m_MinimumNumberOfUsedVoxelsEstimation );
    mean->SetRadius( m_RadiusEstimation );
    // Update the pipeline:
    stats->Update();
    // Compute the statistics:
    if( !stats->GetReady() )
      {
      itkExceptionMacro( << "Could not compute the statistics!!!" );
      }
    double smean = stats->GetMean();
    double std   = stats->GetStd();
    double min   = stats->GetMin();
    double max   = stats->GetMax();
    // Confidence factor for the std:
    double SFac = 2.0;
    // Compute the limits in the histogram:
    double dMin = smean - SFac * std - 0.5;
    if( dMin < min )
      {
      dMin = min - 0.5;
      }
    if( dMin <= 0 )
      {
      dMin = 0;
      }
    double dMax = smean + SFac * std;
    if( dMax > max )
      {
      dMax = max;
      }
    // Create the minipipeline to compute the histogram:
    histogram->SetInput( stats->GetOutput() );
    histogram->SetMin( dMin );
    histogram->SetMax( dMax );
    unsigned long bins = (unsigned long)(   ( dMax - dMin) * m_HistogramResolutionFactor   );
    if( bins > m_MaximumNumberOfBins )
      {
      bins = m_MaximumNumberOfBins;
      }
    histogram->SetBins( bins );
    // Execute the pipeline:
    histogram->Update();
    // Compute the noise:
    double noiseStd  = histogram->GetMode();
    noiseStd        *= sqrt(2 / M_PI);
    if( noiseStd > m_MaximumNoiseSTD )
      {
      noiseStd = m_MaximumNoiseSTD;
      }
    if( noiseStd < m_MinimumNoiseSTD )
      {
      noiseStd = m_MinimumNoiseSTD;
      }
    if( iter == 0 )
      {
      lmmse->SetInput(  icast->GetOutput()  );
      }
    else
      {
      lmmse->SetInput(  lmmse->GetOutput()  );
      }
    // Set the parameters:
    lmmse->SetUseAbsoluteValue( m_UseAbsoluteValue );
    lmmse->SetKeepValue( m_KeepValue );
    lmmse->SetMinimumNumberOfUsedVoxelsFiltering( m_MinimumNumberOfUsedVoxelsFiltering );
    lmmse->SetNoiseVariance( noiseStd * noiseStd );
    lmmse->SetRadius( m_RadiusFiltering );
    lmmse->SetChannels( m_Channels );
    lmmse->Modified();
    lmmse->Update();
    }
  ocast->SetInput( lmmse->GetOutput() );
  ocast->Update();
  this->GraftOutput( ocast->GetOutput() );
  return;
}

/** Standard "PrintSelf" method */
template <class TInputImage, class TOutput>
void LMMSEVectorImageFilter<TInputImage, TOutput>
::PrintSelf( std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius filtering: " << m_RadiusFiltering << std::endl;
  os << indent << "Radius estimation: " << m_RadiusEstimation << std::endl;
  os << indent << "Histogram resolution factor: " << m_HistogramResolutionFactor << std::endl;
  os << indent << "Iterations: " << m_Iterations << std::endl;
  os << indent << "Minimum number of used voxels filtering: " << m_MinimumNumberOfUsedVoxelsFiltering << std::endl;
  os << indent << "Minimum number of used voxels estimation: " << m_MinimumNumberOfUsedVoxelsEstimation << std::endl;
  os << indent << "Use absolute value: " << m_UseAbsoluteValue << std::endl;
  os << indent << "Keep value: " << m_KeepValue << std::endl;

}

} // end namespace itk

#endif
