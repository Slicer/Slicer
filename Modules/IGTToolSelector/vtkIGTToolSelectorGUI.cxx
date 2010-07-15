/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkIGTToolSelectorGUI.h"
#include "vtkMRMLLinearTransformNode.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkSmartPointer.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"

//#include "vtkKWPushButton.h"
#include "vtkKWListBox.h"
#include "vtkKWListBoxWithScrollbars.h"
#include "vtkKWListBoxWithScrollbarsWithLabel.h"
#include "vtkKWScale.h"
#include "vtkKWScaleWithEntry.h"

#include "vtkCornerAnnotation.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkIGTToolSelectorGUI );
vtkCxxRevisionMacro ( vtkIGTToolSelectorGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkIGTToolSelectorGUI::vtkIGTToolSelectorGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkIGTToolSelectorGUI::DataCallback);
  
  //----------------------------------------------------------------
  // GUI widgets
  this->ProbeTransformSelector = NULL;
  this->ToolTypeListBox = NULL;
  this->EnableToolCheckButton = NULL;
  this->ShowAxesCheckButton = NULL;
  this->ShowProjectionCheckButton = NULL;
  this->ShowToolTipCheckButton = NULL;
  
  //----------------------------------------------------------------
  // Locator  (MRML)
  this->TimerFlag = 0;

}

//---------------------------------------------------------------------------
vtkIGTToolSelectorGUI::~vtkIGTToolSelectorGUI ( )
{

  //----------------------------------------------------------------
  // Remove Callbacks

  if (this->DataCallbackCommand)
    {
    this->DataCallbackCommand->Delete();
    }

  //----------------------------------------------------------------
  // Remove Observers

  this->RemoveGUIObservers();

  //----------------------------------------------------------------
  // Remove GUI widgets

  // using vtkSmartPointers.  This is no longer needed.
//  if (this->ProbeTransformSelector)
//    {
//    this->ProbeTransformSelector->SetParent(NULL);
//    this->ProbeTransformSelector->Delete();
//    }
//
//  if (this->ToolTypeListBox)
//  {
//    this->ToolTypeListBox->SetParent(NULL);
//    this->ToolTypeListBox->Delete();
//  }
//
//  if (this->EnableToolCheckButton)
//  {
//    this->EnableToolCheckButton->SetParent(NULL);
//    this->EnableToolCheckButton->Delete();
//  }
//
//  if (this->ShowAxesCheckButton)
//  {
//    this->ShowAxesCheckButton->SetParent(NULL);
//    this->ShowAxesCheckButton->Delete();
//  }

  //----------------------------------------------------------------
  // Unregister Logic class

  this->SetModuleLogic ( NULL );

}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::Enter()
{
  // Fill in
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  
  if (this->TimerFlag == 0)
    {
    this->TimerFlag = 1;
    this->TimerInterval = 100;  // 100 ms
    ProcessTimerEvents();
    }

}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "IGTToolSelectorGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (this->ProbeTransformSelector)
    {
    this->ProbeTransformSelector->
        RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->ToolTypeListBox)
    {
     // note the double GetWidget calls to get to the vtkKWListBox object.
    this->ToolTypeListBox->GetWidget()->GetWidget()
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->EnableToolCheckButton)
  {
    this->EnableToolCheckButton
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }

  if (this->ShowAxesCheckButton)
  {
    this->ShowAxesCheckButton
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }

  if (this->ShowProjectionCheckButton)
  {
    this->ShowProjectionCheckButton
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }

  if (this->ShowToolTipCheckButton)
  {
    this->ShowToolTipCheckButton
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }

  if (this->ProjectionLengthScale)
  {
    this->ProjectionLengthScale->GetWidget()
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }

  if (this->ProjectionDiameterScale)
  {
    this->ProjectionDiameterScale->GetWidget()
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }
  if (this->ToolTipDiameterScale)
  {
    this->ToolTipDiameterScale->GetWidget()
        ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
  }


  this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::AddGUIObservers ( )
{
  this->RemoveGUIObservers();

  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  //----------------------------------------------------------------
  // MRML

  vtkIntArray* events = vtkIntArray::New();
  //events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  //events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::SceneClosedEvent);
  
  if (this->GetMRMLScene() != NULL)
    {
    this->SetAndObserveMRMLSceneEvents(this->GetMRMLScene(), events);
    }
  events->Delete();

  //----------------------------------------------------------------
  // GUI Observers

  // node selectors.
  this->ProbeTransformSelector
    ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  // combo boxes
  // note the double GetWidget calls to get to the vtkKWListBox object.
  this->ToolTypeListBox->GetWidget()->GetWidget()
    ->AddObserver(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  // check box.
  this->EnableToolCheckButton
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ShowAxesCheckButton
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ShowProjectionCheckButton
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ShowToolTipCheckButton
      ->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ProjectionLengthScale->GetWidget()
      ->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ProjectionDiameterScale->GetWidget()
      ->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->ToolTipDiameterScale->GetWidget()
      ->AddObserver(vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}




//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkIGTToolSelectorLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }

  //node selectors.
  if (this->ProbeTransformSelector ==  vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
          this->ProbeTransformSelector->GetSelected() != NULL)
    {
    //std::cerr << "DEBUG: ProbeTransformSelector is selected." << std::endl;
    vtkMRMLLinearTransformNode* node =
      vtkMRMLLinearTransformNode::SafeDownCast(this->ProbeTransformSelector->GetSelected());
    this->GetLogic()->SetToolTransformNode(node->GetID());
    }

  //list boxes
   // note the double GetWidget calls to get to the vtkKWListBox object.
  else if (this->ToolTypeListBox->GetWidget()->GetWidget() == vtkKWListBox::SafeDownCast(caller)
    && event == vtkKWListBox::ListBoxSelectionChangedEvent)
    {
    //std::cerr << "DEBUG: ToolTypeListBox: " << this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelection() << std::endl;
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }

    }

  // check box.
  else if (this->EnableToolCheckButton == vtkKWCheckButton::SafeDownCast(caller)
    && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //std::cerr << "DEBUG: EnableToolCheckButton checked." << std::endl;
    this->GetLogic()->
        EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                   this->EnableToolCheckButton->GetSelectedState());
    }
  else if (this->ShowAxesCheckButton == vtkKWCheckButton::SafeDownCast(caller)
    && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //std::cerr << "DEBUG: ShowAxesCheckButton checked." << std::endl;
    this->GetLogic()->SetShowAxes(this->ShowAxesCheckButton->GetSelectedState());
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }
    }
  else if (this->ShowProjectionCheckButton == vtkKWCheckButton::SafeDownCast(caller)
    && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //std::cerr << "DEBUG: ShowProjectionCheckButton checked." << std::endl;
    this->GetLogic()->SetShowProjection(this->ShowProjectionCheckButton->GetSelectedState());
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }
    }
  else if (this->ShowToolTipCheckButton == vtkKWCheckButton::SafeDownCast(caller)
    && event == vtkKWCheckButton::SelectedStateChangedEvent)
    {
    //std::cerr << "DEBUG: ShowToolTipCheckButton checked." << std::endl;
    this->GetLogic()->SetShowToolTip(this->ShowToolTipCheckButton->GetSelectedState());
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }
    }
  else if (this->ProjectionLengthScale->GetWidget() == vtkKWScale::SafeDownCast(caller)
    && event == vtkKWScale::ScaleValueChangedEvent)
    {
    //std::cerr << "DEBUG: ProjectionLengthScale value changed." << std::endl;
    this->GetLogic()->SetProjectionLength(this->ProjectionLengthScale->GetValue());
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }
    }
  else if (this->ProjectionDiameterScale->GetWidget() == vtkKWScale::SafeDownCast(caller)
    && event == vtkKWScale::ScaleValueChangedEvent)
    {
    //std::cerr << "DEBUG: ProjectionDiameterScale value changed." << std::endl;
    this->GetLogic()->SetProjectionDiameter(this->ProjectionDiameterScale->GetValue());
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }
    }
  else if (this->ToolTipDiameterScale->GetWidget() == vtkKWScale::SafeDownCast(caller)
    && event == vtkKWScale::ScaleValueChangedEvent)
    {
    //std::cerr << "DEBUG: ToolTipDiameterScale value changed." << std::endl;
    this->GetLogic()->SetToolTipDiameter(this->ToolTipDiameterScale->GetValue());
    if(this->EnableToolCheckButton->GetSelectedState())
    {
      this->GetLogic()->
          EnableTool(this->ToolTypeListBox->GetWidget()->GetWidget()->GetSelectionIndex(),
                     this->EnableToolCheckButton->GetSelectedState());
    }
    }
  }


void vtkIGTToolSelectorGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkIGTToolSelectorGUI *self = reinterpret_cast<vtkIGTToolSelectorGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkIGTToolSelectorGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkIGTToolSelectorLogic::SafeDownCast(caller))
    {
    if (event == vtkIGTToolSelectorLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in

  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    }
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::ProcessTimerEvents()
{
  if (this->TimerFlag)
    {
    // update timer
    vtkKWTkUtilities::CreateTimerHandler(vtkKWApplication::GetMainInterp(), 
                                         this->TimerInterval,
                                         this, "ProcessTimerEvents");        
    }
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "IGTToolSelector", "IGTToolSelector", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForToolPropertiesFrame();
}


void vtkIGTToolSelectorGUI::BuildGUIForHelpFrame ()
{
  // Define your help text here.
  const char *help = 
    "This module is used to select a virtual representation of a tool that is tracked via OpenIGTLink. Details can be found at <a>http://www.slicer.org/slicerWiki/index.php/Modules:IGTToolSelector-Documentation-3.6</a>.";
  const char *about =
      "This was developed by Andrew Wiles at NDI, <a>http://www.ndigital.com</a>.";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "IGTToolSelector" );
  this->BuildHelpAndAboutFrame (page, help, about);
}


//---------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::BuildGUIForToolPropertiesFrame()
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("IGTToolSelector");

  vtkSmartPointer<vtkSlicerModuleCollapsibleFrame> conBrowsFrame
      = vtkSmartPointer<vtkSlicerModuleCollapsibleFrame>::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Tool Properties");
  //conBrowsFrame->CollapseFrame(); // set default to have the frame open.
  app->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Tool properties child frame

  vtkSmartPointer<vtkKWFrameWithLabel> frame
      = vtkSmartPointer<vtkKWFrameWithLabel>::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  frame->SetLabelText ("Tool Properties Child Frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 frame->GetWidgetName() );

  // -----------------------------------------
  // Probe Transform Selector

  this->ProbeTransformSelector = vtkSmartPointer<vtkSlicerNodeSelectorWidget>::New();
  this->ProbeTransformSelector->SetNodeClass("vtkMRMLLinearTransformNode", NULL, NULL, NULL);
  //this->ProbeTransformSelector->SetNewNodeEnabled(1); // don't want to create this here. Create in OpenIGTLink.
  this->ProbeTransformSelector->SetParent( frame->GetFrame() );
  this->ProbeTransformSelector->Create();
  this->ProbeTransformSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->ProbeTransformSelector->UpdateMenu();

  this->ProbeTransformSelector->SetBorderWidth(2);
  this->ProbeTransformSelector->SetLabelText( "Probe Transform: ");
  this->ProbeTransformSelector->SetBalloonHelpString("Input probe transform with tracker info from OpenIGTLink.");

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ProbeTransformSelector->GetWidgetName());

  // -----------------------------------------
  // Create the Tool Type List.
  this->ToolTypeListBox = vtkSmartPointer<vtkKWListBoxWithScrollbarsWithLabel>::New ( );
  this->ToolTypeListBox->SetParent ( frame->GetFrame() );
  this->ToolTypeListBox->Create ( );
  //this->ToolTypeListBox->SetWidth (12);
  this->ToolTypeListBox->SetLabelText("Tool Type:");
  this->ToolTypeListBox->GetWidget()->GetWidget()->SetHeight(3);
  this->ToolTypeListBox->GetWidget()->GetWidget()->InsertEntry(OPENIGT_DEFAULT, "OpenIGTLink Default");
  this->ToolTypeListBox->GetWidget()->GetWidget()->InsertEntry(NEEDLE, "Needle");
  //this->ToolTypeListBox->GetWidget()->GetWidget()->InsertEntry(NEEDLE_PROJ, "Needle w/ Projection");
  this->ToolTypeListBox->GetWidget()->GetWidget()->InsertEntry(POINTER, "Pointer");
  //this->ToolTypeListBox->GetWidget()->GetWidget()->InsertEntry(POINTER_PROJ, "Pointer w/ Projection");
  // note the double GetWidget calls to get to the vtkKWListBox object.

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ToolTypeListBox->GetWidgetName());

  // -----------------------------------------
  // Create the enable check box.
  this->EnableToolCheckButton = vtkSmartPointer<vtkKWCheckButton>::New();
  this->EnableToolCheckButton->SetParent(frame->GetFrame());
  this->EnableToolCheckButton->Create();
  this->EnableToolCheckButton->SetText("Enable Tool Polydata");

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->EnableToolCheckButton->GetWidgetName());

  // -----------------------------------------
  // Create the enable check box.
  this->ShowAxesCheckButton = vtkSmartPointer<vtkKWCheckButton>::New();
  this->ShowAxesCheckButton->SetParent(frame->GetFrame());
  this->ShowAxesCheckButton->Create();
  this->ShowAxesCheckButton->SetText("Show Axes");

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ShowAxesCheckButton->GetWidgetName());

  // -----------------------------------------
  // Create the projection check box.
  this->ShowProjectionCheckButton = vtkSmartPointer<vtkKWCheckButton>::New();
  this->ShowProjectionCheckButton->SetParent(frame->GetFrame());
  this->ShowProjectionCheckButton->Create();
  this->ShowProjectionCheckButton->SetText("Show Tool Projection");

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ShowProjectionCheckButton->GetWidgetName());

  this->ProjectionLengthScale = vtkSmartPointer<vtkKWScaleWithEntry>::New();
  this->ProjectionLengthScale->SetParent(frame->GetFrame());
  this->ProjectionLengthScale->Create();
  this->ProjectionLengthScale->SetRange(1.0, 250.0);
  this->ProjectionLengthScale->SetValue(100.0);
  this->ProjectionLengthScale->SetRangeVisibility(1);
  this->ProjectionLengthScale->SetResolution(5.0);
  this->ProjectionLengthScale->SetLabelText("Set Projection Length: ");
  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ProjectionLengthScale->GetWidgetName());

  this->ProjectionDiameterScale = vtkSmartPointer<vtkKWScaleWithEntry>::New();
  this->ProjectionDiameterScale->SetParent(frame->GetFrame());
  this->ProjectionDiameterScale->Create();
  this->ProjectionDiameterScale->SetRange(0.5, 10.0);
  this->ProjectionDiameterScale->SetValue(1.0);
  this->ProjectionDiameterScale->SetRangeVisibility(1);
  this->ProjectionDiameterScale->SetResolution(0.5);
  this->ProjectionDiameterScale->SetLabelText("Set Projection Diameter: ");
  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ProjectionDiameterScale->GetWidgetName());

  // -----------------------------------------
  // Create the tool tip check box.
  this->ShowToolTipCheckButton = vtkSmartPointer<vtkKWCheckButton>::New();
  this->ShowToolTipCheckButton->SetParent(frame->GetFrame());
  this->ShowToolTipCheckButton->Create();
  this->ShowToolTipCheckButton->SetText("Show Tool Tip Location");

  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ShowToolTipCheckButton->GetWidgetName());

  this->ToolTipDiameterScale = vtkSmartPointer<vtkKWScaleWithEntry>::New();
  this->ToolTipDiameterScale->SetParent(frame->GetFrame());
  this->ToolTipDiameterScale->Create();
  this->ToolTipDiameterScale->SetRange(0.25, 10.0);
  this->ToolTipDiameterScale->SetValue(0.25);
  this->ToolTipDiameterScale->SetRangeVisibility(1);
  this->ToolTipDiameterScale->SetResolution(0.5);
  this->ToolTipDiameterScale->SetLabelText("Set Tool Tip Diameter: ");
  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->ToolTipDiameterScale->GetWidgetName());


  // clean up temporary objects.  **removed after implementing smart pointers.
  //conBrowsFrame->Delete();
  //frame->Delete();
}

//----------------------------------------------------------------------------
void vtkIGTToolSelectorGUI::UpdateAll()
{
}

