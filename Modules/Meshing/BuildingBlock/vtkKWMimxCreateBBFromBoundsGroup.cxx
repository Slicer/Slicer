/*=========================================================================

Program:   MIMX Meshing Toolkit
Module:    $RCSfile: vtkKWMimxCreateBBFromBoundsGroup.cxx,v $
Language:  C++
Date:      $Date: 2008/10/26 18:51:24 $
Version:   $Revision: 1.39.4.3 $

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

#include "vtkKWMimxCreateBBFromBoundsGroup.h"

#include "vtkActor.h"
#include "vtkObjectFactory.h"
#include "vtkPolyData.h"
#include "vtkRenderer.h"
#include "vtkUnstructuredGrid.h"

#include "vtkKWApplication.h"
#include "vtkKWComboBox.h"
#include "vtkKWComboBoxWithLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWFileBrowserDialog.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWPushButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWRenderWidget.h"
#include "vtkKWTkUtilities.h"

#include "vtkMimxBoundingBoxSource.h"
#include "vtkMimxCreateBuildingBlockFromPickWidget.h"
#include "vtkMimxErrorCallback.h"
#include "vtkMimxSurfacePolyDataActor.h"
#include "vtkMimxUnstructuredGridActor.h"

#include "vtkKWMimxMainNotebook.h"
#include "vtkKWMimxMainWindow.h"
#include "vtkKWMimxMainUserInterfacePanel.h"

#include "vtkLinkedListWrapper.h"

#include <vtksys/stl/algorithm>
#include <vtksys/stl/list>

// define the option types
#define VTK_KW_OPTION_NONE         0
#define VTK_KW_OPTION_LOAD                   1

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkKWMimxCreateBBFromBoundsGroup);
vtkCxxRevisionMacro(vtkKWMimxCreateBBFromBoundsGroup, "$Revision: 1.39.4.3 $");
//----------------------------------------------------------------------------
class vtkRightButtonBBManualCreateCallback : public vtkCommand
{
public:
        static vtkRightButtonBBManualCreateCallback *New() 
        { return new vtkRightButtonBBManualCreateCallback; }
        virtual void Execute(vtkObject *vtkNotUsed(caller), unsigned long, void*)
        {
                this->PickInstance->CreateBBFromBoundsApplyCallback();
        }

        vtkKWMimxCreateBBFromBoundsGroup *PickInstance;
};
//----------------------------------------------------------------------------
vtkKWMimxCreateBBFromBoundsGroup::vtkKWMimxCreateBBFromBoundsGroup()
{
  this->ObjectListComboBox = NULL;
  this->ComponentFrame = NULL;
  this->ModeFrame = NULL;
  this->SurfaceBoundButton = NULL;
  this->ManualBoundButton = NULL;
  this->EntryFrame = NULL;
  this->ExtrusionLengthEntry = NULL;
  this->CreateBuildingBlockFromPickWidget = NULL;
  this->RightButtonPressCallback = vtkRightButtonBBManualCreateCallback::New();
  this->RightButtonPressCallback->PickInstance = this;
  this->MaualBBCreationCount = 0;
}

//----------------------------------------------------------------------------
vtkKWMimxCreateBBFromBoundsGroup::~vtkKWMimxCreateBBFromBoundsGroup()
{
  if (this->ObjectListComboBox)
    this->ObjectListComboBox->Delete();
  if (this->ComponentFrame)
     this->ComponentFrame->Delete();   
  if (this->ModeFrame)
    this->ModeFrame->Delete();
  if (this->SurfaceBoundButton)
    this->SurfaceBoundButton->Delete();
  if (this->ManualBoundButton)
    this->ManualBoundButton->Delete();
  if (this->EntryFrame)
    this->EntryFrame->Delete();
  if (this->ExtrusionLengthEntry)
    this->ExtrusionLengthEntry->Delete();
  if (this->CreateBuildingBlockFromPickWidget)
  {
          this->CreateBuildingBlockFromPickWidget->RemoveObservers(
                  vtkCommand::RightButtonPressEvent, this->RightButtonPressCallback);
    this->CreateBuildingBlockFromPickWidget->Delete();
  }
  this->RightButtonPressCallback->Delete();
 }
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::CreateWidget()
{
  if(this->IsCreated())
  {
    vtkErrorMacro("class already created");
    return;
  }

  this->Superclass::CreateWidget();
  
  this->MainFrame->SetParent(this->GetParent());
  this->MainFrame->Create();
  //this->MainFrame->SetLabelText("Create Building Block From Bounds");

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
  
  if (!this->ObjectListComboBox)        
     this->ObjectListComboBox = vtkKWComboBoxWithLabel::New();
  ObjectListComboBox->SetParent(this->ComponentFrame->GetFrame());
  ObjectListComboBox->Create();
  ObjectListComboBox->SetLabelText("Surface : ");
  ObjectListComboBox->SetLabelWidth( 15 );
  ObjectListComboBox->GetWidget()->ReadOnlyOn();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -expand y -padx 2 -pady 6 -fill x", 
    ObjectListComboBox->GetWidgetName());

  if (!this->ModeFrame) 
     this->ModeFrame = vtkKWFrame::New();
  this->ModeFrame->SetParent(this->MainFrame);
  this->ModeFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor n -padx 2 -pady 6", 
    this->ModeFrame->GetWidgetName());
  
  if (!this->SurfaceBoundButton)        
     this->SurfaceBoundButton = vtkKWRadioButton::New();
  this->SurfaceBoundButton->SetParent( this->ModeFrame );
  this->SurfaceBoundButton->Create();
  this->SurfaceBoundButton->SetText("Create block from surface bounds");
  this->SurfaceBoundButton->SetCommand(this, "SurfaceBoundCallback");
  this->SurfaceBoundButton->SetValueAsInt(1);
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw  -padx 2 -pady 2", 
    this->SurfaceBoundButton->GetWidgetName());
    
  if (!this->ManualBoundButton) 
     this->ManualBoundButton = vtkKWRadioButton::New();
  this->ManualBoundButton->SetParent( this->ModeFrame );
  this->ManualBoundButton->Create();
  this->ManualBoundButton->SetText("Create block manually");
  this->ManualBoundButton->SetCommand(this, "ManualBoundCallback");
  this->ManualBoundButton->SetValueAsInt(2);
  this->ManualBoundButton->SetVariableName(this->SurfaceBoundButton->GetVariableName()); 
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw  -padx 2 -pady 2", 
    this->ManualBoundButton->GetWidgetName());      
  
  if (!this->EntryFrame)        
     this->EntryFrame = vtkKWFrame::New();
  this->EntryFrame->SetParent(this->MainFrame);
  this->EntryFrame->Create();
  this->GetApplication()->Script(
    "pack %s -side top -anchor nw -padx 2 -pady 6 -fill x", 
    this->EntryFrame->GetWidgetName());
  
  if (!this->ExtrusionLengthEntry)
          this->ExtrusionLengthEntry = vtkKWEntryWithLabel::New();
  this->ExtrusionLengthEntry->SetParent(this->EntryFrame);
  this->ExtrusionLengthEntry->Create();
  this->ExtrusionLengthEntry->GetWidget()->SetCommand(this, "ExtrusionLengthChangedCallback");
  this->ExtrusionLengthEntry->SetLabelText("Extrusion Length : ");
  this->ExtrusionLengthEntry->GetWidget()->SetValueAsDouble(1.0);
  this->ExtrusionLengthEntry->GetWidget()->SetRestrictValueToDouble();
  this->GetApplication()->Script(
          "pack %s -side top -anchor n -padx 2 -pady 2", 
          this->ExtrusionLengthEntry->GetWidgetName());
            
  this->ApplyButton->SetParent(this->MainFrame);
  this->ApplyButton->Create();
  this->ApplyButton->SetReliefToFlat();
  this->ApplyButton->SetImageToIcon( this->GetMimxMainWindow()->GetApplyButtonIcon() );
  this->ApplyButton->SetCommand(this, "CreateBBFromBoundsApplyCallback");
  this->GetApplication()->Script(
          "pack %s -side left -anchor nw -expand y -padx 5 -pady 6", 
          this->ApplyButton->GetWidgetName());


  this->CancelButton->SetParent(this->MainFrame);
  this->CancelButton->Create();
  this->CancelButton->SetReliefToFlat();
  this->CancelButton->SetImageToIcon( this->GetMimxMainWindow()->GetCancelButtonIcon() );
  this->CancelButton->SetCommand(this, "CreateBBFromBoundsCancelCallback");
  this->GetApplication()->Script(
    "pack %s -side right -anchor ne -expand y -padx 5 -pady 6", 
    this->CancelButton->GetWidgetName());

  /* Set the Initial State */
  this->SurfaceBoundButton->SetSelectedState(1);
  
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::Update()
{
        this->UpdateEnableState();
}
//---------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::UpdateEnableState()
{
        this->UpdateObjectLists();
        this->Superclass::UpdateEnableState();
}
//----------------------------------------------------------------------------
int vtkKWMimxCreateBBFromBoundsGroup::CreateBBFromBoundsApplyCallback()
{
        vtkMimxErrorCallback *callback = this->GetMimxMainWindow()->GetErrorCallback();
  if(!strcmp(this->ObjectListComboBox->GetWidget()->GetValue(),""))
  {
                callback->ErrorMessage("Object not chosen");
                return 0;
  }
  vtkKWComboBox *combobox = this->ObjectListComboBox->GetWidget();
    const char *name = combobox->GetValue();
        int num = combobox->GetValueIndex(name);
        if(num < 0 || num > combobox->GetNumberOfValues()-1)
        {
                callback->ErrorMessage("Choose valid surface");
                combobox->SetValue("");
                return 0;
        }
        
  vtkPolyData *polydata = vtkMimxSurfacePolyDataActor::SafeDownCast(
                this->SurfaceList->GetItem(combobox->GetValueIndex(name)))->GetDataSet();
        
        callback->SetState(0);
        const char *foundationname = this->SurfaceList->GetItem(combobox->GetValueIndex(name))->GetFileName();
        if (this->SurfaceBoundButton->GetSelectedState())
        {
    vtkMimxBoundingBoxSource *bbox = vtkMimxBoundingBoxSource::New();
    bbox->SetSource(polydata);
    bbox->AddObserver(vtkCommand::ErrorEvent, callback, 1.0);
    bbox->Update();
    if (!callback->GetState())
    {
      this->Count++;
      this->AddBuildingBlockToDisplay(bbox->GetOutput(), "BBb-", foundationname);
          this->GetMimxMainWindow()->SetStatusText("Created Building Block From Bounds");
    }
    bbox->Delete();
  }
  else
  {
    if(this->CreateBuildingBlockFromPickWidget)
    {
          if(this->CreateBuildingBlockFromPickWidget->GetEnabled())
          {
                  vtkUnstructuredGrid *ugrid = this->CreateBuildingBlockFromPickWidget->GetBuildingBlock();
                  if(!ugrid->GetNumberOfCells())
                  {
                          callback->ErrorMessage("Building block has not yet been defined");
                          return 0;
                  }
        this->MaualBBCreationCount++;
                  this->AddBuildingBlockToDisplay(ugrid, "BBm-", foundationname);
            this->GetMimxMainWindow()->SetStatusText("Created Building Block From Manual definition");                    
          }
    }
  }
        
        this->CreateBBFromBoundsCancelCallback();
        
        return 1;
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::CreateBBFromBoundsCancelCallback()
{
  this->SurfaceBoundButton->SetSelectedState(1);
  this->GetApplication()->Script("pack forget %s", this->MainFrame->GetWidgetName() );
  this->MenuGroup->SetMenuButtonsEnabled(1);
  this->GetMimxMainWindow()->GetMainUserInterfacePanel()->SetEnabled(1);
}
//------------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::UpdateObjectLists()
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

//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::SurfaceBoundCallback( )
{
        this->GetApplication()->Script(
          "pack forget %s", this->EntryFrame->GetWidgetName());
        this->DisableRubberBandPick();
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::ManualBoundCallback( )
{
        this->GetApplication()->Script(
          "pack %s -side top -anchor nw -padx 2 -pady 6 -fill x -after %s",
          this->EntryFrame->GetWidgetName(), this->ModeFrame->GetWidgetName() );
        
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
        
        this->EnableRubberBandPick(surface, extrusionLength);
}
//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::EnableRubberBandPick( vtkPolyData *surface, double extrusionLength )
{
        if (!this->CreateBuildingBlockFromPickWidget)
        {
                this->CreateBuildingBlockFromPickWidget = vtkMimxCreateBuildingBlockFromPickWidget::New();
                this->CreateBuildingBlockFromPickWidget->SetInteractor(
      this->MimxMainWindow->GetRenderWidget()->GetRenderWindowInteractor());
        }
        this->CreateBuildingBlockFromPickWidget->AddObserver(
                vtkCommand::RightButtonPressEvent, this->RightButtonPressCallback, 1.0);
        this->CreateBuildingBlockFromPickWidget->SetInput(surface);
        this->CreateBuildingBlockFromPickWidget->SetEnabled(1);
        this->CreateBuildingBlockFromPickWidget->SetExtrusionLength(extrusionLength);   
}


//----------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::DisableRubberBandPick( )
{
        if (this->CreateBuildingBlockFromPickWidget)
        {
                this->CreateBuildingBlockFromPickWidget->RemoveObservers(
                        vtkCommand::RightButtonPressEvent, this->RightButtonPressCallback);
                this->CreateBuildingBlockFromPickWidget->SetEnabled(0);
        }
}

//---------------------------------------------------------------------------------
void vtkKWMimxCreateBBFromBoundsGroup::ExtrusionLengthChangedCallback(const char *vtkNotUsed(value))
{
        if (this->CreateBuildingBlockFromPickWidget)
        {
                this->CreateBuildingBlockFromPickWidget->SetExtrusionLength(
                this->ExtrusionLengthEntry->GetWidget()->GetValueAsDouble());
        }
}

//---------------------------------------------------------------------------------
/*
void vtkKWMimxCreateBBFromBoundsGroup::AddBuildingBlockToDisplay(
        vtkUnstructuredGrid *ugrid, const char *namePrefix, const char *foundationName, int count)
{
  // Create the New Display Node 
  this->BBoxList->AppendItem(vtkMimxUnstructuredGridActor::New());
  int currentitem = this->BBoxList->GetNumberOfItems()-1;
        this->BBoxList->GetItem(currentitem)->SetDataType(ACTOR_BUILDING_BLOCK);
        
        vtkMimxUnstructuredGridActor *actor = vtkMimxUnstructuredGridActor::SafeDownCast(
          this->BBoxList->GetItem(currentitem));
        actor->SetFoundationName(foundationName);
        actor->GetDataSet()->DeepCopy( ugrid );
        
        // Create the Redo/Undo tree 
  this->DoUndoTree->AppendItem(new Node);
  this->DoUndoTree->GetItem(currentitem)->Parent = NULL;
  this->DoUndoTree->GetItem(currentitem)->Child = NULL;
  this->DoUndoTree->GetItem(currentitem)->Data = actor;
                       
  // Assign Mesh Seeds - Should be an Application Configured Parameter
  double edgeLength = app->GetAverageElementLength();
  actor->MeshSeedFromAverageElementLength( edgeLength, edgeLength, edgeLength );
  vtkIdType index = count;
  actor->SetObjectName(namePrefix,index);
  actor->GetDataSet()->Modified();
  
  this->GetMimxMainWindow()->GetRenderWidget()->AddViewProp( actor );
  this->GetMimxMainWindow()->GetRenderWidget()->Render();
  this->GetMimxMainWindow()->GetRenderWidget()->ResetCamera();
  this->GetMimxMainWindow()->GetViewProperties()->AddObjectList( this->BBoxList->GetItem(currentitem));
}       
*/  
//------------------------------------------------------------------------------

