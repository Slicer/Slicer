/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkIntensityNormalizationFilterGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkIntensityNormalizationFilterGUI.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWPushButton.h"

//------------------------------------------------------------------------------
vtkIntensityNormalizationFilterGUI* vtkIntensityNormalizationFilterGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkIntensityNormalizationFilterGUI");
  if(ret)
    {
      return (vtkIntensityNormalizationFilterGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkIntensityNormalizationFilterGUI;
}


//----------------------------------------------------------------------------
vtkIntensityNormalizationFilterGUI::vtkIntensityNormalizationFilterGUI()
{
  this->ConductanceScale = vtkKWScaleWithEntry::New();
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->GADNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->IntensityNormalizationFilterNode = NULL;
}

//----------------------------------------------------------------------------
vtkIntensityNormalizationFilterGUI::~vtkIntensityNormalizationFilterGUI()
{
    
    if ( this->ConductanceScale ) {
        this->ConductanceScale->SetParent(NULL);
        this->ConductanceScale->Delete();
        this->ConductanceScale = NULL;
    }
    if ( this->TimeStepScale ) {
        this->TimeStepScale->SetParent(NULL);
        this->TimeStepScale->Delete();
        this->TimeStepScale = NULL;
    }
    if ( this->NumberOfIterationsScale ) {
        this->NumberOfIterationsScale->SetParent(NULL);
        this->NumberOfIterationsScale->Delete();
        this->NumberOfIterationsScale = NULL;
    }
    if ( this->VolumeSelector ) {
        this->VolumeSelector->SetParent(NULL);
        this->VolumeSelector->Delete();
        this->VolumeSelector = NULL;
    }
    if ( this->OutVolumeSelector ) {
        this->OutVolumeSelector->SetParent(NULL);
        this->OutVolumeSelector->Delete();
        this->OutVolumeSelector = NULL;
    }
    if ( this->GADNodeSelector ) {
        this->GADNodeSelector->SetParent(NULL);
        this->GADNodeSelector->Delete();
        this->GADNodeSelector = NULL;
    }
    if ( this->ApplyButton ) {
        this->ApplyButton->SetParent(NULL);
        this->ApplyButton->Delete();
        this->ApplyButton = NULL;
    }

  this->SetLogic (NULL);
  vtkSetMRMLNodeMacro(this->IntensityNormalizationFilterNode, NULL);

}

//----------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::AddGUIObservers ( ) 
{
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->GADNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::RemoveGUIObservers ( )
{
  this->ConductanceScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TimeStepScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimeStepScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NumberOfIterationsScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NumberOfIterationsScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->GADNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if ( s == this->ConductanceScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (s == this->TimeStepScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (s == this->NumberOfIterationsScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->VolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    }
  else if (selector == this->OutVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->OutVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    }
  if (selector == this->GADNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->GADNodeSelector->GetSelected() != NULL) 
    { 
    vtkMRMLIntensityNormalizationFilterNode* n = vtkMRMLIntensityNormalizationFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
    this->Logic->SetAndObserveIntensityNormalizationFilterNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->IntensityNormalizationFilterNode, n);
    this->UpdateGUI();
    }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Apply();
    }
  
}

//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::UpdateMRML ()
{
  vtkMRMLIntensityNormalizationFilterNode* n = this->GetIntensityNormalizationFilterNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->GADNodeSelector->SetSelectedNew("vtkMRMLIntensityNormalizationFilterNode");
    this->GADNodeSelector->ProcessNewNodeCommand("vtkMRMLIntensityNormalizationFilterNode", "GADParameters");
    n = vtkMRMLIntensityNormalizationFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveIntensityNormalizationFilterNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->IntensityNormalizationFilterNode, n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  // set node parameters from GUI widgets
  n->SetConductance(this->ConductanceScale->GetValue());
  
  n->SetTimeStep(this->TimeStepScale->GetValue());
  
  n->SetNumberOfIterations((int)floor(this->NumberOfIterationsScale->GetValue()));
  
  if (this->VolumeSelector->GetSelected() != NULL)
    {
    n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }
}

//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::UpdateGUI ()
{
  vtkMRMLIntensityNormalizationFilterNode* n = this->GetIntensityNormalizationFilterNode();
  if (n != NULL)
    {
    // set GUI widgest from parameter node
    this->ConductanceScale->SetValue(n->GetConductance());
    
    this->TimeStepScale->SetValue(n->GetTimeStep());
    
    this->NumberOfIterationsScale->SetValue(n->GetNumberOfIterations());
    }
}

//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLIntensityNormalizationFilterNode* node = vtkMRMLIntensityNormalizationFilterNode::SafeDownCast(caller);
  if (node != NULL && this->GetIntensityNormalizationFilterNode() == node) 
    {
    this->UpdateGUI();
    }
}




//---------------------------------------------------------------------------
void vtkIntensityNormalizationFilterGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  vtkMRMLIntensityNormalizationFilterNode* gadNode = vtkMRMLIntensityNormalizationFilterNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(gadNode);
  gadNode->Delete();

  this->UIPanel->AddPage ( "IntensityNormalizationFilter", "IntensityNormalizationFilter", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
   // Define your help text and build the help frame here.
    const char *help = "The IntensityNormalizationFilter module....";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "IntensityNormalizationFilter" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "IntensityNormalizationFilter" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Intensity Normalization Filter");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("IntensityNormalizationFilter")->GetWidgetName());
  
  this->GADNodeSelector->SetNodeClass("vtkMRMLIntensityNormalizationFilterNode", NULL, NULL, "GADParameters");
  this->GADNodeSelector->SetNewNodeEnabled(1);
  this->GADNodeSelector->NoneEnabledOn();
  this->GADNodeSelector->SetShowHidden(1);
  this->GADNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->GADNodeSelector->Create();
  this->GADNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->GADNodeSelector->UpdateMenu();

  this->GADNodeSelector->SetBorderWidth(2);
  this->GADNodeSelector->SetLabelText( "IN Parameters");
  this->GADNodeSelector->SetBalloonHelpString("select a GAD node from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->GADNodeSelector->GetWidgetName());


  this->ConductanceScale->SetParent( moduleFrame->GetFrame() );
  this->ConductanceScale->SetLabelText("NormValue");
  this->ConductanceScale->Create();
  int w = this->ConductanceScale->GetScale()->GetWidth ( );
  this->ConductanceScale->SetRange(0,200);
  this->ConductanceScale->SetResolution (1);
  this->ConductanceScale->SetValue(80);
  
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->ConductanceScale->GetWidgetName());

  this->NumberOfIterationsScale->SetParent( moduleFrame->GetFrame() );
  this->NumberOfIterationsScale->SetLabelText("Histo Smooth With Min");
  this->NumberOfIterationsScale->Create();
  this->NumberOfIterationsScale->GetScale()->SetWidth ( w );
  this->NumberOfIterationsScale->SetValue(5);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->NumberOfIterationsScale->GetWidgetName());


  this->TimeStepScale->SetParent( moduleFrame->GetFrame() );
  this->TimeStepScale->SetLabelText("Histo Smooth Width Max");
  this->TimeStepScale->Create();
  this->TimeStepScale->GetScale()->SetWidth ( w );
  //this->TimeStepScale->SetRange(0.0, 20);
  this->TimeStepScale->SetValue(10);
  this->TimeStepScale->SetResolution (1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TimeStepScale->GetWidgetName());



  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetLabelText( "Input Volume: ");
  this->VolumeSelector->SetBalloonHelpString("select an input volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->VolumeSelector->GetWidgetName());
  
  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, "GADVolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText( "Output Volume: ");
  this->OutVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->OutVolumeSelector->GetWidgetName());


  this->ApplyButton->SetParent( moduleFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());

  moduleFrame->Delete();

  
}
