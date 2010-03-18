/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: MainBoundingBoxFromBoundsTest.cxx,v $
Language:  C++
Date:      $Date: 2010/01/03 17:48:17 $
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


#include "vtkMimxBoundingBoxFromBounds.h"

#include "vtkMimxTestErrorCallback.h"

int main(int argc, char * argv [])
{

    vtkMimxTestErrorCallback *callback = vtkMimxTestErrorCallback::New();
       
    double inputBounds[6] = {0,0,0,1,1,1};
    vtkMimxBoundingBoxFromBounds *BBFromBounds = vtkMimxBoundingBoxFromBounds::New();
    BBFromBounds->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
        BBFromBounds->SetBounds( inputBounds );
    BBFromBounds->Update();

    int status = callback->GetState();

    callback->Delete();
    BBFromBounds->Delete();

    return status;
}
