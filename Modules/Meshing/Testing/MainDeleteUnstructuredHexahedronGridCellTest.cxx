/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainDeleteUnstructuredHexahedronGridCellTest.cxx,v $
Language:  C++
Date:      $Date: 2009/12/21 22:38:27 $
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


#include "vtkMimxDeleteUnstructuredHexahedronGridCell.h"

#include "vtkDataSetMapper.h"
#include "vtkIdList.h"
#include "vtkRenderer.h"
#include "vtkRenderWindow.h"
#include "vtkRenderWindowInteractor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

int main(int argc, char * argv [])
{
  if( argc < 2 )
    {
    std::cerr << "MainDeleteUnstructuredHexahedronGridCellTest "
              << " InputVtkUnstructuredGrid"
              << std::endl;
    return EXIT_FAILURE;
    }


  vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
  reader->SetFileName( argv[1] );
  vtkMimxDeleteUnstructuredHexahedronGridCell *Delete1 = vtkMimxDeleteUnstructuredHexahedronGridCell::New();
  Delete1->SetInput(reader->GetOutput());
  vtkIdList *cellToBeDeleted = vtkIdList::New();
  cellToBeDeleted->SetNumberOfIds(1);
  cellToBeDeleted->SetId(0, 6);
  Delete1->SetCellList(cellToBeDeleted);
  Delete1->Update( );
    std::cout << "Test 1 - Number of Cells: " << Delete1->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Delete1->GetOutput()->GetNumberOfCells() != 6 )
    {
    std::cout << "Error Test 1 - Invalid number of cells - Delete Cell 6" << std::endl;
    return EXIT_FAILURE;
    }

  vtkMimxDeleteUnstructuredHexahedronGridCell *Delete2 = 
    vtkMimxDeleteUnstructuredHexahedronGridCell::New();
  Delete2->SetInput(Delete1->GetOutput());
  vtkIdList *cellToBeDeleted2 = vtkIdList::New();
  cellToBeDeleted2->SetNumberOfIds(1);
  cellToBeDeleted2->SetId(0, 3);
  Delete2->SetCellList(cellToBeDeleted2);
  Delete2->Update( );

  std::cout << "Test 2 - Number of Cells: " << Delete2->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Delete2->GetOutput()->GetNumberOfCells() != 5 )
    {
    std::cout << "Error Test 2 - Invalid number of cells - Delete Cell 3" << std::endl;
    return EXIT_FAILURE;
    }

  Delete1->Delete();
  Delete2->Delete();
  cellToBeDeleted->Delete();
  cellToBeDeleted2->Delete();
  
  reader->Delete();
  

  return EXIT_SUCCESS;
}
