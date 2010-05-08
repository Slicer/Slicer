
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerGUILayout.h"
#include "vtkSlicerTheme.h"

#include "vtkKWApplication.h"
#include "vtkKWWidget.h"
#include "vtkKWEvent.h"
#include "vtkKWEntry.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTopLevel.h"
#include "vtkKWTkUtilities.h"
#include "vtkKWSplitFrame.h"
#include "vtkKWText.h"
#include "vtkKWTextWithHyperlinksWithScrollbars.h"

#include "vtkSlicerWelcomeGUI.h"
#include "vtkSlicerWelcomeIcons.h"

#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <sstream>



//------------------------------------------------------------------------------
vtkCxxRevisionMacro ( vtkSlicerWelcomeGUI, "$Revision: 1.0 $");


//------------------------------------------------------------------------------
vtkSlicerWelcomeGUI* vtkSlicerWelcomeGUI::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret =
    vtkObjectFactory::CreateInstance("vtkSlicerWelcomeGUI");

  if(ret)
    {
      return (vtkSlicerWelcomeGUI*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkSlicerWelcomeGUI;
}



//----------------------------------------------------------------------------
vtkSlicerWelcomeGUI::vtkSlicerWelcomeGUI()
{
  this->Logic = NULL;
  this->WelcomeNode = NULL;
  
  this->SlicerWelcomeIcons = NULL;
  this->WelcomeFrame = NULL;
  this->OverviewFrame = NULL;
  this->ModulesFrame = NULL;
  this->LoadFrame = NULL;
  this->SaveFrame = NULL;
  this->DisplayFrame = NULL;
  this->ConfigureFrame = NULL;
  this->HintsFrame = NULL;
  this->CommunityFrame = NULL;
  this->CreditFrame = NULL;
  this->MouseModeFrame = NULL;
  this->ViewAndLayoutFrame = NULL;
  this->StartWithWelcome = NULL;
  this->Observed = 0;
}

//----------------------------------------------------------------------------
vtkSlicerWelcomeGUI::~vtkSlicerWelcomeGUI()
{
//    this->RemoveMRMLNodeObservers ( );
//    this->RemoveLogicObservers ( );
    
  if ( this->StartWithWelcome )
    {
    this->StartWithWelcome->SetParent ( NULL );
    this->StartWithWelcome->Delete();    
    this->StartWithWelcome = NULL;
    }
  if ( this->WelcomeFrame )
    {
    this->WelcomeFrame->SetParent ( NULL );
    this->WelcomeFrame->Delete();
    this->WelcomeFrame = NULL;
    }
  if ( this->OverviewFrame )
    {
    this->OverviewFrame->SetParent ( NULL );
    this->OverviewFrame->Delete();
    this->OverviewFrame = NULL;    
    }
  if ( this->LoadFrame )
    {
    this->LoadFrame->SetParent (NULL );
    this->LoadFrame->Delete();
    this->LoadFrame = NULL;    
    }
  if ( this->ModulesFrame )
    {
    this->ModulesFrame->SetParent (NULL );
    this->ModulesFrame->Delete();
    this->ModulesFrame = NULL;    
    }
  if ( this->SaveFrame )
    {
    this->SaveFrame->SetParent ( NULL );
    this->SaveFrame->Delete();
    this->SaveFrame = NULL;    
    }
  if ( this->DisplayFrame )  
    {
    this->DisplayFrame->SetParent ( NULL );
    this->DisplayFrame->Delete();
    this->DisplayFrame = NULL;
    }
  if ( this->ConfigureFrame )
    {
    this->ConfigureFrame->SetParent ( NULL );
    this->ConfigureFrame->Delete();
    this->ConfigureFrame = NULL;    
    }
  if ( this->HintsFrame )
    {
    this->HintsFrame->SetParent ( NULL );
    this->HintsFrame->Delete();
    this->HintsFrame = NULL;    
    }
  if ( this->CommunityFrame )
    {
    this->CommunityFrame->SetParent (NULL);
    this->CommunityFrame->Delete();
    this->CommunityFrame = NULL;    
    }
  if ( this->CreditFrame )  
    {
    this->CreditFrame->SetParent ( NULL);
    this->CreditFrame->Delete();
    this->CreditFrame = NULL;    
    }
  if ( this->MouseModeFrame )
    {
    this->MouseModeFrame->SetParent ( NULL );
    this->MouseModeFrame->Delete();
    this->MouseModeFrame = NULL;
    }
  if ( this->ViewAndLayoutFrame )
    {
    this->ViewAndLayoutFrame->SetParent (NULL);
    this->ViewAndLayoutFrame->Delete();
    this->ViewAndLayoutFrame = NULL;    
    }
  if ( this->SlicerWelcomeIcons )
    {
    this->SlicerWelcomeIcons->Delete();
    this->SlicerWelcomeIcons = NULL;
    }
  
  this->Observed = 0;
  if ( this->Logic )
    {
    this->Logic = NULL;
    }
  if ( this->WelcomeNode )
    {
    this->SetWelcomeNode ( NULL );
    }
}



//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::Enter()
{

  //--- get or create node
  vtkMRMLSlicerWelcomeNode *node = NULL;
  node = this->GetWelcomeNode();
  if ( node == NULL )
    {
    node = vtkMRMLSlicerWelcomeNode::New();
    this->SetWelcomeNode ( node );
    node->Delete();
    }

  if (this->GetWelcomeNode()  == NULL )
    {
    vtkErrorMacro ( "Enter: got NULL Welcome Node." );
    return;
    }
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( app== NULL )
    {
    vtkErrorMacro ( "Enter: NULL Application -- can not raise welcome module." );
    return;
    }
  vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
  if ( geom == NULL )
    {
    vtkErrorMacro ( "Enter: NULL geometry -- can not raise welcome module." );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  if ( ! appGUI )
    {
    vtkErrorMacro ( "Enter: NULL applicationGUI -- can not raise welcome module." );
    return;
    }

  if ( !appGUI->GetGUILayoutNode() )
    {
    vtkErrorMacro ( "Enter: NULL LayoutNode" );
    return;
    }
  if ( !appGUI->GetMainSlicerWindow() )
    {
    vtkErrorMacro ( "Enter: NULL Main Slicer Window.");
    return;
    }

  int w;
  //--- set the size of this GUI panel.
  if ( this->Visited == false )
    {
    //--- if first time visiting welcome module,
    //--- display GUI panel wider than default,
    //--- and save both widths in node.
    w = geom->GetDefaultGUIPanelWidth();
    this->GetWelcomeNode()->SetGUIWidth (w);
    w = (int)(floor(w * 1.75));
    this->GetWelcomeNode()->SetWelcomeGUIWidth (w);
    appGUI->GetGUILayoutNode()->SetMainPanelSize( w );
    appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->SetFrame1Size ( w );
    }
  else
    {
    //--- if user has visited before, then change
    //--- the GUI panel size to be what it was
    //--- last time user exited module.
    w = this->GetWelcomeNode()->GetWelcomeGUIWidth();
    appGUI->GetGUILayoutNode()->SetMainPanelSize( w );
    appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->SetFrame1Size ( w );
    }
  
  this->Visited = true;  
  if (this->Built == false )
    {
    this->BuildGUI();
    this->Built = true;

    //--- hook up gui
    this->AddGUIObservers();
    this->AddObserver ( vtkSlicerModuleGUI::ModuleSelectedEvent, (vtkCommand *)this->ApplicationGUI->GetGUICallbackCommand() );
    }
  else
    {
    if ( !this->Observed )
      {
      this->AddGUIObservers();
      }
    }
}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::Exit ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( app == NULL )
    {
    vtkErrorMacro ( "Exit: Got NULL application." );
    return;
    }
  vtkSlicerGUILayout *geom = app->GetDefaultGeometry ( );
  if ( geom == NULL )
    {
    vtkErrorMacro ( "Enter: NULL geometry -- can not raise welcome module." );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = this->GetApplicationGUI();
  if ( appGUI == NULL )
    {
    vtkErrorMacro ( "Exit: Got NULL application GUI." );
    return;
    }
  if ( appGUI->GetMainSlicerWindow() == NULL )
    {
    vtkErrorMacro ( "Exit: Got NULL application window.");
    return;  
    }
  if ( appGUI->GetGUILayoutNode() == NULL )
    {
    vtkErrorMacro ( "Exit: Got NULL layout node.");
    return;  
    }
  if ( this->GetWelcomeNode() == NULL )
    {
    vtkErrorMacro ( "Exit: Got NULL welcome node.");
    return;      
    }

  int w;
  
  //--- restore gui panel to default size, if bigger than usual.
  w = appGUI->GetGUILayoutNode()->GetMainPanelSize( );
  if ( w > geom->GetDefaultGUIPanelWidth() )
    {
    this->GetWelcomeNode()->SetWelcomeGUIWidth ( w );
    w = this->GetWelcomeNode()->GetGUIWidth();
    appGUI->GetGUILayoutNode()->SetMainPanelSize( w );
    appGUI->GetMainSlicerWindow()->GetMainSplitFrame()->SetFrame1Size ( w );
    }

  if ( this->Built )
    {
    this->RemoveGUIObservers();
    }
}


//----------------------------------------------------------------------------
vtkIntArray *vtkSlicerWelcomeGUI::NewObservableEvents()
{
 vtkIntArray *events = vtkIntArray::New();
  // Slicer3.cxx calls delete on events
  return events;
}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::TearDownGUI ( )
{

  this->RemoveGUIObservers ( );
  this->SetLogic ( NULL );
  this->SetAndObserveMRMLScene ( NULL );

}


//----------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::AddGUIObservers ( ) 
{
  if ( this->StartWithWelcome )
    {
    this->StartWithWelcome->GetWidget()->AddObserver (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->WelcomeFrame )
    {
    this->WelcomeFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->OverviewFrame )
    {
    this->OverviewFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->LoadFrame )
    {
    this->LoadFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ModulesFrame )
    {
    this->ModulesFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->SaveFrame )
    {
    this->SaveFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->DisplayFrame )
    {
    this->DisplayFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ConfigureFrame )
    {
    this->ConfigureFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->HintsFrame )
    {
    this->HintsFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->CommunityFrame )
    {
    this->CommunityFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->CreditFrame )
    {
    this->CreditFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->MouseModeFrame )
    {
    this->MouseModeFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ViewAndLayoutFrame )
    {
    this->ViewAndLayoutFrame->AddObserver (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  this->Observed = 1;
}



//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::RemoveGUIObservers ( )
{
  if ( this->StartWithWelcome )
    {
    this->StartWithWelcome->GetWidget()->RemoveObservers (vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->WelcomeFrame )
    {
    this->WelcomeFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->OverviewFrame )
    {
    this->OverviewFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if (this->LoadFrame )
    {
    this->LoadFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ModulesFrame )
    {
    this->ModulesFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->SaveFrame )
    {
    this->SaveFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->DisplayFrame )
    {
    this->DisplayFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ConfigureFrame )
    {
    this->ConfigureFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->HintsFrame )
    {
    this->HintsFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->CommunityFrame )
    {
    this->CommunityFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->CreditFrame )
    {
    this->CreditFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->MouseModeFrame )
    {
    this->MouseModeFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  if ( this->ViewAndLayoutFrame )
    {
    this->ViewAndLayoutFrame->RemoveObservers (vtkSlicerModuleCollapsibleFrame::FrameExpandEvent, (vtkCommand *)this->GUICallbackCommand );
    }
  this->Observed = 0;
}






//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::ProcessGUIEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *vtkNotUsed(callData)) 
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "ProcessGUIEvents: got Null SlicerApplication" );
    return;
    }

  vtkSlicerModuleCollapsibleFrame *f = vtkSlicerModuleCollapsibleFrame::SafeDownCast ( caller );
  vtkKWCheckButton *cb = vtkKWCheckButton::SafeDownCast ( caller );

  if ( cb != NULL && event == vtkKWCheckButton::SelectedStateChangedEvent )
    {
    if ( cb == this->StartWithWelcome->GetWidget() )
      {
      app->SetUseWelcomeModuleAtStartup ( this->StartWithWelcome->GetWidget()->GetSelectedState() ? 0 : 1 );
      }
    }
  
  //--- Brute force: if one frame is expanded, close the others.
  if ( f != NULL && event == vtkSlicerModuleCollapsibleFrame::FrameExpandEvent )
    {
    if ( f == this->WelcomeFrame )
      {
      this->OverviewFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->OverviewFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->ModulesFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->LoadFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->SaveFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->DisplayFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->ConfigureFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->HintsFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->CommunityFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->CreditFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->MouseModeFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->ViewAndLayoutFrame->CollapseFrame();
      }
    else if ( f == this->ViewAndLayoutFrame )
      {
      this->WelcomeFrame->CollapseFrame();
      this->ModulesFrame->CollapseFrame();
      this->OverviewFrame->CollapseFrame();
      this->LoadFrame->CollapseFrame();
      this->SaveFrame->CollapseFrame();
      this->DisplayFrame->CollapseFrame();
      this->ConfigureFrame->CollapseFrame();
      this->HintsFrame->CollapseFrame();
      this->CommunityFrame->CollapseFrame();
      this->CreditFrame->CollapseFrame();
      this->MouseModeFrame->CollapseFrame();
      }
    }
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::SetStatusText(const char *txt)
{
  if ( this->GetApplicationGUI() )
    {
    if ( this->GetApplicationGUI()->GetMainSlicerWindow() )
      {
      this->GetApplicationGUI()->GetMainSlicerWindow()->SetStatusText (txt);
      }
    }
}




//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildGUI ( ) 
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  if ( !app )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplication" );
    return;
    }
  vtkSlicerApplicationGUI *appGUI = app->GetApplicationGUI();
  if ( !appGUI )
    {
    vtkErrorMacro ( "BuildGUI: got Null SlicerApplicationGUI" );
    return;
    }
  vtkSlicerWindow *win = appGUI->GetMainSlicerWindow ();
  if ( win == NULL )
    {
    vtkErrorMacro ( "BuildGUI: got NULL MainSlicerWindow");
    return;
    }
  win->SetStatusText ( "Building Interface for Slicer Welcome Module...." );
  app->Script ( "update idletasks" );

  this->UIPanel->AddPage ( "SlicerWelcome", "SlicerWelcome", NULL );
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "SlicerWelcome" );    


  // create icons
  this->SlicerWelcomeIcons = vtkSlicerWelcomeIcons::New();

    //---
    // WELCOME & ABOUT FRAME
    //---
    this->WelcomeFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->WelcomeFrame->SetParent ( page );
    this->WelcomeFrame->Create ( );
    this->WelcomeFrame->SetLabelText ("Welcome & About");
    this->WelcomeFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->WelcomeFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildWelcomeAndAboutPanel( this->WelcomeFrame->GetFrame() );
    
    //---
    // OVERVIEW FRAME
    //---
    this->OverviewFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->OverviewFrame->SetParent ( page );
    this->OverviewFrame->Create ( );
    this->OverviewFrame->SetLabelText ("Overview");
    this->OverviewFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->OverviewFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildOverviewPanel( this->OverviewFrame->GetFrame() );
    
    //---
    // MODULES FRAME
    //---
    this->ModulesFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->ModulesFrame->SetParent ( page );
    this->ModulesFrame->Create ( );
    this->ModulesFrame->SetLabelText ("Basic & Extended Modules");
    this->ModulesFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->ModulesFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildModulesPanel( this->ModulesFrame->GetFrame() );

    //---
    // LOADING DATA FRAME
    //---
    this->LoadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->LoadFrame->SetParent ( page );
    this->LoadFrame->Create ( );
    this->LoadFrame->SetLabelText ("Loading Scenes & Data");
    this->LoadFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->LoadFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildLoadDataPanel( this->LoadFrame->GetFrame() );

    //---
    // SAVING DATA FRAME
    //---
    this->SaveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->SaveFrame->SetParent ( page );
    this->SaveFrame->Create ( );
    this->SaveFrame->SetLabelText ("Saving Scenes & Data");
    this->SaveFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->SaveFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildSaveDataPanel( this->SaveFrame->GetFrame() );

    //---
    // DISPLAY DATA FRAME
    //---
    this->DisplayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->DisplayFrame->SetParent ( page );
    this->DisplayFrame->Create ( );
    this->DisplayFrame->SetLabelText ("Adjusting Data Display");
    this->DisplayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->DisplayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildDataDisplayPanel (this->DisplayFrame->GetFrame() );
    
    //---
    // VIEW AND LAYOUT
    //---
    this->ViewAndLayoutFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->ViewAndLayoutFrame->SetParent ( page );
    this->ViewAndLayoutFrame->Create ( );
    this->ViewAndLayoutFrame->SetLabelText ("Configuring Viewers & Layout");
    this->ViewAndLayoutFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->ViewAndLayoutFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildViewAndLayoutPanel( this->ViewAndLayoutFrame->GetFrame() );
    
    //---
    // MOUSE MODES FRAME
    //---
    this->MouseModeFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->MouseModeFrame->SetParent ( page );
    this->MouseModeFrame->Create ( );
    this->MouseModeFrame->SetLabelText ("Mouse Modes");
    this->MouseModeFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->MouseModeFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildMouseModePanel( this->MouseModeFrame->GetFrame() );

    //---
    // ADD & REMOVE MODULES FRAME
    //---
    this->ConfigureFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->ConfigureFrame->SetParent ( page );
    this->ConfigureFrame->Create ( );
    this->ConfigureFrame->SetLabelText ("Customizing Slicer");
    this->ConfigureFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->ConfigureFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
     this->BuildConfigureModulesPanel( this->ConfigureFrame->GetFrame() );

    //---
    // COMMON ACTIVITIES FRAME
    //---
    this->HintsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->HintsFrame->SetParent ( page );
    this->HintsFrame->Create ( );
    this->HintsFrame->SetLabelText ("Other Useful Hints");
    this->HintsFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->HintsFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildOtherHelpPanel( this->HintsFrame->GetFrame() );

    //---
    // COMMUNITY FRAME
    //---
    this->CommunityFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->CommunityFrame->SetParent ( page );
    this->CommunityFrame->Create ( );
    this->CommunityFrame->SetLabelText ("Community");
    this->CommunityFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  this->CommunityFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("SlicerWelcome")->GetWidgetName());
    this->BuildCommunityPanel ( this->CommunityFrame->GetFrame() );
    
    //---
    // ACKNOWLEDGMENT FRAME
    //---
    this->CreditFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    this->CreditFrame->SetParent ( page );
    this->CreditFrame->Create ( );
    this->CreditFrame->SetLabelText ("Acknowledgment");
    this->CreditFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                  this->CreditFrame->GetWidgetName() );

    vtkKWFrame *f = vtkKWFrame::New();
    f->SetParent ( this->CreditFrame->GetFrame() );
    f->Create();
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                  f->GetWidgetName() );    

    vtkKWLabel *NACLabel = vtkKWLabel::New();
    NACLabel->SetParent ( f );
    NACLabel->Create();
    NACLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

    vtkKWLabel *NAMICLabel = vtkKWLabel::New();
    NAMICLabel->SetParent ( f );
    NAMICLabel->Create();
    NAMICLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );    

    vtkKWLabel *NCIGTLabel = vtkKWLabel::New();
    NCIGTLabel->SetParent ( f );
    NCIGTLabel->Create();
    NCIGTLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );
    
    vtkKWLabel *BIRNLabel = vtkKWLabel::New();
    BIRNLabel->SetParent ( f );
    BIRNLabel->Create();
    BIRNLabel->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );

    vtkKWLabel *CTSCLabel = vtkKWLabel::New();
    CTSCLabel->SetParent ( f );
    CTSCLabel->Create();
    CTSCLabel->SetImageToIcon (this->GetAcknowledgementIcons()->GetCTSCLogo() );

    vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
    txt->SetParent ( f ) ;
    txt->Create();
    txt->SetHorizontalScrollbarVisibility ( 0 );
    txt->SetVerticalScrollbarVisibility ( 1 );
    txt->GetWidget()->SetReliefToFlat();
    txt->GetWidget()->SetWrapToWord();
    txt->GetWidget()->QuickFormattingOn();
    txt->GetWidget()->SetHeight ( 4 );
    const char *ack = "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC and the Slicer Community. See <a>http://www.slicer.org</a> for details. We would also like to express our sincere thanks to members of the Slicer User Community who have helped us to design the contents of this Welcome Module, and whose feedback continues to improve functionality, usability and Slicer user experience\n\n.";    
    txt->SetText ( ack );
    txt->GetWidget()->ReadOnlyOn();

   app->Script ( "grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky e", NAMICLabel->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky e", NACLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky e",  BIRNLabel->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky e",  NCIGTLabel->GetWidgetName());                  
    app->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 2 -sticky w",  CTSCLabel->GetWidgetName());                  
    app->Script ( "grid %s -row 2 -column 0 -columnspan 3 -padx 2 -pady 2 -sticky ew",  txt->GetWidgetName());                  
    app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 0", f->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 1", f->GetWidgetName() );


    txt->Delete();
    NACLabel->Delete();
    NAMICLabel->Delete();
    NCIGTLabel->Delete();
    BIRNLabel->Delete();
    CTSCLabel->Delete();
    f->Delete();
    this->Built = true;
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildWelcomeAndAboutPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildWelcomeAndAboutPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                f->GetWidgetName() );
  vtkKWLabel *l = vtkKWLabel::New();
  l->SetParent ( f );
  l->Create();
  l->SetImageToIcon ( this->SlicerWelcomeIcons->GetLogoIcon() );
  app->Script ( "pack %s -side top -anchor c -fill x -padx 2 -pady 2",
                l->GetWidgetName() );
    
  vtkKWTextWithHyperlinksWithScrollbars *welcome = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  welcome->SetParent ( parent );
  welcome->Create();
  welcome->SetHorizontalScrollbarVisibility ( 0 );
  welcome->SetVerticalScrollbarVisibility ( 0);
  welcome->GetWidget()->SetReliefToFlat();
  welcome->GetWidget()->SetWrapToWord();
  welcome->GetWidget()->QuickFormattingOn();
  welcome->GetWidget()->SetHeight ( 5 );
  const char *about = "**3D Slicer** is a free open source software platform for medical image processing and 3D visualization of image data. This module contains some basic information and useful links to get you started using Slicer. Please see our website <a>http://www.slicer.org</a> and the documentation on our wiki for more information: <a>http://www.slicer.org/slicerWiki/index.php/Documentation-3.6</a>.";
  welcome->SetText ( about );
  //Important that Read only after SetText otherwise it doesn't work
  welcome->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *disclaimer = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  disclaimer->SetParent ( parent );
  disclaimer->Create();
  disclaimer->SetHorizontalScrollbarVisibility ( 0 );
  disclaimer->SetVerticalScrollbarVisibility ( 0);
  disclaimer->GetWidget()->SetReliefToFlat();
  disclaimer->GetWidget()->SetWrapToWord();
  disclaimer->GetWidget()->QuickFormattingOn();
  disclaimer->GetWidget()->SetForegroundColor ( 0.4, 0.4, 0.4 );
  disclaimer->GetWidget()->SetHeight ( 5 );
  const char *info = "**3D Slicer** is distributed under a BSD-style license; for details about the contribution and software license agreement, please see <a>http://www.slicer.org/cgi-bin/License/SlicerLicenseForm.pl</a>. The software has been designed for research purposes only and has not been reviewed or approved by the Food and Drug Administration, or by any other agency. ";
  disclaimer->SetText ( info );
  //Important that Read only after SetText otherwise it doesn't work
  disclaimer->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *hint = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  hint->SetParent ( parent );
  hint->Create();
  hint->SetHorizontalScrollbarVisibility ( 0 );
  hint->SetVerticalScrollbarVisibility ( 0);
  hint->GetWidget()->SetReliefToFlat();
  hint->GetWidget()->SetWrapToWord();
  hint->GetWidget()->QuickFormattingOn();
  hint->GetWidget()->SetHeight ( 3 );
  const char *hinttxt = "**Hint**: to open any information panel below, click on its grey title bar.";
  hint->SetText ( hinttxt );
  //Important that Read only after SetText otherwise it doesn't work
  hint->GetWidget()->ReadOnlyOn();

  this->StartWithWelcome = vtkKWCheckButtonWithLabel::New();
  this->StartWithWelcome->SetParent ( parent );
  this->StartWithWelcome->Create();
  this->StartWithWelcome->SetLabelPositionToRight ( );
  this->StartWithWelcome->GetLabel()->SetAnchorToCenter();
  this->StartWithWelcome->GetLabel()->SetText ( " Don't show this module on startup." );
  if ( app->GetUseWelcomeModuleAtStartup() )
    {
    this->StartWithWelcome->GetWidget()->SetSelectedState (0);
    }
  else
    {
    this->StartWithWelcome->GetWidget()->SetSelectedState (1);
    }

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 2",
                welcome->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 2",
                disclaimer->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 2",
                hint->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw  -padx 6 -pady 2",
                this->StartWithWelcome->GetWidgetName() );
  welcome->Delete();
  disclaimer->Delete();
  hint->Delete();
  l->Delete();
  f->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildOverviewPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildOverviewPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();
  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetMapIcon () );
  app->Script ( "pack %s -side top -anchor c -fill x -padx 2 -pady 2",
                l1->GetWidgetName() );
    
  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( parent );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 1 );
  txt->GetWidget()->SetHeight ( 28 );
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->QuickFormattingOn();
  txt->SetPadX ( 6 );
  txt->SetPadY ( 6 );
  const char *info = "\nThe basic organization of Slicer's user interface (UI) is shown above. This module's content will reference these following components, labeled in the figure:\n\n**File Menu:** contains basic load and save functionality, access to application settings, Tcl and Python interfaces for developers, help and mechanisms for users to provide feedback.\n\n**Toolbar:** contains tools for selecting and navigating among Slicer modules, layout and other utilities.\n\n**GUI Panel:** contains a UI for any selected module.\n\n**Slices Controls:** contains a UI to configure the Slice Viewers. This panel can be collapsed if you need more room for the GUI Panel.\n\n**3D View Controls:** contains a UI to configure the 3D Viewer. This panel can also be collapsed if you need more room for the GUI Panel.\n\n**3D Viewer:** displays a rendered 3D view of the scene along with some visual references to clarify its spatial orientation (**A** = anterior, **P** = posterior, **R** = right, **L** = left, **S** = superior and **I** = inferior).\n\n**Slice Viewers:** Three default slice viewers are provided (Red, Yellow and Green) in which  **Axial**, **Saggital**, **Coronal** or **Oblique** 2D slices of volume images can be displayed.\n\n**Progress & Error Log:** displays progress feedback for most processing operations. If an error occurs, the icon in the lower right corner provides access to the error log. (The information it contains is useful for providing feedback to the developer community).\n\n";
  txt->SetText ( info );
  //Important that Read only after SetText otherwise it doesn't work
  txt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4",
                f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 6 -pady 4",
                txt->GetWidgetName() );

  l1->Delete();
  txt->Delete();
  f->Delete();

}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildModulesPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildModulesPanel: got NULL application." );
    return;
    }
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetModuleIcon () );
  
  vtkKWTextWithHyperlinksWithScrollbars *basictxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  basictxt->SetParent ( f );
  basictxt->Create();
  basictxt->SetHorizontalScrollbarVisibility ( 0 );
  basictxt->SetVerticalScrollbarVisibility ( 1) ;
  basictxt->GetWidget()->SetReliefToGroove();
  basictxt->GetWidget()->SetWrapToWord();
  basictxt->GetWidget()->QuickFormattingOn();
  basictxt->GetWidget()->SetHeight (10);
  const char *t1 = "**Basic Modules:** You can navigate among Slicer's basic (core) modules using the toolbar icons (shown above) or by choosing them by name from the \"Module Menu\" (shown below and also found in the toolbar). The icons above, from left to right, navigate to the following basic modules: **Home**, **Data**, **Volumes**, **Models**, **Transforms**, **Fiducials**, **EditBox**, **Editor**, **Measurements**, and **Colors**.\n\nAny Slicer module can be set to your \"Home Module\" by typing **Ctrl-h** while that module is raised; you can subsequently visit your \"Home Module\" by clicking the **Home** icon, and that module will automatically be selected each time Slicer starts up. \n\n";
  basictxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work  
  basictxt->GetWidget()->ReadOnlyOn();
  
  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetModuleNavIcon () );
  
  vtkKWTextWithHyperlinksWithScrollbars *extendedtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  extendedtxt->SetParent ( f );
  extendedtxt->Create();
  extendedtxt->SetHorizontalScrollbarVisibility ( 0 );
  extendedtxt->SetVerticalScrollbarVisibility ( 1) ;
  extendedtxt->GetWidget()->SetReliefToGroove();
  extendedtxt->GetWidget()->SetWrapToWord();
  extendedtxt->GetWidget()->QuickFormattingOn();
  extendedtxt->GetWidget()->SetHeight (11);
const char *t2 = "The Module Navigation and Search toolbar (shown above) contains a **Modules Menu**, **Previous**, **Next**, and **History** icons, and the **Search** icon with text-box as convenient ways to move quickly among loaded modules. To customize the modules you'd like Slicer to include, select **View->ApplicationSettings->Module Settings** from Slicer's File Menu, select the **Load Modules** checkbox, and the **Select Modules** button.\n\n**Extended Modules:**  this toolbar also provides access, via the **Extensions** (gear) icon, to the Module Extensions Wizard.  This Wizard can be used to locate, and add/remove additional extension modules to/from Slicer. This Wizard requires a network connection. \n\n";
  extendedtxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work  
  extendedtxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4",
                f->GetWidgetName() );
  app->Script ( "pack %s %s %s %s -side top -anchor nw -fill x -expand y -padx 6 -pady 4",
                l1->GetWidgetName(),
                basictxt->GetWidgetName(),
                l2->GetWidgetName(),
                extendedtxt->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  basictxt->Delete();
  extendedtxt->Delete();
  f->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildLoadDataPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildLoadDataPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneIcon () );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetLoadIcon () );

  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( f );
  l3->Create();
  l3->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneTreeIcon () );
    
  vtkKWTextWithHyperlinksWithScrollbars *scenetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  scenetxt->SetParent ( f );
  scenetxt->Create();
  scenetxt->SetHorizontalScrollbarVisibility ( 0 );
  scenetxt->SetVerticalScrollbarVisibility ( 1) ;
  scenetxt->GetWidget()->SetReliefToGroove();
  scenetxt->GetWidget()->SetWrapToWord();
  scenetxt->GetWidget()->QuickFormattingOn();
  scenetxt->GetWidget()->SetHeight (14);
  const char *t1 = "**MRML:** The Medical Reality Markup Language (MRML) scene file is an XML-based text file that references all data loaded into Slicer, and records its display and processing parameterization. Loading a MRML scene file will also load its referenced data and display it in the manner it was saved. \n\n**File->Load Scene:** This load command selected from Slicer's File Menu will close the current scene, pop up a File Browser, and load data from the MRML scene file you select. \n\n**File->Import Scene:** This load command selected from Slicer's File Menu will pop up a File Browser, and add all data referenced in the MRML scene file you select into the current scene. \n\nSelect the **File->Load Volumes:** option to load individual volumes or to use the DICOM browser. **File->Add Data:** will raise a load panel with a set of options for loading individual datasets or entire directories containing data. **File->Add Transform:** provides an option to load a transformation matrix. \n\n";
  scenetxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work  
  scenetxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *loadtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  loadtxt->SetParent ( f );
  loadtxt->Create();
  loadtxt->SetHorizontalScrollbarVisibility ( 0 );
  loadtxt->SetVerticalScrollbarVisibility ( 1);
  loadtxt->GetWidget()->SetReliefToGroove();
  loadtxt->GetWidget()->SetWrapToWord();
  loadtxt->GetWidget()->QuickFormattingOn();
  loadtxt->GetWidget()->SetHeight ( 5 );
  const char *t2 = "**Load or Import Scenes or Data:** Clicking the **Load** icon (shown at the left) from Slicer's Toolbar exposes the options to Load or Import a MRML scene, or to raise the \"Load Data\" panel to select individual datasets or entire data directories.";
  loadtxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  loadtxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *treetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  treetxt->SetParent ( f );
  treetxt->Create();
  treetxt->SetHorizontalScrollbarVisibility ( 0 );
  treetxt->SetVerticalScrollbarVisibility ( 1);
  treetxt->GetWidget()->SetReliefToGroove();
  treetxt->GetWidget()->SetWrapToWord();
  treetxt->GetWidget()->QuickFormattingOn();
  treetxt->GetWidget()->SetHeight ( 5);
  const char *t3 = "**Load Any:** Find a convenient collection of options to load scenes and all supported data types in the \"Data Module's\", **Load & Add Scenes or Individual Datasets** Panel. The \"Data Module\" can be accessed using its icon (shown at the left) in Slicer's toolbar, or by using the **Modules Menu**. Once data has been added to the scene, it will be displayed in the \"Data Module's\" data tree. ";
  treetxt->SetText ( t3 );
  //Important that Read only after SetText otherwise it doesn't work
  treetxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 6", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky new -padx 2 -pady 4", scenetxt->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 0 -sticky nw -padx 4 -pady 6", l2->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky new -padx 2 -pady 4", loadtxt->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 0 -sticky nw -padx 4 -pady 6", l3->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 1 -sticky new -padx 2 -pady 4", treetxt->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  l3->Delete();
  scenetxt->Delete();
  loadtxt->Delete();
  treetxt->Delete();
  f->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildSaveDataPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildSaveDataPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneIcon () );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetSaveIcon () );

  vtkKWTextWithHyperlinksWithScrollbars *scenetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  scenetxt->SetParent ( f );
  scenetxt->Create();
  scenetxt->SetHorizontalScrollbarVisibility ( 0 );
  scenetxt->SetVerticalScrollbarVisibility ( 1) ;
  scenetxt->GetWidget()->SetReliefToGroove();
  scenetxt->GetWidget()->SetWrapToWord();
  scenetxt->GetWidget()->QuickFormattingOn();
  scenetxt->GetWidget()->SetHeight( 12 );
  const char *t1 = "**MRML:** The Medical Reality Markup Language (MRML) scene file is an XML-based text file that references all data loaded into Slicer, and records its display and processing parameterization. Saving a MRML scene file will NOT save its reference data automatically. If you wish to save another instance of data in the scene, you must do so explicitly. Two ways to access Slicer's save interface are described below. \n\n**File->Save:** This option will raise Slicer's Save interface. This panel gives you the option to save the MRML scene file and all unsaved data by default. You may optionally choose to rename the scene, to change the destination directory for any or all of the data to be written, and to save data that has not been modified during your session too. \n";
  scenetxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  scenetxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *savetxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  savetxt->SetParent ( f );
  savetxt->Create();
  savetxt->SetHorizontalScrollbarVisibility ( 0 );
  savetxt->SetVerticalScrollbarVisibility ( 1);
  savetxt->GetWidget()->SetReliefToGroove();
  savetxt->GetWidget()->SetWrapToWord();
  savetxt->GetWidget()->QuickFormattingOn();
  savetxt->GetWidget()->SetHeight ( 6 );
  const char *t2 = "**Save:** Clicking the icon shown at the left from Slicer's Toolbar raises the same interface described above. \n\n**IMPORTANT:** Once a MRML scene file is saved, deleting or moving any dataset that the scene references on disk will cause the scene description to become invalid. Loading a MRML scene that cannot locate its referenced data can cause unstable results. \n";
  savetxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  savetxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 6", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky new -padx 2 -pady 4", scenetxt->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 0 -sticky nw -padx 4 -pady 6", l2->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky new -padx 2 -pady 4", savetxt->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  scenetxt->Delete();
  savetxt->Delete();
  f->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildDataDisplayPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildDataDisplayPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetVolumesIcon () );

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetModelsIcon () );

  vtkKWLabel *l3 = vtkKWLabel::New();
  l3->SetParent ( f );
  l3->Create();
  l3->SetImageToIcon ( this->SlicerWelcomeIcons->GetFiducialsIcon () );
    
  vtkKWLabel *l4 = vtkKWLabel::New();
  l4->SetParent ( f );
  l4->Create();
  l4->SetImageToIcon ( this->SlicerWelcomeIcons->GetSceneSnapshotsIcon () );

  vtkKWTextWithHyperlinksWithScrollbars *voltxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  voltxt->SetParent ( f );
  voltxt->Create();
  voltxt->SetHorizontalScrollbarVisibility ( 0 );
  voltxt->SetVerticalScrollbarVisibility ( 1) ;
  voltxt->GetWidget()->SetReliefToGroove();
  voltxt->GetWidget()->SetWrapToWord();
  voltxt->GetWidget()->QuickFormattingOn();
  voltxt->GetWidget()->SetHeight(6);
  const char *t1 = "**Volumetric Image Data:** The interface for adjusting  Window, Level, Threshold, Color LUT and other parameters that control the display of volume image data in the scene is found in the \"Volumes Module\". To access this interface, either select the **Volumes** icon (shown at the left) from Slicer's Toolbar, or choose the module from the Toolbar's **Modules Menu**; then open the module's \"Display\" panel. ";
  voltxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  voltxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *modtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  modtxt->SetParent ( f );
  modtxt->Create();
  modtxt->SetHorizontalScrollbarVisibility ( 0 );
  modtxt->SetVerticalScrollbarVisibility ( 1 );
  modtxt->GetWidget()->SetReliefToGroove();
  modtxt->GetWidget()->SetWrapToWord();
  modtxt->GetWidget()->QuickFormattingOn();
  modtxt->GetWidget()->SetHeight(6);
  const char *t2 = "**Models:** Display options for geometric data like VTK models and FreeSurfer surfaces are found in the \"Models module\". To adjust parameters such as Color, Transparency, and Clipping, select the **Models** icon (shown at the left) from Slicer's Toolbar, or select the module from the Toolbar's **Modules Menu**; then open the module's \"Display\" panel.";
  modtxt->SetText ( t2 );
  //Important that Read only after SetText otherwise it doesn't work
  modtxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *fidtxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  fidtxt->SetParent ( f );
  fidtxt->Create();
  fidtxt->SetHorizontalScrollbarVisibility ( 0 );
  fidtxt->SetVerticalScrollbarVisibility ( 1);
  fidtxt->GetWidget()->SetReliefToGroove();
  fidtxt->GetWidget()->SetWrapToWord();
  fidtxt->GetWidget()->QuickFormattingOn();
  fidtxt->GetWidget()->SetHeight(6);
  const char *t3 = "**Fiducial Points:** Fiducials are useful tools employed in many Slicer analysis modules. Their visual representation and display can be adjusted in the \"Fiducials module\". Select the **Fiducials** icon (shown at the left) from Slicer's Toolbar, or select the module from the Toolbar's **Modules Menu**; then open the module's \"Display\" panel.";
  fidtxt->SetText ( t3 );
  //Important that Read only after SetText otherwise it doesn't work
  fidtxt->GetWidget()->ReadOnlyOn();

  vtkKWTextWithHyperlinksWithScrollbars *sstxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  sstxt->SetParent ( f );
  sstxt->Create();
  sstxt->SetHorizontalScrollbarVisibility ( 0 );
  sstxt->SetVerticalScrollbarVisibility ( 1);
  sstxt->GetWidget()->SetReliefToGroove();
  sstxt->GetWidget()->SetWrapToWord();
  sstxt->GetWidget()->QuickFormattingOn();
  sstxt->GetWidget()->SetHeight(6);
  const char *t4 = "**Scene Snapshots:** The icons shown at the left are found in the 3D View Controls panel. The top-most icon is used to capture a \"snapshot\" of the scene configuration and save it with a unique name. That \"snapshot\" can be restored at a later time by selecting the **Scene Snapshot Menubutton**, shown at the lower left. Scene Snapshots will be saved with your scene.";
  sstxt->SetText ( t4 );
  //Important that Read only after SetText otherwise it doesn't work
  sstxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 0 -sticky nw -padx 2 -pady 4", l1->GetWidgetName() );
  app->Script ( "grid %s -row 0 -column 1 -sticky new -padx 2 -pady 4", voltxt->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 0 -sticky nw -padx 4 -pady 4", l2->GetWidgetName() );
  app->Script ( "grid %s -row 1 -column 1 -sticky new -padx 2 -pady 4", modtxt->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 0 -sticky nw -padx 4 -pady 4", l3->GetWidgetName() );
  app->Script ( "grid %s -row 2 -column 1 -sticky new -padx 2 -pady 4", fidtxt->GetWidgetName() );
  app->Script ( "grid %s -row 3 -column 0 -sticky nw -padx 4 -pady 4", l4->GetWidgetName() );
  app->Script ( "grid %s -row 3 -column 1 -sticky new -padx 2 -pady 4", sstxt->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 0 -weight 0", f->GetWidgetName() );
  app->Script ( "grid columnconfigure %s 1 -weight 1", f->GetWidgetName() );

  l1->Delete();
  l2->Delete();
  l3->Delete();
  l4->Delete();

  voltxt->Delete();
  modtxt->Delete();
  fidtxt->Delete();
  sstxt->Delete();

  f->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildViewAndLayoutPanel ( vtkKWFrame *parent )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildDataDisplayPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l0 = vtkKWLabel::New();
  l0->SetParent ( f );
  l0->Create();
  l0->SetImageToIcon ( this->SlicerWelcomeIcons->GetLayoutIcon () );

  vtkKWTextWithHyperlinksWithScrollbars *ltxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  ltxt->SetParent ( f );
  ltxt->Create();
  ltxt->SetHorizontalScrollbarVisibility ( 0 );
  ltxt->SetVerticalScrollbarVisibility ( 1);
  ltxt->GetWidget()->SetReliefToGroove();
  ltxt->GetWidget()->SetWrapToWord();
  ltxt->GetWidget()->QuickFormattingOn();
  ltxt->GetWidget()->SetHeight(5);
  const char *t4 = "**Application Layout:** The Toolbar's **Layout** icon, shown above at the left-most, exposes a menu of application layout options. Selecting any of the options (also shown above) will adjust the arrangement and visibility of the 3D Viewer, Slice Viewers, and the GUI Panel.";
  ltxt->SetText ( t4 );
  //Important that Read only after SetText otherwise it doesn't work
  ltxt->GetWidget()->ReadOnlyOn();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetSlicesControlIcon () );

  vtkKWTextWithHyperlinksWithScrollbars *sctxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  sctxt->SetParent ( f );
  sctxt->Create();
  sctxt->SetHorizontalScrollbarVisibility ( 0 );
  sctxt->SetVerticalScrollbarVisibility ( 1);
  sctxt->GetWidget()->SetReliefToGroove();
  sctxt->GetWidget()->SetWrapToWord();
  sctxt->GetWidget()->QuickFormattingOn();
  sctxt->GetWidget()->SetHeight(10);
  const char *t5 = "**Slices Controls:** Each Slice Viewer has 3 layers: Foreground, Background, and a Label Layer.  The interface shown above, found in the \"Manipulate Slice Views\" panel, can be used to adjust the view of one or more of these layers in all Slice Viewers simultaneously. From left to right, the icons expose options for: **Visibility**, **Data Centering**, **Label Map Opacity**, **Annotations**, **Layer Compositing**, **Crosshairs**, **Axis Labels**, and **Field of view**.Most of these -- and additional -- features can be found in each individual Slice Viewer's user interface (where they can be applied to that viewer exclusively). The **Fade Slider** and its surrounding icons shown above right, provide the ability to view only the Foreground or Background layer, or to fade between them; this feature is especially useful for comparing two datasets.";
  sctxt->SetText ( t5 );
  //Important that Read only after SetText otherwise it doesn't work
  sctxt->GetWidget()->ReadOnlyOn();

  vtkKWLabel *l2 = vtkKWLabel::New();
  l2->SetParent ( f );
  l2->Create();
  l2->SetImageToIcon ( this->SlicerWelcomeIcons->GetViewControlIcon () );
  
  vtkKWTextWithHyperlinksWithScrollbars *vctxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  vctxt->SetParent ( f );
  vctxt->Create();
  vctxt->SetHorizontalScrollbarVisibility ( 0 );
  vctxt->SetVerticalScrollbarVisibility ( 1);
  vctxt->GetWidget()->SetReliefToGroove();
  vctxt->GetWidget()->SetWrapToWord();
  vctxt->GetWidget()->QuickFormattingOn();
  vctxt->GetWidget()->SetHeight(8);
  const char *t6 = "**3D View Controls:** The interface shown above, found in the \"Manipulate 3D View\" panel, can be used to modify the 3D view. Clicking on any axis labels in the left-most panel changes the viewer's **Point of View**. The collection of icons in the center controls options such as **Pitch**, **Roll**, and **Yaw**; **View Centering**, orthographic or perspective **Projection**, **Visibility & Display** parameters, **Screen Snapshot** and **Scene Snapshot** capabilities, **Zoom** and **Stereo Viewing** options. The right-most checkboxes toggle view **Spin** and **Rock** animations for added enhancement of depth and layout. \n\nFinally, the small window on the right displays different information depending on the location of your mouse: when the mouse is over a Slice Viewer, the window provides a **magnified view** of the area surrounding the mouse pointer; otherwise, the window displays a **bird's-eye view** of the entire scene.";
  vctxt->SetText ( t6 );
  //Important that Read only after SetText otherwise it doesn't work
  vctxt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "pack %s %s %s %s %s %s -side top -anchor c  -fill x -expand y -padx 6 -pady 4",
                l0->GetWidgetName(),
                ltxt->GetWidgetName(),
                l1->GetWidgetName(),
                sctxt->GetWidgetName(),
                l2->GetWidgetName(),
                vctxt->GetWidgetName() );


  l0->Delete();
  l1->Delete();
  l2->Delete();
  ltxt->Delete();
  sctxt->Delete();
  vctxt->Delete();
  f->Delete();
}


//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildMouseModePanel ( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildMouseModePanel: got NULL application." );
    return;
    }
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent (parent);
  f->Create();

  vtkKWLabel *l1 = vtkKWLabel::New();
  l1->SetParent ( f );
  l1->Create();
  l1->SetImageToIcon ( this->SlicerWelcomeIcons->GetMouseModeIcon () );
  
  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( f );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 1) ;
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->QuickFormattingOn();
  txt->GetWidget()->SetHeight (7);
  const char *t1 = "**Mouse Modes:** Slicer has three main mouse modes that apply to the 3D viewer:  **Pick** (which allows \"pickable\" objects to be selected and manipulated), **Place** (which permits objects to be interactively created and placed), and **Transform** (rotate, pan and zoom). The toolbar icons that switch between these mouse modes are shown from left to right above, respectively. \n\n**Hint:** Transform mode is the default mouse mode. Selecting the Pick or Place icon changes the mode for **one mouse-click only**, and then the mode reverts back to \"Transform\". The \"option arrows\" beside each icon provide a way  to \"pick\" or \"place\" persistently, until another mode is manually selected. \n\n";
  txt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work  
  txt->GetWidget()->ReadOnlyOn();
  
  app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4",
                f->GetWidgetName() );
  app->Script ( "pack %s %s -side top -anchor nw -fill x -expand y -padx 6 -pady 4",
                l1->GetWidgetName(),
                txt->GetWidgetName() );

  l1->Delete();
  txt->Delete();
  f->Delete();
}



//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildOtherHelpPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildOtherHelpPanel: got NULL application." );
    return;
    }

  vtkKWTextWithHyperlinksWithScrollbars *basictxt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  basictxt->SetParent ( parent );
  basictxt->Create();
  basictxt->SetHorizontalScrollbarVisibility ( 0 );
  basictxt->SetVerticalScrollbarVisibility ( 1);
  basictxt->GetWidget()->SetReliefToGroove();
  basictxt->GetWidget()->SetWrapToWord();
  basictxt->GetWidget()->QuickFormattingOn();
  basictxt->GetWidget()->SetHeight(10);
  const char *t1 = " **Help For Performing Common Tasks:**\n\nFor general application and specific module documentation, see:\n  <a>http://www.slicer.org/slicerWiki/index.php/Documentation-3.6</a>.\n\nFor information on merging multiple datasets (and multiple modalities) for a single subject, see:\n  <a>http://www.slicer.org/slicerWiki/index.php/Documentation-3.6#Registration</a>. \n\nFor information on how basic visualization operations are performed, or information about manual segmentation of image data, see **Basic** and **Core** tutorials on our training page:\n  <a>http://www.slicer.org/slicerWiki/index.php/Slicer3.4:Training</a>. \n\n**Information About Specialized Tools:** \n\nFind tutorials for advanced features, like:\n * automated brain segmentation using the \"EMSegmenter module\",\n * Diffusion MRI processing, \n * Tool Tracking and \n * Image Guided Therapy Research \nin **Specialized** tutorials on the training page:\n  <a>http://www.slicer.org/slicerWiki/index.php/Slicer3.4:Training</a>. \n";

  basictxt->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  basictxt->GetWidget()->ReadOnlyOn();
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", basictxt->GetWidgetName() );
  basictxt->Delete();

}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildConfigureModulesPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildConfigureModulesPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();
  
  vtkKWLabel *l = vtkKWLabel::New();
  l->SetParent ( f );
  l->Create();
  l->SetImageToIcon ( this->SlicerWelcomeIcons->GetModuleConfigureIcon() );

  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( f );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 1 );
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->SetHeight ( 8 );
  txt->GetWidget()->QuickFormattingOn();
  const char *t = "**Configuring your Slicer build:** Slicer can be customized using the **Application Settings Interface**. Open this utility by selecting **View->Application Settings** from the File Menu, or by using the **F2** hot-key. Any options selected in this panel will be saved in the Application Registry and restored each time you start Slicer. Many useful options are provided within the interface's sub-panels: font family and size can be adjusted using the **Font Settings** panel, the **Slicer Settings** panel allows you to specify paths to web browsers and other useful applications, and the **Remote Data Handling Settings** panel allows you to select data caching options. \n"; 
  txt->SetText ( t );
  //Important that Read only after SetText otherwise it doesn't work
  txt->GetWidget()->ReadOnlyOn();
  
  vtkKWTextWithHyperlinksWithScrollbars *txt2 = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt2->SetParent ( f );
  txt2->Create();
  txt2->SetHorizontalScrollbarVisibility ( 0 );
  txt2->SetVerticalScrollbarVisibility ( 1 );
  txt2->GetWidget()->SetReliefToGroove();
  txt2->GetWidget()->SetWrapToWord();
  txt2->GetWidget()->SetHeight ( 10 );
  txt2->GetWidget()->QuickFormattingOn();
  const char *t1 = "**Adding and Removing Modules:** Slicer has two basic types of modules, **Loadable Modules** and **Command Line Plugins**. By default, Slicer starts up with all modules included. You can customize Slicer to include only the Loadable and Command Line modules you need; which keeps Slicer as compact as possible and accelerates application startup time. To customize Slicer this way, select the Application Settings Interface's **Module Settings** panel. In this panel:\n\n* Deselecting the **Load Modules** checkbutton allows you to supress all Loadable Modules.\n\n* Selecting the **Load Modules** checkbutton will cause all modules selected in the **Select Modules** panel (shown above) to be loaded next time you start up Slicer.\n\n* Deselecting the **Load Command-Line Plugins** checkbox will surpress all Command-Line modules, and\n\n* Selecting that checkbutton will include all Command-Line modules next time you start up.\n";

  txt2->SetText ( t1 );
  //Important that Read only after SetText otherwise it doesn't work
  txt2->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "pack %s %s %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4",
                txt->GetWidgetName(),
                l->GetWidgetName(),
                txt2->GetWidgetName() );
  txt->Delete();
  l->Delete();
  txt2->Delete();
  f->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::BuildCommunityPanel( vtkKWFrame *parent )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication());
  if ( !app )
    {
    vtkErrorMacro ( "BuildCommunityPanel: got NULL application." );
    return;
    }

  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();
  
  vtkKWLabel *l = vtkKWLabel::New();
  l->SetParent ( f );
  l->Create();
  l->SetImageToIcon ( this->SlicerWelcomeIcons->GetCommunityIcon() );
  
  vtkKWTextWithHyperlinksWithScrollbars *txt = vtkKWTextWithHyperlinksWithScrollbars::New ( );
  txt->SetParent ( f );
  txt->Create();
  txt->SetHorizontalScrollbarVisibility ( 0 );
  txt->SetVerticalScrollbarVisibility ( 1 );
  txt->GetWidget()->SetReliefToGroove();
  txt->GetWidget()->SetWrapToWord();
  txt->GetWidget()->QuickFormattingOn();
  txt->GetWidget()->SetHeight( 12 );
  const char *t = "We welcome you to participate in or engage with the Slicer user and developer community. Some useful links are listed below:\n\n**Subscribe to User or Developer Mailing Lists:**\n<a>http://www.slicer.org/pages/Mailinglist</a>\n\n**Report Bugs and Request Features:**\n<a>http://www.na-mic.org/Bug/main_page.php</a>\n\n**Track upcoming events and join a workshop:**\n<a>http://www.na-mic.org/Wiki/index.php/Events</a>\n\n";
  txt->SetText ( t );
  //Important that Read only after SetText otherwise it doesn't work
  txt->GetWidget()->ReadOnlyOn();

  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", f->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor c -fill x -expand y -padx 2 -pady 4", l->GetWidgetName() );
  app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 4", txt->GetWidgetName() );
  
  l->Delete();
  txt->Delete();
  f->Delete();
}

//---------------------------------------------------------------------------
void vtkSlicerWelcomeGUI::Init ( )
{
}
