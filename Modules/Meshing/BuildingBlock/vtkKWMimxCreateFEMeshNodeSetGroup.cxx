/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshNodeSetGroup.cxx,v $
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

#include "vtkKWMimxCreateFEMeshNodeSetGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxSelectPointsWidget.h"
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
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"
#include "vtkRenderer.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkIdList.h"
#include "vtkPointData.h"
#include "vtkKWDialog.h"
#include "vtkIntArray.h"
#include "vtkMimxMeshActor.h"
#include "vtkStringArray.h"

#include "vtkUnstructuredGridWriter.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateFEMeshNodeSetGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateFEMeshNodeSetGroup, "$Revision: 1.19.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshNodeSetGroup::vtkKWMimxCreateFEMeshNodeSetGroup()
{
  this->ObjectListComboBox = NULL;
  this->SelectSubsetRadiobuttonSet = NULL;
  this->SelectPointsWidget = NULL;
  this->NodeSetNameEntry = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshNodeSetGroup::~vtkKWMimxCreateFEMeshNodeSetGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->SelectSubsetRadiobuttonSet)
          this->SelectSubsetRadiobuttonSet->Delete();
  if(this->SelectPointsWidget)
          this->SelectPointsWidget->Delete();
  if(this->NodeSetNameEntry)
          this->NodeSetNameEntry->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Node Set");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth(15);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  if(!this->NodeSetNameEntry)
          this->NodeSetNameEntry = vtkKWEntryWithLabel::New();
  this->NodeSetNameEntry->SetParent(this->MainFrame);
  this->NodeSetNameEntry->Create();
  this->NodeSetNameEntry->SetLabelWidth(15);
  this->NodeSetNameEntry->SetLabelText("Node Set Name : ");
 // this->NodeSetNameEntry->SetLabelWidth(20);
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
          this->NodeSetNameEntry->GetWidgetName());
          
  if(!this->SelectSubsetRadiobuttonSet)
          this->SelectSubsetRadiobuttonSet = vtkKWRadioButtonSet::New();
  this->SelectSubsetRadiobuttonSet->SetParent(this->MainFrame);
  this->SelectSubsetRadiobuttonSet->Create();
  this->SelectSubsetRadiobuttonSet->SetBorderWidth(2);
  this->SelectSubsetRadiobuttonSet->SetReliefToGroove();
  this->SelectSubsetRadiobuttonSet->SetMaximumNumberOfWidgetsInPackingDirection(1);
  for (int id = 0; id < 6; id++)         
          this->SelectSubsetRadiobuttonSet->AddWidget(id);

  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetCommand(this, "SelectNodesThroughCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetText("NT");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetBalloonHelpString(
          "Select Nodes Through");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetValue("NT");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetText("NS");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetCommand(this, "SelectNodesSurfaceCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetBalloonHelpString(
          "Select Surface Nodes");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetValue("NS");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetText("NV");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetCommand(this, "SelectVisibleNodesSurfaceCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetBalloonHelpString(
          "Select Visible Surface Nodes");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetValue("NV");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetText("SN");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetCommand(this, "SelectSingleNodeCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetBalloonHelpString(
          "Select Single Node");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetValue("SN");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetText("FN");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetCommand(this, "SelectFaceNodeCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetBalloonHelpString(
          "Select Nodes Belonging to a Face of the Building block from which the FE mesh was constructed");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetValue("FN");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SetText("VTK");
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SetCommand(this, "VTKInteractionCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SetBalloonHelpString(
          "Normal VTK Interaction");
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SetValue("VTK");
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SetCompoundModeToLeft();
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SelectedStateOn();

  this->GetApplication()->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->SelectSubsetRadiobuttonSet->GetWidgetName());

  

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "CreateNodeSetApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateNodeSetCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshNodeSetGroup::CreateNodeSetApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("FEMesh selection required");
          return 0;
  }

  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
  const char *name = combobox->GetValue();

  int num = combobox->GetValueIndex(name);
  if(num < 0 || num > combobox->GetNumberOfValues()-1)
  {
          callback->ErrorMessage("Choose valid FE Mesh");
          combobox->SetValue("");
          return 0;
  }

  vtkMimxMeshActor *ugridactor = vtkMimxMeshActor::SafeDownCast(
          this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
          this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  if (!this->SelectPointsWidget)
  {
          callback->ErrorMessage("Selection of nodes should be made");
          return 0;
  }
  vtkIdList *idlist = this->SelectPointsWidget->GetSelectedPointIds();
  if(idlist->GetNumberOfIds() == 0)
  {
          callback->ErrorMessage("Number of Nodes selected is 0");
          return 0;
  }
  const char *nodeset = this->NodeSetNameEntry->GetWidget()->GetValue();
  if(!strcmp(nodeset, ""))
  {
          callback->ErrorMessage("Enter the node set name");
          return 0;
  }

  if(ugrid->GetPointData()->GetArray(nodeset))
  {
          vtkKWMessageDialog *KWMessageDialog = vtkKWMessageDialog::New();
          KWMessageDialog->SetStyleToOkCancel();
          KWMessageDialog->SetApplication(this->GetApplication());
          KWMessageDialog->Create();
          KWMessageDialog->SetTitle("Your Attention Please!");
          KWMessageDialog->SetText("A node set with this name already exists. Would you like to overwrite the existing node set?");
          KWMessageDialog->Invoke();
          if(KWMessageDialog->GetStatus() == vtkKWDialog::StatusCanceled)
          {
                  return 0;
          }
          else
          {
                ugridactor->DeleteNodeSet(nodeset);
          }
  }
  int i;
  vtkIntArray *nodearray = vtkIntArray::New();
  nodearray->SetNumberOfValues(ugrid->GetNumberOfPoints());

  for (i=0; i<ugrid->GetNumberOfPoints(); i++) {
          nodearray->SetValue(i, 0);
  }

  for (i=0; i<ugrid->GetNumberOfPoints(); i++)
  {
          if(idlist->IsId(i) != -1)
          {
                  nodearray->SetValue(i,1);
          }
  }
  
  this->GetMimxMainWindow()->SetStatusText("Created Node Set");
  
  nodearray->SetName(nodeset);
  ugrid->GetPointData()->AddArray(nodearray);
  //
  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
          ugrid->GetFieldData()->GetAbstractArray("Node_Set_Names"));
  stringarray->InsertNextValue(nodeset);
  //
  nodearray->Delete();
  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::CreateNodeSetCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
  if(this->SelectPointsWidget)
  {
          if(this->SelectPointsWidget->GetEnabled())
                  this->SelectPointsWidget->SetEnabled(0);
  }
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SelectedStateOn();
        this->VTKInteractionCallback();
}
//-----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::UpdateObjectLists()
{
        this->UpdateMeshComboBox( this->ObjectListComboBox->GetWidget() );
        /*
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->FEMeshList->GetItem(i)->GetFileName());
          int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                                         this->FEMeshList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }                               
        }
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->FEMeshList->GetItem(defaultItem)->GetFileName());
  }
  */
}
//--------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshNodeSetGroup::SelectNodesThroughCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("FEMesh selection required");
                return 0;
        }

        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid FE Mesh");
                combobox->SetValue("");
                return 0;
        }

        vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        if(this->GetSelectedObject())
        {
                this->GetSelectedObject()->HideMesh();
        }
    if(!this->SelectPointsWidget)
        {
                this->SelectPointsWidget = vtkMimxSelectPointsWidget::New();
                this->SelectPointsWidget->SetInteractor(this->GetMimxMainWindow()->
                        GetRenderWidget()->GetRenderWindowInteractor());
        }
        if(this->SelectPointsWidget->GetEnabled())
        {
                this->SelectPointsWidget->SetEnabled(0);
        }
        this->SelectPointsWidget->SetInput(ugrid);
        this->SelectPointsWidget->SetPointSelectionState(0);
        this->SelectPointsWidget->SetEnabled(1);
        return 1;
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshNodeSetGroup::SelectNodesSurfaceCallback()
{
        if(this->SelectNodesThroughCallback())
        {
                this->SelectPointsWidget->SetPointSelectionState(1);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshNodeSetGroup::SelectVisibleNodesSurfaceCallback()
{
        if(this->SelectNodesThroughCallback())
        {
                this->SelectPointsWidget->SetPointSelectionState(2);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshNodeSetGroup::SelectSingleNodeCallback()
{
        if(this->SelectNodesThroughCallback())
        {
                this->SelectPointsWidget->SetPointSelectionState(3);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshNodeSetGroup::VTKInteractionCallback()
{
        if(this->SelectPointsWidget)
        {
                if(this->SelectPointsWidget->GetEnabled())
                {
                        this->SelectPointsWidget->SetEnabled(0);
                }
        }
        if(this->GetSelectedObject())
        {
                if(this->SelectPointsWidget)
                        this->SelectPointsWidget->SetInput(this->GetSelectedObject()->GetDataSet());
                this->GetSelectedObject()->ShowMesh();
                this->GetMimxMainWindow()->GetRenderWidget()->Render();
        }
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshNodeSetGroup::SelectFaceNodeCallback()
{
        if(this->SelectNodesThroughCallback())
        {
                this->SelectPointsWidget->SetPointSelectionState(4);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
vtkMimxMeshActor* vtkKWMimxCreateFEMeshNodeSetGroup::GetSelectedObject()
{
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                return NULL;
        }

        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                return NULL;
        }

        return vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

}
//-----------------------------------------------------------------------------------
