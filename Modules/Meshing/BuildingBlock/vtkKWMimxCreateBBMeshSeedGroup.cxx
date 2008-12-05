/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateBBMeshSeedGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.28.4.1 $

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

#include "vtkKWMimxCreateBBMeshSeedGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkMimxUnstructuredGridWidget.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRenderer.h"

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
#include "vtkKWMimxMainUserInterfacePanel.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateBBMeshSeedGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateBBMeshSeedGroup, "$Revision: 1.28.4.1 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateBBMeshSeedGroup::vtkKWMimxCreateBBMeshSeedGroup()
{
        //this->MainFrame = NULL;
//  this->BBoxList = vtkLinkedListWrapper::New();
  this->MimxMainWindow = NULL;
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
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Building Block Mesh Seeding");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if(!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->MainFrame);
  ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  ObjectListComboBox->SetLabelText("Building Block : ");
  this->ObjectListComboBox->SetLabelWidth(20);
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  this->AvElementLength->SetParent(this->MainFrame);
  this->AvElementLength->Create();
  //this->AvElementLength->SetWidth(5);
  this->AvElementLength->SetLabelText("Average Element Length ");
  this->AvElementLength->GetWidget()->SetValueAsDouble(1.0);
  this->AvElementLength->GetWidget()->SetRestrictValueToDouble();
  this->AvElementLength->SetLabelWidth( 20 );
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 6 -fill x", 
    this->AvElementLength->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "CreateBBMeshSeedApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateBBMeshSeedCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand n -padx 5 -pady 6", 
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
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateBBMeshSeedGroup::CreateBBMeshSeedApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
//      callback->SetState(0);
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("Building Block to be mesh seeded not selected");
          return 0;
  }
  
  if(this->AvElementLength->GetWidget()->GetValueAsDouble() <= 0.0)
  {
          callback->ErrorMessage("Average element length cannot be <= 0.0");
          return 0;
  }

    vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();

        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid Building-block structure");
                combobox->SetValue("");
                return 0;
        }

    vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
      SafeDownCast(this->BBoxList->GetItem(combobox->GetValueIndex(name)));
    vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
    if(!ugrid->GetCellData()->GetArray("Mesh_Seed"))
    {
      double elementLength = this->AvElementLength->GetWidget()->GetValueAsDouble();
      ugridactor->MeshSeedFromAverageElementLength(elementLength, elementLength, elementLength);
    }
        else
        {
                vtkKWMessageDialog *Dialog = vtkKWMessageDialog::New();
                Dialog->SetStyleToOkCancel();
                Dialog->SetApplication(this->GetApplication());
                Dialog->Create();
                Dialog->SetTitle("A user-defined material property has already been assigned to this element set");
                Dialog->SetText("Mesh seeds are already present, would you like to overwrite?");
                Dialog->Invoke();
                if(Dialog->GetStatus() == vtkKWMessageDialog::StatusOK)
                {
                        double elementLength = this->AvElementLength->GetWidget()->GetValueAsDouble();
                        ugridactor->MeshSeedFromAverageElementLength(elementLength, elementLength, elementLength);
                }
                Dialog->Delete();
        }
        callback->ErrorMessage("Mesh Seeding Done.");
        return 1;
 // this->CreateBBMeshSeedCancelCallback();
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
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::UpdateObjectLists()
{
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
  
  int defaultItem = -1;
        for (int i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->BBoxList->GetItem(i)->GetFileName());
                        
          int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->BBoxList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
        }
        
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->BBoxList->GetItem(defaultItem)->GetFileName());
  }
  
}
//--------------------------------------------------------------------------------
void vtkKWMimxCreateBBMeshSeedGroup::CreateBBMeshSeedDoneCallback()
{
        if(this->CreateBBMeshSeedApplyCallback())
                this->CreateBBMeshSeedCancelCallback();
}
//---------------------------------------------------------------------------------
