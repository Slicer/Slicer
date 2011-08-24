/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Applications/CLI/OrientImage.cxx $
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

#include "vtkDebugLeaks.h"
#include "vtkXMLPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkPolyDataReader.h"
#include "vtkPolyDataWriter.h"
#include "vtkAppendPolyData.h"
#include "MergeModelsCLP.h"

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  vtkDebugLeaks::SetExitError(true);

  vtkPolyData *         model1 = NULL;
  vtkPolyDataReader *   pdReader1 = NULL;
  vtkXMLPolyDataReader *pdxReader1 = NULL;
  vtkPolyData *         model2 = NULL;
  vtkPolyDataReader *   pdReader2 = NULL;
  vtkXMLPolyDataReader *pdxReader2 = NULL;

  // do we have vtk or vtp models?
  std::string::size_type loc = Model1.find_last_of(".");
  if( loc == std::string::npos )
    {
    std::cerr << "Failed to find an extension for " << Model1 << std::endl;
    return EXIT_FAILURE;
    }
  std::string extension = Model1.substr(loc);
  // read the first poly data

  if( extension == std::string(".vtk") )
    {
    pdReader1 = vtkPolyDataReader::New();
    pdReader1->SetFileName(Model1.c_str() );
    pdReader1->Update();
    model1 = pdReader1->GetOutput();

    }
  else if( extension == std::string(".vtp") )
    {
    pdxReader1 = vtkXMLPolyDataReader::New();
    pdxReader1->SetFileName(Model1.c_str() );
    pdxReader1->Update();
    model1 = pdxReader1->GetOutput();
    }
  if( model1 == NULL )
    {
    std::cerr << "Failed to read model 1 " << Model1 << std::endl;
    return EXIT_FAILURE;
    }
  // read the second poly data

  loc = Model2.find_last_of(".");
  if( loc == std::string::npos )
    {
    std::cerr << "Failed to find an extension for " << Model2 << std::endl;
    return EXIT_FAILURE;
    }
  if( extension == std::string(".vtk") )
    {
    pdReader2 = vtkPolyDataReader::New();
    pdReader2->SetFileName(Model2.c_str() );
    pdReader2->Update();
    model2 = pdReader2->GetOutput();
    }
  else if( extension == std::string(".vtp") )
    {
    pdxReader2 = vtkXMLPolyDataReader::New();
    pdxReader2->SetFileName(Model2.c_str() );
    pdxReader2->Update();
    model2 = pdxReader2->GetOutput();
    }
  if( model2 == NULL )
    {
    std::cerr << "Failed to read model 1 " << Model2 << std::endl;
    return EXIT_FAILURE;
    }
  // add them together
  vtkAppendPolyData *add = vtkAppendPolyData::New();
  add->AddInput(model1);
  add->AddInput(model2);
  add->Update();

  // write the output
  loc = ModelOutput.find_last_of(".");
  if( loc == std::string::npos )
    {
    std::cerr << "Failed to find an extension for " << ModelOutput << std::endl;
    return EXIT_FAILURE;
    }
  if( extension == std::string(".vtk") )
    {
    vtkPolyDataWriter *pdWriter = vtkPolyDataWriter::New();
    pdWriter->SetFileName(ModelOutput.c_str() );
    pdWriter->SetInput(add->GetOutput() );
    pdWriter->Write();
    pdWriter->Delete();
    }
  else if( extension == std::string(".vtp") )
    {
    vtkXMLPolyDataWriter *pdWriter = vtkXMLPolyDataWriter::New();
    pdWriter->SetIdTypeToInt32();
    pdWriter->SetFileName(ModelOutput.c_str() );
    pdWriter->SetInput(add->GetOutput() );
    pdWriter->Write();
    pdWriter->Delete();
    }

  // clean up
  add->Delete();
  if( pdReader1 )
    {
    pdReader1->Delete();
    }
  if( pdReader2 )
    {
    pdReader2->Delete();
    }
  if( pdxReader1 )
    {
    pdxReader1->Delete();
    }
  if( pdxReader2 )
    {
    pdxReader2->Delete();
    }
  return EXIT_SUCCESS;
}
