/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMaterialPropertyMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.10.2.2 $

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

#include "vtkKWMimxMaterialPropertyMenuGroup.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"

#include "vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup.h"
#include "vtkKWMimxConstMatPropElSetGroup.h"
#include "vtkKWMimxDisplayMatPropGroup.h"
#include "vtkKWMimxExportAbaqusFEMeshGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxSaveVTKFEMeshGroup.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/list>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMaterialPropertyMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxMaterialPropertyMenuGroup, "$Revision: 1.10.2.2 $");

//----------------------------------------------------------------------------
vtkKWMimxMaterialPropertyMenuGroup::vtkKWMimxMaterialPropertyMenuGroup()
{
        this->ImageList = NULL;
        this->FEMeshList = NULL;
        
        this->OperationMenuButton = NULL;
        this->ConstMatPropElSetGroup = NULL;
        this->ImageMatPropGroup = NULL;
        this->DisplayMaterialPropertyGroup = NULL;
        this->ExportAbaqusFEMeshGroup = NULL;
        this->SaveVTKFEMeshGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMaterialPropertyMenuGroup::~vtkKWMimxMaterialPropertyMenuGroup()
{
  if(this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if(this->ConstMatPropElSetGroup)
    this->ConstMatPropElSetGroup->Delete();
  if(this->ImageMatPropGroup)
    this->ImageMatPropGroup->Delete();
  if(this->DisplayMaterialPropertyGroup)
    this->DisplayMaterialPropertyGroup->Delete();
  if(this->ExportAbaqusFEMeshGroup)
    this->ExportAbaqusFEMeshGroup->Delete();
  if(this->SaveVTKFEMeshGroup)
    this->SaveVTKFEMeshGroup->Delete();
}
 
//----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::CreateWidget()
{
        if(this->IsCreated())
        {
                vtkErrorMacro("class already created");
                return;
        }
        this->Superclass::CreateWidget();
        
  if(!this->MainFrame)
    this->MainFrame = vtkKWFrame::New();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

        // declare operations menu
        if(!this->OperationMenuButton)  
                this->OperationMenuButton = vtkKWMenuButtonWithLabel::New();
        this->OperationMenuButton->SetParent(this->MainFrame);
        this->OperationMenuButton->Create();
        this->OperationMenuButton->SetBorderWidth(0);
        this->OperationMenuButton->SetReliefToGroove();
        this->OperationMenuButton->GetWidget()->SetWidth(40);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 15 ", 
                this->OperationMenuButton->GetWidgetName());
        this->OperationMenuButton->SetEnabled(1);
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "User-Defined",this, "AssignMaterialPropertiesElementSetCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Image-Based",this, "AssignImageBasedMaterialPropertiesCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Display Material Properties",this, "DisplayMaterialPropertyCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Export ABAQUS File",this, "ExportAbaqusFEMeshCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Save",this, "SaveVTKFEMeshCallback");
        
        // Set the default value
        this->OperationMenuButton->GetWidget()->SetValue( "User-Defined" );     
}

//----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::Update()
{
        this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}

//-----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::AssignMaterialPropertiesElementSetCallback()
{
        if (!this->ConstMatPropElSetGroup)
        {
                this->ConstMatPropElSetGroup = vtkKWMimxConstMatPropElSetGroup::New();
                this->ConstMatPropElSetGroup->SetApplication(this->GetApplication());
                this->ConstMatPropElSetGroup->SetParent( this->GetParent() );
                this->ConstMatPropElSetGroup->SetSurfaceList(this->SurfaceList);
                this->ConstMatPropElSetGroup->SetBBoxList(this->BBoxList);
                this->ConstMatPropElSetGroup->SetFEMeshList(this->FEMeshList);
                this->ConstMatPropElSetGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->ConstMatPropElSetGroup->SetViewProperties(
                        this->GetMimxMainWindow()->GetViewProperties());
                this->ConstMatPropElSetGroup->SetMenuGroup(this);
                this->ConstMatPropElSetGroup->Create();
        }
        else
        {
                this->ConstMatPropElSetGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand n -padx 0 -pady 2 -fill x", 
                this->ConstMatPropElSetGroup->GetMainFrame()->GetWidgetName());
}

//-----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::DisplayMaterialPropertyCallback()
{
        if (!this->DisplayMaterialPropertyGroup)
        {
                this->DisplayMaterialPropertyGroup = vtkKWMimxDisplayMatPropGroup::New();
                this->DisplayMaterialPropertyGroup->SetApplication(this->GetApplication());
                this->DisplayMaterialPropertyGroup->SetParent( this->GetParent() );
                this->DisplayMaterialPropertyGroup->SetFEMeshList(this->FEMeshList);
                this->DisplayMaterialPropertyGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->DisplayMaterialPropertyGroup->SetViewProperties(
                        this->GetMimxMainWindow()->GetViewProperties());
                this->DisplayMaterialPropertyGroup->SetMenuGroup(this);
                this->DisplayMaterialPropertyGroup->Create();
        }
        else
        {
                this->DisplayMaterialPropertyGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand n -padx 0 -pady 2 -fill x", 
                this->DisplayMaterialPropertyGroup->GetMainFrame()->GetWidgetName());
}

//-----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::UpdateObjectLists()
{
        if(this->ConstMatPropElSetGroup)
                this->ConstMatPropElSetGroup->UpdateObjectLists();
}

//----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::AssignImageBasedMaterialPropertiesCallback()
{
                if(!this->ImageMatPropGroup)
                {
                        this->ImageMatPropGroup = vtkKWMimxApplyFEMeshMaterialPropertiesFromImageGroup::New();
                        this->ImageMatPropGroup->SetParent( this->GetParent() );
                        this->ImageMatPropGroup->SetFEMeshList(this->FEMeshList);
                        this->ImageMatPropGroup->SetImageList(this->ImageList);
                        this->ImageMatPropGroup->SetMenuGroup(this);
                        this->ImageMatPropGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                        this->ImageMatPropGroup->SetViewProperties(
                                this->GetMimxMainWindow()->GetViewProperties());
                        this->ImageMatPropGroup->Create();
                }
                else
                {
                        this->ImageMatPropGroup->UpdateObjectLists();
                }
                this->SetMenuButtonsEnabled(0);
                  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);
        
                this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                        this->ImageMatPropGroup->GetMainFrame()->GetWidgetName());
}

//------------------------------------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::SaveVTKFEMeshCallback()
{
        if(!this->SaveVTKFEMeshGroup)
        {
                this->SaveVTKFEMeshGroup = vtkKWMimxSaveVTKFEMeshGroup::New();
                this->SaveVTKFEMeshGroup->SetParent(this->GetParent() );
                this->SaveVTKFEMeshGroup->SetFEMeshList(this->FEMeshList);
                this->SaveVTKFEMeshGroup->SetMenuGroup(this);
                this->SaveVTKFEMeshGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->SaveVTKFEMeshGroup->Create();
        }
        else
        {
                this->SaveVTKFEMeshGroup->UpdateObjectLists();
        }

        this->SetMenuButtonsEnabled(0);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                                                                   this->SaveVTKFEMeshGroup->GetMainFrame()->GetWidgetName());
}

//------------------------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::ExportAbaqusFEMeshCallback()
{
        if(!this->ExportAbaqusFEMeshGroup)
        {
                this->ExportAbaqusFEMeshGroup = vtkKWMimxExportAbaqusFEMeshGroup::New();
                this->ExportAbaqusFEMeshGroup->SetParent( this->GetParent() );
                this->ExportAbaqusFEMeshGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->ExportAbaqusFEMeshGroup->SetFEMeshList(this->FEMeshList);
                this->ExportAbaqusFEMeshGroup->SetMenuGroup(this);
                this->ExportAbaqusFEMeshGroup->Create();
        }
        else
        {
                this->ExportAbaqusFEMeshGroup->UpdateObjectLists();
        }

        this->SetMenuButtonsEnabled(0);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                this->ExportAbaqusFEMeshGroup->GetMainFrame()->GetWidgetName());
}

//----------------------------------------------------------------------------
void vtkKWMimxMaterialPropertyMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "ImageList: " << this->ImageList << endl;
  os << indent << "FEMeshList: " << this->FEMeshList << endl;
  
  os << indent << "MainFrame: " << this->MainFrame << endl;
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  
  os << indent << "ConstMatPropElSetGroup: " << this->ConstMatPropElSetGroup << endl;
  os << indent << "ImageMatPropGroup: " << this->ImageMatPropGroup << endl;
  os << indent << "DisplayMaterialPropertyGroup: " << this->DisplayMaterialPropertyGroup << endl;
  os << indent << "ExportAbaqusFEMeshGroup: " << this->ExportAbaqusFEMeshGroup << endl;
  os << indent << "SaveVTKFEMeshGroup: " << this->SaveVTKFEMeshGroup << endl;
        
}
//---------------------------------------------------------------------------
