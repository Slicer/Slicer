/*==========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $HeadURL$
Date:      $Date$
Version:   $Revision$

==========================================================================*/

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
#include "vtkKWRadioButton.h"
#include "vtkKWRadioButtonSet.h"
#include "vtkKWRadioButtonSetWithLabel.h"
#include "vtkKWText.h"
#include "vtkKWTextWithScrollbars.h"
#include "vtkKWMessage.h"

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

  this->NewNodeCallbackCommand = vtkCallbackCommand::New ( );
  this->NewNodeCallbackCommand->SetClientData(reinterpret_cast<void *>(this) );
  this->NewNodeCallbackCommand
    ->SetCallback( vtkCommandLineModuleGUI::NewNodeCallback );
  
//  this->DebugOn();
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
    this->CommandLineModuleNodeSelector->SetParent(NULL);
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

  if ( this->NewNodeCallbackCommand != NULL )
    {
    this->NewNodeCallbackCommand->Delete ( );
    this->NewNodeCallbackCommand = NULL;
    }
  
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
  this->Superclass::PrintSelf(os, indent);

  os << indent << "ModuleDescription: " << this->GetModuleDescription()
     << std::endl;
  
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::AddGUIObservers ( ) 
{
  this->CommandLineModuleNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->CommandLineModuleNodeSelector->AddObserver (vtkSlicerNodeSelectorWidget::NewNodeEvent, (vtkCommand *)this->NewNodeCallbackCommand );  

  (*this->InternalWidgetMap)["ApplyButton"]->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  (*this->InternalWidgetMap)["DefaultButton"]->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  // add an observer for each widget created
  ModuleWidgetMap::const_iterator wit;
  for (wit = this->InternalWidgetMap->begin();
       wit != this->InternalWidgetMap->end(); ++wit)
    {
    // Need to determine what type of widget we are using so we can
    // set the appropriate type of observer
    vtkKWSpinBoxWithLabel *sb = vtkKWSpinBoxWithLabel::SafeDownCast((*wit).second);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast((*wit).second);
    vtkKWCheckButtonWithLabel *cb = vtkKWCheckButtonWithLabel::SafeDownCast((*wit).second);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast((*wit).second);
    vtkSlicerNodeSelectorWidget *ns = vtkSlicerNodeSelectorWidget::SafeDownCast((*wit).second);
    vtkKWLoadSaveButtonWithLabel *lsb = vtkKWLoadSaveButtonWithLabel::SafeDownCast((*wit).second);
    vtkKWRadioButtonSetWithLabel *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast((*wit).second);

    if (sb)
      {
      sb->GetWidget()->AddObserver(vtkKWSpinBox::SpinBoxValueChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (se)
      {
      se->AddObserver(vtkKWScale::ScaleValueStartChangingEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      se->AddObserver(vtkKWScale::ScaleValueChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (cb)
      {
      cb->GetWidget()->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (e)
      {
      e->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (ns)
      {
      ns->AddObserver(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (lsb)
      {
      lsb->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent,
                       (vtkCommand *) this->GUICallbackCommand);
      }
    else if (rbs)
      {
      int num = rbs->GetWidget()->GetNumberOfWidgets();
      for (int i=0; i < num; ++i)
        {
        int id = rbs->GetWidget()->GetIdOfNthWidget(i);
        vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
        rb->AddObserver(vtkKWRadioButton::SelectedStateChangedEvent,
                        (vtkCommand *) this->GUICallbackCommand);
        }
      }
    }

}



//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::RemoveGUIObservers ( )
{
  this->CommandLineModuleNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->CommandLineModuleNodeSelector->RemoveObservers (vtkSlicerNodeSelectorWidget::NewNodeEvent, (vtkCommand *)this->NewNodeCallbackCommand );  

  (*this->InternalWidgetMap)["ApplyButton"]->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  (*this->InternalWidgetMap)["DefaultButton"]->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );

  // remove observers for each widget created
  ModuleWidgetMap::const_iterator wit;
  for (wit = this->InternalWidgetMap->begin();
       wit != this->InternalWidgetMap->end(); ++wit)
    {
    // Need to determine what type of widget we are using so we can
    // set the appropriate type of observer
    vtkKWSpinBoxWithLabel *sb = vtkKWSpinBoxWithLabel::SafeDownCast((*wit).second);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast((*wit).second);
    vtkKWCheckButtonWithLabel *cb = vtkKWCheckButtonWithLabel::SafeDownCast((*wit).second);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast((*wit).second);
    vtkSlicerNodeSelectorWidget *ns = vtkSlicerNodeSelectorWidget::SafeDownCast((*wit).second);
    vtkKWLoadSaveButtonWithLabel *lsb = vtkKWLoadSaveButtonWithLabel::SafeDownCast((*wit).second);
    vtkKWRadioButtonSetWithLabel *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast((*wit).second);

    if (sb)
      {
      sb->GetWidget()->RemoveObservers(vtkKWSpinBox::SpinBoxValueChangedEvent,
                          (vtkCommand *) this->GUICallbackCommand);
      }
    else if (se)
      {
      se->RemoveObservers(vtkKWScale::ScaleValueStartChangingEvent,
                          (vtkCommand *) this->GUICallbackCommand);
      se->RemoveObservers(vtkKWScale::ScaleValueChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (cb)
      {
      cb->GetWidget()
        ->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (e)
      {
      e->GetWidget()->RemoveObservers(vtkKWEntry::EntryValueChangedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (ns)
      {
      ns->RemoveObservers(vtkSlicerNodeSelectorWidget::NodeSelectedEvent,
                      (vtkCommand *) this->GUICallbackCommand);
      }
    else if (lsb)
      {
      lsb->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent,
                       (vtkCommand *) this->GUICallbackCommand);
      }
    else if (rbs)
      {
      int num = rbs->GetWidget()->GetNumberOfWidgets();
      for (int i=0; i < num; ++i)
        {
        int id = rbs->GetWidget()->GetIdOfNthWidget(i);
        vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
        rb->RemoveObservers(vtkKWRadioButton::SelectedStateChangedEvent,
                        (vtkCommand *) this->GUICallbackCommand);
        }
      }
    }
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{
  //std::cout << "ProcessGUIEvents()" << std::endl;
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  if (selector != NULL && selector != this->CommandLineModuleNodeSelector)
    {
    return;
    }
  else if (selector == this->CommandLineModuleNodeSelector && selector->GetSelected() == NULL)
    {
    return;
    }

  if (selector == this->CommandLineModuleNodeSelector && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    // Selected a new parameter node
    //std::cout << "  Selector" << std::endl;
    vtkMRMLCommandLineModuleNode* n = vtkMRMLCommandLineModuleNode::SafeDownCast(this->CommandLineModuleNodeSelector->GetSelected());
    this->Logic->SetCommandLineModuleNode(n);
    this->SetCommandLineModuleNode(n);
    this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode), n);
    this->UpdateGUI();
    }
  else if (selector == this->CommandLineModuleNodeSelector && event == vtkSlicerNodeSelectorWidget::NewNodeEvent )
    {
    // creating a new parameter node
    //std::cout << "  New node" << std::endl;
    vtkMRMLCommandLineModuleNode* n = vtkMRMLCommandLineModuleNode::SafeDownCast((vtkObjectBase*)callData);
    n->SetModuleDescription( this->ModuleDescriptionObject );
    }
  else if (b == (*this->InternalWidgetMap)["ApplyButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
    {
    // Apply button was pressed
    //std::cout << "  Apply" << std::endl;
    this->UpdateMRML();
    this->Logic->SetTemporaryDirectory( ((vtkSlicerApplication*)this->GetApplication())->GetTemporaryDirectory() );
    this->Logic->Apply();
    }
  else if (b == (*this->InternalWidgetMap)["DefaultButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
    {
    // Defaults button was pressed
    // (may need additional code to get any node delection widgets to
    // return to their default state)
    this->CommandLineModuleNode
      ->SetModuleDescription( this->ModuleDescriptionObject);
    this->UpdateGUI();
    }
  else
    {
    // Propagate the rest of the gui to the node.  This is sloppy.  We
    // could check for each type of event and only update the one
    // parameter that changed.
    //std::cout << "  Not recognized. Call UpdateMRML()" << std::endl;
    this->UpdateMRML();
    }
  
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::UpdateMRML ()
{
  //std::cout << "UpdateMRML()" << std::endl;
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  bool createdNode = false;
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    //std::cout << "  Creating a new node" << std::endl;
    this->CommandLineModuleNodeSelector->SetSelectedNew("vtkMRMLCommandLineModuleNode");
    this->CommandLineModuleNodeSelector->ProcessNewNodeCommand("vtkMRMLCommandLineModuleNode", this->ModuleDescriptionObject.GetTitle().c_str());
    n = vtkMRMLCommandLineModuleNode::SafeDownCast(this->CommandLineModuleNodeSelector->GetSelected());

    if (n == NULL)
      {
      vtkDebugMacro("No CommandLineModuleNode available");
      return;
      }
    
    // set the a module description for this node
    n->SetModuleDescription( this->ModuleDescriptionObject );
    
    // set an observe new node in Logic
    this->Logic->SetCommandLineModuleNode(n);
    this->SetCommandLineModuleNode(n);
    this->SetMRML(vtkObjectPointer(&this->CommandLineModuleNode),n);

    createdNode = true;
   }

  // save node parameters for Undo
  this->GetLogic()->GetMRMLScene()->SaveStateForUndo(n);
  
  //  set node parameters from GUI widgets
  //
  ModuleWidgetMap::const_iterator wit;
  for (wit = this->InternalWidgetMap->begin();
       wit != this->InternalWidgetMap->end(); ++wit)
    {
    // Need to determine what type of widget we are using so that we
    // can get the value.
    vtkKWSpinBoxWithLabel *sb = vtkKWSpinBoxWithLabel::SafeDownCast((*wit).second);
    vtkKWScaleWithEntry *se = vtkKWScaleWithEntry::SafeDownCast((*wit).second);
    vtkKWCheckButtonWithLabel *cb = vtkKWCheckButtonWithLabel::SafeDownCast((*wit).second);
    vtkKWEntryWithLabel *e = vtkKWEntryWithLabel::SafeDownCast((*wit).second);
    vtkSlicerNodeSelectorWidget *ns = vtkSlicerNodeSelectorWidget::SafeDownCast((*wit).second);
    vtkKWLoadSaveButtonWithLabel *lsb = vtkKWLoadSaveButtonWithLabel::SafeDownCast((*wit).second);
    vtkKWRadioButtonSetWithLabel *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast((*wit).second);

    if (sb)
      {
      // std::cout << "SpinBox" << std::endl;
      n->SetParameterAsDouble((*wit).first, sb->GetWidget()->GetValue());
      }
    else if (se)
      {
      n->SetParameterAsDouble((*wit).first, se->GetValue());
      }
    else if (cb)
      {
      n->SetParameterAsBool((*wit).first, cb->GetWidget()->GetSelectedState());
      }
    else if (e)
      {
      n->SetParameterAsString((*wit).first, e->GetWidget()->GetValue());
      }
    else if (ns && ns->GetSelected() != NULL)
      {
      n->SetParameterAsString((*wit).first, ns->GetSelected()->GetID());
      }
    else if (lsb)
      {
      if (lsb->GetWidget()->GetFileName())
        {
        n->SetParameterAsString((*wit).first, lsb->GetWidget()->GetFileName());
        }
      }
    else if (rbs)
      {
      // find out who is set
      int num = rbs->GetWidget()->GetNumberOfWidgets();
      for (int i=0; i < num; ++i)
        {
        int id = rbs->GetWidget()->GetIdOfNthWidget(i);
        vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
        if (rb->GetSelectedState())
          {
          n->SetParameterAsString((*wit).first, rb->GetValue());
          break;
          }
        }
      }
    }

  if (createdNode)
    {
    this->SetAndObserveMRML( vtkObjectPointer(&this->CommandLineModuleNode),n);
    }
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::UpdateGUI ()
{
  // std::cout << "UpdateGUI()" << std::endl;
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  if (n != NULL)
    {
    // set GUI widgets from parameter node
    ModuleWidgetMap::iterator wit;

    // iterate over each parameter group
    std::vector<ModuleParameterGroup>::const_iterator pgbeginit
      = this->ModuleDescriptionObject.GetParameterGroups().begin();
    std::vector<ModuleParameterGroup>::const_iterator pgendit
      = this->ModuleDescriptionObject.GetParameterGroups().end();
    std::vector<ModuleParameterGroup>::const_iterator pgit;
    
    for (pgit = pgbeginit; pgit != pgendit; ++pgit)
      {
      // iterate over each parameter in this group
      std::vector<ModuleParameter>::const_iterator pbeginit
        = (*pgit).GetParameters().begin();
      std::vector<ModuleParameter>::const_iterator pendit
        = (*pgit).GetParameters().end();
      std::vector<ModuleParameter>::const_iterator pit;
      
      for (pit = pbeginit; pit != pendit; ++pit)
        {
        // find a widget with this name
        wit = this->InternalWidgetMap->find((*pit).GetName());
        if (wit != this->InternalWidgetMap->end())
          {
          // Get the value of this parameter from the node (stored as
          // a default value in the node's ModuleDescription
          std::string value = n->GetParameterAsString( (*pit).GetName() );

          // Need to determine what type of widget we are using so that we
          // can get the value.
          vtkKWSpinBoxWithLabel *sb
            = vtkKWSpinBoxWithLabel::SafeDownCast((*wit).second);
          vtkKWScaleWithEntry *se
            = vtkKWScaleWithEntry::SafeDownCast((*wit).second);
          vtkKWCheckButtonWithLabel *cb
            = vtkKWCheckButtonWithLabel::SafeDownCast((*wit).second);
          vtkKWEntryWithLabel *e
            = vtkKWEntryWithLabel::SafeDownCast((*wit).second);
          vtkSlicerNodeSelectorWidget *ns
            = vtkSlicerNodeSelectorWidget::SafeDownCast((*wit).second);
          vtkKWLoadSaveButtonWithLabel *lsb
            = vtkKWLoadSaveButtonWithLabel::SafeDownCast((*wit).second);
          vtkKWRadioButtonSetWithLabel *rbs = vtkKWRadioButtonSetWithLabel::SafeDownCast((*wit).second);

          if (sb)
            {
            sb->GetWidget()->SetValue(atof(value.c_str()));
            }
          else if (se)
            {
            se->SetValue(atof(value.c_str()));
            }
          else if (cb)
            {
            cb->GetWidget()
              ->SetSelectedState(value=="true" ? 1 : 0 );
            }
          else if (e)
            {
            e->GetWidget()
              ->SetValue(value.c_str());
            }
          else if (ns)
            {
            if (value == "")
              {
              // No value, use the default "None"-node. THis will not
              // work if the selector is configured to select something
              // other than a MRMLScalarVolumeNode
              ns->SetSelected(this->Logic->GetMRMLScene()
                              ->GetNodeByID("None"));
              }
            else
              {
              ns->SetSelected(this->Logic->GetMRMLScene()
                              ->GetNodeByID(value.c_str()));
              }
            }
          else if (lsb)
            {
            lsb->GetWidget()->GetLoadSaveDialog()->SetFileName(value.c_str());
            }
          else if (rbs)
            {
            // set one of the radiobuttons
            int num = rbs->GetWidget()->GetNumberOfWidgets();
            for (int i=0; i < num; ++i)
              {
              int id = rbs->GetWidget()->GetIdOfNthWidget(i);
              vtkKWRadioButton* rb = rbs->GetWidget()->GetWidget(id);
              if (rb->GetValue() == value )
                {
                rb->SetSelectedState(1);
                break;
                }
              }
            }
          }
        }    
      }
    }
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  //std::cout << "ProcessMRMLEvents()" << std::endl;
  // if parameter node has been changed externally, update GUI widgets
  // with new values 
  vtkMRMLCommandLineModuleNode* node
    = vtkMRMLCommandLineModuleNode::SafeDownCast(caller);
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
  helpFrame->CollapseFrame ( );

  vtkKWTextWithScrollbars *helpMessage = vtkKWTextWithScrollbars::New();
  helpMessage->SetParent( helpFrame->GetFrame() );
  helpMessage->Create();
  helpMessage->HorizontalScrollbarVisibilityOff();
  helpMessage->GetWidget()->ReadOnlyOn();
  helpMessage->GetWidget()->SetReliefToFlat();
  helpMessage->GetWidget()->SetText(this->ModuleDescriptionObject.GetDescription().c_str());

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

  this->CommandLineModuleNodeSelector->SetNodeClass("vtkMRMLCommandLineModuleNode", "CommandLineModule", title.c_str(), title.c_str());
  this->CommandLineModuleNodeSelector->SetNewNodeEnabled(1);
  //this->CommandLineModuleNodeSelector->SetNewNodeName((title+" parameters").c_str());
  this->CommandLineModuleNodeSelector->SetParent( moduleFrame->GetFrame() );
  this->CommandLineModuleNodeSelector->Create();
  this->CommandLineModuleNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  this->CommandLineModuleNodeSelector->UpdateMenu();

  this->CommandLineModuleNodeSelector->SetBorderWidth(2);
  this->CommandLineModuleNodeSelector->SetReliefToFlat();
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
          tparameter->GetWidget()->SetIncrement(1);
          tparameter->GetWidget()->RestrictValuesToIntegersOn();
          tparameter->GetWidget()
            ->SetRange(itk::NumericTraits<int>::NonpositiveMin(),
                       itk::NumericTraits<int>::max());
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
          tparameter->GetWidget()
            ->SetRange(itk::NumericTraits<float>::NonpositiveMin(),
                       itk::NumericTraits<float>::max());
          tparameter->GetWidget()->SetIncrement( 0.1 );
          tparameter->GetWidget()->SetValueFormat("%f");
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
            max = atof((*pit).GetMaximum().c_str());
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
          tparameter->GetWidget()
            ->SetRange(itk::NumericTraits<double>::NonpositiveMin(),
                       itk::NumericTraits<double>::max());
          tparameter->GetWidget()->SetIncrement( 0.1 );
          tparameter->GetWidget()->SetValueFormat("%f");
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
            max = atof((*pit).GetMaximum().c_str());
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
               || (*pit).GetTag() == "double-vector"
               || (*pit).GetTag() == "string-vector")
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
        tparameter->SetNodeClass("vtkMRMLScalarVolumeNode",
                                 NULL,
                                 NULL,
                                 (title + " Volume").c_str());
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
        tparameter->UpdateMenu();
        
        tparameter->SetBorderWidth(2);
        tparameter->SetReliefToFlat();
        tparameter->SetLabelText( (*pit).GetLabel().c_str());
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "image" && (*pit).GetChannel() == "output")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        
        tparameter->SetNodeClass("vtkMRMLScalarVolumeNode",
                                 NULL,
                                 NULL,
                                 (title + " Volume").c_str());
        tparameter->SetNewNodeEnabled(1);
        // tparameter->SetNewNodeName((title+" output").c_str());
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
        tparameter->UpdateMenu();
        
        tparameter->SetBorderWidth(2);
        tparameter->SetReliefToFlat();
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
      else if ((*pit).GetTag() == "string-enumeration"
               || (*pit).GetTag() == "integer-enumeration"
               || (*pit).GetTag() == "float-enumeration"
               || (*pit).GetTag() == "double-enumeration")
        {
        vtkKWRadioButtonSetWithLabel *tparameter
          = vtkKWRadioButtonSetWithLabel::New();
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetLabelText( (*pit).GetLabel().c_str() );
        tparameter->GetWidget()->PackHorizontallyOn();
        tparameter->GetWidget()->SetMaximumNumberOfWidgetsInPackingDirection(4);
        std::vector<std::string>::const_iterator sbeginit
          = (*pit).GetElements().begin();
        std::vector<std::string>::const_iterator sendit
          = (*pit).GetElements().end();
        std::vector<std::string>::const_iterator sit;
        int id;
        for(sit = sbeginit, id=0; sit != sendit; ++sit, ++id)
          {
          vtkKWRadioButton *b = tparameter->GetWidget()->AddWidget(id);
          b->SetValue( (*sit).c_str() );
          b->SetText( (*sit).c_str() );
          b->SetAnchorToWest();
          if (*sit == (*pit).GetDefault())
            {
            b->SetSelectedState(1);
            }
          else
            {
            b->SetSelectedState(0);
            }
          }
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
      (*this->InternalWidgetMap)[(*pit).GetName()] = parameter;
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



//---------------------------------------------------------------------------
// Description:
// the NewNodeCallback is a static function that relays observed events from 
// observed widgets into the GUI's 'ProcessGUIEvents" mediator method, which in
// turn makes appropriate changes to the application layer.
//
void vtkCommandLineModuleGUI::NewNodeCallback ( vtkObject *__caller,
                                           unsigned long eid, void *__clientData, void *callData)
{
    static int inCallback = 0;

    vtkCommandLineModuleGUI *self = reinterpret_cast<vtkCommandLineModuleGUI *>(__clientData);

    if ( inCallback )
        {
            vtkErrorWithObjectMacro ( self, "In vtkCommandLineModuleGUI *!* NewNodeCallback called recursively?");
            return;
        }

    vtkDebugWithObjectMacro ( self, "In vtkCommandLineModuleGUI NewNodeCallback");

    inCallback = 1;
    self->ProcessGUIEvents ( __caller, eid, callData );
    inCallback = 0;

}
