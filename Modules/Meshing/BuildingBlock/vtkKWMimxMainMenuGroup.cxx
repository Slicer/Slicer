/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/17 03:37:39 $
Version:   $Revision: 1.20.4.1 $

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

#include "vtkKWMimxMainMenuGroup.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMenuButtonWithLabel.h"

#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxViewProperties.h"

#include <vtksys/SystemTools.hxx>

#include <string.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMainMenuGroup);

vtkCxxRevisionMacro(vtkKWMimxMainMenuGroup, "$Revision: 1.20.4.1 $");

//----------------------------------------------------------------------------

vtkKWMimxMainMenuGroup::vtkKWMimxMainMenuGroup()
{
  this->MimxMainWindow = NULL;
  this->SurfaceList = NULL;
  this->BBoxList = NULL;
  this->FEMeshList = NULL;
  this->ImageList = NULL;
  this->MainFrame = NULL;
  this->ErrorState = 0;
  this->DoUndoTree = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMainMenuGroup::~vtkKWMimxMainMenuGroup()
{
  if(this->MainFrame)
    this->MainFrame->Delete();
  this->SetDoUndoTree(NULL);
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::CreateWidget()
{
        if(this->IsCreated())
        {
                vtkErrorMacro("class already created");
                return;
        }
        this->Superclass::CreateWidget();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::Update()
{
        this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
        OperationMenuButton->SetEnabled(GetEnabled());
        this->MimxMainWindow->ForceWidgetRedraw();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::SetMenuButtonsEnabled(int State)
{
        //this->ObjectMenuButton->SetEnabled(State);
        this->OperationMenuButton->SetEnabled(State);
        //this->TypeMenuButton->SetEnabled(State);
}
//----------------------------------------------------------------------------
const char* vtkKWMimxMainMenuGroup::ExtractFileName(const char* FName)
{
        std::string fileName = vtksys::SystemTools::GetFilenameWithoutExtension(FName);
  
  char *fileNameCopy = new char[fileName.length()+1];
  strcpy( fileNameCopy, fileName.c_str() );
  
        return fileNameCopy;
}
//------------------------------------------------------------------------------

