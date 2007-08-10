/*=========================================================================

  Module:    $RCSfile: vtkKWMimxMainMenuGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkKWMimxViewProperties.h"

#include "vtkKWApplication.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkLinkedListWrapper.h"
#include <itksys/SystemTools.hxx>
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

#include <string.h>


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMainMenuGroup);

vtkCxxRevisionMacro(vtkKWMimxMainMenuGroup, "$Revision: 1.6 $");

//----------------------------------------------------------------------------
vtkKWMimxMainMenuGroup::vtkKWMimxMainMenuGroup()
{
  MimxViewWindow = NULL;
//  this->ObjectList = NULL;
  this->SurfaceList = NULL;
  this->BBoxList = NULL;
  this->FEMeshList = NULL;
  this->MainFrame = NULL;
  this->MimxViewProperties = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMainMenuGroup::~vtkKWMimxMainMenuGroup()
{
  if(this->MainFrame)
    this->MainFrame->Delete();
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
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxMainMenuGroup::SetMenuButtonsEnabled(int State)
{
  this->ObjectMenuButton->SetEnabled(State);
  this->OperationMenuButton->SetEnabled(State);
  this->TypeMenuButton->SetEnabled(State);
}
//----------------------------------------------------------------------------
const char* vtkKWMimxMainMenuGroup::ExtractFileName(const char* FName)
{
//  string trial = SystemTools::ConvertToUnixOutputPath(FName);
//  const char* ptr = strrchr(SystemTools::ConvertToUnixOutputPath(FName).c_str(), '/');
  const char *ptr = strrchr(FName, '\\');
   if(ptr)
  {
    if(ptr[1]!='\0')
    {
      return ptr+1;
    }
    else
    {
      return ptr;
    }
  }
  return NULL;
}
