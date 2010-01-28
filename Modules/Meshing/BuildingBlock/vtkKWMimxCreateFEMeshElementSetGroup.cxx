/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshElementSetGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.20.4.2 $

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

#include "vtkKWMimxCreateFEMeshElementSetGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkMimxCreateElementSetWidgetFEMesh.h"
#include "vtkUnstructuredGrid.h"
#include "vtkStringArray.h"

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
#include "vtkIdList.h"
#include "vtkIntArray.h"
#include "vtkCellData.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWDialog.h"

#include "vtkUnstructuredGridWriter.h"
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkKWPushButtonSet.h"
#include "vtkMimxMeshActor.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateFEMeshElementSetGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateFEMeshElementSetGroup, "$Revision: 1.20.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshElementSetGroup::vtkKWMimxCreateFEMeshElementSetGroup()
{
  this->ObjectListComboBox = NULL;
  this->SelectSubsetRadiobuttonSet = NULL;
  this->SelectCellsWidget = NULL;
  this->ElementSetNameEntry = NULL;
  this->AcceptSelectionButtonSet = NULL;
  this->SelectionState = 0;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshElementSetGroup::~vtkKWMimxCreateFEMeshElementSetGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->SelectSubsetRadiobuttonSet)
          this->SelectSubsetRadiobuttonSet->Delete();
  if(this->SelectCellsWidget)
          this->SelectCellsWidget->Delete();
  if(this->ElementSetNameEntry)
          this->ElementSetNameEntry->Delete();
  if(this->AcceptSelectionButtonSet)
          this->AcceptSelectionButtonSet->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Element Set");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth(15);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  if(!this->ElementSetNameEntry)
          this->ElementSetNameEntry = vtkKWEntryWithLabel::New();

  this->ElementSetNameEntry->SetParent(this->MainFrame);
  this->ElementSetNameEntry->Create();
  this->ElementSetNameEntry->SetLabelWidth(15);
  this->ElementSetNameEntry->SetLabelText("Element Set Name : ");
  // this->ElementSetNameEntry->SetLabelWidth(20);
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand n -padx 2 -pady 2 -fill x", 
          this->ElementSetNameEntry->GetWidgetName());
          
          
  if(!this->SelectSubsetRadiobuttonSet)
          this->SelectSubsetRadiobuttonSet = vtkKWRadioButtonSet::New();
  this->SelectSubsetRadiobuttonSet->SetParent(this->MainFrame);
  this->SelectSubsetRadiobuttonSet->Create();
  this->SelectSubsetRadiobuttonSet->SetBorderWidth(2);
  this->SelectSubsetRadiobuttonSet->SetReliefToGroove();
  this->SelectSubsetRadiobuttonSet->SetMaximumNumberOfWidgetsInPackingDirection(1);
  for (int id = 0; id < 6; id++)         
          this->SelectSubsetRadiobuttonSet->AddWidget(id);

  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetCommand(this, "SelectElementsThroughCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetText("ET");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetBalloonHelpString(
          "Select Elements Through");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetValue("ET");
  this->SelectSubsetRadiobuttonSet->GetWidget(0)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetText("ES");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetCommand(this, "SelectElementsSurfaceCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetBalloonHelpString(
          "Select Surface Elements");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetValue("ES");
  this->SelectSubsetRadiobuttonSet->GetWidget(1)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetText("EV");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetCommand(this, "SelectVisibleElementsSurfaceCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetBalloonHelpString(
          "Select Visible Surface Elements");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetValue("EV");
  this->SelectSubsetRadiobuttonSet->GetWidget(2)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetText("SE");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetCommand(this, "SelectSingleElementCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetBalloonHelpString(
          "Select Single Element");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetValue("SE");
  this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetCompoundModeToLeft();

  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetText("ME");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetCommand(this, "SelectMultipleElementsCallback");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->IndicatorVisibilityOff();
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetBalloonHelpString(
          "Select Multiple Elements");
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetVariableName(
          this->SelectSubsetRadiobuttonSet->GetWidget(0)->GetVariableName());
  this->SelectSubsetRadiobuttonSet->GetWidget(4)->SetValue("ME");
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

  if(!this->AcceptSelectionButtonSet)
          this->AcceptSelectionButtonSet = vtkKWPushButtonSet::New();

  this->AcceptSelectionButtonSet->SetParent(this->MainFrame);
  this->AcceptSelectionButtonSet->Create();
  this->AcceptSelectionButtonSet->SetBorderWidth(2);
  this->AcceptSelectionButtonSet->SetReliefToGroove();
  this->AcceptSelectionButtonSet->SetMaximumNumberOfWidgetsInPackingDirection(1);
  for (int id = 0; id < 2; id++)          this->AcceptSelectionButtonSet->AddWidget(id);

  this->AcceptSelectionButtonSet->GetWidget(0)->SetCommand(this, "AcceptSelectionCallback");
  
  this->AcceptSelectionButtonSet->GetWidget(0)->SetBalloonHelpString("Accept the selected subset");
  this->AcceptSelectionButtonSet->GetWidget(0)->SetCompoundModeToLeft();
        this->AcceptSelectionButtonSet->GetWidget(0)->SetText("Acccept");

  this->AcceptSelectionButtonSet->GetWidget(1)->SetCommand(this, "FullCallback");
  this->AcceptSelectionButtonSet->GetWidget(1)->SetBalloonHelpString("Revert back to the original mesh");
  this->AcceptSelectionButtonSet->GetWidget(1)->SetCompoundModeToRight();
  this->AcceptSelectionButtonSet->GetWidget(1)->SetText("Full");

  this->GetApplication()->Script( "pack %s -side top -anchor nw -expand n -padx 2 -pady 6", 
          this->AcceptSelectionButtonSet->GetWidgetName());

  

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "CreateElementSetApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateElementSetCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshElementSetGroup::CreateElementSetApplyCallback()
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
  
  vtkMimxMeshActor *ugridactor = vtkMimxMeshActor::
    SafeDownCast(this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
  vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
  
  if (!this->SelectCellsWidget)
    {
    callback->ErrorMessage("Selection of elements should be made");
    return 0;
    }
  vtkIdList *idlist = this->SelectCellsWidget->GetSelectedCellIds();
  if(idlist->GetNumberOfIds() == 0)
    {
    callback->ErrorMessage("Number of elements selected is 0");
    return 0;
    }
  const char *elementset = this->ElementSetNameEntry->GetWidget()->GetValue();
  if(!strcmp(elementset, ""))
    {
    callback->ErrorMessage("Enter the element set name");
    return 0;
    }
  
  if(ugrid->GetCellData()->GetArray(elementset))
    {
    vtkKWMessageDialog *KWMessageDialog = vtkKWMessageDialog::New();
    KWMessageDialog->SetStyleToOkCancel();
    KWMessageDialog->SetApplication(this->GetApplication());
    KWMessageDialog->Create();
    KWMessageDialog->SetTitle("Your Atttention Please.!");
    KWMessageDialog->SetText("Element Set with the name already exists. Would you like to overwrite the existing Element Set");
    KWMessageDialog->Invoke();
    if(KWMessageDialog->GetStatus() == vtkKWDialog::StatusCanceled)
      {
      return 0;
      }
    else
      {
      ugridactor->DeleteElementSet(elementset);
      }
    }
  int i;
  vtkIntArray *elementarray = vtkIntArray::New();
  elementarray->SetNumberOfValues(ugrid->GetNumberOfCells());
  
  for (i=0; i<ugrid->GetNumberOfCells(); i++)
    {
    elementarray->SetValue(i, 0);
    }

  for (i=0; i<ugrid->GetNumberOfCells(); i++)
    {
    if(idlist->IsId(i) != -1)
      {
      elementarray->SetValue(i,1);
      }
    }
  elementarray->SetName(elementset);
  ugrid->GetCellData()->AddArray(elementarray);
  
  vtkStringArray *stringarray = vtkStringArray::SafeDownCast(
    ugrid->GetFieldData()->GetAbstractArray("Element_Set_Names"));
  if (! stringarray )
    {
    stringarray = vtkStringArray::New();
    stringarray->SetName("Element_Set_Names");
    ugrid->GetFieldData()->AddArray( stringarray );
    stringarray->Delete();
    }
  stringarray->InsertNextValue(elementset);
  elementarray->Delete();
        
  this->GetMimxMainWindow()->SetStatusText("Created Element Set");
        
  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::CreateElementSetCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
  if(this->SelectCellsWidget)
  {
          //if(this->SelectCellsWidget->GetEnabled())
                  //this->SelectCellsWidget->SetEnabled(0);
  }
  this->SelectSubsetRadiobuttonSet->GetWidget(5)->SelectedStateOn();
  this->VTKInteractionCallback();
}
//-----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::UpdateObjectLists()
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
int vtkKWMimxCreateFEMeshElementSetGroup::SelectElementsThroughCallback()
{
        this->SelectionState = 1;
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
    if(!this->SelectCellsWidget)
        {
                this->SelectCellsWidget = vtkMimxCreateElementSetWidgetFEMesh::New();
                this->SelectCellsWidget->SetInteractor(this->GetMimxMainWindow()->
                        GetRenderWidget()->GetRenderWindowInteractor());
                this->SelectCellsWidget->SetInput(ugrid);
        }
        if(this->SelectCellsWidget->GetEnabled())
        {
                this->SelectCellsWidget->SetEnabled(0);
        }
        if(this->GetSelectedObject())
        {
                this->GetSelectedObject()->HideMesh();
        }
        this->SelectCellsWidget->SetEnabled(1);
        this->SelectCellsWidget->SetCellSelectionState(0);
        return 1;
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshElementSetGroup::SelectElementsSurfaceCallback()
{
        if(this->SelectElementsThroughCallback())
        {
                this->SelectCellsWidget->SetCellSelectionState(1);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshElementSetGroup::SelectVisibleElementsSurfaceCallback()
{
        if(this->SelectElementsThroughCallback())
        {
                this->SelectCellsWidget->SetCellSelectionState(2);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshElementSetGroup::SelectSingleElementCallback()
{
        if(this->SelectElementsThroughCallback())
        {
                this->SelectCellsWidget->SetCellSelectionState(3);
                return 1;
        }
        return 0;
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::VTKInteractionCallback()
{
        this->SelectionState = 0;
        if(this->SelectCellsWidget)
        {
                if(this->SelectCellsWidget->GetEnabled())
                {
                        this->SelectCellsWidget->SetEnabled(0);
                }
        }
        if(this->GetSelectedObject())
        {
                if(this->SelectCellsWidget)
                        this->SelectCellsWidget->SetInput(this->GetSelectedObject()->GetDataSet());
                this->GetSelectedObject()->ShowMesh();
                this->GetMimxMainWindow()->GetRenderWidget()->Render();
        }
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::FullCallback()
{
        if (this->SelectCellsWidget)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
                {
                        callback->ErrorMessage("FEMesh selection required");
                        return;
                }

                vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                const char *name = combobox->GetValue();

                int num = combobox->GetValueIndex(name);
                if(num < 0 || num > combobox->GetNumberOfValues()-1)
                {
                        callback->ErrorMessage("Choose valid FE Mesh");
                        combobox->SetValue("");
                        return;
                }

                vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
                        this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                this->SelectCellsWidget->SetInput(ugrid);
                this->GetMimxMainWindow()->GetRenderWidget()->Render();
                this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetEnabled(1);
        }
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshElementSetGroup::SelectionChangedCallback(const char *vtkNotUsed(Selection))
{
        if (this->SelectCellsWidget)
        {
                vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
                if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
                {
                        return;
                }

                vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                const char *name = combobox->GetValue();

                int num = combobox->GetValueIndex(name);
                if(num < 0 || num > combobox->GetNumberOfValues()-1)
                {
                        callback->ErrorMessage("Choose valid FE Mesh");
                        combobox->SetValue("");
                        return;
                }

                vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
                        this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                this->SelectCellsWidget->SetInput(ugrid);
                this->SelectSubsetRadiobuttonSet->GetWidget(3)->SetEnabled(1);
        }
}
//---------------------------------------------------------------------------------
vtkMimxMeshActor* vtkKWMimxCreateFEMeshElementSetGroup::GetSelectedObject()
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
int vtkKWMimxCreateFEMeshElementSetGroup::SelectMultipleElementsCallback()
{
        if(this->SelectElementsThroughCallback())
        {
                this->SelectCellsWidget->SetCellSelectionState(4);
                return 1;
        }
        return 0;
}
//------------------------------------------------------------------------------------
