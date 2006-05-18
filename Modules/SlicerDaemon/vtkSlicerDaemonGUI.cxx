/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkSlicerDaemonGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkSlicerDaemonGUI.h"

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

//------------------------------------------------------------------------------
vtkSlicerDaemonGUI* vtkSlicerDaemonGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkSlicerDaemonGUI");
  if(ret)
    {
      return (vtkSlicerDaemonGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerDaemonGUI;
}


//----------------------------------------------------------------------------
vtkSlicerDaemonGUI::vtkSlicerDaemonGUI()
{
  this->ConductanceScale = vtkKWScaleWithEntry::New();
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->Logic = NULL;

}

//----------------------------------------------------------------------------
vtkSlicerDaemonGUI::~vtkSlicerDaemonGUI()
{
  this->ConductanceScale->Delete();
  this->TimeStepScale->Delete();
  this->NumberOfIterationsScale->Delete();
  this->VolumeSelector->Delete();
  this->ApplyButton->Delete();
  if (this->Logic != NULL) 
    {
    this->Logic->Delete();
    
    }
  
}

//----------------------------------------------------------------------------
void vtkSlicerDaemonGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkSlicerDaemonGUI::AddGUIObservers ( ) 
{
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->GetWidget()->GetWidget()->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkSlicerDaemonGUI::RemoveGUIObservers ( )
{
    // Fill in
    this->ApplyButton->RemoveObservers ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkSlicerDaemonGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWMenu *v = vtkKWMenu::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);

  if ( s == this->ConductanceScale && event == vtkKWScale::ScaleValueChangedEvent ) {
    this->Logic->GetSlicerDaemonNode()->SetConductance(this->ConductanceScale->GetValue());
  }
  else if (s == this->TimeStepScale && event == vtkKWScale::ScaleValueChangedEvent ) {
    this->Logic->GetSlicerDaemonNode()->SetTimeStep(this->TimeStepScale->GetValue());
  }
  else if (s == this->NumberOfIterationsScale && event == vtkKWScale::ScaleValueChangedEvent ) {
    this->Logic->GetSlicerDaemonNode()->SetConductance(this->ConductanceScale->GetValue());
  }
  else if (v == this->VolumeSelector->GetWidget()->GetWidget()->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent ) { 
    this->Logic->GetSlicerDaemonNode()->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
  }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) {
    this->Logic->Apply();
  }
}


//---------------------------------------------------------------------------
void vtkSlicerDaemonGUI::ProcessMrmlEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  /**  
  vtkMRMLSlicerDaemonNode* node = dynamic_cast<vtkMRMLSlicerDaemonNode *> (this->ApplicationLogic->GetMRMLScene()->GetNextNodeByClass("vtkMRMLSlicerDaemonNode"));

  if (node) {
    this->SetSlicerDaemonNode(node);
  }
  **/
}




//---------------------------------------------------------------------------
void vtkSlicerDaemonGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    
  this->UIPanel->AddPage ( "SlicerDaemon", "SlicerDaemon", NULL );
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
    
  // HELP FRAME
  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( "SlicerDaemon" ) );
  helpFrame->Create ( );
  helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("SlicerDaemon")->GetWidgetName());

  vtkKWFrameWithLabel *moduleFrame = vtkKWFrameWithLabel::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "SlicerDaemon" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("SlicerDaemon");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("SlicerDaemon")->GetWidgetName());

  this->ConductanceScale->SetParent( moduleFrame->GetFrame() );
  this->ConductanceScale->SetLabelText("Conductance");
  this->ConductanceScale->Create();
  int w = this->ConductanceScale->GetScale()->GetWidth ( );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->ConductanceScale->GetWidgetName());

  this->TimeStepScale->SetParent( moduleFrame->GetFrame() );
  this->TimeStepScale->SetLabelText("Time Step");
  this->TimeStepScale->Create();
  this->TimeStepScale->GetScale()->SetWidth ( w );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->TimeStepScale->GetWidgetName());

  this->NumberOfIterationsScale->SetParent( moduleFrame->GetFrame() );
  this->NumberOfIterationsScale->SetLabelText("Iterations");
  this->NumberOfIterationsScale->Create();
  this->NumberOfIterationsScale->GetScale()->SetWidth ( w );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->NumberOfIterationsScale->GetWidgetName());

  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode");
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetReliefToGroove();
  this->VolumeSelector->SetPadX(2);
  this->VolumeSelector->SetPadY(2);
  this->VolumeSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->VolumeSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->VolumeSelector->SetLabelText( "Volume Select: ");
  this->VolumeSelector->SetBalloonHelpString("select a volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->VolumeSelector->GetWidgetName());

  this->ApplyButton->SetParent( moduleFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  this->ApplyButton->SetWidth ( 8 );
  app->Script("pack %s -side top -anchor e -padx 20 -pady 10", 
                this->ApplyButton->GetWidgetName());



  
}
