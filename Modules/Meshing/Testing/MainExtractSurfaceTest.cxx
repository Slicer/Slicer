/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainExtractSurfaceTest.cxx,v $
Language:  C++
Date:      $Date: 2009/12/21 22:38:27 $
Version:   $Revision: 1.1 $

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

#include "vtkMimxExtractSurface.h"

#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkIdList.h"

#include "vtkMimxTestErrorCallback.h"


int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "vtkMimxBoundingBoxToStructuredGridsModified "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();

    vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    vtkIdList *CellIdList = vtkIdList::New();
    vtkIdList *FaceIdList = vtkIdList::New();

    CellIdList->InsertNextId(1); CellIdList->InsertNextId(1); CellIdList->InsertNextId(0);
    FaceIdList->InsertNextId(1); FaceIdList->InsertNextId(3); FaceIdList->InsertNextId(5);

    vtkMimxExtractSurface *Extract = vtkMimxExtractSurface::New();
    Extract->SetInput(reader->GetOutput());
    Extract->SetCellIdList(CellIdList);
    Extract->SetFaceIdList(FaceIdList);
    Extract->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    Extract->Update();

    int status = callback->GetState();

    callback->Delete();
    reader->Delete();
    Extract->Delete();
    CellIdList->Delete();
    FaceIdList->Delete();


    return status;
}
