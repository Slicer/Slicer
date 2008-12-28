/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMergeBBGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.26.4.2 $

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


#include "vtkKWMimxMergeBBGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWIcon.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWCheckButton.h"
#include "vtkPointLocator.h"
#include "vtkIntArray.h"
#include "vtkPointData.h"
#include "vtkProperty.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButtonSet.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkAppendFilter.h"
#include "vtkCellData.h"

#include "vtkUnstructuredGridWriter.h"

#include "vtkMergeCells.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMergeBBGroup);
vtkCxxRevisionMacro(vtkKWMimxMergeBBGroup, "$Revision: 1.26.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxMergeBBGroup::vtkKWMimxMergeBBGroup()
{
        this->MultiColumnList = NULL;
        this->MergeTolerence = vtkKWEntryWithLabel::New();
        //this->EditBBGroup = NULL;
        this->VTKRadioButton = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMergeBBGroup::~vtkKWMimxMergeBBGroup()
{
        if(this->MultiColumnList)
                this->MultiColumnList->Delete();
        this->MergeTolerence->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::CreateWidget()
{
  if(this->IsCreated())
  {
  vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Merge BB/ Vertices");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());
  if(!this->MultiColumnList)
  {
          this->MultiColumnList = vtkKWMultiColumnListWithScrollbars::New();
  }
  this->MultiColumnList->SetParent(this->MainFrame);

  this->MultiColumnList->Create();
  this->MultiColumnList->SetHorizontalScrollbarVisibility(1);
  this->MultiColumnList->SetVerticalScrollbarVisibility(1);
  this->MultiColumnList->SetHeight(5);
  // this->MultiColumnList->SetWidth(10);
  //  this->MultiColumnList->GetWidget()->GetMovableColumnsOn();
  //  this->MultiColumnList->SetPotentialCellColorsChangedCommand(
  //    this->MultiColumnList, "ScheduleRefreshColorsOfAllCellsWithWindowCommand");
  int col_index;

  // Add the columns 
  col_index = MultiColumnList->GetWidget()->AddColumn("Name");

  col_index = MultiColumnList->GetWidget()->AddColumn(NULL);
  MultiColumnList->GetWidget()->SetColumnFormatCommandToEmptyOutput(col_index);
  MultiColumnList->GetWidget()->SetColumnLabelImageToPredefinedIcon(
          col_index, vtkKWIcon::IconPlus);
  //this->MultiColumnList->InsertCellText(0, 0, "");
  // this->MultiColumnList->InsertCellTextAsInt(0, 1, 0);
  // this->MultiColumnList->SetCellWindowCommandToCheckButton(0, 1);
  // this->MultiColumnList->GetCellWindowAsCheckButton(0,1)->SetEnabled(0);
  this->MultiColumnList->GetWidget()->SetSortArrowVisibility(0);
  this->MultiColumnList->GetWidget()->ColumnSeparatorsVisibilityOff();
  this->MultiColumnList->GetWidget()->SetHeight(5);
  //  this->MultiColumnList->SetSelectioModeToSingle();
  //  this->MultiColumnList->RowSeparatorsVisibilityOff();
    this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
          this->MultiColumnList->GetWidgetName());

  if(!this->MergeTolerence)
          this->MergeTolerence = vtkKWEntryWithLabel::New();

  this->MergeTolerence->SetParent(this->MainFrame);
  this->MergeTolerence->Create();
  this->MergeTolerence->SetWidth(4);
  this->MergeTolerence->SetLabelText("Merge Tolerance : ");
  this->MergeTolerence->GetWidget()->SetRestrictValueToDouble();
  this->MergeTolerence->GetWidget()->SetWidth(5);
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->MergeTolerence->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "MergeBBApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "MergeBBCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxMergeBBGroup::MergeBBApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        callback->SetState(0);

        int i, count_value = 0;
        vtkUnstructuredGrid *singleugrid = NULL;       
        if(MultiColumnList->GetWidget()->GetNumberOfRows() > 0)
        {
                int count = 0;
                for (i=0; i < this->MultiColumnList->GetWidget()->GetNumberOfRows(); i++)
                {
                        if(this->MultiColumnList->GetWidget()->
                                GetCellWindowAsCheckButton(i,1)->GetSelectedState())
                        {
                                count++;
                                if(count == 1)
                                {
                                        singleugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
                                                this->BBoxList->GetItem(i))->GetDataSet();
                                        count_value = i;
                                }
                        }
                }
                if(count < 1)
                {
                        callback->ErrorMessage("Minimum of 1 Building Block should be selected");
                        return 0;
                }
                
                // append the chosen items first
                vtkAppendFilter *AppendFilter = vtkAppendFilter::New();
                AppendFilter->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);

                for (i=0; i < this->MultiColumnList->GetWidget()->GetNumberOfRows(); i++)
                {
                        if(this->MultiColumnList->GetWidget()->
                                GetCellWindowAsCheckButton(i,1)->GetSelectedState())
                        {
                                AppendFilter->AddInput(vtkMimxUnstructuredGridActor::SafeDownCast(
                                        this->BBoxList->GetItem(i))->GetDataSet());
                                this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
                                        this->BBoxList->GetItem(i)->GetActor());
                        }
                }
                AppendFilter->Update();

                vtkUnstructuredGrid *Ugrid = vtkUnstructuredGrid::New();
                vtkMergeCells* mergecells = vtkMergeCells::New();
                mergecells->SetUnstructuredGrid(Ugrid);
                double tol = this->MergeTolerence->GetWidget()->GetValueAsDouble();
                mergecells->SetPointMergeTolerance(tol);
                mergecells->SetTotalNumberOfDataSets(1);
                mergecells->SetTotalNumberOfCells(AppendFilter->GetOutput()->GetNumberOfCells());
                mergecells->SetTotalNumberOfPoints(AppendFilter->GetOutput()->GetNumberOfPoints());
                mergecells->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                mergecells->MergeDataSet(AppendFilter->GetOutput());
                mergecells->Finish();
                mergecells->RemoveObserver(callback);
                AppendFilter->Delete();
                if(!callback->GetState())
                {
                        this->Count++;
                        this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
                        this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->
                                SetDataType(ACTOR_BUILDING_BLOCK);

                        vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
                                this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1))->GetDataSet();

                        ugrid->DeepCopy(Ugrid);
                        Ugrid->Delete();
                        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
                                this->BBoxList->GetNumberOfItems()-1))->SetObjectName("MergedBB_",Count);
                        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
                                this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
                        this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor()->
                                GetProperty()->SetRepresentation(2);
                        if(count == 1)
                        {
                                this->GetMimxMainWindow()->GetRenderWidget()->RemoveViewProp(
                                        this->BBoxList->GetItem(count_value)->GetActor());
                                this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor()->
                                        GetProperty()->SetRepresentation(
                                        this->BBoxList->GetItem(count_value)->GetActor()
                                        ->GetProperty()->GetRepresentation());
                        }
                        this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(
                                this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
                        this->GetMimxMainWindow()->GetRenderWidget()->Render();
                        this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
                        this->GetMimxMainWindow()->GetViewProperties()->AddObjectList(
                                this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1));
                        this->UpdateObjectLists();
                        //mergecells->Delete();
                        //if(this->EditBBGroup)
                        //      this->EditBBGroup->UpdateObjectLists();
                        if(this->VTKRadioButton)
                                this->VTKRadioButton->SetSelectedState(1);
                        if(count == 1)
                        {
                                this->CopyConstraintValues(singleugrid, Ugrid);
                        }
                                this->DoUndoTree->AppendItem(new Node);
                                int num = this->DoUndoTree->GetNumberOfItems()-1;
                                this->DoUndoTree->GetItem(num)->Data = 
                                        vtkMimxUnstructuredGridActor::SafeDownCast(
                                        this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1));
                                this->DoUndoTree->GetItem(num)->Parent = NULL;
                                this->DoUndoTree->GetItem(num)->Child = NULL;
                        //this->EditBBGroup->GetObjectListComboBox()->GetWidget()->SetValue(
                        //      this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)
                        //      ->GetFileName());
                        //this->EditBBGroup->GetDoUndoButtonSet()->GetWidget(0)->SetEnabled(0);
                        //this->EditBBGroup->GetDoUndoButtonSet()->GetWidget(1)->SetEnabled(0);
                        this->GetMimxMainWindow()->SetStatusText("Merged Building Block");
                        
                        return 1;
                }
                else{
                        Ugrid->Delete();
                        return 0;
                }
        }
        else
        {
                callback->ErrorMessage("Building Blocks not available for merging");
        }
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::MergeBBCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
//  this->MenuGroup->SetMenuButtonsEnabled(1);
 // if(this->EditBBGroup)
        //this->GetApplication()->Script(
        //       "pack %s -side bottom -anchor nw -expand y -padx 0 -pady 2", 
        //      this->EditBBGroup->GetMainFrame()->GetWidgetName());
 // elseg
            this->MenuGroup->SetMenuButtonsEnabled(1);
  if(this->VTKRadioButton)
          this->VTKRadioButton->SetSelectedState(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::UpdateObjectLists()
{
        this->MultiColumnList->GetWidget()->DeleteAllRows();
        //int localcount = 0;
        for (int i=0; i < this->BBoxList->GetNumberOfItems(); i++)
        {
                vtkMimxUnstructuredGridActor *actor = vtkMimxUnstructuredGridActor::
                        SafeDownCast(this->BBoxList->GetItem(i));
                //vtkUnstructuredGrid *ugrid = actor->GetDataSet();
                this->MultiColumnList->GetWidget()->InsertCellText(i, 0, actor->GetFileName());
                this->MultiColumnList->GetWidget()->InsertCellTextAsInt(i, 1, 0);
                this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(i, 1);
        }
}
//--------------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::MergeBBDoneCallback()
{
        if(this->MergeBBApplyCallback())
                this->MergeBBCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxMergeBBGroup::CopyConstraintValues(
        vtkUnstructuredGrid *input, vtkUnstructuredGrid *output)
{
        vtkIntArray *constrain = vtkIntArray::SafeDownCast(
                input->GetPointData()->GetArray("Constrain"));
        if(constrain)
        {
                vtkPoints *inpoints = input->GetPoints();
                vtkPoints *outpoints = output->GetPoints();
                vtkPointLocator *locator = vtkPointLocator::New();

                int i;
                vtkPoints *points = vtkPoints::New();
                locator->InitPointInsertion(points, outpoints->GetBounds());
                for (i=0; i<inpoints->GetNumberOfPoints(); i++)
                {
                        locator->InsertNextPoint(inpoints->GetPoint(i));
                }

                vtkIntArray *constrainout = vtkIntArray::New();
                constrainout->SetNumberOfValues(outpoints->GetNumberOfPoints());
                for (i=0; i<outpoints->GetNumberOfPoints(); i++)
                {
                        vtkIdType pos;
                        pos = locator->IsInsertedPoint(outpoints->GetPoint(i));
                        if (pos != -1)
                        {
                                constrainout->SetValue(i, constrain->GetValue(pos));
                        }
                        else{
                                constrainout->SetValue(i, 0);
                        }
                }
                constrainout->SetName("Constrain");
                output->GetPointData()->AddArray(constrainout);
                constrainout->Delete();
                locator->Delete();
                points->Delete();
        }
}
//----------------------------------------------------------------------------------
