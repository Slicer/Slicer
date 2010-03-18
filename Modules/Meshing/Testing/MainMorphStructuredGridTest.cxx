/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainMorphStructuredGridTest.cxx,v $
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


#include "vtkMimxMorphStructuredGrid.h"

#include "vtkMimxBoundingBoxToStructuredGrids.h"
#include "vtkMimxTestErrorCallback.h"

#include "vtkSTLReader.h"
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
    
    vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
    reader->SetFileName(argv[1]);
    reader->Update();


    vtkMimxBoundingBoxToStructuredGrids *bboxtostructgrids = vtkMimxBoundingBoxToStructuredGrids::New();
    bboxtostructgrids->SetInput(reader->GetOutput());
    bboxtostructgrids->Update();

    vtkSTLReader *stlreader = vtkSTLReader::New();
    stlreader->SetFileName(argv[2]);
    stlreader->Update();

    vtkMimxMorphStructuredGrid *morphgrid = vtkMimxMorphStructuredGrid::New();
    morphgrid->SetGridStructure(reader->GetOutput());
    morphgrid->SetInput(bboxtostructgrids->GetStructuredGrid(0));
    morphgrid->SetSource(stlreader->GetOutput());
    morphgrid->SetCellNum(0);
    morphgrid->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    morphgrid->Update();

    int status = callback->GetState();

    callback->Delete();
    bboxtostructgrids->Delete();
    reader->Delete();
    morphgrid->Delete();
    stlreader->Delete();
    

    return status;
}
