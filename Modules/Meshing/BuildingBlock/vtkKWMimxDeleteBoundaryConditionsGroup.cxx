/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxDeleteBoundaryConditionsGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.7.4.2 $

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

#include "vtkKWMimxDeleteBoundaryConditionsGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainMenuGroup.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxMeshActor.h"

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
#include "vtkKWMimxMainUserInterfacePanel.h"
#include "vtkMimxMeshActor.h"
#include "vtkKWMessageDialog.h"

#include "vtkUnstructuredGrid.h"
#include "vtkIntArray.h"
#include "vtkFieldData.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxDeleteBoundaryConditionsGroup);
vtkCxxRevisionMacro(vtkKWMimxDeleteBoundaryConditionsGroup, "$Revision: 1.7.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxDeleteBoundaryConditionsGroup::vtkKWMimxDeleteBoundaryConditionsGroup()
{
  this->ObjectListComboBox = NULL;
  this->StepNumberComboBox = NULL;
  this->CancelStatus = 0;
}

//----------------------------------------------------------------------------
vtkKWMimxDeleteBoundaryConditionsGroup::~vtkKWMimxDeleteBoundaryConditionsGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->StepNumberComboBox)
          this->StepNumberComboBox->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::CreateWidget()
{
  if(this->IsCreated())
  {
  vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Delete Boundary Condition ");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Mesh : ");
  ObjectListComboBox->SetLabelWidth( 15 );
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  // step number combobox
  if(!this->StepNumberComboBox) 
  {
          this->StepNumberComboBox = vtkKWComboBoxWithLabel::New();
  }
  this->StepNumberComboBox->SetParent(this->MainFrame);
  this->StepNumberComboBox->Create();
  this->StepNumberComboBox->SetLabelText("Step Number : ");
  this->StepNumberComboBox->SetLabelWidth( 15 );
  this->StepNumberComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
          this->StepNumberComboBox->GetWidgetName());
  //
  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "DeleteBoundaryConditionApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "DeleteBoundaryConditionCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxDeleteBoundaryConditionsGroup::DeleteBoundaryConditionApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("FE Mesh selection required");;
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
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));

        vtkKWMessageDialog *Dialog = vtkKWMessageDialog::New();
        Dialog->SetStyleToOkCancel();
        Dialog->SetApplication(this->GetApplication());
        Dialog->Create();
        Dialog->SetTitle("Your Attention Please!");
        Dialog->SetText("Would you like to delete the chosen step number?");
        Dialog->Invoke();
        if(Dialog->GetStatus() != vtkKWMessageDialog::StatusOK)
        {
                Dialog->Delete();
                return 1;
        }

        vtkKWComboBox *stepcombobox = this->StepNumberComboBox->GetWidget();
        meshActor->DeleteBoundaryConditionStep(stepcombobox->GetValueAsInt());
        Dialog->Delete();
        int numSteps = stepcombobox->GetNumberOfValues();
        stepcombobox->DeleteValue(numSteps-1);

        vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();

        vtkIntArray *boundCond = vtkIntArray::SafeDownCast(
                ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));
        if(!boundCond)
        {
                return 0;
        }
        boundCond->SetValue(0, numSteps-1);
        return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::DeleteBoundaryConditionCancelCallback()
{
//  this->MainFrame->UnpackChildren();
        this->CancelStatus = 1;
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
    this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
        this->CancelStatus = 0;
}
//------------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::UpdateObjectLists()
{
        this->UpdateMeshComboBox( this->ObjectListComboBox->GetWidget() );
        /*
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->FEMeshList->GetItem(i)->GetFileName());
                
                vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(i));  
    bool viewedItem = meshActor->GetMeshVisibility();
    
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
        this->SelectionChangedCallback(NULL);
}
//--------------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::DeleteBoundaryConditionDoneCallback()
{
        if(this->DeleteBoundaryConditionApplyCallback())
                this->DeleteBoundaryConditionCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxDeleteBoundaryConditionsGroup::SelectionChangedCallback(
        const char *vtkNotUsed(Selection))
{
        if(this->CancelStatus)
        {
                return;
        }
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();

        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();

        if(!strcmp(combobox->GetValue(),""))
        {
                return;
        }

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                return;
        }
        vtkMimxMeshActor *meshActor = vtkMimxMeshActor::SafeDownCast(
                this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
        vtkUnstructuredGrid *ugrid = meshActor->GetDataSet();

        vtkIntArray *boundCond = vtkIntArray::SafeDownCast(
                ugrid->GetFieldData()->GetArray("Boundary_Condition_Number_Of_Steps"));
        if(!boundCond)  return;
        int numSteps = boundCond->GetValue(0);
        this->StepNumberComboBox->GetWidget()->DeleteAllValues();
        if(!boundCond || !numSteps)
        {
                callback->ErrorMessage("Boundary condition data does not exist for the mesh selected");
                this->StepNumberComboBox->GetWidget()->SetValue("");
                return;
        }
        // populate the step number combobox.
        int i;
        for (i=0; i<numSteps; i++)
        {
                this->StepNumberComboBox->GetWidget()->AddValueAsInt(i+1);
        }
        this->StepNumberComboBox->GetWidget()->SetValue(
                this->StepNumberComboBox->GetWidget()->GetValueFromIndex(0));
}
//---------------------------------------------------------------------------------
