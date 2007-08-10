/*=========================================================================

  Module:    $RCSfile: vtkKWMimxFEMeshMenuGroup.cxx,v $coyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxFEMeshMenuGroup.h"
#include "vtkKWMimxCreateBBFromBoundsGroup.h"
#include "vtkKWMimxCreateBBMeshSeedGroup.h"
#include "vtkKWMimxEditBBGroup.h"
#include "vtkKWMimxCreateFEMeshFromBBGroup.h"
#include "vtkKWMimxViewProperties.h"
#include "vtkKWMimxSaveVTKBBGroup.h"
#include "vtkKWMimxEditBBMeshSeedGroup.h"
#include "vtkKWMimxSaveVTKFEMeshGroup.h"
#include "vtkKWMimxDeleteObjectGroup.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkKWMimxEditFEMeshLaplacianSmoothGroup.h"

#include "vtkMimxUnstructuredGridWidget.h"

#include "vtkMimxActorBase.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkActor.h"
#include "vtkDataSet.h"
#include "vtkDataSetMapper.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"
#include "vtkUnstructuredGrid.h"
#include "vtkUnstructuredGridReader.h"

#include "vtkKWApplication.h"
#include "vtkKWCheckButton.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWEvent.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithScrollbar.h"
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
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLoadSaveDialog.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxFEMeshMenuGroup);
vtkCxxRevisionMacro(vtkKWMimxFEMeshMenuGroup, "$Revision: 1.9 $");

//----------------------------------------------------------------------------
vtkKWMimxFEMeshMenuGroup::vtkKWMimxFEMeshMenuGroup()
{
  this->ObjectMenuButton = vtkKWMenuButtonWithLabel::New();
  this->OperationMenuButton = NULL;
  this->TypeMenuButton = NULL;
  this->BBoxList = vtkLinkedListWrapper::New();
  this->FEMeshList = vtkLinkedListWrapper::New();
  this->CreateBBFromBounds = NULL;
  this->CreateBBMeshSeed = NULL;
  this->EditBB = NULL;
  this->FEMeshFromBB = NULL;
  this->MainFrame = NULL;
  this->BBViewProperties = NULL;
  this->MimxViewProperties = NULL;
  this->SaveVTKBBGroup = NULL;
  this->EditBBMeshSeedGroup = NULL;
  this->SaveVTKFEMeshGroup = NULL;
  this->FileBrowserDialog = NULL;
  this->DeleteObjectGroup = NULL;
  this->FEMeshLaplacianSmooth = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxFEMeshMenuGroup::~vtkKWMimxFEMeshMenuGroup()
{
  if (this->BBoxList) {
    for (int i=0; i < this->BBoxList->GetNumberOfItems(); i++) {
      vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(i))->Delete();
    }
    this->BBoxList->Delete();
  }

  if (this->FEMeshList) {
    for (int i=0; i < this->FEMeshList->GetNumberOfItems(); i++) {
      vtkMimxUnstructuredGridActor::SafeDownCast(
        this->FEMeshList->GetItem(i))->Delete();
    }
    this->FEMeshList->Delete();
  }

  if (this->OperationMenuButton)
    this->OperationMenuButton->Delete();

  if (this->TypeMenuButton)
    this->TypeMenuButton->Delete();

  this->ObjectMenuButton->Delete();

  if(this->CreateBBFromBounds)
    this->CreateBBFromBounds->Delete();

  if(this->EditBB)
    this->EditBB->Delete();

  if(this->CreateBBMeshSeed)
    this->CreateBBMeshSeed->Delete();

  if (this->FEMeshFromBB)
    this->FEMeshFromBB->Delete();
  if(this->BBViewProperties)
    this->BBViewProperties;
  if(this->MimxViewProperties)
    this->MimxViewProperties->Delete();
  if(this->SaveVTKBBGroup)
    this->SaveVTKBBGroup->Delete();
  if (this->EditBBMeshSeedGroup)
    this->EditBBMeshSeedGroup->Delete();
  if(this->SaveVTKFEMeshGroup)
    this->SaveVTKFEMeshGroup->Delete();
  if(this->FileBrowserDialog)
    this->FileBrowserDialog->Delete();
  if(this->DeleteObjectGroup)
    this->DeleteObjectGroup->Delete();
  if(this->FEMeshLaplacianSmooth)
    this->FEMeshLaplacianSmooth->Delete();
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }
  this->Superclass::CreateWidget();
  // add menu button with options for various Object
  // for surface
  if(!this->MainFrame)
    this->MainFrame = vtkKWFrameWithLabel::New();

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->MainFrame->SetLabelText("FE Mesh and Bounding Box Operations");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  this->ObjectMenuButton->SetParent(this->MainFrame->GetFrame());
  this->ObjectMenuButton->Create();
  this->ObjectMenuButton->SetBorderWidth(2);
  this->ObjectMenuButton->SetReliefToGroove();
  this->ObjectMenuButton->SetLabelText("Object :");
  this->ObjectMenuButton->SetPadX(2);
  this->ObjectMenuButton->SetPadY(2);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->ObjectMenuButton->GetWidgetName());
  this->ObjectMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "BB",this, "BBMenuCallback");
  this->ObjectMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "BB Mesh Seed",this, "BBMeshSeedMenuCallback");
  this->ObjectMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "FE Mesh",this, "FEMeshMenuCallback");
  // declare operations menu
  if(!this->OperationMenuButton)  
    this->OperationMenuButton = vtkKWMenuButtonWithLabel::New();
  this->OperationMenuButton->SetParent(this->MainFrame->GetFrame());
  this->OperationMenuButton->Create();
  this->OperationMenuButton->SetBorderWidth(2);
  this->OperationMenuButton->SetReliefToGroove();
  this->OperationMenuButton->SetLabelText("Operation :");
  this->OperationMenuButton->SetPadX(2);
  this->OperationMenuButton->SetPadY(2);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->OperationMenuButton->GetWidgetName());
  this->OperationMenuButton->SetEnabled(0);
  // declare operations menu
  if(!this->TypeMenuButton)  
    this->TypeMenuButton = vtkKWMenuButtonWithLabel::New();
  this->TypeMenuButton->SetParent(this->MainFrame->GetFrame());
  this->TypeMenuButton->Create();
  this->TypeMenuButton->SetBorderWidth(2);
  this->TypeMenuButton->SetReliefToGroove();
  this->TypeMenuButton->SetLabelText("Type :");
  this->TypeMenuButton->SetPadX(2);
  this->TypeMenuButton->SetPadY(2);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->TypeMenuButton->GetWidgetName());
  this->TypeMenuButton->SetEnabled(0);
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::Update()
{
  this->UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::BBMenuCallback()
{
  this->HideAllDialogBoxes();
  this->OperationMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->OperationMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Load",this, "LoadBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Save",this, "SaveBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Create",this, "CreateBBCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Delete",this, "DeleteBBCallback");
    this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Edit",this, "EditBBCallback");

  this->OperationMenuButton->SetEnabled(1);
  if(!this->BBViewProperties)
  {
    this->BBViewProperties = vtkKWMimxViewProperties::New();
    this->BBViewProperties->SetParent(this->GetParent());
    this->BBViewProperties->SetObjectList(this->BBoxList);
    this->BBViewProperties->SetMimxViewWindow(this->GetMimxViewWindow());
    this->BBViewProperties->Create();
  }

  if(this->MimxViewProperties)
  {
    this->GetApplication()->Script("pack forget %s", 
      this->MimxViewProperties->GetMainFrame()->GetWidgetName());
  }

  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->BBViewProperties->GetMainFrame()->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::LoadBBCallback()
{
  this->HideAllDialogBoxes();
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "VTK",this, "LoadVTKBBCallback");
  this->TypeMenuButton->SetEnabled(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateBBCallback()
{
  this->HideAllDialogBoxes();
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "From Bounds",this, "CreateBBFromBoundsCallback");
  this->TypeMenuButton->SetEnabled(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::LoadVTKBBCallback()
{
  this->HideAllDialogBoxes();
  if(!this->FileBrowserDialog)
  {
    this->FileBrowserDialog = vtkKWLoadSaveDialog::New();
  this->FileBrowserDialog->SetApplication(this->GetApplication());
  this->FileBrowserDialog->Create();
  }
  this->FileBrowserDialog->SetDefaultExtension(".vtk");
  this->FileBrowserDialog->SetFileTypes("{{VTK files} {.vtk}}");
  this->FileBrowserDialog->Invoke();
  if(this->FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
  {
    if(this->FileBrowserDialog->GetFileName())
    {
      const char *filename = this->FileBrowserDialog->GetFileName();
      vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
      reader->SetFileName(this->FileBrowserDialog->GetFileName());
      reader->Update();
      if(reader->GetOutput())
      {
        this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->DeepCopy(reader->GetOutput());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
        this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
        this->GetMimxViewWindow()->GetRenderWidget()->Render();
        this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
        this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->SetFilePath(
          this->FileBrowserDialog->GetFileName());
        this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->SetFileName(
          this->ExtractFileName(this->FileBrowserDialog->GetFileName()));
    this->BBViewProperties->AddObjectList();
      }
      reader->Delete();
    }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//---------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateBBFromBoundsCallback()
{
   if (this->CreateBBFromBounds)
  {
    this->CreateBBFromBounds->Delete();
  }

      this->CreateBBFromBounds = vtkKWMimxCreateBBFromBoundsGroup::New();
       this->CreateBBFromBounds->SetApplication(this->GetApplication());
       CreateBBFromBounds->SetParent(this->MainFrame->GetFrame());
       this->CreateBBFromBounds->SetSurfaceList(this->SurfaceList);
       this->CreateBBFromBounds->SetBBoxList(this->BBoxList);
        this->CreateBBFromBounds->SetMimxViewWindow(this->GetMimxViewWindow());
    this->CreateBBFromBounds->SetViewProperties(this->BBViewProperties);
    this->CreateBBFromBounds->SetMenuGroup(this);
    this->SetMenuButtonsEnabled(0);
       CreateBBFromBounds->Create();
      this->GetApplication()->Script(
        "pack %s -side bottom -anchor nw -expand y -padx 0 -pady 2", 
        CreateBBFromBounds->GetWidgetName()); 
 }
//----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::BBMeshSeedMenuCallback()
{
  this->HideAllDialogBoxes();
  this->OperationMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->OperationMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Create",this, "CreateBBMeshSeedCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Edit",this, "EditBBMeshSeedCallback");
  this->OperationMenuButton->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::FEMeshMenuCallback()
{
  this->HideAllDialogBoxes();
  this->OperationMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->OperationMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Load",this, "LoadFEMeshCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Save",this, "SaveFEMeshCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Create",this, "CreateFEMeshCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Delete",this, "DeleteFEMeshCallback");
  this->OperationMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Edit",this, "EditFEMeshCallback");

  this->OperationMenuButton->SetEnabled(1);

  if(!this->MimxViewProperties)
  {
    this->MimxViewProperties = vtkKWMimxViewProperties::New();
    this->MimxViewProperties->SetParent(this->GetParent());
    this->MimxViewProperties->SetObjectList(this->FEMeshList);
    this->MimxViewProperties->SetMimxViewWindow(this->GetMimxViewWindow());
    this->MimxViewProperties->Create();
  }
  if(this->BBViewProperties)
  {
    this->GetApplication()->Script("pack forget %s", 
      this->BBViewProperties->GetMainFrame()->GetWidgetName());
  }
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->MimxViewProperties->GetMainFrame()->GetWidgetName());

}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateBBMeshSeedCallback()
{
  this->HideAllDialogBoxes();
  if (this->CreateBBMeshSeed)
  {
 /*   this->GetApplication()->Script("pack forget %s", 
      this->CreateBBMeshSeed->GetMainFrame()->GetWidgetName());*/
    this->CreateBBMeshSeed->Delete();
  }
  this->CreateBBMeshSeed = vtkKWMimxCreateBBMeshSeedGroup::New();

  this->CreateBBMeshSeed->SetApplication(this->GetApplication());
  this->CreateBBMeshSeed->SetParent(this->MainFrame->GetFrame());
  this->CreateBBMeshSeed->SetBBoxList(this->BBoxList);
  this->CreateBBMeshSeed->SetMenuGroup(this);
  this->SetMenuButtonsEnabled(0);
  this->CreateBBMeshSeed->Create();
  this->GetApplication()->Script(
    "pack %s -side bottom -anchor nw -expand y -padx 0 -pady 2", 
    CreateBBMeshSeed->GetWidgetName());
//  vtkKWFrame::SafeDownCast(this->GetParent())->SetHeight(350);
  this->CreateBBMeshSeed->SetMimxViewWindow(this->GetMimxViewWindow());

}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::EditBBMeshSeedCallback()
{
  if (this->EditBBMeshSeedGroup)
  {
    //this->GetApplication()->Script("pack forget %s", 
    //  this->EditBBMeshSeedGroup->GetMainFrame()->GetWidgetName());
    this->EditBBMeshSeedGroup->Delete();
  }
  this->EditBBMeshSeedGroup = vtkKWMimxEditBBMeshSeedGroup::New();

  this->EditBBMeshSeedGroup->SetApplication(this->GetApplication());
  this->EditBBMeshSeedGroup->SetParent(this->MainFrame->GetFrame());
  this->EditBBMeshSeedGroup->SetBBoxList(this->BBoxList);
  this->EditBBMeshSeedGroup->SetMenuGroup(this);
  this->SetMenuButtonsEnabled(0);
  this->EditBBMeshSeedGroup->Create();
  this->GetApplication()->Script(
    "pack %s -side bottom -anchor nw -expand y -padx 0 -pady 2", 
    EditBBMeshSeedGroup->GetWidgetName());
  //  vtkKWFrame::SafeDownCast(this->GetParent())->SetHeight(350);
  this->EditBBMeshSeedGroup->SetMimxViewWindow(this->GetMimxViewWindow());
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::EditBBCallback()
{
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->SetEnabled(0);

  if (this->EditBB)
  {
    //this->GetApplication()->Script("pack forget %s", 
    //  this->EditBB->GetMainFrame()->GetWidgetName());
    this->EditBB->Delete();
  }
  this->EditBB = vtkKWMimxEditBBGroup::New();

  this->EditBB->SetApplication(this->GetApplication());
  this->EditBB->SetParent(this->MainFrame->GetFrame());
  this->EditBB->SetBBoxList(this->BBoxList);
  this->EditBB->Create();
  this->EditBB->SetMimxViewWindow(this->GetMimxViewWindow());
//  vtkKWFrame::SafeDownCast(this->GetParent())->SetHeight(350);
  this->EditBB->SetViewProperties(this->BBViewProperties);
  this->EditBB->SetMenuGroup(this);
  this->SetMenuButtonsEnabled(0);
  this->GetApplication()->Script(
    "pack %s -side bottom -anchor nw -expand y -padx 0 -pady 2", 
    EditBB->GetWidgetName());

}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::HideAllDialogBoxes()
{
  if(this->CreateBBFromBounds)
  {
    this->GetApplication()->Script("pack forget %s", 
      this->CreateBBFromBounds->GetMainFrame()->GetWidgetName());
  }
  if (this->CreateBBMeshSeed)
  {
    this->GetApplication()->Script("pack forget %s", 
      this->CreateBBMeshSeed->GetMainFrame()->GetWidgetName());
  }
  if (this->FEMeshFromBB)
  {
    this->GetApplication()->Script("pack forget %s", 
      this->FEMeshFromBB->GetMainFrame()->GetWidgetName());
  }
  if(this->EditBB)
  {
    this->GetApplication()->Script("pack forget %s", 
      this->EditBB->GetMainFrame()->GetWidgetName());
  }
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateFEMeshCallback()
{
  this->HideAllDialogBoxes();
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "From BB",this, "CreateFEMeshFromBBCallback");

  this->TypeMenuButton->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::CreateFEMeshFromBBCallback()
{
  this->HideAllDialogBoxes();
  if (this->FEMeshFromBB)
  {
 /*   this->FEMeshFromBB->UpdateObjectLists();
    this->GetApplication()->Script(
      "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
      this->FEMeshFromBB->GetMainFrame()->GetWidgetName());*/
    this->FEMeshFromBB->Delete();
  }

    this->FEMeshFromBB = vtkKWMimxCreateFEMeshFromBBGroup::New();
    this->FEMeshFromBB->SetApplication(this->GetApplication());
    this->FEMeshFromBB->SetParent(this->MainFrame->GetFrame());
    this->FEMeshFromBB->SetSurfaceList(this->SurfaceList);
    this->FEMeshFromBB->SetBBoxList(this->BBoxList);
    this->FEMeshFromBB->SetFEMeshList(this->FEMeshList);
    this->FEMeshFromBB->SetMimxViewWindow(this->GetMimxViewWindow());
  this->FEMeshFromBB->SetViewProperties(this->MimxViewProperties);
  this->FEMeshFromBB->SetMenuGroup(this);
  this->SetMenuButtonsEnabled(0);
    this->FEMeshFromBB->Create();
    this->GetApplication()->Script(
      "pack %s -side bottom -anchor nw -expand y -padx 0 -pady 2", 
      FEMeshFromBB->GetWidgetName());
 
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::SaveBBCallback()
{
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "VTK",this, "SaveVTKBBCallback");
  this->TypeMenuButton->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::SaveVTKBBCallback()
{
  if(this->SaveVTKBBGroup)
  {
    this->SaveVTKBBGroup->Delete();
  }
    this->SaveVTKBBGroup = vtkKWMimxSaveVTKBBGroup::New();
    this->SaveVTKBBGroup->SetParent(this->MainFrame->GetFrame());
    this->SaveVTKBBGroup->SetBBoxList(this->BBoxList);
    this->SaveVTKBBGroup->SetMenuGroup(this);
    this->SaveVTKBBGroup->Create();
  this->SetMenuButtonsEnabled(0);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->SaveVTKBBGroup->GetMainFrame()->GetWidgetName());
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::EditFEMeshCallback()
{
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "Smooth - Laplacian",this, "SmoothLaplacianFEMeshCallback");

  this->TypeMenuButton->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::LoadFEMeshCallback()
{
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "VTK",this, "LoadVTKFEMeshCallback");

  this->TypeMenuButton->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::SaveFEMeshCallback()
{
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->AddRadioButton(
    "VTK",this, "SaveVTKFEMeshCallback");

  this->TypeMenuButton->SetEnabled(1);
}
//-----------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::LoadVTKFEMeshCallback()
{
  if(!this->FileBrowserDialog)
  {
    this->FileBrowserDialog = vtkKWLoadSaveDialog::New() ;
//    this->FileBrowserDialog->LoadDialogOn();
    this->FileBrowserDialog->SetApplication(this->GetApplication());
    this->FileBrowserDialog->Create();
  }
  this->FileBrowserDialog->SetTitle ("Save VTK Unstructured Grid File Format");
  this->FileBrowserDialog->SetFileTypes ("{{VTK files} {.vtk}}");
  this->FileBrowserDialog->SetDefaultExtension (".vtk");

  this->FileBrowserDialog->Invoke();
  if(FileBrowserDialog->GetStatus() == vtkKWDialog::StatusOK)
  {
    if(FileBrowserDialog->GetFileName())
    {
      const char *filename = FileBrowserDialog->GetFileName();
      vtkUnstructuredGridReader *reader = vtkUnstructuredGridReader::New();
      reader->SetFileName(FileBrowserDialog->GetFileName());
      reader->Update();
      if(reader->GetOutput())
      {
        this->FEMeshList->AppendItem(vtkMimxUnstructuredGridActor::New());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->FEMeshList->GetItem(
          this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->DeepCopy(reader->GetOutput());
        vtkMimxUnstructuredGridActor::SafeDownCast(this->FEMeshList->GetItem(
          this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->Modified();
        this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
          this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->GetActor());
        this->GetMimxViewWindow()->GetRenderWidget()->Render();
        this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
        this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->SetFilePath(
          FileBrowserDialog->GetFileName());
        this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->SetFileName(
          this->ExtractFileName(FileBrowserDialog->GetFileName()));
        this->MimxViewProperties->AddObjectList();
        /*       if(!this->MimxViewWindow->UnstructuredGridWidget){
        this->MimxViewWindow->UnstructuredGridWidget = vtkMimxUnstructuredGridWidget::New();
        this->MimxViewWindow->UnstructuredGridWidget->SetInteractor(
        this->GetMimxViewWindow()->GetRenderWidget()->GetRenderWindowInteractor());
        this->MimxViewWindow->UnstructuredGridWidget->SetUGrid(
        vtkUnstructuredGrid::SafeDownCast(this->BBoxList->GetItem(
        this->BBoxList->GetNumberOfItems()-1)->GetDataSet()));
        this->MimxViewWindow->UnstructuredGridWidget->Execute();
        *///         this->MimxViewWindow->UnstructuredGridWidget->SetEnabled(1);
      }
      reader->Delete();
    }
  }
}
//---------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::DeleteBBCallback()
{
  if(this->DeleteObjectGroup)
  {
    this->DeleteObjectGroup->Delete();
  }
  this->DeleteObjectGroup = vtkKWMimxDeleteObjectGroup::New();
  this->DeleteObjectGroup->SetParent(this->MainFrame->GetFrame());
  this->DeleteObjectGroup->SetBBoxList(this->BBoxList);
  this->DeleteObjectGroup->SetMenuGroup(this);
  this->DeleteObjectGroup->SetViewProperties(this->BBViewProperties);
  this->DeleteObjectGroup->Create();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->SetEnabled(0);
  this->SetMenuButtonsEnabled(0);
  this->DeleteObjectGroup->GetMainFrame()->SetLabelText("Delete Bounding Box");
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->DeleteObjectGroup->GetMainFrame()->GetWidgetName());
}
//---------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::DeleteFEMeshCallback()
{
  if(this->DeleteObjectGroup)
  {
    this->DeleteObjectGroup->Delete();
  }
  this->DeleteObjectGroup = vtkKWMimxDeleteObjectGroup::New();
  this->DeleteObjectGroup->SetParent(this->MainFrame->GetFrame());
  this->DeleteObjectGroup->SetFEMeshList(this->FEMeshList);
  this->DeleteObjectGroup->SetMenuGroup(this);
  this->DeleteObjectGroup->SetViewProperties(this->MimxViewProperties);
  this->DeleteObjectGroup->Create();
  this->TypeMenuButton->GetWidget()->SetValue("");
  this->TypeMenuButton->GetWidget()->GetMenu()->DeleteAllItems();
  this->TypeMenuButton->SetEnabled(0);
  this->SetMenuButtonsEnabled(0);
  this->DeleteObjectGroup->GetMainFrame()->SetLabelText("Delete FE Mesh");
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->DeleteObjectGroup->GetMainFrame()->GetWidgetName());
}
//---------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::SaveVTKFEMeshCallback()
{
  if(this->SaveVTKFEMeshGroup)
  {
    this->SaveVTKFEMeshGroup->Delete();
  }
  this->SaveVTKFEMeshGroup = vtkKWMimxSaveVTKFEMeshGroup::New();
  this->SaveVTKFEMeshGroup->SetParent(this->MainFrame->GetFrame());
  this->SaveVTKFEMeshGroup->SetFEMeshList(this->FEMeshList);
  this->SaveVTKFEMeshGroup->SetMenuGroup(this);
  this->SaveVTKFEMeshGroup->Create();
  this->SetMenuButtonsEnabled(0);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
                   this->SaveVTKFEMeshGroup->GetMainFrame()->GetWidgetName());
}
//------------------------------------------------------------------------------------------
void vtkKWMimxFEMeshMenuGroup::SmoothLaplacianFEMeshCallback()
{
  if(this->FEMeshLaplacianSmooth)
  {
    this->FEMeshLaplacianSmooth->Delete();
  }
  this->FEMeshLaplacianSmooth = vtkKWMimxEditFEMeshLaplacianSmoothGroup::New();
  this->FEMeshLaplacianSmooth->SetParent(this->MainFrame->GetFrame());
  this->FEMeshLaplacianSmooth->SetSurfaceList(this->SurfaceList);
  this->FEMeshLaplacianSmooth->SetFEMeshList(this->FEMeshList);
  this->FEMeshLaplacianSmooth->SetBBoxList(this->BBoxList);
  this->FEMeshLaplacianSmooth->SetMenuGroup(this);
  this->FEMeshLaplacianSmooth->SetMimxViewWindow(this->GetMimxViewWindow());
  this->FEMeshLaplacianSmooth->Create();
  this->SetMenuButtonsEnabled(0);
  this->GetApplication()->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 5", 
    this->FEMeshLaplacianSmooth->GetMainFrame()->GetWidgetName());
}
