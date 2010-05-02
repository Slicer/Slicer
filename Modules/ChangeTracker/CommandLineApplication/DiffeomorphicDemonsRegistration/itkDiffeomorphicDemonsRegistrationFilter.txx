#ifndef _itkDiffeomorphicDemonsRegistrationFilter_txx
#define _itkDiffeomorphicDemonsRegistrationFilter_txx
#include "itkDiffeomorphicDemonsRegistrationFilter.h"

#include "itkSmoothingRecursiveGaussianImageFilter.h"

namespace itk {

/*
 * Default constructor
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::DiffeomorphicDemonsRegistrationFilter()
{
 
  typename DemonsRegistrationFunctionType::Pointer drfp;
  drfp = DemonsRegistrationFunctionType::New();

  this->SetDifferenceFunction( static_cast<FiniteDifferenceFunctionType *>(
                                 drfp.GetPointer() ) );

  m_Multiplier = MultiplyByConstantType::New();
  m_Multiplier->InPlaceOn();

  m_Exponentiator = FieldExponentiatorType::New();
  
  m_Warper = VectorWarperType::New();
  FieldInterpolatorPointer VectorInterpolator =
     FieldInterpolatorType::New();
  m_Warper->SetInterpolator(VectorInterpolator);

  m_Adder = AdderType::New();
  m_Adder->InPlaceOn();
}


/*
 * Set the function state values before each iteration
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
void
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetMetric() const
{
 
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to DiffeomorphicDemonsRegistrationFunction" );
   }
   
  return drfp->GetMetric();
}

/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
double
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GetRMSChange() const
{
  DemonsRegistrationFunctionType *drfp = 
    dynamic_cast<DemonsRegistrationFunctionType *>
      (this->GetDifferenceFunction().GetPointer());
 
  if( !drfp )
   {
   itkExceptionMacro( << 
     "Could not cast difference function to DiffeomorphicDemonsRegistrationFunction" );
   }
   
  return drfp->GetRMSChange();
}


/*
 * 
 */
template <class TFixedImage, class TMovingImage, class TDeformationField>
typename DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::GradientType
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
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
     //m_Multiplier->UpdateLargestPossibleRegion();
     m_Multiplier->Update();
     // graft output back to this->GetUpdateBuffer()
     this->GetUpdateBuffer()->Graft( m_Multiplier->GetOutput() );
  }

  // compute the exponential
  m_Exponentiator->SetInput( this->GetUpdateBuffer() );
  const double imposedMaxUpStep = this->GetMaximumUpdateStepLength();
  if ( imposedMaxUpStep > 0.0 )
  {
     // max(norm(Phi))/2^N < 0.25*pixelspacing
     const double numiterfloat = 2.0 + vcl_log(imposedMaxUpStep)/vnl_math::ln2;
     unsigned int numiter = 0;
     if ( numiterfloat > 0.0 )
        numiter = static_cast<unsigned int>( 1.0 + numiterfloat );
     
     m_Exponentiator->AutomaticNumberOfIterationsOff();
     m_Exponentiator->SetMaximumNumberOfIterations( numiter );
  }
  else
  {
     m_Exponentiator->AutomaticNumberOfIterationsOn();
     m_Exponentiator->SetMaximumNumberOfIterations( 2000u ); // just a high value
  }

  // compose the vector fields
  m_Warper->SetOutputSpacing( this->GetUpdateBuffer()->GetSpacing() );
  m_Warper->SetOutputOrigin( this->GetUpdateBuffer()->GetOrigin() );
  m_Warper->SetInput( this->GetOutput() );
  m_Warper->SetDeformationField( m_Exponentiator->GetOutput() );

  m_Adder->SetInput1( m_Warper->GetOutput() );
  m_Adder->SetInput2( m_Exponentiator->GetOutput() );
  
  //m_Adder->UpdateLargestPossibleRegion();
  m_Adder->GetOutput()->SetRequestedRegion( this->GetOutput()->GetRequestedRegion() );
  m_Adder->Update();

  //std::cout<<"out buff spac: "<<this->GetOutput()->GetSpacing()<<std::endl;
  //std::cout<<"up buff spac: "<<this->GetUpdateBuffer()->GetSpacing()<<std::endl;
  //std::cout<<"exp out spac: "<<m_Exponentiator->GetOutput()->GetSpacing()<<std::endl;
  //std::cout<<"warp out spac: "<<m_Warper->GetOutput()->GetSpacing()<<std::endl;
  //std::cout<<"add out spac: "<<m_Adder->GetOutput()->GetSpacing()<<std::endl;
  
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

     //double var[DeformationFieldType::ImageDimension];
     //for (unsigned int i=0; i<DeformationFieldType::ImageDimension; ++i)
     //   var[i] = vnl_math_sqr( this->GetUpdateFieldStandardDeviations()[i] );
        
     //typedef SmoothingRecursiveGaussianImageFilter<DeformationFieldType,DeformationFieldType> GaussianFilterType;
     //typename GaussianFilterType::Pointer smoother = GaussianFilterType::New();
     //smoother->SetInput( this->GetOutput() );
     //smoother->SetVariance( var );
     //smoother->SetSigma( this->GetUpdateFieldStandardDeviations()[0] );
     //smoother->Update();

     //this->GraftOutput( smoother->GetOutput() );
  }
}

template <class TFixedImage, class TMovingImage, class TDeformationField>
void
DiffeomorphicDemonsRegistrationFilter<TFixedImage,TMovingImage,TDeformationField>
::PrintSelf(std::ostream& os, Indent indent) const
{ 
  Superclass::PrintSelf( os, indent );
  os << indent << "Intensity difference threshold: " <<
    this->GetIntensityDifferenceThreshold() << std::endl;
}


} // end namespace itk

#endif
