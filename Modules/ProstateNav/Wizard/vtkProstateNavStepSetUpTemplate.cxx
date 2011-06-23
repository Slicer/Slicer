/*==========================================================================

  Portions (c) Copyright 2008 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $HeadURL: $
  Date:      $Date: $
  Version:   $Revision: $

==========================================================================*/

#include "vtkProstateNavStepSetUpTemplate.h"

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

#include "vtkMRMLRobotNode.h"
#include "vtkMRMLTransPerinealProstateRobotNode.h"

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkProstateNavStepSetUpTemplate);
vtkCxxRevisionMacro(vtkProstateNavStepSetUpTemplate, "$Revision: 1.1 $");

//----------------------------------------------------------------------------
vtkProstateNavStepSetUpTemplate::vtkProstateNavStepSetUpTemplate()
{

  //this->SetName("Configuration");
  this->SetTitle("Configuration");
  this->SetDescription("System configuration.");

  this->ConnectorFrame                  = NULL;
  this->ScannerConnectorSelector        = NULL;

}


//----------------------------------------------------------------------------
vtkProstateNavStepSetUpTemplate::~vtkProstateNavStepSetUpTemplate()
{

  // Connectors
  if (this->ConnectorFrame)
    {
    this->ConnectorFrame->SetParent(NULL);
    this->ConnectorFrame->Delete();
    this->ConnectorFrame = NULL;
    }

  if (this->ScannerConnectorSelector)
    {
    this->ScannerConnectorSelector->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->ScannerConnectorSelector->SetParent(NULL);
    this->ScannerConnectorSelector->Delete();
    this->ScannerConnectorSelector = NULL;
    }


}


//----------------------------------------------------------------------------
void vtkProstateNavStepSetUpTemplate::ShowUserInterface()
{

  this->Superclass::ShowUserInterface();
  vtkKWWizardWidget *wizardWidget = this->GetGUI()->GetWizardWidget();
  vtkKWWidget *parent = wizardWidget->GetClientArea();

  // Connector Frame

  if (!this->ConnectorFrame)
    {
    this->ConnectorFrame = vtkKWFrame::New();
    this->ConnectorFrame->SetParent ( parent );
    this->ConnectorFrame->Create ( );
    }

  this->Script ( "pack %s -side top -fill x",
                 this->ConnectorFrame->GetWidgetName());

  if (!this->ScannerConnectorSelector)
    {
    this->ScannerConnectorSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->ScannerConnectorSelector->SetParent(this->ConnectorFrame);
    this->ScannerConnectorSelector->Create();
    this->ScannerConnectorSelector->SetNodeClass("vtkMRMLIGTLConnectorNode", NULL, NULL, "ScannerConnector");
    this->ScannerConnectorSelector->SetMRMLScene(this->MRMLScene);
    this->ScannerConnectorSelector->SetBorderWidth(2);
    this->ScannerConnectorSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->ScannerConnectorSelector->GetWidget()->GetWidget()->SetWidth(24);
    this->ScannerConnectorSelector->SetNoneEnabled(1);
    this->ScannerConnectorSelector->SetNewNodeEnabled(1);
    this->ScannerConnectorSelector->SetLabelText( "Scanner Connector: ");
    this->ScannerConnectorSelector->SetBalloonHelpString("Select or create a target list.");
    this->ScannerConnectorSelector->ExpandWidgetOff();
    this->ScannerConnectorSelector->SetLabelWidth(18);

    this->ScannerConnectorSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    this->ScannerConnectorSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NewNodeEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }    

  this->Script("pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->ScannerConnectorSelector->GetWidgetName());

}


//----------------------------------------------------------------------------
void vtkProstateNavStepSetUpTemplate::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkProstateNavStepSetUpTemplate::ProcessGUIEvents( vtkObject *caller,
                                         unsigned long event, void *callData )
{

  //if (this->ScannerConnectorSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
  //    && (event == vtkSlicerNodeSelectorWidget::NewNodeEvent || event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent))
  //  {
  //  vtkMRMLIGTLConnectorNode* node = vtkMRMLIGTLConnectorNode::SafeDownCast(this->ScannerConnectorSelector->GetSelected());
  //  if (robotNode && node)
  //    {
  //    robotNode->SetAndObserveScannerConnectorNodeID(node->GetID());
  //    }
  //  }

}

