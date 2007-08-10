/*=========================================================================

  Module:    $RCSfile: vtkKWMimxMainNotebook.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxMainNotebook.h"

#include "vtkActor.h"
#include "vtkPolyDataMapper.h"
#include "vtkSTLReader.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWIcon.h"
#include "vtkKWInternationalization.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMainNotebook);
vtkCxxRevisionMacro(vtkKWMimxMainNotebook, "$Revision: 1.7 $");

//----------------------------------------------------------------------------
vtkKWMimxMainNotebook::vtkKWMimxMainNotebook()
{
  this->Notebook = NULL;
  this->MimxViewWindow = NULL;
  this->SurfaceMenuGroup = NULL;
  this->FEMeshMenuGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMainNotebook::~vtkKWMimxMainNotebook()
{
  if(this->Notebook)
    this->Notebook->Delete();
  if(this->SurfaceMenuGroup)
    this->SurfaceMenuGroup->Delete();
  if(this->FEMeshMenuGroup)
    this->FEMeshMenuGroup->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainNotebook::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
  if(!this->Notebook)  this->Notebook = vtkKWNotebook::New();
  this->Notebook->SetParent(this);
  this->Notebook->Create();
  this->Notebook->UseFrameWithScrollbarsOn();
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->Notebook->GetWidgetName());

  this->Notebook->AddPage("Image");
  this->Notebook->AddPage("Surface");
  this->Notebook->AddPage("F E Mesh");
  this->Notebook->AddPage("Mesh Quality");

  if(!this->SurfaceMenuGroup)  this->SurfaceMenuGroup = vtkKWMimxSurfaceMenuGroup::New();

  this->SurfaceMenuGroup->SetParent(this->Notebook->GetFrame("Surface"));
  this->SurfaceMenuGroup->SetMimxViewWindow(this->GetMimxViewWindow());
  this->SurfaceMenuGroup->SetApplication(this->GetApplication());

  this->SurfaceMenuGroup->Create();

  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->SurfaceMenuGroup->GetWidgetName());

  // generate menu items for FEMesh (which includes the mesh and bounding box)
  if(!this->FEMeshMenuGroup)  this->FEMeshMenuGroup = vtkKWMimxFEMeshMenuGroup::New();

  this->FEMeshMenuGroup->SetParent(this->Notebook->GetFrame("F E Mesh"));
  this->FEMeshMenuGroup->SetMimxViewWindow(this->GetMimxViewWindow());
  this->FEMeshMenuGroup->SetApplication(this->GetApplication());

  this->FEMeshMenuGroup->Create();

  this->FEMeshMenuGroup->SetMimxViewWindow(this->GetMimxViewWindow());

  this->GetApplication()->Script("pack %s -side top -anchor nw  -expand y -padx 2 -pady 5", 
    this->FEMeshMenuGroup->GetWidgetName());
  this->SetLists();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainNotebook::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxMainNotebook::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxMainNotebook::SetLists()
{
  // list for surface operations
  this->SurfaceMenuGroup->SetFEMeshList(this->FEMeshMenuGroup->GetFEMeshList());
  // list for FEMesh operations
  this->FEMeshMenuGroup->SetSurfaceList(this->SurfaceMenuGroup->GetSurfaceList());
}
//----------------------------------------------------------------------------
void vtkKWMimxMainNotebook::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
