#ifndef _itkFastSymmetricForcesDemonsRegistrationFilter_txx
#define _itkFastSymmetricForcesDemonsRegistrationFilter_txx
#include "itkFastSymmetricForcesDemonsRegistrationFilter.h"

namespace itk {

/*
 * Default constructor
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::FastSymmetricForcesDemonsRegistrationFilter()
{
  typename DemonsRegistrationFunctionType::Pointer drfp;
  drfp = DemonsRegistrationFunctionType::New();

  this->SetDifferenceFunction( static_cast<FiniteDifferenceFunctionType *>(
                                 drfp.GetPointer() ) );

  m_Multiplier = MultiplyByConstantType::New();
  m_Multiplier->InPlaceOn();

  m_Adder = AdderType::New();
  m_Adder->InPlaceOn();
}


/*
 * Set the function state values before each iteration
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::InitializeIteration()
{
  // update variables in the equation object
  DemonsRegistrationFunctionType *f = 
    dynamic_cast<DemonsRegistrationFunctionType *>
    (this->GetDifferenceFunction().GetPointer());

  if ( !f )
    {
    itkExceptionMacro(<<"FiniteDifferenceFunction not of type DemonsRegistrationFunctionType");
    }

  f->SetDeformationField( this->GetDeformationField() );

  // call the superclass  implementation ( initializes f )
  Superclass::InitializeIteration();
}


/*
 * Get the metric value from the difference function
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
double
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetMetric() const
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to FastSymmetricForcesDemonsRegistrationFunction" );
   }
   
  return drfp->GetMetric();
}

/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
double
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetIntensityDifferenceThreshold() const
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to DemonsRegistrationFunction" );
   }
   
  return drfp->GetIntensityDifferenceThreshold();
}

/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::SetIntensityDifferenceThreshold(double threshold) 
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to SymmetricDemonsRegistrationFunction" );
   }
   
  drfp->SetIntensityDifferenceThreshold(threshold);
}


/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
double
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetMaximumUpdateStepLength() const
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to DemonsRegistrationFunction" );
   }
  
  return drfp->GetMaximumUpdateStepLength();
}

/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::SetMaximumUpdateStepLength(double threshold) 
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to SymmetricDemonsRegistrationFunction" );
   }
  
  drfp->SetMaximumUpdateStepLength(threshold);
}


/*
 * Get the metric value from the difference function
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
const double &
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetRMSChange() const
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to FastSymmetricForcesDemonsRegistrationFunction" );
   }
   
  return drfp->GetRMSChange();
}


/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
typename FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GradientType
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetUseGradientType() const
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to DemonsRegistrationFunction" );
   }
  
  return drfp->GetUseGradientType();
}

/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::SetUseGradientType(GradientType gtype) 
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to SymmetricDemonsRegistrationFunction" );
   }
  
  drfp->SetUseGradientType(gtype);
}


template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::AllocateUpdateBuffer()
{
  // The update buffer looks just like the output.
  DeformationFieldPointer output = this->GetOutput();
  DeformationFieldPointer upbuf = this->GetUpdateBuffer();

  upbuf->SetLargestPossibleRegion(output->GetLargestPossibleRegion());
  upbuf->SetRequestedRegion(output->GetRequestedRegion());
  upbuf->SetBufferedRegion(output->GetBufferedRegion());
  upbuf->SetSpacing(output->GetSpacing());
  upbuf->SetOrigin(output->GetOrigin());
  upbuf->Allocate();
}


/*
 * Get the metric value from the difference function
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::ApplyUpdate(TimeStepType dt)
{
   this->GetUpdateBuffer()->Modified();
   
  // If we smooth the update buffer before applying it, then the are
  // approximating a viscuous problem as opposed to an elastic problem
  if ( this->GetSmoothUpdateField() )
    {
    this->SmoothUpdateField();
    }

  // use time step if necessary
  if ( fabs(dt - 1.0)>1.0e-4 )
  {
     std::cout<<"Using timestep: "<<dt<<std::endl;
     m_Multiplier->SetConstant( dt );
     m_Multiplier->SetInput( this->GetUpdateBuffer() );
     m_Multiplier->GraftOutput( this->GetUpdateBuffer() );
     // in place update
     m_Multiplier->Update();
     // graft output back to this->GetUpdateBuffer()
     this->GetUpdateBuffer()->Graft( m_Multiplier->GetOutput() );
  }
  
  m_Adder->SetInput1( this->GetOutput() );
  m_Adder->SetInput2( this->GetUpdateBuffer() );

  m_Adder->GetOutput()->SetRequestedRegion( this->GetOutput()->GetRequestedRegion() );
  m_Adder->Update();
  
  // Region passing stuff
  this->GraftOutput( m_Adder->GetOutput() );

  

  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to DemonsRegistrationFunction" );
   }

  this->SetRMSChange( drfp->GetRMSChange() );

  /*
   * Smooth the deformation field
   */
  if ( this->GetSmoothDeformationField() )
  {
     this->SmoothDeformationField();
  }
}

template <class TFixedImage, class TMovingImage, class TDeformationField>
void
FastSymmetricForcesDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::PrintSelf(std::ostream& os, Indent indent) const
{ 
  Superclass::PrintSelf( os, indent );
  os << indent << "Intensity difference threshold: " <<
    this->GetIntensityDifferenceThreshold() << std::endl;
}


} // end namespace itk

#endif
