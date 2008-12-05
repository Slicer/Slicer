/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxQualityMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.19.4.2 $

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

#include "vtkKWMimxQualityMenuGroup.h"

/* VTK Headers */
#include "vtkObjectFactory.h"
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

/* KWWidget Headers */
#include "vtkKWApplication.h"
#include "vtkKWFrame.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"

/* MIMX Headers */
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxEditFEMeshLaplacianSmoothGroup.h"
#include "vtkKWMimxEvaluateMeshQualityGroup.h"
#include "vtkKWMimxExportAbaqusFEMeshGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxSaveVTKFEMeshGroup.h"



//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxQualityMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxQualityMenuGroup, "$Revision: 1.19.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxQualityMenuGroup::vtkKWMimxQualityMenuGroup()
{
        this->OperationMenuButton = NULL;
  this->EvaluateMeshInterface = NULL;
  this->FEMeshLaplacianSmoothGroup = NULL;
  this->ExportAbaqusFEMeshGroup = NULL;
        this->SaveVTKFEMeshGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxQualityMenuGroup::~vtkKWMimxQualityMenuGroup()
{
  if(this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if(this->EvaluateMeshInterface)
          this->EvaluateMeshInterface->Delete();
        if(this->FEMeshLaplacianSmoothGroup)
          this->FEMeshLaplacianSmoothGroup->Delete();
        if(this->ExportAbaqusFEMeshGroup)
    this->ExportAbaqusFEMeshGroup->Delete();
  if(this->SaveVTKFEMeshGroup)
    this->SaveVTKFEMeshGroup->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::CreateWidget()
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
                "Evaluate / Display Mesh Quality",this, "EvaluateMeshCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
                "Mesh Improvement",this, "SmoothLaplacianFEMeshCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Export Abaqus File",this, "ExportAbaqusFEMeshCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Save",this, "SaveVTKFEMeshCallback");        
          
        // Set the default value
        this->OperationMenuButton->GetWidget()->SetValue( "Evaluate / Display Mesh Quality" );  
        
}
//----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::Update()
{
        this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::EvaluateMeshCallback()
{
  this->CreateMeshQualityGroup();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::EvaluateMeshVolumeCallback()
{
  this->CreateMeshQualityGroup();
  this->EvaluateMeshInterface->SetQualityTypeToVolume();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::EvaluateMeshEdgeCollapseCallback()
{
  this->CreateMeshQualityGroup();
  this->EvaluateMeshInterface->SetQualityTypeToEdgeCollapse();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::EvaluateMeshJacobianCallback()
{
  this->CreateMeshQualityGroup();
  this->EvaluateMeshInterface->SetQualityTypeToJacobian();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::EvaluateMeshSkewCallback()
{
  this->CreateMeshQualityGroup();
  this->EvaluateMeshInterface->SetQualityTypeToSkew();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::EvaluateMeshAngleCallback()
{
  this->CreateMeshQualityGroup();
  this->EvaluateMeshInterface->SetQualityTypeToAngle();
}

//-----------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::CreateMeshQualityGroup()
{
  if (!this->EvaluateMeshInterface)
  {
    this->EvaluateMeshInterface = vtkKWMimxEvaluateMeshQualityGroup::New();
    this->EvaluateMeshInterface->SetApplication(this->GetApplication());
    this->EvaluateMeshInterface->SetParent( this->GetParent() );
    this->EvaluateMeshInterface->SetFEMeshList(this->FEMeshList);
    this->EvaluateMeshInterface->SetMimxMainWindow(this->GetMimxMainWindow());
          this->EvaluateMeshInterface->SetViewProperties(
                   this->GetMimxMainWindow()->GetViewProperties());
          this->EvaluateMeshInterface->SetMenuGroup(this);
    this->EvaluateMeshInterface->Create();
  }
  else
  {
          this->EvaluateMeshInterface->UpdateObjectLists();
  }
  this->EvaluateMeshInterface->ClearStatsEntry();
  
  this->SetMenuButtonsEnabled(0);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 0 -pady 2 -fill x", 
    this->EvaluateMeshInterface->GetMainFrame()->GetWidgetName());
 
}

//------------------------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::SmoothLaplacianFEMeshCallback()
{
        if(!this->FEMeshLaplacianSmoothGroup)
        {
                this->FEMeshLaplacianSmoothGroup = vtkKWMimxEditFEMeshLaplacianSmoothGroup::New();
                this->FEMeshLaplacianSmoothGroup->SetParent(this->GetParent() /*this->MainFrame->GetFrame()*/);
                this->FEMeshLaplacianSmoothGroup->SetSurfaceList(this->SurfaceList);
                this->FEMeshLaplacianSmoothGroup->SetFEMeshList(this->FEMeshList);
                this->FEMeshLaplacianSmoothGroup->SetBBoxList(this->BBoxList);
                this->FEMeshLaplacianSmoothGroup->SetMenuGroup(this);
                this->FEMeshLaplacianSmoothGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->FEMeshLaplacianSmoothGroup->Create();
        }
        else
        {
                this->FEMeshLaplacianSmoothGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                this->FEMeshLaplacianSmoothGroup->GetMainFrame()->GetWidgetName());
}

//--------------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::UpdateObjectLists()
{
        if(this->EvaluateMeshInterface)
                this->EvaluateMeshInterface->UpdateObjectLists();
}

//---------------------------------------------------------------------------------------
void vtkKWMimxQualityMenuGroup::SaveVTKFEMeshCallback()
{
        if(!this->SaveVTKFEMeshGroup)
        {
                this->SaveVTKFEMeshGroup = vtkKWMimxSaveVTKFEMeshGroup::New();
                this->SaveVTKFEMeshGroup->SetParent( this->GetParent() );
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
void vtkKWMimxQualityMenuGroup::ExportAbaqusFEMeshCallback()
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
void vtkKWMimxQualityMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "FEMeshList: " << this->FEMeshList << endl;
  
  os << indent << "MainFrame: " << this->MainFrame << endl;
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  
  os << indent << "EvaluateMeshInterface: " << this->EvaluateMeshInterface << endl;
  os << indent << "FEMeshLaplacianSmoothGroup: " << this->FEMeshLaplacianSmoothGroup << endl;
  os << indent << "ExportAbaqusFEMeshGroup: " << this->ExportAbaqusFEMeshGroup << endl;
  os << indent << "SaveVTKFEMeshGroup: " << this->SaveVTKFEMeshGroup << endl;
}
//---------------------------------------------------------------------------
