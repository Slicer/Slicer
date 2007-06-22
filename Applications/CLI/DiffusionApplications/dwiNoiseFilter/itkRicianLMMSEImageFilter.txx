/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkRicianLMMSEImageFilter.txx,v $
  Language:  C++
  Date:      $Date: 2006/01/11 19:43:31 $
  Version:   $Revision: 1.14 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _itkRicianLMMSEImageFilter_txx
#define _itkRicianLMMSEImageFilter_txx
#include "itkRicianLMMSEImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkNeighborhoodInnerProduct.h"
#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include "itkOffset.h"
#include "itkProgressReporter.h"

#include "itkMaskedMeanImageFilter.h"
#include "itkExtractVolumeFilter.h"

#include <itkThresholdImageFilter.h>
#include <itkThresholdLabelerImageFilter.h>
#include <itkLabelStatisticsImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>

#include <itkAndImageFilter.h>

#include <math.h>

#include <iostream>
#include <fstream>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.141516
#endif

using namespace std;

namespace itk
{

template <class TInputImage, class TOutputImage>
RicianLMMSEImageFilter<TInputImage, TOutputImage>
::RicianLMMSEImageFilter()
{

  m_RadiusEstimation.Fill(1);
  m_RadiusFiltering.Fill(1);
  m_NoiseSTD = 75.0;
  m_Iterations = 5;
  m_MinimumNumberOfUsedVoxelsEstimation = 1;
  m_MinimumNumberOfUsedVoxelsFiltering = 1;
  m_FirstBaseline = 0;
  m_UseAbsoluteValue = false;
  m_WriteHistogram = false;
  m_MinimumNoiseSTD = 0;
  m_MaximumNoiseSTD = 10000;

  m_HistogramResolutionFactor = 2.0;

}

template <class TInputImage, class TOutputImage>
void
RicianLMMSEImageFilter<TInputImage, TOutputImage>
::SetupComputationMask() 
{
  // set up everything for the static label used for computing the statistics

  // computed the masked mean and variance of the masked mean
  // first create a labelmap that contains all voxels that are different from zero

  typedef itk::ExtractVolumeFilter< TInputImage,ScalarImageType > ExtractInputVolumeFilterType;

  typename ExtractInputVolumeFilterType::Pointer eviFilter = ExtractInputVolumeFilterType::New();
  
  eviFilter->SetInput( this->GetInput() );
  eviFilter->SetVolumeNr( m_FirstBaseline );
  eviFilter->Update();

  zeroMaskImageFilter = ThresholdImageFilterType::New();
  zeroMaskImageFilter->SetInput( eviFilter->GetOutput() ); // do it from the original volume to save a little bit of memory, could also use maskedMeanImageFilter, because zero stays zero there
  zeroMaskImageFilter->ThresholdOutside(0,0);
  zeroMaskImageFilter->SetOutsideValue( 1 );

}

template <class TInputImage, class TOutputImage>
void 
RicianLMMSEImageFilter<TInputImage, TOutputImage>
::GenerateOutputInformation() 
{
  Superclass::GenerateOutputInformation();

  this->GetOutput()->SetRegions( this->GetInput()->GetLargestPossibleRegion() );
  this->GetOutput()->SetLargestPossibleRegion( this->GetInput()->GetLargestPossibleRegion() );
  this->GetOutput()->SetNumberOfComponentsPerPixel( this->GetInput()->GetNumberOfComponentsPerPixel() );
  this->GetOutput()->Allocate();

}

template <class TInputImage, class TOutputImage>
void 
RicianLMMSEImageFilter<TInputImage, TOutputImage>
::GenerateInputRequestedRegion() throw (InvalidRequestedRegionError)
{
  // call the superclass' implementation of this method
  Superclass::GenerateInputRequestedRegion();
  
  // get pointers to the input and output
  typename Superclass::InputImagePointer inputPtr = 
    const_cast< TInputImage * >( this->GetInput() );
  typename Superclass::OutputImagePointer outputPtr = this->GetOutput();
  
  if ( !inputPtr || !outputPtr )
    {
    return;
    }

  // get a copy of the input requested region (should equal the output
  // requested region)
  typename TInputImage::RegionType inputRequestedRegion;
  inputRequestedRegion = inputPtr->GetRequestedRegion();

  // pad the input requested region by the operator radius
  inputRequestedRegion.PadByRadius( m_RadiusFiltering );

  // crop the input requested region at the input's largest possible region
  if ( inputRequestedRegion.Crop(inputPtr->GetLargestPossibleRegion()) )
    {
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    return;
    }
  else
    {
    // Couldn't crop the region (requested region is outside the largest
    // possible region).  Throw an exception.

    // store what we tried to request (prior to trying to crop)
    inputPtr->SetRequestedRegion( inputRequestedRegion );
    
    // build an exception
    InvalidRequestedRegionError e(__FILE__, __LINE__);
    e.SetLocation(ITK_LOCATION);
    e.SetDescription("Requested region is (at least partially) outside the largest possible region.");
    e.SetDataObject(inputPtr);
    throw e;
    }
}

template <class TInputImage, class TOutputImage>
double
RicianLMMSEImageFilter< TInputImage, TOutputImage >
::EstimateNoiseSTD( OutputPointer pImage, int iIter ) {

  typedef itk::ExtractVolumeFilter< TOutputImage,ScalarImageType> ExtractVolumeFilterType;

  typename ExtractVolumeFilterType::Pointer evFilter = ExtractVolumeFilterType::New();
  
  evFilter->SetInput( pImage );
  evFilter->SetVolumeNr( m_FirstBaseline );
  evFilter->Update();

  // now we compute the mean image for this

  typedef itk::MaskedMeanImageFilter<ScalarImageType, ScalarDoubleImageType> MaskedMeanImageFilterType;

  typename MaskedMeanImageFilterType::Pointer maskedMeanImageFilter = MaskedMeanImageFilterType::New();
  maskedMeanImageFilter->SetInput( evFilter->GetOutput() );
  maskedMeanImageFilter->SetMinimumNumberOfUsedVoxels( m_MinimumNumberOfUsedVoxelsEstimation );
  maskedMeanImageFilter->SetRadius( m_RadiusEstimation );


  // now generate the restricted mask, based on the mean of the filtered values and standard deviations
  
  typedef itk::LabelStatisticsImageFilter< ScalarDoubleImageType, ScalarImageType > LabelStatisticsImageFilterType;
  
  typename LabelStatisticsImageFilterType::Pointer labelStatisticsImageFilter = LabelStatisticsImageFilterType::New();
  labelStatisticsImageFilter->SetInput( maskedMeanImageFilter->GetOutput() );
  labelStatisticsImageFilter->SetLabelInput( zeroMaskImageFilter->GetOutput() );
  labelStatisticsImageFilter->Update();
  double dMean = labelStatisticsImageFilter->GetMean( 1 );
  double dVariance = labelStatisticsImageFilter->GetVariance( 1 );
  double dSTD = sqrt(dVariance);
  double dMin = labelStatisticsImageFilter->GetMinimum( 1 );
  double dMax = labelStatisticsImageFilter->GetMaximum( 1 );

  std::cout << "mean = " << dMean << std::endl;
  std::cout << "variance = " << dVariance << std::endl;
  std::cout << "standard deviation = " << dSTD << std::endl;
  std::cout << "min = " << dMin << std::endl;
  std::cout << "max = " << dMax << std::endl;

  // now set up the range we are interested in

  double dSigmaFac = 2.0;

  double dDesiredMin = dMean-dSigmaFac*dSTD-0.5;
  if ( dDesiredMin<dMin ) dDesiredMin = dMin-0.5;
  if ( dDesiredMin<=0 ) dDesiredMin = 0;

  double dDesiredMax = dMean+dSigmaFac*dSTD;
  if ( dDesiredMax>dMax ) dDesiredMax = dMax;
  
  // and extract values only within this range

  typedef itk::BinaryThresholdImageFilter<ScalarDoubleImageType,ScalarImageType> BinaryThresholdImageFilterType;

  typename BinaryThresholdImageFilterType::Pointer binaryRestrictedMask = BinaryThresholdImageFilterType::New();
  //binaryRestrictedMask->SetInput( maskRestrictedFilter->GetOutput() );
  binaryRestrictedMask->SetInput( maskedMeanImageFilter->GetOutput() );
  binaryRestrictedMask->SetLowerThreshold( dDesiredMin );
  binaryRestrictedMask->SetUpperThreshold( dDesiredMax );
  binaryRestrictedMask->SetInsideValue( 1 );
  binaryRestrictedMask->SetOutsideValue( 0 );

  typedef itk::AndImageFilter<ScalarImageType,ScalarImageType,ScalarImageType> AndImageFilterType;

  typename AndImageFilterType::Pointer combinedMaskFilter = AndImageFilterType::New();
  //combinedMaskFilter->SetInput1( maskRestrictedFilter->GetOutput() );
  combinedMaskFilter->SetInput1( binaryRestrictedMask->GetOutput() );
  combinedMaskFilter->SetInput2( zeroMaskImageFilter->GetOutput() );

  typedef itk::LabelStatisticsImageFilter< ScalarDoubleImageType, ScalarImageType > LabelStatisticsImageFilterType;

  typename LabelStatisticsImageFilterType::Pointer labelStatisticsImageFilterRestricted = LabelStatisticsImageFilterType::New();
  labelStatisticsImageFilterRestricted->SetInput( maskedMeanImageFilter->GetOutput() );
  labelStatisticsImageFilterRestricted->SetLabelInput( combinedMaskFilter->GetOutput() );

  dLowerBound = dDesiredMin;
  dUpperBound = dDesiredMax;

  iNumBins = (int) ( (dUpperBound-dLowerBound)*m_HistogramResolutionFactor );
  
  std::cout << "number of bins = " << iNumBins << std::endl;

  std::cout << "Histogram settings: iNumBins = " << iNumBins << "  dLowerBound = " << dLowerBound << "  dUpperBound = " << dUpperBound << std::endl;

  labelStatisticsImageFilterRestricted->UseHistogramsOn();
  labelStatisticsImageFilterRestricted->SetHistogramParameters( iNumBins, dLowerBound, dUpperBound);
  labelStatisticsImageFilterRestricted->Update();
  double dMeanR = labelStatisticsImageFilterRestricted->GetMean( 1 );
  double dVarianceR = labelStatisticsImageFilterRestricted->GetVariance( 1 );
  double dSTDR = sqrt(dVarianceR);
  double dMinR = labelStatisticsImageFilterRestricted->GetMinimum( 1 );
  double dMaxR = labelStatisticsImageFilterRestricted->GetMaximum( 1 );

  std::cout << "Restricted values:" << std::endl;
  std::cout << "mean = " << dMeanR << std::endl;
  std::cout << "variance = " << dVarianceR << std::endl;
  std::cout << "standard deviation = " << dSTDR << std::endl;
  std::cout << "min = " << dMinR << std::endl;
  std::cout << "max = " << dMaxR << std::endl;

  // get the histogram

  typedef typename LabelStatisticsImageFilterType::HistogramPointer HistogramPointer;
  typedef typename LabelStatisticsImageFilterType::HistogramType HistogramType;

  HistogramPointer hp =    labelStatisticsImageFilterRestricted->GetHistogram( 1 );

  // iterate through this thing

  unsigned int iSize = (hp->GetSize())[0]; // this is a one-dimensional histogram

  int iCurrentMaxIndex = 0;
  int iCurrentMaxFrequency = 0;

  ofstream outputStream;
  char currentFilename[256];

  if ( m_WriteHistogram ) {
    sprintf( currentFilename, "hist_%d.dat", iIter );
    outputStream.open( currentFilename );
  }  

  for ( unsigned int iI=0; iI<iSize; iI++ ) {
  //for ( unsigned int iI=1; iI<iSize; iI++ ) { // don't look at the very smallest bin

    double dCurrentBinValue = (hp->GetMeasurementVector( iI ))[0];

    if ( hp->GetFrequency( iI )>iCurrentMaxFrequency && (dCurrentBinValue<=m_MaximumNoiseSTD) && (dCurrentBinValue>=m_MinimumNoiseSTD) ) {
      iCurrentMaxIndex = iI;
      iCurrentMaxFrequency = (int)hp->GetFrequency( iI );
    }

    if ( m_WriteHistogram ) {
      outputStream << hp->GetFrequency( iI ) << " " << (hp->GetMeasurementVector( iI ))[0] << std::endl;
    }

    //std::cout << "frequency = " << hp->GetFrequency( iI ) << std::endl;
  }

  if ( m_WriteHistogram ) {
    outputStream.close();
  }

  /*std::cout << "max frequency = " << iCurrentMaxFrequency << " at index = " << iCurrentMaxIndex << " which corresponds to = " << hp->GetMeasurementVector( iCurrentMaxIndex ) << std::endl;

  std::cout << "dLowerBound = " << dLowerBound << std::endl;
  std::cout << "dUpperBound = " << dUpperBound << std::endl;
  std::cout << "dStep = " << dStep << std::endl;*/

  double dNoiseSTD_Estimated = sqrt(2/M_PI)*((hp->GetMeasurementVector( iCurrentMaxIndex ))[0]);

  std::cout << "Estimated noise standard deviation is = " << dNoiseSTD_Estimated << std::endl; 


  /*std::cout << "meanR = " << dMeanR << std::endl;
  std::cout << "varianceR = " << dVarianceR << std::endl;
  std::cout << "standard deviationR = " << dSTDR << std::endl;
  std::cout << "minR = " << dMinR << std::endl;
  std::cout << "maxR = " << dMaxR << std::endl;*/

  
  return dNoiseSTD_Estimated;

}


template <class TInputImage, class TOutputImage>
void
RicianLMMSEImageFilter< TInputImage, TOutputImage>
::GenerateData()
{

  //this->PrintSelf(std::cout, 0 );


  typename  InputImageType::ConstPointer input  = this->GetInput();
  const int vDIMENSION = input->GetNumberOfComponentsPerPixel();

  typename TOutputImage::Pointer output = this->GetOutput();

  this->SetupComputationMask();

  // get vector dimension

  unsigned int i;
  ZeroFluxNeumannBoundaryCondition<OutputImageType> nbc;

  ConstNeighborhoodIterator<OutputImageType> bit;
  ImageRegionIterator<OutputImageType> it;
  ImageRegionConstIterator<InputImageType> ciit;

  // Find the data-set boundary "faces"
  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<OutputImageType>::FaceListType faceList;
  NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<OutputImageType> bC;
  //faceList = bC(input, outputRegionForThread, m_Radius);
  faceList = bC(output, output->GetRequestedRegion(), m_RadiusFiltering);

  typename NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<OutputImageType>::FaceListType::iterator fit;

  // support progress methods/callbacks
  //ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());
  ProgressReporter progress(this, 0, output->GetRequestedRegion().GetNumberOfPixels()*m_Iterations);
  
  //InputPixelType sum[vDIMENSION];
  double *sum = new double[vDIMENSION];

  double *dSecondAveragedMoment = new double[vDIMENSION];
  double *dFourthAveragedMoment = new double[vDIMENSION];
  double *dSquaredMagnitude = new double[vDIMENSION];

  int *iNumberOfUsedVoxels = new int[vDIMENSION];

  int iWriteIntoOutput = 1;   // toggles between the two states, so there is no extra copying

  // we need two images of the correct size that can be used to hold the information
  // one is the output image, and the other one is the temporary image

  typename TOutputImage::Pointer iTmp = TOutputImage::New();
  typename TOutputImage::RegionType region;
  region.SetSize( output->GetRequestedRegion().GetSize() );
  region.SetIndex( output->GetRequestedRegion().GetIndex() );

  iTmp->SetRegions( region );
  iTmp->SetVectorLength( vDIMENSION );
  //iTmp->SetNumberOfCompontentsPerPixel( vDIMENSION );
  iTmp->Allocate();

  // initially, copy the whole input to the output

  it = ImageRegionIterator<OutputImageType>(output,output->GetRequestedRegion() );
  ciit = ImageRegionConstIterator<InputImageType>(input,output->GetRequestedRegion() );

  it.GoToBegin();
  ciit.GoToBegin();

  while ( !it.IsAtEnd() && !ciit.IsAtEnd() ) {

    it.Set( static_cast<OutputPixelType>( ciit.Get() ) );   

    ++it;
    ++ciit;

  }

  // now loop for the number of iterations

  for ( int iI=0; iI<m_Iterations; iI++ ) {

    // estimating the noise level

    if ( iWriteIntoOutput ) {
      m_NoiseSTD = this->EstimateNoiseSTD( output, iI );
    } else {
      m_NoiseSTD = this->EstimateNoiseSTD( iTmp, iI );
    }

    double dNoiseVariance = m_NoiseSTD*m_NoiseSTD;

    std::cout << "Iterations " << iI << std::endl;

    // Process each of the boundary faces.  These are N-d regions which border
    // the edge of the buffer.
    for (fit=faceList.begin(); fit != faceList.end(); ++fit)
      { 

        if ( iWriteIntoOutput ) {
            bit = ConstNeighborhoodIterator<OutputImageType>(m_RadiusFiltering,
                            output, *fit);
      it = ImageRegionIterator<OutputImageType>(iTmp, *fit);
    } else {
      bit = ConstNeighborhoodIterator<OutputImageType>(m_RadiusFiltering,
                            iTmp, *fit);
      it = ImageRegionIterator<OutputImageType>(output, *fit);
    }

    unsigned int neighborhoodSize = bit.Size();
    bit.OverrideBoundaryCondition(&nbc);
    bit.GoToBegin();
    
    while ( ! bit.IsAtEnd() )
      {
        
        OutputPixelType dMagnitude = bit.GetCenterPixel();
        for ( int iJ=0; iJ<vDIMENSION; iJ++ ) {
          dSquaredMagnitude[iJ] = dMagnitude[iJ]*dMagnitude[iJ];
          dSecondAveragedMoment[iJ] = 0;
          dFourthAveragedMoment[iJ] = 0;
          iNumberOfUsedVoxels[iJ] = 0;
          sum[iJ] = 0;
        }

        for (i = 0; i < neighborhoodSize; ++i)
          {

        OutputPixelType currentPixelValue = bit.GetPixel(i);

        for ( int iJ=0; iJ<vDIMENSION; iJ++ ) {

          if ( currentPixelValue[iJ]>0 ) { // exactly zero indicates an artifical value filled in by the scanner, maybe make a flag for this test
            iNumberOfUsedVoxels[iJ]++;
            double dMagnitudeSquared = currentPixelValue[iJ]*currentPixelValue[iJ];
            dSecondAveragedMoment[iJ]+= dMagnitudeSquared;
            dFourthAveragedMoment[iJ]+= dMagnitudeSquared*dMagnitudeSquared;
          }

        }

          }
        
        OutputPixelType dFiltered;
        dFiltered.SetSize( vDIMENSION );
        
        for ( int iJ=0; iJ<vDIMENSION; iJ++ ) {

          if ( iNumberOfUsedVoxels[iJ]>=m_MinimumNumberOfUsedVoxelsFiltering && dMagnitude[iJ]>0 ) {
          
        dSecondAveragedMoment[iJ]/=iNumberOfUsedVoxels[iJ];
        dFourthAveragedMoment[iJ]/=iNumberOfUsedVoxels[iJ];
        
        double dDenominator = (dFourthAveragedMoment[iJ]-dSecondAveragedMoment[iJ]*dSecondAveragedMoment[iJ]);
        
        const double dAbsFactor = 1;

        if ( fabs(dDenominator)>dAbsFactor*std::numeric_limits<double>::epsilon() ) {

          double dGain = 1-(4*dNoiseVariance*(dSecondAveragedMoment[iJ]-dNoiseVariance))/dDenominator;
          if ( dGain<0 ) dGain = 0.0;

          double dFilteredSquared = dSecondAveragedMoment[iJ]-2*dNoiseVariance+dGain*(dSquaredMagnitude[iJ]-dSecondAveragedMoment[iJ]);

          if ( dFilteredSquared>=0 ) {
            dFiltered[iJ] = sqrt( dFilteredSquared );
          } else {
            if ( m_UseAbsoluteValue ) {
              dFiltered[iJ] = sqrt( -dFilteredSquared );
            } else {
              dFiltered[iJ] = dMagnitude[iJ];
            }

            //std::cout << "dFilteredSquared == " << dFilteredSquared << " dGain == " << dGain << " sam == " << dSecondAveragedMoment[iJ] << " 2*dNVar == " << 2*dNoiseVariance << std::endl;

            //dFiltered[iJ] = std::numeric_limits<double>::epsilon();
            //dFiltered[iJ] = 0;
          }
        } else {
          dFiltered[iJ] = dMagnitude[iJ];
        }
        
          } else {
        dFiltered[iJ] = dMagnitude[iJ];
          }
        
        }

        // get the mean value
        it.Set( static_cast<OutputPixelType>( dFiltered ) );       
        ++bit;
        ++it;
        progress.CompletedPixel();
      }
      }

    iWriteIntoOutput = 1-iWriteIntoOutput; // toggle the output

  } // end for m_Iterations
  
  // if there were zero iterations requested, simply copy the input to the output

  if ( m_Iterations==0 ) {
    std::cerr << "Warning: zero iterations requested in itkRicianLMMSEImageFilter.txx" << std::endl;
    std::cerr << "Input was copied to output." << std::endl;
  }

  if ( !iWriteIntoOutput ) { // the output is in iTmp and needs to be copied, otherwise it is already in the output memory

    ImageRegionConstIterator<OutputImageType> ifrom;

    ifrom = ImageRegionConstIterator<OutputImageType>(iTmp,output->GetRequestedRegion() );
    it = ImageRegionIterator<OutputImageType>(output,output->GetRequestedRegion() );

    it.GoToBegin();
    ifrom.GoToBegin();

    while ( !it.IsAtEnd() && ! ifrom.IsAtEnd() ) {

      it.Set( static_cast<OutputPixelType>( ifrom.Get() ) );   

      ++it;
      ++ifrom;

    }


  }

  // delete the temporary image
  delete[] sum; 
  delete[] dSecondAveragedMoment;
  delete[] dFourthAveragedMoment;
  delete[] dSquaredMagnitude;
  delete[] iNumberOfUsedVoxels;

}

/**
 * Standard "PrintSelf" method
 */
template <class TInputImage, class TOutput>
void
RicianLMMSEImageFilter<TInputImage, TOutput>
::PrintSelf(
  std::ostream& os, 
  Indent indent) const
{
  Superclass::PrintSelf( os, indent );
  os << indent << "Radius filtering: " << m_RadiusFiltering << std::endl;
  os << indent << "Radius estimation: " << m_RadiusEstimation << std::endl;
  os << indent << "Histogram resolution factor: " << m_HistogramResolutionFactor << std::endl;
  os << indent << "Noise STD: " << m_NoiseSTD << std::endl;
  os << indent << "Iterations: " << m_Iterations << std::endl;
  os << indent << "Minimum number of used voxels filtering: " << m_MinimumNumberOfUsedVoxelsFiltering << std::endl;
  os << indent << "Minimum number of used voxels estimation: " << m_MinimumNumberOfUsedVoxelsEstimation << std::endl;
  os << indent << "Write histogram: " << m_WriteHistogram << std::endl;
  os << indent << "Use absolute value: " << m_UseAbsoluteValue << std::endl;

}

} // end namespace itk

#endif
