/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile: itkDiffeomorphicDemonsRegistrationWithMaskFilter.txx,v
 * $
 *  Language:  C++
 *  Date:      $Date: 2008-11-07 19:39:44 $
 *  Version:   $Revision: 1.7 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *    This software is distributed WITHOUT ANY WARRANTY; without even
 *    the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *    PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/

#ifndef __itkDiffeomorphicDemonsRegistrationWithMaskFilter_txx
#define __itkDiffeomorphicDemonsRegistrationWithMaskFilter_txx

#include "itkDiffeomorphicDemonsRegistrationWithMaskFilter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"

namespace itk
{
/**
  * Default constructor
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::DiffeomorphicDemonsRegistrationWithMaskFilter():
  m_UseFirstOrderExp(false)
{
  typename DemonsRegistrationFunctionType::Pointer drfp;

  drfp = DemonsRegistrationFunctionType::New();

  this->SetDifferenceFunction( static_cast< FiniteDifferenceFunctionType * >(
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

/**
  * Checks whether the DifferenceFunction is of type DemonsRegistrationFunction.
  * It throws and exception, if it is not.
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
typename DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage,
                                                        TMovingImage,
                                                        TDeformationField >::DemonsRegistrationFunctionType *
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::DownCastDifferenceFunctionType()
{
  DemonsRegistrationFunctionType *drfp =
    dynamic_cast< DemonsRegistrationFunctionType * >
    ( this->GetDifferenceFunction().GetPointer() );

  if ( !drfp )
    {
    itkExceptionMacro(
      <<
      "Could not cast difference function to SymmetricDemonsRegistrationFunction");
    }

  return drfp;
}

/**
  * Checks whether the DifferenceFunction is of type DemonsRegistrationFunction.
  * It throws and exception, if it is not.
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
const typename DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage,
                                                              TMovingImage,
                                                              TDeformationField >::DemonsRegistrationFunctionType *
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::DownCastDifferenceFunctionType() const
{
  const DemonsRegistrationFunctionType *drfp =
    dynamic_cast< const DemonsRegistrationFunctionType * >
    ( this->GetDifferenceFunction().GetPointer() );

  if ( !drfp )
    {
    itkExceptionMacro(
      <<
      "Could not cast difference function to SymmetricDemonsRegistrationFunction");
    }

  return drfp;
}

/**
  * Set the function state values before each iteration
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::InitializeIteration()
{
  // update variables in the equation object
  DemonsRegistrationFunctionType *f = this->DownCastDifferenceFunctionType();

  f->SetDeformationField( this->GetDeformationField() );

  // call the superclass  implementation ( initializes f )
  Superclass::InitializeIteration();
}

/*
  * Get the metric value from the difference function
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
double
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetMetric() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetMetric();
}

/**
  *  Get Intensity Difference Threshold
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
double
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetIntensityDifferenceThreshold() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetIntensityDifferenceThreshold();
}

/**
  *  Set Intensity Difference Threshold
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::SetIntensityDifferenceThreshold(double threshold)
{
  DemonsRegistrationFunctionType *drfp = this->DownCastDifferenceFunctionType();

  drfp->SetIntensityDifferenceThreshold(threshold);
}

/**
  *  Get Maximum Update Step Length
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
double
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetMaximumUpdateStepLength() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetMaximumUpdateStepLength();
}

/**
  *  Set Maximum Update Step Length
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::SetMaximumUpdateStepLength(double threshold)
{
  DemonsRegistrationFunctionType *drfp = this->DownCastDifferenceFunctionType();

  drfp->SetMaximumUpdateStepLength(threshold);
}

/**
  * Get the metric value from the difference function
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
const double &
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetRMSChange() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetRMSChange();
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
typename DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage,
                                                        TMovingImage, TDeformationField >
::GradientType
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetUseGradientType() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetUseGradientType();
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::SetUseGradientType(GradientType gtype)
{
  DemonsRegistrationFunctionType *drfp = this->DownCastDifferenceFunctionType();

  drfp->SetUseGradientType(gtype);
}

template< class TFixedImage, class TMovingImage, class TDeformationField >
const
typename DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage,
                                                        TMovingImage, TDeformationField >::MaskType *
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetMovingImageMask() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetMovingImageMask();
}

template< class TFixedImage, class TMovingImage, class TDeformationField >
const
typename DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage,
                                                        TMovingImage, TDeformationField >::MaskType *
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::GetFixedImageMask() const
{
  const DemonsRegistrationFunctionType *drfp =
    this->DownCastDifferenceFunctionType();

  return drfp->GetFixedImageMask();
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::SetMovingImageMask(MaskType *mask)
{
  DemonsRegistrationFunctionType *drfp = this->DownCastDifferenceFunctionType();

  drfp->SetMovingImageMask(mask);
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::SetFixedImageMask(MaskType *mask)
{
  DemonsRegistrationFunctionType *drfp = this->DownCastDifferenceFunctionType();

  drfp->SetFixedImageMask(mask);
}

/**
  *
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::AllocateUpdateBuffer()
{
  // The update buffer looks just like the output.
  DeformationFieldPointer output = this->GetOutput();
  DeformationFieldPointer upbuf = this->GetUpdateBuffer();

  upbuf->SetLargestPossibleRegion( output->GetLargestPossibleRegion() );
  upbuf->SetRequestedRegion( output->GetRequestedRegion() );
  upbuf->SetBufferedRegion( output->GetBufferedRegion() );
  upbuf->SetOrigin( output->GetOrigin() );
  upbuf->SetSpacing( output->GetSpacing() );
  upbuf->SetDirection( output->GetDirection() );
  upbuf->Allocate();
}

/**
  * Get the metric value from the difference function
  */
template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::ApplyUpdate(TimeStepType dt)
{
  // If we smooth the update buffer before applying it, then the are
  // approximating a viscuous problem as opposed to an elastic problem
  if ( this->GetSmoothUpdateField() )
    {
    this->SmoothUpdateField();
    }

  // Use time step if necessary. In many cases
  // the time step is one so this will be skipped
  if ( fabs(dt - 1.0) > 1.0e-4 )
    {
    itkDebugMacro("Using timestep: " << dt);
    m_Multiplier->SetConstant(dt);
    m_Multiplier->SetInput( this->GetUpdateBuffer() );
    m_Multiplier->GraftOutput( this->GetUpdateBuffer() );
    // in place update
    m_Multiplier->Update();
    // graft output back to this->GetUpdateBuffer()
    this->GetUpdateBuffer()->Graft( m_Multiplier->GetOutput() );
    }

  if ( this->m_UseFirstOrderExp )
    {
    // use s <- s o (Id +u)

    // skip exponential and compose the vector fields
    m_Warper->SetOutputOrigin( this->GetUpdateBuffer()->GetOrigin() );
    m_Warper->SetOutputSpacing( this->GetUpdateBuffer()->GetSpacing() );
    m_Warper->SetOutputDirection( this->GetUpdateBuffer()->GetDirection() );
    m_Warper->SetInput( this->GetOutput() );
    m_Warper->SetDeformationField( this->GetUpdateBuffer() );

    m_Adder->SetInput1( m_Warper->GetOutput() );
    m_Adder->SetInput2( this->GetUpdateBuffer() );

    m_Adder->GetOutput()->SetRequestedRegion(
      this->GetOutput()->GetRequestedRegion() );
    }
  else
    {
    // use s <- s o exp(u)

    // compute the exponential
    m_Exponentiator->SetInput( this->GetUpdateBuffer() );

    const double imposedMaxUpStep = this->GetMaximumUpdateStepLength();
    if ( imposedMaxUpStep > 0.0 )
      {
      // max(norm(Phi))/2^N <= 0.25*pixelspacing
      const double numiterfloat = 2.0 + vcl_log(imposedMaxUpStep)
                                  / vnl_math::ln2;
      unsigned int numiter = 0;
      if ( numiterfloat > 0.0 )
        {
        numiter = static_cast< unsigned int >( vcl_ceil(numiterfloat) );
        }

      m_Exponentiator->AutomaticNumberOfIterationsOff();
      m_Exponentiator->SetMaximumNumberOfIterations(numiter);
      }
    else
      {
      m_Exponentiator->AutomaticNumberOfIterationsOn();
      // just set a high value so that automatic number of step
      // is not thresholded
      m_Exponentiator->SetMaximumNumberOfIterations(2000u);
      }

    m_Exponentiator->GetOutput()->SetRequestedRegion(
      this->GetOutput()->GetRequestedRegion() );

    m_Exponentiator->Update();

    // compose the vector fields
    m_Warper->SetOutputOrigin( this->GetUpdateBuffer()->GetOrigin() );
    m_Warper->SetOutputSpacing( this->GetUpdateBuffer()->GetSpacing() );
    m_Warper->SetOutputDirection( this->GetUpdateBuffer()->GetDirection() );
    m_Warper->SetInput( this->GetOutput() );
    m_Warper->SetDeformationField( m_Exponentiator->GetOutput() );

    m_Warper->Update();

    m_Adder->SetInput1( m_Warper->GetOutput() );
    m_Adder->SetInput2( m_Exponentiator->GetOutput() );

    m_Adder->GetOutput()->SetRequestedRegion(
      this->GetOutput()->GetRequestedRegion() );
    }

  // Triggers update
  m_Adder->Update();

  // Region passing stuff
  this->GraftOutput( m_Adder->GetOutput() );
  this->GetOutput()->Modified();

  DemonsRegistrationFunctionType *drfp = this->DownCastDifferenceFunctionType();

  this->SetRMSChange( drfp->GetRMSChange() );

  /**
    * Smooth the deformation field
    */
  if ( this->GetSmoothDeformationField() )
    {
    this->SmoothDeformationField();
    }
}

template< class TFixedImage, class TMovingImage, class TDeformationField >
void
DiffeomorphicDemonsRegistrationWithMaskFilter< TFixedImage, TMovingImage,
                                               TDeformationField >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Intensity difference threshold: "
     << this->GetIntensityDifferenceThreshold() << std::endl;
  os << indent << "Use First Order exponential: "
     << this->m_UseFirstOrderExp << std::endl;
}
} // end namespace itk

#endif
