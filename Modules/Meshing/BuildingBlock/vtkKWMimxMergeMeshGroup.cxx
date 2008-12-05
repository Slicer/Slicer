/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxMergeMeshGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.22.4.1 $

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

#include "vtkKWMimxMergeMeshGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxMeshActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkCellData.h"
#include "vtkPointData.h"

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
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkMimxMeshActor.h"

#include "vtkDataSetCollection.h"
#include "vtkMimxMergeNodesPointAndCellData.h"
#include "vtkKWMimxMainUserInterfacePanel.h"


#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxMergeMeshGroup);
vtkCxxRevisionMacro(vtkKWMimxMergeMeshGroup, "$Revision: 1.22.4.1 $");

//----------------------------------------------------------------------------
vtkKWMimxMergeMeshGroup::vtkKWMimxMergeMeshGroup()
{
        this->MultiColumnList = NULL;
        this->NodeMergingFrame = NULL;
        this->NodeMergeCheckButton = NULL;
        this->ToleranceEntry = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxMergeMeshGroup::~vtkKWMimxMergeMeshGroup()
{
        if(this->MultiColumnList)
                this->MultiColumnList->Delete();
        this->NodeMergingFrame->Delete();
        this->NodeMergeCheckButton->Delete();
        this->ToleranceEntry->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::CreateWidget()
{
  if(this->IsCreated())
  {
  vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Merge Meshes");

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
  this->MultiColumnList->GetWidget()->SetHeight(5);
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
  //  this->MultiColumnList->SetSelectioModeToSingle();
  //  this->MultiColumnList->RowSeparatorsVisibilityOff();
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
          this->MultiColumnList->GetWidgetName());

  this->NodeMergingFrame = vtkKWFrameWithLabel::New();
  this->NodeMergingFrame->SetParent(this->MainFrame);
  this->NodeMergingFrame->Create();
  this->NodeMergingFrame->SetLabelText("Merge Nodes");
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->NodeMergingFrame->GetWidgetName());

  this->NodeMergeCheckButton = vtkKWCheckButtonWithLabel::New();
  this->NodeMergeCheckButton->SetParent(this->NodeMergingFrame->GetFrame());
  this->NodeMergeCheckButton->Create();
  this->NodeMergeCheckButton->SetLabelText("Merge Nodes");
//  this->NodeMergeCheckButton->SetLabelWidth();
  this->NodeMergeCheckButton->SetLabelPositionToLeft();
  this->NodeMergeCheckButton->GetWidget()->SetCommand(this, "MergeNodesCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
          this->NodeMergeCheckButton->GetWidgetName());

  if (!this->ToleranceEntry)
          this->ToleranceEntry = vtkKWEntryWithLabel::New();

  this->ToleranceEntry->SetParent(this->NodeMergingFrame->GetFrame());
  this->ToleranceEntry->Create();
  this->ToleranceEntry->SetWidth(2);
  this->ToleranceEntry->SetLabelText("Tolerance : ");
  this->ToleranceEntry->GetWidget()->SetValueAsDouble(0.0);
  this->ToleranceEntry->GetWidget()->SetRestrictValueToDouble();
  this->ToleranceEntry->SetEnabled(0);
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand n -padx 2 -pady 2", 
          this->ToleranceEntry->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "MergeMeshApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "MergeMeshCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxMergeMeshGroup::MergeMeshApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        vtkDataSetCollection *collection = vtkDataSetCollection::New();
        collection->InitTraversal();

        vtkMimxMergeNodesPointAndCellData *merge = new vtkMimxMergeNodesPointAndCellData;
        
        int i;
        
        if(MultiColumnList->GetWidget()->GetNumberOfRows() > 1)
        {
                int localcount = 0;
                for (i=0; i < this->FEMeshList->GetNumberOfItems(); i++)
                {
                        vtkMimxMeshActor *actor = vtkMimxMeshActor::
                                SafeDownCast(this->FEMeshList->GetItem(i));
                        vtkUnstructuredGrid *ugrid = actor->GetDataSet();
                        vtkDataArray *nodearray = ugrid->GetPointData()->GetArray("Node_Numbers");
                        vtkDataArray *elementarray = ugrid->GetCellData()->GetArray("Element_Numbers");
                        int num = ugrid->GetCellData()->GetNumberOfArrays();
                        if(nodearray && elementarray && num > 1)
                        {
                                if(this->MultiColumnList->GetWidget()->GetCellWindowAsCheckButton(localcount++,1))
                                {
                                        collection->AddItem(ugrid);
                                }
                        }
                        
                }
                if(collection->GetNumberOfItems() < 2)
                {
                        callback->ErrorMessage("At least 2 items should be selected");
                        delete merge;
                        collection->Delete();
                        return 0;
                }

                merge->SetDataSetCollection(collection);
                merge->SetNodesMerge(this->NodeMergeCheckButton->GetWidget()->GetSelectedState());
                merge->SetTolerance(this->ToleranceEntry->GetWidget()->GetValueAsDouble());

                if(merge->MergeDataSets())
                {
                        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::New();
                this->FEMeshList->AppendItem(meshActor);
                meshActor->SetDataSet( merge->GetMergedDataSet() );
                meshActor->SetRenderer( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer() );
                meshActor->SetInteractor( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor() );
                        
                        this->Count++;
                        vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                this->FEMeshList->GetNumberOfItems()-1))->SetObjectName("MergedMesh_",Count);
                        //vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                        //      this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->Modified();
                        //this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(
                        //      this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->GetActor());
                        this->GetMimxMainWindow()->GetRenderWidget()->Render();
                        this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
                        this->GetMimxMainWindow()->GetViewProperties()->AddObjectList(
                                this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1));
                        this->UpdateObjectLists();
                        
                        this->GetMimxMainWindow()->SetStatusText("Merged Meshes");
                        
                        return 1;
                }
        }
        else
        {
                callback->ErrorMessage("Minimum of 2 FE meshes should be present");
        }
        delete merge;
        collection->Delete();
        return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::MergeMeshCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::UpdateObjectLists()
{
        this->MultiColumnList->GetWidget()->DeleteAllRows();
        int localcount = 0;
        for (int i=0; i < this->FEMeshList->GetNumberOfItems(); i++)
        {
                vtkMimxMeshActor *actor = vtkMimxMeshActor::
                        SafeDownCast(this->FEMeshList->GetItem(i));
                vtkUnstructuredGrid *ugrid = actor->GetDataSet();
                vtkDataArray *nodearray = ugrid->GetPointData()->GetArray("Node_Numbers");
                vtkDataArray *elementarray = ugrid->GetCellData()->GetArray("Element_Numbers");
                int num = ugrid->GetCellData()->GetNumberOfArrays();
                if(nodearray && elementarray && num > 1)
                {
                        this->MultiColumnList->GetWidget()->InsertCellText(localcount, 0, actor->GetFileName());
                        this->MultiColumnList->GetWidget()->InsertCellTextAsInt(localcount, 1, 0);
                        this->MultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton(localcount++, 1);
                }
        }
}
//--------------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::MergeMeshDoneCallback()
{
        if(this->MergeMeshApplyCallback())
                this->MergeMeshCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxMergeMeshGroup::MergeNodesCallback(int State)
{
        this->ToleranceEntry->SetEnabled(State);
}
//---------------------------------------------------------------------------------
