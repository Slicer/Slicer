/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainBoundingBoxToStructuredGridsTest.cxx,v $
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


#include "vtkMimxBoundingBoxToStructuredGrids.h"

#include "vtkMimxTestErrorCallback.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "vtkMimxBoundingBoxToStructuredGrids "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
    reader->SetFileName( argv[1] );
    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();
    vtkMimxBoundingBoxToStructuredGrids *BBToSgrids = 
        vtkMimxBoundingBoxToStructuredGrids::New();
    BBToSgrids->SetInput(reader->GetOutput());
    BBToSgrids->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    BBToSgrids->Update();

    int status = callback->GetState();

    callback->Delete();
    reader->Delete();
    BBToSgrids->Delete();

    return status;
}
