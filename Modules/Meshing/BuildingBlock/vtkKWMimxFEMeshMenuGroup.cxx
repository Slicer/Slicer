/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxFEMeshMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.70.4.2 $

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

#include "vtkKWMimxFEMeshMenuGroup.h"

#include "vtkActor.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
#include "vtkKWIcon.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"

#include "vtkMimxErrorCallback.h"
#include "vtkMimxMeshActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkKWMimxCreateFEMeshFromBBGroup.h"
#include "vtkKWMimxDeleteObjectGroup.h"
#include "vtkKWMimxEditBBMeshSeedGroup.h"
#include "vtkKWMimxEditElementSetNumbersGroup.h"
#include "vtkKWMimxEditBBMeshSeedGroup.h"
#include "vtkKWMimxExportAbaqusFEMeshGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWMimxLoadMeshGroup.h"
#include "vtkKWMimxSaveVTKFEMeshGroup.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/list>
#include <vtksys/SystemTools.hxx>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxFEMeshMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxFEMeshMenuGroup, "$Revision: 1.70.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxFEMeshMenuGroup::vtkKWMimxFEMeshMenuGroup()
{
        this->OperationMenuButton = NULL;
  this->FEMeshList = vtkLinkedListWrapper::New();
  this->ImageList = NULL;
  this->BBoxList = NULL;
  this->FEMeshFromBB = NULL;
  this->MainFrame = NULL;
  this->SaveVTKFEMeshGroup = NULL;
  this->DeleteObjectGroup = NULL;
  this->EditElementSetNumbersGroup = NULL;
  this->ExportAbaqusFEMeshGroup = NULL;
  this->EditBBMeshSeedGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxFEMeshMenuGroup::~vtkKWMimxFEMeshMenuGroup()
{
  if (this->FEMeshList) 
  {
    for (int i=0; i < this->FEMeshList->GetNumberOfItems(); i++) {
      vtkMimxMeshActor::SafeDownCast(
        this->FEMeshList->GetItem(i))->Delete();
    }
    this->FEMeshList->Delete();
  }

  if (this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if (this->FEMeshFromBB)
    this->FEMeshFromBB->Delete();
  if(this->SaveVTKFEMeshGroup)
          this->SaveVTKFEMeshGroup->Delete();
  if(this->DeleteObjectGroup)
          this->DeleteObjectGroup->Delete();
  if(this->EditElementSetNumbersGroup)
          EditElementSetNumbersGroup->Delete();
  if(this->ExportAbaqusFEMeshGroup)
          this->ExportAbaqusFEMeshGroup->Delete();
  if (this->EditBBMeshSeedGroup)
          this->EditBBMeshSeedGroup->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateWidget()
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
    "Assign/Edit Mesh Seeds",this, "EditBBMeshSeedCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Create",this, "CreateFEMeshFromBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Load",this, "LoadVTKFEMeshCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Renumber Nodes/Elements",this, "EditElementNumbersCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Export ABAQUS File",this, "ExportAbaqusFEMeshCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Delete",this, "DeleteFEMeshCallback");
        this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
          "Save",this, "SaveVTKFEMeshCallback");

  //Set the Default Option
        this->OperationMenuButton->GetWidget()->SetValue( "Assign/Edit Mesh Seeds" );       
}

//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::Update()
{
        this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::UpdateEnableState()
{
        this->Superclass::UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  
  os << indent << "FEMeshList: " << this->FEMeshList << endl;
  os << indent << "ImageList: " << this->ImageList << endl;
  os << indent << "BBoxList: " << this->BBoxList << endl;
  
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  os << indent << "MainFrame: " << this->MainFrame << endl;
  os << indent << "FEMeshFromBB: " << this->FEMeshFromBB << endl;
  os << indent << "SaveVTKFEMeshGroup: " << this->SaveVTKFEMeshGroup << endl;  
  os << indent << "DeleteObjectGroup: " << this->DeleteObjectGroup << endl;
  os << indent << "EditElementSetNumbersGroup: " << this->EditElementSetNumbersGroup << endl;
  os << indent << "ExportAbaqusFEMeshGroup: " << this->ExportAbaqusFEMeshGroup << endl;
  os << indent << "EditBBMeshSeedGroup: " << this->EditBBMeshSeedGroup << endl;
}

//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateFEMeshFromBBCallback()
{
  if (!this->FEMeshFromBB)
  {
    this->FEMeshFromBB = vtkKWMimxCreateFEMeshFromBBGroup::New();
    this->FEMeshFromBB->SetApplication(this->GetApplication());
    this->FEMeshFromBB->SetParent( this->GetParent() );
    this->FEMeshFromBB->SetSurfaceList(this->SurfaceList);
    this->FEMeshFromBB->SetBBoxList(this->BBoxList);
    this->FEMeshFromBB->SetFEMeshList(this->FEMeshList);
    this->FEMeshFromBB->SetMimxMainWindow(this->GetMimxMainWindow());
    this->FEMeshFromBB->SetViewProperties(
                this->GetMimxMainWindow()->GetViewProperties());
                this->FEMeshFromBB->SetMenuGroup(this);
    this->FEMeshFromBB->Create();
  }
  else
  {
          this->FEMeshFromBB->UpdateObjectLists();
  }
  this->SetMenuButtonsEnabled(0);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 0 -pady 2 -fill x", 
    this->FEMeshFromBB->GetMainFrame()->GetWidgetName());
 
}

//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::LoadVTKFEMeshCallback()
{

        vtkKWMimxLoadMeshGroup *loadMeshGroup = vtkKWMimxLoadMeshGroup::New();
        loadMeshGroup->SetParent( this->GetParent() );
  loadMeshGroup->SetFEMeshList(this->FEMeshList);
  loadMeshGroup->SetMenuGroup(this);
  loadMeshGroup->SetMimxMainWindow(this->GetMimxMainWindow());
  loadMeshGroup->Create();
  loadMeshGroup->Delete();
}

//---------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::DeleteFEMeshCallback()
{
        if(!this->DeleteObjectGroup)
        {
                this->DeleteObjectGroup = vtkKWMimxDeleteObjectGroup::New();
                this->DeleteObjectGroup->SetParent(this->GetParent() /*this->MainFrame->GetFrame()*/);
                this->DeleteObjectGroup->SetBBoxList(NULL);
                this->DeleteObjectGroup->SetSurfaceList(NULL);
                this->DeleteObjectGroup->SetFEMeshList(this->FEMeshList);
                this->DeleteObjectGroup->SetMenuGroup(this);
                this->DeleteObjectGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->DeleteObjectGroup->SetViewProperties(
                        this->GetMimxMainWindow()->GetViewProperties());
                this->DeleteObjectGroup->Create();
        }
        else
        {
                this->DeleteObjectGroup->SetBBoxList(NULL);
                this->DeleteObjectGroup->SetSurfaceList(NULL);
                this->DeleteObjectGroup->SetFEMeshList(this->FEMeshList);
                this->DeleteObjectGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->DeleteObjectGroup->GetObjectListComboBox()->SetLabelText("Mesh: ");
        this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                this->DeleteObjectGroup->GetMainFrame()->GetWidgetName());
}

//---------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::SaveVTKFEMeshCallback()
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
void vtkKWMimxFEMeshMenuGroup::UpdateObjectLists()
{
        if(this->FEMeshFromBB)
                this->FEMeshFromBB->UpdateObjectLists();
        if(this->SaveVTKFEMeshGroup)
                this->SaveVTKFEMeshGroup->UpdateObjectLists();
        if(this->DeleteObjectGroup)
                this->DeleteObjectGroup->UpdateObjectLists();
}

//------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::EditElementNumbersCallback()
{
        if(!this->EditElementSetNumbersGroup)
        {
                this->EditElementSetNumbersGroup = vtkKWMimxEditElementSetNumbersGroup::New();
                this->EditElementSetNumbersGroup->SetParent(this->MainFrame);
                this->EditElementSetNumbersGroup->SetFEMeshList(this->FEMeshList);
                this->EditElementSetNumbersGroup->SetMenuGroup(this);
                this->EditElementSetNumbersGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->EditElementSetNumbersGroup->SetViewProperties(
                        this->GetMimxMainWindow()->GetViewProperties());
                this->EditElementSetNumbersGroup->Create();
        }
        else
        {
                this->EditElementSetNumbersGroup->UpdateObjectLists();
        }
        this->SetMenuButtonsEnabled(0);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5 -fill x", 
                this->EditElementSetNumbersGroup->GetMainFrame()->GetWidgetName());
}

//-------------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::ExportAbaqusFEMeshCallback()
{
        if(!this->ExportAbaqusFEMeshGroup)
        {
                this->ExportAbaqusFEMeshGroup = vtkKWMimxExportAbaqusFEMeshGroup::New();
                this->ExportAbaqusFEMeshGroup->SetParent(this->GetParent() );
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

//------------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::EditBBMeshSeedCallback()
{
        if(!this->EditBBMeshSeedGroup)
        {
                this->EditBBMeshSeedGroup = vtkKWMimxEditBBMeshSeedGroup::New();
                this->EditBBMeshSeedGroup->SetParent(this->GetParent() );
                this->EditBBMeshSeedGroup->SetSurfaceList(this->SurfaceList);
                this->EditBBMeshSeedGroup->SetFEMeshList(this->FEMeshList);
                this->EditBBMeshSeedGroup->SetBBoxList(this->BBoxList);
                this->EditBBMeshSeedGroup->SetMenuGroup(this);
                this->EditBBMeshSeedGroup->SetMimxMainWindow(this->GetMimxMainWindow());
                this->EditBBMeshSeedGroup->SetViewProperties(
                        this->GetMimxMainWindow()->GetViewProperties());
                this->EditBBMeshSeedGroup->Create();
        }
        else
        {
                this->EditBBMeshSeedGroup->UpdateObjectLists();
        }
        this->EditBBMeshSeedGroup->InitializeState();
        this->SetMenuButtonsEnabled(0);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

        this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5 -fill x", 
                this->EditBBMeshSeedGroup->GetMainFrame()->GetWidgetName());
}
//------------------------------------------------------------------------------------------------------
