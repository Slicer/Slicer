/*=========================================================================

  Module:    $RCSfile: vtkKWMimxEditFEMeshLaplacianSmoothGroup.cxx,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include "vtkKWMimxEditFEMeshLaplacianSmoothGroup.h"
#include "vtkKWMimxViewWindow.h"

#include "vtkLinkedListWrapper.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkProperty.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxSmoothUnstructuredGridFilter.h"
#include "vtkMimxRecalculateInteriorNodes.h"

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
#include "vtkKWEntryWithLabel.h"
#include "vtkKWCheckButton.h"

#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD       1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxEditFEMeshLaplacianSmoothGroup);
vtkCxxRevisionMacro(vtkKWMimxEditFEMeshLaplacianSmoothGroup, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkKWMimxEditFEMeshLaplacianSmoothGroup::vtkKWMimxEditFEMeshLaplacianSmoothGroup()
{
  this->FEMeshListComboBox = NULL;
  this->BBListComboBox = NULL;
  this->SurfaceListComboBox = NULL;
  this->SmoothIterations = vtkKWEntryWithLabel::New();
  this->RecalculateIterations = vtkKWEntryWithLabel::New();
  this->RecalculateCheckButton = vtkKWCheckButton::New();
}

//----------------------------------------------------------------------------
vtkKWMimxEditFEMeshLaplacianSmoothGroup::~vtkKWMimxEditFEMeshLaplacianSmoothGroup()
{
  if(this->FEMeshListComboBox)
     this->FEMeshListComboBox->Delete();
  if(this->BBListComboBox)
    this->BBListComboBox->Delete();
  if(this->SurfaceListComboBox)
    this->SurfaceListComboBox->Delete();
  this->SmoothIterations->Delete();
  this->RecalculateIterations->Delete();
  this->RecalculateCheckButton->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  if(!this->FEMeshListComboBox)  
  {
     this->FEMeshListComboBox = vtkKWComboBoxWithLabel::New();
  }
  if(!this->BBListComboBox)  
  {
    this->BBListComboBox = vtkKWComboBoxWithLabel::New();
  }

  if(!this->SurfaceListComboBox)  
  {
    this->SurfaceListComboBox = vtkKWComboBoxWithLabel::New();
  }

  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  this->MainFrame->SetLabelText("Create FE Mesh From Bounds");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    this->MainFrame->GetWidgetName());

  FEMeshListComboBox->SetParent(this->MainFrame->GetFrame());
  FEMeshListComboBox->Create();
  FEMeshListComboBox->SetLabelText("F E Mesh Selection : ");
  FEMeshListComboBox->GetWidget()->ReadOnlyOn();
  FEMeshListComboBox->GetWidget()->SetBalloonHelpString(
    "F E Mesh whose surface nodes are moved using laplacian smoothing");
  int i;
  for (i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
    FEMeshListComboBox->GetWidget()->AddValue(
      this->FEMeshList->GetItem(i)->GetFileName());
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    FEMeshListComboBox->GetWidgetName());

  // surface list
  SurfaceListComboBox->SetParent(this->MainFrame->GetFrame());
  SurfaceListComboBox->Create();
  SurfaceListComboBox->SetLabelText("Surface Selection : ");
  SurfaceListComboBox->GetWidget()->ReadOnlyOn();
  SurfaceListComboBox->GetWidget()->SetBalloonHelpString(
    "Surface from which F E mesh is created");
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
  BBListComboBox->GetWidget()->SetBalloonHelpString(
    "Bounding box from which F E Mesh is generated");

  for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  {
    vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
      SafeDownCast(this->BBoxList->GetItem(i));
    vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
    if(ugrid->GetCellData()->GetArray("Mesh_Seed"))
    {
    BBListComboBox->GetWidget()->AddValue(
      this->BBoxList->GetItem(i)->GetFileName());
    }
  }

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6", 
    BBListComboBox->GetWidgetName());

  // number of iterations for laplacian smoothing
  this->SmoothIterations->SetParent(this->MainFrame->GetFrame());
  this->SmoothIterations->Create();
  this->SmoothIterations->GetWidget()->SetWidth(4);
  this->SmoothIterations->GetWidget()->SetValueAsInt(1);
  this->SmoothIterations->SetLabelText("Number of Smoothing Iterations : ");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 1 -pady 6", 
    this->SmoothIterations->GetWidgetName());
 
  // number of iterations for recalculating interior nodes
  this->RecalculateIterations->SetParent(this->MainFrame->GetFrame());
  this->RecalculateIterations->Create();
  this->RecalculateIterations->GetWidget()->SetWidth(4);
  this->RecalculateIterations->GetWidget()->SetValueAsInt(10);
  this->RecalculateIterations->SetLabelText("Number of Recalculation Iterations : ");

  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 1 -pady 6", 
    this->RecalculateIterations->GetWidgetName());
  // check button for enabling and disabling iteration
  this->RecalculateCheckButton->SetParent(this->MainFrame->GetFrame());
  this->RecalculateCheckButton->Create();
  this->RecalculateCheckButton->SetCommand(this, "RecalculateCheckButtonCallback");
  this->RecalculateCheckButton->Select();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 1 -pady 6", 
    this->RecalculateCheckButton->GetWidgetName());
  
  this->DoneButton->SetParent(this->MainFrame->GetFrame());
  this->DoneButton->Create();
  this->DoneButton->SetText("Done");
  this->DoneButton->SetCommand(this, "EditFEMeshLaplacianSmoothDoneCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 1 -pady 6", 
    this->DoneButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame->GetFrame());
  this->CancelButton->Create();
  this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "EditFEMeshLaplacianSmoothCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 2 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::UpdateEnableState()
{
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::EditFEMeshLaplacianSmoothDoneCallback()
{
  if(strcmp(this->SurfaceListComboBox->GetWidget()->GetValue(),"") && 
    strcmp(this->FEMeshListComboBox->GetWidget()->GetValue(),""))
  {
    vtkKWComboBox *combobox = this->SurfaceListComboBox->GetWidget();
    const char *name = combobox->GetValue();
    vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList
     ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

    combobox = this->FEMeshListComboBox->GetWidget();
    name = combobox->GetValue();
    vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
      this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
     vtkMimxSmoothUnstructuredGridFilter *smooth = 
      vtkMimxSmoothUnstructuredGridFilter::New();
   smooth->SetSource(polydata);
   smooth->SetInput(ugrid);
  smooth->SetNumberOfIterations(this->SmoothIterations->GetWidget()->GetValueAsInt());
    smooth->Update();
    if (smooth->GetOutput())
    {
    ugrid->GetPoints()->DeepCopy(smooth->GetOutput()->GetPoints());
    ugrid->GetPoints()->Modified();
    this->GetMimxViewWindow()->GetRenderWidget()->Render();
    this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
     }
    smooth->Delete();
  }
  else  return;
  if(this->RecalculateCheckButton->GetSelectedState())
  {
    if(strcmp(this->BBListComboBox->GetWidget()->GetValue(),""))
    {
      vtkKWComboBox *combobox = this->BBListComboBox->GetWidget();
      combobox = this->BBListComboBox->GetWidget();
      const char *name = combobox->GetValue();
      vtkUnstructuredGrid *bbox = vtkMimxUnstructuredGridActor::SafeDownCast(
        this->BBoxList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

      combobox = this->FEMeshListComboBox->GetWidget();
      name = combobox->GetValue();
      vtkUnstructuredGrid *ugrid = vtkMimxUnstructuredGridActor::SafeDownCast(
        this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();

      vtkMimxRecalculateInteriorNodes *recalc = 
        vtkMimxRecalculateInteriorNodes::New();
      recalc->SetBoundingBox(bbox);
      recalc->SetInput(ugrid);
      recalc->SetNumberOfIterations(this->RecalculateIterations->GetWidget()->GetValueAsInt());
      recalc->Update();
      if (recalc->GetOutput())
      {
        ugrid->GetPoints()->DeepCopy(recalc->GetOutput()->GetPoints());
        ugrid->GetPoints()->Modified();
        this->GetMimxViewWindow()->GetRenderWidget()->Render();
        this->GetMimxViewWindow()->GetRenderWidget()->ResetCamera();
      }
      recalc->Delete();
    }
  }
}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::EditFEMeshLaplacianSmoothCancelCallback()
{
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::UpdateObjectLists()
{
  //this->SurfaceListComboBox->GetWidget()->DeleteAllValues();
  //this->BBListComboBox->GetWidget()->DeleteAllValues();

  //int i;
  //for (i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
  //{
  //  SurfaceListComboBox->GetWidget()->AddValue(
  //    this->SurfaceList->GetItem(i)->GetFileName());
  //}

  //for (i = 0; i < this->BBoxList->GetNumberOfItems(); i++)
  //{
  //  BBListComboBox->GetWidget()->AddValue(
  //    this->BBoxList->GetItem(i)->GetFileName());
  //}
}
//------------------------------------------------------------------------------
void vtkKWMimxEditFEMeshLaplacianSmoothGroup::RecalculateCheckButtonCallback(int State)
{
  this->RecalculateIterations->SetEnabled(State);
}
