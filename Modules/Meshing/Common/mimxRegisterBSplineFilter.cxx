/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxRegisterBSplineFilter.cxx,v $
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

#include "mimxRegisterBSplineFilter.h"


namespace itk
{

mimxRegisterBSplineFilter::mimxRegisterBSplineFilter()
{    
    
  m_SpatialSampleScale = 100;
  m_MaximumNumberOfIterations = 500;
  m_MaximumNumberOfEvaluations = 500;
  m_MaximumNumberOfCorrections = 12;
  m_BSplineHistogramBins = 50;
  m_GridSize = 12;
  m_GridBorderSize = 3;
  m_CostFunctionConvergenceFactor = 1e+7;
  m_ProjectedGradientTolerance = 1e-4;
  m_BulkTransform = NULL;
  m_Output = TransformType::New();
  
}


  
void mimxRegisterBSplineFilter::Update ( )
{

  std::cout << "RegisterBSplineFilter()...." << std::endl;
    

  MetricTypePointer         metric        = MetricType::New();
  OptimizerTypePointer      optimizer     = OptimizerType::New();
  InterpolatorTypePointer   interpolator  = InterpolatorType::New();
  RegistrationTypePointer   registration  = RegistrationType::New();
  
      
  /*** Set up the Registration ***/
  registration->SetMetric(        metric        );
  registration->SetOptimizer(     optimizer     );
  registration->SetInterpolator(  interpolator  );
  registration->SetTransform( m_Output );  
  
  /*** Setup the Registration ***/
  registration->SetFixedImage(  m_FixedImage   );
  registration->SetMovingImage(   m_MovingImage   );
  

  FixedImageRegionType fixedImageRegion = m_FixedImage->GetBufferedRegion();
  
  registration->SetFixedImageRegion( fixedImageRegion );

  /*** Setup the B-SPline Parameters ***/
  TransformRegionType    bsplineRegion;
  TransformSizeType      gridSizeOnImage;
  TransformSizeType      gridBorderSize;
  TransformSizeType      totalGridSize;

  gridSizeOnImage.Fill( m_GridSize );
  gridBorderSize.Fill( m_GridBorderSize );    // Border for spline order = 3 ( 1 lower, 2 upper )
  totalGridSize = gridSizeOnImage + gridBorderSize;

  bsplineRegion.SetSize( totalGridSize );

  TransformSpacingType spacing = m_FixedImage->GetSpacing();
  TransformOriginType origin = m_FixedImage->GetOrigin();

  FixedImageSizeType fixedImageSize = fixedImageRegion.GetSize();

  for(unsigned int r=0; r<3; r++)
    {
    spacing[r] *= floor( static_cast<double>(fixedImageSize[r] - 1)  / 
                  static_cast<double>(gridSizeOnImage[r] - 1) );
    origin[r]  -=  spacing[r]; 
    }

  m_Output->SetGridSpacing( spacing );
  m_Output->SetGridOrigin( origin );
  m_Output->SetGridRegion( bsplineRegion );
  
  if ( m_BulkTransform.IsNotNull() )
  {
    std::cout << "Using Bulk Transform" << std::endl;
    m_Output->SetBulkTransform(   m_BulkTransform   );
  }
    

  const unsigned int numberOfParameters =
               m_Output->GetNumberOfParameters();
  
  TransformParametersType parameters( numberOfParameters );

  parameters.Fill( 0.0 );

  m_Output->SetParameters( parameters );
  
  registration->SetInitialTransformParameters( m_Output->GetParameters() );
  
  
  OptimizerBoundSelectionType boundSelect( m_Output->GetNumberOfParameters() );
  OptimizerBoundValueType     upperBound( m_Output->GetNumberOfParameters() );
  OptimizerBoundValueType     lowerBound( m_Output->GetNumberOfParameters() );

  boundSelect.Fill( 0 );
  upperBound.Fill( 0.0 );
  lowerBound.Fill( 0.0 );

  optimizer->SetBoundSelection( boundSelect );
  optimizer->SetUpperBound( upperBound );
  optimizer->SetLowerBound( lowerBound );

  optimizer->SetCostFunctionConvergenceFactor( m_CostFunctionConvergenceFactor );
  optimizer->SetProjectedGradientTolerance( m_ProjectedGradientTolerance );
  optimizer->SetMaximumNumberOfIterations( m_MaximumNumberOfIterations );
  optimizer->SetMaximumNumberOfEvaluations( m_MaximumNumberOfEvaluations );
  optimizer->SetMaximumNumberOfCorrections( m_MaximumNumberOfCorrections );
    
  metric->SetNumberOfHistogramBins( m_BSplineHistogramBins );
  
  /*** Make this a Parameter ***/
  const unsigned int numberOfSamples = fixedImageRegion.GetNumberOfPixels() / m_SpatialSampleScale;

  metric->SetNumberOfSpatialSamples( numberOfSamples );
  metric->ReinitializeSeed( 76926294 );


  // Add a time probe
  itk::TimeProbesCollectorBase collector;

  std::cout << std::endl << "Starting Registration" << std::endl;

  try 
    { 
    collector.Start( "Registration" );
    registration->StartRegistration(); 
    collector.Stop( "Registration" );
    } 
  catch( itk::ExceptionObject & err ) 
    { 
    std::cerr << "ExceptionObject caught !" << std::endl; 
    std::cerr << err << std::endl; 
    return;
    } 
  
  OptimizerType::ParametersType finalParameters = 
                    registration->GetLastTransformParameters();

  collector.Report();

  /* This call is required to copy the parameters */
  m_Output->SetParametersByValue( finalParameters );

  std::cout << m_Output << std::endl;
   
}



}// end namespace itk

