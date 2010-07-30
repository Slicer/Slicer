#ifndef __ValidationInputParser_txx
#define __ValidationInputParser_txx

#include "ValidationInputParser.h"
#include "itkMetaDataObject.h"
#include "itkNumericTraits.h"
#include "vnl/vnl_math.h"
#include "itkImage.h"
#include "itkAffineTransform.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkIO.h"
#include "itkTransformToDeformationFieldSource.h"
#include "itkTransform.h"
#include "itkAffineTransform.h"
#include "itkScaleVersor3DTransform.h"
#include "ConvertToRigidAffine.h"

// The following was just copied out of iccdefWarpImage.cc.  Sorry.
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkLinearInterpolateImageFunction.h"
#include "itkSpatialOrientation.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkBSplineInterpolateImageFunction.h"
#include <itkIO.h>
#include <metaCommand.h>
#include "itkImageRegionIterator.h"
#include "TransformToDeformationField.h"
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
ValidationInputParser< TImage >
::ValidationInputParser()
{
  m_TheMovingImageFilename = "";
  m_TheFixedImageFilename = "";

  m_ParameterFilename = "";

  m_TheMovingImage = NULL;
  m_TheFixedImage = NULL;

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
ValidationInputParser< TImage >
::Execute()
{
  /*************************
    * Read in the images
    */
  if ( this->m_ForceCoronalZeroOrigin == true )
    {
#ifdef FORCE_ZERO_ORIENT_BRAINS2_IMAGE  // NOTE:  IN THE AUTOSEGMENTATION CODE,
                                        // DO NOT CHANGE THIS FROM 1.  WE
    if ( this->GetOutDebug() )
      {
      std::cout << "---Forcing Brains2 Orientation " << std::endl;
      }
    m_TheFixedImage = itkUtil::ReadBrains2Image< TImage >(
      m_TheFixedImageFilename);
    m_TheMovingImage = itkUtil::ReadBrains2Image< TImage >(
      m_TheMovingImageFilename);
#else
    std::cout << "---Forcing Brains2 Orientation not yet implemented"
              << std::endl;
    exit (-1);
#endif
    }
  else
    {
    m_TheFixedImage = itkUtil::ReadImage< TImage >(m_TheFixedImageFilename);
    m_TheMovingImage = itkUtil::ReadImage< TImage >(m_TheMovingImageFilename);
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
  else if ( m_InitialTransformFilename != "" )
    {
#if 1
    // TODO: Zhao,  please implement the following, and test that it works
    // within
    //      the regression test suite.

    // VBRAINSDemonWarpPrimary.cxx
    // Review reading of transform files from BRAINSFit code for
    // reading Versor/Euler/Affine file from
    // Apparently when you register one transform, you need to register all your
    // transforms.
    //
    itk::AddExtraTransformRegister();

    //  ####### Now use TransformToDeformationFieldSource
    typedef itk::TransformToDeformationFieldSource< TDeformationField, double >
    DeformationFieldGeneratorType;
    typedef typename DeformationFieldGeneratorType::TransformType TransformType;
    // Only a templated base class.

    typename TransformType::Pointer trsf = itk::ReadTransformFromDisk(m_InitialTransformFilename);

    typename DeformationFieldGeneratorType::Pointer defGenerator = DeformationFieldGeneratorType::New();
    defGenerator->SetOutputSpacing( this->GetTheFixedImage()->GetSpacing() );
    defGenerator->SetOutputOrigin( this->GetTheFixedImage()->GetOrigin() );
    defGenerator->SetOutputDirection( this->GetTheFixedImage()->GetDirection() );
    defGenerator->SetOutputSize( this->GetTheFixedImage()->GetLargestPossibleRegion().GetSize() );
    defGenerator->SetOutputIndex( this->GetTheFixedImage()->GetLargestPossibleRegion().GetIndex() );
    defGenerator->SetTransform(trsf);
    try
      {
      defGenerator->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cerr << "ExceptionObject caught !" << std::endl;
      std::cerr << err << std::endl;
      exit(-1);
      }
    m_InitialDeformationField = defGenerator->GetOutput();
    // itkUtil::WriteImage<TDeformationField>(m_InitialDeformationField,
    // "initialDeformationfield.nii.gz");
#endif
    }
  else if ( m_InitialCoefficientFilename != "" )
    {
#ifdef __USE_BRAINS2_INTEGRATION
    DeformationFieldFFTType::Pointer mu;        // mu1, mu2, mu3;
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
#else
    std::cout << "ERROR:  InitialCoefficientFilename not supported yet" << std::endl;
    exit(-1);
#endif
    }

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
