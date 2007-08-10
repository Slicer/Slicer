/*=========================================================================

  Module:    $RCSfile: vtkKWMimxCreateBBMeshSeedGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxCreateBBMeshSeedGroup.h"
#include "vtkKWMimxViewWindow.h"

#include "vtkMimxUnstructuredGridWidget.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
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
#include "vtkLinkedListWrapper.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWEntryWithLabel.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateBBMeshSeedGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateBBMeshSeedGroup, "$Revision: 1.5 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateBBMeshSeedGroup::vtkKWMimxCreateBBMeshSeedGroup()
{
  //this->MainFrame = NULL;
//  this->BBoxList = vtkLinkedListWrapper::New();
  this->MimxViewWindow = NULL;
  this->AvElementLength = vtkKWEntryWithLabel::New();
  this->ObjectListComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateBBMeshSeedGroup::~vtkKWMimxCreateBBMeshSeedGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
//  this->BBoxList->Delete();
  this->AvElementLength->Delete();
  if(this->ObjectListComboBox)
    this->ObjectListComboBox->Delete();
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();

  if(!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->MainFrame->SetLabelText("Create BB Mesh Seeds");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame->GetFrame());
  ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  ObjectListComboBox->SetLabelText("BB Selection : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  int i;
  for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
    ObjectListComboBox->GetWidget()->AddValue(
      this->BBoxList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    ObjectListComboBox->GetWidgetName());

  this->AvElementLength->SetParent(this->MainFrame->GetFrame());
  this->AvElementLength->Create();
  this->AvElementLength->SetWidth(5);
  this->AvElementLength->SetLabelText("Average Element Length ");
  this->AvElementLength->GetWidget()->SetValueAsDouble(1.0);
  this->AvElementLength->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    this->AvElementLength->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
//  this->DoneButton->SetWidth(50);
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "CreateBBMeshSeedCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 20 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateBBMeshSeedCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::CreateBBMeshSeedCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
    vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
      SafeDownCast(this->BBoxList->GetItem(combobox->GetValueIndex(name)));
    vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
    if(!ugrid->GetCellData()->GetArray("Mesh_Seed"))
    {
      ugridactor->MeshSeedFromAverageElementLength(
        this->AvElementLength->GetWidget()->GetValueAsDouble());
    }
  }
  this->CreateBBMeshSeedCancelCallback();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::CreateBBMeshSeedCancelCallback()
{
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
