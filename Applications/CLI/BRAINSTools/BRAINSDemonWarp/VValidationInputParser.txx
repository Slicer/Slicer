#ifndef _VValidationInputParser_txx
#define _VValidationInputParser_txx

#include "VValidationInputParser.h"
#include "itkMetaDataObject.h"
#include "itkNumericTraits.h"
#include "vnl/vnl_math.h"
#include "itkImage.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkIO.h"

// The following was just copied out of iccdefWarpImage.cc.  Sorry.
#include "itkBrains2MaskImageIO.h"
#include "itkBrains2MaskImageIOFactory.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkSpatialOrientation.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include "itkMultiplyByConstantImageFilter.h"
#include <itkIO.h>
#include <metaCommand.h>
#include "itkImageRegionIterator.h"
#ifdef __USE_BRAINS2_INTEGRATION
#  include "TransformToDeformationField.h"
#endif
#include <itkDeformationFieldJacobianDeterminantFilter.h>
// #include <itkMultiplyImageFilter.h>
// #include "Jacobian.h"
#include <fstream>

#ifdef __USE_BRAINS2_INTEGRATION
#  include "iccdefdeformation/Deformation.h"
#  include "iccdefdeformation/Utils.h"
#  include "b2Affine_rw.h"
#  include "iccdefdeformation/HarmonicArrayIO.h"
#endif

namespace itk
{
template< typename TImage >
VValidationInputParser< TImage >
::VValidationInputParser()
{
  //      m_TheMovingImageFilename = "";
  //      m_TheFixedImageFilename = "" ;

  m_ParameterFilename = "";

  m_TheMovingImages.reserve(10);
  m_TheFixedImages.reserve(10);

  m_NumberOfHistogramLevels = 1024;
  m_NumberOfMatchPoints = 7;

  m_NumberOfLevels = 1;
  m_TheMovingImageShrinkFactors.Fill(1);
  m_TheFixedImageShrinkFactors.Fill(1);

  m_NumberOfIterations = IterationsArrayType(1);
  m_NumberOfIterations.Fill(10);

  m_OutDebug = false;
  m_ForceCoronalZeroOrigin = false;
}

template< typename TImage >
void
VValidationInputParser< TImage >
::Execute()
{
  /*************************
   * Read in the images
   */
  if ( this->m_ForceCoronalZeroOrigin == true )
    {
    std::cout << "---Forcing Brains2 Orientation not yet implemented"
              << std::endl;
    exit (-1);
    }
  else
    {
    for ( unsigned int i = 0; i < m_TheFixedImageFilename.size(); ++i )
      {
      m_TheFixedImages.push_back( itkUtil::ReadImage< TImage >(
                                    m_TheFixedImageFilename[i]) );
      m_TheMovingImages.push_back( itkUtil::ReadImage< TImage >(
                                     m_TheMovingImageFilename[i]) );
      }
    }
  // HACK:  TODO:  Need to ensure that the fixed and moving images have the same
  // orientations.

  // TODO:  Need to figure out how to read in the initial deformation field.
  // std::cerr << "About to check for deformation field file " <<
  // m_InitialDeformationFieldFilename << std::endl;
  // std::cerr << "About to check for transform file " <<
  // m_InitialTransformFilename << std::endl;
  // std::cerr << "About to check for Coefficient file" <<
  // m_InitialCoefficientFilename << std::endl;
  if ( m_InitialDeformationFieldFilename != "" )
    {
    typedef   itk::ImageFileReader< TDeformationField > FieldReaderType;
    typename FieldReaderType::Pointer fieldReader = FieldReaderType::New();
    fieldReader->SetFileName( m_InitialDeformationFieldFilename.c_str() );
    try
      {
      fieldReader->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cerr << "Caught an ITK exception: " << std::endl;
      throw err;
      }
    if ( this->GetOutDebug() )
      {
      std::cout << "\nReading Deformation fields.\n";
      }
    m_InitialDeformationField = fieldReader->GetOutput();
    //  typename ImageType::DirectionType DeformationOrientation;
    //  DeformationOrientation=deformationField->GetDirection();
    }
#ifdef __USE_BRAINS2_INTEGRATION
  else if ( m_InitialTransformFilename != "" )
    {
    //  REFACTOR continuing here:
    //  At this point, setting an AffineTransform to generate the initial
    // deformation field
    //  reads and converts a brains2 .xfrm file.

    // read brains2 transform file
    typedef B2AffineTransform< ImageType >                B2AffineTransformType;
    typedef typename B2AffineTransformType::TransformType AffineTransformType;
    B2AffineTransformType transform;
    transform.Read(m_InitialTransformFilename);
    typename AffineTransformType::Pointer inputAffineTransform =
      transform.GetAffineTransformPointer();
    if ( inputAffineTransform.IsNull() )
      {
      std::cerr << "Can't read transform file" << m_InitialTransformFilename
                << std::endl;
      }

    typename TDeformationField::RegionType::SizeType size =
      transform.GetFixedImageSize();
    typename TDeformationField::SpacingType spacing =
      transform.GetFixedImageSpacing();

    // convert brains2 transform, which is in index, to ITK transform, which is
    // in mm
    typedef itk::AffineTransform< double, 3 > ITKAffineTransformType;
    typedef itk::Vector< double, 3 >          VectorType;
    VectorType const fixedImageScaleReciprocal( Reciprocal< double, 3 >(
                                                  transform.GetFixedImageSpacing() ) );

    VectorType const movingImageScale( transform.GetMovingImageSpacing() );

    typedef CrossOverAffineSystem< double, 3 > CrossOverAffineSystemType;
    CrossOverAffineSystemType::Pointer crossOverAffineSystem =
      CrossOverAffineSystemType::New();
    crossOverAffineSystem->EncloseInScaling(fixedImageScaleReciprocal,
                                            movingImageScale);

    const bool                      ApplyUpstream = false;
    ITKAffineTransformType::Pointer InitialITKAffineTransform =
      ITKAffineTransformType::New();
    InitialITKAffineTransform->SetIdentity();
    InitialITKAffineTransform->Compose(
      crossOverAffineSystem->GetInhaleEncodeConversion(),
      ApplyUpstream);
    InitialITKAffineTransform->Compose(inputAffineTransform, ApplyUpstream);
    InitialITKAffineTransform->Compose(
      crossOverAffineSystem->GetInhaleDecodeConversion(),
      ApplyUpstream);
#  ifdef USE_TRANSFORM_INVERSE_FOR_INIT_FROM_AFFINE_TRANSFORM
    ITKAffineTransformType::Pointer InitialITKAffineTransformInverse =
      ITKAffineTransformType::New();
    InitialITKAffineTransform->GetInverse(InitialITKAffineTransformInverse);
    m_InitialDeformationField =
      TransformToDeformationField(m_TheFixedImage,
                                  InitialITKAffineTransformInverse);
#  else
    m_InitialDeformationField =
      TransformToDeformationField(m_TheFixedImage,
                                  InitialITKAffineTransform);
#  endif
    }
#endif
#ifdef __USE_BRAINS2_INTEGRATION
  else if ( m_InitialCoefficientFilename != "" )
    {
    DeformationFieldFFTType::Pointer mu;    // mu1, mu2, mu3;
    std::string                      CoeffNameInput(
      m_InitialCoefficientFilename.c_str() );

      {
      if ( this->GetOutDebug() )
        {
        std::cout << "Reading: " << CoeffNameInput << std::endl;
        }
      HarmonicReadAll3D(mu, CoeffNameInput);
      }
    if ( this->GetOutDebug() )
      {
      std::cout << "\nCreating Deformation fields from Coefficient files\n";
      }
    m_InitialDeformationField = CreateITKDisplacementFieldFromCoeffs(mu);
    }
#endif

  // Print out the parameters.
  if ( this->GetOutDebug() )
    {
    std::cout << "NumberOfHistogramLevels : " << m_NumberOfHistogramLevels
              << std::endl;
    std::cout << "NumberOfMatchPoints : " << m_NumberOfMatchPoints << std::endl;
    std::cout << "NumberOfLevels : " << m_NumberOfLevels << std::endl;
    std::cout << "NumberOfIterations : " << m_NumberOfIterations << std::endl;
    std::cout << "TheMovingImageShrinkFactors : "
              << m_TheMovingImageShrinkFactors << std::endl;
    std::cout << "TheFixedImageShrinkFactors : "
              << m_TheFixedImageShrinkFactors << std::endl;
    }
}
} // namespace itk

#endif
