/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavConfigurationStep.h"

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

#include "vtkOpenIGTLinkIFLogic.h"
#include "vtkOpenIGTLinkIFGUI.h"


//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavConfigurationStep);
vtkCxxRevisionMacro(vtkProstateNavConfigurationStep, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavConfigurationStep::vtkProstateNavConfigurationStep()
{

  this->SetName("1/5. Configuration");
  this->SetDescription("Perform system configuration.");

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
vtkProstateNavConfigurationStep::~vtkProstateNavConfigurationStep()
{

  if (this->RobotFrame)
    {
    this->RobotFrame->SetParent(NULL);
    this->RobotFrame->Delete();
    }

  if (this->RobotLabel1)
    {
    this->RobotLabel1->SetParent(NULL);
    this->RobotLabel1->Delete();
    }

  if (this->RobotLabel2)
    {
    this->RobotLabel2->SetParent(NULL);
    this->RobotLabel2->Delete();
    }

  if (this->RobotAddressEntry)
    {
    this->RobotAddressEntry->SetParent(NULL);
    this->RobotAddressEntry->Delete();
    }

  if (this->RobotPortEntry)
    {
    this->RobotPortEntry->SetParent(NULL);
    this->RobotPortEntry->Delete();
    }

  if (this->RobotConnectButton)
    {
    this->RobotConnectButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->RobotConnectButton->SetParent(NULL);
    this->RobotConnectButton->Delete();
    }

  if (this->ScannerFrame)
    {
    this->ScannerFrame->SetParent(NULL);
    this->ScannerFrame->Delete();
    }
  if (this->ScannerLabel1)
    {
    this->ScannerLabel1->SetParent(NULL);
    this->ScannerLabel1->Delete();
    }
  if (this->ScannerLabel2)
    {
    this->ScannerLabel2->SetParent(NULL);
    this->ScannerLabel2->Delete();
    }
  if (this->ScannerAddressEntry)
    {
    this->ScannerAddressEntry->SetParent(NULL);
    this->ScannerAddressEntry->Delete();
    }
  if (this->ScannerPortEntry)
    {
    this->ScannerPortEntry->SetParent(NULL);
    this->ScannerPortEntry->Delete();
    }
  if (this->ScannerConnectButton)
    {
    this->ScannerConnectButton->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->ScannerConnectButton->SetParent(NULL);
    this->ScannerConnectButton->Delete();
    }

}


//----------------------------------------------------------------------------
void vtkProstateNavConfigurationStep::ShowUserInterface()
{

  this->Superclass::ShowUserInterface();
  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

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
void vtkProstateNavConfigurationStep::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavConfigurationStep::ProcessGUIEvents( vtkObject *caller,
                                         unsigned long event, void *callData )
{

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
          igtlGUI->GetLogic()->AddClientConnector("BRPRobot", address, port);
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
            igtlGUI->GetLogic()->AddClientConnector("BRPScanner", address, port);
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



