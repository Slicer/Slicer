/*=========================================================================

  Module:    $RCSfile: vtkKWMimxEditBBMeshSeedGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxEditBBMeshSeedGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkMimxPlaceLocalAxesWidget.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxSplitUnstructuredHexahedronGridCell.h"
#include "vtkMimxAddUnstructuredHexahedronGridCell.h"
#include "vtkMimxDeleteUnstructuredHexahedronGridCell.h"
#include "vtkIdList.h"

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
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButton.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxEditBBMeshSeedGroup);
vtkCxxRevisionMacro(vtkKWMimxEditBBMeshSeedGroup, "$Revision: 1.2 $");

//----------------------------------------------------------------------------
vtkKWMimxEditBBMeshSeedGroup::vtkKWMimxEditBBMeshSeedGroup()
{
  //this->MainFrame = NULL;
//  this->BBoxList = vtkLinkedListWrapper::New();
  this->MimxViewWindow = NULL;
  this->ObjectListComboBox = NULL;
  this->VtkInteractionButton = vtkKWRadioButton::New();
  this->LocalAxesWidget = NULL;
  this->EditButton = NULL;
  this->XMeshSeed = NULL;
  this->YMeshSeed = NULL;
  this->ZMeshSeed = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxEditBBMeshSeedGroup::~vtkKWMimxEditBBMeshSeedGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
  if(this->LocalAxesWidget)
    this->LocalAxesWidget->Delete();
  if(this->EditButton)
    this->EditButton->Delete();
  if(XMeshSeed)
  {
    this->XMeshSeed->Delete();
    this->YMeshSeed->Delete();
    this->ZMeshSeed->Delete();
  }
}
//--------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::SelectionChangedCallback(const char* dummy)
{
  if(this->EditButton->GetSelectedState())
  {
    this->EditButton->SetSelectedState(1);
    this->EditBBMeshSeedPickCellCallback();
  }
}
//--------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::CreateWidget()
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
  this->MainFrame->SetLabelText("Edit BB");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  this->ObjectListComboBox->SetParent(this->MainFrame->GetFrame());
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  this->ObjectListComboBox->SetLabelText("BB Selection : ");
  this->ObjectListComboBox->GetWidget()->ReadOnlyOn();

  int i;
  for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
    vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
      SafeDownCast(this->BBoxList->GetItem(i));
    vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
    if(ugrid->GetCellData()->GetArray("Mesh_Seed"))
    {
      ObjectListComboBox->GetWidget()->AddValue(
        this->BBoxList->GetItem(i)->GetFileName());
    }
   }
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6", 
    this->ObjectListComboBox->GetWidgetName());

  if(!this->EditButton)
    this->EditButton = vtkKWRadioButton::New();
  this->EditButton->SetParent(this->MainFrame->GetFrame());
  this->EditButton->Create();
  this->EditButton->SetCommand(this, "EditBBMeshSeedPickCellCallback");
  this->EditButton->SetText("Edit");
  this->EditButton->SetImageToPredefinedIcon(vtkKWIcon::IconBoundingBox);
  this->EditButton->IndicatorVisibilityOff();
  this->EditButton->SetBalloonHelpString("Pick cell whose mesh seed needs to be modified");
  this->EditButton->SetValue("Edit");
  this->EditButton->SetCompoundModeToLeft();

  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand 0 -padx 6 -pady 6", 
    this->EditButton->GetWidgetName());

  this->VtkInteractionButton->SetParent(this->MainFrame->GetFrame());
  this->VtkInteractionButton->Create();
  this->VtkInteractionButton->SetText("VTK");
  this->VtkInteractionButton->SetCommand(this, "EditBBVtkInteractionCallback");
  this->VtkInteractionButton->SetImageToPredefinedIcon(vtkKWIcon::IconRotate);
  this->VtkInteractionButton->IndicatorVisibilityOff();
  this->VtkInteractionButton->SetBalloonHelpString("Default VTK Interaction");
  this->VtkInteractionButton->SetVariableName(this->EditButton->GetVariableName());
  this->VtkInteractionButton->SetValue("VtkInteraction");
  this->VtkInteractionButton->SetCompoundModeToLeft();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 1 -padx 6 -pady 6", 
    this->VtkInteractionButton->GetWidgetName());
  this->VtkInteractionButton->SelectedStateOn();

  if(!this->XMeshSeed)
  {
    this->XMeshSeed = vtkKWEntryWithLabel::New();
    this->YMeshSeed = vtkKWEntryWithLabel::New();
    this->ZMeshSeed = vtkKWEntryWithLabel::New();
  }
  this->XMeshSeed->SetParent(this->MainFrame->GetFrame());
  this->YMeshSeed->SetParent(this->MainFrame->GetFrame());
  this->ZMeshSeed->SetParent(this->MainFrame->GetFrame());
  
  this->XMeshSeed->Create();
  this->YMeshSeed->Create();
  this->ZMeshSeed->Create();

  this->XMeshSeed->SetLabelText("X : ");
  this->YMeshSeed->SetLabelText("Y : ");
  this->ZMeshSeed->SetLabelText("Z : ");
  this->XMeshSeed->GetWidget()->SetRestrictValueToInteger();
  this->YMeshSeed->GetWidget()->SetRestrictValueToInteger();
  this->ZMeshSeed->GetWidget()->SetRestrictValueToInteger();
  this->XMeshSeed->SetEnabled(0);
  this->YMeshSeed->SetEnabled(0);
  this->ZMeshSeed->SetEnabled(0);
  
  this->XMeshSeed->GetWidget()->SetWidth(5);
  this->YMeshSeed->GetWidget()->SetWidth(5);
  this->ZMeshSeed->GetWidget()->SetWidth(5);

  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 2 -pady 6", 
    this->XMeshSeed->GetWidgetName());

  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 2 -pady 6", 
    this->YMeshSeed->GetWidgetName());

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
    this->ZMeshSeed->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "EditBBMeshSeedDoneCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 6 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "EditBBMeshSeedCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::EditBBMeshSeedDoneCallback()
{
  if(this->EditButton->GetSelectedState())
  {
    if(this->LocalAxesWidget)
    {
      if(this->LocalAxesWidget->GetEnabled())
      {
        if(this->LocalAxesWidget->GetPickedCell() > -1)
        {
          if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
          {
            vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
            const char *name = combobox->GetValue();
            int num = combobox->GetValueIndex(name);
            int x, y, z;
            x = this->XMeshSeed->GetWidget()->GetValueAsInt();
            y = this->YMeshSeed->GetWidget()->GetValueAsInt();
            z = this->ZMeshSeed->GetWidget()->GetValueAsInt();
            if(x > 0 && y > 0 && z > 0)
            {
              vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
                SafeDownCast(this->BBoxList->GetItem(combobox->GetValueIndex(name)));
              ugridactor->ChangeMeshSeed(this->LocalAxesWidget->GetPickedCell(), 0, x);
              ugridactor->ChangeMeshSeed(this->LocalAxesWidget->GetPickedCell(), 1, y);  
              ugridactor->ChangeMeshSeed(this->LocalAxesWidget->GetPickedCell(), 2, z);
            }
          }
        }
      }
    }
    this->VtkInteractionButton->SelectedStateOn();
    return;  
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::EditBBMeshSeedCancelCallback()
{
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->EditBBMeshSeedVtkInteractionCallback();
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::EditBBMeshSeedPickCellCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
    int num = combobox->GetValueIndex(name);
    vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
      ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
    if(this->LocalAxesWidget)
    {
      if(this->LocalAxesWidget->GetEnabled())
      {
        this->LocalAxesWidget->SetEnabled(0);
      }
      this->LocalAxesWidget->Delete();
      this->LocalAxesWidget = NULL;
    }
    this->LocalAxesWidget = vtkMimxPlaceLocalAxesWidget::New();
    this->LocalAxesWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
      ->GetRenderWindowInteractor());
    this->LocalAxesWidget->SetInput(ugrid);
    this->LocalAxesWidget->SetInputActor(this->BBoxList
      ->GetItem(combobox->GetValueIndex(name))->GetActor());
    this->LocalAxesWidget->SetXMeshSeed(this->XMeshSeed);
    this->LocalAxesWidget->SetYMeshSeed(this->YMeshSeed);
    this->LocalAxesWidget->SetZMeshSeed(this->ZMeshSeed);
    this->LocalAxesWidget->SetEnabled(1);
  }
}
//----------------------------------------------------------------------------------------
void vtkKWMimxEditBBMeshSeedGroup::EditBBMeshSeedVtkInteractionCallback()
{
  if(this->LocalAxesWidget)
  {
    if(this->LocalAxesWidget->GetEnabled())
    {
      this->LocalAxesWidget->SetEnabled(0);
    }
  }
  this->XMeshSeed->SetEnabled(0);
  this->YMeshSeed->SetEnabled(0);
  this->ZMeshSeed->SetEnabled(0);
}
//-----------------------------------------------------------------------------------------
