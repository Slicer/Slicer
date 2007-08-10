/*=========================================================================

  Module:    $RCSfile: vtkKWMimxDeleteObjectGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxDeleteObjectGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxMainMenuGroup.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDeleteObjectGroup);
vtkCxxRevisionMacro(vtkKWMimxDeleteObjectGroup, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMimxDeleteObjectGroup::vtkKWMimxDeleteObjectGroup()
{
  this->ObjectListComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxDeleteObjectGroup::~vtkKWMimxDeleteObjectGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  if(!this->ObjectListComboBox)  
  {
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  }
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->MainFrame->SetLabelText("Delete Objects");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Object Selection : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  this->UpdateObjectLists();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    ObjectListComboBox->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "DeleteObjectCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 20 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "DeleteObjectCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::DeleteObjectCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
  int num = combobox->GetValueIndex(name);
  if(this->SurfaceList)
  {
    vtkMimxActorBase *actorbase = this->SurfaceList->GetItem(
      combobox->GetValueIndex(name));
    vtkMimxSurfacePolyDataActor *polydataactor = vtkMimxSurfacePolyDataActor::
      SafeDownCast(this->SurfaceList->GetItem(combobox->GetValueIndex(name)));
    if(polydataactor)
    {
      this->ViewProperties->DeleteObjectList(num);
      polydataactor->Delete();
      this->SurfaceList->RemoveItem(num);
      this->UpdateObjectLists();
      this->ObjectListComboBox->GetWidget()->SetValue("");
      return;
    }
  }
  if(this->BBoxList)
  {
    vtkMimxActorBase *actorbase = this->BBoxList->GetItem(
      combobox->GetValueIndex(name));
    vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
      SafeDownCast(this->BBoxList->GetItem(combobox->GetValueIndex(name)));
    if(ugridactor)
    {
      this->ViewProperties->DeleteObjectList(num);
      ugridactor->Delete();
      this->BBoxList->RemoveItem(num);
      this->UpdateObjectLists();
      this->ObjectListComboBox->GetWidget()->SetValue("");
      return;
    }
  }
  if(this->FEMeshList)
  {
    vtkMimxActorBase *actorbase = this->FEMeshList->GetItem(
      combobox->GetValueIndex(name));
    vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
      SafeDownCast(this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
    if(ugridactor)
    {
      this->ViewProperties->DeleteObjectList(num);
      ugridactor->Delete();
      this->FEMeshList->RemoveItem(num);
      this->UpdateObjectLists();
      this->ObjectListComboBox->GetWidget()->SetValue("");
      return;
    }
  }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::DeleteObjectCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxDeleteObjectGroup::UpdateObjectLists()
{
  this->ObjectListComboBox->GetWidget()->DeleteAllValues();
  int i;
  if(this->SurfaceList)
  {
    for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
    {
    ObjectListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
    }
    return;
  }
  if(this->BBoxList)
  {
    for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
    {
      ObjectListComboBox->GetWidget()->AddValue(
        this->BBoxList->GetItem(i)->GetFileName());
    }
    return;
  }
  if(this->FEMeshList)
  {
    for (i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
    {
      ObjectListComboBox->GetWidget()->AddValue(
        this->FEMeshList->GetItem(i)->GetFileName());
    }
    return;
  }
}
//------------------------------------------------------------------------------
