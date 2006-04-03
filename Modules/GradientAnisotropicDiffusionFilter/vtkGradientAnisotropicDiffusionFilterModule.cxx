/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterModule.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <ostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkGradientAnisotropicDiffusionFilterModule.h"
#include "vtkGradientAnisotropicDiffusionFilterWidget.h"
#include "vtkMRMLGradientAnisotropicDiffusionFilterNode.h"

#include "vtkCommand.h"
#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerVolumeSelectGUI.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerStyle.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"

//------------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterModule* vtkGradientAnisotropicDiffusionFilterModule::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGradientAnisotropicDiffusionFilterModule");
  if(ret)
    {
      return (vtkGradientAnisotropicDiffusionFilterModule*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkGradientAnisotropicDiffusionFilterModule;
}


//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterModule::vtkGradientAnisotropicDiffusionFilterModule()
{
  this->ConductanceScale = vtkKWScaleWithEntry::New();
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerVolumeSelectGUI::New();
  this->ApplyButton = vtkKWPushButton::New();
}

//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterModule::~vtkGradientAnisotropicDiffusionFilterModule()
{
  this->ConductanceScale->Delete();
  this->TimeStepScale->Delete();
  this->NumberOfIterationsScale->Delete();
  this->VolumeSelector->Delete();
  this->ApplyButton->Delete();
}

//----------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::AddGUIObservers ( ) 
{
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
  this->ConductanceScale->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );

  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
  this->TimeStepScale->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );

  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICommand );
  this->NumberOfIterationsScale->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );

  this->VolumeSelector->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand );
  this->ApplyButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICommand ); 
}




//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::AddMrmlObservers ( )
{
  this->AddObserver ( vtkCommand::ModifiedEvent, (vtkCommand *)this->MrmlCommand );
}

//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  if ( !(event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkCommand::ModifiedEvent )) {
    return;
  }

  // save node state for Undo
  if ( event == vtkKWScale::ScaleValueStartChangingEvent && this->Mrml != NULL ) {
    this->Mrml->SaveStateForUndo ( this->GradientAnisotropicDiffusionFilterNode );
  }

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkSlicerVolumeSelectGUI *v = vtkSlicerVolumeSelectGUI::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);

  if ( s == this->ConductanceScale && event == vtkCommand::ModifiedEvent ) {
    this->GradientAnisotropicDiffusionFilterNode->SetConductance(this->ConductanceScale->GetValue());
  }
  else if (s == this->TimeStepScale && event == vtkCommand::ModifiedEvent ) {
    this->GradientAnisotropicDiffusionFilterNode->SetTimeStep(this->TimeStepScale->GetValue());
  }
  else if (s == this->NumberOfIterationsScale && event == vtkCommand::ModifiedEvent ) {
    this->GradientAnisotropicDiffusionFilterNode->SetConductance(this->ConductanceScale->GetValue());
  }
  else if (v == this->VolumeSelector && event == vtkCommand::ModifiedEvent ) {
    this->GradientAnisotropicDiffusionFilterNode->SetVolumeNodeID(this->VolumeSelector->GetSelected()->GetID());
  }
  else if (b == this->ApplyButton && event == vtkCommand::ModifiedEvent ) {
    //Update();
  }
}


//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::ProcessMrmlEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
    
  vtkMRMLGradientAnisotropicDiffusionFilterNode* node = dynamic_cast<vtkMRMLGradientAnisotropicDiffusionFilterNode *> (this->Mrml->GetNextNodeByClass("vtkMRMLGradientAnisotropicDiffusionFilterNode"));

  if (node) {
    this->SetGradientAnisotropicDiffusionFilterNode(node);
  }
}




//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterModule::BuildGUI ( ) 
{
  if (this->Parent == NULL) {
    return;
  }

  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkSlicerStyle *style = app->GetSlicerStyle();
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
  // HELP FRAME
  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
  helpFrame->SetParent ( this->Parent );
  helpFrame->Create ( );
  helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");
  helpFrame->SetDefaultLabelFontWeightToNormal( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  helpFrame->GetWidgetName(), this->Parent->GetWidgetName());

  this->ConductanceScale->SetParent(this->Parent);
  this->ConductanceScale->SetLabelText("Conductance");
  this->ConductanceScale->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ConductanceScale->GetWidgetName(), this->Parent->GetWidgetName());

  this->TimeStepScale->SetParent(this->Parent);
  this->TimeStepScale->SetLabelText("Time Step");
  this->TimeStepScale->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->TimeStepScale->GetWidgetName(), this->Parent->GetWidgetName());

  this->NumberOfIterationsScale->SetParent(this->Parent);
  this->NumberOfIterationsScale->SetLabelText("Number of Iterations");
  this->NumberOfIterationsScale->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->NumberOfIterationsScale->GetWidgetName(), this->Parent->GetWidgetName());

  this->VolumeSelector->SetParent(this->Parent);
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Mrml);
  this->VolumeSelector->SetLabelText( "Volume Select: ");
  this->VolumeSelector->SetBalloonHelpString("select a volume from the current mrml scene.");
  //pack -side top -anchor nw -expand false -fill x -padx 2 -pady 2
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->VolumeSelector->GetWidgetName(), this->Parent->GetWidgetName());

  this->ApplyButton->SetParent(this->Parent);
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ApplyButton->GetWidgetName(), this->Parent->GetWidgetName());
}
