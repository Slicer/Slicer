/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainEquivalancePointsTest.cxx,v $
Language:  C++
Date:      $Date: 2010/01/17 18:46:17 $
Version:   $Revision: 1.2 $

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


#include "vtkMimxEquivalancePoints.h"

#include "vtkMimxTestErrorCallback.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "MainEquivalancePointsTest "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkUnstructuredGridReader *reader1 = vtkUnstructuredGridReader::New();
    reader1->SetFileName(argv[1]);
    reader1->Update();

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();

    vtkMimxEquivalancePoints *equivalance = vtkMimxEquivalancePoints::New();
    equivalance->SetInput(reader1->GetOutput());
    equivalance->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    equivalance->Update();

    int status = callback->GetState();

    callback->Delete();
    reader1->Delete();
    equivalance->Delete();

    return status;
}
