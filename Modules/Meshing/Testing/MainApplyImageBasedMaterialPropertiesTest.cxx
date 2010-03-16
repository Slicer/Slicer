/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainApplyImageBasedMaterialPropertiesTest.cxx,v $
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


#include "itkImageFileReader.h"
#include "itkImage.h"
#include "vtkMimxApplyImageBasedMaterialProperties.h"
#include "vtkUnstructuredGridReader.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxTestErrorCallback.h"

int main(int argc, char * argv [])
{
    if( argc < 3 )
    {
        std::cerr << "vtkMimxBoundingBoxToStructuredGridsModified "
            << " InputVtkUnstructuredGrid"
            << " InputImage"
            << std::endl;
        return EXIT_FAILURE;
    }

    typedef itk::Image<signed short, 3>  ImageType;

    vtkUnstructuredGridReader *reader1 = vtkUnstructuredGridReader::New();
    typedef itk::ImageFileReader<ImageType> ReaderType;
    ReaderType::Pointer Reader;

    Reader = ReaderType::New();

    Reader->SetFileName(argv[1]);
    Reader->Update();

    reader1->SetFileName(argv[2]);
    reader1->Update();

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();

    vtkMimxApplyImageBasedMaterialProperties *matprop = vtkMimxApplyImageBasedMaterialProperties::New();
        matprop->SetElementSetName("Initial_Elements");
    matprop->SetInput(reader1->GetOutput());
    matprop->SetITKImage(Reader->GetOutput());
    matprop->Update();

    matprop->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    matprop->Update();

    int status = callback->GetState();

    callback->Delete();
    reader1->Delete();
    Reader->Delete();

    return status;
}
//-----------------------------------------------------------------------------------------------------
