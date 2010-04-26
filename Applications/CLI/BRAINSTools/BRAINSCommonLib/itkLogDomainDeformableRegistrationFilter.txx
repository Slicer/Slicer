#ifndef __itkLogDomainDeformableRegistrationFilter_txx
#define __itkLogDomainDeformableRegistrationFilter_txx

#include "itkLogDomainDeformableRegistrationFilter.h"

#include "itkExceptionObject.h"
#include "itkImageRegionIterator.h"
#include "itkImageLinearIteratorWithIndex.h"
#include "itkDataObject.h"

#include "itkGaussianOperator.h"
#include "itkVectorNeighborhoodOperatorImageFilter.h"

#include "vnl/vnl_math.h"

namespace itk {

// Default constructor
template <class TFixedImage, class TMovingImage, class TField>
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::LogDomainDeformableRegistrationFilter()
{
 
  this->SetNumberOfRequiredInputs(2);

  this->SetNumberOfIterations(10);
 
  unsigned int j;
  for( j = 0; j < ImageDimension; j++ )
    {
    m_StandardDeviations[j] = 1.0;
    m_UpdateFieldStandardDeviations[j] = 1.0;
    }

  m_TempField = VelocityFieldType::New();
  m_MaximumError = 0.1;
  m_MaximumKernelWidth = 30;
  m_StopRegistrationFlag = false;

  m_SmoothVelocityField = true;
  m_SmoothUpdateField = false;

  m_Exponentiator = FieldExponentiatorType::New();
  m_Exponentiator->ComputeInverseOff();
  
  m_InverseExponentiator = FieldExponentiatorType::New();
  m_InverseExponentiator->ComputeInverseOn();
}


// Set the fixed image.
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SetFixedImage(
  const FixedImageType * ptr )
{
  this->ProcessObject::SetNthInput( 1, const_cast< FixedImageType * >( ptr ) );
}


// Get the fixed image.
template <class TFixedImage, class TMovingImage, class TField>
const typename LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::FixedImageType *
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GetFixedImage() const
{
  return dynamic_cast< const FixedImageType * >
    ( this->ProcessObject::GetInput( 1 ) );
}


// Set the moving image.
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SetMovingImage(
  const MovingImageType * ptr )
{
  this->ProcessObject::SetNthInput( 2, const_cast< MovingImageType * >( ptr ) );
}


// Get the moving image.
template <class TFixedImage, class TMovingImage, class TField>
const typename LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::MovingImageType *
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GetMovingImage() const
{
  return dynamic_cast< const MovingImageType * >
    ( this->ProcessObject::GetInput( 2 ) );
}


template <class TFixedImage, class TMovingImage, class TField>
std::vector<SmartPointer<DataObject> >::size_type
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GetNumberOfValidRequiredInputs() const
{
  typename std::vector<SmartPointer<DataObject> >::size_type num = 0;

  if (this->GetFixedImage())
    {
    num++;
    }

  if (this->GetMovingImage())
    {
    num++;
    }
  
  return num;
}


// Set the standard deviations.
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SetStandardDeviations(
  double value )
{

  unsigned int j;
  for( j = 0; j < ImageDimension; j++ )
    {
    if( value != m_StandardDeviations[j] )
      {
      break;
      }
    }
  if( j < ImageDimension )
    {
    this->Modified();
    for( j = 0; j < ImageDimension; j++ )
      {
      m_StandardDeviations[j] = value;
      }
    }

}

// Set the standard deviations.
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SetUpdateFieldStandardDeviations(
  double value )
{

  unsigned int j;
  for( j = 0; j < ImageDimension; j++ )
    {
    if( value != m_UpdateFieldStandardDeviations[j] )
      {
      break;
      }
    }
  if( j < ImageDimension )
    {
    this->Modified();
    for( j = 0; j < ImageDimension; j++ )
      {
      m_UpdateFieldStandardDeviations[j] = value;
      }
    }

}


// Standard PrintSelf method.
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Smooth velocity field: "
     << (m_SmoothVelocityField ? "on" : "off") << std::endl;
  os << indent << "Standard deviations: [";
  unsigned int j;
  for( j = 0; j < ImageDimension - 1; j++ )
    {
    os << m_StandardDeviations[j] << ", ";
    }
  os << m_StandardDeviations[j] << "]" << std::endl;
  os << indent << "Smooth update field: "
     << (m_SmoothUpdateField ? "on" : "off") << std::endl;
  os << indent << "Update field standard deviations: [";
  for( j = 0; j < ImageDimension - 1; j++ )
    {
    os << m_UpdateFieldStandardDeviations[j] << ", ";
    }
  os << m_UpdateFieldStandardDeviations[j] << "]" << std::endl;
  os << indent << "StopRegistrationFlag: ";
  os << m_StopRegistrationFlag << std::endl;
  os << indent << "MaximumError: ";
  os << m_MaximumError << std::endl;
  os << indent << "MaximumKernelWidth: ";
  os << m_MaximumKernelWidth << std::endl;
  os << indent << "Exponentiator: ";
  os << m_Exponentiator << std::endl;
  os << indent << "InverseExponentiator: ";
  os << m_InverseExponentiator << std::endl;

}


// Set the function state values before each iteration
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::InitializeIteration()
{
  //std::cout<<"LogDomainDeformableRegistrationFilter::InitializeIteration"<<std::endl;
  MovingImageConstPointer movingPtr = this->GetMovingImage();
  FixedImageConstPointer fixedPtr = this->GetFixedImage();

  if( !movingPtr || !fixedPtr )
    {
    itkExceptionMacro( << "Fixed and/or moving image not set" );
    }

  // update variables in the equation object
  PDEDeformableRegistrationFunctionType *f = 
    dynamic_cast<PDEDeformableRegistrationFunctionType *>
    (this->GetDifferenceFunction().GetPointer());

  if ( !f )
    {
    itkExceptionMacro(<<"FiniteDifferenceFunction not of type LogDomainDeformableRegistrationFilterFunction");
    }

  f->SetFixedImage( fixedPtr );
  f->SetMovingImage( movingPtr );

  this->Superclass::InitializeIteration();

}


/* Override the default implementation for the case when the 
 * initial velocity is not set.
 * If the initial velocity is not set, the output is
 * fill with zero vectors.*/
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::CopyInputToOutput()
{

  typename Superclass::InputImageType::ConstPointer  inputPtr  = this->GetInput();
  
  if( inputPtr )
    {
    this->Superclass::CopyInputToOutput();
    }
  else
    {
    typename Superclass::PixelType zeros;
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      zeros[j] = 0;
      }

    typename OutputImageType::Pointer output = this->GetOutput();
  
    ImageRegionIterator<OutputImageType> out(output, output->GetRequestedRegion());

    while( ! out.IsAtEnd() )
      {
      out.Value() =  zeros;
      ++out;
      }
    }
}


template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GenerateOutputInformation()
{
  //std::cout<<"LogDomainDeformableRegistrationFilter::GenerateOutputInformation"<<std::endl;
  typename DataObject::Pointer output;

  if( this->GetInput(0) )
    {
    // Initial velocity field is set.
    // Copy information from initial field.
    this->Superclass::GenerateOutputInformation();

    }
  else if( this->GetFixedImage() )
    {
    // Initial deforamtion field is not set. 
    // Copy information from the fixed image.
    for (unsigned int idx = 0; idx < 
           this->GetNumberOfOutputs(); ++idx )
      {
      output = this->GetOutput(idx);
      if (output)
        {
        output->CopyInformation(this->GetFixedImage());
        }  
      }

    }

}


template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GenerateInputRequestedRegion()
{
  //std::cout<<"LogDomainDeformableRegistrationFilter::GenerateInputRequestedRegion"<<std::endl;
  // call the superclass's implementation
  Superclass::GenerateInputRequestedRegion();

  // request the largest possible region for the moving image
  MovingImagePointer movingPtr = 
    const_cast< MovingImageType * >( this->GetMovingImage() );
  if( movingPtr )
    {
    movingPtr->SetRequestedRegionToLargestPossibleRegion();
    }
  
  // just propagate up the output requested region for
  // the fixed image and initial velocity field.
  VelocityFieldPointer inputPtr = 
    const_cast< VelocityFieldType * >( this->GetInput() );
  VelocityFieldPointer outputPtr = this->GetOutput();
  FixedImagePointer fixedPtr = 
    const_cast< FixedImageType *>( this->GetFixedImage() );

  if( inputPtr )
    {
    inputPtr->SetRequestedRegion( outputPtr->GetRequestedRegion() );
    }

  if( fixedPtr )
    {
    fixedPtr->SetRequestedRegion( outputPtr->GetRequestedRegion() );
    }
}


// Release memory of internal buffers
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::PostProcessOutput()
{
  this->Superclass::PostProcessOutput();
  m_TempField->Initialize();
}


// Initialize flags
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::Initialize()
{
  //std::cout<<"LogDomainDeformableRegistrationFilter::Initialize"<<std::endl;
  this->Superclass::Initialize();
  m_StopRegistrationFlag = false;
}


// Smooth velocity using a separable Gaussian kernel
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SmoothVelocityField()
{
  // The output buffer will be overwritten with new data.
  this->SmoothGivenField(this->GetOutput(), this->m_StandardDeviations);
}

// Smooth update field using a separable Gaussian kernel
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SmoothUpdateField()
{
  // The update buffer will be overwritten with new data.
  this->SmoothGivenField(this->GetUpdateBuffer(), this->m_UpdateFieldStandardDeviations);
}


// Smooth velocity using a separable Gaussian kernel
template <class TFixedImage, class TMovingImage, class TField>
void
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::SmoothGivenField(VelocityFieldType * field, const double StandardDeviations[ImageDimension])
{
  // copy field to TempField
  m_TempField->SetOrigin( field->GetOrigin() );
  m_TempField->SetSpacing( field->GetSpacing() );
  m_TempField->SetDirection( field->GetDirection() );
  m_TempField->SetLargestPossibleRegion( 
    field->GetLargestPossibleRegion() );
  m_TempField->SetRequestedRegion(
    field->GetRequestedRegion() );
  m_TempField->SetBufferedRegion( field->GetBufferedRegion() );
  m_TempField->Allocate();
  
  typedef typename VelocityFieldType::PixelType       VectorType;
  typedef typename VectorType::ValueType              ScalarType;
  typedef GaussianOperator<ScalarType,ImageDimension> OperatorType;
  typedef VectorNeighborhoodOperatorImageFilter<
    VelocityFieldType,
    VelocityFieldType>                                SmootherType;

  OperatorType * oper = new OperatorType;
  typename SmootherType::Pointer smoother = SmootherType::New();

  typedef typename VelocityFieldType::PixelContainerPointer 
    PixelContainerPointer;
  PixelContainerPointer swapPtr;

  // graft the output field onto the mini-pipeline
  smoother->GraftOutput( m_TempField );

  for( unsigned int j = 0; j < ImageDimension; j++ )
    {
    // smooth along this dimension
    oper->SetDirection( j );
    double variance = vnl_math_sqr( StandardDeviations[j] );
    oper->SetVariance( variance );
    oper->SetMaximumError( m_MaximumError );
    oper->SetMaximumKernelWidth( m_MaximumKernelWidth );
    oper->CreateDirectional();

    // todo: make sure we only smooth within the buffered region
    smoother->SetOperator( *oper );
    smoother->SetInput( field );
    smoother->Update();

    if ( j < ImageDimension - 1 )
      {
      // swap the containers
      swapPtr = smoother->GetOutput()->GetPixelContainer();
      smoother->GraftOutput( field );
      field->SetPixelContainer( swapPtr );
      smoother->Modified();
      }

    }

  // graft the output back to field
  m_TempField->SetPixelContainer( field->GetPixelContainer() );

  // field to contain the final smoothed data, do the equivalent of a graft
  field->SetPixelContainer( smoother->GetOutput()->GetPixelContainer() );
  field->SetRequestedRegion( smoother->GetOutput()->GetRequestedRegion() );
  field->SetBufferedRegion( smoother->GetOutput()->GetBufferedRegion() );
  field->SetLargestPossibleRegion( smoother->GetOutput()->GetLargestPossibleRegion() );
  field->CopyInformation( smoother->GetOutput() );

  delete oper;

}


template <class TFixedImage, class TMovingImage, class TField>
typename LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::DeformationFieldPointer
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GetDeformationField()
{
  //std::cout<<"LogDomainDeformableRegistration::GetDeformationField"<<std::endl;
  m_Exponentiator->SetInput( this->GetVelocityField() );
  m_Exponentiator->GetOutput()->SetRequestedRegion( this->GetVelocityField()->GetRequestedRegion() );
  m_Exponentiator->Update();
  return m_Exponentiator->GetOutput();
}


template <class TFixedImage, class TMovingImage, class TField>
typename LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::DeformationFieldPointer
LogDomainDeformableRegistrationFilter<TFixedImage,TMovingImage,TField>
::GetInverseDeformationField()
{
  //std::cout<<"LogDomainDeformableRegistration::GetInverseDeformationField"<<std::endl;
  m_InverseExponentiator->SetInput( this->GetVelocityField() );
  m_InverseExponentiator->GetOutput()->SetRequestedRegion( this->GetVelocityField()->GetRequestedRegion() );
  m_InverseExponentiator->Update();
  return m_InverseExponentiator->GetOutput();
}


} // end namespace itk

#endif
