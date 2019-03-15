/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDifferenceDiffusionTensor3DImageFilter.txx,v $
  Language:  C++

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef itkDifferenceDiffusionTensor3DImageFilter_txx
#define itkDifferenceDiffusionTensor3DImageFilter_txx

#include "itkDifferenceDiffusionTensor3DImageFilter.h"

#include "itkConstNeighborhoodIterator.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include "itkNeighborhoodAlgorithm.h"
#include "itkProgressReporter.h"
#include "itkZeroFluxNeumannBoundaryCondition.h"
#include <iomanip>

namespace itk
{

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::DifferenceDiffusionTensor3DImageFilter()
{
  // We require two inputs to execute.
  this->SetNumberOfRequiredInputs(2);

  // Set the default DifferenceThreshold.
  m_DifferenceThreshold = NumericTraits<OutputPixelType>::ZeroValue();

  // Set the default ToleranceRadius.
  m_ToleranceRadius = 0;

  // Initialize statistics about difference image.
  m_MeanDifference = NumericTraits<RealType>::ZeroValue();
  m_TotalDifference = NumericTraits<AccumulateType>::ZeroValue();
  m_NumberOfPixelsWithDifferences = 0;
  m_IgnoreBoundaryPixels = false;
  m_MeasurementFrameValid.SetIdentity();
  m_MeasurementFrameTest.SetIdentity();

  // Keep using the ITKv4 threading system.
  // This class could be updated to use the ITKv5 dynamic threading system in the future
  // Check the ITK migration guide:
  // https://github.com/InsightSoftwareConsortium/ITK/blob/master/Documentation/ITK5MigrationGuide.md
  // This class in particular does use threadId, so a more complex solution is needed
  // Check the example of using std::atomic in the migration guide.
  this->DynamicMultiThreadingOff();
}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
void
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "ToleranceRadius: " << m_ToleranceRadius << "\n";
  os << indent << "DifferenceThreshold: " << m_DifferenceThreshold << "\n";
  os << indent << "MeanDifference: " << m_MeanDifference << "\n";
  os << indent << "TotalDifference: " << m_TotalDifference << "\n";
  os << indent << "NumberOfPixelsWithDifferences: "
     << m_NumberOfPixelsWithDifferences << "\n";
  os << indent << "IgnoreBoundaryPixels: "
     << m_IgnoreBoundaryPixels << "\n";
}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
void
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::SetValidInput(const InputImageType* validImage)
{
  // The valid image should be input 0.
  this->SetInput(0, validImage);
}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
void
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::SetTestInput(const InputImageType* testImage)
{
  // The test image should be input 1.
  this->SetInput(1, testImage);
}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
typename DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>::MatrixType
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::GetMetaDataDictionary( const InputImageType* image )
{
  const itk::MetaDataDictionary & dict = image->GetMetaDataDictionary();

  std::vector<std::string> keys = dict.GetKeys();
  MatrixType               measurementFrame;
  measurementFrame.SetIdentity();
  for( std::vector<std::string>::const_iterator it = keys.begin();
       it != keys.end(); ++it )
    {
    if( it->find("measurement frame") != std::string::npos )
      {
      DoubleVectorType tagvalue;
      itk::ExposeMetaData<DoubleVectorType>( dict, *it, tagvalue );
      for( int i = 0; i < 3; i++ )
        {
        for( int j = 0; j < 3; j++ )
          {
          measurementFrame[i][j] = tagvalue.at( j ).at( i );
          }
        }
      }
    }
  return measurementFrame;
}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
void
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::BeforeThreadedGenerateData()
{
  int numberOfThreads = this->GetNumberOfThreads();

  // Initialize statistics about difference image.
  m_MeanDifference = NumericTraits<RealType>::ZeroValue();
  m_TotalDifference = NumericTraits<AccumulateType>::ZeroValue();
  m_NumberOfPixelsWithDifferences = 0;

  // Resize the thread temporaries
  m_ThreadDifferenceSum.SetSize(numberOfThreads);
  m_ThreadNumberOfPixels.SetSize(numberOfThreads);

  // Initialize the temporaries
  m_ThreadDifferenceSum.Fill(NumericTraits<AccumulateType>::ZeroValue());
  m_ThreadNumberOfPixels.Fill(0);

  m_MeasurementFrameValid = GetMetaDataDictionary( this->GetInput(0) );
  m_MeasurementFrameTest = GetMetaDataDictionary( this->GetInput(1) );

}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
typename DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>::InputPixelType
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::ApplyMeasurementFrameToTensor( InputPixelType tensor, const MatrixType & measurementFrame )
{
  MatrixType matrixTensor;

  for( int i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      matrixTensor[i][j] = static_cast<double>( tensor(i, j ) );
      }
    }
  MatrixType transformedMatrix;
  transformedMatrix = measurementFrame * matrixTensor * measurementFrame.GetTranspose();
  for( int i = 0; i < 3; i++ )
    {
    for( int j = 0; j < 3; j++ )
      {
      tensor(i, j ) = static_cast<typename InputPixelType::RealValueType>(transformedMatrix[i][j] );
      }
    }
  return tensor;
}

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
void
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::ThreadedGenerateData(const OutputImageRegionType &threadRegion, ThreadIdType threadId)
  {
  typedef ConstNeighborhoodIterator<InputImageType> SmartIterator;
  typedef ImageRegionConstIterator<InputImageType>  InputIterator;
  typedef ImageRegionIterator<OutputImageType>      OutputIterator;

  typedef NeighborhoodAlgorithm::ImageBoundaryFacesCalculator<InputImageType>
  FacesCalculator;

  typedef typename FacesCalculator::RadiusType   RadiusType;
  typedef typename FacesCalculator::FaceListType FaceListType;
  typedef typename FaceListType::iterator        FaceListIterator;
  typedef typename InputImageType::PixelType     InputPixelType;

  // Prepare standard boundary condition.
  ZeroFluxNeumannBoundaryCondition<InputImageType> nbc;

  // Get a pointer to each image.
  const InputImageType* validImage = this->GetInput(0);
  const InputImageType* testImage = this->GetInput(1);
  OutputImageType*      outputPtr = this->GetOutput();

  // Create a radius of pixels.
  RadiusType radius;
  if( m_ToleranceRadius > 0 )
    {
    radius.Fill(m_ToleranceRadius);
    }
  else
    {
    radius.Fill(0);
    }

  // Find the data-set boundary faces.
  FacesCalculator boundaryCalculator;
  FaceListType    faceList = boundaryCalculator(testImage, threadRegion, radius);

  // Support progress methods/callbacks.
  ProgressReporter progress(this, threadId, threadRegion.GetNumberOfPixels() );
  // Process the internal face and each of the boundary faces.
  for( FaceListIterator face = faceList.begin(); face != faceList.end(); ++face )
    {
    SmartIterator  test(radius, testImage, *face); // Iterate over test image.
    InputIterator  valid(validImage, *face);       // Iterate over valid image.
    OutputIterator out(outputPtr, *face);          // Iterate over output image.
    if( !test.GetNeedToUseBoundaryCondition() || !m_IgnoreBoundaryPixels )
      {
      test.OverrideBoundaryCondition(&nbc);
      for( valid.GoToBegin(), test.GoToBegin(), out.GoToBegin();
           !valid.IsAtEnd();
           ++valid, ++test, ++out )
        {
        // Get the current valid pixel.

        InputPixelType t = ApplyMeasurementFrameToTensor( valid.Get(), m_MeasurementFrameValid );

        //  Assume a good match - so test center pixel first, for speed
        typename InputPixelType::Iterator it;
        typename InputPixelType::Iterator ittest;
        RealType       sumdifference = NumericTraits<RealType>::ZeroValue();
        InputPixelType centerTensor = ApplyMeasurementFrameToTensor( test.GetCenterPixel(), m_MeasurementFrameTest );
        for( it = t.Begin(), ittest = centerTensor.Begin(); it != t.End(); ++it, ++ittest )
          {
          RealType difference = static_cast<RealType>( (*it) ) - (*ittest);
          if( NumericTraits<RealType>::IsNegative( difference ) )
            {
            difference = -difference;
            }
          sumdifference += difference;
          }
        OutputPixelType minimumDifference = static_cast<OutputPixelType>(sumdifference);
        // If center pixel isn't good enough, then test the neighborhood
        if( minimumDifference > m_DifferenceThreshold )
          {
          unsigned int neighborhoodSize = test.Size();
          // Find the closest-valued pixel in the neighborhood of the test
          // image.
          for( unsigned int i = 0; i < neighborhoodSize; ++i )
            {
            // Use the RealType for the difference to make sure we get the
            // sign.
            sumdifference = NumericTraits<RealType>::ZeroValue();
            InputPixelType tensor = ApplyMeasurementFrameToTensor( test.GetPixel(i), m_MeasurementFrameTest );
            for( it = t.Begin(), ittest = tensor.Begin(); it != t.End(); ++it, ++ittest )
              {
              RealType difference = static_cast<RealType>( *it ) - (*ittest);
              if( NumericTraits<RealType>::IsNegative( difference ) )
                {
                difference = -difference;
                }
              sumdifference += difference;
              }

            OutputPixelType d = static_cast<OutputPixelType>(sumdifference);
            if( d < minimumDifference )
              {
              minimumDifference = d;
              if( minimumDifference <= m_DifferenceThreshold )
                {
                break;
                }
              }
            }
          }

        // Check if difference is above threshold.
        if( minimumDifference > m_DifferenceThreshold )
          {
          // Store the minimum difference value in the output image.
          out.Set(minimumDifference);
          // Update difference image statistics.
          m_ThreadDifferenceSum[threadId] += minimumDifference;
          m_ThreadNumberOfPixels[threadId]++;
          }
        else
          {
          // Difference is below threshold.
          out.Set(NumericTraits<OutputPixelType>::ZeroValue());
          }

        // Update progress.
        progress.CompletedPixel();
        }
      }
    else
      {
      for( out.GoToBegin(); !out.IsAtEnd(); ++out )
        {
        out.Set(NumericTraits<OutputPixelType>::ZeroValue());
        progress.CompletedPixel();
        }
      }
    }
  }

// ----------------------------------------------------------------------------
template <class TInputImage, class TOutputImage>
void
DifferenceDiffusionTensor3DImageFilter<TInputImage, TOutputImage>
::AfterThreadedGenerateData()
{
  // Set statistics about difference image.
  int numberOfThreads = this->GetNumberOfThreads();

  for( int i = 0; i < numberOfThreads; ++i )
    {
    m_TotalDifference += m_ThreadDifferenceSum[i];
    m_NumberOfPixelsWithDifferences += m_ThreadNumberOfPixels[i];
    }
  // Get the total number of pixels processed in the region.
  // This is different from the m_TotalNumberOfPixels which
  // is the number of pixels that actually have differences
  // above the intensity threshold.
  OutputImageRegionType region = this->GetOutput()->GetRequestedRegion();
  AccumulateType        numberOfPixels = region.GetNumberOfPixels();

  // Calculate the mean difference.
  m_MeanDifference = m_TotalDifference / numberOfPixels;
}

} // end namespace itk

#endif
