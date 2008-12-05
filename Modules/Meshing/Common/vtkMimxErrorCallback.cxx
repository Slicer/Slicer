/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkMimxErrorCallback.cxx,v $
Language:  C++
Date:      $Date: 2008/08/07 02:30:37 $
Version:   $Revision: 1.6 $

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
#include "vtkMimxErrorCallback.h"

#include <vtkSetGet.h>
#include <vtkObject.h>
#include <vtkKWMessageDialog.h>
#include <vtkKWPushButton.h>
#include <string.h>
#include <ctype.h>

//----------------------------------------------------------------
vtkMimxErrorCallback::vtkMimxErrorCallback() 
{ 
  this->ClientData = NULL;
  this->Callback = NULL; 
  this->ClientDataDeleteCallback = NULL;
  this->KWMessageDialog = vtkKWMessageDialog::New();
  this->State = 0;
}
  
vtkMimxErrorCallback::~vtkMimxErrorCallback() 
{ 
  if (this->ClientDataDeleteCallback)
    {
    this->ClientDataDeleteCallback(this->ClientData);
    }
  this->KWMessageDialog->Delete();
}
 
void vtkMimxErrorCallback::Execute(vtkObject *,unsigned long, void *calldata)
{
  if (this->Callback)
    {
    this->Callback(this->ClientData);
    }
  // displaying the error messages
  const char* message = reinterpret_cast<const char*>( calldata );
  this->ErrorMessage(message);
}
//------------------------------------------------------------------------------
void vtkMimxErrorCallback::ErrorMessage(const char *Message)
{
        this->State = 1;
        this->KWMessageDialog->SetApplication(this->KWApplication);
        this->KWMessageDialog->SetStyleToCancel();
        this->KWMessageDialog->Create();
        this->KWMessageDialog->SetTitle("Your Attention Please!");
        this->KWMessageDialog->SetOptions( vtkKWMessageDialog::ErrorIcon );
        this->KWMessageDialog->SetIcon();
        this->KWMessageDialog->SetText(Message);
        
        vtkKWPushButton *cancelButton = this->KWMessageDialog->GetCancelButton();
        cancelButton->SetText("Close");
        
        this->KWMessageDialog->Invoke();
}
