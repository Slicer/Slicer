/*=========================================================================

  Module:    $RCSfile: vtkKWMimxCreateFEMeshFromBBGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxCreateFEMeshFromBBGroup.h"
#include "vtkKWMimxViewWindow.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxUnstructuredGridFromBoundingBox.h"

//#include "vtkUnstructuredGridWriter.h"

#include "vtkKWApplication.h"
#include "vtkKWFileBrowserDialog.h"
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

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateFEMeshFromBBGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateFEMeshFromBBGroup, "$Revision: 1.3 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshFromBBGroup::vtkKWMimxCreateFEMeshFromBBGroup()
{
  this->SurfaceListComboBox = NULL;
  this->BBListComboBox = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshFromBBGroup::~vtkKWMimxCreateFEMeshFromBBGroup()
{
  if(this->SurfaceListComboBox)
     this->SurfaceListComboBox->Delete();
  if(this->BBListComboBox)
    this->BBListComboBox->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  if(!this->SurfaceListComboBox)  
  {
     this->SurfaceListComboBox = vtkKWComboBoxWithLabel::New();
  }
  if(!this->BBListComboBox)  
  {
    this->BBListComboBox = vtkKWComboBoxWithLabel::New();
  }
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->MainFrame->SetLabelText("Create FE Mesh From Bounds");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  SurfaceListComboBox->SetParent(this->MainFrame->GetFrame());
  SurfaceListComboBox->Create();
  SurfaceListComboBox->SetLabelText("Surface Selection : ");
  SurfaceListComboBox->GetWidget()->ReadOnlyOn();
  SurfaceListComboBox->GetWidget()->SetBalloonHelpString("Surface onto which the resulting FE Mesh projected");
  int i;
  for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    SurfaceListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    SurfaceListComboBox->GetWidgetName());

  BBListComboBox->SetParent(this->MainFrame->GetFrame());
  BBListComboBox->Create();
  BBListComboBox->SetLabelText("Bounding box Selection : ");
  BBListComboBox->GetWidget()->ReadOnlyOn();
  BBListComboBox->GetWidget()->SetBalloonHelpString("Bounding box from which F E Mesh is generated");

  for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
    BBListComboBox->GetWidget()->AddValue(
      this->BBoxList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    BBListComboBox->GetWidgetName());

  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "CreateFEMeshFromBBCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 20 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateFEMeshFromBBCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::CreateFEMeshFromBBCallback()
{
  if(strcmp(this->SurfaceListComboBox->GetWidget()->GetValue(),"") && 
    strcmp(this->BBListComboBox->GetWidget()->GetValue(),""))
  {
    vtkKWComboBox *combobox = this->SurfaceListComboBox->GetWidget();
    const char *name = combobox->GetValue();
    vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

    combobox = this->BBListComboBox->GetWidget();
    name = combobox->GetValue();
    vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
      this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
    if(!ugrid->GetCellData()->GetScalars("Mesh_Seed"))
      return;
    vtkMimxUnstructuredGridFromBoundingBox *ugridfrombbox = 
      vtkMimxUnstructuredGridFromBoundingBox::New();
    ugridfrombbox->SetBoundingBox(ugrid);
    ugridfrombbox->SetSurface(polydata);
    ugridfrombbox->Update();
    if (ugridfrombbox->GetOutput())
    {
      this->FEMeshList->AppendItem(vtkMimxUnstructuredGridActor::New());
      vtkMimxUnstructuredGridActor::SafeDownCast(this->FEMeshList->GetItem(
        this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->
        DeepCopy(ugridfrombbox->GetOutput());
      this->Count++;
      vtkMimxUnstructuredGridActor::SafeDownCast(this->FEMeshList->GetItem(
        this->FEMeshList->GetNumberOfItems()-1))->SetObjectName("FEMeshFromBB_",Count);
      vtkMimxUnstructuredGridActor::SafeDownCast(this->FEMeshList->GetItem(
        this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->Modified();
    this->ViewProperties->AddObjectList();
      this->GetMimxViewWindow()->GetRenderWidget()->AddViewProp(
        this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->GetActor());
      this->GetMimxViewWindow()->GetRenderWidget()->Render();
      this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
      //vtkUnstructuredGridWriter *writer = vtkUnstructuredGridWriter::New();
      //writer->SetFileName("Debug.vtk");
      //writer->SetInput(ugridfrombbox->GetOutput());
      //writer->Write();
      //writer->Delete();
    }
    ugridfrombbox->Delete();
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::CreateFEMeshFromBBCancelCallback()
{
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromBBGroup::UpdateObjectLists()
{
  this->SurfaceListComboBox->GetWidget()->DeleteAllValues();
  this->BBListComboBox->GetWidget()->DeleteAllValues();

  int i;
  for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  {
    SurfaceListComboBox->GetWidget()->AddValue(
      this->SurfaceList->GetItem(i)->GetFileName());
  }

  for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
    BBListComboBox->GetWidget()->AddValue(
      this->BBoxList->GetItem(i)->GetFileName());
  }
}
//------------------------------------------------------------------------------
