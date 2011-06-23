/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkObject.h"
#include "vtkObjectFactory.h"

#include "vtkCollectFiducialsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerSlicesGUI.h"
#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkSlicerColor.h"
#include "vtkSlicerTheme.h"

#include "vtkKWTkUtilities.h"
#include "vtkKWWidget.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"
#include "vtkKWEvent.h"

#include "vtkKWPushButton.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkCornerAnnotation.h"


//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkCollectFiducialsGUI );
vtkCxxRevisionMacro ( vtkCollectFiducialsGUI, "$Revision: 1.0 $");
//---------------------------------------------------------------------------


//---------------------------------------------------------------------------
vtkCollectFiducialsGUI::vtkCollectFiducialsGUI ( )
{

  //----------------------------------------------------------------
  // Logic values
  this->Logic = NULL;
  this->DataCallbackCommand = vtkCallbackCommand::New();
  this->DataCallbackCommand->SetClientData( reinterpret_cast<void *> (this) );
  this->DataCallbackCommand->SetCallback(vtkCollectFiducialsGUI::DataCallback);
  
  //----------------------------------------------------------------
  // GUI widgets
  this->FiducialListSelector = NULL;
  this->FiducialListMultiColumnList = NULL;
  this->ResetFiducialsButton = NULL;
  this->NumFiducialsEntry = NULL;
  this->GetNewMeasureButton = NULL;
  this->SaveFiducialsButton = NULL;

  this->ProbeTransformSelector = NULL;
  //----------------------------------------------------------------
  // Locator  (MRML)
  this->TimerFlag = 0;

}

//---------------------------------------------------------------------------
vtkCollectFiducialsGUI::~vtkCollectFiducialsGUI ( )
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

  if (this->FiducialListSelector)
    {
    this->FiducialListSelector->SetParent(NULL);
    this->FiducialListSelector->Delete();
    }

  if (this->FiducialListMultiColumnList)
    {
    this->FiducialListMultiColumnList->SetParent(NULL);
    this->FiducialListMultiColumnList->Delete();
    }

  if (this->ProbeTransformSelector)
    {
    this->ProbeTransformSelector->SetParent(NULL);
    this->ProbeTransformSelector->Delete();
    }

  if (this->ResetFiducialsButton)
    {
    this->ResetFiducialsButton->SetParent(NULL);
    this->ResetFiducialsButton->Delete();
    }

  if (this->GetNewMeasureButton)
    {
    this->GetNewMeasureButton->SetParent(NULL);
    this->GetNewMeasureButton->Delete();
    }

  if (this->SaveFiducialsButton)
    {
    this->SaveFiducialsButton->SetParent(NULL);
    this->SaveFiducialsButton->Delete();
    }

  if (this->NumFiducialsEntry)
    {
    this->NumFiducialsEntry->SetParent(NULL);
    this->NumFiducialsEntry->Delete();
    }


  //----------------------------------------------------------------
  // Unregister Logic class

  this->SetModuleLogic ( NULL );

}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::Init()
{
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::Enter()
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
void vtkCollectFiducialsGUI::Exit ( )
{
  // Fill in
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
  this->vtkObject::PrintSelf ( os, indent );

  os << indent << "CollectFiducialsGUI: " << this->GetClassName ( ) << "\n";
  os << indent << "Logic: " << this->GetLogic ( ) << "\n";
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::RemoveGUIObservers ( )
{
  //vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();

  if (this->FiducialListSelector)
    {
    this->FiducialListSelector->
        RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->ProbeTransformSelector)
    {
    this->ProbeTransformSelector->
        RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
    }

  if (this->ResetFiducialsButton)
    {
    this->ResetFiducialsButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->SaveFiducialsButton)
    {
    this->SaveFiducialsButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  if (this->GetNewMeasureButton)
    {
    this->GetNewMeasureButton
      ->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    }

  this->RemoveLogicObservers();

}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::AddGUIObservers ( )
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

  // node selectors
  this->FiducialListSelector
    ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ProbeTransformSelector
    ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );

  //buttons
  this->ResetFiducialsButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SaveFiducialsButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->GetNewMeasureButton
    ->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand);

  this->AddLogicObservers();

}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::RemoveLogicObservers ( )
{
  if (this->GetLogic())
    {
    this->GetLogic()->RemoveObservers(vtkCommand::ModifiedEvent,
                                      (vtkCommand *)this->LogicCallbackCommand);
    }
}




//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::AddLogicObservers ( )
{
  this->RemoveLogicObservers();  

  if (this->GetLogic())
    {
    this->GetLogic()->AddObserver(vtkCollectFiducialsLogic::StatusUpdateEvent,
                                  (vtkCommand *)this->LogicCallbackCommand);
    }
}

//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::HandleMouseEvent(vtkSlicerInteractorStyle *style)
{
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::ProcessGUIEvents(vtkObject *caller,
                                         unsigned long event, void *callData)
{

  const char *eventName = vtkCommand::GetStringFromEventId(event);
  //vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (strcmp(eventName, "LeftButtonPressEvent") == 0)
    {
    vtkSlicerInteractorStyle *style = vtkSlicerInteractorStyle::SafeDownCast(caller);
    HandleMouseEvent(style);
    return;
    }


  //***************************************
  // Node Selectors.
  //***************************************
  if (this->FiducialListSelector ==  vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
    && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->FiducialListSelector->GetSelected() != NULL)
    {
    //TODO:this->UpdateMRML();
    //std::cerr << "DEBUG: FiducialListSelector is selected." << std::endl;
    }
  else if (this->ProbeTransformSelector ==  vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
    && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->ProbeTransformSelector->GetSelected() != NULL)
    {
    //TODO:this->UpdateMRML();
    //std::cerr << "DEBUG: ProbeTransformSelector is selected." << std::endl;
    }
  //***************************************
  // Buttons.
  //***************************************
  else if (this->ResetFiducialsButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    this->ResetFiducialList();
    }
  else if (this->SaveFiducialsButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
      SaveFiducialList();
    }
  else if (this->GetNewMeasureButton == vtkKWPushButton::SafeDownCast(caller)
      && event == vtkKWPushButton::InvokedEvent)
    {
    GetNewFiducialMeasure();
    }
} 


void vtkCollectFiducialsGUI::DataCallback(vtkObject *caller, 
                                     unsigned long eid, void *clientData, void *callData)
{
  vtkCollectFiducialsGUI *self = reinterpret_cast<vtkCollectFiducialsGUI *>(clientData);
  vtkDebugWithObjectMacro(self, "In vtkCollectFiducialsGUI DataCallback");
  self->UpdateAll();
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::ProcessLogicEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{

  if (this->GetLogic() == vtkCollectFiducialsLogic::SafeDownCast(caller))
    {
    if (event == vtkCollectFiducialsLogic::StatusUpdateEvent)
      {
      //this->UpdateDeviceStatus();
      }
    }
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
  // Fill in

  if (event == vtkMRMLScene::SceneClosedEvent)
    {
    }
}


//---------------------------------------------------------------------------
void vtkCollectFiducialsGUI::ProcessTimerEvents()
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
void vtkCollectFiducialsGUI::BuildGUI ( )
{

  // ---
  // MODULE GUI FRAME 
  // create a page
  this->UIPanel->AddPage ( "CollectFiducials", "CollectFiducials", NULL );

  BuildGUIForHelpFrame();
  BuildGUIForTrackerInfo();
  BuildGUIForFiducialListNode();  
}


void vtkCollectFiducialsGUI::BuildGUIForHelpFrame()
{
  // Define your help text here.
  const char *help = 
      "This module is used to collect patient fiducial markers using a tracking system. Details on the module can be found here: <a>http://www.slicer.org/slicerWiki/index.php/Modules:CollectFiducials-Documentation-3.6</a>";
  const char *about =
    "This was developed by Andrew Wiles at NDI, <a>http://www.ndigital.com</a>.";

  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "CollectFiducials" );
  this->BuildHelpAndAboutFrame (page, help, about);
}

void vtkCollectFiducialsGUI::BuildGUIForFiducialListNode()
{
  //vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("CollectFiducials");

  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Fiducial List Node Selection");
  //conBrowsFrame->CollapseFrame();
  this->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Fiducial List Child Frame.

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  //frame->SetLabelText ("Fiducial List Child Frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 frame->GetWidgetName() );

  // -----------------------------------------
  // Fiducials List Selector

  this->FiducialListSelector = vtkSlicerNodeSelectorWidget::New();
  this->FiducialListSelector->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, NULL);
  this->FiducialListSelector->SetNewNodeEnabled(1);
  this->FiducialListSelector->SetParent( frame->GetFrame() );
  this->FiducialListSelector->Create();
  this->FiducialListSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->FiducialListSelector->UpdateMenu();

  this->FiducialListSelector->SetBorderWidth(2);
  this->FiducialListSelector->SetLabelText( "Fiducial List: ");
  this->FiducialListSelector->SetBalloonHelpString("Output patient fiducial list for registration with image.");
  this->Script("pack %s -side top -anchor e -fill x -padx 20 -pady 4",
                this->FiducialListSelector->GetWidgetName());

  this->NumFiducialsEntry = vtkKWEntryWithLabel::New();
  this->NumFiducialsEntry->SetParent(frame->GetFrame());
  this->NumFiducialsEntry->Create();
  this->NumFiducialsEntry->SetWidth(40);
  this->NumFiducialsEntry->SetLabelWidth(30);
  this->NumFiducialsEntry->SetLabelText("Input Number of Fiducials:");
  this->NumFiducialsEntry->GetWidget()->SetValue ( "8" );
  this->Script(
               "pack %s -side top -anchor e -padx 2 -pady 2",
               this->NumFiducialsEntry->GetWidgetName());

  // -----------------------------------------
  // Reset Fiducials Button

  this->ResetFiducialsButton = vtkKWPushButton::New ( );
  this->ResetFiducialsButton->SetParent ( frame->GetFrame() );
  this->ResetFiducialsButton->Create ( );
  this->ResetFiducialsButton->SetText ("Initialize/Reset");
  this->ResetFiducialsButton->SetWidth (30);

  this->Script("pack %s -side top -anchor e -padx 2 -pady 2",
               this->ResetFiducialsButton->GetWidgetName());

  // add the multicolumn list to show the input points
  this->FiducialListMultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
  this->FiducialListMultiColumnList->SetParent ( frame->GetFrame() );
  this->FiducialListMultiColumnList->Create ( );
  this->FiducialListMultiColumnList->SetHeight(1);
  this->FiducialListMultiColumnList->GetWidget()->SetSelectionTypeToRow();
  this->FiducialListMultiColumnList->GetWidget()->MovableRowsOff();
  this->FiducialListMultiColumnList->GetWidget()->MovableColumnsOff();
  // set up the columns of data for each point
  // refer to the header file for order
  this->FiducialListMultiColumnList->GetWidget()->AddColumn("X");
  this->FiducialListMultiColumnList->GetWidget()->AddColumn("Y");
  this->FiducialListMultiColumnList->GetWidget()->AddColumn("Z");

  // make the selected column editable by checkbox -- from NEURONAV.
  //    this->PointPairMultiColumnList->GetWidget()->SetColumnEditWindowToCheckButton(this->SelectedColumn);

  // now set the attributes that are equal across the columns
  for (int col = 0; col < 3; col++)
    {
    this->FiducialListMultiColumnList->GetWidget()->SetColumnWidth(col, 10);

    this->FiducialListMultiColumnList->GetWidget()->SetColumnAlignmentToRight(col);
    this->FiducialListMultiColumnList->GetWidget()->ColumnEditableOff(col);
    }

  this->Script ( "pack %s -fill x -expand true -side top -anchor e",
                this->FiducialListMultiColumnList->GetWidgetName());

  // -----------------------------------------
  // Get New Measure Button

  this->GetNewMeasureButton = vtkKWPushButton::New ( );
  this->GetNewMeasureButton->SetParent ( frame->GetFrame() );
  this->GetNewMeasureButton->Create ( );
  this->GetNewMeasureButton->SetText ("Get New Measure");
  this->GetNewMeasureButton->SetWidth (30);

  this->Script("pack %s -side top -anchor e -padx 2 -pady 2",
               this->GetNewMeasureButton->GetWidgetName());

  // -----------------------------------------
  // Save Fiducials Button

  this->SaveFiducialsButton = vtkKWPushButton::New ( );
  this->SaveFiducialsButton->SetParent ( frame->GetFrame() );
  this->SaveFiducialsButton->Create ( );
  this->SaveFiducialsButton->SetText ("Save Fiducials");
  this->SaveFiducialsButton->SetWidth (30);

  this->Script("pack %s -side top -anchor e -padx 2 -pady 2",
               this->SaveFiducialsButton->GetWidgetName());

  conBrowsFrame->Delete();
  frame->Delete();
}

void vtkCollectFiducialsGUI::BuildGUIForTrackerInfo()
{
  //vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkKWWidget *page = this->UIPanel->GetPageWidget ("CollectFiducials");

  vtkSlicerModuleCollapsibleFrame *conBrowsFrame = vtkSlicerModuleCollapsibleFrame::New();

  conBrowsFrame->SetParent(page);
  conBrowsFrame->Create();
  conBrowsFrame->SetLabelText("Tracker Information");
  //conBrowsFrame->CollapseFrame();
  this->Script ("pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
               conBrowsFrame->GetWidgetName(), page->GetWidgetName());

  // -----------------------------------------
  // Fiducial List Child Frame.

  vtkKWFrameWithLabel *frame = vtkKWFrameWithLabel::New();
  frame->SetParent(conBrowsFrame->GetFrame());
  frame->Create();
  //frame->SetLabelText ("Tracker List Child Frame");
  this->Script ( "pack %s -side top -fill x -expand y -anchor w -padx 2 -pady 2",
                 frame->GetWidgetName() );

  // -----------------------------------------
  // Probe Transform Selector

  this->ProbeTransformSelector = vtkSlicerNodeSelectorWidget::New();
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

  conBrowsFrame->Delete();
  frame->Delete();
}

//----------------------------------------------------------------------------
void vtkCollectFiducialsGUI::UpdateAll()
{
}

void vtkCollectFiducialsGUI::ResetFiducialList()
{
  //std::cerr << "DEBUG: ResetFiducialsButton is pressed." << std::endl;

  this->FiducialListMultiColumnList->GetWidget()->DeleteAllRows();
  // note that the GetWidget() call returns the MultiColumnList object that contains all the data.

  int nFids = atoi(this->NumFiducialsEntry->GetWidget()->GetValue());
  this->FiducialListMultiColumnList->GetWidget()->AddRows(nFids);

  for (int i = 0; i < nFids; i++)
  {
    //std::cerr << "DEBUG: Fiducial No. " << i << std::endl;
    for (int j = 0; j < 3; j++)
    {
      this->FiducialListMultiColumnList->GetWidget()->SetCellTextAsDouble(i,j,0.0);
    } // for each coordinate
  }
  this->FiducialListMultiColumnList->GetWidget()->SelectRow(0);
}

void vtkCollectFiducialsGUI::GetNewFiducialMeasure()
{
  float x, y, z;
  int row;

  std::cerr << "GetNewMeasureButton is pressed." << std::endl;

  if(this->ProbeTransformSelector->GetSelected() == NULL )
    {
    return;
    }

  if(this->ProbeTransformSelector->GetSelected()->GetID())
  {
    this->GetLogic()->GetNewFiducialMeasure(this->ProbeTransformSelector->GetSelected()->GetID(), &x, &y, &z);
  }
  row = this->FiducialListMultiColumnList->GetWidget()->GetIndexOfFirstSelectedRow();
  this->FiducialListMultiColumnList->GetWidget()->SetCellTextAsDouble(row,0,double(x));
  this->FiducialListMultiColumnList->GetWidget()->SetCellTextAsDouble(row,1,double(y));
  this->FiducialListMultiColumnList->GetWidget()->SetCellTextAsDouble(row,2,double(z));
  if((row+1) < this->FiducialListMultiColumnList->GetWidget()->GetNumberOfRows())
  {
    this->FiducialListMultiColumnList->GetWidget()->DeselectRow(row);
    row++;
    this->FiducialListMultiColumnList->GetWidget()->SelectRow(row);
  }
  else
  {
    this->FiducialListMultiColumnList->GetWidget()->DeselectRow(row);
    this->FiducialListMultiColumnList->GetWidget()->SelectRow(0);
  }

}

void vtkCollectFiducialsGUI::SaveFiducialList()
{
  char name[50];
  std::cerr << "Save Fiducials Button is pressed." << std::endl;

  // remove all the fiducials currently present in the list.
  this->GetLogic()->ResetAllFiducials(this->FiducialListSelector->GetSelected()->GetID());

  // now go through the table in the GUI and add each new fiducial.
  int nFids = atoi(this->NumFiducialsEntry->GetWidget()->GetValue());

  if (nFids == this->FiducialListMultiColumnList->GetWidget()->GetNumberOfRows())
  {
    for(int i = 0; i<nFids; i++)
    {
      sprintf(name, "Probe-%02d", (i+1));
      this->GetLogic()->AddFiducial(this->FiducialListSelector->GetSelected()->GetID(), name,
                                       float(this->FiducialListMultiColumnList->GetWidget()->GetCellTextAsDouble(i,0)),
                                       float(this->FiducialListMultiColumnList->GetWidget()->GetCellTextAsDouble(i,1)),
                                       //float(i));
                                       float(this->FiducialListMultiColumnList->GetWidget()->GetCellTextAsDouble(i,2)));
    }
    //std::cerr << "DEBUG: Finsihed updating the fiducials." << std::endl;
  }
  else
  {
    vtkErrorMacro("CollectFiducialsGUI: The number of nodes in the entry box is different from the fiducial list table.");
    return;
  }
}
