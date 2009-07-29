/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavStepSetUp.h"

#include "vtkProstateNavGUI.h"
#include "vtkProstateNavLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWEntry.h"
#include "vtkKWCheckButton.h"
#include "vtkKWWizardWidget.h"
#include "vtkKWWizardWorkflow.h"
#include "vtkKWPushButton.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"

#include "vtkSlicerNodeSelectorWidget.h"

#include "vtkOpenIGTLinkIFLogic.h"
#include "vtkOpenIGTLinkIFGUI.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavStepSetUp);
vtkCxxRevisionMacro(vtkProstateNavStepSetUp, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavStepSetUp::vtkProstateNavStepSetUp()
{

  //this->SetName("Configuration");
  this->SetTitle("Configuration");
  this->SetDescription("System configuration.");

  this->FiducialFrame                   = NULL;
  this->TargetPlanFiducialSelector      = NULL;
  this->TargetCompletedFiducialSelector = NULL;

  this->RobotFrame          = NULL;
  this->RobotLabel1         = NULL;
  this->RobotLabel2         = NULL;
  this->RobotAddressEntry   = NULL;
  this->RobotPortEntry      = NULL;
  this->RobotConnectButton  = NULL;

  this->ScannerFrame        = NULL;
  this->ScannerLabel1       = NULL;
  this->ScannerLabel2       = NULL;
  this->ScannerAddressEntry = NULL;
  this->ScannerPortEntry    = NULL;
  this->ScannerConnectButton  = NULL;

}


//----------------------------------------------------------------------------
vtkProstateNavStepSetUp::~vtkProstateNavStepSetUp()
{

  if (this->RobotFrame)
    {
    this->RobotFrame->SetParent(NULL);
    this->RobotFrame->Delete();
    this->RobotFrame = NULL;
    }
  if (this->TargetPlanFiducialSelector)
    {
    this->TargetPlanFiducialSelector->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->TargetPlanFiducialSelector->SetParent(NULL);
    this->TargetPlanFiducialSelector->Delete();
    this->TargetPlanFiducialSelector = NULL;
    }
  if (this->TargetCompletedFiducialSelector)
    {
    this->TargetCompletedFiducialSelector->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->TargetCompletedFiducialSelector->SetParent(NULL);
    this->TargetCompletedFiducialSelector->Delete();
    this->TargetCompletedFiducialSelector = NULL;
    }
  if (this->RobotLabel1)
    {
    this->RobotLabel1->SetParent(NULL);
    this->RobotLabel1->Delete();
    this->RobotLabel1 = NULL;
    }
  if (this->RobotLabel2)
    {
    this->RobotLabel2->SetParent(NULL);
    this->RobotLabel2->Delete();
    this->RobotLabel2 = NULL;
    }
  if (this->RobotAddressEntry)
    {
    this->RobotAddressEntry->SetParent(NULL);
    this->RobotAddressEntry->Delete();
    this->RobotAddressEntry = NULL;
    }
  if (this->RobotPortEntry)
    {
    this->RobotPortEntry->SetParent(NULL);
    this->RobotPortEntry->Delete();
    this->RobotPortEntry = NULL;
    }
  if (this->RobotConnectButton)
    {
    this->RobotConnectButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->RobotConnectButton->SetParent(NULL);
    this->RobotConnectButton->Delete();
    this->RobotConnectButton = NULL;
    }
  if (this->ScannerFrame)
    {
    this->ScannerFrame->SetParent(NULL);
    this->ScannerFrame->Delete();
    this->ScannerFrame = NULL;
    }
  if (this->ScannerLabel1)
    {
    this->ScannerLabel1->SetParent(NULL);
    this->ScannerLabel1->Delete();
    this->ScannerLabel1 = NULL;
    }
  if (this->ScannerLabel2)
    {
    this->ScannerLabel2->SetParent(NULL);
    this->ScannerLabel2->Delete();
    this->ScannerLabel2 = NULL;
    }
  if (this->ScannerAddressEntry)
    {
    this->ScannerAddressEntry->SetParent(NULL);
    this->ScannerAddressEntry->Delete();
    this->ScannerAddressEntry = NULL;
    }
  if (this->ScannerPortEntry)
    {
    this->ScannerPortEntry->SetParent(NULL);
    this->ScannerPortEntry->Delete();
    this->ScannerPortEntry = NULL;
    }
  if (this->ScannerConnectButton)
    {
    this->ScannerConnectButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->ScannerConnectButton->SetParent(NULL);
    this->ScannerConnectButton->Delete();
    this->ScannerConnectButton = NULL;
    }
}


//----------------------------------------------------------------------------
void vtkProstateNavStepSetUp::ShowUserInterface()
{

  this->Superclass::ShowUserInterface();
  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  if (!this->FiducialFrame)
    {
    this->FiducialFrame = vtkKWFrame::New();
    this->FiducialFrame->SetParent ( parent );
    this->FiducialFrame->Create ( );
    }

  this->Script ( "pack %s -side top -fill x",
                 this->FiducialFrame->GetWidgetName());

  if (!this->TargetPlanFiducialSelector)
    {
    this->TargetPlanFiducialSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->TargetPlanFiducialSelector->SetParent(this->FiducialFrame);
    this->TargetPlanFiducialSelector->Create();
    this->TargetPlanFiducialSelector->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, "FiducialList");
    this->TargetPlanFiducialSelector->SetMRMLScene(this->MRMLScene);
    this->TargetPlanFiducialSelector->SetBorderWidth(2);
    this->TargetPlanFiducialSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TargetPlanFiducialSelector->GetWidget()->GetWidget()->SetWidth(24);
    this->TargetPlanFiducialSelector->SetNoneEnabled(1);
    this->TargetPlanFiducialSelector->SetNewNodeEnabled(0);
    this->TargetPlanFiducialSelector->SetLabelText( "Targets planned: ");
    this->TargetPlanFiducialSelector->SetBalloonHelpString("Select or create a target list.");
    this->TargetPlanFiducialSelector->ExpandWidgetOff();
    this->TargetPlanFiducialSelector->SetLabelWidth(18);

    this->TargetPlanFiducialSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    this->TargetPlanFiducialSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NewNodeEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }    

  if (!this->TargetCompletedFiducialSelector)
    {
    this->TargetCompletedFiducialSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->TargetCompletedFiducialSelector->SetParent(this->FiducialFrame);
    this->TargetCompletedFiducialSelector->Create();
    this->TargetCompletedFiducialSelector->SetNodeClass("vtkMRMLFiducialListNode", NULL, NULL, "FiducialList");
    this->TargetCompletedFiducialSelector->SetMRMLScene(this->MRMLScene);
    this->TargetCompletedFiducialSelector->SetBorderWidth(2);
    this->TargetCompletedFiducialSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->TargetCompletedFiducialSelector->GetWidget()->GetWidget()->SetWidth(24);
    this->TargetCompletedFiducialSelector->SetNoneEnabled(1);
    this->TargetCompletedFiducialSelector->SetNewNodeEnabled(0);
    this->TargetCompletedFiducialSelector->SetLabelText( "Targets completed: ");
    this->TargetCompletedFiducialSelector->SetBalloonHelpString("Select or create a target list.");
    this->TargetCompletedFiducialSelector->ExpandWidgetOff();
    this->TargetCompletedFiducialSelector->SetLabelWidth(18);

    this->TargetCompletedFiducialSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    this->TargetCompletedFiducialSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NewNodeEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }    

  this->Script("pack %s %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->TargetPlanFiducialSelector->GetWidgetName(),
               this->TargetCompletedFiducialSelector->GetWidgetName());

  if (!this->RobotFrame)
    {
    this->RobotFrame = vtkKWFrame::New();
    this->RobotFrame->SetParent ( parent );
    this->RobotFrame->Create ( );
    }

  this->Script ( "pack %s -side top -fill x",
                 this->RobotFrame->GetWidgetName());

  if (!this->RobotLabel1)
    {
    this->RobotLabel1 = vtkKWLabel::New();
    this->RobotLabel1->SetParent(this->RobotFrame);
    this->RobotLabel1->Create();
    this->RobotLabel1->SetWidth(15);
    this->RobotLabel1->SetText("Robot Addr: ");
    }

  if (!this->RobotAddressEntry)
    {
    this->RobotAddressEntry = vtkKWEntry::New();
    this->RobotAddressEntry->SetParent(this->RobotFrame);
    this->RobotAddressEntry->Create();
    this->RobotAddressEntry->SetWidth(15);
    }

  if (!this->RobotLabel2)
    {
    this->RobotLabel2 = vtkKWLabel::New();
    this->RobotLabel2->SetParent(this->RobotFrame);
    this->RobotLabel2->Create();
    this->RobotLabel2->SetWidth(1);
    this->RobotLabel2->SetText(":");
    }
  if (!this->RobotPortEntry)
    {
    this->RobotPortEntry = vtkKWEntry::New();
    this->RobotPortEntry->SetParent(this->RobotFrame);
    this->RobotPortEntry->Create();
    this->RobotPortEntry->SetWidth(10);
    this->RobotPortEntry->SetRestrictValueToInteger();
    }
  
  if (!this->RobotConnectButton)
    {
    this->RobotConnectButton = vtkKWPushButton::New();
    this->RobotConnectButton->SetParent (this->RobotFrame);
    this->RobotConnectButton->Create();
    this->RobotConnectButton->SetText("OFF");
    this->RobotConnectButton->SetBalloonHelpString("Connect to Robot");
    this->RobotConnectButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                          (vtkCommand *)this->GUICallbackCommand);
    }


  this->Script("pack %s %s %s %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->RobotLabel1->GetWidgetName(), this->RobotAddressEntry->GetWidgetName(),
               this->RobotLabel2->GetWidgetName(), this->RobotPortEntry->GetWidgetName(),
               this->RobotConnectButton->GetWidgetName());

  if (!this->ScannerFrame)
    {
    this->ScannerFrame = vtkKWFrame::New();
    this->ScannerFrame->SetParent ( parent );
    this->ScannerFrame->Create ( );
    }

  this->Script ( "pack %s -side top -fill x",  
                 this->ScannerFrame->GetWidgetName());

  if (!this->ScannerLabel1)
    {
    this->ScannerLabel1 = vtkKWLabel::New();
    this->ScannerLabel1->SetParent(this->ScannerFrame);
    this->ScannerLabel1->Create();
    this->ScannerLabel1->SetWidth(15);
    this->ScannerLabel1->SetText("Scanner Addr: ");
    }

  if (!this->ScannerAddressEntry)
    {
    this->ScannerAddressEntry = vtkKWEntry::New();
    this->ScannerAddressEntry->SetParent(this->ScannerFrame);
    this->ScannerAddressEntry->Create();
    this->ScannerAddressEntry->SetWidth(15);
    }


  if (!this->ScannerLabel2)
    {
    this->ScannerLabel2 = vtkKWLabel::New();
    this->ScannerLabel2->SetParent(this->ScannerFrame);
    this->ScannerLabel2->Create();
    this->ScannerLabel2->SetWidth(1);
    this->ScannerLabel2->SetText(":");
    }

  if (!this->ScannerPortEntry)
    {
    this->ScannerPortEntry = vtkKWEntry::New();
    this->ScannerPortEntry->SetParent(this->ScannerFrame);
    this->ScannerPortEntry->Create();
    this->ScannerPortEntry->SetWidth(10);
    this->ScannerPortEntry->SetRestrictValueToInteger();
    }

  if (!this->ScannerConnectButton)
    {
    this->ScannerConnectButton = vtkKWPushButton::New();
    this->ScannerConnectButton->SetParent (this->ScannerFrame);
    this->ScannerConnectButton->Create();
    this->ScannerConnectButton->SetText("OFF");
    this->ScannerConnectButton->SetBalloonHelpString("Connect to Scanner");
    this->ScannerConnectButton->AddObserver(vtkKWPushButton::InvokedEvent,
                                          (vtkCommand *)this->GUICallbackCommand);
    }
  
  this->Script("pack %s %s %s %s %s -side left -anchor w -fill x -padx 2 -pady 2", 
               this->ScannerLabel1->GetWidgetName(), this->ScannerAddressEntry->GetWidgetName(),
               this->ScannerLabel2->GetWidgetName(), this->ScannerPortEntry->GetWidgetName(),
               this->ScannerConnectButton->GetWidgetName());



}


//----------------------------------------------------------------------------
void vtkProstateNavStepSetUp::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavStepSetUp::ProcessGUIEvents( vtkObject *caller,
                                         unsigned long event, void *callData )
{

  if (this->TargetPlanFiducialSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && (event == vtkSlicerNodeSelectorWidget::NewNodeEvent || event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent))
    {
    vtkMRMLFiducialListNode* node = vtkMRMLFiducialListNode::SafeDownCast(this->TargetPlanFiducialSelector->GetSelected());
    if (this->ProstateNavManager)
      {
      this->ProstateNavManager->SetAndObserveTargetPlanList(node);
      }
    }

  if (this->TargetCompletedFiducialSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && (event == vtkSlicerNodeSelectorWidget::NewNodeEvent || event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent))
    {
    vtkMRMLFiducialListNode* node = vtkMRMLFiducialListNode::SafeDownCast(this->TargetCompletedFiducialSelector->GetSelected());
    if (this->ProstateNavManager)
      {
      this->ProstateNavManager->SetAndObserveTargetCompletedList(node);
      }
    }

  if (this->RobotConnectButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent )
    {
    if (strcmp(this->RobotConnectButton->GetText(), "OFF") == 0)
      {
      const char* address = this->RobotAddressEntry->GetValue();
      int port    = this->RobotPortEntry->GetValueAsInt();
      if (strlen(address) > 0 && port > 0)
        {
        vtkOpenIGTLinkIFGUI* igtlGUI = 
          vtkOpenIGTLinkIFGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())
                                            ->GetModuleGUIByName("OpenIGTLink IF"));
        if (igtlGUI)
          {
          //igtlGUI->GetLogic()->AddClientConnector("BRPRobot", address, port);
          this->RobotConnectButton->SetText("ON ");
          }
        }
      }
    else
      {
      this->RobotConnectButton->SetText("OFF");
      }
    }
  else if (this->ScannerConnectButton == vtkKWPushButton::SafeDownCast(caller) 
      && event == vtkKWPushButton::InvokedEvent )
    {
    if (strcmp(this->ScannerConnectButton->GetText(), "OFF") == 0)
      {
      const char* address = this->ScannerAddressEntry->GetValue();
      int port    = this->ScannerPortEntry->GetValueAsInt();
      if (strlen(address) > 0 && port > 0)
        {
        if (strlen(address) > 0 && port > 0)
          {
          vtkOpenIGTLinkIFGUI* igtlGUI = 
            vtkOpenIGTLinkIFGUI::SafeDownCast(vtkSlicerApplication::SafeDownCast(this->GetApplication())
                                              ->GetModuleGUIByName("OpenIGTLink IF"));
          if (igtlGUI)
            {
            //igtlGUI->GetLogic()->AddClientConnector("BRPScanner", address, port);
            this->ScannerConnectButton->SetText("ON ");
            }
          }
        }
      }
    else
      {
      this->ScannerConnectButton->SetText("OFF");
      }
    }

  /*
  if (this->LoadConfigButtonNT->GetWidget() == vtkKWLoadSaveButton::SafeDownCast(caller) 
           && event == vtkKWPushButton::InvokedEvent )
    {
    const char* filename = this->LoadConfigButtonNT->GetWidget()->GetFileName();
    if (filename)
      {
      const vtksys_stl::string fname(filename);
      this->ConfigFileEntryNT->SetValue(fname.c_str());
      }
    else
      {
      this->ConfigFileEntryNT->SetValue("");
      }
    }

  else if (this->ConnectCheckButtonNT == vtkKWCheckButton::SafeDownCast(caller) 
           && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {

    if (this->ConnectCheckButtonNT->GetSelectedState() && this->Logic)
      {
      // Activate NaviTrack Stream
      const char* filename = this->LoadConfigButtonNT->GetWidget()->GetFileName();
      this->Logic->ConnectTracker(filename);
      }
    else
      {
      // Deactivate NaviTrack Stream
      this->Logic->DisconnectTracker();
      }
    }
  */

}



