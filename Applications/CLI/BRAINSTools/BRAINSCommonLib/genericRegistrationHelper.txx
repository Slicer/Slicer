/*=========================================================================
 *
 *  Program:   Insight Segmentation & Registration Toolkit
 *  Module:    $RCSfile$
 *  Language:  C++
 *  Date:      $Date: 2007-08-02 14:58:12 -0500 (Thu, 02 Aug 2007) $
 *  Version:   $Revision: 10282 $
 *
 *  Copyright (c) Insight Software Consortium. All rights reserved.
 *  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.
 *
 *  This software is distributed WITHOUT ANY WARRANTY; without even
 *  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the above copyright notices for more information.
 *
 *  =========================================================================*/
#ifndef __genericRegistrationHelper_txx
#define __genericRegistrationHelper_txx

#include "genericRegistrationHelper.h"

#include "itkVersor.h"
#include "itkMatrix.h"
#include "ConvertToRigidAffine.h"

#include "itkWindowedSincInterpolateImageFunction.h"
#include "itkMattesMutualInformationImageToImageMetric.h"

namespace itk
{
/*
  * Constructor
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType>
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::MultiModal3DMutualRegistrationHelper():
  m_FixedImage(0),                         // has to be provided by the user.
  m_MovingImage(0),                        // has to be provided by the user.
  m_InitialTransform(0),                   // has to be provided by the user.
  m_Transform(0),                          // has to be provided by
                                           // this->Initialize().
  m_Registration(0),                       // has to be provided by
                                           // this->Initialize().
  m_PermitParameterVariation(0),
  m_NumberOfSamples(100000),
  m_NumberOfHistogramBins(200),
  m_NumberOfIterations(0),
  m_RelaxationFactor(0.5),
  m_MaximumStepLength(0.2000),
  m_MinimumStepLength(0.0001),
  m_TranslationScale(1000.0),
  m_ReproportionScale(25.0),
  m_SkewScale(25.0),
  m_InitialTransformPassThruFlag(false),
  m_BackgroundFillValue(0.0),
  m_DisplayDeformedImage(false),
  m_PromptUserAfterDisplay(false),
  m_FinalMetricValue(0),
  m_ObserveIterations(true)
{
  this->SetNumberOfRequiredOutputs(1);    // for the Transform

  TransformOutputPointer transformDecorator =
    static_cast< TransformOutputType * >( this->MakeOutput(0).GetPointer() );

  this->ProcessObject::SetNthOutput( 0, transformDecorator.GetPointer() );

  this->SetTransform( TransformType::New() );
  this->m_Transform->SetIdentity();
  this->m_CostMetricObject = NULL;
}

/*
  * Initialize by setting the interconnects between components.
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::Initialize(void)
throw ( ExceptionObject )
{
  if ( !m_FixedImage )
    {
    itkExceptionMacro(<< "FixedImage is not present");
    }

  if ( !m_MovingImage )
    {
    itkExceptionMacro(<< "MovingImage is not present");
    }

  //
  // Connect the transform to the Decorator.
  //
  TransformOutputType *transformOutput =
    static_cast< TransformOutputType * >( this->ProcessObject::GetOutput(0) );

  transformOutput->Set( m_Transform.GetPointer() );

  typename OptimizerType::Pointer optimizer      = OptimizerType::New();
  typename InterpolatorType::Pointer interpolator   = InterpolatorType::New();

  optimizer->SetMaximize(false);   // Mutual Information metrics are to be
                                   // minimized.

  m_Registration = RegistrationType::New();

    {
    // Special BUG work around for MMI metric
    // that does not work in multi-threaded mode
    typedef itk::MattesMutualInformationImageToImageMetric<FixedImageType,MovingImageType> MattesMutualInformationMetricType;
    typename MattesMutualInformationMetricType::Pointer test_MMICostMetric= dynamic_cast<MattesMutualInformationMetricType *>(this->m_CostMetricObject.GetPointer());
    if(test_MMICostMetric.IsNotNull())
      {
      this->m_CostMetricObject->SetNumberOfThreads(1);
      this->m_Registration->SetNumberOfThreads(1);
      }
    }
  m_Registration->SetMetric(this->m_CostMetricObject);
  m_Registration->SetOptimizer(optimizer);
  m_Registration->SetInterpolator(interpolator);

  m_Registration->SetTransform(m_Transform);
  m_Registration->SetFixedImage(m_FixedImage);
  m_Registration->SetMovingImage(m_MovingImage);

  m_Registration->SetFixedImageRegion( m_FixedImage->GetLargestPossibleRegion() );

  std::vector< int > localPermissionToVary( m_Transform->GetNumberOfParameters() );

    {
    unsigned int i = 0;
    while ( i < m_Transform->GetNumberOfParameters() )
      {
      if ( i < m_PermitParameterVariation.size() )
        {
        localPermissionToVary[i] = m_PermitParameterVariation[i];
        }
      else
        {
        localPermissionToVary[i] = 1;
        }
      i++;
      }
    }
  // Decode localPermissionToVary from its initial segment,
  // PermitParameterVariation.
  if ( ( m_PermitParameterVariation.size() != m_Transform->GetNumberOfParameters() )
       && ( m_PermitParameterVariation.size() != 0 ) )
    {
    std::cout
    << "WARNING:  The permit parameters SHOULD match the number of parameters used for this registration type."
    << std::endl;
    std::cout << "WARNING:  Padding with 1's for the unspecified parameters" << std::endl;
    std::cout << "m_PermitParameterVariation " << m_PermitParameterVariation.size() << " != "
              << m_Transform->GetNumberOfParameters() << std::endl;
    std::cout << "\nUSING: [ ";
    for ( unsigned int i = 0; i < localPermissionToVary.size(); i++ )
      {
      std::cout << localPermissionToVary[i] << " ";
      }
    std::cout << " ]" << std::endl;
    }

  if ( m_InitialTransform )
    {
    // TODO: There should be no need to convert here, just assign m_Transform
    // from m_InitialTransform.
    //      They should be the same type!
      {
      const typename TTransformType::ConstPointer tempInitializerITKTransform = m_InitialTransform.GetPointer();
      // NOTE By calling AssignConvertedTransform, it also copies the values
      AssignRigid::AssignConvertedTransform(m_Transform, tempInitializerITKTransform);
      }

    // No need to step on parameters that may not vary; they will remain
    // identical with
    // values from InitialTransform which defaults correctly to SetIdentity().
    }
  else      // Won't happen under BRAINSFitPrimary.
    {
    std::cout
    << "FAILURE:  InitialTransform must be set in MultiModal3DMutualRegistrationHelper before Initialize is called."
    << std::endl;
    exit(-1);
    //  m_Transform would be SetIdentity() if this case continued.
    }

  //  We now pass the parameters of the current transform as the initial
  //  parameters to be used when the registration process starts.
  m_Registration->SetInitialTransformParameters( m_Transform->GetParameters() );

  const double translationScale  = 1.0 / m_TranslationScale;
  const double reproportionScale = 1.0 / m_ReproportionScale;
  const double skewScale         = 1.0 / m_SkewScale;

  OptimizerScalesType optimizerScales( m_Transform->GetNumberOfParameters() );

  if ( m_Transform->GetNumberOfParameters() == 15 )    //  ScaleSkew
    {
    for ( unsigned int i = 0; i < m_Transform->GetNumberOfParameters(); i++ )
      {
      optimizerScales[i] = 1.0;
      }
    for ( unsigned int i = 3; i < 6; i++ )
      {
      optimizerScales[i] = translationScale;
      }
    for ( unsigned int i = 6; i < 9; i++ )
      {
      optimizerScales[i] = reproportionScale;
      }
    for ( unsigned int i = 9; i < 15; i++ )
      {
      optimizerScales[i] = skewScale;
      }
    }
  else if ( m_Transform->GetNumberOfParameters() == 12 )    //  Affine
    {
    for ( unsigned int i = 0; i < 9; i++ )
      {
      optimizerScales[i] = 1.0;
      }
    for ( unsigned int i = 9; i < 12; i++ )
      {
      optimizerScales[i] = translationScale;
      }
    }
  else if ( m_Transform->GetNumberOfParameters() == 9 )   // ScaleVersorRigid3D
    {
    for ( unsigned int i = 0; i < 3; i++ )
      {
      optimizerScales[i] = 1.0;
      }
    for ( unsigned int i = 3; i < 6; i++ )
      {
      optimizerScales[i] = translationScale;
      }
    for ( unsigned int i = 6; i < 9; i++ )
      {
      optimizerScales[i] = reproportionScale;
      }
    }
  else if ( m_Transform->GetNumberOfParameters() == 6 )    //  VersorRigid3D
    {
    for ( unsigned int i = 0; i < 3; i++ )
      {
      optimizerScales[i] = 1.0;
      }
    for ( unsigned int i = 3; i < 6; i++ )
      {
      optimizerScales[i] = translationScale;
      }
    }
  else     // most likely (m_Transform->GetNumberOfParameters() == 3): uniform
           // parameter scaling, whether
           // just rotating OR just translating.
    {
    for ( unsigned int i = 0; i < m_Transform->GetNumberOfParameters(); i++ )
      {
      optimizerScales[i] = 1.0;
      }
    }

  // Step on parameters that may not vary; they also must be identical with
  // SetIdentity().
  for ( unsigned int i = 0; i < m_Transform->GetNumberOfParameters(); i++ )
    {
    if ( localPermissionToVary[i] == 0 )
      {
      //Make huge to greatly penilize any motion
      optimizerScales[i] = 0.5 * vcl_numeric_limits< float >::max();
      }
    }

  std::cout << "Initializer, optimizerScales: " << optimizerScales << "."
            << std::endl;
  optimizer->SetScales(optimizerScales);

  optimizer->SetRelaxationFactor(m_RelaxationFactor);
  optimizer->SetMaximumStepLength(m_MaximumStepLength);
  optimizer->SetMinimumStepLength(m_MinimumStepLength);
  optimizer->SetNumberOfIterations(m_NumberOfIterations);

  // if (globalVerbose)
  if ( 0 )
    {
    std::cout << "Initializer, RelaxationFactor: " << m_RelaxationFactor
              << "." << std::endl;
    std::cout << "Initializer, MaximumStepLength: " << m_MaximumStepLength
              << "." << std::endl;
    std::cout << "Initializer, MinimumStepLength: " << m_MinimumStepLength
              << "." << std::endl;
    std::cout << "Initializer, NumberOfIterations: " << m_NumberOfIterations
              << "." << std::endl;
    std::cout << "Registration, Transform : " << m_Transform << "."
              << std::endl;
    std::cout << "Registration, FixedImage : " << m_FixedImage << "."
              << std::endl;
    std::cout << "Registration, MovingImage : " << m_MovingImage << "."
              << std::endl;
    }

  // Create the Command observer and register it with the optimizer.
  // TODO:  make this output optional.
  //
  if ( m_ObserveIterations == true )
    {
    typedef BRAINSFit::CommandIterationUpdate< TOptimizer, TTransformType, TMovingImage >
    CommandIterationUpdateType;
    typename CommandIterationUpdateType::Pointer observer =
      CommandIterationUpdateType::New();
    observer->SetDisplayDeformedImage(m_DisplayDeformedImage);
    observer->SetPromptUserAfterDisplay(m_PromptUserAfterDisplay);
    observer->SetPrintParameters(true);
    observer->SetMovingImage(m_MovingImage);
    observer->SetFixedImage(m_FixedImage);
    observer->SetTransform(m_Transform);
    optimizer->AddObserver(itk::IterationEvent(), observer);
    }
}

/*
  * Starts the Registration Process
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::StartRegistration(void)
{
  if ( !m_InitialTransformPassThruFlag )
    {
    bool               successful = false;
    const unsigned int diff = this->m_NumberOfSamples / 10;
    while ( !successful )
      {
      try
        {
        m_Registration->StartRegistration();
        successful = true;
        }
      catch ( itk::ExceptionObject & err )
        {
        //Attempt to auto-recover i too many samples were requested.
        // std::cerr << "ExceptionObject caught !" << std::endl;
        // std::cerr << err << std::endl;
        // Pass exception to caller
        //        throw err;
        //
        // lower the number of samples you request
        typename MetricType::Pointer autoResetNumberOfSamplesMetric =
               dynamic_cast< MetricType * >( this->m_Registration->GetMetric() );
        if ( autoResetNumberOfSamplesMetric.IsNull() )
          {
          std::cout << "ERROR::" << __FILE__ << " " << __LINE__ << std::endl;
          throw err;
          }
        unsigned int localNumberOfSamples=autoResetNumberOfSamplesMetric->GetNumberOfSpatialSamples();
        if ( diff > localNumberOfSamples )
          {
          // we are done.  This can not be recovered from.
          throw err;
          }
        localNumberOfSamples -= diff;
        autoResetNumberOfSamplesMetric->SetNumberOfSpatialSamples(localNumberOfSamples);
        }
      }

    OptimizerParametersType finalParameters( m_Transform->GetNumberOfParameters() );

    finalParameters = m_Registration->GetLastTransformParameters();

    OptimizerPointer optimizer =
      dynamic_cast< OptimizerPointer >( m_Registration->GetOptimizer() );
    std::cout << "Stop condition from optimizer." << optimizer->GetStopConditionDescription() << std::endl;
    m_FinalMetricValue = optimizer->GetValue();
    m_ActualNumberOfIterations = optimizer->GetCurrentIteration();
    m_Transform->SetParametersByValue(finalParameters);
    }

  typename TransformType::MatrixType matrix = m_Transform->GetMatrix();
  typename TransformType::OffsetType offset = m_Transform->GetOffset();

  std::cout << std::endl << "Matrix = " << std::endl << matrix << std::endl;
  std::cout << "Offset = " << offset << std::endl << std::endl;
}

/**
  *
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
unsigned long
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::GetMTime() const
{
  unsigned long mtime = Superclass::GetMTime();
  unsigned long m;

  // Some of the following should be removed once ivars are put in the
  // input and output lists

  if ( m_Transform )
    {
    m = m_Transform->GetMTime();
    mtime = ( m > mtime ? m : mtime );
    }

  if ( m_Registration )
    {
    m = m_Registration->GetMTime();
    mtime = ( m > mtime ? m : mtime );
    }

  if ( m_InitialTransform )
    {
    m = m_InitialTransform->GetMTime();
    mtime = ( m > mtime ? m : mtime );
    }

  if ( m_FixedImage )
    {
    m = m_FixedImage->GetMTime();
    mtime = ( m > mtime ? m : mtime );
    }

  if ( m_MovingImage )
    {
    m = m_MovingImage->GetMTime();
    mtime = ( m > mtime ? m : mtime );
    }

  return mtime;
}

/*
  * Generate Data
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::GenerateData()
{
  this->StartRegistration();
}

template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::SetFixedImage(FixedImagePointer fixedImage)
{
  itkDebugMacro("setting Fixed Image to " << fixedImage);

  if ( this->m_FixedImage.GetPointer() != fixedImage )
    {
    this->m_FixedImage = fixedImage;
    this->ProcessObject::SetNthInput(0, this->m_FixedImage);
    this->Modified();
    }
}

template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::SetMovingImage(MovingImagePointer movingImage)
{
  itkDebugMacro("setting Moving Image to " << movingImage);

  if ( this->m_MovingImage.GetPointer() != movingImage )
    {
    this->m_MovingImage = movingImage;
    this->ProcessObject::SetNthInput(1, this->m_MovingImage);
    this->Modified();
    }
}

template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::SetInitialTransform(typename TransformType::Pointer initialTransform)
{
  itkDebugMacro("setting Initial Transform to " << initialTransform);
  if ( this->m_InitialTransform.GetPointer() != initialTransform )
    {
    this->m_InitialTransform = initialTransform;
    this->Modified();
    }
}

template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
typename MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer,
                                               TFixedImage, TMovingImage, MetricType >::TransformType::Pointer
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::GetTransform(void)
{
  this->Update();
  return m_Transform;
}

/*
  *  Get Output
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
const typename MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer,
                                                     TFixedImage, TMovingImage, MetricType >::TransformOutputType *
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::GetOutput() const
{
  return static_cast< const TransformOutputType * >(
           this->ProcessObject::GetOutput(0) );
}

template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
DataObject::Pointer
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::MakeOutput(unsigned int output)
{
  switch ( output )
    {
    case 0:
      return static_cast< DataObject * >( TransformOutputType::New().GetPointer() );
      break;
    default:
      itkExceptionMacro(
        "MakeOutput request for an output number larger than the expected number of outputs");
      return 0;
    }
}

/*
  * PrintSelf
  */
template< typename TTransformType, typename TOptimizer, typename TFixedImage,
          typename TMovingImage, typename MetricType >
void
MultiModal3DMutualRegistrationHelper< TTransformType, TOptimizer, TFixedImage,
                                      TMovingImage, MetricType >
::PrintSelf(std::ostream & os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Transform: " << m_Transform.GetPointer() << std::endl;
  os << indent << "Fixed Image: " << m_FixedImage.GetPointer() << std::endl;
  os << indent << "Moving Image: " << m_MovingImage.GetPointer() << std::endl;
}
} // end namespace itk

#endif
