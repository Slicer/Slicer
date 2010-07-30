#ifndef __itkVectorMultiResolutionPDEDeformableRegistration_txx
#define __itkVectorMultiResolutionPDEDeformableRegistration_txx
#include "itkVectorMultiResolutionPDEDeformableRegistration.h"

#include "itkImageRegionIterator.h"
#include "vnl/vnl_math.h"

#include "itkVectorLinearInterpolateNearestNeighborExtrapolateImageFunction.h"

#include "itkImageToVectorImageFilter.h"
#include "itkVectorIndexSelectionCastImageFilter.h"

namespace itk
{
/*
 * Default constructor
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::VectorMultiResolutionPDEDeformableRegistration()
{
  this->SetNumberOfRequiredInputs(2);

  typename DefaultRegistrationType::Pointer registrator =
    DefaultRegistrationType::New();
  m_RegistrationFilter = static_cast< RegistrationType * >(
    registrator.GetPointer() );
  m_FixedVectorImagePyramid.reserve(10);
  m_MovingVectorImagePyramid.reserve(10);

  m_NumberOfLevels = 3;
  m_NumberOfIterations.resize(m_NumberOfLevels);
  m_FieldExpander     = FieldExpanderType::New();

  m_MovingImagePyramid  = MovingImagePyramidType::New();
  m_MovingImagePyramid->UseShrinkImageFilterOff();
  m_FixedImagePyramid     = FixedImagePyramidType::New();
  m_FixedImagePyramid->UseShrinkImageFilterOff();
  m_FixedImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
  m_MovingImagePyramid->SetNumberOfLevels(m_NumberOfLevels);

  for ( unsigned int i = 0; i < 3; i++ )
    {
    typename MovingImagePyramidType::Pointer movingImagePyramid = MovingImagePyramidType::New();
    movingImagePyramid->UseShrinkImageFilterOff();
    movingImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
    typename FixedImagePyramidType::Pointer fixedImagePyramid = FixedImagePyramidType::New();
    fixedImagePyramid->UseShrinkImageFilterOff();
    fixedImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
    m_MovingVectorImagePyramid.push_back(movingImagePyramid);
    m_FixedVectorImagePyramid.push_back(fixedImagePyramid);
    }

  unsigned int ilevel;
  for ( ilevel = 0; ilevel < m_NumberOfLevels; ilevel++ )
    {
    m_NumberOfIterations[ilevel] = 10;
    }
  m_CurrentLevel = 0;

  m_StopRegistrationFlag = false;
  m_InitialDeformationField = NULL;
}

/*
 * Set the moving image image.
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::SetMovingImage(
  const MovingImageType *ptr)
{
  this->ProcessObject::SetNthInput( 2, const_cast< MovingImageType * >( ptr ) );
}

/*
 * Get the moving image image.
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
const typename VectorMultiResolutionPDEDeformableRegistration< TFixedImage,
                                                               TMovingImage, TDeformationField, TRealType >
::MovingImageType *
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::GetMovingImage(void) const
{
  return dynamic_cast< const MovingImageType * >
         ( this->ProcessObject::GetInput(2) );
}

/*
 * Set the fixed image.
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::SetFixedImage(
  const FixedImageType *ptr)
{
  this->ProcessObject::SetNthInput( 1, const_cast< FixedImageType * >( ptr ) );
}

/*
 * Get the fixed image.
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
const typename VectorMultiResolutionPDEDeformableRegistration< TFixedImage,
                                                               TMovingImage, TDeformationField, TRealType >
::FixedImageType *
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::GetFixedImage(void) const
{
  return dynamic_cast< const FixedImageType * >
         ( this->ProcessObject::GetInput(1) );
}

template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
std::vector< SmartPointer< DataObject > >::size_type
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::GetNumberOfValidRequiredInputs() const
{
  typename std::vector< SmartPointer< DataObject > >::size_type num = 0;

  if ( this->GetFixedImage() )
    {
    num++;
    }

  if ( this->GetMovingImage() )
    {
    num++;
    }

  return num;
}

/*
 * Set the number of multi-resolution levels
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::SetNumberOfLevels(
  unsigned int num)
{
  if ( m_NumberOfLevels != num )
    {
    this->Modified();
    m_NumberOfLevels = num;
    m_NumberOfIterations.resize(m_NumberOfLevels);
    }

  if ( m_MovingImagePyramid && m_MovingImagePyramid->GetNumberOfLevels() != num )
    {
    m_MovingImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
    for ( unsigned int i = 0; i < this->GetMovingImage()->GetVectorLength(); i++ )
      {
      m_MovingVectorImagePyramid[i]->SetNumberOfLevels(m_NumberOfLevels);
      }
    }
  if ( m_FixedImagePyramid && m_FixedImagePyramid->GetNumberOfLevels() != num )
    {
    m_FixedImagePyramid->SetNumberOfLevels(m_NumberOfLevels);
    for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); i++ )
      {
      m_FixedVectorImagePyramid[i]->SetNumberOfLevels(m_NumberOfLevels);
      }
    }
}

/*
 * Standard PrintSelf method.
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "NumberOfLevels: " << m_NumberOfLevels << std::endl;
  os << indent << "CurrentLevel: " << m_CurrentLevel << std::endl;

  os << indent << "NumberOfIterations: [";
  unsigned int ilevel;
  for ( ilevel = 0; ilevel < m_NumberOfLevels - 1; ilevel++ )
    {
    os << m_NumberOfIterations[ilevel] << ", ";
    }
  os << m_NumberOfIterations[ilevel] << "]" << std::endl;

  os << indent << "RegistrationFilter: ";
  os << m_RegistrationFilter.GetPointer() << std::endl;
  os << indent << "MovingImagePyramid: ";
  os << m_MovingImagePyramid.GetPointer() << std::endl;
  os << indent << "FixedImagePyramid: ";
  os << m_FixedImagePyramid.GetPointer() << std::endl;

  os << indent << "StopRegistrationFlag: ";
  os << m_StopRegistrationFlag << std::endl;
}

/*
 * Perform a the deformable registration using a multiresolution scheme
 * using an internal mini-pipeline
 *
 *  ref_pyramid ->  registrator  ->  field_expander --|| tempField
 * test_pyramid ->           |                              |
 *                           |                              |
 *                           --------------------------------
 *
 * A tempField image is used to break the cycle between the
 * registrator and field_expander.
 *
 */
template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::GenerateData()
{
  // Check for NULL images and pointers
  MovingImageConstPointer movingVectorImage = this->GetMovingImage();
  FixedImageConstPointer  fixedVectorImage = this->GetFixedImage();

  if ( !movingVectorImage || !fixedVectorImage )
    {
    itkExceptionMacro(<< "Fixed and/or moving image not set");
    }

  if ( !m_MovingImagePyramid || !m_FixedImagePyramid )
    {
    itkExceptionMacro(<< "Fixed and/or moving pyramid not set");
    }

  if ( !m_RegistrationFilter )
    {
    itkExceptionMacro(<< "Registration filter not set");
    }

  if ( this->m_InitialDeformationField && this->GetInput(0) )
    {
    itkExceptionMacro(
      << "Only one initial deformation can be given. "
      << "SetInitialDeformationField should not be used in "
      <<
      "cunjunction with SetArbitraryInitialDeformationField "
      << "or SetInput.");
    }

  // Seperate the VectorInputImage to scalar Image

  typedef VectorIndexSelectionCastImageFilter< TFixedImage, FloatImageType > VectorIndexSelectionType;

  // Create the image pyramids.
  for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); ++i )
    {
    typename VectorIndexSelectionType::Pointer vectorFixedImageIndex = VectorIndexSelectionType::New();
    typename VectorIndexSelectionType::Pointer vectorMovingImageIndex = VectorIndexSelectionType::New();

    vectorFixedImageIndex->SetInput( this->GetFixedImage() );
    vectorFixedImageIndex->SetIndex(i);
    vectorFixedImageIndex->Update();

    vectorMovingImageIndex->SetInput( this->GetMovingImage() );
    vectorMovingImageIndex->SetIndex(i);
    vectorMovingImageIndex->Update();

    m_MovingImagePyramid->SetInput( vectorMovingImageIndex->GetOutput() );
    m_MovingImagePyramid->UpdateLargestPossibleRegion();

    /*
      m_MovingVectorImagePyramid[i]->SetSchedule(this->GetMovingImagePyramid()->GetSchedule());
      */
    m_MovingVectorImagePyramid[i]->SetInput( vectorMovingImageIndex->GetOutput() );
    m_MovingVectorImagePyramid[i]->UpdateLargestPossibleRegion();

    m_FixedImagePyramid->SetInput( vectorFixedImageIndex->GetOutput() );
    m_FixedImagePyramid->UpdateLargestPossibleRegion();

    /*
      m_FixedVectorImagePyramid[i]->SetSchedule(this->GetFixedImagePyramid()->GetSchedule());
      */
    m_FixedVectorImagePyramid[i]->SetInput( vectorFixedImageIndex->GetOutput() );
    m_FixedVectorImagePyramid[i]->UpdateLargestPossibleRegion();
    }
  // Initializations
  m_CurrentLevel = 0;
  m_StopRegistrationFlag = false;

  unsigned int movingLevel = vnl_math_min(
    (int)m_CurrentLevel, (int)m_MovingImagePyramid->GetNumberOfLevels() );

  unsigned int fixedLevel = vnl_math_min(
    (int)m_CurrentLevel, (int)m_FixedImagePyramid->GetNumberOfLevels() );

  DeformationFieldPointer tempField = NULL;

  DeformationFieldPointer inputPtr = const_cast< DeformationFieldType * >(
    this->GetInput(0) );

  if ( this->m_InitialDeformationField )
    {
    tempField = this->m_InitialDeformationField;
    }
  else if ( inputPtr )
    {
    // Arbitrary initial deformation field is set.
    // smooth it and resample

    // First smooth it
    tempField = inputPtr;

    typedef RecursiveGaussianImageFilter< DeformationFieldType,
                                          DeformationFieldType > GaussianFilterType;
    typename GaussianFilterType::Pointer smoother =
      GaussianFilterType::New();

    for ( unsigned int dim = 0;
          dim < DeformationFieldType::ImageDimension;
          ++dim )
      {
      // sigma accounts for the subsampling of the pyramid
      double sigma = 0.5 * static_cast< float >(
        m_FixedImagePyramid->GetSchedule()[fixedLevel][dim] );

      // but also for a possible discrepancy in the spacing
      sigma *= fixedVectorImage->GetSpacing()[dim]
               / inputPtr->GetSpacing()[dim];

      smoother->SetInput(tempField);
      smoother->SetSigma(sigma);
      smoother->SetDirection(dim);

      smoother->Update();

      tempField = smoother->GetOutput();
      tempField->DisconnectPipeline();
      }

    // Now resample
    m_FieldExpander->SetInput(tempField);

    typename FloatImageType::Pointer fi = m_FixedImagePyramid->GetOutput(
      fixedLevel);
    m_FieldExpander->SetSize( fi->GetLargestPossibleRegion().GetSize() );
    m_FieldExpander->SetOutputStartIndex(
      fi->GetLargestPossibleRegion().GetIndex() );
    m_FieldExpander->SetOutputOrigin( fi->GetOrigin() );
    m_FieldExpander->SetOutputSpacing( fi->GetSpacing() );
    m_FieldExpander->SetOutputDirection( fi->GetDirection() );

    m_FieldExpander->UpdateLargestPossibleRegion();
    m_FieldExpander->SetInput(NULL);
    tempField = m_FieldExpander->GetOutput();
    tempField->DisconnectPipeline();
    }

  bool lastShrinkFactorsAllOnes = false;

  while ( !this->Halt() )
    {
    if ( tempField.IsNull() )
      {
      m_RegistrationFilter->SetInitialDeformationField(NULL);
      }
    else
      {
      // Resample the field to be the same size as the fixed image
      // at the current level
      m_FieldExpander->SetInput(tempField);

      typename FloatImageType::Pointer fi =
        m_FixedImagePyramid->GetOutput(fixedLevel);
      m_FieldExpander->SetSize(
        fi->GetLargestPossibleRegion().GetSize() );
      m_FieldExpander->SetOutputStartIndex(
        fi->GetLargestPossibleRegion().GetIndex() );
      m_FieldExpander->SetOutputOrigin( fi->GetOrigin() );
      m_FieldExpander->SetOutputSpacing( fi->GetSpacing() );
      m_FieldExpander->SetOutputDirection( fi->GetDirection() );

      m_FieldExpander->UpdateLargestPossibleRegion();
      m_FieldExpander->SetInput(NULL);
      tempField = m_FieldExpander->GetOutput();
      tempField->DisconnectPipeline();

      m_RegistrationFilter->SetInitialDeformationField(tempField);
      }

    typedef itk::ImageToVectorImageFilter< FloatImageType >
    ImageToVectorImageType;
    typename ImageToVectorImageType::Pointer vectorFixedImage =
      ImageToVectorImageType::New();
    typename ImageToVectorImageType::Pointer vectorMovingImage =
      ImageToVectorImageType::New();
    for ( unsigned int i = 0; i < this->GetFixedImage()->GetVectorLength(); ++i )
      {
      vectorFixedImage->SetNthInput( i,
                                     m_FixedVectorImagePyramid[i]->GetOutput(fixedLevel) );
      vectorMovingImage->SetNthInput( i,
                                      m_MovingVectorImagePyramid[i]->GetOutput(movingLevel) );
      }
    try
      {
      vectorFixedImage->Update();
      vectorMovingImage->Update();
      }
    catch ( itk::ExceptionObject & err )
      {
      std::cout << "Caught an exception: " << std::endl;
      std::cout << err << " " << __FILE__ << " " << __LINE__ << std::endl;
      throw err;
      throw err;
      }
    catch ( ... )
      {
      std::cout << "Caught a non-ITK exception " << __FILE__ << " "
                << __LINE__ << std::endl;
      }

    // setup registration filter and pyramids
    m_RegistrationFilter->SetMovingImage( vectorMovingImage->GetOutput() );
    m_RegistrationFilter->SetFixedImage( vectorFixedImage->GetOutput() );

    m_RegistrationFilter->SetNumberOfIterations(m_NumberOfIterations[
                                                  m_CurrentLevel]);

    // cache shrink factors for computing the next expand factors.
    lastShrinkFactorsAllOnes = true;
    for ( unsigned int idim = 0; idim < ImageDimension; idim++ )
      {
      if ( m_FixedImagePyramid->GetSchedule()[fixedLevel][idim] > 1 )
        {
        lastShrinkFactorsAllOnes = false;
        break;
        }
      }

    // compute new deformation field
    m_RegistrationFilter->UpdateLargestPossibleRegion();
    tempField = m_RegistrationFilter->GetOutput();
    tempField->DisconnectPipeline();

    // Increment level counter.
    m_CurrentLevel++;
    movingLevel = vnl_math_min( (int)m_CurrentLevel,
                                (int)m_MovingImagePyramid->GetNumberOfLevels() );
    fixedLevel = vnl_math_min( (int)m_CurrentLevel,
                               (int)m_FixedImagePyramid->GetNumberOfLevels() );

    // Invoke an iteration event.
    this->InvokeEvent( IterationEvent() );

    // We can release data from pyramid which are no longer required.
    if ( movingLevel > 0 )
      {
      m_MovingImagePyramid->GetOutput(movingLevel - 1)->ReleaseData();
      }
    if ( fixedLevel > 0 )
      {
      m_FixedImagePyramid->GetOutput(fixedLevel - 1)->ReleaseData();
      }
    } // while not Halt()

  if ( !lastShrinkFactorsAllOnes )
    {
    // Some of the last shrink factors are not one
    // graft the output of the expander filter to
    // to output of this filter

    // resample the field to the same size as the fixed image
    m_FieldExpander->SetInput(tempField);
    m_FieldExpander->SetSize(
      fixedVectorImage->GetLargestPossibleRegion().GetSize() );
    m_FieldExpander->SetOutputStartIndex(
      fixedVectorImage->GetLargestPossibleRegion().GetIndex() );
    m_FieldExpander->SetOutputOrigin( fixedVectorImage->GetOrigin() );
    m_FieldExpander->SetOutputSpacing( fixedVectorImage->GetSpacing() );
    m_FieldExpander->SetOutputDirection( fixedVectorImage->GetDirection() );

    m_FieldExpander->UpdateLargestPossibleRegion();
    this->GraftOutput( m_FieldExpander->GetOutput() );
    }
  else
    {
    // all the last shrink factors are all ones
    // graft the output of registration filter to
    // to output of this filter
    this->GraftOutput(tempField);
    }

  // Release memory
  m_FieldExpander->SetInput(NULL);
  m_FieldExpander->GetOutput()->ReleaseData();
  m_RegistrationFilter->SetInput(NULL);
  m_RegistrationFilter->GetOutput()->ReleaseData();
}

template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::StopRegistration()
{
  m_RegistrationFilter->StopRegistration();
  m_StopRegistrationFlag = true;
}

template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
bool
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::Halt()
{
  // Halt the registration after the user-specified number of levels
  if ( m_NumberOfLevels != 0 )
    {
    this->UpdateProgress( static_cast< float >( m_CurrentLevel )
                          / static_cast< float >( m_NumberOfLevels ) );
    }

  if ( m_CurrentLevel >= m_NumberOfLevels )
    {
    return true;
    }
  if ( m_StopRegistrationFlag )
    {
    return true;
    }
  else
    {
    return false;
    }
}

template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::GenerateOutputInformation()
{
  typename DataObject::Pointer output;

  if ( this->GetInput(0) )
    {
    // Initial deformation field is set.
    // Copy information from initial field.
    this->Superclass::GenerateOutputInformation();
    }
  else if ( this->GetFixedImage() )
    {
    // Initial deforamtion field is not set.
    // Copy information from the fixed image.
    for ( unsigned int idx = 0; idx <
          this->GetNumberOfOutputs(); ++idx )
      {
      output = this->GetOutput(idx);
      if ( output )
        {
        output->CopyInformation( this->GetFixedImage() );
        }
      }
    }
}

template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::GenerateInputRequestedRegion()
{
  // call the superclass's implementation
  Superclass::GenerateInputRequestedRegion();

  // request the largest possible region for the moving image
  MovingImagePointer movingPtr =
    const_cast< MovingImageType * >( this->GetMovingImage() );
  if ( movingPtr )
    {
    movingPtr->SetRequestedRegionToLargestPossibleRegion();
    }

  // just propagate up the output requested region for
  // the fixed image and initial deformation field.
  DeformationFieldPointer inputPtr =
    const_cast< DeformationFieldType * >( this->GetInput() );
  DeformationFieldPointer outputPtr = this->GetOutput();
  FixedImagePointer       fixedPtr =
    const_cast< FixedImageType * >( this->GetFixedImage() );

  if ( inputPtr )
    {
    inputPtr->SetRequestedRegion( outputPtr->GetRequestedRegion() );
    }

  if ( fixedPtr )
    {
    fixedPtr->SetRequestedRegion( outputPtr->GetRequestedRegion() );
    }
}

template< class TFixedImage, class TMovingImage, class TDeformationField,
          class TRealType >
void
VectorMultiResolutionPDEDeformableRegistration< TFixedImage, TMovingImage,
                                                TDeformationField, TRealType >
::EnlargeOutputRequestedRegion(
  DataObject *ptr)
{
  // call the superclass's implementation
  Superclass::EnlargeOutputRequestedRegion(ptr);

  // set the output requested region to largest possible.
  DeformationFieldType *outputPtr;
  outputPtr = dynamic_cast< DeformationFieldType * >( ptr );

  if ( outputPtr )
    {
    outputPtr->SetRequestedRegionToLargestPossibleRegion();
    }
}
} // end namespace itk

#endif
