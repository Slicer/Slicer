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

  this->ConnectorFrame                  = NULL;
  this->RobotConnectorSelector          = NULL;
  this->ScannerConnectorSelector        = NULL;

}


//----------------------------------------------------------------------------
vtkProstateNavStepSetUp::~vtkProstateNavStepSetUp()
{

  // Target fiducials
  if (this->FiducialFrame)
    {
    this->FiducialFrame->SetParent(NULL);
    this->FiducialFrame->Delete();
    this->FiducialFrame = NULL;
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

  // Connectors
  if (this->ConnectorFrame)
    {
    this->ConnectorFrame->SetParent(NULL);
    this->ConnectorFrame->Delete();
    this->ConnectorFrame = NULL;
    }

  if (this->RobotConnectorSelector)
    {
    this->RobotConnectorSelector->RemoveObserver((vtkCommand *)this->GUICallbackCommand);
    this->RobotConnectorSelector->SetParent(NULL);
    this->RobotConnectorSelector->Delete();
    this->RobotConnectorSelector = NULL;
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
    this->TargetPlanFiducialSelector->SetNoneEnabled(0);
    this->TargetPlanFiducialSelector->SetNewNodeEnabled(1);
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
    this->TargetCompletedFiducialSelector->SetNoneEnabled(0);
    this->TargetCompletedFiducialSelector->SetNewNodeEnabled(1);
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


  // Connector Frame

  if (!this->ConnectorFrame)
    {
    this->ConnectorFrame = vtkKWFrame::New();
    this->ConnectorFrame->SetParent ( parent );
    this->ConnectorFrame->Create ( );
    }

  this->Script ( "pack %s -side top -fill x",
                 this->ConnectorFrame->GetWidgetName());

  if (!this->RobotConnectorSelector)
    {
    this->RobotConnectorSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->RobotConnectorSelector->SetParent(this->FiducialFrame);
    this->RobotConnectorSelector->Create();
    this->RobotConnectorSelector->SetNodeClass("vtkMRMLIGTLConnectorNode", NULL, NULL, "RobotConnector");
    this->RobotConnectorSelector->SetMRMLScene(this->MRMLScene);
    this->RobotConnectorSelector->SetBorderWidth(2);
    this->RobotConnectorSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->RobotConnectorSelector->GetWidget()->GetWidget()->SetWidth(24);
    this->RobotConnectorSelector->SetNoneEnabled(1);
    this->RobotConnectorSelector->SetNewNodeEnabled(1);
    this->RobotConnectorSelector->SetLabelText( "Robot connector: ");
    this->RobotConnectorSelector->SetBalloonHelpString("Select or create a target list.");
    this->RobotConnectorSelector->ExpandWidgetOff();
    this->RobotConnectorSelector->SetLabelWidth(18);

    this->RobotConnectorSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    this->RobotConnectorSelector
      ->AddObserver(vtkSlicerNodeSelectorWidget::NewNodeEvent,
                    (vtkCommand *)this->GUICallbackCommand );
    }    

  if (!this->ScannerConnectorSelector)
    {
    this->ScannerConnectorSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->ScannerConnectorSelector->SetParent(this->FiducialFrame);
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

  this->Script("pack %s %s -side top -anchor nw -fill x -padx 2 -pady 2",
               this->RobotConnectorSelector->GetWidgetName(),
               this->ScannerConnectorSelector->GetWidgetName());

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

  if (this->RobotConnectorSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && (event == vtkSlicerNodeSelectorWidget::NewNodeEvent || event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent))
    {
    vtkMRMLIGTLConnectorNode* node = vtkMRMLIGTLConnectorNode::SafeDownCast(this->RobotConnectorSelector->GetSelected());
    if (this->ProstateNavManager)
      {
      this->ProstateNavManager->SetAndObserveRobotConnector(node);
      }
    }

  if (this->ScannerConnectorSelector == vtkSlicerNodeSelectorWidget::SafeDownCast(caller)
      && (event == vtkSlicerNodeSelectorWidget::NewNodeEvent || event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent))
    {
    vtkMRMLIGTLConnectorNode* node = vtkMRMLIGTLConnectorNode::SafeDownCast(this->ScannerConnectorSelector->GetSelected());
    if (this->ProstateNavManager)
      {
      this->ProstateNavManager->SetAndObserveScannerConnector(node);
      }
    }

}



