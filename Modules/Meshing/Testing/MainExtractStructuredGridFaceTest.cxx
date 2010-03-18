/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainExtractStructuredGridFaceTest.cxx,v $
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


#include "vtkMimxExtractStructuredGridFace.h"

#include "vtkStructuredGrid.h"
#include "vtkStructuredGridReader.h"
#include "vtkMimxTestErrorCallback.h"

int main(int argc, char * argv [])
{
    if( argc < 2 )
    {
        std::cerr << "vtkMimxExtractStructuredGridFace "
            << " InputVtkUnstructuredGrid"
            << std::endl;
        return EXIT_FAILURE;
    }

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();
    
    vtkStructuredGridReader *reader = vtkStructuredGridReader::New();
    reader->SetFileName(argv[1]);
    reader->Update();
    vtkMimxExtractStructuredGridFace *ExtractStructuredGridFace = vtkMimxExtractStructuredGridFace::New();
    ExtractStructuredGridFace->SetInput(reader->GetOutput());
    ExtractStructuredGridFace->SetFaceNum(0);
    ExtractStructuredGridFace->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    ExtractStructuredGridFace->Update();

    int status = callback->GetState();

    callback->Delete();
    ExtractStructuredGridFace->Delete();
    reader->Delete();

    return status;
}
