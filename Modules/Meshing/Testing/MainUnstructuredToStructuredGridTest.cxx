/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainUnstructuredToStructuredGridTest.cxx,v $
Language:  C++
Date:      $Date: 2010/01/05 15:28:44 $
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

#include "vtkMimxUnstructuredToStructuredGrid.h"

#include "vtkMimxTestErrorCallback.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

int main(int argc, char * argv [])
{
    if( argc < 3 )
    {
        std::cerr << "vtkMimxBoundingBoxToStructuredGridsModified "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();

    vtkUnstructuredGridReader *ugrid = vtkUnstructuredGridReader::New();
    ugrid->SetFileName(argv[1]);
    ugrid->Update();

    vtkUnstructuredGridReader *boundingbox = vtkUnstructuredGridReader::New();
    boundingbox->SetFileName(argv[2]);
    boundingbox->Update();

    vtkMimxUnstructuredToStructuredGrid *utosgrid = vtkMimxUnstructuredToStructuredGrid::New();
    utosgrid->SetInput(ugrid->GetOutput());
    utosgrid->SetBoundingBox(boundingbox->GetOutput());
    utosgrid->SetStructuredGridNum(0);
    utosgrid->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    utosgrid->Update();

    int status = callback->GetState();

    callback->Delete();
    utosgrid->Delete();
    boundingbox->Delete();
    ugrid->Delete();


    return status;
}
