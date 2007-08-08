/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: mimxTransformIO.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 16:30:26 $
Version:   $Revision: 1.4 $

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

#ifndef __mimxTransformIO_cxx
#define __mimxTransformIO_cxx

#include "mimxTransformIO.h"

#include <iostream>


namespace itk
{
  
  
mimxTransformIO
::mimxTransformIO()
{    
  
}

void mimxTransformIO::SetFileName( char *fileName)
{  
  m_FileName = fileName;
}

void mimxTransformIO::SetFileName( std::string fileName)
{  
  m_FileName = fileName;
}



void mimxTransformIO::LoadTransform( )
{
  
  typedef itk::TransformFileReader  TransformReaderType;
  TransformReaderType::Pointer      transformReader =  TransformReaderType::New();

  transformReader->SetFileName( m_FileName.c_str() );

  std::cout << "Reading ITK transform file: " << m_FileName << " ..." << std::endl;

  try 
  { 
  transformReader->Update(); 
  } 
  catch( itk::ExceptionObject & err ) 
  { 
  std::cerr << "Failed to load Transform: " << std::endl; 
  std::cerr << err << std::endl;
  throw; 
  }
  
  
  std::string readTransformType = (transformReader->GetTransformList()->back())->GetTransformTypeAsString();

  if ( strcmp(readTransformType.c_str(),"VersorRigid3DTransform_double_3_3") == 0)
  {
  // Load Versor Transform
  m_RigidTransform = RigidTransformType::New();
  m_RigidTransform->SetIdentity();
  m_RigidTransform->SetParameters( 
        (*transformReader->GetTransformList()->begin())->GetParameters() );
  m_RigidTransform->SetFixedParameters( 
        (*transformReader->GetTransformList()->begin())->GetFixedParameters() );
  //std::cout << "Parameters " << (*transformReader->GetTransformList()->begin())->GetParameters() << std::endl;
  //std::cout << "Fixed Parameters " << (*transformReader->GetTransformList()->begin())->GetFixedParameters() << std::endl;
  }
  else if ( strcmp(readTransformType.c_str(),"BSplineDeformableTransform_double_3_3") == 0)
    {
  // Load B-Spline Transform
  m_BSplineTransform = BSplineTransformType::New();
  m_BSplineTransform->SetIdentity();

  m_BSplineTransform->SetFixedParameters( 
              (transformReader->GetTransformList()->back())->GetFixedParameters() );
  m_BSplineTransform->SetParameters( 
              (transformReader->GetTransformList()->back())->GetParameters() );
  std::string initTransformType = (*transformReader->GetTransformList()->begin())->GetTransformTypeAsString();
  if ( strcmp(initTransformType.c_str(),"VersorRigid3DTransform_double_3_3") == 0)
    {
    //std::cout << " Set Bulk Transform: " << std::endl;
    RigidTransformType::Pointer bulkTransform = RigidTransformType::New();
    bulkTransform->SetIdentity();
      bulkTransform->SetParameters( 
              (*transformReader->GetTransformList()->begin())->GetParameters() );
    bulkTransform->SetFixedParameters( 
              (*transformReader->GetTransformList()->begin())->GetFixedParameters() );
    m_BSplineTransform->SetBulkTransform( bulkTransform );
    }

  //std::cout << " Transform: " << m_AlignAnatomicalTransform << std::endl;  
    }
  else if ( strcmp(readTransformType.c_str(),"AffineTransform_double_3_3") == 0)
    {
  // Import Affine Transform
  m_AffineTransform = AffineTransformType::New();
  m_AffineTransform->SetIdentity();
  m_AffineTransform->SetParameters( 
            (*transformReader->GetTransformList()->begin())->GetParameters() );
    }
  else if ( strcmp(readTransformType.c_str(),"ThinPlateR2LogRSplineKernelTransform_double_3_3") == 0)
    {
  // Load Thin Plate Spline Transform - B-Spline Inverse
  m_InverseBSplineTransform = ThinPlateSplineTransformType::New();
  std::cout << "Set Fixed Parameters" << std::endl;
  m_InverseBSplineTransform->SetFixedParameters( 
            (*transformReader->GetTransformList()->begin())->GetFixedParameters() );
  std::cout << "Set Parameters" << std::endl;
  m_InverseBSplineTransform->SetParameters( 
            (*transformReader->GetTransformList()->begin())->GetParameters() );
  std::cout << "Compute W Matrix" << std::endl;
  m_InverseBSplineTransform->ComputeWMatrix();
  std::cout << "Done" << std::endl;
    }
}

void mimxTransformIO::SaveTransform( int type )
{
  typedef itk::TransformFileWriter  TransformWriterType;
  TransformWriterType::Pointer    transformWriter =  TransformWriterType::New();
  transformWriter->SetFileName( m_FileName.c_str() );
  
  switch ( type )
    {
  case 0:
    transformWriter->SetInput( m_RigidTransform );
    break;
  case 1:
    transformWriter->SetInput( m_BSplineTransform->GetBulkTransform() );  
    transformWriter->AddTransform( m_BSplineTransform );
    break;
  case 2:
    transformWriter->SetInput( m_AffineTransform );
    break;
  case 3:
    transformWriter->SetInput( m_InverseBSplineTransform );
    break;
  }      
  transformWriter->Update();        
  std::cout << "Wrote ITK transform to file: " << m_FileName << std::endl;
}  


}// end namespace itk
#endif
