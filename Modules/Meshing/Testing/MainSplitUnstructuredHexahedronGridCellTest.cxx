/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainSplitUnstructuredHexahedronGridCellTest.cxx,v $
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


#include "vtkMimxSplitUnstructuredHexahedronGridCell.h"

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
    std::cerr << "MainSplitUnstructuredHexahedronGridCellTest "
              << " InputVtkUnstructuredGrid"
              << std::endl;
    return EXIT_FAILURE;
    }


  vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
  reader->SetFileName( argv[1] );

  vtkMimxSplitUnstructuredHexahedronGridCell *split1 = vtkMimxSplitUnstructuredHexahedronGridCell::New();
  vtkIdList *idlist1 = vtkIdList::New();
  idlist1->SetNumberOfIds(2);
  idlist1->SetId(0,11); idlist1->SetId(1,13); 
  split1->SetInput(reader->GetOutput());
  split1->SetIdList(idlist1);
  split1->Update();

  std::cout << "Test 1 - Number of Cells: " << split1->GetOutput()->GetNumberOfCells() << std::endl;
  if ( split1->GetOutput()->GetNumberOfCells() != 4 )
    {
    std::cout << "Test 1 - Invalid number of cells" << std::endl;
    return EXIT_FAILURE;
    }



  vtkMimxSplitUnstructuredHexahedronGridCell *split2 = vtkMimxSplitUnstructuredHexahedronGridCell::New();
  split2->SetInput(split1->GetOutput());
  vtkIdList *idlist2 = vtkIdList::New();
  idlist2->SetNumberOfIds(2);
  idlist2->SetId(0,13); idlist2->SetId(1,17); 
  split2->SetIdList(idlist2);
  split2->Update();

  std::cout << "Test 2 - Number of Cells: " << split2->GetOutput()->GetNumberOfCells() << std::endl;
  if ( split2->GetOutput()->GetNumberOfCells() != 5 )
    {
    std::cout << "Test 2 - Invalid number of cells" << std::endl;
    return EXIT_FAILURE;
    }


  vtkMimxSplitUnstructuredHexahedronGridCell *split3 = vtkMimxSplitUnstructuredHexahedronGridCell::New();
  split3->SetInput(split2->GetOutput());
  vtkIdList *idlist3 = vtkIdList::New();
  idlist3->SetNumberOfIds(2);
  idlist3->SetId(0,13); idlist3->SetId(1,21); 
  split3->SetIdList(idlist3);
  split3->Update();

  std::cout << "Test 3 - Number of Cells: " << split3->GetOutput()->GetNumberOfCells() << std::endl;
  if ( split3->GetOutput()->GetNumberOfCells() != 6 )
    {
    std::cout << "Test 3 - Invalid number of cells" << std::endl;
    return EXIT_FAILURE;
    }



  idlist1->Delete();
  idlist2->Delete();
  idlist3->Delete();
  split1->Delete();
  split2->Delete();
  split3->Delete();
  reader->Delete();

  return EXIT_SUCCESS;
}
