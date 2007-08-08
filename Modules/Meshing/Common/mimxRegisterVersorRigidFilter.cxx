/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxRegisterVersorRigidFilter.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.3 $

 Musculoskeletal Imaging, Modelling and Experimentation (MIMX)
 Center for Computer Aided Design
 The University of Iowa
 Iowa City, IA 52242
 http://www.ccad.uiowa.edu/mimx/
 
Copyright (c) The University of Iowa. All rights reserved.
See MIMXCopyright.txt or http://www.ccad.uiowa.edu/mimx/Copyright.htm for details.

This software is distributed WITHOUT ANY WARRANTY; without even 
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <iostream>

#include <itkObject.h>
#include <itkImage.h>
#include <itkIOCommon.h>
//#include <itkCastImageFilter.h>
//#include <itkConstantPadImageFilter.h>
//#include "itkMetaDataObject.h"
//#include "itkProgressAccumulator.h"


#include "mimxRegisterVersorRigidFilter.h"



namespace itk
{

mimxRegisterVersorRigidFilter::mimxRegisterVersorRigidFilter()
{    
    
  m_NumberOfSpatialSamples = 100000;
  m_NumberOfIterations = 1000;
  m_TranslationScale = 1000.0;
  m_MaximumStepLength = 0.2;
  m_MinimumStepLength = 0.0001;
  m_RelaxationFactor = 0.5;  
}


  
void mimxRegisterVersorRigidFilter::Update ( )
{

  std::cout << "RegisterVersorRigidFilter()...." << std::endl;
    

  MetricTypePointer         metric        = MetricType::New();
  OptimizerTypePointer      optimizer     = OptimizerType::New();
  InterpolatorTypePointer   interpolator  = InterpolatorType::New();
  RegistrationTypePointer   registration  = RegistrationType::New();
   

  /*** Set up the Registration ***/
  metric->SetNumberOfSpatialSamples( m_NumberOfSpatialSamples );
  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetInterpolator(  interpolator  );

  TransformType::Pointer  transform = TransformType::New();
  registration->SetTransform( transform );
  registration->SetFixedImage(   m_FixedImage   );
  registration->SetMovingImage(   m_MovingImage   );
  registration->SetFixedImageRegion( m_FixedImage->GetBufferedRegion() );
  

  TransformInitializerTypePointer initializer = TransformInitializerType::New();
  initializer->SetTransform(   transform );
  initializer->SetFixedImage(  m_FixedImage );
  initializer->SetMovingImage( m_MovingImage );
  initializer->MomentsOn();
  initializer->InitializeTransform();
  
  std::cout << "Initializer, center: " << transform->GetCenter() 
                        << ", offset: " << transform->GetOffset() 
                        << "." << std::endl;

  VersorType     rotation;
  VectorType     axis;

  axis[0] = 0.0;
  axis[1] = 0.0;
  axis[2] = 1.0;

  const double angle = 0;

  rotation.Set(  axis, angle  );
  transform->SetRotation( rotation );  
  registration->SetInitialTransformParameters( transform->GetParameters() );

  const double translationScale = 1.0 / m_TranslationScale;

  OptimizerScalesType optimizerScales( transform->GetNumberOfParameters() );

  optimizerScales[0] = 1.0;
  optimizerScales[1] = 1.0;
  optimizerScales[2] = 1.0;
  optimizerScales[3] = translationScale;
  optimizerScales[4] = translationScale;
  optimizerScales[5] = translationScale;
  optimizer->SetScales( optimizerScales );

  optimizer->SetMaximumStepLength( m_MaximumStepLength );
  optimizer->SetMinimumStepLength( m_MinimumStepLength );

  optimizer->SetRelaxationFactor( m_RelaxationFactor );

  optimizer->SetNumberOfIterations( m_NumberOfIterations );

  std::cout << "Before Rigid Registration, center: " << transform->GetCenter() 
                          << ", offset: " << transform->GetOffset() 
                          << "." << std::endl;


  try 
    { 
    registration->StartRegistration(); 
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    } 

  OptimizerParameterType finalParameters = registration->GetLastTransformParameters();


  const double versorX              = finalParameters[0];
  const double versorY              = finalParameters[1];
  const double versorZ              = finalParameters[2];
  const double finalTranslationX    = finalParameters[3];
  const double finalTranslationY    = finalParameters[4];
  const double finalTranslationZ    = finalParameters[5];
  const unsigned int numberOfIterations = optimizer->GetCurrentIteration();
  const double bestValue = optimizer->GetValue();

  // Print out results
  //
  std::cout << std::endl << std::endl;
  std::cout << "Result = " << std::endl;
  std::cout << " versor X      = " << versorX  << std::endl;
  std::cout << " versor Y      = " << versorY  << std::endl;
  std::cout << " versor Z      = " << versorZ  << std::endl;
  std::cout << " Translation X = " << finalTranslationX  << std::endl;
  std::cout << " Translation Y = " << finalTranslationY  << std::endl;
  std::cout << " Translation Z = " << finalTranslationZ  << std::endl;
  std::cout << " Iterations    = " << numberOfIterations << std::endl;
  std::cout << " Metric value  = " << bestValue          << std::endl;


  transform->SetParameters( finalParameters );

  std::cout << "After Rigid Registration, center: " << transform->GetCenter() 
                          << ", offset: " << transform->GetOffset() 
                          << "." << std::endl;


  TransformType::MatrixType matrix = transform->GetRotationMatrix();
  TransformType::OffsetType offset = transform->GetOffset();

  std::cout << "Matrix = " << std::endl << matrix << std::endl;
  std::cout << "Offset = " << std::endl << offset << std::endl;

  m_Output = TransformType::New();
  m_Output->SetCenter( transform->GetCenter() );
  m_Output->SetParameters( transform->GetParameters() );
   
}



}// end namespace itk

