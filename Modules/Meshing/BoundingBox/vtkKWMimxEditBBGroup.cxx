/*=========================================================================

  Module:    $RCSfile: vtkKWMimxEditBBGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxEditBBGroup.h"
#include "vtkKWMimxViewWindow.h"

#include "vtkMimxUnstructuredGridWidget.h"
#include "vtkMimxExtractEdgeWidget.h"
#include "vtkMimxExtractFaceWidget.h"
#include "vtkMimxExtractCellWidget.h"

#include "vtkActor.h"
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
vtkStandardNewMacro(vtkKWMimxEditBBGroup);
vtkCxxRevisionMacro(vtkKWMimxEditBBGroup, "$Revision: 1.8 $");

//----------------------------------------------------------------------------
vtkKWMimxEditBBGroup::vtkKWMimxEditBBGroup()
{
  //this->MainFrame = NULL;
//  this->BBoxList = vtkLinkedListWrapper::New();
  this->MimxViewWindow = NULL;
  this->ObjectListComboBox = NULL;
  this->UnstructuredGridWidget = NULL;
  this->ExtractEdgeWidget = NULL;
  this->ExtractFaceWidget = NULL;
  this->ExtractCellWidget = NULL;
  this->MoveButton = vtkKWRadioButton::New();
  this->AddButton = vtkKWRadioButton::New();
  this->DeleteButton = vtkKWRadioButton::New();
  this->SplitButton = vtkKWRadioButton::New();
  this->VtkInteractionButton = vtkKWRadioButton::New();
  this->AddButtonState = 0;
  this->MoveButtonState = 0;
//  this->SelectionState = 0;
  this->SplitButtonState = 0;
  this->RegularButtonState = 1;
  this->SplitCount = 0;
  this->AddCount = 0;
  this->DeleteCount = 0;
  this->DeleteButtonState = 0;
}

//----------------------------------------------------------------------------
vtkKWMimxEditBBGroup::~vtkKWMimxEditBBGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
//  this->BBoxList->Delete();
  this->AddButton->Delete();
  this->DeleteButton->Delete();
  this->SplitButton->Delete();
  this->MoveButton->Delete();
  this->VtkInteractionButton->Delete();
  if(this->UnstructuredGridWidget)
    this->UnstructuredGridWidget->Delete();
  if(this->ExtractEdgeWidget)
    this->ExtractEdgeWidget->Delete();
  if(this->ExtractFaceWidget)
    this->ExtractFaceWidget->Delete();
  if(this->ExtractCellWidget)
    this->ExtractCellWidget->Delete();
}
//--------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::SelectionChangedCallback(const char* dummy)
{
  this->AddButtonState = 1;
  this->MoveButtonState = 1;
  this->SplitButtonState = 1;
  this->DeleteButtonState = 1;
  if(this->MoveButton->GetSelectedState())
  {
    this->MoveButton->SetSelectedState(1);
    this->EditBBMoveCellCallback();
  }
  if(this->SplitButton->GetSelectedState())
  {
    this->SplitButton->SetSelectedState(1);
    this->EditBBSplitCellCallback();
  }
  if(this->AddButton->GetSelectedState())
  {
    this->AddButton->SetSelectedState(1);
    this->EditBBAddCellCallback();
  }
  if(this->DeleteButton->GetSelectedState())
  {
    this->DeleteButton->SetSelectedState(1);
    this->EditBBDeleteCellCallback();
  }

}
//--------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::CreateWidget()
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
    ObjectListComboBox->GetWidget()->AddValue(
      this->BBoxList->GetItem(i)->GetFileName());
  }
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6", 
    this->ObjectListComboBox->GetWidgetName());

 
  this->AddButton->SetParent(this->MainFrame->GetFrame());
  this->AddButton->Create();
  this->AddButton->SetCommand(this, "EditBBAddCellCallback");
  this->AddButton->SetText("Add");
  this->AddButton->SetImageToPredefinedIcon(vtkKWIcon::IconPlus);
  this->AddButton->IndicatorVisibilityOff();
  this->AddButton->SetBalloonHelpString("Add cell to bounding box by picking a face");
  this->AddButton->SetValue("Add");
  this->AddButton->SetCompoundModeToLeft();

  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand 0 -padx 6 -pady 6", 
    this->AddButton->GetWidgetName());

  this->DeleteButton->SetParent(this->MainFrame->GetFrame());
  this->DeleteButton->Create();
  this->DeleteButton->SetCommand(this, "EditBBDeleteCellCallback");
  this->DeleteButton->SetText("Delete");
  this->DeleteButton->SetImageToPredefinedIcon(vtkKWIcon::IconMinus);
  this->DeleteButton->IndicatorVisibilityOff();
  this->DeleteButton->SetBalloonHelpString("Delete cell from a bounding box by picking");
  this->DeleteButton->SetVariableName(this->AddButton->GetVariableName());
  this->DeleteButton->SetValue("Delete");
  this->DeleteButton->SetCompoundModeToLeft();
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand 0 -padx 6 -pady 6", 
    this->DeleteButton->GetWidgetName());

  this->SplitButton->SetParent(this->MainFrame->GetFrame());
  this->SplitButton->Create();
  this->SplitButton->SetText("Split");
  this->SplitButton->SetCommand(this, "EditBBSplitCellCallback");
  this->SplitButton->SetImageToPredefinedIcon(vtkKWIcon::IconCropTool);
  this->SplitButton->IndicatorVisibilityOff();
  this->SplitButton->SetBalloonHelpString("Split cells of a bounding box");
  this->SplitButton->SetVariableName(this->AddButton->GetVariableName());
  this->SplitButton->SetValue("Split");
  this->SplitButton->SetCompoundModeToLeft();
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand 1 -padx 6 -pady 6", 
    this->SplitButton->GetWidgetName());

  this->MoveButton->SetParent(this->MainFrame->GetFrame());
  this->MoveButton->Create();
  this->MoveButton->SetText("Move");
  this->MoveButton->SetCommand(this, "EditBBMoveCellCallback");
  this->MoveButton->SetImageToPredefinedIcon(vtkKWIcon::IconPanHand);
  this->MoveButton->IndicatorVisibilityOff();
  this->MoveButton->SetBalloonHelpString("Move the vertex of the bounding box");
  this->MoveButton->SetVariableName(this->AddButton->GetVariableName());
  this->MoveButton->SetValue("Move");
  this->MoveButton->SetCompoundModeToLeft();
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand 1 -padx 6 -pady 6", 
    this->MoveButton->GetWidgetName());

  this->VtkInteractionButton->SetParent(this->MainFrame->GetFrame());
  this->VtkInteractionButton->Create();
  this->VtkInteractionButton->SetText("VTK");
  this->VtkInteractionButton->SetCommand(this, "EditBBVtkInteractionCallback");
  this->VtkInteractionButton->SetImageToPredefinedIcon(vtkKWIcon::IconRotate);
  this->VtkInteractionButton->IndicatorVisibilityOff();
  this->VtkInteractionButton->SetBalloonHelpString("Default VTK Interaction");
  this->VtkInteractionButton->SetVariableName(this->AddButton->GetVariableName());
  this->VtkInteractionButton->SetValue("VtkInteraction");
  this->VtkInteractionButton->SetCompoundModeToLeft();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 1 -padx 6 -pady 6", 
    this->VtkInteractionButton->GetWidgetName());
  this->VtkInteractionButton->SelectedStateOn();

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "EditBBDoneCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 6 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "EditBBCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand n -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::EditBBDoneCallback()
{
  if(this->SplitButton->GetSelectedState())
  {
    if(this->ExtractEdgeWidget)
    {
      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
       vtkMimxSplitUnstructuredHexahedronGridCell *split = 
        vtkMimxSplitUnstructuredHexahedronGridCell::New();
       split->SetInput(ugrid);
       vtkIdType *edgepoints = this->ExtractEdgeWidget->GetEdgePoints();
       vtkIdList *idlist = vtkIdList::New();
       idlist->SetNumberOfIds(2);
       idlist->SetId(0, edgepoints[0]);
       idlist->SetId(1, edgepoints[1]);
       split->SetIdList(idlist);
      split->Update();
      vtkIdType objnum = combobox->GetValueIndex(name);
      if (split->GetOutput())
      {
        this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->
          DeepCopy(split->GetOutput());
        this->SplitCount++;
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->SetObjectName("Split_",SplitCount);
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
        this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
        this->GetMimxViewWindow()->GetRenderWidget()->RemoveViewProp(
          this->BBoxList->GetItem(objnum)->GetActor());
        this->ObjectListComboBox->GetWidget()->SetValue(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetFileName());
        this->ObjectListComboBox->GetWidget()->AddValue(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetFileName());
         this->GetMimxViewWindow()->GetRenderWidget()->Render();
        this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
    this->ViewProperties->AddObjectList();
      }
      idlist->Delete();
      split->Delete();
    }
  this->VtkInteractionButton->SelectedStateOn();
  return;  
  }
  //
  if(this->AddButton->GetSelectedState())
  {
    if(this->ExtractFaceWidget)
    {
      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
      vtkMimxAddUnstructuredHexahedronGridCell *add = 
        vtkMimxAddUnstructuredHexahedronGridCell::New();
      add->SetInput(ugrid);
      vtkIdType *facepoints = this->ExtractFaceWidget->GetFacePoints();
      vtkIdList *idlist = vtkIdList::New();
      idlist->SetNumberOfIds(4);
      int i;
      for (i=0; i<4; i++)
      {
        idlist->SetId(i, facepoints[i]);
      }
      add->SetIdList(idlist);
      add->Update();
      vtkIdType objnum = combobox->GetValueIndex(name);
      if (add->GetOutput())
      {
        this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->
          DeepCopy(add->GetOutput());
        this->AddCount++;
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->SetObjectName("Add_",AddCount);
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
        this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
        this->GetMimxViewWindow()->GetRenderWidget()->RemoveViewProp(
          this->BBoxList->GetItem(objnum)->GetActor());
        this->ObjectListComboBox->GetWidget()->SetValue(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetFileName());
        this->ObjectListComboBox->GetWidget()->AddValue(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetFileName());
        this->GetMimxViewWindow()->GetRenderWidget()->Render();
        this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
        this->ViewProperties->AddObjectList();
      }
      idlist->Delete();
      add->Delete();
    }
    this->VtkInteractionButton->SelectedStateOn();
    return;  
  }

  if(this->DeleteButton->GetSelectedState())
  {
    if(this->ExtractCellWidget)
    {
      if(this->ExtractCellWidget->GetPickedCells()->GetNumberOfIds() > 0)
      {

      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
      vtkMimxDeleteUnstructuredHexahedronGridCell *del = 
        vtkMimxDeleteUnstructuredHexahedronGridCell::New();
      del->SetInput(ugrid);
      del->SetCellNum(this->ExtractCellWidget->GetPickedCells()->GetId(0));
      del->Update();
      vtkIdType objnum = combobox->GetValueIndex(name);
      if (del->GetOutput())
      {
        this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->
          DeepCopy(del->GetOutput());
        this->DeleteCount++;
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->SetObjectName("Delete_",DeleteCount);
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
        this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
        this->GetMimxViewWindow()->GetRenderWidget()->RemoveViewProp(
          this->BBoxList->GetItem(objnum)->GetActor());
        this->ObjectListComboBox->GetWidget()->SetValue(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetFileName());
        this->ObjectListComboBox->GetWidget()->AddValue(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetFileName());
        this->GetMimxViewWindow()->GetRenderWidget()->Render();
        this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
        this->ViewProperties->AddObjectList();
      }
      del->Delete();
      }
    }
    this->VtkInteractionButton->SelectedStateOn();
    return;  
  }

}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::EditBBCancelCallback()
{
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->EditBBVtkInteractionCallback();
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::EditBBDeleteCellCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    if(this->DeleteButtonState)
    {
      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      int num = combobox->GetValueIndex(name);
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
        ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
      if(this->ExtractCellWidget)
      {
        if(this->ExtractCellWidget->GetEnabled())
        {
          this->ExtractCellWidget->SetEnabled(0);
        }
        this->ExtractCellWidget->Delete();
        this->ExtractCellWidget = NULL;
      }
      this->ExtractCellWidget = vtkMimxExtractCellWidget::New();
      this->ExtractCellWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
        ->GetRenderWindowInteractor());
      this->ExtractCellWidget->SetInput(ugrid);
      this->ExtractCellWidget->SetInputActor(this->BBoxList
        ->GetItem(combobox->GetValueIndex(name))->GetActor());
      this->ExtractCellWidget->SetEnabled(1);
      this->DeleteButtonState = 0;
    }
    else
    {
      if(!this->ExtractCellWidget)
      {
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        this->ExtractCellWidget = vtkMimxExtractCellWidget::New();
        this->ExtractCellWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
          ->GetRenderWindowInteractor());
        this->ExtractCellWidget->SetInput(ugrid);
        this->ExtractCellWidget->SetInputActor(this->BBoxList
          ->GetItem(combobox->GetValueIndex(name))->GetActor());
        this->ExtractCellWidget->SetEnabled(1);
        this->DeleteButtonState = 0;
      }
      else
      {
        if(!this->ExtractCellWidget->GetEnabled())
        {
          this->ExtractCellWidget->SetEnabled(1);
        }
      }
    }
  }
  if(UnstructuredGridWidget)
  {
    if(UnstructuredGridWidget->GetEnabled())
    {
      UnstructuredGridWidget->SetEnabled(0);
      return;
    }
  }
  if(this->ExtractEdgeWidget)
  {
    if(this->ExtractEdgeWidget->GetEnabled())
    {
      this->ExtractEdgeWidget->SetEnabled(0);
      return;
    }
  }
  if(this->ExtractFaceWidget)
  {
    if(this->ExtractFaceWidget->GetEnabled())
    {
      this->ExtractFaceWidget->SetEnabled(0);
      return;
    }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::EditBBSplitCellCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    if(this->SplitButtonState)
    {
      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      int num = combobox->GetValueIndex(name);
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
        ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
      if(this->ExtractEdgeWidget)
      {
        if(this->ExtractEdgeWidget->GetEnabled())
        {
          this->ExtractEdgeWidget->SetEnabled(0);
        }
        this->ExtractEdgeWidget->Delete();
        this->ExtractEdgeWidget = NULL;
      }
      this->ExtractEdgeWidget = vtkMimxExtractEdgeWidget::New();
      this->ExtractEdgeWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
        ->GetRenderWindowInteractor());
      this->ExtractEdgeWidget->SetInput(ugrid);
    this->ExtractEdgeWidget->SetInputActor(this->BBoxList
      ->GetItem(combobox->GetValueIndex(name))->GetActor());
      this->ExtractEdgeWidget->SetEnabled(1);
      this->SplitButtonState = 0;
    }
    else
    {
      if(!this->ExtractEdgeWidget)
      {
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        this->ExtractEdgeWidget = vtkMimxExtractEdgeWidget::New();
        this->ExtractEdgeWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
          ->GetRenderWindowInteractor());
        this->ExtractEdgeWidget->SetInput(ugrid);
    this->ExtractEdgeWidget->SetInputActor(this->BBoxList
      ->GetItem(combobox->GetValueIndex(name))->GetActor());
        this->ExtractEdgeWidget->SetEnabled(1);
        this->SplitButtonState = 0;
      }
      else
      {
        if(!this->ExtractEdgeWidget->GetEnabled())
        {
          this->ExtractEdgeWidget->SetEnabled(1);
        }
      }
    }
  }
  if(this->ExtractFaceWidget)
  {
    if(this->ExtractFaceWidget->GetEnabled())
    {
      this->ExtractFaceWidget->SetEnabled(0);
      return;
    }
  }
  if(UnstructuredGridWidget)
  {
    if(UnstructuredGridWidget->GetEnabled())
    {
      UnstructuredGridWidget->SetEnabled(0);
      return;
    }
  }
  if(this->ExtractCellWidget)
  {
    if(this->ExtractCellWidget->GetEnabled())
    {
      this->ExtractCellWidget->SetEnabled(0);
      return;
    }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::EditBBAddCellCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
    if(this->AddButtonState)
    {
      vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      int num = combobox->GetValueIndex(name);
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
        ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
      if(this->ExtractFaceWidget)
      {
        if(this->ExtractFaceWidget->GetEnabled())
        {
          this->ExtractFaceWidget->SetEnabled(0);
        }
        this->ExtractFaceWidget->Delete();
        this->ExtractFaceWidget = NULL;
      }
      this->ExtractFaceWidget = vtkMimxExtractFaceWidget::New();
      this->ExtractFaceWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
        ->GetRenderWindowInteractor());
      this->ExtractFaceWidget->SetInput(ugrid);
    this->ExtractFaceWidget->SetInputActor(this->BBoxList
      ->GetItem(combobox->GetValueIndex(name))->GetActor());
      this->ExtractFaceWidget->SetEnabled(1);
      this->AddButtonState = 0;
    }
    else
    {
      if(!this->ExtractFaceWidget)
      {
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        this->ExtractFaceWidget = vtkMimxExtractFaceWidget::New();
        this->ExtractFaceWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
          ->GetRenderWindowInteractor());
        this->ExtractFaceWidget->SetInput(ugrid);
    this->ExtractFaceWidget->SetInputActor(this->BBoxList
      ->GetItem(combobox->GetValueIndex(name))->GetActor());
        this->ExtractFaceWidget->SetEnabled(1);
        this->AddButtonState = 0;
      }
      else
      {
        if(!this->ExtractFaceWidget->GetEnabled())
        {
          this->ExtractFaceWidget->SetEnabled(1);
        }
      }
    }
  }
  if(UnstructuredGridWidget)
  {
    if(UnstructuredGridWidget->GetEnabled())
    {
      UnstructuredGridWidget->SetEnabled(0);
      return;
    }
  }
  if(this->ExtractEdgeWidget)
  {
    if(this->ExtractEdgeWidget->GetEnabled())
    {
      this->ExtractEdgeWidget->SetEnabled(0);
      return;
    }
  }
  if(this->ExtractCellWidget)
  {
    if(this->ExtractCellWidget->GetEnabled())
    {
      this->ExtractCellWidget->SetEnabled(0);
      return;
    }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxEditBBGroup::EditBBMoveCellCallback()
{
    if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
      if(this->MoveButtonState)
      {
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
          ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        if(this->UnstructuredGridWidget)
        {
          if(this->UnstructuredGridWidget->GetEnabled())
          {
            this->UnstructuredGridWidget->SetEnabled(0);
          }
          this->UnstructuredGridWidget->Delete();
          this->UnstructuredGridWidget = NULL;
        }
        this->UnstructuredGridWidget = vtkMimxUnstructuredGridWidget::New();
        this->UnstructuredGridWidget->SetUGrid(ugrid);
        this->UnstructuredGridWidget->SetInteractor(this->GetMimxViewWindow()->GetRenderWidget()
          ->GetRenderWindowInteractor());
        this->UnstructuredGridWidget->Execute();
        this->UnstructuredGridWidget->SetEnabled(1);
        this->MoveButtonState = 0;
      }
      else
      {
        if(!this->UnstructuredGridWidget)
        {
          vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
          const char *name = combobox->GetValue();
          int num = combobox->GetValueIndex(name);
          vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList
            ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
          this->UnstructuredGridWidget = vtkMimxUnstructuredGridWidget::New();
          this->UnstructuredGridWidget->SetUGrid(ugrid);
          this->UnstructuredGridWidget->SetInteractor(this->GetMimxViewWindow()->
            GetRenderWidget()->GetRenderWindowInteractor());
          this->UnstructuredGridWidget->Execute();
          this->UnstructuredGridWidget->SetEnabled(1);
          this->MoveButtonState = 0;
        }
        else
        {
          if(!this->UnstructuredGridWidget->GetEnabled())
          {
            this->UnstructuredGridWidget->SetEnabled(1);
          }
        }
      }
    }
    if(ExtractEdgeWidget)
    {
      if(ExtractEdgeWidget->GetEnabled())
      {
        ExtractEdgeWidget->SetEnabled(0);
        return;
      }
    }
    if(ExtractFaceWidget)
    {
      if(ExtractFaceWidget->GetEnabled())
      {
        ExtractFaceWidget->SetEnabled(0);
        return;
      }
    }
  if(this->ExtractCellWidget)
  {
    if(this->ExtractCellWidget->GetEnabled())
    {
      this->ExtractCellWidget->SetEnabled(0);
      return;
    }
  }
}

void vtkKWMimxEditBBGroup::EditBBVtkInteractionCallback()
{
  if(this->UnstructuredGridWidget)
  {
    if (this->UnstructuredGridWidget->GetEnabled())
    {
      this->UnstructuredGridWidget->SetEnabled(0);
    }
  }
  if(this->ExtractEdgeWidget)
  {
    if(this->ExtractEdgeWidget->GetEnabled())
    {
      this->ExtractEdgeWidget->SetEnabled(0);
    }
  }
  if(this->ExtractFaceWidget)
  {
    if(this->ExtractFaceWidget->GetEnabled())
    {
      this->ExtractFaceWidget->SetEnabled(0);
    }
  }
  if(this->ExtractCellWidget)
  {
    if(this->ExtractCellWidget->GetEnabled())
    {
      this->ExtractCellWidget->SetEnabled(0);
      return;
    }
  }
}
