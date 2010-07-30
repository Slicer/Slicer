#ifndef _VDemonsPreprocessor_txx
#define _VDemonsPreprocessor_txx

#include "VDemonsPreprocessor.h"
#include "itkMinimumMaximumImageFilter.h"
#include "itkShiftScaleImageFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkBOBFFilter.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkIO.h"
#include "itkMedianImageFilter.h"

#include "itkMultiResolutionPDEDeformableRegistration.h"
#include "itkDiffeomorphicDemonsRegistrationFilter.h"
#include "itkImageFileWriter.h"

namespace itk
{
template< typename TInputImage, typename TOutputImage >
VDemonsPreprocessor< TInputImage, TOutputImage >
::VDemonsPreprocessor()
{
  m_UseHistogramMatching = 0;
  m_NumberOfHistogramLevels = 256;
  m_NumberOfMatchPoints = 1;

  m_FixedImageMinimum =  NumericTraits< InputPixelType >::NonpositiveMin();
  m_MovingImageMinimum = NumericTraits< InputPixelType >::NonpositiveMin();

  m_InputFixedImage.reserve(10);
  m_InputMovingImage.reserve(10);
  m_OutputFixedImage.reserve(10);
  m_OutputMovingImage.reserve(10);
  m_UnNormalizedFixedImage.reserve(10);
  m_UnNormalizedMovingImage.reserve(10);

  m_FixedBinaryVolume = "none";
  m_MovingBinaryVolume = "none";

  //    m_Seed =  NumericTraits<IndexType>::Zero;
  for ( unsigned i = 0; i < TInputImage::ImageDimension; i++ )
    {
    m_Seed[i] = 0;
    m_MedianFilterSize[i] = 0;
    }
  m_Lower = NumericTraits< PixelType >::NonpositiveMin();
  m_Upper = NumericTraits< PixelType >::max();

  m_DefaultPixelValue = NumericTraits< PixelType >::One;
  m_Radius.Fill(1);
  m_OutDebug = false;
}

template< typename TInputImage, typename TOutputImage >
void
VDemonsPreprocessor< TInputImage, TOutputImage >
::Execute()
{
  typedef itk::MultiplyByConstantImageFilter< TOutputImage, float,
                                              TOutputImage > MultiplyByConstantImageType;

  if ( m_MedianFilterSize[0] > 0  ||  m_MedianFilterSize[1] > 0
       ||  m_MedianFilterSize[2] > 0 )
    {
    for ( unsigned int i = 0; i < m_InputFixedImage.size(); ++i )
      {
      typedef typename itk::MedianImageFilter< TInputImage,
                                               TInputImage > MedianImageFilterType;
      typename MedianImageFilterType::Pointer medianFilter =
        MedianImageFilterType::New();
      medianFilter->SetRadius(m_MedianFilterSize);
      medianFilter->SetInput(m_InputFixedImage[i]);
      medianFilter->Update();
      m_InputFixedImage.push_back( medianFilter->GetOutput() );
      m_InputFixedImage.erase( m_InputFixedImage.begin() );

      // reinitialize
      medianFilter = MedianImageFilterType::New();
      medianFilter->SetRadius(m_MedianFilterSize);
      medianFilter->SetInput(m_InputMovingImage[i]);
      medianFilter->Update();
      m_InputMovingImage.push_back( medianFilter->GetOutput() );
      m_InputMovingImage.erase( m_InputMovingImage.begin() );
      }
    }
  for ( unsigned int i = 0; i < m_InputFixedImage.size(); ++i )
    {
    // Create UnNormalized...Images
    /*
      *  typename MultiplyByConstantImageType::Pointer multi_FixedImageConstant
      *    = MultiplyByConstantImageType::New();
      *  multi_FixedImageConstant->SetInput( itkUtil::PreserveCast<TInputImage,
      *      TOutputImage>(this->m_InputFixedImage[i]) );
      *  multi_FixedImageConstant->SetConstant(m_WeightFactors[i]);
      *  multi_FixedImageConstant->Update();
      *
      *  typename MultiplyByConstantImageType::Pointer multi_MovingImageConstant
      *    = MultiplyByConstantImageType::New();
      *  multi_MovingImageConstant->SetInput( itkUtil::PreserveCast<TInputImage,
      *      TOutputImage>(this->m_InputMovingImage[i]) );
      *  multi_MovingImageConstant->SetConstant(m_WeightFactors[i]);
      *  multi_MovingImageConstant->Update();
      */
    this->m_UnNormalizedFixedImage.push_back( itkUtil::PreserveCast< TInputImage,
                                                                     TOutputImage >(this->m_InputFixedImage[i]) );
    this->m_UnNormalizedMovingImage.push_back( itkUtil::PreserveCast< TInputImage,
                                                                      TOutputImage >(this->m_InputMovingImage[i]) );

    m_OutputMovingImage.push_back( itkUtil::CopyImage< TOutputImage >(
                                     m_UnNormalizedMovingImage[i]) );

    if ( this->GetUseHistogramMatching () )
      {
      typedef HistogramMatchingImageFilter< OutputImageType,
                                            OutputImageType > HistogramMatchingFilterType;
      typename HistogramMatchingFilterType::Pointer histogramfilter =
        HistogramMatchingFilterType::New();
      if ( this->GetOutDebug() )
        {
        std::cout << "Performing Histogram Matching \n";
        }

      if ( ( vcl_numeric_limits< typename OutputImageType::PixelType >::max()
             - vcl_numeric_limits< typename OutputImageType::PixelType >::min() )
           < m_NumberOfHistogramLevels )
        {
        std::cout
        << "The intensity of range is less than Histogram levels!!"
        << std::endl;
        }

      histogramfilter->SetInput(m_UnNormalizedMovingImage[i]);
      histogramfilter->SetReferenceImage(m_UnNormalizedFixedImage[i]);

      histogramfilter->SetNumberOfHistogramLevels(m_NumberOfHistogramLevels);
      histogramfilter->SetNumberOfMatchPoints(m_NumberOfMatchPoints);
      histogramfilter->ThresholdAtMeanIntensityOn();
      histogramfilter->Update();
      m_OutputMovingImage.pop_back();
      m_OutputMovingImage.push_back( histogramfilter->GetOutput() );
      }

    m_OutputFixedImage.push_back( itkUtil::CopyImage< TOutputImage >(
                                    m_UnNormalizedFixedImage[i]) );

    // Make BOBF Images if specified
    if ( this->m_FixedBinaryVolume != std::string ("none") )
      {
      if ( this->GetOutDebug() )
        {
        std::cout << "Making BOBF \n";
        std::cout << "PRE Fixed Origin"
                  << m_OutputFixedImage[i]->GetOrigin() << std::endl;
        }
      m_OutputFixedImage[i] = this->MakeBOBFImage(m_OutputFixedImage[i],
                                                  m_FixedBinaryVolume);
      if ( this->GetOutDebug() )
        {
        std::cout << "Fixed Origin" << m_OutputFixedImage[i]->GetOrigin()
                  << std::endl;
        std::cout << "PRE Moving Origin"
                  << m_OutputMovingImage[i]->GetOrigin() << std::endl;
        }
      m_OutputMovingImage[i] = this->MakeBOBFImage(m_OutputMovingImage[i],
                                                   m_MovingBinaryVolume);
      if ( this->GetOutDebug() )
        {
        std::cout << "Moving Origin" << m_OutputMovingImage[i]->GetOrigin()
                  << std::endl;
        std::cout << "Writing Brain Only Background Filled Moving image"
                  << std::endl;
        itkUtil::WriteImage< TOutputImage >(m_OutputMovingImage[i],
                                            "BOBF_Moving.nii.gz");
        itkUtil::WriteImage< TOutputImage >(m_OutputFixedImage[i],
                                            "BOBF_Fixed.nii.gz");
        }
      }
    m_InputMovingImage[i] = NULL;
    m_InputFixedImage[i] = NULL;
    }

  if ( m_OutputFixedImage.size() > 1 )
    {
    int condition = 1;
    for ( unsigned int i = 1; i < m_InputFixedImage.size(); ++i )
      {
      if ( m_OutputFixedImage[i]->GetLargestPossibleRegion().GetSize() ==
           m_OutputFixedImage[0]->GetLargestPossibleRegion().GetSize() )
        {
        condition *= 1;
        }
      else
        {
        condition *= 0;
        }

      if ( m_OutputFixedImage[i]->GetSpacing() ==
           m_OutputFixedImage[0]->GetSpacing() )
        {
        condition *= 1;
        }
      else
        {
        condition *= 0;
        }

      if ( m_OutputMovingImage[i]->GetLargestPossibleRegion().GetSize() ==
           m_OutputMovingImage[0]->GetLargestPossibleRegion().GetSize() )
        {
        condition *= 1;
        }
      else
        {
        condition *= 0;
        }

      if ( m_OutputMovingImage[i]->GetDirection() ==
           m_OutputMovingImage[0]->GetDirection() )
        {
        condition *= 1;
        }
      else
        {
        condition *= 0;
        }
      }
    if ( condition != 1 )
      {
      std::cout
      << "Fixed images or Moving images have different size or spacing!"
      << std::endl;
      exit (-1);
      }
    }
}

/*This function takes in a brain image and a whole brain mask and strips the
  * skull of the image. It uses the BOBF filter to perform the skull
  * stripping.*/

template< typename TInputImage, typename TOutputImage >
typename VDemonsPreprocessor< TInputImage,
                              TOutputImage >::OutputImagePointer VDemonsPreprocessor< TInputImage,
                                                                                      TOutputImage >
::MakeBOBFImage(OutputImagePointer input, std::string MaskName)
{
  OutputImagePointer Mask = itkUtil::ReadImage< OutputImageType >(MaskName);

  if ( ( m_UnNormalizedFixedImage[0]->GetLargestPossibleRegion().GetSize() !=
         Mask->GetLargestPossibleRegion().GetSize() )
       || ( m_UnNormalizedFixedImage[0]->GetSpacing() != Mask->GetSpacing() ) )
    {
    if ( this->GetOutDebug() )
      {
      std::cout << "Writing Resampled Output image" << std::endl;
      itkUtil::WriteImage< TOutputImage >(Mask, "Resampled.mask");
      }
    }

  typedef BOBFFilter< OutputImageType, OutputImageType > BOBFFilterType;
  typename BOBFFilterType::Pointer BOBFfilter = BOBFFilterType::New();
  if ( this->GetOutDebug() )
    {
    std::cout
    <<
    "Making Brain only Background filled image with the following parameters. "
    << std::endl;
    std::cout << "Lower Threshold:  " << m_Lower << std::endl;
    std::cout << "Upper Threshold:  " << m_Upper << std::endl;
    std::cout << "Neighborhood:  " << m_Radius << std::endl;
    std::cout << "Background fill Value:  " << m_DefaultPixelValue << std::endl;
    std::cout << "Seed :  " << m_Seed  << std::endl;
    }

  BOBFfilter->SetLower(m_Lower);
  BOBFfilter->SetUpper(m_Upper);
  BOBFfilter->SetRadius(m_Radius);
  BOBFfilter->SetReplaceValue(m_DefaultPixelValue);
  BOBFfilter->SetSeed(m_Seed);
  BOBFfilter->SetInputImage(input);
  BOBFfilter->SetInputMask(Mask);
  try
    {
    BOBFfilter->Update();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    exit (-1);
    }

  OutputImagePointer output = BOBFfilter->GetOutput();
  return output;
}
}   // namespace itk

#endif // _VDemonsPreprocessor_txx
