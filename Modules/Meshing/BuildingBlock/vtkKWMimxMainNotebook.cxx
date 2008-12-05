/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMainNotebook.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.39.4.2 $

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

#include "vtkKWMimxMainNotebook.h"

/* VTK Headers */
#include "vtkObjectFactory.h"
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

/* KWWidget Headers */
#include "vtkKWApplication.h"
#include "vtkKWEvent.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkKWUserInterfaceManagerNotebook.h"

/* MIMX Headers */
#include "vtkKWMimxBBMenuGroup.h"
#include "vtkKWMimxBoundaryConditionsMenuGroup.h"
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxImageMenuGroup.h"
#include "vtkKWMimxMaterialPropertyMenuGroup.h"
#include "vtkKWMimxQualityMenuGroup.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxSurfaceMenuGroup.h"


// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMainNotebook);
vtkCxxRevisionMacro(vtkKWMimxMainNotebook, "$Revision: 1.39.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxMainNotebook::vtkKWMimxMainNotebook()
{
        this->Notebook = NULL;
        this->MimxMainWindow = NULL;
        this->SurfaceMenuGroup = NULL;
        this->BBMenuGroup = NULL;
        this->FEMeshMenuGroup = NULL;
        this->ImageMenuGroup = NULL;
  this->QualityMenuGroup = NULL;
        this->DoUndoTree = NULL;
        this->MaterialPropertyMenuGroup = NULL;
        this->BoundaryConditionsMenuGroup = NULL;
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
        if(this->BBMenuGroup)
                this->BBMenuGroup->Delete();
        if(this->ImageMenuGroup)
                this->ImageMenuGroup->Delete();
  if(this->MaterialPropertyMenuGroup)
          this->MaterialPropertyMenuGroup->Delete();
  if(this->BoundaryConditionsMenuGroup)
          this->BoundaryConditionsMenuGroup->Delete();  
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
  
  if (!this->Notebook)  
          this->Notebook = vtkKWNotebook::New();
        this->Notebook->SetParent( this );
        this->Notebook->Create();
        
  /* This disables the Pin/Show Menu */
  this->Notebook->EnablePageTabContextMenuOff();
  this->Notebook->SetMinimumHeight(550);
  
  /* Add the Notebook Pages */          
        this->Notebook->AddPage("Image");
        this->Notebook->AddPage("Surface");
        this->Notebook->AddPage("Block(s)");
        this->Notebook->AddPage("Mesh");
        this->Notebook->AddPage("Quality");
        this->Notebook->AddPage("Materials");
        this->Notebook->AddPage("Load/BC");
        this->Notebook->Raise();
        this->Notebook->RaisePage("Surface");
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand yes -padx 2 -pady 2 -fill both", 
                this->Notebook->GetWidgetName());
                
        /* Image Page */
  if(!this->ImageMenuGroup)     
    this->ImageMenuGroup = vtkKWMimxImageMenuGroup::New();
        this->ImageMenuGroup->SetParent(this->Notebook->GetFrame("Image"));
        this->ImageMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->ImageMenuGroup->SetApplication(this->GetApplication());
        this->ImageMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->ImageMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5", 
                this->ImageMenuGroup->GetWidgetName());

  /* Surface Page */
  if (!this->SurfaceMenuGroup)  
    this->SurfaceMenuGroup = vtkKWMimxSurfaceMenuGroup::New();
        this->SurfaceMenuGroup->SetParent(this->Notebook->GetFrame("Surface"));
        this->SurfaceMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->SurfaceMenuGroup->SetApplication(this->GetApplication());
        this->SurfaceMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->SurfaceMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5", 
                this->SurfaceMenuGroup->GetWidgetName());

  /* Building Block Page */
  if (!this->BBMenuGroup)       
    this->BBMenuGroup = vtkKWMimxBBMenuGroup::New();
        this->BBMenuGroup->SetParent(this->Notebook->GetFrame("Block(s)"));
        this->BBMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->BBMenuGroup->SetApplication(this->GetApplication());
        this->BBMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->BBMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill both", 
                this->BBMenuGroup->GetWidgetName());
                
        /* Mesh Page */
        if (!this->FEMeshMenuGroup)     
          this->FEMeshMenuGroup = vtkKWMimxFEMeshMenuGroup::New();
        this->FEMeshMenuGroup->SetParent(this->Notebook->GetFrame("Mesh"));
        this->FEMeshMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->FEMeshMenuGroup->SetApplication(this->GetApplication());
        this->FEMeshMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->FEMeshMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw  -expand n -padx 2 -pady 5", 
                this->FEMeshMenuGroup->GetWidgetName());
        
        /* Quality Page */
        if (!this->QualityMenuGroup)    
          this->QualityMenuGroup = vtkKWMimxQualityMenuGroup::New();
        this->QualityMenuGroup->SetParent(this->Notebook->GetFrame("Quality"));
        this->QualityMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->QualityMenuGroup->SetApplication(this->GetApplication());
        this->QualityMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->QualityMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5", 
                this->QualityMenuGroup->GetWidgetName());

  /* Material Property Page */
  if (!this->MaterialPropertyMenuGroup) 
                this->MaterialPropertyMenuGroup = vtkKWMimxMaterialPropertyMenuGroup::New();
        this->MaterialPropertyMenuGroup->SetParent(this->Notebook->GetFrame("Materials"));
        this->MaterialPropertyMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->MaterialPropertyMenuGroup->SetApplication(this->GetApplication());
        this->MaterialPropertyMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->MaterialPropertyMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5", 
                this->MaterialPropertyMenuGroup->GetWidgetName());

  /* Boundary Condition Page */
  if (!this->BoundaryConditionsMenuGroup)       
                this->BoundaryConditionsMenuGroup = vtkKWMimxBoundaryConditionsMenuGroup::New();
        this->BoundaryConditionsMenuGroup->SetParent(this->Notebook->GetFrame("Load/BC"));
        this->BoundaryConditionsMenuGroup->SetMimxMainWindow(this->GetMimxMainWindow());
        this->BoundaryConditionsMenuGroup->SetApplication(this->GetApplication());
        this->BoundaryConditionsMenuGroup->SetDoUndoTree(this->DoUndoTree);
        this->BoundaryConditionsMenuGroup->Create();
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5", 
                this->BoundaryConditionsMenuGroup->GetWidgetName());

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
        this->SurfaceMenuGroup->SetEnabled(this->GetEnabled());
        this->ImageMenuGroup->SetEnabled(this->GetEnabled());
        this->FEMeshMenuGroup->SetEnabled(this->GetEnabled());
        this->BBMenuGroup->SetEnabled(this->GetEnabled());
        this->QualityMenuGroup->SetEnabled(this->GetEnabled());
        this->MaterialPropertyMenuGroup->SetEnabled(this->GetEnabled());
        this->BoundaryConditionsMenuGroup->SetEnabled(this->GetEnabled());

        this->Notebook->SetPageEnabled("Image", this->GetEnabled());
        this->Notebook->SetPageEnabled("Surface", this->GetEnabled());
        this->Notebook->SetPageEnabled("Block(s)", this->GetEnabled());
        this->Notebook->SetPageEnabled("Mesh", this->GetEnabled());
        this->Notebook->SetPageEnabled("Quality", this->GetEnabled());
        this->Notebook->SetPageEnabled("Materials", this->GetEnabled());
        this->Notebook->SetPageEnabled("Load/BC", this->GetEnabled());
        this->Notebook->SetEnabled(this->GetEnabled());
}

//----------------------------------------------------------------------------
void vtkKWMimxMainNotebook::SetLists()
{
  // Lists used for surface operations
  this->SurfaceMenuGroup->SetFEMeshList(this->FEMeshMenuGroup->GetFEMeshList());
  this->SurfaceMenuGroup->SetBBoxList(this->BBMenuGroup->GetBBoxList());
  
  // Lists used for Mesh operations
  this->FEMeshMenuGroup->SetSurfaceList(this->SurfaceMenuGroup->GetSurfaceList());
  this->FEMeshMenuGroup->SetImageList(this->ImageMenuGroup->GetImageList());
  this->FEMeshMenuGroup->SetBBoxList(this->BBMenuGroup->GetBBoxList());
  
  // List used for Building Block Operations
  this->BBMenuGroup->SetSurfaceList(this->SurfaceMenuGroup->GetSurfaceList());

  // Lists used for Quality Operations
  this->QualityMenuGroup->SetFEMeshList(this->FEMeshMenuGroup->GetFEMeshList());
  this->QualityMenuGroup->SetBBoxList(this->BBMenuGroup->GetBBoxList());
  this->QualityMenuGroup->SetSurfaceList(this->SurfaceMenuGroup->GetSurfaceList());
  
  // Lists used for Material Property Operations
  this->MaterialPropertyMenuGroup->SetFEMeshList(this->FEMeshMenuGroup->GetFEMeshList());
  this->MaterialPropertyMenuGroup->SetImageList(this->ImageMenuGroup->GetImageList());
  
  // Lists used for Boundary Condition Operations
  this->BoundaryConditionsMenuGroup->SetFEMeshList(this->FEMeshMenuGroup->GetFEMeshList());  
}

//----------------------------------------------------------------------------
void vtkKWMimxMainNotebook::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  this->Superclass::PrintSelf(os,indent);
  os << indent << "MimxMainWindow: " << this->MimxMainWindow << endl;
  os << indent << "Notebook: " << this->Notebook << endl;
  os << indent << "SurfaceMenuGroup: " << this->SurfaceMenuGroup << endl;
  os << indent << "BBMenuGroup: " << this->BBMenuGroup << endl;
  os << indent << "FEMeshMenuGroup: " << this->FEMeshMenuGroup << endl;
  os << indent << "ImageMenuGroup: " << this->ImageMenuGroup << endl;
  os << indent << "QualityMenuGroup: " << this->QualityMenuGroup << endl;
  os << indent << "MaterialPropertyMenuGroup: " << this->MaterialPropertyMenuGroup << endl;
  os << indent << "BoundaryConditionsMenuGroup: " << this->BoundaryConditionsMenuGroup << endl;
  os << indent << "DoUndoTree: " << this->DoUndoTree << endl;
}
//----------------------------------------------------------------------------
