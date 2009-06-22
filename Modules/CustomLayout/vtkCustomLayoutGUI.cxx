#include <string>
#include <iostream>
#include <sstream>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkCustomLayoutGUI.h"
#include "vtkSlicerWaitMessageWidget.h"
#include "vtkSlicerApplication.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWEvent.h"
#include "vtkKWEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWFrameWithLabel.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWNotebook.h"
#include "vtkKWLabel.h"
#include "vtkKWCanvas.h"

#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>

#define _br 0.945
#define _bg 0.945
#define _bb 0.975

#define _fr 0.75
#define _fg 0.75
#define _fb 0.75


//------------------------------------------------------------------------------
vtkCustomLayoutGUI* vtkCustomLayoutGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkCustomLayoutGUI");
  if(ret)
    {
      return (vtkCustomLayoutGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkCustomLayoutGUI;
}


//----------------------------------------------------------------------------
vtkCustomLayoutGUI::vtkCustomLayoutGUI()
{
  this->Logic = NULL;
  this->CustomLayoutNode = NULL;
  this->Raised = false;

}

//----------------------------------------------------------------------------
vtkCustomLayoutGUI::~vtkCustomLayoutGUI()
{
    this->RemoveMRMLNodeObservers ( );
    this->RemoveLogicObservers ( );

    this->UpdatingMRML = 0;
    this->UpdatingGUI = 0;
    
    if ( this->Logic )
      {
      this->Logic->Delete();
      this->Logic = NULL;
      }

    vtkSetAndObserveMRMLNodeMacro( this->CustomLayoutNode, NULL );
    this->Raised = false;
}



//----------------------------------------------------------------------------
void vtkCustomLayoutGUI::Enter()
{
  
  
  //--- mark as currently being visited.
  this->Raised = true;


  //--- only build when first visited.
  if ( this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;

    //--- Do a parallel thing in Logic
    this->Logic->Enter();

    //--- Set up GUI observers 
    vtkIntArray *guiEvents = this->NewObservableEvents ( );
    if ( guiEvents != NULL )
      {
      this->SetAndObserveMRMLSceneEvents ( this->MRMLScene, guiEvents );
      guiEvents->Delete();
      }
    }

  this->AddGUIObservers();    

  //--- make GUI reflect current MRML state
  this->UpdateGUI();
}


//----------------------------------------------------------------------------
void vtkCustomLayoutGUI::Exit ( )
{

  //--- mark as no longer selected.
  this->Raised = false;

  this->RemoveGUIObservers();

  //--- Do a parallel thing in Logic
  this->Logic->Exit();
}


//----------------------------------------------------------------------------
vtkIntArray *vtkCustomLayoutGUI::NewObservableEvents()
{
  
 vtkIntArray *events = vtkIntArray::New();
  events->InsertNextValue(vtkMRMLScene::SceneCloseEvent);
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkCustomLayoutGUI::TearDownGUI ( )
{
  if ( !this->Built )
    {
    return;
    }

  if ( this->CustomLayoutNode )
    {
    vtkSetAndObserveMRMLNodeMacro( this->CustomLayoutNode, NULL );
    }
  this->RemoveGUIObservers ( );
  this->Logic->SetCustomLayoutNode ( NULL );
  this->SetLogic ( NULL );
  this->SetAndObserveMRMLScene ( NULL );
}


//----------------------------------------------------------------------------
void vtkCustomLayoutGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  
}


//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::AddGUIObservers ( ) 
{
  if ( !this->Built )
    {
    return;
    }

}



//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::RemoveGUIObservers ( )
{
  if ( !this->Built )
    {
    return;
    }

}




//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::RemoveMRMLNodeObservers ( ) {
    // Fill in.
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::RemoveLogicObservers ( ) {
    // Fill in
}




//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::RaiseSplitPaneMenu()
{
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::WithdrawSplitPaneMenu()
{
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::DestroySplitPaneMenu()
{
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::RaiseContentMenu()
{
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::WithdrawContentMenu()
{
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::DestroyContentMenu()
{
}



//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData ) 
{

  if ( !this->Built )
    {
    return;
    }
  if ( this->CustomLayoutNode == NULL )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got NULL CustomLayoutNode" );
    return;
    }
  if ( this->Logic == NULL )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got NULL Module Logic" );
    return;
    }

  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWEntry *e = vtkKWEntry::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
}


//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::SetEchoText(const char *txt)
{
}

//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::SetSlicerText(const char *txt)
{
}





//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::ProcessMRMLEvents ( vtkObject *caller,
                                            unsigned long event,
                                            void *callData ) 
{
  if ( !this->Raised )
    {
    return;
    }
  if ( this->CustomLayoutNode == NULL )
    {
    vtkErrorMacro ("ProcessMRMLEvents has a NULL CustomLayoutNode");
    return;
    }


  // if parameter node has been changed externally, update GUI widgets with new values
  vtkMRMLCustomLayoutNode* node = vtkMRMLCustomLayoutNode::SafeDownCast(caller);
  vtkMRMLScene *scene = vtkMRMLScene::SafeDownCast ( caller );
}




//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::UpdateGUI ()
{
  if ( !this->Built )
    {
    return;
    }
  
  // update from MRML
  if ( this->UpdatingMRML )
    {
    return;
    }
  this->UpdatingGUI = 1;
  
}






//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::BuildGUI ( ) 
{
  
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( this->MRMLScene != NULL )
    {
    vtkMRMLCustomLayoutNode* node = vtkMRMLCustomLayoutNode::New();
    this->MRMLScene->RegisterNodeClass(node );
    node->Delete();
    }
  else
    {
    vtkErrorMacro("GUI is being built before MRML Scene is set");
    }

//  this->LoadTclPackage();
  this->Init();
  this->Built = true;

  this->UpdateGUI();
}


//---------------------------------------------------------------------------
void vtkCustomLayoutGUI::Init ( )
{
}
