#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleChooseGUI.h"
#include "vtkSlicerModuleNavigationIcons.h"

#include "vtkKWWidget.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"



//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModuleChooseGUI );
vtkCxxRevisionMacro ( vtkSlicerModuleChooseGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkSlicerModuleChooseGUI::vtkSlicerModuleChooseGUI ( )
{
      //--- ui for the ModuleChooseFrame,
    this->ModulesMenuButton = vtkKWMenuButton::New();
    this->ModulesLabel = vtkKWLabel::New();
    this->ModulesPrev = vtkKWPushButton::New ( );
    this->ModulesNext = vtkKWPushButton::New ( );
    this->ModulesHistory = vtkKWPushButton::New ( );
    this->ModulesRefresh = vtkKWPushButton::New ( );
    this->SlicerModuleNavigationIcons = vtkSlicerModuleNavigationIcons::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerModuleChooseGUI::~vtkSlicerModuleChooseGUI ( )
{

      if ( this->SlicerModuleNavigationIcons ) {
        this->SlicerModuleNavigationIcons->Delete ( );
        this->SlicerModuleNavigationIcons = NULL;
    }
      if ( this->ModulesMenuButton ) {
      this->ModulesMenuButton->SetParent ( NULL );
        this->ModulesMenuButton->Delete();
        this->ModulesMenuButton = NULL;
    }
    if ( this->ModulesLabel ) {
      this->ModulesLabel->SetParent ( NULL );
        this->ModulesLabel->Delete ( );
        this->ModulesLabel = NULL;
    }
    if ( this->ModulesPrev ) {
      this->ModulesPrev->SetParent ( NULL );
        this->ModulesPrev->Delete ( );
        this->ModulesPrev = NULL;
    }
    if ( this->ModulesNext ) {
      this->ModulesNext->SetParent ( NULL );
        this->ModulesNext->Delete ( );
        this->ModulesNext = NULL;
    }
    if ( this->ModulesHistory) {
      this->ModulesHistory->SetParent ( NULL );
        this->ModulesHistory->Delete ( );
        this->ModulesHistory = NULL;
    }
    if ( this->ModulesRefresh) {
      this->ModulesRefresh->SetParent ( NULL );
        this->ModulesRefresh->Delete ( );
        this->ModulesRefresh = NULL;
    }
    this->SetApplicationGUI ( NULL );
}





//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );
    os << indent << "SlicerModuleChooseGUI: " << this->GetClassName ( ) << "\n";

}



//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::RemoveGUIObservers ( )
{
  // FILL IN
 this->ModulesMenuButton->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::AddGUIObservers ( )
{
  // add observer onto the menubutton in the ModuleChoose GUI Panel
  this->ModulesMenuButton->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast (caller );

  if ( menu == this->ModulesMenuButton->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
    {
    this->SelectModule(this->ModulesMenuButton->GetValue());
    }
}

 
//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::SelectModule ( const char *moduleName )
{
  if ( this->GetApplicationGUI() != NULL )
    {
    vtkSlicerApplicationGUI *p = vtkSlicerApplicationGUI::SafeDownCast( this->GetApplicationGUI ( ));
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );

    if ( app != NULL && app->GetModuleGUICollection ( ) != NULL )
      {
      vtkSlicerModuleGUI * m;
      const char *mName;

      app->GetModuleGUICollection( )->InitTraversal( );
      m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
      while (m != NULL )
        {
          mName = m->GetUIPanel()->GetName();
          if ( !strcmp (moduleName, mName) ) 
           {
            m->GetUIPanel()->Raise();
            //this->RemoveGUIObservers();
            //this->ModulesMenuButton->GetMenu()->SelectItem(moduleName);
            //this->AddGUIObservers();
            break;
           }
          m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
        } // end while
      } // end if ( app != NULL
    }
}

//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::ProcessLogicEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::Enter ( )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::Exit ( )
{
    // Fill in
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI )
{
  this->ApplicationGUI = appGUI;
}





//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::BuildGUI ( vtkKWFrame *appF )
{

  vtkSlicerApplicationGUI *p = this->GetApplicationGUI ( );
  //--- Populate the Slice Control Frame
  if ( p != NULL )
    {
    if ( p->GetApplication() != NULL )
      {
      vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( p->GetApplication() );

      //--- ALL modules menu button label
      this->ModulesLabel->SetParent ( appF );
      this->ModulesLabel->Create ( );

      this->ModulesLabel->SetText ( "Modules:");
      this->ModulesLabel->SetAnchorToWest ( );
      this->ModulesLabel->SetWidth ( 7 );

      //--- All modules menu button
      this->ModulesMenuButton->SetParent ( appF );
      this->ModulesMenuButton->Create ( );
      this->ModulesMenuButton->SetWidth ( 28 );
      this->ModulesMenuButton->IndicatorVisibilityOn ( );
      this->ModulesMenuButton->SetBalloonHelpString ("Select a Slicer module.");

      //--- Next and previous module button
      this->ModulesNext->SetParent ( appF );
      this->ModulesNext->Create ( );
      this->ModulesNext->SetBorderWidth ( 0 );
      this->ModulesNext->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleNextIcon() );
      this->ModulesNext->SetBalloonHelpString ("Navigate to the next module in your use history.");

      this->ModulesPrev->SetParent ( appF );
      this->ModulesPrev->Create ( );
      this->ModulesPrev->SetBorderWidth ( 0 );
      this->ModulesPrev->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModulePrevIcon() );
      this->ModulesPrev->SetBalloonHelpString ("Navigate to the previous module in your use history.");
        
      this->ModulesHistory->SetParent ( appF );
      this->ModulesHistory->Create ( );
      this->ModulesHistory->SetBorderWidth ( 0 );
      this->ModulesHistory->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleHistoryIcon() );
      this->ModulesHistory->SetBalloonHelpString ("Pop up a window showing your module use history.");

      this->ModulesRefresh->SetParent ( appF );
      this->ModulesRefresh->Create ( );
      this->ModulesRefresh->SetBorderWidth ( 0 );
      this->ModulesRefresh->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleRefreshIcon() );
      this->ModulesRefresh->SetBalloonHelpString ("Refresh the list of available modules.");
        
      //--- pack everything up.
      app->Script ( "pack %s -side left -anchor n -padx 1 -ipadx 1 -pady 3", this->ModulesLabel->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor n -padx 1 -ipady 0 -pady 2", this->ModulesMenuButton->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesPrev->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesNext->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesHistory->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesRefresh->GetWidgetName( ) );
    }
  }

}





