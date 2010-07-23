#ifndef __CreateField_txx
#define __CreateField_txx
#include "CreateField.h"
#include "itkIOCommon.h"
#include "itkVectorIndexSelectionCastImageFilter.h"
#include "itkIO.h"
#include <errno.h>

namespace itk
{
template< typename TImage, typename T2Image >
CreateField< TImage,
             T2Image >::CreateField()
{
  // MUST GRAB IMAGE 1 AND 2, Parameter Map and Warped Image Name
  m_Image1Filename = "";
  m_Image2Filename = "";
  m_ParameterFilename = "";

  m_ImageOne = NULL;
  m_ImageTwo = NULL;

  m_NumberOfHistogramLevels = 1024;
  m_NumberOfMatchPoints = 7;

  m_NumberOfLevels = 1;
  m_Image1ShrinkFactors.Fill(1);
  m_Image2ShrinkFactors.Fill(1);

  m_NumberOfIterations = IterationsArrayType(1);
  m_NumberOfIterations.Fill(10);

  m_FixedImage  = NULL;
  m_MovingImage = NULL;

  m_NumberOfHistogramLevels = 256;
  m_NumberOfMatchPoints = 1;

  m_FixedImageMinimum = 0;
  m_MovingImageMinimum = 0;

  m_DeformationField = NULL;

  m_FixedImagePyramid  = FixedImagePyramidType::New();
  m_FixedImagePyramid->UseShrinkImageFilterOff();
  m_MovingImagePyramid = MovingImagePyramidType::New();
  m_MovingImagePyramid->UseShrinkImageFilterOff();
  m_Registration       = RegistrationType::New();

  m_Registration->SetFixedImagePyramid(m_FixedImagePyramid);
  m_Registration->SetMovingImagePyramid(m_MovingImagePyramid);

  typedef SimpleMemberCommand< Self > CommandType;
  typename CommandType::Pointer command = CommandType::New();
  command->SetCallbackFunction(this, &Self::StartNewLevel);

  m_Tag = m_Registration->AddObserver(IterationEvent(), command);
}

template< typename TImage, typename T2Image >
CreateField< TImage,
             T2Image >::~CreateField()
{
  m_Registration->RemoveObserver(m_Tag);
}

template< typename TImage, typename T2Image >
void CreateField< TImage,
                  T2Image >::Execute()
{
  try
    {
    std::cout << "  Reading Input Images and Parameters" << std::endl;
    m_ImageOne = itkUtil::ReadImage< TImage >(m_Image1Filename);
    m_ImageTwo = itkUtil::ReadImage< TImage >(m_Image2Filename);

    FILE *paramFile;
    paramFile = fopen(m_ParameterFilename.c_str(), "r");
    if ( !paramFile )
      {
      itkExceptionMacro(<< "  Could not open parameter file. ");
      }

    unsigned int uNumber;

    if ( fscanf(paramFile, "%d", &uNumber) != 1 )
      {
      itkExceptionMacro(<< "  Could not find the number of histogram levels.");
      }
    m_NumberOfMatchPoints = uNumber;

    if ( fscanf(paramFile, "%d", &uNumber) != 1 )
      {
      itkExceptionMacro(<< "Could not find the number of match points.");
      }
    m_NumberOfHistogramLevels = uNumber;

    if ( fscanf(paramFile, "%d", &uNumber) != 1 )
      {
      itkExceptionMacro(<< "Could not find the number of levels.");
      }
    m_NumberOfLevels = uNumber;

      {
      itk::Array< unsigned int > temp(m_NumberOfLevels);

      temp.Fill(0);
      m_NumberOfIterations = temp;
      }

    for ( unsigned int j = 0; j < m_NumberOfLevels; j++ )
      {
      if ( fscanf(paramFile, "%d", &uNumber) != 1 )
        {
        itkExceptionMacro(<< "Could not find number of iterations per level. ");
        }
      m_NumberOfIterations[j] = uNumber;
      }

    for ( unsigned int j = 0; j < ImageDimension; j++ )
      {
      if ( fscanf(paramFile, "%d", &uNumber) != 1 )
        {
        itkExceptionMacro(<< "Could not find atlas starting shrink factor. ");
        }
      m_Image1ShrinkFactors[j] = uNumber;
      }

    for ( unsigned int j = 0; j < ImageDimension; j++ )
      {
      if ( fscanf(paramFile, "%d", &uNumber) != 1 )
        {
        itkExceptionMacro(
          << "  Could not find subject starting shrink factor. ");
        }
      m_Image2ShrinkFactors[j] = uNumber;
      }
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "  Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::cout << "  Error occurred during input parsing." << std::endl;
    throw;
    }

  std::cout << "  Preprocessing the images" << std::endl;
  try
    {
    this->NormalizeImage(m_ImageTwo, m_FixedImage, m_FixedImageMinimum);
    this->NormalizeImage(m_ImageOne, m_MovingImage, m_MovingImageMinimum);

    typedef HistogramMatchingImageFilter< OutputImageType,
                                          OutputImageType > FilterType;
    typename FilterType::Pointer filter = FilterType::New();

    filter->SetInput(m_MovingImage);
    filter->SetReferenceImage(m_FixedImage);
    filter->SetNumberOfHistogramLevels(m_NumberOfHistogramLevels);
    filter->SetNumberOfMatchPoints(m_NumberOfMatchPoints);
    filter->ThresholdAtMeanIntensityOn();
    filter->Update();

    m_MovingImage = filter->GetOutput();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "  Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::cout << "  Error occured during preprocessing." << std::endl;
    throw;
    }

  std::cout << "  Registering the images" << std::endl;

  try
    {
    m_FixedImage->SetMetaDataDictionary( m_ImageTwo->GetMetaDataDictionary() );
    m_MovingImage->SetMetaDataDictionary( m_ImageOne->GetMetaDataDictionary() );
    // HACK:  Need to fixe DirectionType != operator to be const correct
    // if(m_FixedImage->GetDirection() != m_MovingImage->GetDirection()
    typename TImage::DirectionType temp = m_FixedImage->GetDirection();
    if ( temp != m_MovingImage->GetDirection()
         ||
         ( itk::SpatialOrientationAdapter().FromDirectionCosines( m_FixedImage
                                                                  ->GetDirection() ) !=
           itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RIP )
         )
      {
      std::cout
      << "Image Directions are not the same or are not in RIP orientation "
      << std::endl
      << m_FixedImage->GetDirection()
      << "=============" << std::endl
      << m_MovingImage->GetDirection()
      << std::endl;
      }
    m_ImageOne->DisconnectPipeline();
    m_ImageOne = NULL;
    m_ImageTwo->DisconnectPipeline();
    m_ImageTwo = NULL;

    m_FixedImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
    m_FixedImagePyramid->SetStartingShrinkFactors(
      m_Image2ShrinkFactors.GetDataPointer() );

    m_MovingImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
    m_MovingImagePyramid->SetStartingShrinkFactors(
      m_Image1ShrinkFactors.GetDataPointer() );

    m_Registration->SetFixedImage(m_FixedImage);
    m_Registration->SetMovingImage(m_MovingImage);
    m_Registration->SetNumberOfLevels(m_NumberOfLevels);
    m_Registration->SetNumberOfIterations( m_NumberOfIterations.data_block() );
    try
      {
      m_Registration->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "  Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      }
    catch ( ... )
      {
      std::cout << errno << " " << __FILE__ << " " << __LINE__ << std::endl;
      std::cout << "  Caught a non-ITK exception " << __FILE__ << " "
                << __LINE__ << std::endl;
      }

    try
      {
      m_Registration->ReleaseDataFlagOn();
      m_DeformationField = m_Registration->GetOutput();
      m_DeformationField->DisconnectPipeline();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "  Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      }
    catch ( ... )
      {
      std::cout << "  Caught a non-ITK exception " << __FILE__ << " "
                << __LINE__ << std::endl;
      throw;
      }
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "  Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::cout << "  Error occured during registration" << std::endl;
    throw;
    }
}

template< typename TImage, typename T2Image >
void CreateField< TImage,
                  T2Image >::ReleaseDataFlagOn()
{
  m_FixedImage->DisconnectPipeline();
  m_FixedImage = NULL;
  m_MovingImage->DisconnectPipeline();
  m_MovingImage = NULL;
}

template< typename TImage, typename T2Image >
void CreateField< TImage,
                  T2Image >::NormalizeImage(InputImageType *input,
                                            OutputImagePointer & output,
                                            InputPixelType & min)
{
  typedef MinimumMaximumImageFilter< InputImageType > MinMaxFilterType;
  typename MinMaxFilterType::Pointer minMaxFilter = MinMaxFilterType::New();

  minMaxFilter->SetInput(input);
  minMaxFilter->Update();

  min = minMaxFilter->GetMinimum();
  double shift = -1.0 * static_cast< double >( min );
  double scale = static_cast< double >( minMaxFilter->GetMaximum() );
  scale += shift;
  scale = 1.0 / scale;

  typedef ShiftScaleImageFilter< InputImageType, OutputImageType > FilterType;
  typename FilterType::Pointer filter = FilterType::New();

  filter->SetInput(input);
  filter->SetShift(shift);
  filter->SetScale(scale);
  filter->Update();

  output = filter->GetOutput();
}

template< typename OutputImageType,
          typename InputImageType >
void FFCreateNewImageFromTemplate(
  typename OutputImageType::Pointer & PointerToOutputImage,
  const typename InputImageType::Pointer & PreInitializedImage)
{
  PointerToOutputImage = OutputImageType::New();
  PointerToOutputImage->SetRegions(
    PreInitializedImage->GetLargestPossibleRegion() );
  PointerToOutputImage->CopyInformation(PreInitializedImage);
  PointerToOutputImage->Allocate();
  PointerToOutputImage->FillBuffer(0);
  CHECK_CORONAL( PointerToOutputImage->GetDirection() );
}

template< typename TImage, typename T2Image >
void CreateField< TImage,
                  T2Image >::StartNewLevel()
{
  std::cout << "  Starting level " << m_Registration->GetCurrentLevel()
            << std::endl;
}
}
#endif
