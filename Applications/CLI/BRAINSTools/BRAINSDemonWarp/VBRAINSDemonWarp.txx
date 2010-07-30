#ifndef _VBRAINSDemonWarp_txx
#define _VBRAINSDemonWarp_txx

#include "VBRAINSDemonWarp.h"

namespace itk
{
template< typename TImage, typename TRealImage, typename TOutputImage >
VBRAINSDemonWarp< TImage, TRealImage, TOutputImage >
::VBRAINSDemonWarp()
{
  m_TheMovingImageFilename.reserve(10);
  m_TheFixedImageFilename.reserve(10);

  // m_ParameterFilename = "";
  m_OutputFilename = "";
  m_AppendOutputFile = true;
  m_WarpedImageName = "none";
  m_CheckerBoardFilename = "none";
  m_DeformationFieldOutputName = "none";
  m_DisplacementBaseName = "none";
  m_CheckerBoardPattern.Fill(4);
  m_Lower = NumericTraits< PixelType >::NonpositiveMin();
  m_Upper = NumericTraits< PixelType >::max();
  m_DefaultPixelValue = NumericTraits< PixelType >::Zero;
  m_Radius.Fill(1);
  m_FixedBinaryVolume = "none";
  m_MovingBinaryVolume = "none";
  m_ForceCoronalZeroOrigin = false;
  m_OutNormalized = "OFF";
  m_UseHistogramMatching = false;
  //    m_OutDebug = false;
  m_NumberOfHistogramLevels = 256;
  m_NumberOfMatchPoints = 2;
  m_NumberOfLevels = 4;   // if that fixes it, I'm going to do something else
  m_NumberOfIterations = IterationsArrayType(m_NumberOfLevels);
  m_NumberOfIterations[0] = 2000;
  m_NumberOfIterations[1] = 500;
  m_NumberOfIterations[2] = 250;
  m_NumberOfIterations[3] = 100;
  for ( unsigned i = 0; i < ImageType::ImageDimension; i++ )
    {
    m_TheMovingImageShrinkFactors[i] = 4;   // 16;
    m_TheFixedImageShrinkFactors[i] = 4;    // 16;
    m_Seed[i] = 0;
    m_MedianFilterSize[i] = 0;
    }
}

/*This method initializes the input parser which reads in the moving image,
  * fixed image and parameter file.*/

template< typename TImage, typename TRealImage, typename TOutputImage >
void
VBRAINSDemonWarp< TImage, TRealImage, TOutputImage >
::InitializeParser()
{
  this->m_Parser->SetTheMovingImageFilename(this->m_TheMovingImageFilename);

  this->m_Parser->SetTheFixedImageFilename(this->m_TheFixedImageFilename);
  this->m_Parser->SetForceCoronalZeroOrigin( this->GetForceCoronalZeroOrigin() );

  this->m_Parser->SetInitialDeformationFieldFilename(
    this->m_InitialDeformationFieldFilename.c_str() );
  this->m_Parser->SetInitialCoefficientFilename(
    this->m_InitialCoefficientFilename.c_str() );
  this->m_Parser->SetInitialTransformFilename(
    this->m_InitialTransformFilename.c_str() );
  //            this->m_Parser->SetParameterFilename(
  // this->m_ParameterFilename.c_str() );
  this->m_Parser->SetNumberOfHistogramLevels( this->GetNumberOfHistogramLevels() );
  this->m_Parser->SetNumberOfMatchPoints( this->GetNumberOfMatchPoints() );
  this->m_Parser->SetNumberOfLevels( this->GetNumberOfLevels() );
  this->m_Parser->SetTheMovingImageShrinkFactors(
    this->GetTheMovingImageShrinkFactors() );
  this->m_Parser->SetTheFixedImageShrinkFactors(
    this->GetTheFixedImageShrinkFactors() );
  this->m_Parser->SetNumberOfIterations( this->GetNumberOfIterations() );
  this->m_Parser->SetOutDebug( this->GetOutDebug() );
}

/*This method initializes the preprocessor which processes the moving and fixed
  * images before registration. The image files which are read in using the
  * parser
  * are given to the preprocessor.*/

template< typename TImage, typename TRealImage, typename TOutputImage >
void
VBRAINSDemonWarp< TImage, TRealImage, TOutputImage >
::InitializePreprocessor()
{
  this->m_Preprocessor->SetInputFixedImage( this->m_Parser->GetTheFixedImages() );
  this->m_Preprocessor->SetInputMovingImage( this->m_Parser->GetTheMovingImages() );
  this->m_Preprocessor->SetInitialDeformationField(
    this->m_Parser->GetInitialDeformationField() );
  this->m_Preprocessor->SetUseHistogramMatching( this->GetUseHistogramMatching() );
  this->m_Preprocessor->SetNumberOfHistogramLevels(
    this->m_Parser->GetNumberOfHistogramLevels() );
  this->m_Preprocessor->SetNumberOfMatchPoints(
    this->m_Parser->GetNumberOfMatchPoints() );
  this->m_Preprocessor->SetFixedBinaryVolume( this->GetFixedBinaryVolume() );
  this->m_Preprocessor->SetMovingBinaryVolume( this->GetMovingBinaryVolume() );
  this->m_Preprocessor->SetLower( this->GetLower() );
  this->m_Preprocessor->SetUpper( this->GetUpper() );
  this->m_Preprocessor->SetRadius( this->GetRadius() );
  this->m_Preprocessor->SetDefaultPixelValue( this->GetDefaultPixelValue() );
  this->m_Preprocessor->SetSeed( this->GetSeed() );
  this->m_Preprocessor->SetOutDebug( this->GetOutDebug() );
  this->m_Preprocessor->SetMedianFilterSize( this->GetMedianFilterSize() );
  this->m_Preprocessor->SetInitialDeformationField(
    this->m_Parser->GetInitialDeformationField() );
  //  this->m_Preprocessor->SetWeightFactors( this->GetWeightFactors() );
}

/*This method initializes the registration process. The preprocessed output
  * files are passed to the registrator.*/

template< typename TImage, typename TRealImage, typename TOutputImage >
void
VBRAINSDemonWarp< TImage, TRealImage, TOutputImage >
::InitializeRegistrator()
{
  this->m_Registrator->SetDisplacementBaseName( this->GetDisplacementBaseName() );
  this->m_Registrator->SetWarpedImageName( this->GetWarpedImageName() );
  this->m_Registrator->SetCheckerBoardFilename( this->GetCheckerBoardFilename() );
  this->m_Registrator->SetDeformationFieldOutputName(
    this->GetDeformationFieldOutputName() );
  this->m_Registrator->SetCheckerBoardPattern( this->GetCheckerBoardPattern() );
  this->m_Registrator->SetFixedImage( this->m_Preprocessor->GetOutputFixedImage() );
  this->m_Registrator->SetMovingImage(
    this->m_Preprocessor->GetOutputMovingImage() );
  this->m_Registrator->SetUnNormalizedMovingImage(
    this->m_Preprocessor->GetUnNormalizedMovingImage() );
  this->m_Registrator->SetUnNormalizedFixedImage(
    this->m_Preprocessor->GetUnNormalizedFixedImage() );
  this->m_Registrator->SetInitialDeformationField(
    this->m_Parser->GetInitialDeformationField() );
  this->m_Registrator->SetDefaultPixelValue(
    this->m_Preprocessor->GetDefaultPixelValue() );
  this->m_Registrator->SetUseHistogramMatching( this->GetUseHistogramMatching() );
  this->m_Registrator->SetNumberOfLevels( this->m_Parser->GetNumberOfLevels() );
  this->m_Registrator->SetNumberOfIterations(
    this->m_Parser->GetNumberOfIterations() );
  this->m_Registrator->SetInterpolationMode( this->GetInterpolationMode() );

  this->m_Registrator->SetFixedImageShrinkFactors(
    this->m_Parser->GetTheFixedImageShrinkFactors() );
  this->m_Registrator->SetMovingImageShrinkFactors(
    this->m_Parser->GetTheMovingImageShrinkFactors() );

  this->m_Registrator->SetOutNormalized( this->GetOutNormalized() );
  this->m_Registrator->SetOutDebug( this->GetOutDebug() );
  this->m_Registrator->SetDeformationFieldOutputName(
    this->m_DeformationFieldOutputName);
  this->m_Registrator->SetFixedLandmarkFilename(this->m_FixedLandmarkFilename);
  this->m_Registrator->SetMovingLandmarkFilename(this->m_MovingLandmarkFilename);
  this->m_Registrator->SetWeightFactors( this->GetWeightFactors() );
}
}   // namespace itk

#endif
