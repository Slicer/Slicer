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

  vtkAlgorithmOutput* reader1 = NULL;
  vtkAlgorithmOutput* reader2 = NULL;

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
    reader1 = pdReader1->GetOutputPort();
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataReader* pdxReader1 = vtkXMLPolyDataReader::New();
    pdxReader1->SetFileName(Model1.c_str() );
    pdxReader1->Update();
    reader1 = pdxReader1->GetOutputPort();
    }
  if( reader1->GetProducer()->GetErrorCode() != 0 )
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
    reader2 = pdReader2->GetOutputPort();
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataReader* pdxReader2 = vtkXMLPolyDataReader::New();
    pdxReader2->SetFileName(Model2.c_str() );
    pdxReader2->Update();
    reader2 = pdxReader2->GetOutputPort();
    }
  if( reader2->GetProducer()->GetErrorCode() != 0 )
    {
    std::cerr << "Failed to read model 1 " << Model2 << std::endl;
    return EXIT_FAILURE;
    }
  // add them together
  vtkAppendPolyData *add = vtkAppendPolyData::New();
  add->AddInputConnection(reader1);
  add->AddInputConnection(reader2);
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
    pdWriter->SetInputConnection(add->GetOutputPort() );
    pdWriter->Write();
    pdWriter->Delete();
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataWriter *pdWriter = vtkXMLPolyDataWriter::New();
    pdWriter->SetIdTypeToInt32();
    pdWriter->SetFileName(ModelOutput.c_str() );
    pdWriter->SetInputConnection(add->GetOutputPort() );
    pdWriter->Write();
    pdWriter->Delete();
    }

  // clean up
  add->Delete();
  reader1->GetProducer()->Delete();
  reader2->GetProducer()->Delete();
  return EXIT_SUCCESS;
}
