/*=========================================================================
 *
 *  Program:   BRAINSDemonsWarp
 *  Module:    $RCSfile: ITKHeader.h,v $
 *  Language:  C++
 *  Date:      $Date: 2006-04-25 23:20:16 $
 *  Version:   $Revision: 1.1 $
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#include "itkSimpleDiffeomorphicRegistration.h"
#include "itkVector.h"
#include "itkArray.h"

#include "itkDiffeomorphicDemonsRegistrationWithMaskFilter.h"
#include "itkPDEDeformableRegistrationFilter.h"
#include "itkLargestForegroundFilledMaskImageFilter.h"
#include "itkImageMaskSpatialObject.h"

itkSimpleDiffeomorphicRegistration::itkSimpleDiffeomorphicRegistration()
{
  m_DemonsPreprocessor = DemonsPreprocessorType::New();
  m_DemonsRegistrator = DemonsRegistratorType::New();
  m_DeformationField = NULL;
}

/*
  * void itkSimpleDiffeomorphicRegistration::SetDeformedImageName(std::string
  * name) {
  * m_DeformedImageName = name;
  * }
  */
/*
  * std::string itkSimpleDiffeomorphicRegistration::GetDeformedImageName(void) {
  * return m_DeformedImageName;
  * }
  */

void itkSimpleDiffeomorphicRegistration::InitializePreprocessor()
{
  m_DemonsPreprocessor->SetInputFixedImage(m_FixedImage);
  m_DemonsPreprocessor->SetInputMovingImage(m_MovingImage);
  m_DemonsPreprocessor->SetUseHistogramMatching(true);
  m_DemonsPreprocessor->SetNumberOfHistogramLevels(NumberOfHistogramLevels);
  m_DemonsPreprocessor->SetNumberOfMatchPoints(NumberOfMatchPoints);
}

void itkSimpleDiffeomorphicRegistration::Initialization()
{
  typedef itk::DiffeomorphicDemonsRegistrationWithMaskFilter< TRealImage,
                                                              TRealImage,
                                                              TDeformationField >
  RegistrationFilterType;
  RegistrationFilterType::Pointer filter = RegistrationFilterType::New();
  typedef itk::PDEDeformableRegistrationFilter< TRealImage, TRealImage,
                                                TDeformationField >
  BaseRegistrationFilterType;
  BaseRegistrationFilterType::Pointer actualfilter;
  // typedef RegistrationFilterType::GradientType TGradientType;
  TRealImage::Pointer movingBinaryVolumeImage;
  TRealImage::Pointer fixedBinaryVolumeImage;
  const double        otsuPercentileThreshold = 0.01;
  const int           closingSize = 7;
  //   fixedBinaryVolumeImage = FindLargestForgroundFilledMask<TRealImage>(
  //     m_FixedImage,
  //     otsuPercentileThreshold,
  //     closingSize);
  //   movingBinaryVolumeImage = FindLargestForgroundFilledMask<TRealImage>(
  //     m_MovingImage,
  //     otsuPercentileThreshold,
  //     closingSize);
  typedef itk::LargestForegroundFilledMaskImageFilter< TRealImage > LFFMaskFilterType;
  LFFMaskFilterType::Pointer LFF = LFFMaskFilterType::New();
  LFF->SetInput(m_FixedImage);
  LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
  LFF->SetClosingSize(closingSize);
  LFF->UpdateLargestPossibleRegion();
  fixedBinaryVolumeImage = LFF->GetOutput();
  //  LFF = LFFMaskFilterType::New();
  LFF->SetInput(m_MovingImage);
  LFF->SetOtsuPercentileThreshold(otsuPercentileThreshold);
  LFF->SetClosingSize(closingSize);
  LFF->UpdateLargestPossibleRegion();
  movingBinaryVolumeImage = LFF->GetOutput();

  typedef unsigned char                                     MaskPixelType;
  typedef itk::Image< MaskPixelType, DIM >                  MaskImageType;
  typedef itk::CastImageFilter< TRealImage, MaskImageType > CastImageFilter;

  typedef itk::SpatialObject< DIM > ImageMaskType;
  typedef ImageMaskType::Pointer    ImageMaskPointer;

  CastImageFilter::Pointer castFixedMaskImage = CastImageFilter::New();
  castFixedMaskImage->SetInput(fixedBinaryVolumeImage);
  castFixedMaskImage->Update();

  // convert mask image to mask
  typedef itk::ImageMaskSpatialObject< DIM > ImageMaskSpatialObjectType;
  ImageMaskSpatialObjectType::Pointer fixedMask = ImageMaskSpatialObjectType::New();
  fixedMask->SetImage( castFixedMaskImage->GetOutput() );
  fixedMask->ComputeObjectToWorldTransform();

  CastImageFilter::Pointer castMovingMaskImage = CastImageFilter::New();
  castMovingMaskImage->SetInput(movingBinaryVolumeImage);
  castMovingMaskImage->Update();

  // convert mask image to mask

  ImageMaskSpatialObjectType::Pointer movingMask = ImageMaskSpatialObjectType::New();
  movingMask->SetImage( castMovingMaskImage->GetOutput() );
  movingMask->ComputeObjectToWorldTransform();

  filter->SetFixedImageMask( dynamic_cast< ImageMaskType * >( fixedMask.GetPointer() ) );
  filter->SetMovingImageMask( dynamic_cast< ImageMaskType * >( fixedMask.GetPointer() ) );

  filter->SetMaximumUpdateStepLength(MaxStepLength);
  //  filter->SetUseGradientType(static_cast<TGradientType> (0));
  filter->SmoothDeformationFieldOn();
  filter->SetStandardDeviations(SmoothDeformationFieldSigma);
  filter->SmoothUpdateFieldOff();
  actualfilter = filter;
  m_DemonsRegistrator->SetRegistrationFilter(actualfilter);

  typedef itk::Array< unsigned int > IterationsArrayType;
  IterationsArrayType numberOfIterations;
  numberOfIterations.SetSize(NumberOfLevels);
  numberOfIterations.SetElement(0, NumberOfIteration0);
  numberOfIterations.SetElement(1, NumberOfIteration1);
  numberOfIterations.SetElement(2, NumberOfIteration2);
  numberOfIterations.SetElement(3, NumberOfIteration3);
  numberOfIterations.SetElement(4, NumberOfIteration4);

  typedef itk::FixedArray< unsigned int, 3 > ShrinkFactorsType;
  ShrinkFactorsType theMovingImageShrinkFactors;
  ShrinkFactorsType theFixedImageShrinkFactors;

  for ( int i = 0; i < 3; i++ )
    {
    theMovingImageShrinkFactors[i] = FixedPyramid;
    theFixedImageShrinkFactors[i] = FixedPyramid;
    }

  m_DemonsRegistrator->SetFixedImage( m_DemonsPreprocessor->GetOutputFixedImage() );
  m_DemonsRegistrator->SetMovingImage(
    m_DemonsPreprocessor->GetOutputMovingImage() );
  m_DemonsRegistrator->SetMovingImageShrinkFactors(theMovingImageShrinkFactors);
  m_DemonsRegistrator->SetFixedImageShrinkFactors(theFixedImageShrinkFactors);
  m_DemonsRegistrator->SetNumberOfLevels(NumberOfLevels);
  m_DemonsRegistrator->SetNumberOfIterations(numberOfIterations);
  m_DemonsRegistrator->SetWarpedImageName( this->GetDeformedImageName() );
  // m_DemonsRegistrator->SetDeformationFieldOutputName(m_DeformationFieldName);
  m_DemonsRegistrator->SetUseHistogramMatching(true);
}

// EXPORT
void itkSimpleDiffeomorphicRegistration::Update()
{
  InitializePreprocessor();
  try
    {
    m_DemonsPreprocessor->Execute();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::cout << "Error occured during preprocessing." << std::endl;
    throw;
    }
  Initialization();
  try
    {
    m_DemonsRegistrator->Execute();
    }
  catch ( itk::ExceptionObject & err )
    {
    std::cout << "Caught an ITK exception: " << std::endl;
    std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
    throw err;
    }
  catch ( ... )
    {
    std::
    cout << "Caught a non-ITK exception " << __FILE__ << " " << __LINE__
         << std::endl;
    }

  m_DeformationField = m_DemonsRegistrator->GetDeformationField();
}
