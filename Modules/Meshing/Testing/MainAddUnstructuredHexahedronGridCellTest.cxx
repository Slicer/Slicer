/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainAddUnstructuredHexahedronGridCellTest.cxx,v $
Language:  C++
Date:      $Date: 2007/05/17 17:09:46 $
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

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif


#include "vtkMimxAddUnstructuredHexahedronGridCell.h"

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
    std::cerr << "vtkMimxAddUnstructuredHexahedronGridCell "
              << " InputVtkUnstructuredGrid"
              << std::endl;
    return EXIT_FAILURE;
    }

  vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
  reader->SetFileName( argv[1] );
  vtkMimxAddUnstructuredHexahedronGridCell *Add1 = 
        vtkMimxAddUnstructuredHexahedronGridCell::New();
  vtkIdList *idlist1 = vtkIdList::New();
  idlist1->SetNumberOfIds(4);
  idlist1->SetId(0,10); idlist1->SetId(1,14); 
  idlist1->SetId(2,13); idlist1->SetId(3,11); 
  Add1->SetInput(reader->GetOutput());
  Add1->SetIdList(idlist1);
  Add1->Update( );
  std::cout << "Test 1 - Number of Cells: " << Add1->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Add1->GetOutput()->GetNumberOfCells() != 4 )
    {
    std::cout << "Error Test 1 - Invalid number of cells " << std::endl;
    return EXIT_FAILURE;
    }
  idlist1->Delete();

  vtkMimxAddUnstructuredHexahedronGridCell *Add2 = 
    vtkMimxAddUnstructuredHexahedronGridCell::New();
  Add2->SetInput(Add1->GetOutput());
  vtkIdList *idlist2 = vtkIdList::New();
  idlist2->SetNumberOfIds(4);
  idlist2->SetId(0,16); idlist2->SetId(1,17);
  idlist2->SetId(2,18); idlist2->SetId(3,19); 
  Add2->SetIdList(idlist2);
  Add2->Update( );
  std::cout << "Test 2 - Number of Cells: " << Add2->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Add2->GetOutput()->GetNumberOfCells() != 5 )
    {
    std::cout << "Error Test 2 - Invalid number of cells " << std::endl;
    return EXIT_FAILURE;
    }
  idlist2->Delete();

  vtkMimxAddUnstructuredHexahedronGridCell *Add3 = 
    vtkMimxAddUnstructuredHexahedronGridCell::New();
  Add3->SetInput(Add2->GetOutput());
  vtkIdList *idlist3 = vtkIdList::New();
  idlist3->SetNumberOfIds(4);
  idlist3->SetId(0,10); idlist3->SetId(1,11); 
  idlist3->SetId(2,6); idlist3->SetId(3,2); 
  Add3->SetIdList(idlist3);
  Add3->Update( );
  std::cout << "Test 3 - Number of Cells: " << Add3->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Add3->GetOutput()->GetNumberOfCells() != 6 )
    {
    std::cout << "Error Test 3 - Invalid number of cells " << std::endl;
    return EXIT_FAILURE;
    }
  idlist3->Delete();

  vtkMimxAddUnstructuredHexahedronGridCell *Add4 = 
    vtkMimxAddUnstructuredHexahedronGridCell::New();
  Add4->SetInput(Add3->GetOutput());
  vtkIdList *idlist4 = vtkIdList::New();
  idlist4->SetNumberOfIds(4);
  idlist4->SetId(0,4); idlist4->SetId(1,5); 
  idlist4->SetId(2,10); idlist4->SetId(3,9); 
  Add4->SetIdList(idlist4);
  Add4->Update( );
  std::cout << "Test 4 - Number of Cells: " << Add4->GetOutput()->GetNumberOfCells() << std::endl;
  if ( Add4->GetOutput()->GetNumberOfCells() != 7 )
    {
    std::cout << "Error Test 4 - Invalid number of cells " << std::endl;
    return EXIT_FAILURE;
    }
  idlist4->Delete();
  Add1->Delete();
  Add2->Delete();
  Add3->Delete();
  Add4->Delete();
  reader->Delete();

  return EXIT_SUCCESS;
}
