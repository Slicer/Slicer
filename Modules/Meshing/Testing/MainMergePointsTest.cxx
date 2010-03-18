/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainMergePointsTest.cxx,v $
Language:  C++
Date:      $Date: 2010/02/01 20:47:02 $
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

#include "vtkMimxMergePoints.h"

#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkMimxTestErrorCallback.h"

int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "vtkMimxMergePoints "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();

    vtkUnstructuredGridReader *reader1 = vtkUnstructuredGridReader::New();
    reader1->SetFileName(argv[1]);
    reader1->Update();

    vtkMimxMergePoints *MergePoints = vtkMimxMergePoints::New();
    MergePoints->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    MergePoints->SetTolerance(1.0);  
    MergePoints->SetInput(reader1->GetOutput());  

    int status = callback->GetState();

    callback->Delete();
    MergePoints->Delete();
    reader1->Delete();

    return status;
}
