/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxBoundaryConditionsMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.10.4.2 $

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

#include "vtkKWMimxBoundaryConditionsMenuGroup.h"

#include "vtkObjectFactory.h"

#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWTkUtilities.h"

#include "vtkKWMimxAssignBoundaryConditionsGroup.h"
#include "vtkKWMimxExportAbaqusFEMeshGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxSaveVTKFEMeshGroup.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/list>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxBoundaryConditionsMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxBoundaryConditionsMenuGroup, "$Revision: 1.10.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxBoundaryConditionsMenuGroup::vtkKWMimxBoundaryConditionsMenuGroup()
{
        this->OperationMenuButton = NULL;
        this->FEMeshList = NULL;
        this->AssignBoundaryConditionsGroup = NULL;
        this->ExportAbaqusFEMeshGroup = NULL;
        this->SaveVTKFEMeshGroup = NULL;
}
//----------------------------------------------------------------------------
vtkKWMimxBoundaryConditionsMenuGroup::~vtkKWMimxBoundaryConditionsMenuGroup()
{
  if(this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if(this->AssignBoundaryConditionsGroup)
    this->AssignBoundaryConditionsGroup->Delete();
  if(this->ExportAbaqusFEMeshGroup)
    this->ExportAbaqusFEMeshGroup->Delete();
  if(this->SaveVTKFEMeshGroup)
    this->SaveVTKFEMeshGroup->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxBoundaryConditionsMenuGroup::CreateWidget()
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
        this->OperationMenuButton->SetPadX(2);
        this->OperationMenuButton->SetPadY(2);
        this->OperationMenuButton->GetWidget()->SetWidth(40);
        this->GetApplication()->Script("pack %s -side top -anchor n -padx 2 -pady 15", 
                this->OperationMenuButton->GetWidgetName());
        this->OperationMenuButton->SetEnabled(1);
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "STEP - Load/BC Assignments",this, "AssignBoundaryConditionsCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Export ABAQUS File",this, "ExportAbaqusFEMeshCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Save",this, "SaveVTKFEMeshCallback");
                
        this->OperationMenuButton->GetWidget()->SetValue( "STEP - Load/BC Assignments" );       
        
}
//----------------------------------------------------------------------------
void vtkKWMimxBoundaryConditionsMenuGroup::Update()
{
        this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxBoundaryConditionsMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}

//-----------------------------------------------------------------------------
void vtkKWMimxBoundaryConditionsMenuGroup::AssignBoundaryConditionsCallback()
{
        if (!this->AssignBoundaryConditionsGroup)
        {
                this->AssignBoundaryConditionsGroup = vtkKWMimxAssignBoundaryConditionsGroup::New();
                this->AssignBoundaryConditionsGroup->SetApplication(this->GetApplication());
                this->AssignBoundaryConditionsGroup->SetParent( this->GetParent() );
                this->AssignBoundaryConditionsGroup->SetFEMeshList(this->FEMeshList);
                this->AssignBoundaryConditionsGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->AssignBoundaryConditionsGroup->SetViewProperties(
                        this->GetMimxMainWindow()->GetViewProperties());
                this->AssignBoundaryConditionsGroup->SetMenuGroup(this);
                this->AssignBoundaryConditionsGroup->Create();
        }
        else
        {
                this->AssignBoundaryConditionsGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script(
                "pack %s -side top -anchor nw -expand n -padx 0 -pady 2 -fill x", 
                this->AssignBoundaryConditionsGroup->GetMainFrame()->GetWidgetName());


}

//-----------------------------------------------------------------------------
void vtkKWMimxBoundaryConditionsMenuGroup::UpdateObjectLists()
{
}

//------------------------------------------------------------------------------
void vtkKWMimxBoundaryConditionsMenuGroup::SaveVTKFEMeshCallback()
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
void vtkKWMimxBoundaryConditionsMenuGroup::ExportAbaqusFEMeshCallback()
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
void vtkKWMimxBoundaryConditionsMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "FEMeshList: " << this->FEMeshList << endl;
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  
  os << indent << "AssignBoundaryConditionsGroup: " << this->AssignBoundaryConditionsGroup << endl;
  os << indent << "ExportAbaqusFEMeshGroup: " << this->ExportAbaqusFEMeshGroup << endl;
  os << indent << "SaveVTKFEMeshGroup: " << this->SaveVTKFEMeshGroup << endl;
}
//---------------------------------------------------------------------------
