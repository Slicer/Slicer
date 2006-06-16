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
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveDialog.h"

#include "itkNumericTraits.h"

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
  this->CommandLineModuleNodeSelector = vtkSlicerNodeSelectorWidget::New();

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
  
  if ( this->CommandLineModuleNodeSelector )
    {
    this->CommandLineModuleNodeSelector->Delete();
    this->CommandLineModuleNodeSelector = NULL;
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
  this->CommandLineModuleNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  (*this->InternalWidgetMap)["ApplyButton"]->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  (*this->InternalWidgetMap)["DefaultButton"]->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}



//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::RemoveGUIObservers ( )
{
  this->CommandLineModuleNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  (*this->InternalWidgetMap)["ApplyButton"]->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  (*this->InternalWidgetMap)["DefaultButton"]->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  // Hypothesis: May need to call UpdateMRML() if any gui change would
  // result in a parameter change.

  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);

  if (selector == this->CommandLineModuleNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    { 
    vtkMRMLCommandLineModuleNode* n = vtkMRMLCommandLineModuleNode::SafeDownCast(this->CommandLineModuleNodeSelector->GetSelected());
    this->Logic->SetCommandLineModuleNode(n);
    this->SetCommandLineModuleNode(n);
    this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode), n);
    this->UpdateGUI();
    }
  else if (b == (*this->InternalWidgetMap)["ApplyButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
    {
    this->UpdateMRML();
    this->Logic->Apply();
    }
  else if (b == (*this->InternalWidgetMap)["DefaultButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
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
    this->CommandLineModuleNodeSelector->SetSelectedNew("vtkMRMLCommandLineModuleNode");
    this->CommandLineModuleNodeSelector->ProcessNewNodeCommand("vtkMRMLCommandLineModuleNode", "CommandLineModuleNode");
    n = vtkMRMLCommandLineModuleNode::SafeDownCast(this->CommandLineModuleNodeSelector->GetSelected());

    // set an observe new node in Logic
    this->Logic->SetCommandLineModuleNode(n);
    this->SetCommandLineModuleNode(n);
    this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode), n);
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);

  // set node parameters from GUI widgets
  // For instance,
  //  n->SetConductance(this->ConductanceScale->GetValue());
  //  n->SetInputVolumeRef(this->VolumeSelector->GetSelected()->GetID());
  //  n->SetOutputVolumeRef(this->OutVolumeSelector->GetSelected()->GetID());
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::UpdateGUI ()
{
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  if (n != NULL)
    {
    // set GUI widgets from parameter node
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
  
  this->CommandLineModuleNodeSelector->SetNodeClass("vtkMRMLCommandLineModuleNode", NULL, NULL, NULL);
  this->CommandLineModuleNodeSelector->SetNewNodeEnabled(1);
  //this->CommandLineModuleNodeSelector->SetNewNodeName((title+" parameters").c_str());
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
    if ((*pgit).GetAdvanced() == "true")
      {
      parameterGroupFrame->CollapseFrame ( );
      }
    
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

      if ((*pit).GetTag() == "integer")
        {
        if ((*pit).GetConstraints() == "")
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
        else
          {
          int min, max, step;
          if ((*pit).GetMinimum() != "")
            {
            min = atoi((*pit).GetMinimum().c_str());
            }
          else
            {
            min = itk::NumericTraits<int>::NonpositiveMin();
            }
          if ((*pit).GetMaximum() != "")
            {
            max = atoi((*pit).GetMaximum().c_str());
            }
          else
            {
            max = itk::NumericTraits<int>::max();
            }
          if ((*pit).GetStep() != "")
            {
            step = atoi((*pit).GetStep().c_str());
            }
          else
            {
            step = 1;
            }

          vtkKWScaleWithEntry *tparameter = vtkKWScaleWithEntry::New();
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->PopupModeOn();
          tparameter->Create();
          tparameter->SetLabelText((*pit).GetLabel().c_str());
          tparameter->SetValue(atof((*pit).GetDefault().c_str()));
          tparameter->RangeVisibilityOn();
          tparameter->SetRange(min, max);
          tparameter->SetResolution(step);
          parameter = tparameter;
          }
        }
      else if ((*pit).GetTag() == "boolean")
        {
        vtkKWCheckButtonWithLabel *tparameter = vtkKWCheckButtonWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetSelectedState((*pit).GetDefault() == "true" ? 1 : 0);
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "float")
        {
        if ((*pit).GetConstraints() == "")
          {
          vtkKWSpinBoxWithLabel *tparameter = vtkKWSpinBoxWithLabel::New();
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->Create();
          tparameter->SetLabelText((*pit).GetLabel().c_str());
          tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
          parameter = tparameter;
          }
        else
          {
          double min, max, step;
          if ((*pit).GetMinimum() != "")
            {
            min = atof((*pit).GetMinimum().c_str());
            }
          else
            {
            min = itk::NumericTraits<float>::NonpositiveMin();
            }
          if ((*pit).GetMaximum() != "")
            {
            max = atoi((*pit).GetMaximum().c_str());
            }
          else
            {
            max = itk::NumericTraits<float>::max();
            }
          if ((*pit).GetStep() != "")
            {
            step = atof((*pit).GetStep().c_str());
            }
          else
            {
            step = 0.1;
            }

          vtkKWScaleWithEntry *tparameter
            = vtkKWScaleWithEntry::New();
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->PopupModeOn();
          tparameter->Create();
          tparameter->SetLabelText((*pit).GetLabel().c_str());
          tparameter->SetValue(atof((*pit).GetDefault().c_str()));
          tparameter->RangeVisibilityOn();
          tparameter->SetRange(min, max);
          tparameter->SetResolution(step);
          parameter = tparameter;
          }
        }
      else if ((*pit).GetTag() == "double")
        {
        if ((*pit).GetConstraints() == "")
          {
          vtkKWSpinBoxWithLabel *tparameter = vtkKWSpinBoxWithLabel::New();
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->Create();
          tparameter->SetLabelText((*pit).GetLabel().c_str());
          tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
          parameter = tparameter;
          }
        else
          {
          double min, max, step;
          if ((*pit).GetMinimum() != "")
            {
            min = atof((*pit).GetMinimum().c_str());
            }
          else
            {
            min = itk::NumericTraits<double>::NonpositiveMin();
            }
          if ((*pit).GetMaximum() != "")
            {
            max = atoi((*pit).GetMaximum().c_str());
            }
          else
            {
            max = itk::NumericTraits<double>::max();
            }
          if ((*pit).GetStep() != "")
            {
            step = atof((*pit).GetStep().c_str());
            }
          else
            {
            step = 0.1;
            }

          vtkKWScaleWithEntry *tparameter
            = vtkKWScaleWithEntry::New();
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->PopupModeOn();
          tparameter->Create();
          tparameter->SetLabelText((*pit).GetLabel().c_str());
          tparameter->SetValue(atof((*pit).GetDefault().c_str()));
          tparameter->RangeVisibilityOn();
          tparameter->SetRange(min, max);
          tparameter->SetResolution(step);
          parameter = tparameter;
          }
        }
      else if ((*pit).GetTag() == "string"
               || (*pit).GetTag() == "integer-vector"
               || (*pit).GetTag() == "float-vector"
               || (*pit).GetTag() == "double-vector")
        {
        vtkKWEntryWithLabel *tparameter = vtkKWEntryWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText((*pit).GetLabel().c_str());
        tparameter->GetWidget()->SetValue((*pit).GetDefault().c_str());
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "image" && (*pit).GetChannel() == "input")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        
        tparameter->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
        tparameter->UpdateMenu();
        
        tparameter->SetBorderWidth(2);
        tparameter->SetReliefToGroove();
        tparameter->SetLabelText( (*pit).GetLabel().c_str());
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "image" && (*pit).GetChannel() == "output")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        
        tparameter->SetNodeClass("vtkMRMLScalarVolumeNode", NULL, NULL, NULL);
        tparameter->SetNewNodeEnabled(1);
//        tparameter->SetNewNodeName((title+"output").c_str());
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
        tparameter->UpdateMenu();
        
        tparameter->SetBorderWidth(2);
        tparameter->SetReliefToGroove();
        tparameter->SetLabelText( (*pit).GetLabel().c_str());
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "directory")
        {
        vtkKWLoadSaveButtonWithLabel *tparameter
          = vtkKWLoadSaveButtonWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        if ((*pit).GetChannel() == "output")
          {
          tparameter->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
          }
        tparameter->SetLabelText( (*pit).GetLabel().c_str() );
        tparameter->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOn();
        tparameter->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( (*pit).GetDefault().c_str() );
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "file")
        {
        vtkKWLoadSaveButtonWithLabel *tparameter
          = vtkKWLoadSaveButtonWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        if ((*pit).GetChannel() == "output")
          {
          tparameter->GetWidget()->GetLoadSaveDialog()->SaveDialogOn();
          }
        tparameter->Create();
        tparameter->SetLabelText( (*pit).GetLabel().c_str() );
        tparameter->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( (*pit).GetDefault().c_str() );
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
      app->Script ( "pack %s -side top -anchor ne -padx 2 -pady 2",
                    parameter->GetWidgetName() );

      // Store the parameter widget in a SmartPointer
      (*this->InternalWidgetMap)[(*pit).GetLabel()] = parameter;
      parameter->Delete();
      }
    }
  
  
  // Create a "Default" button
  vtkKWPushButton *defaultB = vtkKWPushButton::New();
  defaultB->SetParent( moduleFrame->GetFrame() );
  defaultB->Create();
  defaultB->SetText("Default");
  defaultB->SetWidth ( 8 );
  app->Script("pack %s -side left -anchor w -padx 20 -pady 10", 
              defaultB->GetWidgetName());

  std::string defaultBalloonHelp("Reset parameters to default.");
  defaultB->SetBalloonHelpString(defaultBalloonHelp.c_str());

  (*this->InternalWidgetMap)["DefaultButton"] = defaultB;
  defaultB->Delete();

  // Create an "Apply" button
  vtkKWPushButton *apply = vtkKWPushButton::New();
  apply->SetParent( moduleFrame->GetFrame() );
  apply->Create();
  apply->SetText("Apply");
  apply->SetWidth ( 8 );
  app->Script("pack %s -side right -anchor e -padx 20 -pady 10", 
              apply->GetWidgetName());

  std::string applyBalloonHelp("Execute the module");
  apply->SetBalloonHelpString(applyBalloonHelp.c_str());


  (*this->InternalWidgetMap)["ApplyButton"] = apply;
  apply->Delete();

  
}
