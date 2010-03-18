/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainBoundingBoxFromUnstructuredGridTest.cxx,v $
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


#include "vtkMimxBoundingBoxFromUnstructuredGrid.h"

#include "vtkMimxTestErrorCallback.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "vtkMimxBoundingBoxToStructuredGridsModified "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkUnstructuredGridReader *reader1 = vtkUnstructuredGridReader::New();
    reader1->SetFileName(argv[1]);
    reader1->Update();
    vtkUnstructuredGridReader *reader2 = vtkUnstructuredGridReader::New();
    reader2->SetFileName(argv[2]);
    reader2->Update();

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();

    vtkMimxBoundingBoxFromUnstructuredGrid *boundingboxfromunstructuredgrid = 
        vtkMimxBoundingBoxFromUnstructuredGrid::New();
    boundingboxfromunstructuredgrid->SetInput(reader2->GetOutput());
    boundingboxfromunstructuredgrid->SetBoundingBox(reader1->GetOutput());
    boundingboxfromunstructuredgrid->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    boundingboxfromunstructuredgrid->Update();


    int status = callback->GetState();

    callback->Delete();
    reader1->Delete();
    reader2->Delete();
    boundingboxfromunstructuredgrid->Delete();

    return status;
}
