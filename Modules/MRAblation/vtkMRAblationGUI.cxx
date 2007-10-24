/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRAblationGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkMRAblationGUI.h"

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
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWEntryWithLabel.h"

//------------------------------------------------------------------------------
vtkMRAblationGUI* vtkMRAblationGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkMRAblationGUI");
  if(ret)
    {
      return (vtkMRAblationGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkMRAblationGUI;
}


//----------------------------------------------------------------------------
vtkMRAblationGUI::vtkMRAblationGUI()
{
  this->ImageDirectory = vtkKWLoadSaveButtonWithLabel::New();
  this->WorkingDirectory = vtkKWLoadSaveButtonWithLabel::New();
  this->TimepointsScale = vtkKWScaleWithEntry::New();
  this->SlicesScale = vtkKWScaleWithEntry::New();
  this->TEEntry = vtkKWEntryWithLabel::New();
  this->w0Entry = vtkKWEntryWithLabel::New();
  this->TCEntry = vtkKWEntryWithLabel::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->MRAblationNode = NULL;
}

//----------------------------------------------------------------------------
vtkMRAblationGUI::~vtkMRAblationGUI()
{
    this->RemoveMRMLNodeObservers ( );
    this->RemoveLogicObservers ( );
    
    if ( this->ImageDirectory ) {
        this->ImageDirectory->Delete();
        this->ImageDirectory = NULL;
    }
    if ( this->WorkingDirectory ) {
        this->WorkingDirectory->Delete();
        this->WorkingDirectory = NULL;
    }
    if ( this->TimepointsScale ) {
        this->TimepointsScale->Delete();
        this->TimepointsScale = NULL;
    }
    if ( this->SlicesScale ) {
        this->SlicesScale->Delete();
        this->SlicesScale = NULL;
    }
    if ( this->TEEntry ) {
        this->TEEntry->Delete();
        this->TEEntry = NULL;
    }
    if ( this->w0Entry ) {
        this->w0Entry->Delete();
        this->w0Entry = NULL;
    }
    if ( this->TCEntry ) {
        this->TCEntry->Delete();
        this->TCEntry = NULL;
    }
    if ( this->OutVolumeSelector ) {
        this->OutVolumeSelector->Delete();
        this->OutVolumeSelector = NULL;
    }
    if ( this->ApplyButton ) {
        this->ApplyButton->Delete();
        this->ApplyButton = NULL;
    }

  this->SetLogic (NULL);
  if ( this->MRAblationNode ) {
      this->SetAndObserveMRML( vtkObjectPointer(&this->MRAblationNode), NULL );
  }
  this->SetMRAblationNode (NULL); 
}

//----------------------------------------------------------------------------
void vtkMRAblationGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkMRAblationGUI::AddGUIObservers ( ) 
{
  this->TimepointsScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimepointsScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SlicesScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SlicesScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TEEntry->AddObserver (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->w0Entry->AddObserver (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TCEntry->AddObserver (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OutVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkMRAblationGUI::RemoveGUIObservers ( )
{
  this->TimepointsScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimepointsScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SlicesScale->RemoveObservers (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SlicesScale->RemoveObservers (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TEEntry->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->w0Entry->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->TCEntry->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand);
  this->OutVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkMRAblationGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in.
}

//---------------------------------------------------------------------------
void vtkMRAblationGUI::RemoveLogicObservers ( ) {
    // Fill in
}


//---------------------------------------------------------------------------
void vtkMRAblationGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast(caller);
  vtkKWMenu *v = vtkKWMenu::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if ( s == this->TimepointsScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (s == this->SlicesScale && event == vtkKWScale::ScaleValueChangedEvent ) 
    {
    this->UpdateMRML();
    }
  else if (e == this->TEEntry && event == vtkKWEntry::EntryValueChangedEvent) 
    {
    this->UpdateMRML();
    }
  else if (e == this->TCEntry && event == vtkKWEntry::EntryValueChangedEvent) 
    {
    this->UpdateMRML();
    }
  else if (e == this->w0Entry && event == vtkKWEntry::EntryValueChangedEvent) 
    {
    this->UpdateMRML();
    }
  else if (selector == this->OutVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    { 
    this->UpdateMRML();
    }

  if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();

    vtkSlicerApplication *slicerApp = (vtkSlicerApplication *)this->GetApplication();
    if (slicerApp != NULL)
    {
      vtkSlicerVolumesGUI* volumesGUI = (vtkSlicerVolumesGUI *)slicerApp->GetModuleGUIByName("Volume");
      this->Logic->Apply(volumesGUI);
    }
    }
  
}

//---------------------------------------------------------------------------
void vtkMRAblationGUI::UpdateMRML ()
{
  vtkMRMLMRAblationNode* n = this->GetMRAblationNode();
  if (n == NULL)
  {
    // no parameter node selected yet, create new
    this->OutVolumeSelector->SetSelectedNew("vtkMRMLMRAblationNode");
    this->OutVolumeSelector->ProcessNewNodeCommand("vtkMRMLMRAblationNode", "OutputVolume");
    n = vtkMRMLMRAblationNode::SafeDownCast(this->OutVolumeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetMRAblationNode(n);
    this->SetMRAblationNode(n);
    this->SetAndObserveMRML( vtkObjectPointer(&this->MRAblationNode), n);
  }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  // set node parameters from GUI widgets
  n->SetTimepoints(this->TimepointsScale->GetValue());
  n->SetSlices(this->SlicesScale->GetValue());
  n->SetTE(this->TEEntry->GetWidget()->GetValueAsDouble());
  n->Setw0(this->w0Entry->GetWidget()->GetValueAsDouble());
  n->SetTC(this->TCEntry->GetWidget()->GetValueAsDouble());
  n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());

std::cerr << "file name = " << this->ImageDirectory->GetWidget()->GetFileName() << std::endl;

  n->SetImageDirectory(this->ImageDirectory->GetWidget()->GetFileName());

std::cerr << "my file \n";


  n->SetWorkingDirectory(this->WorkingDirectory->GetWidget()->GetFileName());
}

//---------------------------------------------------------------------------
void vtkMRAblationGUI::UpdateGUI ()
{
  vtkMRMLMRAblationNode* n = this->GetMRAblationNode();
  if (n != NULL)
    {
    // set GUI widgest from parameter node
    this->TimepointsScale->SetValue(n->GetTimepoints());
    this->SlicesScale->SetValue(n->GetSlices());
    this->TEEntry->GetWidget()->SetValueAsDouble(n->GetTE());
    this->w0Entry->GetWidget()->SetValueAsDouble(n->Getw0());
    this->TCEntry->GetWidget()->SetValueAsDouble(n->GetTC());
    }
}

//---------------------------------------------------------------------------
void vtkMRAblationGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLMRAblationNode* node = vtkMRMLMRAblationNode::SafeDownCast(caller);
  if (node != NULL && this->GetMRAblationNode() == node) 
    {
    this->UpdateGUI();
    }
}




//---------------------------------------------------------------------------
void vtkMRAblationGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkMRMLMRAblationNode* mraNode = vtkMRMLMRAblationNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(mraNode);
  mraNode->Delete();

  this->UIPanel->AddPage ( "MRAblation", "MRAblation", NULL );
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
    
  // HELP FRAME
  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( "MRAblation" ) );
  helpFrame->Create ( );
  helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("MRAblation")->GetWidgetName());

  // MRAbltion Frame
  vtkKWFrameWithLabel *moduleFrame = vtkKWFrameWithLabel::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "MRAblation" ) );
  moduleFrame->Create( );
  moduleFrame->SetLabelText ("MRAblation");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("MRAblation")->GetWidgetName());

  this->ImageDirectory->SetParent( moduleFrame->GetFrame() );
  this->ImageDirectory->Create();
  this->ImageDirectory->SetLabelText( "Image Direcotry" );
  this->ImageDirectory->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->ImageDirectory->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( "." );
   app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->ImageDirectory->GetWidgetName());

  this->WorkingDirectory->SetParent( moduleFrame->GetFrame() );
  this->WorkingDirectory->Create();
  this->WorkingDirectory->SetLabelText( "Working Direcotry" );
  this->WorkingDirectory->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
  this->WorkingDirectory->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( "." );
   app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->WorkingDirectory->GetWidgetName());


  this->TimepointsScale->SetParent( moduleFrame->GetFrame() );
  this->TimepointsScale->SetLabelText("# of timepoints");
  this->TimepointsScale->Create();
  int w = this->TimepointsScale->GetScale()->GetWidth ( );
  this->TimepointsScale->SetRange(1,100);
  this->TimepointsScale->SetResolution (1);
  this->TimepointsScale->SetValue(30);
  
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TimepointsScale->GetWidgetName());

  this->SlicesScale->SetParent( moduleFrame->GetFrame() );
  this->SlicesScale->SetLabelText("# of slices");
  this->SlicesScale->Create();
  this->SlicesScale->GetScale()->SetWidth ( w );
  this->SlicesScale->SetRange(1, 50);
  this->SlicesScale->SetValue(10);
  this->SlicesScale->SetResolution (1);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->SlicesScale->GetWidgetName());

  this->TEEntry->SetParent( moduleFrame->GetFrame() );
  this->TEEntry->SetLabelText("TE");
  this->TEEntry->Create();
  this->TEEntry->GetWidget()->SetWidth ( w );
// this->TEEntry->SetRange(1, 50);
  this->TEEntry->GetWidget()->SetValueAsDouble(0.020);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TEEntry->GetWidgetName());

  this->w0Entry->SetParent( moduleFrame->GetFrame() );
  this->w0Entry->SetLabelText("w0");
  this->w0Entry->Create();
  this->w0Entry->GetWidget()->SetWidth ( w );
// this->w0Entry->SetRange(1, 50);
  this->w0Entry->GetWidget()->SetValueAsDouble(21.3);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->w0Entry->GetWidgetName());

  this->TCEntry->SetParent( moduleFrame->GetFrame() );
  this->TCEntry->SetLabelText("TC");
  this->TCEntry->Create();
  this->TCEntry->GetWidget()->SetWidth ( w );
//  this->TCEntry->SetRange(1, 50);
  this->TCEntry->GetWidget()->SetValueAsDouble(0.01076);
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TCEntry->GetWidgetName());


  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, "GADVolumeOut");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetLabelText( "Output Volumes: ");
  this->OutVolumeSelector->SetBalloonHelpString("select an output volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->OutVolumeSelector->GetWidgetName());


  this->ApplyButton->SetParent( moduleFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());



  
}
