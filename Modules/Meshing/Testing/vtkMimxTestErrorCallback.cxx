/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxTestErrorCallback.cxx,v $
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
#include "vtkMimxTestErrorCallback.h"

#include <vtkObject.h>
#include <vtkSetGet.h>

#include <string.h>
#include <ctype.h>

//----------------------------------------------------------------
vtkMimxTestErrorCallback::vtkMimxTestErrorCallback() 
{ 
  this->ClientData = NULL;
  this->Callback = NULL; 
  this->ClientDataDeleteCallback = NULL;
  this->State = 0;
}
//----------------------------------------------------------------  
vtkMimxTestErrorCallback::~vtkMimxTestErrorCallback() 
{ 
  if (this->ClientDataDeleteCallback)
    {
    this->ClientDataDeleteCallback(this->ClientData);
    }
}
//---------------------------------------------------------------- 
void vtkMimxTestErrorCallback::Execute(vtkObject *,unsigned long, void *calldata)
{
  if (this->Callback)
    {
    this->Callback(this->ClientData);
    }
  // displaying the error messages
  const char* message = reinterpret_cast<const char*>( calldata );
  std::cerr << "Error: " << message << std::endl;
  this->State = 1;
}
//------------------------------------------------------------------------------
