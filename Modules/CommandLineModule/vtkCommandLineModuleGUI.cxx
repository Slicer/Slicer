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

#include "vtkCommandLineModuleGUI.h"

#include "vtkCommand.h"
#include "vtkSmartPointer.h"
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
#include "vtkKWLabel.h"
#include "vtkKWSpinBox.h"
#include "vtkKWSpinBoxWithLabel.h"
#include "vtkKWCheckButton.h"
#include "vtkKWCheckButtonWithLabel.h"

// Private implementaton of an std::map
class ModuleWidgetMap : public std::map<std::string, vtkSmartPointer<vtkKWCoreWidget> > {};

//------------------------------------------------------------------------------
vtkCommandLineModuleGUI* vtkCommandLineModuleGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCommandLineModuleGUI");
  if(ret)
    {
      return (vtkCommandLineModuleGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCommandLineModuleGUI;
}


//----------------------------------------------------------------------------
vtkCommandLineModuleGUI::vtkCommandLineModuleGUI()
{
  this->VolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->OutVolumeSelector = vtkSlicerNodeSelectorWidget::New();
  this->CommandLineModuleNodeSelector = vtkSlicerNodeSelectorWidget::New();
  this->ApplyButton = vtkKWPushButton::New();
  this->Logic = NULL;
  this->CommandLineModuleNode = NULL;

  this->InternalWidgetMap = new ModuleWidgetMap;
}

//----------------------------------------------------------------------------
vtkCommandLineModuleGUI::~vtkCommandLineModuleGUI()
{
  this->RemoveMRMLNodeObservers();
  this->RemoveLogicObservers();

  // Delete all the widgets
  delete this->InternalWidgetMap;
  
  if ( this->VolumeSelector )
    {
    this->VolumeSelector->Delete();
    this->VolumeSelector = NULL;
    }
  if ( this->OutVolumeSelector )
    {
    this->OutVolumeSelector->Delete();
    this->OutVolumeSelector = NULL;
    }
  if ( this->CommandLineModuleNodeSelector )
    {
    this->CommandLineModuleNodeSelector->Delete();
    this->CommandLineModuleNodeSelector = NULL;
    }
  if ( this->ApplyButton )
    {
    this->ApplyButton->Delete();
    this->ApplyButton = NULL;
    }

  this->SetLogic (NULL);
  // wjp test
  if ( this->CommandLineModuleNode ) {
      this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode), NULL );
  }
  // end wjp test
  this->SetCommandLineModuleNode (NULL);
  
}

//----------------------------------------------------------------------------
void
vtkCommandLineModuleGUI::SetModuleDescription(const ModuleDescription& description)
{
  ModuleDescriptionObject = description;
  this->Modified();
}


void vtkCommandLineModuleGUI::RemoveMRMLNodeObservers()
{
  // Need to implement - JVM
}


void vtkCommandLineModuleGUI::RemoveLogicObservers()
{
  // Need to implement - JVM
}


//----------------------------------------------------------------------------
void vtkCommandLineModuleGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::AddGUIObservers ( ) 
{
  this->VolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->CommandLineModuleNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::RemoveGUIObservers ( )
{
  this->VolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->OutVolumeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->CommandLineModuleNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->ApplyButton->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  vtkKWScaleWithEntry *s = vtkKWScaleWithEntry::SafeDownCast(caller);
  vtkKWMenu *v = vtkKWMenu::SafeDownCast(caller);
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (selector == this->VolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    { 
    this->UpdateMRML();
    }
  else if (selector == this->OutVolumeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    { 
    this->UpdateMRML();
    }
  if (selector == this->CommandLineModuleNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    { 
    vtkMRMLCommandLineModuleNode* n = vtkMRMLCommandLineModuleNode::SafeDownCast(this->CommandLineModuleNodeSelector->GetSelected());
    this->Logic->SetCommandLineModuleNode(n);
    this->SetCommandLineModuleNode(n);
    this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode), n);
    this->UpdateGUI();
    }
  else if (b == this->ApplyButton && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Apply();
    }
  
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::UpdateMRML ()
{
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    this->CommandLineModuleNodeSelector->SetSelectedNew();
    this->CommandLineModuleNodeSelector->ProcessNewNodeCommand();
    n = vtkMRMLCommandLineModuleNode::SafeDownCast(this->CommandLineModuleNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetCommandLineModuleNode(n);
    this->SetCommandLineModuleNode(n);
    this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode), n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  // set node parameters from GUI widgets
  // For instance, n->SetConductance(this->ConductanceScale->GetValue());
  n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
  
  n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());

}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::UpdateGUI ()
{
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  if (n != NULL)
    {
    // set GUI widgest from parameter node
    // this->ConductanceScale->SetValue(n->GetConductance());
    }
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::SafeDownCast(caller);
  if (node != NULL && this->GetCommandLineModuleNode() == node) 
    {
    this->UpdateGUI();
    }
}




//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::BuildGUI ( ) 
{
  std::string title = this->ModuleDescriptionObject.GetTitle();
  
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  vtkMRMLCommandLineModuleNode* node = vtkMRMLCommandLineModuleNode::New();
  this->Logic->GetMRMLScene()->RegisterNodeClass(node);

  this->UIPanel->AddPage ( title.c_str(), title.c_str(), NULL );

  // HELP FRAME
  vtkKWFrameWithLabel *helpFrame = vtkKWFrameWithLabel::New ( );
  helpFrame->SetParent ( this->UIPanel->GetPageWidget ( title.c_str() ) );
  helpFrame->Create ( );
  // helpFrame->CollapseFrame ( );
  helpFrame->SetLabelText ("Help");

  vtkKWLabel *helpMessage = vtkKWLabel::New();
  helpMessage->SetParent( helpFrame->GetFrame() );
  helpMessage->Create();
  helpMessage->SetText(this->ModuleDescriptionObject.GetDescription().c_str());

  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                helpMessage->GetWidgetName());
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                helpFrame->GetWidgetName(), this->UIPanel->GetPageWidget(title.c_str())->GetWidgetName());

  // Store the pointers to the widget in a map of SmartPointers so
  // they can be deleted then the GUI is deleted
  (*this->InternalWidgetMap)["HelpFrame"] = helpFrame;
  (*this->InternalWidgetMap)["HelpText"] = helpMessage;
  helpMessage->Delete();
  helpFrame->Delete();
  
  vtkKWFrameWithLabel *moduleFrame = vtkKWFrameWithLabel::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( title.c_str() ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText (title.c_str());
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget(title.c_str())->GetWidgetName());

  // Store the module frame widget in a SmartPointer
  (*this->InternalWidgetMap)["ModuleFrame"] = moduleFrame;
  moduleFrame->Delete();
  
  this->CommandLineModuleNodeSelector->SetNodeClass("vtkMRMLCommandLineModuleNode");
  this->CommandLineModuleNodeSelector->SetNewNodeEnabled(1);
  this->CommandLineModuleNodeSelector->SetNewNodeName((title+" parameters").c_str());
  this->CommandLineModuleNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->CommandLineModuleNodeSelector->Create();
  this->CommandLineModuleNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->CommandLineModuleNodeSelector->UpdateMenu();

  this->CommandLineModuleNodeSelector->SetBorderWidth(2);
  this->CommandLineModuleNodeSelector->SetReliefToGroove();
  //this->CommandLineModuleNodeSelector->SetPadX(2);
  //this->CommandLineModuleNodeSelector->SetPadY(2);
  //this->CommandLineModuleNodeSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  //this->CommandLineModuleNodeSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->CommandLineModuleNodeSelector->SetLabelText( "Parameter set");
  std::string nodeSelectorBalloonHelp = "select a \"" + title + " parameters\" node from the current mrml scene.";
  this->CommandLineModuleNodeSelector->SetBalloonHelpString(nodeSelectorBalloonHelp.c_str());
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->CommandLineModuleNodeSelector->GetWidgetName());


  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ModuleDescriptionObject.GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ModuleDescriptionObject.GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;
  
  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // each parameter group is its own labeled frame
    vtkKWFrameWithLabel *parameterGroupFrame = vtkKWFrameWithLabel::New ( );
    parameterGroupFrame->SetParent ( moduleFrame->GetFrame() );
    parameterGroupFrame->Create ( );
    parameterGroupFrame->SetLabelText ((*pgit).GetLabel().c_str());
    parameterGroupFrame->ExpandFrame ( );

    std::string parameterGroupBalloonHelp = (*pgit).GetDescription();
    parameterGroupFrame
      ->SetBalloonHelpString(parameterGroupBalloonHelp.c_str());

    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  parameterGroupFrame->GetWidgetName() );

    // Store the parameter group frame in a SmartPointer
    (*this->InternalWidgetMap)[(*pgit).GetLabel()] = parameterGroupFrame;
    parameterGroupFrame->Delete();
    
    // iterate over each parameter in this group
    std::vector<ModuleParameter>::const_iterator pbeginit
      = (*pgit).GetParameters().begin();
    std::vector<ModuleParameter>::const_iterator pendit
      = (*pgit).GetParameters().end();
    std::vector<ModuleParameter>::const_iterator pit;

    int pcount;
    for (pcount = 0, pit = pbeginit; pit != pendit; ++pit, ++pcount)
      {
      // switch on the type of the parameter...
      vtkKWCoreWidget *parameter;

      if ((*pit).GetType() == "int")
        {
        vtkKWSpinBoxWithLabel *tparameter = vtkKWSpinBoxWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
        tparameter->GetWidget()->RestrictValuesToIntegersOn();
        tparameter->GetWidget()->SetValueFormat("%1.0f");
        parameter = tparameter;
        }
      else if ((*pit).GetType() == "bool")
        {
        vtkKWCheckButtonWithLabel *tparameter = vtkKWCheckButtonWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetSelectedState((*pit).GetDefault() == "true" ? 1 : 0);
        parameter = tparameter;
        }
      else if ((*pit).GetType() == "float")
        {
        vtkKWSpinBoxWithLabel *tparameter = vtkKWSpinBoxWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
        parameter = tparameter;
        }
      else if ((*pit).GetType() == "double")
        {
        vtkKWSpinBoxWithLabel *tparameter = vtkKWSpinBoxWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
        parameter = tparameter;
        }
      else if ((*pit).GetType() == "std::string")
        {
        vtkKWEntryWithLabel *tparameter = vtkKWEntryWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetValue((*pit).GetDefault().c_str());
        parameter = tparameter;
        }
      else
        {
        vtkKWLabel *tparameter = vtkKWLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetText( (*pit).GetLabel().c_str() );
        parameter = tparameter;
        }

      // build the balloon help for the parameter
      std::string parameterBalloonHelp = (*pit).GetDescription();
      parameter->SetBalloonHelpString(parameterBalloonHelp.c_str());

      // pack the parameter. if the parameter has a separate label and
      // widget, then pack both side by side.
//         app->Script ( "grid config %s -column 1 -row %d -sticky w",
//                       parameter->GetWidgetName(), pcount );
      app->Script ( "pack %s -side top -anchor ne -padx 2 -pady 2",
                    parameter->GetWidgetName() );

      // Store the parameter widget in a SmartPointer
      (*this->InternalWidgetMap)[(*pit).GetLabel()] = parameter;
      parameter->Delete();
      }
    }
  
  
  
//   this->ConductanceScale->SetParent( moduleFrame->GetFrame() );
//   this->ConductanceScale->SetLabelText("Conductance");
//   this->ConductanceScale->Create();
//   int w = this->ConductanceScale->GetScale()->GetWidth ( );
//   this->ConductanceScale->SetRange(0,10);
//   this->ConductanceScale->SetResolution (0.1);
//   this->ConductanceScale->SetValue(1.0);
  
//   app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
//                 this->ConductanceScale->GetWidgetName());


  this->VolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode");
  this->VolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->VolumeSelector->Create();
  this->VolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->VolumeSelector->UpdateMenu();

  this->VolumeSelector->SetBorderWidth(2);
  this->VolumeSelector->SetReliefToGroove();
  //this->VolumeSelector->SetPadX(2);
  //this->VolumeSelector->SetPadY(2);
  //this->VolumeSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  //this->VolumeSelector->GetWidget()->GetWidget()->SetWidth(24);
  this->VolumeSelector->SetLabelText( "Input Volume: ");
  this->VolumeSelector->SetBalloonHelpString("select an input volume from the current mrml scene.");
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
                this->VolumeSelector->GetWidgetName());
  
  this->OutVolumeSelector->SetNodeClass("vtkMRMLScalarVolumeNode");
  this->OutVolumeSelector->SetNewNodeEnabled(1);
  this->OutVolumeSelector->SetNewNodeName((title+"output").c_str());
  this->OutVolumeSelector->SetParent( moduleFrame->GetFrame() );
  this->OutVolumeSelector->Create();
  this->OutVolumeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->OutVolumeSelector->UpdateMenu();

  this->OutVolumeSelector->SetBorderWidth(2);
  this->OutVolumeSelector->SetReliefToGroove();
  //this->OutVolumeSelector->SetPadX(2);
  //this->OutVolumeSelector->SetPadY(2);
  //this->OutVolumeSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  //this->OutVolumeSelector->GetWidget()->GetWidget()->SetWidth(24);
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



  
}
