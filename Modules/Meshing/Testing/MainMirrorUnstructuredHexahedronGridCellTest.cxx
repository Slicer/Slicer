/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainMirrorUnstructuredHexahedronGridCellTest.cxx,v $
Language:  C++
Date:      $Date: 2007/10/18 21:38:08 $
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

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif


#include "vtkMimxMirrorUnstructuredHexahedronGridCell.h"

#include "vtkDataSetMapper.h"
#include "vtkIdList.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

int main(int argc, char * argv [])
{
 /* if( argc < 2 )
    {
    std::cerr << "MainMirrorUnstructuredHexahedronGridCellTest "
              << " InputVtkUnstructuredGrid"
              << std::endl;
    return EXIT_FAILURE;
    }


  vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
  reader->SetFileName( argv[1] );
  reader->Update();
  vtkMimxMirrorUnstructuredHexahedronGridCell *Mirror1 = 
    vtkMimxMirrorUnstructuredHexahedronGridCell::New();
  double bounds[6];
  reader->GetOutput()->GetBounds(bounds);
  Mirror1->SetInput(reader->GetOutput());
  Mirror1->SetAxis(0);
  Mirror1->SetMirrorPoint(bounds[1]);
  Mirror1->Update();
  Mirror1->GetOutput()->GetBounds(bounds);  
  Mirror1->Update( );
    std::cout << "Test 1 - Number of Cells: " << Mirror1->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Mirror1->GetOutput()->GetNumberOfCells() != 6 )
    {
    std::cout << "Error Test 1 - Invalid number of cells" << std::endl;
    return EXIT_FAILURE;
    }


  vtkMimxMirrorUnstructuredHexahedronGridCell *Mirror2 = 
    vtkMimxMirrorUnstructuredHexahedronGridCell::New();
  Mirror2->SetInput(Mirror1->GetOutput());
  Mirror2->SetAxis(1);
  Mirror2->SetMirrorPoint(bounds[3]);
  Mirror2->Update();
  Mirror2->GetOutput()->GetBounds(bounds);
  Mirror2->Update( );
    std::cout << "Test 2 - Number of Cells: " << Mirror2->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Mirror2->GetOutput()->GetNumberOfCells() != 12 )
    {
    std::cout << "Error Test 2 - Invalid number of cells" << std::endl;
    return EXIT_FAILURE;
    }


  vtkMimxMirrorUnstructuredHexahedronGridCell *Mirror3 = 
    vtkMimxMirrorUnstructuredHexahedronGridCell::New();
  Mirror3->SetInput(Mirror2->GetOutput());
  Mirror3->SetAxis(2);
  Mirror3->SetMirrorPoint(bounds[5]);
  Mirror3->Update( );
    std::cout << "Test 3 - Number of Cells: " << Mirror3->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Mirror3->GetOutput()->GetNumberOfCells() != 24 )
    {
    std::cout << "Error Test 3 - Invalid number of cells" << std::endl;
    return EXIT_FAILURE;
    }


   Turn off the Graphical User Interface testing
  *************************************************
    vtkRenderer* ren = vtkRenderer::New();
    vtkDataSetMapper *mapper = vtkDataSetMapper::New();
    mapper->SetInput(Mirror3->GetOutput());
    vtkActor *actor = vtkActor::New();
    actor->SetMapper(mapper);
    ren->AddActor(actor);
    vtkRenderWindow* renwin = vtkRenderWindow::New();
    renwin->AddRenderer(ren);
    vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::New();
    iren->SetRenderWindow(renwin);
    renwin->Render();
    iren->Start();
    reader->Delete();
    Mirror1->Delete();
    actor->Delete();
    mapper->Delete();
    ren->Delete();
    renwin->Delete();
    iren->Delete();
  *************************************************/
  
  return EXIT_SUCCESS;
}
