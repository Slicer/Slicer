/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkThresholdingFilterGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkThresholdingFilterGUI.h"

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
#include "vtkKWFrameWithLabel.h"
#include "vtkKWNotebook.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWSpinBox.h"

#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSlicesControlGUI.h"
#include "vtkKWScale.h"

//------------------------------------------------------------------------------
vtkThresholdingFilterGUI* vtkThresholdingFilterGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkThresholdingFilterGUI");
  if(ret)
    {
      return (vtkThresholdingFilterGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkThresholdingFilterGUI;
}


//----------------------------------------------------------------------------
vtkThresholdingFilterGUI::vtkThresholdingFilterGUI()
{
  this->ConductanceScale = vtkKWScaleWithEntry::New();
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  /*this->StorageVolumeSelector = vtkSlicerNodeSelectorWidget::New();*/
  this->GADNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->CatchButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->ThresholdingFilterNode = NULL;
  
  this->Sagittal1Scale = vtkKWScaleWithEntry::New();
  this->Sagittal2Scale = vtkKWScaleWithEntry::New();
  
  this->Coronal1Scale = vtkKWScaleWithEntry::New();
  this->Coronal2Scale = vtkKWScaleWithEntry::New();
  
  this->VolumeSelectionFrame = vtkKWFrameWithLabel::New();
  this->ThresholdingFrame = vtkKWFrameWithLabel::New();
  
  this->Notebook = vtkKWNotebook::New();
  
  this->ParametersFrame = vtkKWFrameWithLabel::New();
  this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
  
  this->ShrinkFactor = vtkKWScaleWithEntry::New();
  this->MaxNumOfIt   = vtkKWScaleWithEntry::New();
  this->NumOfFitLev  = vtkKWScaleWithEntry::New();
  this->WienFilNoi   = vtkKWScaleWithEntry::New();
  this->BiasField    = vtkKWScaleWithEntry::New();
  this->Conv         = vtkKWScaleWithEntry::New();
  
  this->SlidePositionScale = vtkKWScaleWithEntry::New();

  this->SlidePositionScale2 = vtkKWScaleWithEntry::New();

  this->BiasIntensityScale         = vtkKWScaleWithEntry::New();
}

//----------------------------------------------------------------------------
vtkThresholdingFilterGUI::~vtkThresholdingFilterGUI()
{
   if ( this->BiasIntensityScale ) {
        this->BiasIntensityScale->SetParent(NULL);
        this->BiasIntensityScale->Delete();
        this->BiasIntensityScale = NULL;
    }
    
    if ( this->SlidePositionScale2 ) {
        this->SlidePositionScale2->SetParent(NULL);
        this->SlidePositionScale2->Delete();
        this->SlidePositionScale2 = NULL;
    }

   if ( this->SlidePositionScale ) {
        this->SlidePositionScale->SetParent(NULL);
        this->SlidePositionScale->Delete();
        this->SlidePositionScale = NULL;
    }
   

  if ( this->MaxNumOfIt ) {
        this->MaxNumOfIt->SetParent(NULL);
        this->MaxNumOfIt->Delete();
        this->MaxNumOfIt = NULL;
    }
    
   if ( this->NumOfFitLev ) {
        this->NumOfFitLev->SetParent(NULL);
        this->NumOfFitLev->Delete();
        this->NumOfFitLev = NULL;
    }
    
  if ( this->WienFilNoi ) {
        this->WienFilNoi->SetParent(NULL);
        this->WienFilNoi->Delete();
        this->WienFilNoi = NULL;
    }
    
  if ( this->BiasField ) {
        this->BiasField->SetParent(NULL);
        this->BiasField->Delete();
        this->BiasField = NULL;
    }     

  if ( this->Conv ) {
        this->Conv->SetParent(NULL);
        this->Conv->Delete();
        this->Conv = NULL;
    }


   if ( this->ShrinkFactor ) {
        this->ShrinkFactor->SetParent(NULL);
        this->ShrinkFactor->Delete();
        this->ShrinkFactor = NULL;
    }

    if ( this->ParametersFrame ) {
        this->ParametersFrame->SetParent(NULL);
        this->ParametersFrame->Delete();
        this->ParametersFrame = NULL;
    }

    if ( this->AdvancedParametersFrame ) {
        this->AdvancedParametersFrame->SetParent(NULL);
        this->AdvancedParametersFrame->Delete();
        this->AdvancedParametersFrame = NULL;
    }
    
    
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
   /*if ( this->StorageVolumeSelector ) {
        this->StorageVolumeSelector->SetParent(NULL);
        this->StorageVolumeSelector->Delete();
        this->StorageVolumeSelector = NULL;
    }*/
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
    if ( this->CatchButton ) {
        this->CatchButton->SetParent(NULL);
        this->CatchButton->Delete();
        this->CatchButton = NULL;
    }
    
    if (this->VolumeSelectionFrame ) {
        this->VolumeSelectionFrame->SetParent(NULL);
        this->VolumeSelectionFrame->Delete();
        this->VolumeSelectionFrame = NULL;
    }
    
    if (this->Sagittal1Scale ) {
        this->Sagittal1Scale->SetParent(NULL);
        this->Sagittal1Scale->Delete();
        this->Sagittal1Scale = NULL;
    }
    if (this->Sagittal2Scale ) {
        this->Sagittal2Scale->SetParent(NULL);
        this->Sagittal2Scale->Delete();
        this->Sagittal2Scale = NULL;
    }
    if (this->Coronal1Scale ) {
        this->Coronal1Scale->SetParent(NULL);
        this->Coronal1Scale->Delete();
        this->Coronal1Scale = NULL;
    }
    if (this->Coronal2Scale ) {
        this->Coronal2Scale->SetParent(NULL);
        this->Coronal2Scale->Delete();
        this->Coronal2Scale = NULL;
    }
    
    if (this->ThresholdingFrame ) {
        this->ThresholdingFrame->SetParent(NULL);
        this->ThresholdingFrame->Delete();
        this->ThresholdingFrame = NULL;
    }

  this->SetLogic (NULL);
  vtkSetMRMLNodeMacro(this->ThresholdingFilterNode, NULL);

}

//----------------------------------------------------------------------------
void vtkThresholdingFilterGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::AddGUIObservers ( ) 
{
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand ); 
  
  /*this->StorageVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
*/
  this->GADNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->CatchButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->Sagittal1Scale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->Sagittal1Scale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->SlidePositionScale2->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SlidePositionScale2->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::RemoveGUIObservers ( )
{
  this->ConductanceScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->RemoveObservers (vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );
  /*
  this->StorageVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
*/
  this->GADNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  this->CatchButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
  
  this->Sagittal1Scale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->Sagittal1Scale->RemoveObservers (vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->SlidePositionScale2->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SlidePositionScale2->RemoveObservers (vtkKWScale::ScaleValueChangingEvent, (vtkCommand *)this->GUICallbackCommand );

}



//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if ( s == this->ConductanceScale && event == vtkKWScale::ScaleValueChangingEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Preview();
    this->UpdateGUI();
    std::cout<<"ScaleValue changing"<<std::endl;
    }

  if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent &&
    this->VolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    this->ConductanceScale->SetRange(this->Logic->InitMinThreshold(),this->Logic->InitMaxThreshold()/3+1);
    this->Logic->Cut();
    this->UpdateGUI();
    std::cout<<"Input changed"<<std::endl;
    }
  else if (selector == this->OutVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->OutVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML(); 
    this->UpdateGUI();
    }
   /*else if (selector == this->StorageVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->StorageVolumeSelector->GetSelected() != NULL) 
    { 
    this->UpdateMRML();
    this->Logic->Cut();
    this->UpdateGUI();
    std::cout<<"store changed"<<std::endl;
    }*/
  if (selector == this->GADNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent  &&
    this->GADNodeSelector->GetSelected() != NULL) 
    { 
    vtkMRMLThresholdingFilterNode* n = vtkMRMLThresholdingFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());
    this->Logic->SetAndObserveThresholdingFilterNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->ThresholdingFilterNode, n);
    this->UpdateGUI();
    std::cout<<"node changed"<<std::endl;
    }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Apply();
    this->UpdateGUI();
    }
    
  else if (b == this->CatchButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->ApplyPreview();
    this->UpdateGUI();
      /*    vtkSlicerApplicationGUI *applicationGUI     = this->GetApplicationGUI();

      double oldSliceSetting;
      oldSliceSetting = double(applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->GetOffsetScale()->GetValue());
      //std::cout<<oldSliceSetting<<std::endl;
      this->Logic->Cut(oldSliceSetting);
    //this->Logic->Apply();*/
    }
   else if (s == this->Sagittal1Scale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->ChangeSlide();
    this->Logic->Preview();
    this->UpdateGUI();
    //this->Logic->Apply();
    std::cout<<"ScaleValue changed"<<std::endl;
    }
    else if (s == this->SlidePositionScale2 && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->ChangeSlide2();
    //this->Logic->Preview();
    this->UpdateGUI();
    //this->Logic->Apply();
    std::cout<<"ScaleValue changed para frame"<<std::endl;
    }
  
}

//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::UpdateMRML ()
{
  vtkMRMLThresholdingFilterNode* n = this->GetThresholdingFilterNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->GADNodeSelector->SetSelectedNew("vtkMRMLThresholdingFilterNode");
    this->GADNodeSelector->ProcessNewNodeCommand("vtkMRMLThresholdingFilterNode", "GADParameters");
    n = vtkMRMLThresholdingFilterNode::SafeDownCast(this->GADNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetAndObserveThresholdingFilterNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->ThresholdingFilterNode, n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  // set node parameters from GUI widgets
  n->SetConductance(this->ConductanceScale->GetValue());

  n->SetSagittal1((int)floor(this->Sagittal1Scale->GetValue()));

  n->SetPosition2((int)floor(this->SlidePositionScale2->GetValue()));  
  
  n->SetShrink((int)floor(this->ShrinkFactor->GetValue()));  
  
  n->SetMax((int)floor(this->MaxNumOfIt->GetValue()));  
  
  n->SetNum(this->NumOfFitLev->GetValue());
  
  n->SetWien(this->WienFilNoi->GetValue());
  
  n->SetField(this->BiasField->GetValue());
  
  n->SetCon(this->Conv->GetValue());
 
  if (this->VolumeSelector->GetSelected() != NULL)
    {
    n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
    }

  if (this->OutVolumeSelector->GetSelected() != NULL)
    {
    n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
    }
  /*if (this->StorageVolumeSelector->GetSelected() != NULL)
    {
    n->SetOutputVolumeRef(this->StorageVolumeSelector->GetSelected()->GetID());
    }*/
}

//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::UpdateGUI ()
{
  vtkMRMLThresholdingFilterNode* n = this->GetThresholdingFilterNode();
  if (n != NULL)
    {
    std::cout<<"Update GUI"<<std::endl;
    // set GUI widgest from parameter node
    this->ConductanceScale->SetValue(n->GetConductance());
    
    this->Sagittal1Scale->SetValue(n->GetSagittal1());
    
    this->SlidePositionScale2->SetValue(n->GetPosition2());

    this->ShrinkFactor->SetValue(n->GetShrink());
    
    this->MaxNumOfIt->SetValue(n->GetMax());
    
    this->NumOfFitLev->SetValue(n->GetNum());
    
    this->WienFilNoi->SetValue(n->GetWien());
    
    this->BiasField->SetValue(n->GetField());
    
    this->Conv->SetValue(n->GetCon());
/*
    vtkSlicerApplicationLogic *applicationLogic = this->Logic->GetApplicationLogic();
    applicationLogic->GetSelectionNode()->SetReferenceActiveVolumeID(this->ThresholdingFilterNode->GetOutputVolumeRef());
    applicationLogic->PropagateVolumeSelection();
    
    vtkSlicerApplicationGUI *applicationGUI     = this->GetApplicationGUI();
    
    applicationGUI->GetMainSliceGUI("Red")->GetSliceController()->FitSliceToBackground(1);
  */     
    }
}

//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLThresholdingFilterNode* node = vtkMRMLThresholdingFilterNode::SafeDownCast(caller);
  if (node != NULL && this->GetThresholdingFilterNode() == node) 
    {
    this->UpdateGUI();
    //this->Logic->Cut();
    std::cout<<"MRML changed"<<std::endl;
    }
}




//---------------------------------------------------------------------------
void vtkThresholdingFilterGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  vtkMRMLThresholdingFilterNode* gadNode = vtkMRMLThresholdingFilterNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(gadNode);
  gadNode->Delete();

  this->UIPanel->AddPage ( "ThresholdingFilter", "ThresholdingFilter", NULL );
  // ---
  // MODULE GUI FRAME 
  // ---
   // Define your help text and build the help frame here.
    const char *help = "The ThresholdingFilter module....";
    const char *about = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. See <a>http://www.slicer.org</a> for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "ThresholdingFilter" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    
    
    
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "ThresholdingFilter" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Bias Correction Module");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("ThresholdingFilter")->GetWidgetName());
  
  
  
  this->GADNodeSelector->SetNodeClass("vtkMRMLThresholdingFilterNode", NULL, NULL, "GADParameters");
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
  /*
  this->StorageVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, "GADVolumeStorage");
  this->StorageVolumeSelector->SetNewNodeEnabled(1);
  this->StorageVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->StorageVolumeSelector->Create();
  this->StorageVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->StorageVolumeSelector->UpdateMenu();

  this->StorageVolumeSelector->SetBorderWidth(2);
  this->StorageVolumeSelector->SetLabelText( "Storage Volume: ");
  this->StorageVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->StorageVolumeSelector->GetWidgetName());*/
////////////////////////////////////////////////////////////////////////////////////////////  
  
  this->Notebook->SetParent(moduleFrame->GetFrame());
  this->Notebook->Create();
  this->Notebook->SetMinimumWidth(300);
  this->Notebook->SetMinimumHeight(300);
  this->Notebook->AddPage("Mask Creation");
  this->Notebook->AddPage("Bias Correction");
  
  app->Script("pack %s -side top -anchor nw -expand y -padx 2 -pady 2",this->Notebook->GetWidgetName());

////////////////////////////////////////////////////////////////////////////////////////////
  

    vtkKWFrame *mask_page = 
    this->Notebook->GetFrame("Mask Creation");
  vtkKWFrame *bias_page = 
    this->Notebook->GetFrame("Bias Correction");

////////////////////////////////////////////////////////////////////////////////////////////

/*
  if (!this->VolumeSelectionFrame)
    {
    this->VolumeSelectionFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->VolumeSelectionFrame->IsCreated())
    {
    this->VolumeSelectionFrame->SetParent(this->Notebook->GetFrame(mask_page));
    this->VolumeSelectionFrame->Create();
    this->VolumeSelectionFrame->SetLabelText(
      "Volume selection");
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->VolumeSelectionFrame->GetWidgetName());

  vtkKWFrame* selectionFrame = this->VolumeSelectionFrame->GetFrame();
*/
/*

  this->NumberOfIterationsScale->SetParent( selectionFrame);
  this->NumberOfIterationsScale->SetLabelText("R -> L");
  this->NumberOfIterationsScale->Create();
  this->NumberOfIterationsScale->SetRange(0,1000);
  this->NumberOfIterationsScale->SetResolution (1);
  this->NumberOfIterationsScale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->NumberOfIterationsScale->GetWidgetName());


  this->TimeStepScale->SetParent( selectionFrame);
  this->TimeStepScale->SetLabelText("L -> R");
  this->TimeStepScale->Create();
  this->TimeStepScale->SetRange(0,1000);
  this->TimeStepScale->SetResolution (1);
  this->TimeStepScale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TimeStepScale->GetWidgetName());
              
  this->Sagittal1Scale->SetParent( selectionFrame);
  this->Sagittal1Scale->SetLabelText("S -> I");
  this->Sagittal1Scale->Create();
  this->Sagittal1Scale->SetRange(0,10);
  this->Sagittal1Scale->SetResolution (1);
  this->Sagittal1Scale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->Sagittal1Scale->GetWidgetName());
                
              
  this->Sagittal2Scale->SetParent( selectionFrame);
  this->Sagittal2Scale->SetLabelText("I -> S");
  this->Sagittal2Scale->Create();
  this->Sagittal2Scale->SetRange(0,1000);
  this->Sagittal2Scale->SetResolution (1);
  this->Sagittal2Scale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->Sagittal2Scale->GetWidgetName());
                
  this->Coronal1Scale->SetParent( selectionFrame);
  this->Coronal1Scale->SetLabelText("P -> A");
  this->Coronal1Scale->Create();
  this->Coronal1Scale->SetRange(0,1000);
  this->Coronal1Scale->SetResolution (1);
  this->Coronal1Scale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->Coronal1Scale->GetWidgetName());
                
                
  this->Coronal2Scale->SetParent( selectionFrame);
  this->Coronal2Scale->SetLabelText("A -> P");
  this->Coronal2Scale->Create();
  this->Coronal2Scale->SetRange(0,1000);
  this->Coronal2Scale->SetResolution (1);
  this->Coronal2Scale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->Coronal2Scale->GetWidgetName());
*/
  // Create the volume selection frame

////////////////////////////////////////////////////////////////////////////////////////////

 if (!this->ThresholdingFrame)
    {
    this->ThresholdingFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->ThresholdingFrame->IsCreated())
    {
    this->ThresholdingFrame->SetParent(mask_page);
    this->ThresholdingFrame->Create();
    this->ThresholdingFrame->SetLabelText(
      "Thresholding");
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->ThresholdingFrame->GetWidgetName());

  vtkKWFrame* treshFrame = this->ThresholdingFrame->GetFrame();




 if (!this->ConductanceScale)
    {
    this->ConductanceScale = vtkKWScaleWithEntry::New();
    }
 if (!this->ConductanceScale->IsCreated())
    {
  this->ConductanceScale->SetParent( treshFrame);
  this->ConductanceScale->Create();
  this->ConductanceScale->SetRange(0,1000);
  this->ConductanceScale->SetResolution (1);
  this->ConductanceScale->SetValue(1);
  this->ConductanceScale->SetLabelText("threshold");
  }
  
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->ConductanceScale->GetWidgetName());
      
      
      
  this->Sagittal1Scale->SetParent( treshFrame);
  this->Sagittal1Scale->SetLabelText("Slice Position");
  this->Sagittal1Scale->Create();
  this->Sagittal1Scale->SetRange(1,5);
  this->Sagittal1Scale->SetResolution (1);
  this->Sagittal1Scale->SetValue(1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->Sagittal1Scale->GetWidgetName());


  


      /*          
  if (!this->SlidePositionScale)
    {
    this->SlidePositionScale = vtkKWScaleWithEntry::New();
    }
 if (!this->SlidePositionScale->IsCreated())
    {
  this->SlidePositionScale->SetParent( treshFrame);
  this->SlidePositionScale->Create();
  this->SlidePositionScale->SetRange(0,10);
  this->SlidePositionScale->SetResolution (1);
  this->SlidePositionScale->SetValue(1);
  this->SlidePositionScale->SetLabelText("Slide position");
  }
  
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->SlidePositionScale->GetWidgetName());               
                */
////////////////////////////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////////////////////////////

 if (!this->ParametersFrame)
    {
    this->ParametersFrame = vtkKWFrameWithLabel::New();
    }
  if (!this->ParametersFrame->IsCreated())
    {
    this->ParametersFrame->SetParent(bias_page);
    this->ParametersFrame->Create();
    this->ParametersFrame->SetLabelText(
      "Preview");
   } 
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->ParametersFrame->GetWidgetName());

  vtkKWFrame* parametersFrame = this->ParametersFrame->GetFrame();
  /*
  if (!this->BiasIntensityScale)
    {
    this->BiasIntensityScale = vtkKWScaleWithEntry::New();
    }
 if (!this->BiasIntensityScale->IsCreated())
    {
    this->BiasIntensityScale->SetParent(parametersFrame);
    this->BiasIntensityScale->Create();
    this->BiasIntensityScale->SetRange(0,10);
    this->BiasIntensityScale->SetResolution(0.1);
    this->BiasIntensityScale->SetValue(3);
    this->BiasIntensityScale->SetLabelText(
      "Strength of the Bias");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->BiasIntensityScale->GetWidgetName());
  */
  this->SlidePositionScale2->SetParent( parametersFrame);
  this->SlidePositionScale2->SetLabelText("Slice Position");
  this->SlidePositionScale2->Create();
  this->SlidePositionScale2->SetRange(1,5);
  this->SlidePositionScale2->SetResolution (1);
  this->SlidePositionScale2->SetValue(1);
  app->Script("pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
                this->SlidePositionScale2->GetWidgetName());
  
  this->CatchButton->SetParent(bias_page);//parametersFrame);
  this->CatchButton->Create();
  this->CatchButton->SetText("Preview");
  this->CatchButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->CatchButton->GetWidgetName());

 ////////////////////////////////////////////////////////////////////////////////////////////

 if (!this->AdvancedParametersFrame)
    {
    this->AdvancedParametersFrame = vtkKWFrameWithLabel::New();
   }
  if (!this->AdvancedParametersFrame->IsCreated())
    {
    this->AdvancedParametersFrame->SetParent(bias_page);
    this->AdvancedParametersFrame->Create();
    //this->AdvancedParametersFrame->CollapseFrame();
    this->AdvancedParametersFrame->SetLabelText(
      "Parameters");
    }
    
    this->Script(
    "pack %s -side top -anchor nw -fill both -padx 2 -pady 2", 
    this->AdvancedParametersFrame->GetWidgetName());

  vtkKWFrame* aparametersFrame = this->AdvancedParametersFrame->GetFrame();
////////////////////////////////////////////////////////////////////////////////////////////


if (!this->ShrinkFactor)
    {
    this->ShrinkFactor = vtkKWScaleWithEntry::New();
    }
 if (!this->ShrinkFactor->IsCreated())
    {
    this->ShrinkFactor->SetParent(aparametersFrame);
    this->ShrinkFactor->Create();
    this->ShrinkFactor->SetRange(0,10);
    this->ShrinkFactor->SetResolution(1);
    this->ShrinkFactor->SetValue(3);
    this->ShrinkFactor->SetLabelText(
      "Set Shrink Factor");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->ShrinkFactor->GetWidgetName());


 if (!this->MaxNumOfIt)
    {
    this->MaxNumOfIt = vtkKWScaleWithEntry::New();
   }
 if (!this->MaxNumOfIt->IsCreated())
    {
    this->MaxNumOfIt->SetParent(aparametersFrame);
    this->MaxNumOfIt->Create();
    this->MaxNumOfIt->SetRange(0,100);
    this->MaxNumOfIt->SetResolution(1);
    this->MaxNumOfIt->SetValue(50);
    this->MaxNumOfIt->SetLabelText(
      "Set Max Number of Iterations");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->MaxNumOfIt->GetWidgetName());

 if (!this->NumOfFitLev)
    {
    this->NumOfFitLev = vtkKWScaleWithEntry::New();
    }
 if (!this->NumOfFitLev->IsCreated())
    {
    this->NumOfFitLev->SetParent(aparametersFrame);
    this->NumOfFitLev->Create();
    this->NumOfFitLev->SetRange(0,20);
    this->NumOfFitLev->SetResolution(1);
    this->NumOfFitLev->SetValue(4);
    this->NumOfFitLev->SetLabelText(
      "Set Number of Fitting Level");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->NumOfFitLev->GetWidgetName());


 if (!this->WienFilNoi)
    {
    this->WienFilNoi = vtkKWScaleWithEntry::New();
    }
 if (!this->WienFilNoi->IsCreated())
    {
    this->WienFilNoi->SetParent(aparametersFrame);
    this->WienFilNoi->Create();
    this->WienFilNoi->SetRange(0,10);
    this->WienFilNoi->SetResolution(0.1);
    this->WienFilNoi->SetValue(0.1);
    this->WienFilNoi->SetLabelText(
      "Set Wien Filter Noise");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->WienFilNoi->GetWidgetName());


 if (!this->BiasField)
    {
    this->BiasField = vtkKWScaleWithEntry::New();
    }
 if (!this->BiasField->IsCreated())
    {
    this->BiasField->SetParent(aparametersFrame);
    this->BiasField->Create();
    this->BiasField->SetRange(0,10);
    this->BiasField->SetResolution(0.05);
    this->BiasField->SetValue(0.15);
    this->BiasField->SetLabelText(
      "Set Bias Field Full.");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->BiasField->GetWidgetName());

 if (!this->Conv)
    {
    this->Conv = vtkKWScaleWithEntry::New();
    }
 if (!this->Conv->IsCreated())
    {
    this->Conv->SetParent(aparametersFrame);
    this->Conv->Create();
    this->Conv->SetRange(0,10);
    this->Conv->SetResolution(0.001);
    this->Conv->SetValue(0.001);
    this->Conv->SetLabelText(
      "Set Convergence Threshold");
    }
    
    this->Script(
    "pack %s -side top -anchor e -expand n -padx 2 -pady 6", 
    this->Conv->GetWidgetName());




////////////////////////////////////////////////////////////////////////////////////////////

  this->ApplyButton->SetParent(bias_page);
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());
                
    /*        
  mask_page->Delete();
  bias_page->Delete();              
  treshFrame->Delete();
  moduleFrame->Delete();
  aparametersFrame->Delete();
  parametersFrame->Delete();
  this->Notebook->Delete();
*/
  
}
