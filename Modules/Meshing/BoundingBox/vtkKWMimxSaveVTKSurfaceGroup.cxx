/*=========================================================================

  Module:    $RCSfile: vtkKWMimxSaveVTKSurfaceGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxSaveVTKSurfaceGroup.h"
#include "vtkKWMimxViewWindow.h"
#include "vtkKWMimxMainMenuGroup.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"

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

#include "vtkPolyDataWriter.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxSaveVTKSurfaceGroup);
vtkCxxRevisionMacro(vtkKWMimxSaveVTKSurfaceGroup, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKSurfaceGroup::vtkKWMimxSaveVTKSurfaceGroup()
{
  this->ObjectListComboBox = NULL;
  this->FileBrowserDialog = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxSaveVTKSurfaceGroup::~vtkKWMimxSaveVTKSurfaceGroup()
{
  if(this->ObjectListComboBox)
     this->ObjectListComboBox->Delete();
  if(this->FileBrowserDialog)
    this->FileBrowserDialog->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKSurfaceGroup::CreateWidget()
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
  this->MainFrame->SetLabelText("Save VTK Surface");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  ObjectListComboBox->SetParent(this->MainFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Object Selection : ");
  ObjectListComboBox->GetWidget()->ReadOnlyOn();

  int i;
  for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    ObjectListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    ObjectListComboBox->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "SaveVTKSurfaceDoneCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 20 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "SaveVTKSurfaceCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKSurfaceGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxSaveVTKSurfaceGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKSurfaceGroup::SaveVTKSurfaceDoneCallback()
{
  if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
  vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  if(!this->FileBrowserDialog)
  {
    this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
    this->FileBrowserDialog->SaveDialogOn();
    this->FileBrowserDialog->SetApplication(this->GetApplication());
//    dialog->SetParent(this->RenderWidget->GetParentTopLevel()) ;
    this->FileBrowserDialog->Create();
//    this->FileBrowserDialog->RetrieveLastPathFromRegistry("FEMeshDataPath");
    this->FileBrowserDialog->SetTitle ("Save VTK Polydata File Format");
    this->FileBrowserDialog->SetFileTypes ("{{VTK files} {.vtk}}");
    this->FileBrowserDialog->SetDefaultExtension (".vtk");
  }
  this->FileBrowserDialog->Invoke();
  if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
  {
    if(this->FileBrowserDialog->GetFileName())
    {
      const char *filename = this->FileBrowserDialog->GetFileName();
      vtkPolyDataWriter *writer = vtkPolyDataWriter::New();
      writer->SetFileName(this->FileBrowserDialog->GetFileName());
      writer->SetInput(polydata);
      writer->Update();
      writer->Delete();
    }
  }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKSurfaceGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxSaveVTKSurfaceGroup::SaveVTKSurfaceCancelCallback()
{
//  this->MainFrame->UnpackChildren();
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//------------------------------------------------------------------------------
