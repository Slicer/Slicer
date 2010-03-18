/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainGeometryFilterTest.cxx,v $
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


#include "vtkMimxGeometryFilter.h"

#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkMimxTestErrorCallback.h"

int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "vtkMimxGeometryFilter "
            << " InputVtkunstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();
    
    vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
    reader->SetFileName(argv[1]);
    reader->Update();

    vtkMimxGeometryFilter *GeometryFilter = vtkMimxGeometryFilter::New();
    GeometryFilter->SetInput(reader->GetOutput());
    GeometryFilter->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    GeometryFilter->Update();

    int status = callback->GetState();

    callback->Delete();
    GeometryFilter->Delete();
    reader->Delete();

    return status;
}
