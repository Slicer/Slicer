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
#include "vtkKWWidget.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWScale.h"
#include "vtkKWMenu.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWLabelWithLabel.h"
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
#include "vtkKWProgressGauge.h"
#include "vtkStringArray.h"

#include "itkNumericTraits.h"
#include "itksys/DynamicLoader.hxx" 

// Private implementaton of an std::map
class ModuleWidgetMap : public std::map<std::string, vtkSmartPointer<vtkKWCoreWidget> > {};



// Split a comma separated list of file names.  A filename can itsefl
// contain a comma.  So we neeed to split on commas that are not
// within quoted strings.
void
splitFilenames (const std::string& text, vtkStringArray *words)
{
  int n = text.length();
  int start, stop, startq, stopq;
  bool quoted;
  std::string comma(",");
  std::string quote("\"");
  start = text.find_first_not_of(comma);
  while ((start >= 0) && (start < n))
    {
    // find any quotes
    quoted = false;
    startq = text.find_first_of(quote, start);
    stopq = text.find_first_of(quote, startq+1);

    stop = text.find_first_of(comma, start);
    if ((stop < 0) || (stop > n)) stop = n;

    if (startq != std::string::npos && stopq != std::string::npos)
      {
      // start and end quotes found in the string, check if comma was
      // within the quotes, if so keep searching for next comma
      // outside of quotes
      while (startq < stop && stop < stopq && stop != n)
        {
        quoted = true;
        stop = text.find_first_of(comma, stop+1);
        if ((stop < 0) || (stop > n)) stop = n;
        }
      }

    if (!quoted)
      {
      words->InsertNextValue(text.substr(start, stop - start).c_str());
      }
    else
      {
      words->InsertNextValue(text.substr(start+1, stop - start - 2).c_str());
      }
    start = text.find_first_not_of(comma, stop+1);
    }
}


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
  this->Logic = NULL;
  this->CommandLineModuleNode = NULL;

  this->InternalWidgetMap = new ModuleWidgetMap;

  this->NewNodeCallbackCommand = vtkCallbackCommand::New ( );
  this->NewNodeCallbackCommand->SetClientData(reinterpret_cast<void *>(this) );
  this->NewNodeCallbackCommand
    ->SetCallback( vtkCommandLineModuleGUI::NewNodeCallback );
  this->CreatingNewNode = false;
  this->InUpdateMRML = false;
  this->InUpdateGUI = false;
  this->InMRMLCallbackFlag = 0;
//  this->DebugOn();
}

//----------------------------------------------------------------------------
vtkCommandLineModuleGUI::~vtkCommandLineModuleGUI()
{
  this->RemoveMRMLNodeObservers();
  this->RemoveLogicObservers();
  this->RemoveGUIObservers();

  // Delete all the widgets
  delete this->InternalWidgetMap;
  
  this->SetLogic (NULL);
  // wjp test
  if ( this->CommandLineModuleNode ) {
      vtkSetAndObserveMRMLNodeMacro( this->CommandLineModuleNode, NULL );
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

  if (ModuleDescriptionObject.GetLogo().GetBufferLength() != 0)
    {
    vtkKWIcon* logo = vtkKWIcon::New();
    logo->SetImage(ModuleDescriptionObject.GetLogo().GetLogo(),
                   ModuleDescriptionObject.GetLogo().GetWidth(),
                   ModuleDescriptionObject.GetLogo().GetHeight(),
                   ModuleDescriptionObject.GetLogo().GetPixelSize(),
                   ModuleDescriptionObject.GetLogo().GetBufferLength(),
                   ModuleDescriptionObject.GetLogo().GetOptions());
    this->Logo = logo;
    logo->Delete();
    }
  else
    {
    this->Logo = 0;
    }

  this->Modified();
}


void
vtkCommandLineModuleGUI::SetCommandLineModuleNode(vtkMRMLCommandLineModuleNode *node)
{
  //vtkSetObjectBodyMacro(CommandLineModuleNode, vtkMRMLCommandLineModuleNode, node);
  vtkSetAndObserveMRMLNodeMacro( this->CommandLineModuleNode, node);
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
  (*this->InternalWidgetMap)["CommandLineModuleNodeSelector"]->AddObserver (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

  (*this->InternalWidgetMap)["CommandLineModuleNodeSelector"]->AddObserver (vtkSlicerNodeSelectorWidget::NewNodeEvent, (vtkCommand *)this->NewNodeCallbackCommand );  

  (*this->InternalWidgetMap)["ApplyButton"]->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  (*this->InternalWidgetMap)["CancelButton"]->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
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
      lsb->GetWidget()->GetLoadSaveDialog()->AddObserver (vtkKWTopLevel::WithdrawEvent,
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
  ModuleWidgetMap::const_iterator wit;
  ModuleWidgetMap::const_iterator wend;

  wend = (*this->InternalWidgetMap).end();

  wit = (*this->InternalWidgetMap).find("CommandLineModuleNodeSelector");
  if ( wit != wend)
    {
    (*wit).second->RemoveObservers (vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  

    (*wit).second->RemoveObservers (vtkSlicerNodeSelectorWidget::NewNodeEvent, (vtkCommand *)this->NewNodeCallbackCommand );  
    }

  wit = (*this->InternalWidgetMap).find("ApplyButton"); 
  if ( wit != wend )
    {
    (*wit).second->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }

  wit = (*this->InternalWidgetMap).find("CancelButton"); 
  if ( wit != wend )
    {
    (*wit).second->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }
  
  wit =  (*this->InternalWidgetMap).find("DefaultButton");
  if ( wit != wend )
    {
    (*wit).second->RemoveObservers ( vtkKWPushButton::InvokedEvent,  (vtkCommand *)this->GUICallbackCommand );
    }

  // remove observers for each widget created
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
      lsb->GetWidget()->GetLoadSaveDialog()->RemoveObservers (vtkKWTopLevel::WithdrawEvent,
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
  if (this->CreatingNewNode) 
    {
    return;
    }
  // std::cout << "ProcessGUIEvents()" << std::endl;
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *selector = vtkSlicerNodeSelectorWidget::SafeDownCast(caller);
  vtkSlicerNodeSelectorWidget *moduleNodeSelector = vtkSlicerNodeSelectorWidget::SafeDownCast( (*this->InternalWidgetMap)["CommandLineModuleNodeSelector"].GetPointer() );

  if (selector && selector == moduleNodeSelector
      && event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) 
    {
    // Selected a new parameter node
    // std::cout << "  Selector" << std::endl;
    vtkMRMLCommandLineModuleNode* n = vtkMRMLCommandLineModuleNode::SafeDownCast(moduleNodeSelector->GetSelected());
    if (n == NULL) 
      {
      return;
      }
    this->Logic->SetCommandLineModuleNode(n);
    vtkSetAndObserveMRMLNodeMacro( this->CommandLineModuleNode, n);

    this->UpdateGUI();
    }
  else if (selector && selector == moduleNodeSelector && event == vtkSlicerNodeSelectorWidget::NewNodeEvent )
    {
    // creating a new parameter node
    //std::cout << "  New node" << std::endl;
    vtkMRMLCommandLineModuleNode* n = vtkMRMLCommandLineModuleNode::SafeDownCast((vtkObjectBase*)callData);
    n->SetModuleDescription( this->ModuleDescriptionObject );
    }
  else if (selector && selector == moduleNodeSelector && selector->GetSelected() == NULL)
    {
    return;
    }
  else if (b == (*this->InternalWidgetMap)["ApplyButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
    {
    // Apply button was pressed
    //std::cout << "  Apply" << std::endl;
    this->UpdateMRML();
    this->Logic->SetTemporaryDirectory( ((vtkSlicerApplication*)this->GetApplication())->GetTemporaryDirectory() );

    // Lazy evaluation of module target
    if (this->ModuleDescriptionObject.GetTarget() == "Unknown")
      {
      // What about python targets?
      if (this->ModuleDescriptionObject.GetType() == "SharedObjectModule")
        {
        typedef int (*ModuleEntryPoint)(int argc, char* argv[]);
        
        itksys::DynamicLoader::LibraryHandle lib
          = itksys::DynamicLoader::OpenLibrary(this->ModuleDescriptionObject.GetLocation().c_str());
        if ( lib )
          {
          ModuleEntryPoint entryPoint
            = (ModuleEntryPoint)itksys::DynamicLoader::GetSymbolAddress(lib, "ModuleEntryPoint");

          if (entryPoint)
            {
            char entryPointAsText[256];
            std::string entryPointAsString;
            
            sprintf(entryPointAsText, "%p", entryPoint);
            entryPointAsString = std::string("slicer:") + entryPointAsText;
            
            this->ModuleDescriptionObject.SetTarget( entryPointAsString );
            }
          else
            {
            // can't find entry point, eject.
            itksys::DynamicLoader::CloseLibrary(lib);

            vtkErrorMacro(<< "Cannot find entry point for " << this->ModuleDescriptionObject.GetLocation() << "\nCannot run module." );
            return;
            }
          }
        }
      }
    // make sure the entry point is set on the node
    this->GetCommandLineModuleNode()->GetModuleDescription()
      .SetTarget( this->ModuleDescriptionObject.GetTarget() );
    
    // apply
    this->Logic->Apply();
    }
  else if (b == (*this->InternalWidgetMap)["CancelButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
    {
    // Abort button was pressed
    //std::cout << "  Cancel" << std::endl;
    this->GetCommandLineModuleNode()->GetModuleDescription().GetProcessInformation()->Abort = 1;
    this->GetCommandLineModuleNode()->SetStatus(vtkMRMLCommandLineModuleNode::Cancelled);
    }
  else if (b == (*this->InternalWidgetMap)["DefaultButton"].GetPointer() && event == vtkKWPushButton::InvokedEvent ) 
    {
    // Defaults button was pressed
    // (may need additional code to get any node selection widgets to
    // return to their default state)
    if (this->CommandLineModuleNode != NULL) 
      {
      this->CommandLineModuleNode
        ->SetModuleDescription( this->ModuleDescriptionObject);
      this->UpdateGUI();
      }
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
  if (this->InUpdateGUI) {
    return;
  }

  this->InUpdateMRML = true;
  // std::cout << "UpdateMRML()" << std::endl;
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  bool createdNode = false;
  if (n == NULL)
    {
    // no parameter node selected yet, create new
    //std::cout << "  Creating a new node" << std::endl;
    vtkSlicerNodeSelectorWidget *moduleNodeSelector = vtkSlicerNodeSelectorWidget::SafeDownCast((*this->InternalWidgetMap)["CommandLineModuleNodeSelector"]);
    moduleNodeSelector->SetSelectedNew("vtkMRMLCommandLineModuleNode");
    this->CreatingNewNode = true;
    moduleNodeSelector->ProcessNewNodeCommand("vtkMRMLCommandLineModuleNode", this->ModuleDescriptionObject.GetTitle().c_str());
    n = vtkMRMLCommandLineModuleNode::SafeDownCast(moduleNodeSelector->GetSelected());
    this->CreatingNewNode = false;

    if (n == NULL)
      {
      this->InUpdateMRML = false;
      vtkDebugMacro("No CommandLineModuleNode available");
      return;
      }
    
    // set the a module description for this node
    n->SetModuleDescription( this->ModuleDescriptionObject );
    
    // set an observe new node in Logic
    this->Logic->SetCommandLineModuleNode(n);
    vtkSetAndObserveMRMLNodeMacro(this->CommandLineModuleNode,n);

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
      int numberOfFiles
        = lsb->GetWidget()->GetLoadSaveDialog()->GetNumberOfFileNames();
      if (numberOfFiles > 0)
        {
        // build a comma separated list of file names
        std::string names;
        for (int i=0; i < numberOfFiles; ++i)
          {
          // get a filename
          std::string n
            = lsb->GetWidget()->GetLoadSaveDialog()->GetNthFileName(i);

          // quote it as needed (if multiple filenames and a filename
          // contains a comma and is not already quoted, then quote it)
          if (lsb->GetWidget()->GetLoadSaveDialog()->GetMultipleSelection())
            {
            int s1, len;
            len = n.length();
            s1 = n.find_first_of(",");
            if (s1 > 0 && s1 < len)
              {
              // filename contains a comma
              int q1, qn;
              q1 = n.find_first_of("\"");
              qn = n.find_last_of("\"");
              if (q1 != 0 && qn != len-1)
                {
                // first and last character in name are not quotes, so
                // quote
                n = std::string("\"") + n + "\"";
                }
              }
            }
          
          // add it to the list
          names = names + n;
          if (i < numberOfFiles-1)  // comma after all but last
            {
            names = names + ",";
            }
          }
        //vtkWarningMacro(<< "Selected filenames: " << names);
        n->SetParameterAsString((*wit).first, names);

        // set the filenames as the selected filenames for next time
        lsb->GetWidget()->GetLoadSaveDialog()->SetInitialSelectedFileNames( lsb->GetWidget()->GetLoadSaveDialog()->GetFileNames() );
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
    vtkSetAndObserveMRMLNodeMacro( this->CommandLineModuleNode,n);
    }
  this->InUpdateMRML = false;
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::UpdateGUI ()
{
  if (this->InUpdateMRML) {
    return;
  }

  this->InUpdateGUI = true;
  // vtkWarningMacro(<<"UpdateGUI()");
  // std::cout << "UpdateGUI()" << std::endl;
  vtkMRMLCommandLineModuleNode* n = this->GetCommandLineModuleNode();
  std::string statusString;
  if (n != NULL)
    {
    // Show the status of the node
    switch (n->GetStatus())
      {
      case vtkMRMLCommandLineModuleNode::Idle: statusString = "Idle";
        break;
      case vtkMRMLCommandLineModuleNode::Scheduled: statusString = "Scheduled";
        break;
      case vtkMRMLCommandLineModuleNode::Running: statusString = "Running";
        break;
      case vtkMRMLCommandLineModuleNode::Completed: statusString = "Completed";
        break;
      case vtkMRMLCommandLineModuleNode::CompletedWithErrors: statusString = "Completed with errors";
        break;
      case vtkMRMLCommandLineModuleNode::Cancelled: statusString = "Cancelled";
        break;
      default:
        statusString = "Idle";
        break;
      }
    vtkKWLabelWithLabel *statusWidget =
      vtkKWLabelWithLabel::SafeDownCast((*this->InternalWidgetMap)["Status"]);
    statusWidget->GetWidget()->SetText( statusString.c_str() );

    // If node is not scheduled or running, the enable all widgets.
    // Otherwise turn off all the widget except Cancel
    if (n->GetStatus() == vtkMRMLCommandLineModuleNode::Running
        || n->GetStatus() == vtkMRMLCommandLineModuleNode::Scheduled)
      {
      // running or scheduled, disable widgets
      ModuleWidgetMap::const_iterator wit;
      for (wit = this->InternalWidgetMap->begin();
         wit != this->InternalWidgetMap->end(); ++wit)
        {
        vtkSmartPointer<vtkKWCoreWidget> w = (*wit).second;
        w->EnabledOff();
        w->UpdateEnableState();
        }
      (*this->InternalWidgetMap)["Status"]->EnabledOn();
      (*this->InternalWidgetMap)["Status"]->UpdateEnableState();
      (*this->InternalWidgetMap)["CancelButton"]->EnabledOn();
      (*this->InternalWidgetMap)["CancelButton"]->UpdateEnableState();
      }
    else
      {
      // idle, completed, or cancelled, enable widgets
      ModuleWidgetMap::const_iterator wit;
      for (wit = this->InternalWidgetMap->begin();
         wit != this->InternalWidgetMap->end(); ++wit)
        {
        vtkSmartPointer<vtkKWCoreWidget> w = (*wit).second;
        w->EnabledOn();
        w->UpdateEnableState();
        }
      (*this->InternalWidgetMap)["CancelButton"]->EnabledOff();
      (*this->InternalWidgetMap)["CancelButton"]->UpdateEnableState();
      }

    // Set the progress value and balloon help
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetValue(n->GetModuleDescription().GetProcessInformation()->Progress * 100);
    this->GetApplicationGUI()->GetMainSlicerWindow()->GetProgressGauge()->SetNthValue(1, n->GetModuleDescription().GetProcessInformation()->StageProgress * 100);

    std::string message = statusString + ": " +
      n->GetModuleDescription().GetProcessInformation()->ProgressMessage;

    message = message + "(" + n->GetName() +")";

    std::ostrstream strvalue;
    strvalue << std::setiosflags(ios::fixed) << std::setprecision(2)
             << n->GetModuleDescription().GetProcessInformation()->ElapsedTime;
    strvalue << std::ends;
    
    message = message + ", " + strvalue.str() + "s";
    strvalue.rdbuf()->freeze(0);

    this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText( message.c_str() );
    
    
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
            vtkSmartPointer<vtkStringArray> names = vtkStringArray::New();
            splitFilenames(value, names);
            //vtkWarningMacro(<<"Filenames being set: " << value);
            lsb->GetWidget()->GetLoadSaveDialog()->SetInitialSelectedFileNames(names);
            names->Delete();
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
    this->InUpdateGUI = false;

}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  //std::cout << "ProcessMRMLEvents()" << std::endl;
  // if parameter node has been changed externally, update GUI widgets
  // with new values 
 if (this->CreatingNewNode) 
    {
    return;
    }
  vtkMRMLCommandLineModuleNode* node
    = vtkMRMLCommandLineModuleNode::SafeDownCast(caller);
  if (node != NULL && this->GetCommandLineModuleNode() == node) 
    {
    this->UpdateGUI();
    }
}



//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::CreateModuleEventBindings ( )
{
}

//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::ReleaseModuleEventBindings ( )
{
  
}


//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::Enter ( )
{
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;
    this->AddGUIObservers();
    this->Logic->SetTemporaryDirectory( ((vtkSlicerApplication*)this->GetApplication())->GetTemporaryDirectory() );
    }
    this->CreateModuleEventBindings();
}



//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::Exit ( )
{
  this->ReleaseModuleEventBindings();
}


//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::TearDownGUI ( )
{
  this->Exit();
  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//---------------------------------------------------------------------------
void vtkCommandLineModuleGUI::BuildGUI ( ) 
{
  std::map<std::string, std::string> defaultExtensionMap;
  defaultExtensionMap[".png"] = "Portable Network Graphics";
  defaultExtensionMap[".jpg"] = "JPEG";
  defaultExtensionMap[".jpeg"] = "JPEG";
  defaultExtensionMap[".bmp"] = "BMP";
  defaultExtensionMap[".tiff"] = "TIFF";
  defaultExtensionMap[".tif"] = "TIFF";
  defaultExtensionMap[".gipl"] = "GIPL";

  defaultExtensionMap[".dcm"] = "DICOM";
  defaultExtensionMap[".dicom"] = "DICOM";
  defaultExtensionMap[".nhdr"] = "NRRD";
  defaultExtensionMap[".nrrd"] = "NRRD";
  defaultExtensionMap[".mhd"] = "META";
  defaultExtensionMap[".mha"] = "META";
  defaultExtensionMap[".mask"] = "Brains2 Mask";
  defaultExtensionMap[".hdr"] = "Analyze or NIFTI";
  defaultExtensionMap[".hdr.gz"] = "Analyze or NIFTI";
  defaultExtensionMap[".nii"] = "NIFTI";
  defaultExtensionMap[".nii.gz"] = "NIFTI";

  defaultExtensionMap[".pic"] = "Bio-Rad";
  defaultExtensionMap[".lsm"] = "LSM (Zeiss)";
  defaultExtensionMap[".spr"] = "STD/SPR (Stimulate)";

  defaultExtensionMap[".vtk"] = "VTK (Legacy)";
  defaultExtensionMap[".vtp"] = "VTK PolyData";
  defaultExtensionMap[".vti"] = "VTK ImageData";
  defaultExtensionMap[".stl"] = "Stereolithography";

  defaultExtensionMap[".csv"] = "Comma Separated Value";
  defaultExtensionMap[".txt"] = "Text Document";
  defaultExtensionMap[".xml"] = "XML Document";
  defaultExtensionMap[".html"] = "HTML Document";

  
  std::string title = this->ModuleDescriptionObject.GetTitle();
  
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  this->UIPanel->AddPage ( title.c_str(), title.c_str(), NULL );

  // Build the Help and About frame
  this->BuildHelpAndAboutFrame(this->UIPanel->GetPageWidget ( title.c_str() ),
                    this->ModuleDescriptionObject.GetDescription().c_str(),
                    (this->ModuleDescriptionObject.GetAcknowledgements()
                     + "\n\n" + 
                     this->ModuleDescriptionObject.GetContributor()).c_str());

  // If the module has a logo, then add it to logo frame
  if (this->GetLogo())
    {
    vtkKWLabel *logoLabel = vtkKWLabel::New();
    logoLabel->SetParent( this->GetLogoFrame() );
    logoLabel->Create();
    logoLabel->SetImageToIcon( this->GetLogo() );
    app->Script("pack %s", logoLabel->GetWidgetName() );
    
    (*this->InternalWidgetMap)["LogoLabel"] = logoLabel;
    logoLabel->Delete();
    }
  
  // Make a frame for the module parameters
  vtkSlicerModuleCollapsibleFrame *moduleFrame = vtkSlicerModuleCollapsibleFrame::New ( );
  moduleFrame->SetParent ( this->UIPanel->GetPageWidget ( title.c_str() ) );
  moduleFrame->Create ( );
  moduleFrame->SetLabelText (title.c_str());
  moduleFrame->ExpandFrame ( );
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                moduleFrame->GetWidgetName(), this->UIPanel->GetPageWidget(title.c_str())->GetWidgetName());

  // Store the module frame widget in a SmartPointer
  (*this->InternalWidgetMap)["ModuleFrame"] = moduleFrame;
  moduleFrame->Delete();

  vtkSlicerNodeSelectorWidget*
    moduleNodeSelector = vtkSlicerNodeSelectorWidget::New();
  moduleNodeSelector->ShowHiddenOn();
  moduleNodeSelector->SetNodeClass("vtkMRMLCommandLineModuleNode", "CommandLineModule", title.c_str(), title.c_str());
  moduleNodeSelector->SetNewNodeEnabled(1);
  moduleNodeSelector->SetParent( moduleFrame->GetFrame() );
  moduleNodeSelector->Create();
  moduleNodeSelector->SetMRMLScene(this->Logic->GetMRMLScene());
  moduleNodeSelector->SetSelected(NULL);  // force empt select

  moduleNodeSelector->SetBorderWidth(2);
  moduleNodeSelector->SetReliefToFlat();
  moduleNodeSelector->SetLabelText( "Parameter set");


  std::string nodeSelectorBalloonHelp = "select a \"" + title + " parameters\" node from the current mrml scene.";
  moduleNodeSelector->SetBalloonHelpString(nodeSelectorBalloonHelp.c_str());
  app->Script("pack %s -side top -anchor e -padx 20 -pady 4", 
              moduleNodeSelector->GetWidgetName());

  (*this->InternalWidgetMap)["CommandLineModuleNodeSelector"]
    = moduleNodeSelector;
  moduleNodeSelector->Delete();

  // Add a block indicating the status of the module (on this
  // parameter set)
  vtkKWLabelWithLabel *statusText = vtkKWLabelWithLabel::New();
  statusText->SetParent( moduleFrame->GetFrame() );
  statusText->Create();
  statusText->SetLabelText("Status");
  statusText->GetWidget()->SetText("Idle");
  app->Script ( "pack %s -side top -anchor ne -padx 2 -pady 2",
                statusText->GetWidgetName() );
  (*this->InternalWidgetMap)["Status"] = statusText;
  statusText->Delete();

  
  // iterate over each parameter group
  std::vector<ModuleParameterGroup>::const_iterator pgbeginit
    = this->ModuleDescriptionObject.GetParameterGroups().begin();
  std::vector<ModuleParameterGroup>::const_iterator pgendit
    = this->ModuleDescriptionObject.GetParameterGroups().end();
  std::vector<ModuleParameterGroup>::const_iterator pgit;

  for (pgit = pgbeginit; pgit != pgendit; ++pgit)
    {
    // each parameter group is its own labeled frame
    vtkSlicerModuleCollapsibleFrame *parameterGroupFrame = vtkSlicerModuleCollapsibleFrame::New ( );
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

      // parameters with flags can support the None node because they
      // are optional
      int noneEnabled = 0;
      if ((*pit).GetLongFlag() != "" || (*pit).GetFlag() != "")
        {
        noneEnabled = 1;
        }
      
      if ((*pit).GetTag() == "integer")
        {
        if ((*pit).GetConstraints() == "")
          {
          vtkKWSpinBoxWithLabel *tparameter = vtkKWSpinBoxWithLabel::New();
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->Create();
          tparameter->SetLabelText((*pit).GetLabel().c_str());
          tparameter->GetWidget()->SetRestrictValueToInteger();
          tparameter->GetWidget()->SetIncrement(1);
          tparameter->GetWidget()->SetValue(atof((*pit).GetDefault().c_str()));
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
          tparameter->RangeVisibilityOn();
          tparameter->SetRange(min, max);
          tparameter->SetValue(atof((*pit).GetDefault().c_str()));
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
          tparameter->GetWidget()
            ->SetRange(itk::NumericTraits<float>::NonpositiveMin(),
                       itk::NumericTraits<float>::max());
          tparameter->GetWidget()->SetIncrement( 0.1 );
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
          tparameter->RangeVisibilityOn();
          tparameter->SetRange(min, max);
          tparameter->SetResolution(step);
          tparameter->SetValue(atof((*pit).GetDefault().c_str()));
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
          tparameter->GetWidget()
            ->SetRange(itk::NumericTraits<double>::NonpositiveMin(),
                       itk::NumericTraits<double>::max());
          tparameter->GetWidget()->SetIncrement( 0.1 );
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
          tparameter->RangeVisibilityOn();
          tparameter->SetRange(min, max);
          tparameter->SetResolution(step);
          tparameter->SetValue(atof((*pit).GetDefault().c_str()));
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
      else if ((*pit).GetTag() == "point")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        
        tparameter->SetNodeClass("vtkMRMLFiducialListNode",
                                 NULL,
                                 NULL,
                                 (title + " FiducialList").c_str());
        tparameter->SetNewNodeEnabled(1);
        tparameter->SetNoneEnabled(noneEnabled);
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
      else if ((*pit).GetTag() == "region")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        
        tparameter->SetNodeClass("vtkMRMLROIListNode",
                                 NULL,
                                 NULL,
                                 (title + " RegionList").c_str());
        tparameter->SetNewNodeEnabled(1);
        tparameter->SetNoneEnabled(noneEnabled);
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
      else if ((*pit).GetTag() == "image" && (*pit).GetChannel() == "input")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        std::string labelAttrName("LabelMap");
        std::string labelAttrValue("1");
        std::string nodeClass;
        const char *attrName = 0;
        const char *attrValue = 0;
        if ((*pit).GetType() == "any")
          {
          nodeClass = "vtkMRMLVolumeNode";
          }
        else if ((*pit).GetType() == "label")
          {
          nodeClass = "vtkMRMLScalarVolumeNode";
          attrName = labelAttrName.c_str();
          attrValue = labelAttrValue.c_str();
          }
        else if ((*pit).GetType() == "vector")
          {
          nodeClass = "vtkMRMLVectorVolumeNode";
          }
        else if ((*pit).GetType() == "tensor")
          {
          nodeClass = "vtkMRMLDiffusionTensorVolumeNode";
          }
        else if ((*pit).GetType() == "diffusion-weighted")
          {
          nodeClass = "vtkMRMLDiffusionWeightedVolumeNode";
          }
        else
          {
          nodeClass = "vtkMRMLScalarVolumeNode";
          }

        tparameter->SetNodeClass(nodeClass.c_str(), attrName, attrValue, 
                                 (title + " Volume").c_str());
        tparameter->SetNoneEnabled(noneEnabled);
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
        std::string labelAttrName("LabelMap");
        std::string labelAttrValue("1");
        std::string nodeClass;
        const char *attrName = 0;
        const char *attrValue = 0;
        if ((*pit).GetType() == "label")
          {
          nodeClass = "vtkMRMLScalarVolumeNode";
          attrName = labelAttrName.c_str();
          attrValue = labelAttrValue.c_str();
          }
        else if ((*pit).GetType() == "vector")
          {
          nodeClass = "vtkMRMLVectorVolumeNode";
          }
        else if ((*pit).GetType() == "tensor")
          {
          nodeClass = "vtkMRMLDiffusionTensorVolumeNode";
          }
        else if ((*pit).GetType() == "diffusion-weighted")
          {
          nodeClass = "vtkMRMLDiffusionWeightedVolumeNode";
          }
        else
          {
          nodeClass = "vtkMRMLScalarVolumeNode";
          }

        tparameter->SetNodeClass(nodeClass.c_str(), attrName, attrValue, 
                                 (title + " Volume").c_str());
        if ((*pit).GetType() == "any")
          {
          // Add all of the other concrete volume node types
          tparameter->AddNodeClass("vtkMRMLVectorVolumeNode",
                                   attrName, attrValue, 
                                   (title + " VectorVolume").c_str());
          tparameter->AddNodeClass("vtkMRMLDiffusionTensorVolumeNode",
                                   attrName, attrValue, 
                                   (title + " DiffusionTensorVolume").c_str());
          tparameter->AddNodeClass("vtkMRMLDiffusionWeightedVolumeNode",
                                   attrName, attrValue, 
                                   (title + " DiffusionWeightedVolume").c_str());
          }
        tparameter->SetNewNodeEnabled(1);
        tparameter->SetNoneEnabled(noneEnabled);
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
      else if ((*pit).GetTag() == "geometry" && (*pit).GetChannel() == "input")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();

        std::string nodeClass;
        if((*pit).GetType() == "fiberbundle")
          nodeClass = "vtkMRMLFiberBundleNode";
        else //   "model"
          nodeClass = "vtkMRMLModelNode";

        if ((*pit).GetMultiple() == "true" && (*pit).GetAggregate() == "true")
          {
          nodeClass = "vtkMRMLModelHierarchyNode";
          tparameter->ShowHiddenOn();
          }
        
        tparameter->SetNodeClass(nodeClass.c_str(),
                                 NULL,
                                 NULL,
                                 (title + " Model").c_str());
        tparameter->SetNoneEnabled(noneEnabled);
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
        tparameter->UpdateMenu();
        
        tparameter->SetBorderWidth(2);
        tparameter->SetReliefToFlat();
        tparameter->SetLabelText( (*pit).GetLabel().c_str());
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "geometry" && (*pit).GetChannel() =="output")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();
        
        std::string nodeClass;
        if((*pit).GetType() == "fiberbundle")
          nodeClass = "vtkMRMLFiberBundleNode";
        else // "model"
          nodeClass = "vtkMRMLModelNode";

        if ((*pit).GetMultiple() == "true" && (*pit).GetAggregate() == "true")
          {
          nodeClass = "vtkMRMLModelHierarchyNode";
          tparameter->ShowHiddenOn();
          }

        tparameter->SetNodeClass(nodeClass.c_str(),
                                 NULL,
                                 NULL,
                                 (title + " Model").c_str());
        tparameter->SetNewNodeEnabled(1);
        tparameter->SetNoneEnabled(noneEnabled);
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
      else if ((*pit).GetTag() == "table" && (*pit).GetChannel() == "input")
        {
        if ((*pit).GetHidden() != "true")
          {
          vtkSlicerNodeSelectorWidget *tparameter
            = vtkSlicerNodeSelectorWidget::New();
          
          std::string nodeClass;
          if((*pit).GetType() == "color")
            nodeClass = "vtkMRMLColorNode";
          else
            {
            vtkErrorMacro(<< "Only color tables are currently supported.");
            }
          // else 
          //  nodeClass = "vtkMRMLTableNode";
          
          tparameter->SetNodeClass(nodeClass.c_str(),
                                   NULL,
                                   NULL,
                                   (title + " Table").c_str());
          tparameter->SetParent( parameterGroupFrame->GetFrame() );
          tparameter->Create();
          tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
          tparameter->UpdateMenu();
          
          tparameter->SetBorderWidth(2);
          tparameter->SetReliefToFlat();
          tparameter->SetLabelText( (*pit).GetLabel().c_str());
          parameter = tparameter;
          }
        else
          {
          parameter = 0;
          }
        }
      else if ((*pit).GetTag() == "table" && (*pit).GetChannel() == "output")
        {
        if ((*pit).GetHidden() != "true")
          {
          vtkSlicerNodeSelectorWidget *tparameter
            = vtkSlicerNodeSelectorWidget::New();
          
          std::string nodeClass;
          if((*pit).GetType() == "color")
            nodeClass = "vtkMRMLColorNode";
          else
            {
            vtkErrorMacro(<< "Only color tables are currently supported.");
            }
          // else 
          //  nodeClass = "vtkMRMLTableNode";
          
          tparameter->SetNodeClass(nodeClass.c_str(),
                                   NULL,
                                   NULL,
                                   (title + " Table").c_str());
          tparameter->SetNewNodeEnabled(1);
          tparameter->SetNoneEnabled(1);
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
        else
          {
          parameter = 0;
          }
        }
      else if ((*pit).GetTag() == "transform" && (*pit).GetChannel() == "input")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();

        std::string nodeClass = "vtkMRMLTransformNode";
        if ((*pit).GetType() == "linear")
          {
          nodeClass = "vtkMRMLLinearTransformNode";
          }
        else if ((*pit).GetType() == "nonlinear")
          {
          nodeClass = "vtkMRMLGridTransformNode";
          }
        else if ((*pit).GetType() == "bspline")
          {
          nodeClass = "vtkMRMLBSplineTransformNode";
          }

        tparameter->SetNodeClass(nodeClass.c_str(),
                                 NULL,
                                 NULL,
                                 (title + " Transform").c_str());
        tparameter->SetNoneEnabled(noneEnabled);
        tparameter->SetParent( parameterGroupFrame->GetFrame() );
        tparameter->Create();
        tparameter->SetMRMLScene(this->Logic->GetMRMLScene());
        tparameter->UpdateMenu();
        
        tparameter->SetBorderWidth(2);
        tparameter->SetReliefToFlat();
        tparameter->SetLabelText( (*pit).GetLabel().c_str());
        parameter = tparameter;
        }
      else if ((*pit).GetTag() == "transform" && (*pit).GetChannel() =="output")
        {
        vtkSlicerNodeSelectorWidget *tparameter
          = vtkSlicerNodeSelectorWidget::New();

        // Note: TransformNode is abstract making it inappropriate for
        // an output type since the node selector must be able to make
        // an instance of the class.  For now, revert to LinearTransformNode.

        std::string nodeClass = "vtkMRMLLinearTransformNode";
        if ((*pit).GetType() == "linear")
          {
          nodeClass = "vtkMRMLLinearTransformNode";
          }
        else if ((*pit).GetType() == "nonlinear")
          {
          nodeClass = "vtkMRMLGridTransformNode";
          }
        else if ((*pit).GetType() == "bspline")
          {
          nodeClass = "vtkMRMLBSplineTransformNode";
          }


        tparameter->SetNodeClass(nodeClass.c_str(),
                                 NULL,
                                 NULL,
                                 (title + " Transform").c_str());
        tparameter->SetNewNodeEnabled(1);
        tparameter->SetNoneEnabled(noneEnabled);
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
        tparameter->GetWidget()->SetText( (*pit).GetDefault().c_str() );
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
        if ( (*pit).GetMultiple() == "true" )
          {
          tparameter->GetWidget()->GetLoadSaveDialog()->MultipleSelectionOn();
          }
        tparameter->Create();
        tparameter->SetLabelText( (*pit).GetLabel().c_str() );
        tparameter->GetWidget()->SetText( (*pit).GetDefault().c_str() );
        tparameter->GetWidget()->GetLoadSaveDialog()->SetInitialFileName( (*pit).GetDefault().c_str() );
        
        vtkSmartPointer<vtkStringArray> names = vtkStringArray::New();
        splitFilenames((*pit).GetDefault(), names);
        tparameter->GetWidget()->GetLoadSaveDialog()->SetInitialSelectedFileNames( names );
        names->Delete();
        if ((*pit).GetFileExtensions().size() != 0)
          {
          std::string extensionVector;
          std::string fileTypeString;
          std::vector<std::string>::const_iterator begIt
            = (*pit).GetFileExtensions().begin();
          std::vector<std::string>::const_iterator endIt
            = (*pit).GetFileExtensions().end();
          std::vector<std::string>::const_iterator it;
          std::map<std::string, std::string>::iterator mit;

          for (it = begIt; it != endIt; ++it)
            {
            mit = defaultExtensionMap.find( *it );
            if (mit != defaultExtensionMap.end())
              {
              fileTypeString = (*mit).second;
              }
            else
              {
              fileTypeString = "";
              }
            extensionVector.append("{");
            extensionVector.append( "{" + fileTypeString + "} {" + *it + "}" );
            extensionVector.append("} "); // note the space!
            }
          extensionVector.append("{{All files} {*}} ");
          
          tparameter->GetWidget()->GetLoadSaveDialog()
            ->SetFileTypes( extensionVector.c_str() );
          }
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

      // parameter is set iff hidden != true
      if (parameter)
        {
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

  // Create a "Cancel" button
  vtkKWPushButton *cancel = vtkKWPushButton::New();
  cancel->SetParent( moduleFrame->GetFrame() );
  cancel->Create();
  cancel->SetText("Cancel");
  cancel->SetWidth ( 8 );
  app->Script("pack %s -side right -anchor e -padx 20 -pady 10", 
              cancel->GetWidgetName());

  std::string cancelBalloonHelp("Cancel the execution of the module");
  cancel->SetBalloonHelpString(cancelBalloonHelp.c_str());

  (*this->InternalWidgetMap)["CancelButton"] = cancel;
  cancel->Delete();

  (*this->InternalWidgetMap)["CancelButton"]->EnabledOff();
  (*this->InternalWidgetMap)["CancelButton"]->UpdateEnableState();
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
    vtkCommandLineModuleGUI *self = reinterpret_cast<vtkCommandLineModuleGUI *>(__clientData);

    if ( self->GetInMRMLCallbackFlag() )
      {
#ifdef _DEBUG
      vtkDebugWithObjectMacro ( self, "In vtkCommandLineModuleGUI *!* NewNodeCallback called recursively?");
#endif
      return;
      }

    vtkDebugWithObjectMacro ( self, "In vtkCommandLineModuleGUI NewNodeCallback");

    self->SetInMRMLCallbackFlag(1);
    self->ProcessGUIEvents ( __caller, eid, callData );
    self->SetInMRMLCallbackFlag(0);

}


