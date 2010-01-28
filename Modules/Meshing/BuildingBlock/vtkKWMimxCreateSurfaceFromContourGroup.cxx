/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateSurfaceFromContourGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.24.4.2 $

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

#include "vtkKWMimxCreateSurfaceFromContourGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxTraceContourWidget.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkMimxSplitUnstructuredHexahedronGridCell.h"
#include "vtkMimxAddUnstructuredHexahedronGridCell.h"
#include "vtkMimxDeleteUnstructuredHexahedronGridCell.h"
#include "vtkIdList.h"
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
#include "vtkIntArray.h"
#include "vtkMimxSelectSurface.h"
#include "vtkMimxMeshActor.h"

#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWMimxMainUserInterfacePanel.h"


#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateSurfaceFromContourGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateSurfaceFromContourGroup, "$Revision: 1.24.4.2 $");

//----------------------------------------------------------------------------
vtkKWMimxCreateSurfaceFromContourGroup::vtkKWMimxCreateSurfaceFromContourGroup()
{
        //this->MainFrame = NULL;
//  this->FEMeshList = vtkLinkedListWrapper::New();
  this->MimxMainWindow = NULL;
  this->ObjectListComboBox = NULL;
  this->VtkInteractionButton = vtkKWRadioButton::New();
  this->TraceContourButton = NULL;
  this->TraceContourWidget = NULL;
  this->InteractionFrame = NULL;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateSurfaceFromContourGroup::~vtkKWMimxCreateSurfaceFromContourGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
   if(this->TraceContourButton)
          this->TraceContourButton->Delete();
  if(this->TraceContourWidget)
          this->TraceContourWidget->Delete();
        if (this->InteractionFrame)
          this->InteractionFrame->Delete();
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::SelectionChangedCallback(const char* vtkNotUsed(dummy))
{
  if(this->TraceContourButton->GetSelectedState())
  {
          this->TraceContourButton->SetSelectedState(1);
          this->CreateSurfaceFromContourTraceContourCallback();
  }
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Surface by tracing");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if(!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  this->ObjectListComboBox->SetParent(this->MainFrame);
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  this->ObjectListComboBox->SetLabelText("Mesh : ");
  this->ObjectListComboBox->SetLabelWidth( 10 );
  this->ObjectListComboBox->GetWidget()->ReadOnlyOn();

  int i;
  for (i = 0; i < this->FEMeshList->GetNumberOfItems(); i++)
  {
          ObjectListComboBox->GetWidget()->AddValue(
                  this->FEMeshList->GetItem(i)->GetFileName());
   }
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6 -fill x", 
    this->ObjectListComboBox->GetWidgetName());

  if(!this->InteractionFrame)
    this->InteractionFrame = vtkKWFrameWithLabel::New();  
  this->InteractionFrame->SetParent(this->MainFrame);
  this->InteractionFrame->Create();
  this->InteractionFrame->SetLabelText("Interaction");

  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    this->InteractionFrame->GetWidgetName());
    
        if(!this->TraceContourButton)
                this->TraceContourButton = vtkKWRadioButton::New();
  this->TraceContourButton->SetParent(this->InteractionFrame->GetFrame());
  this->TraceContourButton->Create();
  this->TraceContourButton->SetCommand(this, "CreateSurfaceFromContourTraceContourCallback");
  this->TraceContourButton->SetText("Trace ");
  this->TraceContourButton->SetImageToPredefinedIcon(vtkKWIcon::IconContourTool);
  this->TraceContourButton->IndicatorVisibilityOff();
  this->TraceContourButton->SetBalloonHelpString("Pick points for tracing");
  this->TraceContourButton->SetValue("Trace");
  this->TraceContourButton->SetCompoundModeToLeft();
 
  this->GetApplication()->Script(
    "pack %s -side left -anchor nw -expand 0 -padx 6 -pady 6", 
    this->TraceContourButton->GetWidgetName());

  this->VtkInteractionButton->SetParent(this->InteractionFrame->GetFrame());
  this->VtkInteractionButton->Create();
  this->VtkInteractionButton->SetText("VTK");
  this->VtkInteractionButton->SetCommand(this, "CreateSurfaceFromContourVtkInteractionCallback");
  this->VtkInteractionButton->SetImageToPredefinedIcon(vtkKWIcon::IconRotate);
  this->VtkInteractionButton->IndicatorVisibilityOff();
  this->VtkInteractionButton->SetBalloonHelpString("Default VTK Interaction");
  this->VtkInteractionButton->SetVariableName(this->TraceContourButton->GetVariableName());
  this->VtkInteractionButton->SetValue("VtkInteraction");
  this->VtkInteractionButton->SetCompoundModeToLeft();
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand 1 -padx 6 -pady 6", 
    this->VtkInteractionButton->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  //this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetCommand(this, "CreateSurfaceFromContourApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand n -padx 20 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  //this->CancelButton->SetText("Cancel");
  this->CancelButton->SetCommand(this, "CreateSurfaceFromContourCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand n -padx 20 -pady 6", 
    this->CancelButton->GetWidgetName());

  this->VtkInteractionButton->SelectedStateOn();

}
//----------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateSurfaceFromContourGroup::CreateSurfaceFromContourApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!this->TraceContourButton->GetSelectedState())
  {
          callback->ErrorMessage("Select tracing interaction button");
          return 0;
  }
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("Object to be traced not selected");
          return 0;
  }
  if(this->TraceContourWidget)
  {
          if(this->TraceContourWidget->GetEnabled())
          {
                  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                  const char *name = combobox->GetValue();

                  int num = combobox->GetValueIndex(name);
                  if(num < 0 || num > combobox->GetNumberOfValues()-1)
                  {
                          callback->ErrorMessage("Choose valid FE Mesh");
                          combobox->SetValue("");
                          return 0;
                  }
                  vtkMimxUnstructuredGridActor *ugridactor = vtkMimxUnstructuredGridActor::
                          SafeDownCast(this->FEMeshList->GetItem(combobox->GetValueIndex(name)));
                  vtkUnstructuredGrid *ugrid = ugridactor->GetDataSet();
                  vtkMimxSelectSurface *selectsurface = vtkMimxSelectSurface::New();
                  selectsurface->SetInput(ugrid);
                  selectsurface->SetLoop(this->TraceContourWidget->GetContourIdList());
                  callback->SetState(0);
                  selectsurface->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
                  selectsurface->Update();
                  if(!callback->GetState())
                  {
                          this->SurfaceList->AppendItem(vtkMimxSurfacePolyDataActor::New());
                          this->SurfaceList->GetItem(this->SurfaceList->GetNumberOfItems()-1)->
                                  SetDataType(ACTOR_POLYDATA_SURFACE);
                                  this->Count++;
                          vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList->GetItem(
                                  this->SurfaceList->GetNumberOfItems()-1))->SetObjectName(
                                  "SurfaceFromContour_",Count);
                          vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList->GetItem(
                                  this->SurfaceList->GetNumberOfItems()-1))->GetDataSet()->DeepCopy(
                                  selectsurface->GetOutput());
                          vtkMimxSurfacePolyDataActor::SafeDownCast(this->SurfaceList->GetItem(
                                  this->SurfaceList->GetNumberOfItems()-1))->GetDataSet()->Modified();
                          this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(this->
                                  SurfaceList->GetItem(this->SurfaceList->GetNumberOfItems()-1)->GetActor());
                          this->GetMimxMainWindow()->GetRenderWidget()->Render();
                          this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
                          this->GetMimxMainWindow()->GetViewProperties()->AddObjectList(
                                  this->SurfaceList->GetItem(this->SurfaceList->GetNumberOfItems()-1));
                          selectsurface->RemoveObserver(callback);
                          selectsurface->Delete();
                          this->VtkInteractionButton->SelectedStateOn();
                          
                          this->GetMimxMainWindow()->SetStatusText("Created Surface");
                          
                          return 1;
                  }
                  selectsurface->RemoveObserver(callback);
                  selectsurface->Delete();
                  this->VtkInteractionButton->SelectedStateOn();
                  return 0;
          }
  }
  this->VtkInteractionButton->SelectedStateOn();
  return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::CreateSurfaceFromContourCancelCallback()
{
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        this->CreateSurfaceFromContourVtkInteractionCallback();
        this->MenuGroup->SetMenuButtonsEnabled(1);
          this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::CreateSurfaceFromContourTraceContourCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                const char *name = combobox->GetValue();
                int num = combobox->GetValueIndex(name);
                if(num < 0 || num > combobox->GetNumberOfValues()-1)
                {
                        callback->ErrorMessage("Choose valid FE Mesh");
                        combobox->SetValue("");
                        return ;
                }
                vtkUnstructuredGrid *ugrid = vtkMimxMeshActor::SafeDownCast(
                        this->FEMeshList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
                if(this->TraceContourWidget)
                {
                        if(this->TraceContourWidget->GetEnabled())
                        {
                                this->TraceContourWidget->SetEnabled(0);
                        }
                        this->TraceContourWidget->Delete();
                        this->TraceContourWidget = NULL;
                }
                this->TraceContourWidget = vtkMimxTraceContourWidget::New();
                this->TraceContourWidget->SetInteractor(this->GetMimxMainWindow()->
                        GetRenderWidget()->GetRenderWindowInteractor());
                this->TraceContourWidget->SetInput(ugrid);
                this->TraceContourWidget->SetInputActor(this->FEMeshList->GetItem(
                        combobox->GetValueIndex(name))->GetActor());
                this->TraceContourWidget->SetEnabled(1);
        }
}
//----------------------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::CreateSurfaceFromContourVtkInteractionCallback()
{
  if(this->TraceContourWidget)
  {
          if(this->TraceContourWidget->GetEnabled())
          {
                  this->TraceContourWidget->SetEnabled(0);
          }
  }
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxCreateSurfaceFromContourGroup::UpdateObjectLists()
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
void vtkKWMimxCreateSurfaceFromContourGroup::CreateSurfaceFromContourDoneCallback()
{
        if(this->CreateSurfaceFromContourApplyCallback())
                this->CreateSurfaceFromContourCancelCallback();
}
//---------------------------------------------------------------------------------
