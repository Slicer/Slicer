/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Modules/CLI/OrientImage.cxx $
  Language:  C++
  Date:      $Date: 2007-12-20 18:30:38 -0500 (Thu, 20 Dec 2007) $
  Version:   $Revision: 5310 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include "MergeModelsCLP.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkAppendPolyData.h>
#include <vtkDebugLeaks.h>
#include <vtkPolyDataReader.h>
#include <vtkPolyDataWriter.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkXMLPolyDataWriter.h>
#include <vtkVersion.h>
#include <vtksys/SystemTools.hxx>

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  vtkDebugLeaks::SetExitError(true);

#if (VTK_MAJOR_VERSION <= 5)
  vtkPolyData* model1 = NULL;
  vtkPolyData* model2 = NULL;
  vtkAlgorithm* reader1 = NULL;
  vtkAlgorithm* reader2 = NULL;
#else
  vtkAlgorithmOutput* reader1 = NULL;
  vtkAlgorithmOutput* reader2 = NULL;
#endif

  // do we have vtk or vtp models?
  std::string extension = vtksys::SystemTools::LowerCase(
    vtksys::SystemTools::GetFilenameLastExtension(Model1) );
  if( extension.empty() )
    {
    std::cerr << "Failed to find an extension for " << Model1 << std::endl;
    return EXIT_FAILURE;
    }

  // read the first poly data
  if( extension == std::string(".vtk") )
    {
    vtkPolyDataReader* pdReader1 = vtkPolyDataReader::New();
    pdReader1->SetFileName(Model1.c_str() );
    pdReader1->Update();
#if (VTK_MAJOR_VERSION <= 5)
    model1 = pdReader1->GetOutput();
    reader1 = pdReader1;
#else
    reader1 = pdReader1->GetOutputPort();
#endif
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataReader* pdxReader1 = vtkXMLPolyDataReader::New();
    pdxReader1->SetFileName(Model1.c_str() );
    pdxReader1->Update();
#if (VTK_MAJOR_VERSION <= 5)
    model1 = pdxReader1->GetOutput();
    reader1 = pdxReader1;
#else
    reader1 = pdxReader1->GetOutputPort();
#endif
    }
#if (VTK_MAJOR_VERSION <= 5)
  if( model1 == NULL )
#else
  if( reader1->GetProducer()->GetErrorCode() != 0 )
#endif
    {
    std::cerr << "Failed to read model 1 " << Model1 << std::endl;
    return EXIT_FAILURE;
    }
  // read the second poly data

  extension = vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension(Model2) );
  if( extension.empty() )
    {
    std::cerr << "Failed to find an extension for " << Model2 << std::endl;
    return EXIT_FAILURE;
    }
  if( extension == std::string(".vtk") )
    {
    vtkPolyDataReader* pdReader2 = vtkPolyDataReader::New();
    pdReader2->SetFileName(Model2.c_str() );
    pdReader2->Update();
#if (VTK_MAJOR_VERSION <= 5)
    model2 = pdReader2->GetOutput();
    reader2 = pdReader2;
#else
    reader2 = pdReader2->GetOutputPort();
#endif
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataReader* pdxReader2 = vtkXMLPolyDataReader::New();
    pdxReader2->SetFileName(Model2.c_str() );
    pdxReader2->Update();
#if (VTK_MAJOR_VERSION <= 5)
    model2 = pdxReader2->GetOutput();
    reader2 = pdxReader2;
#else
    reader2 = pdxReader2->GetOutputPort();
#endif
    }
#if (VTK_MAJOR_VERSION <= 5)
  if( model2 == NULL )
#else
  if( reader2->GetProducer()->GetErrorCode() != 0 )
#endif
    {
    std::cerr << "Failed to read model 1 " << Model2 << std::endl;
    return EXIT_FAILURE;
    }
  // add them together
  vtkAppendPolyData *add = vtkAppendPolyData::New();
#if (VTK_MAJOR_VERSION <= 5)
  add->AddInput(model1);
  add->AddInput(model2);
#else
  add->AddInputConnection(reader1);
  add->AddInputConnection(reader2);
#endif
  add->Update();

  // write the output
  extension = vtksys::SystemTools::LowerCase( vtksys::SystemTools::GetFilenameLastExtension(ModelOutput) );
  if( extension.empty() )
    {
    std::cerr << "Failed to find an extension for " << ModelOutput << std::endl;
    return EXIT_FAILURE;
    }
  if( extension == std::string(".vtk") )
    {
    vtkPolyDataWriter *pdWriter = vtkPolyDataWriter::New();
    pdWriter->SetFileName(ModelOutput.c_str() );
#if (VTK_MAJOR_VERSION <= 5)
    pdWriter->SetInput(add->GetOutput() );
#else
    pdWriter->SetInputConnection(add->GetOutputPort() );
#endif
    pdWriter->Write();
    pdWriter->Delete();
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataWriter *pdWriter = vtkXMLPolyDataWriter::New();
    pdWriter->SetIdTypeToInt32();
    pdWriter->SetFileName(ModelOutput.c_str() );
#if (VTK_MAJOR_VERSION <= 5)
    pdWriter->SetInput(add->GetOutput() );
#else
    pdWriter->SetInputConnection(add->GetOutputPort() );
#endif
    pdWriter->Write();
    pdWriter->Delete();
    }

  // clean up
  add->Delete();
#if (VTK_MAJOR_VERSION <= 5)
  reader1->Delete();
  reader2->Delete();
#else
  reader1->GetProducer()->Delete();
  reader2->GetProducer()->Delete();
#endif
  return EXIT_SUCCESS;
}
