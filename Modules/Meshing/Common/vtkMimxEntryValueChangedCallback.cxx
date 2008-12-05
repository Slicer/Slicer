/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxEntryValueChangedCallback.cxx,v $
Language:  C++
Date:      $Date: 2008/07/06 20:30:13 $
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
#include "vtkMimxEntryValueChangedCallback.h"

#include "vtkSetGet.h"
#include "vtkObject.h"
#include "vtkKWMessageDialog.h"

#include <string.h>
#include <ctype.h>

//----------------------------------------------------------------
vtkMimxEntryValueChangedCallback::vtkMimxEntryValueChangedCallback() 
{ 
  this->ClientData = NULL;
  this->Callback = NULL; 
  this->ClientDataDeleteCallback = NULL;
  this->State = 0;
}
//--------------------------------------------------------------------  
vtkMimxEntryValueChangedCallback::~vtkMimxEntryValueChangedCallback() 
{ 
  if (this->ClientDataDeleteCallback)
    {
    this->ClientDataDeleteCallback(this->ClientData);
    }
}
//-----------------------------------------------------------------------------------------
void vtkMimxEntryValueChangedCallback::Execute(vtkObject *,unsigned long, void *calldata)
{
  if (this->Callback)
    {
    this->Callback(this->ClientData);
    }
  // displaying the error messages
  this->State = 1;
}
//------------------------------------------------------------------------------
