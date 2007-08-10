/*=========================================================================

  Module:    $RCSfile: vtkKWMimxCreateBBFromBoundsGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxCreateBBFromBoundsGroup.h"
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
vtkStandardNewMacro(vtkKWMimxCreateBBFromBoundsGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateBBFromBoundsGroup, "$Revision: 1.5 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateBBFromBoundsGroup::vtkKWMimxCreateBBFromBoundsGroup()
{
  this->ObjectListComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateBBFromBoundsGroup::~vtkKWMimxCreateBBFromBoundsGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::CreateWidget()
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
  this->MainFrame->SetLabelText("Create BB From Bounds");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Object Selection : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  int i;
  for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    ObjectListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    ObjectListComboBox->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "CreateBBFromBoundsCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 20 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateBBFromBoundsCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::CreateBBFromBoundsCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
    vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
    vtkMimxBoundingBoxSource *bbox = vtkMimxBoundingBoxSource::New();
    bbox->SetSource(polydata);
    bbox->Update();
    if (bbox->GetOutput())
    {
      this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
      vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
        this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->DeepCopy(bbox->GetOutput());
      this->Count++;
      vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
        this->BBoxList->GetNumberOfItems()-1))->SetObjectName("BBFromBounds_",Count);
      vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
        this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
      this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
        this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
      this->GetMimxViewWindow()->GetRenderWidget()->Render();
      this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
    this->ViewProperties->AddObjectList();
    this->CreateBBFromBoundsCancelCallback();
    }
    bbox->Delete();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::CreateBBFromBoundsCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::UpdateObjectLists()
{
  this->ObjectListComboBox->GetWidget()->DeleteAllValues();
  int i;
  for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    ObjectListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
  }

}
//------------------------------------------------------------------------------
