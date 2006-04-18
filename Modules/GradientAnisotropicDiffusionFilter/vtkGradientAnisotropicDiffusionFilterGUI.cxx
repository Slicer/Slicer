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
  this->VolumeSelector = vtkSlicerVolumeSelectGUI::New();
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
void vtkGradientAnisotropicDiffusionFilterGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  if ( !(event == vtkKWScale::ScaleValueStartChangingEvent || event == vtkCommand::ModifiedEvent )) {
    return;
  }

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkSlicerVolumeSelectGUI *v = vtkSlicerVolumeSelectGUI::SafeDownCast(caller);
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

  this->ConductanceScale->SetParent( this->UIPanel->GetPageWidget ( "GradientAnisotropicDiffusionFilter" ));
  this->ConductanceScale->SetLabelText("Conductance");
  this->ConductanceScale->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ConductanceScale->GetWidgetName(), GUIName);

  this->TimeStepScale->SetParent( this->UIPanel->GetPageWidget ( "GradientAnisotropicDiffusionFilter" ));
  this->TimeStepScale->SetLabelText("Time Step");
  this->TimeStepScale->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->TimeStepScale->GetWidgetName(), GUIName);

  this->NumberOfIterationsScale->SetParent( this->UIPanel->GetPageWidget ( "GradientAnisotropicDiffusionFilter" ));
  this->NumberOfIterationsScale->SetLabelText("Number of Iterations");
  this->NumberOfIterationsScale->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->NumberOfIterationsScale->GetWidgetName(), GUIName);

  this->VolumeSelector->SetParent( this->UIPanel->GetPageWidget ( "GradientAnisotropicDiffusionFilter" ));
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->SetLabelText( "Volume Select: ");
  this->VolumeSelector->SetBalloonHelpString("select a volume from the current mrml scene.");
  //pack -side top -anchor nw -expand false -fill x -padx 2 -pady 2
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->VolumeSelector->GetWidgetName(), GUIName);

//  this->ApplyButton->SetParent(this->GetApplication());
  this->ApplyButton->Create();
  this->ApplyButton->SetText("Apply");
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                this->ApplyButton->GetWidgetName(), GUIName);
}
