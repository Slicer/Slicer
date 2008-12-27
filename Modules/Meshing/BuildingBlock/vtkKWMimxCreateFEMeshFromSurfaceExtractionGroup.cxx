/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.23.4.2 $

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

#include "vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxTraceContourWidget.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxExtrudePolyData.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkPolyDataMapper.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkProperty.h"
#include "vtkCellTypes.h"
#include "vtkGeometryFilter.h"
#include "vtkIdList.h"
#include "vtkPointData.h"
#include "vtkMimxMeshActor.h"

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
#include "vtkKWCheckButtonWithLabel.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"
#include "vtkLinkedListWrapper.h"
#include "vtkMath.h"
#include "vtkIntArray.h"
#include "vtkRenderer.h"

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
vtkStandardNewMacro(vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup, "$Revision: 1.23.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup()
{
  this->MimxMainWindow = NULL;
  this->ObjectListComboBox = NULL;
  this->ComponentFrame = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::~vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
  if (this->ComponentFrame)
    this->ComponentFrame->Delete();
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::CreateWidget()
{
  if(this->IsCreated())
    {
    vtkErrorMacro("class already created");
    return;
    }

  this->Superclass::CreateWidget();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Extract Surface");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ComponentFrame)    
    this->ComponentFrame = vtkKWFrameWithLabel::New();
  this->ComponentFrame->SetParent(this->MainFrame);
  this->ComponentFrame->Create();
  this->ComponentFrame->SetLabelText("Mesh");
  this->ComponentFrame->CollapseFrame( );
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x", 
    this->ComponentFrame->GetWidgetName());
  
  if(!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();  
  this->ObjectListComboBox->SetParent(this->ComponentFrame->GetFrame());
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  this->ObjectListComboBox->SetLabelWidth(15);
  this->ObjectListComboBox->SetLabelText("Mesh : ");
  this->ObjectListComboBox->GetWidget()->ReadOnlyOn();

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6 -fill x", 
    this->ObjectListComboBox->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "CreateFEMeshFromSurfaceExtractionApplyCallback");
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
    this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "CreateFEMeshFromSurfaceExtractionCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand n -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::Update()
{
  this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::UpdateEnableState()
{
  this->UpdateObjectLists();
  this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::CreateFEMeshFromSurfaceExtractionApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
    {
    callback->ErrorMessage("FE Mesh not selected");
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

  vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(this->FEMeshList
                                                              ->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
  vtkGeometryFilter *geofil = vtkGeometryFilter::New();
  geofil->SetInput(ugrid);
  geofil->Update();
  vtkCellTypes *celltypes = vtkCellTypes::New();
  geofil->GetOutput()->GetCellTypes(celltypes);
  int i;
  for(i =0; i < celltypes->GetNumberOfTypes(); i++)
    {
    if(celltypes->GetCellType(i) != 9)
      {
      callback->ErrorMessage("Extracted mesh should contain only quadrilaterals");
      celltypes->Delete();
      geofil->Delete();
      return 0;
      }
    }
  vtkUnstructuredGrid *unstgrid = vtkUnstructuredGrid::New();

  unstgrid->SetPoints(geofil->GetOutput()->GetPoints());
  unstgrid->Allocate(geofil->GetOutput()->GetNumberOfCells());

  vtkIdList * unstIdlist = vtkIdList ::New();

  for (i = 0; i<geofil->GetOutput()->GetNumberOfCells(); i++)
    {
    geofil->GetOutput()->GetCellPoints(i,unstIdlist);
    unstgrid->InsertNextCell(9,unstIdlist);
    unstIdlist->Initialize();
    }
  unstIdlist->Delete();
  geofil->Delete();
                
  vtkMimxMeshActor *meshActor = vtkMimxMeshActor::New();
  this->FEMeshList->AppendItem(meshActor);
  meshActor->SetDataSet( unstgrid );
  meshActor->SetRenderer( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer() );
  meshActor->SetInteractor( this->GetMimxMainWindow()->GetRenderWidget()->GetRenderWindowInteractor() );
                                
  unstgrid->Delete();
  if(ugrid->GetPointData()->GetArray("Node_Numbers"))
    {
    vtkIntArray *nodenumbers = vtkIntArray::New();
    nodenumbers->DeepCopy(ugrid->GetPointData()->GetArray("Node_Numbers"));
    nodenumbers->SetName("Node_Numbers");
    vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                     this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->
      GetPointData()->AddArray(nodenumbers);
    nodenumbers->Delete();
    }
  this->Count++;
  vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
                                   this->FEMeshList->GetNumberOfItems()-1))->SetObjectName("Extract_Surface_",Count);
  //vtkMimxMeshActor::SafeDownCast(this->FEMeshList->GetItem(
  //      this->FEMeshList->GetNumberOfItems()-1))->GetDataSet()->Modified();
  //this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(
  //      this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1)->GetActor());
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList(
    this->FEMeshList->GetItem(this->FEMeshList->GetNumberOfItems()-1));
                
  this->GetMimxMainWindow()->SetStatusText("Created Mesh");
                
  return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::CreateFEMeshFromSurfaceExtractionCancelCallback()
{
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//----------------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::UpdateObjectLists()
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
//------------------------------------------------------------------------------
void vtkKWMimxCreateFEMeshFromSurfaceExtractionGroup::CreateFEMeshFromSurfaceExtractionDoneCallback()
{
  if(this->CreateFEMeshFromSurfaceExtractionApplyCallback())
    this->CreateFEMeshFromSurfaceExtractionCancelCallback();
}
//---------------------------------------------------------------------------------
