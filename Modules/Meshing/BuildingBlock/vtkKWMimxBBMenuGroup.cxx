/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxBBMenuGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.32.4.2 $

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

#include "vtkKWMimxBBMenuGroup.h"

/* VTK Headers */
#include "vtkActor.h"
#include "vtkCellTypes.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>
#include <vtksys/SystemTools.hxx>

/* KWWidget Headers */
#include "vtkKWApplication.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWComboBoxWithLabel.h"

/* MIMX Headers */
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkKWMimxCreateBBFromBoundsGroup.h"
#include "vtkKWMimxDeleteObjectGroup.h"
#include "vtkKWMimxEditBBGroup.h"
#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWMimxLoadBBGroup.h"
#include "vtkKWMimxSaveVTKBBGroup.h"




//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxBBMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxBBMenuGroup, "$Revision: 1.32.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxBBMenuGroup::vtkKWMimxBBMenuGroup()
{
  this->OperationMenuButton = NULL;
  this->BBoxList = vtkLinkedListWrapper::New();
  this->CreateBBFromBounds = NULL;
  this->EditBB = NULL;
  this->MainFrame = NULL;
  this->SaveVTKBBGroup = NULL;
  this->DeleteObjectGroup = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxBBMenuGroup::~vtkKWMimxBBMenuGroup()
{
  if (this->BBoxList) 
    {
    for (int i=0; i < this->BBoxList->GetNumberOfItems(); i++) 
      {
      vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(i))->Delete();
      }
    this->BBoxList->Delete();
    }

  if (this->OperationMenuButton)
    this->OperationMenuButton->Delete();
  if(this->CreateBBFromBounds)
    this->CreateBBFromBounds->Delete();
  if(this->EditBB)
    this->EditBB->Delete();
  if(this->SaveVTKBBGroup)
    this->SaveVTKBBGroup->Delete();
  if(this->DeleteObjectGroup)
    this->DeleteObjectGroup->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::CreateWidget()
{
  if(this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }
  this->Superclass::CreateWidget();
        
  // Main Frame 
  if(!this->MainFrame)
    this->MainFrame = vtkKWFrame::New();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  // Operations Menu
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
        
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Create",this, "CreateBBFromBoundsCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Load",this, "LoadVTKBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Build/Edit",this, "EditBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Delete",this, "DeleteBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Save",this, "SaveVTKBBCallback");
        
  // Set the default value for the menu
  this->OperationMenuButton->GetWidget()->SetValue( "Create" );   
}

//----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::Update()
{
  this->UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}

//----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::LoadVTKBBCallback()
{
  vtkKWMimxLoadBBGroup *loadBlockGroup = vtkKWMimxLoadBBGroup::New();
  loadBlockGroup->SetParent( this->GetParent() );
  loadBlockGroup->SetBBoxList(this->BBoxList);
  loadBlockGroup->SetDoUndoTree(this->DoUndoTree);
  loadBlockGroup->SetMenuGroup(this);
  loadBlockGroup->SetMimxMainWindow(this->GetMimxMainWindow());
  loadBlockGroup->Create();
  loadBlockGroup->Delete();
}

//----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
  os << indent << "MainFrame: " << this->MainFrame << endl;
  os << indent << "OperationMenuButton: " << this->OperationMenuButton << endl;
  os << indent << "BBoxList: " << this->BBoxList << endl;
  os << indent << "CreateBBFromBounds: " << this->CreateBBFromBounds << endl;
  os << indent << "SaveVTKBBGroup: " << this->SaveVTKBBGroup << endl;
  os << indent << "DeleteObjectGroup: " << this->DeleteObjectGroup << endl;
}

//---------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::CreateBBFromBoundsCallback()
{
  if (!this->CreateBBFromBounds)
    {
    this->CreateBBFromBounds = vtkKWMimxCreateBBFromBoundsGroup::New();
    this->CreateBBFromBounds->SetApplication(this->GetApplication());
    this->CreateBBFromBounds->SetParent( this->GetParent() );
    this->CreateBBFromBounds->SetSurfaceList( this->SurfaceList );
    this->CreateBBFromBounds->SetBBoxList(this->BBoxList);
    this->CreateBBFromBounds->SetMimxMainWindow(this->GetMimxMainWindow());
    this->CreateBBFromBounds->SetDoUndoTree(this->DoUndoTree);
    this->CreateBBFromBounds->Create();
    this->CreateBBFromBounds->SetViewProperties(
      this->GetMimxMainWindow()->GetViewProperties());
    this->CreateBBFromBounds->SetMenuGroup(this);
    }
  else
    {
    this->CreateBBFromBounds->UpdateObjectLists();
    }
   
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);
  this->SetMenuButtonsEnabled(0);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 0 -pady 2 -fill x", 
                                 this->CreateBBFromBounds->GetMainFrame()->GetWidgetName()); 
}

//-----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::EditBBCallback()
{
  if (!this->EditBB)
    {
    this->EditBB = vtkKWMimxEditBBGroup::New();

    this->EditBB->SetApplication(this->GetApplication());
    this->EditBB->SetParent( this->GetParent() );
    this->EditBB->SetBBoxList(this->BBoxList);
    this->EditBB->SetDoUndoTree(this->DoUndoTree);
    this->EditBB->SetMimxMainWindow(this->GetMimxMainWindow());
    this->EditBB->Create();
    this->EditBB->SetViewProperties(
      this->GetMimxMainWindow()->GetViewProperties());
    this->EditBB->SetMenuGroup(this);
    }
  else
    {
    this->EditBB->UpdateObjectLists();
    }
  this->SetMenuButtonsEnabled(0);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
    this->EditBB->GetMainFrame()->GetWidgetName());

}
//-----------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::SaveVTKBBCallback()
{
  if(!this->SaveVTKBBGroup)
    {
    this->SaveVTKBBGroup = vtkKWMimxSaveVTKBBGroup::New();
    this->SaveVTKBBGroup->SetParent( this->GetParent() );
    this->SaveVTKBBGroup->SetBBoxList(this->BBoxList);
    this->SaveVTKBBGroup->SetMenuGroup(this);
    this->SaveVTKBBGroup->SetMimxMainWindow(this->GetMimxMainWindow());
    this->SaveVTKBBGroup->Create();
    }
  else
    {
    this->SaveVTKBBGroup->UpdateObjectLists();
    }
  this->SetMenuButtonsEnabled(0);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);

  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                                 this->SaveVTKBBGroup->GetMainFrame()->GetWidgetName());
}

//---------------------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::DeleteBBCallback()
{
  if(!this->DeleteObjectGroup)
    {
    this->DeleteObjectGroup = vtkKWMimxDeleteObjectGroup::New();
    this->DeleteObjectGroup->SetParent( this->GetParent() );
    this->DeleteObjectGroup->SetFEMeshList(NULL);
    this->DeleteObjectGroup->SetBBoxList(this->BBoxList);
    this->DeleteObjectGroup->SetDoUndoTree(this->DoUndoTree);
    this->DeleteObjectGroup->SetSurfaceList(NULL);          
    this->DeleteObjectGroup->SetMenuGroup(this);
    this->DeleteObjectGroup->SetViewProperties(
      this->GetMimxMainWindow()->GetViewProperties());
    this->DeleteObjectGroup->SetMimxMainWindow(this->GetMimxMainWindow());
    this->DeleteObjectGroup->Create();
    }
  else
    {
    this->DeleteObjectGroup->SetViewProperties(
      this->GetMimxMainWindow()->GetViewProperties());
    this->DeleteObjectGroup->SetFEMeshList(NULL);
    this->DeleteObjectGroup->SetBBoxList(this->BBoxList);
    this->DeleteObjectGroup->SetDoUndoTree(this->DoUndoTree);
    this->DeleteObjectGroup->SetSurfaceList(NULL);          
    this->DeleteObjectGroup->UpdateObjectLists();
    }
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(0);
  this->SetMenuButtonsEnabled(0);

  this->DeleteObjectGroup->GetObjectListComboBox()->SetLabelText("Building Block: ");
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand n -padx 2 -pady 5 -fill x", 
                                 this->DeleteObjectGroup->GetMainFrame()->GetWidgetName());
}

//--------------------------------------------------------------------------------
void vtkKWMimxBBMenuGroup::UpdateObjectLists()
{
  if(this->CreateBBFromBounds)
    this->CreateBBFromBounds->UpdateObjectLists();
  if(this->EditBB)
    this->EditBB->UpdateObjectLists();
  if(this->SaveVTKBBGroup)
    this->SaveVTKBBGroup->UpdateObjectLists();
  if(this->DeleteObjectGroup)
    this->DeleteObjectGroup->UpdateObjectLists();
}

//------------------------------------------------------------------------------------------
int vtkKWMimxBBMenuGroup::CheckCellTypesInUgrid(vtkUnstructuredGrid *Input)
{
  vtkCellTypes *celltypes = vtkCellTypes::New();
  Input->GetCellTypes(celltypes);
  int i;
  for(i =0; i < celltypes->GetNumberOfTypes(); i++)
    {
    if(celltypes->GetCellType(i) != 12)
      {
      celltypes->Delete();
      return 0;
      }
    }
  celltypes->Delete();
  return 1;
}
//------------------------------------------------------------------------------------------
