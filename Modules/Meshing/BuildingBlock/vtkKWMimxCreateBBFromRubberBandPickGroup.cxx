/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateBBFromRubberBandPickGroup.cxx,v $
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

#include "vtkKWMimxCreateBBFromRubberBandPickGroup.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkMimxCreateBuildingBlockFromPickWidget.h"
#include "vtkMimxErrorCallback.h"
#include "vtkKWMimxMainNotebook.h"

#include "vtkActor.h"
#include "vtkCellData.h"
#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkPolyData.h"
#include "vtkMimxUnstructuredGridActor.h"
#include "vtkUnstructuredGrid.h"
#include "vtkRenderer.h"

#include "vtkKWApplication.h"
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

#include "vtkObjectFactory.h"
#include "vtkKWPushButton.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWComboBox.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWRadioButton.h"
#include "vtkKWMimxMainUserInterfacePanel.h"


#include <vtksys/stl/list>
#include <vtksys/stl/algorithm>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                 1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateBBFromRubberBandPickGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateBBFromRubberBandPickGroup, "$Revision: 1.7.4.2 $");
//----------------------------------------------------------------------------
//class vtkRightButtonPressEventCallback : public vtkCommand
//{
//public:
//      static vtkRightButtonPressEventCallback *New() 
//      { return new vtkRightButtonPressEventCallback; }
//      virtual void Execute(vtkObject *caller, unsigned long, void*)
//      {
//              this->PickInstance->CreateBBFromRubberBandPickApplyCallback();
//      }
//
//      vtkKWMimxCreateBBFromRubberBandPickGroup *PickInstance;
//};
//----------------------------------------------------------------------------
vtkKWMimxCreateBBFromRubberBandPickGroup::vtkKWMimxCreateBBFromRubberBandPickGroup()
{
        this->ComponentFrame = NULL;
  this->MimxMainWindow = NULL;
  this->ObjectListComboBox = NULL;
  this->CreateBuildingBlockFromPickWidget = NULL;
  this->ExtrusionLengthEntry = NULL;
  //this->RightButtonPressCallback = vtkRightButtonPressEventCallback::New();
  //this->RightButtonPressCallback->PickInstance = this;

  strcpy(this->PreviousSurface,"");
}

//----------------------------------------------------------------------------
vtkKWMimxCreateBBFromRubberBandPickGroup::~vtkKWMimxCreateBBFromRubberBandPickGroup()
{
  if(this->ObjectListComboBox)  
    this->ObjectListComboBox->Delete();
  if(this->CreateBuildingBlockFromPickWidget)
          this->CreateBuildingBlockFromPickWidget->Delete();
  if(this->ExtrusionLengthEntry)
          this->ExtrusionLengthEntry->Delete();
        if(this->ComponentFrame)
          this->ComponentFrame->Delete();
        //this->RightButtonPressCallback->Delete();
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::SelectionChangedCallback(const char* surfaceName)
{
        if ( (strcmp(surfaceName, this->PreviousSurface) != 0) && (strcmp(surfaceName, "") != 0) )
        {
        this->CreateBBFromRubberBandPickCallback(0);
        this->CreateBBFromRubberBandPickCallback(1);
        strcpy(this->PreviousSurface, surfaceName);
        }
}
//--------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create BB from RubberBand Pick");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand n -padx 2 -pady 0 -fill x", 
    this->MainFrame->GetWidgetName());

  if (!this->ComponentFrame)    
     this->ComponentFrame = vtkKWFrameWithLabel::New();
  ComponentFrame->SetParent(this->MainFrame);
  ComponentFrame->Create();
  ComponentFrame->SetLabelText("Surface");
  ComponentFrame->CollapseFrame( );
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 2 -fill x", 
    this->ComponentFrame->GetWidgetName());
  
  if(!this->ObjectListComboBox)
    this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();  
  this->ObjectListComboBox->SetParent(this->ComponentFrame->GetFrame());
  this->ObjectListComboBox->Create();
  this->ObjectListComboBox->SetWidth(20);
  this->ObjectListComboBox->SetLabelText("Surface : ");
  this->ObjectListComboBox->SetLabelWidth( 10 );
  this->ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->ObjectListComboBox->GetWidget()->SetCommand(this, "SelectionChangedCallback");
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6 -fill x", 
    this->ObjectListComboBox->GetWidgetName());

  if (!this->ExtrusionLengthEntry)
          this->ExtrusionLengthEntry = vtkKWEntryWithLabel::New();

  this->ExtrusionLengthEntry->SetParent(this->MainFrame);
  this->ExtrusionLengthEntry->Create();
  this->ExtrusionLengthEntry->GetWidget()->SetCommand(this, "ExtrusionLengthChangedCallback");
  this->ExtrusionLengthEntry->SetWidth(4);
  this->ExtrusionLengthEntry->SetLabelText("Extrusion Length : ");
  this->ExtrusionLengthEntry->GetWidget()->SetValueAsDouble(1.0);
  this->ExtrusionLengthEntry->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
          "pack %s -side top -anchor nw -expand 0 -padx 2 -pady 6 -fill x", 
          this->ExtrusionLengthEntry->GetWidgetName());

  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "CreateBBFromRubberBandPickApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand n -padx 20 -pady 6", 
          this->ApplyButton->GetWidgetName());

  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "CreateBBFromRubberBandPickCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand n -padx 20 -pady 6", 
    this->CancelButton->GetWidgetName());
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateBBFromRubberBandPickGroup::CreateBBFromRubberBandPickApplyCallback()
{
  vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
          callback->ErrorMessage("No surface has been selected for defining the building block.");
          return 0;
  }
  if(this->CreateBuildingBlockFromPickWidget)
  {
          if(this->CreateBuildingBlockFromPickWidget->GetEnabled())
          {
                  //vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
                  //const char *name = combobox->GetValue();
                  
                  vtkUnstructuredGrid *ugrid = this->CreateBuildingBlockFromPickWidget->GetBuildingBlock();
                  if(!ugrid->GetNumberOfCells())
                  {
                          callback->ErrorMessage("Failed to create the requested building block");
                          return 0;
                  }
                  this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
                  // for do and undo tree
                  this->DoUndoTree->AppendItem(new Node);
                  int currentitem = this->BBoxList->GetNumberOfItems()-1;
                  this->DoUndoTree->GetItem(currentitem)->Parent = NULL;
                  this->DoUndoTree->GetItem(currentitem)->Child = NULL;
                  this->DoUndoTree->GetItem(currentitem)->Data = 
                          vtkMimxUnstructuredGridActor::SafeDownCast(
                          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1));
                  ////    
                  this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->
                          SetDataType(ACTOR_BUILDING_BLOCK);
                  vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
                          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->DeepCopy(ugrid);
                  this->Count++;
                  /* Assign Mesh Seeds - Should be an Application Configured Parameter*/
                  vtkMimxUnstructuredGridActor *ugridactor = 
                          vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
                          this->BBoxList->GetNumberOfItems()-1));
                  ugridactor->MeshSeedFromAverageElementLength( 1.0, 1.0, 1.0 );

                  vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
                          this->BBoxList->GetNumberOfItems()-1))->SetObjectName("BBFromRubberBandPick_",Count);
                  vtkMimxUnstructuredGridActor::SafeDownCast(this->BBoxList->GetItem(
                          this->BBoxList->GetNumberOfItems()-1))->GetDataSet()->Modified();
                  this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp(
                          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1)->GetActor());
                  this->GetMimxMainWindow()->GetRenderWidget()->Render();
                  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
                  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList(
                          this->BBoxList->GetItem(this->BBoxList->GetNumberOfItems()-1));
                  this->GetMimxMainWindow()->SetStatusText("Created Building Block From RubberBand pick");                        
                  return 1;
          }
  }
  return 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::CreateBBFromRubberBandPickCancelCallback()
{
        this->CancelStatus = 1;
        this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName());
        this->CreateBBFromRubberBandPickCallback(0);
        strcpy(this->PreviousSurface, "");
        this->MenuGroup->SetMenuButtonsEnabled(1);
        this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
        this->CancelStatus = 0;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::CreateBBFromRubberBandPickCallback(int mode)
{
        if(!mode)
        {
                if(!this->CreateBuildingBlockFromPickWidget)    return;
                if(this->CreateBuildingBlockFromPickWidget->GetEnabled())
                        this->CreateBuildingBlockFromPickWidget->SetEnabled(0);
                //this->CreateBuildingBlockFromPickWidget->RemoveObservers(
                //      vtkCommand::RightButtonPressEvent, this->RightButtonPressCallback);
                return;
        }
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
        if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
        {
                callback->ErrorMessage("Surface not chosen");
                return;
        }
        vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
        const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Invalid surface was selected");
                combobox->SetValue("");
                return ;
        }

        double extrusionLength = this->ExtrusionLengthEntry->GetWidget()->GetValueAsDouble();
        if(extrusionLength <= 0.0)
        {
                callback->ErrorMessage("Extrusion length must be greater than 0.0");
                return;
        }
        vtkPolyData *surface = vtkMimxSurfacePolyDataActor::SafeDownCast(
                this->SurfaceList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        if(!this->CreateBuildingBlockFromPickWidget)
        {
                this->CreateBuildingBlockFromPickWidget = vtkMimxCreateBuildingBlockFromPickWidget::New();
                this->CreateBuildingBlockFromPickWidget->SetInteractor(
                        this->MimxMainWindow->GetRenderWidget()->GetRenderWindowInteractor());
                this->CreateBuildingBlockFromPickWidget->SetInput(surface);
                //this->CreateBuildingBlockFromPickWidget->AddObserver(vtkCommand::RightButtonPressEvent, 
                //      this->RightButtonPressCallback, 1.0);
                this->CreateBuildingBlockFromPickWidget->SetEnabled(1);
                this->CreateBuildingBlockFromPickWidget->SetExtrusionLength(extrusionLength);
                return;
        }
        if(this->CreateBuildingBlockFromPickWidget->GetEnabled())
        {
                this->CreateBuildingBlockFromPickWidget->SetEnabled(0);
                this->CreateBuildingBlockFromPickWidget->SetInput(surface);
                //this->CreateBuildingBlockFromPickWidget->AddObserver(vtkCommand::RightButtonPressEvent, 
                //      this->RightButtonPressCallback, 1.0);
                this->CreateBuildingBlockFromPickWidget->SetEnabled(1);
        }
        else
        {
                this->CreateBuildingBlockFromPickWidget->SetInput(surface);
        }
        this->CreateBuildingBlockFromPickWidget->SetExtrusionLength(extrusionLength);
}
//-----------------------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::UpdateObjectLists()
{
        this->UpdateSurfaceComboBox( this->ObjectListComboBox->GetWidget() );
        /*
        this->ObjectListComboBox->GetWidget()->DeleteAllValues();
        
        int defaultItem = -1;
        for (int i = 0; i < this->SurfaceList->GetNumberOfItems(); i++)
        {
                ObjectListComboBox->GetWidget()->AddValue(
                        this->SurfaceList->GetItem(i)->GetFileName());
                        
                int viewedItem = this->GetMimxMainWindow()->GetRenderWidget()->GetRenderer()->HasViewProp(
                        this->SurfaceList->GetItem(i)->GetActor());
    if ( (defaultItem == -1) && ( viewedItem ) )
                {
                  defaultItem = i;
                }
        }
        
        if ((this->SurfaceList->GetNumberOfItems() > 0) && (defaultItem == -1))
    defaultItem = this->SurfaceList->GetNumberOfItems()-1;
    
        if (defaultItem != -1)
  {
    ObjectListComboBox->GetWidget()->SetValue(
          this->SurfaceList->GetItem(defaultItem)->GetFileName());
  }
  */
}
//--------------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::CreateBBFromRubberBandPickDoneCallback()
{
        if(this->CreateBBFromRubberBandPickApplyCallback())
                this->CreateBBFromRubberBandPickCancelCallback();
}
//---------------------------------------------------------------------------------
void vtkKWMimxCreateBBFromRubberBandPickGroup::ExtrusionLengthChangedCallback(const char *Entry)
{
        if(this->CreateBuildingBlockFromPickWidget)
                this->CreateBuildingBlockFromPickWidget->SetExtrusionLength(
                this->ExtrusionLengthEntry->GetWidget()->GetValueAsDouble());
}
//---------------------------------------------------------------------------------
