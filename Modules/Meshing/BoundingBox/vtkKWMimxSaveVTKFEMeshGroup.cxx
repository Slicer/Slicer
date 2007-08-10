/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSaveVTKFEMeshGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxSaveVTKFEMeshGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxMainMenuGroup.h"

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
#include "vtkKWNotebook.h"
#include "vtkKWOptions.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWPushButton.h"

#include "vtkUnstructuredGridWriter.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxSaveVTKFEMeshGroup);
vtkCxxRevisionMacro(vtkKWMimxSaveVTKFEMeshGroup, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKFEMeshGroup::vtkKWMimxSaveVTKFEMeshGroup()
{
  this->ObjectListComboBox = NULL;
  this->FileBrowserDialog = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKFEMeshGroup::~vtkKWMimxSaveVTKFEMeshGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->FileBrowserDialog)
    this->FileBrowserDialog->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::CreateWidget()
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
  this->MainFrame->SetLabelText("Save F E Mesh (VTK file format)");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Object Selection : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  int i;
  for (i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    ObjectListComboBox->GetWidget()->AddValue(
      this->FEMeshList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    ObjectListComboBox->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "SaveVTKFEMeshDoneCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 20 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "SaveVTKFEMeshCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::SaveVTKFEMeshDoneCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
  vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(this->FEMeshList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  if(!this->FileBrowserDialog)
  {
    this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
    this->FileBrowserDialog->SaveDialogOn();
    this->FileBrowserDialog->SetApplication(this->GetApplication());
//    dialog->SetParent(this->RenderWidget->GetParentTopLevel()) ;
    this->FileBrowserDialog->Create();
//    this->FileBrowserDialog->RetrieveLastPathFromRegistry("FEMeshDataPath");
    this->FileBrowserDialog->SetTitle ("Save F E Mesh (VTK Unstructured grid File format)");
    this->FileBrowserDialog->SetFileTypes ("{{VTK files} {.vtk}}");
    this->FileBrowserDialog->SetDefaultExtension (".vtk");
  }
  this->FileBrowserDialog->Invoke();
  if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
  {
    if(this->FileBrowserDialog->GetFileName())
    {
      const char *filename = this->FileBrowserDialog->GetFileName();
      vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
      writer->SetFileName(this->FileBrowserDialog->GetFileName());
      writer->SetInput(ugrid);
      writer->Update();
      writer->Delete();
    }
  }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKFEMeshGroup::SaveVTKFEMeshCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//------------------------------------------------------------------------------
