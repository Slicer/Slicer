/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkGradientAnisotropicDiffusionFilterGUI.cxx,v $
Date:      $Date: 2006/03/17 15:10:10 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/

#include <string>
#include <iostream>
#include <sstream>

#include "vtkObjectFactory.h"

#include "vtkGradientAnisotropicDiffusionFilterGUI.h"

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
#include "vtkSlicerStyle.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"

//------------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterGUI* vtkGradientAnisotropicDiffusionFilterGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkGradientAnisotropicDiffusionFilterGUI");
  if(ret)
    {
      return (vtkGradientAnisotropicDiffusionFilterGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkGradientAnisotropicDiffusionFilterGUI;
}


//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterGUI::vtkGradientAnisotropicDiffusionFilterGUI()
{
  this->ConductanceScale = vtkKWScaleWithEntry::New();
  this->TimeStepScale = vtkKWScaleWithEntry::New();
  this->NumberOfIterationsScale = vtkKWScaleWithEntry::New();
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->Logic = NULL;

}

//----------------------------------------------------------------------------
vtkGradientAnisotropicDiffusionFilterGUI::~vtkGradientAnisotropicDiffusionFilterGUI()
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
void vtkGradientAnisotropicDiffusionFilterGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterGUI::AddGUIObservers ( ) 
{
  this->ConductanceScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ConductanceScale->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->TimeStepScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->TimeStepScale->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->NumberOfIterationsScale->AddObserver (vtkKWScale::ScaleValueStartChangingEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NumberOfIterationsScale->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->VolumeSelector->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ApplyButton->AddObserver (vtkCommand::ModifiedEvent, (vtkCommand *)this->GUICallbackCommand ); 
}



//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterGUI::RemoveGUIObservers ( )
{
    // Fill in
    this->ApplyButton->RemoveObservers ( vtkCommand::ModifiedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  if ( !(event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkCommand::ModifiedEvent )) {
    return;
  }

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *v = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);

  if ( s == this->ConductanceScale && event == vtkCommand::ModifiedEvent ) {
    this->Logic->GetGradientAnisotropicDiffusionFilterNode()->SetConductance(this->ConductanceScale->GetValue());
  }
  else if (s == this->TimeStepScale && event == vtkCommand::ModifiedEvent ) {
    this->Logic->GetGradientAnisotropicDiffusionFilterNode()->SetTimeStep(this->TimeStepScale->GetValue());
  }
  else if (s == this->NumberOfIterationsScale && event == vtkCommand::ModifiedEvent ) {
    this->Logic->GetGradientAnisotropicDiffusionFilterNode()->SetConductance(this->ConductanceScale->GetValue());
  }
  else if (v == this->VolumeSelector && event == vtkCommand::ModifiedEvent ) {
    this->Logic->GetGradientAnisotropicDiffusionFilterNode()->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
  }
  else if (b == this->ApplyButton && event == vtkCommand::ModifiedEvent ) {
    this->Logic->Apply();
  }
}


//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterGUI::ProcessMrmlEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  /**  
  vtkMRMLGradientAnisotropicDiffusionFilterNode* node = dynamic_cast<vtkMRMLGradientAnisotropicDiffusionFilterNode *> (this->ApplicationLogic->GetMRMLScene()->GetNextNodeByClass("vtkMRMLGradientAnisotropicDiffusionFilterNode"));

  if (node) {
    this->SetGradientAnisotropicDiffusionFilterNode(node);
  }
  **/
}




//---------------------------------------------------------------------------
void vtkGradientAnisotropicDiffusionFilterGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkSlicerStyle *style = app->GetSlicerStyle();
    
  this->UIPanel->AddPage ( "GradientAnisotropicDiffusionFilter", "GradientAnisotropicDiffusionFilter", NULL );
  // ---
  // MODULE GUI FRAME 
  // configure a page for a volume loading UI for now.
  // later, switch on the modulesButton in the SlicerControlGUI
  // ---
    
  // HELP FRAME
  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( "GradientAnisotropicDiffusionFilter" ) );
  helpFrame->Create ( );
  helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget("GradientAnisotropicDiffusionFilter")->GetWidgetName());

  vtkKWFrameWithLabel *moduleFrame = vtkKWFrameWithLabel::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( "GradientAnisotropicDiffusionFilter" ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText ("Gradient Anisotropic Diffusion Filter");
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget("GradientAnisotropicDiffusionFilter")->GetWidgetName());


  this->ConductanceScale->SetParent( moduleFrame->GetFrame() );
  this->ConductanceScale->SetLabelText("Conductance");
  this->ConductanceScale->Create();
  app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->ConductanceScale->GetWidgetName());

  this->TimeStepScale->SetParent( moduleFrame->GetFrame() );
  this->TimeStepScale->SetLabelText("Time Step");
  this->TimeStepScale->Create();
  app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->TimeStepScale->GetWidgetName());

  this->NumberOfIterationsScale->SetParent( moduleFrame->GetFrame() );
  this->NumberOfIterationsScale->SetLabelText("Number of Iterations");
  this->NumberOfIterationsScale->Create();
  app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->NumberOfIterationsScale->GetWidgetName());

  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode");
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetReliefToGroove();
  this->VolumeSelector->SetLabelText("Days:");
  this->VolumeSelector->SetPadX(2);
  this->VolumeSelector->SetPadY(2);
  this->VolumeSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->VolumeSelector->GetWidget()->GetWidget()->SetWidth(20);
  this->VolumeSelector->SetLabelText( "Volume Select: ");
  this->VolumeSelector->SetBalloonHelpString("select a volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->VolumeSelector->GetWidgetName());

  this->ApplyButton->SetParent( moduleFrame->GetFrame() );
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  app->Script("pack %s -side top -anchor w -padx 2 -pady 4", 
                this->ApplyButton->GetWidgetName());
}
