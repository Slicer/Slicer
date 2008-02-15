#include <map>
#include <set>
#include <vector>

#include <ctype.h>

#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkSlicerApplication.h"
#include "vtkSlicerApplicationGUI.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleChooseGUI.h"
#include "vtkSlicerModuleNavigationIcons.h"
#include "vtkSlicerModuleNavigator.h"
#include "vtkSlicerGUICollection.h"

#include "vtkKWWidget.h"
#include "vtkKWPushButton.h"
#include "vtkKWLabel.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWEntry.h"
#include "vtkKWFrame.h"
#include "vtkKWToolbar.h"

#define convertToUpper( s ) { while (*s) {*s = toupper(*s); s++; } }

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkSlicerModuleChooseGUI );
vtkCxxRevisionMacro ( vtkSlicerModuleChooseGUI, "$Revision: 1.0 $");


void
splitString (const std::string &text,
             const std::string &separators,
             std::vector<std::string> &words)
{
  int n = text.length();
  int start, stop;
  start = text.find_first_not_of(separators);
  while ((start >= 0) && (start < n))
    {
    stop = text.find_first_of(separators, start);
    if ((stop < 0) || (stop > n)) stop = n;
    words.push_back(text.substr(start, stop - start));
    start = text.find_first_not_of(separators, stop+1);
    }
}


//---------------------------------------------------------------------------
vtkSlicerModuleChooseGUI::vtkSlicerModuleChooseGUI ( )
{
      //--- ui for the ModuleChooseFrame,
    this->ModulesMenuButton = vtkKWMenuButton::New();
    this->ModulesLabel = vtkKWLabel::New();
    this->ModulesPrev = vtkKWPushButton::New ( );
    this->ModulesNext = vtkKWPushButton::New ( );
    this->ModulesHistory = vtkKWMenuButton::New ( );
    this->ModulesRefresh = vtkKWPushButton::New ( );
    this->ModulesSearch = vtkKWMenuButton::New ( );
    this->SlicerModuleNavigationIcons = vtkSlicerModuleNavigationIcons::New ( );
    this->ModulesSearchEntry = vtkKWEntry::New ( );
    this->ModuleNavigationFrame = vtkKWFrame::New ( );
    this->ModuleNavigator = vtkSlicerModuleNavigator::New ( );
}


//---------------------------------------------------------------------------
vtkSlicerModuleChooseGUI::~vtkSlicerModuleChooseGUI ( )
{

  if ( this->ModuleNavigator )
    {
    this->ModuleNavigator->Delete ( );
    this->ModuleNavigator = NULL;
    }
  if ( this->ModuleNavigationFrame )
    {
    this->ModuleNavigationFrame->SetParent ( NULL );
    this->ModuleNavigationFrame->Delete ( );
    this->ModuleNavigationFrame = NULL;
    }
  if ( this->ModulesSearchEntry )
    {
    this->ModulesSearchEntry->SetParent ( NULL );
    this->ModulesSearchEntry->Delete( );
    this->ModulesSearchEntry = NULL;
    }
  if ( this->ModulesMenuButton )
    {
    this->ModulesMenuButton->SetParent ( NULL );
    this->ModulesMenuButton->Delete();
    this->ModulesMenuButton = NULL;
    }
  if ( this->ModulesLabel )
    {
    this->ModulesLabel->SetParent ( NULL );
    this->ModulesLabel->Delete ( );
    this->ModulesLabel = NULL;
    }
  if ( this->ModulesPrev )
    {
    this->ModulesPrev->SetParent ( NULL );
    this->ModulesPrev->Delete ( );
    this->ModulesPrev = NULL;
    }
  if ( this->ModulesNext )
    {
    this->ModulesNext->SetParent ( NULL );
    this->ModulesNext->Delete ( );
    this->ModulesNext = NULL;
    }
  if ( this->ModulesHistory)
    {
    this->ModulesHistory->SetParent ( NULL );
    this->ModulesHistory->Delete ( );
    this->ModulesHistory = NULL;
    }
  if ( this->ModulesRefresh)
    {
    this->ModulesRefresh->SetParent ( NULL );
    this->ModulesRefresh->Delete ( );
    this->ModulesRefresh = NULL;
    }
  if ( this->ModulesSearch)
    {
    this->ModulesSearch->SetParent ( NULL );
    this->ModulesSearch->Delete ( );
    this->ModulesSearch = NULL;
    }
  if ( this->SlicerModuleNavigationIcons )
    {
    this->SlicerModuleNavigationIcons->Delete ( );
    this->SlicerModuleNavigationIcons = NULL;
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
    this->ModulesPrev->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesNext->RemoveObservers (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesHistory->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesSearch->GetMenu()->RemoveObservers (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesSearchEntry->RemoveObservers (vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::AddGUIObservers ( )
{
    this->ModulesPrev->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesNext->AddObserver (vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesHistory->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesSearch->GetMenu()->AddObserver (vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
    this->ModulesSearchEntry->AddObserver ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{

  char *moduleName;
  
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWPushButton *pushb = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMenu *menu = vtkKWMenu::SafeDownCast ( caller );
  vtkKWEntry *entry = vtkKWEntry::SafeDownCast ( caller );
  
  if ( pushb == this->ModulesPrev && event == vtkKWPushButton::InvokedEvent )
    {

    // Exit current module
    char * currentModuleName = this->GetModuleNavigator()->GetCurrentModuleName();
    if ( currentModuleName != NULL )
      {
      vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName( currentModuleName );
      if (currentModule != NULL )
        {
        currentModule->Exit ( );
        }
      }

    // Move current module backward in navigation list
    moduleName = this->GetModuleNavigator()->NavigateBack();

    // Enter selected module.
    currentModuleName = this->GetModuleNavigator()->GetCurrentModuleName();
    if ( currentModuleName != NULL )
      {
      vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName( currentModuleName );        
      if ( currentModule )
        {
        currentModule->Enter ( );
        this->RaiseModule ( moduleName );
        this->GetModuleNavigator()->AddModuleNameToHistoryList ( moduleName );
        this->PopulateHistoryListMenu ( );
        this->GetModuleNavigator()->AddModuleNameToNavigationList ( moduleName );
        }
      }
//    this->RaiseModule ( moduleName );
    }
  if ( pushb == this->ModulesNext && event == vtkKWPushButton::InvokedEvent )
    {

    // Exit current module
    char * currentModuleName = this->GetModuleNavigator()->GetCurrentModuleName();
    if ( currentModuleName != NULL )
      {
      vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName( currentModuleName );
      if (currentModule != NULL )
        {
        currentModule->Exit ( );
        }
      }

    // move current module forward in navigation list
    moduleName = this->GetModuleNavigator()->NavigateForward ();

    // Enter selected module.
    currentModuleName = this->GetModuleNavigator()->GetCurrentModuleName();
    if ( currentModuleName != NULL )
      {
      vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName( currentModuleName );        
      if ( currentModule )
        {
        currentModule->Enter ( );
        this->RaiseModule ( moduleName );
        this->GetModuleNavigator()->AddModuleNameToHistoryList ( moduleName );
        this->PopulateHistoryListMenu ( );
        this->GetModuleNavigator()->AddModuleNameToNavigationList ( moduleName );
        }
      }

//    this->RaiseModule ( moduleName );
    }
  if ( menu == this->ModulesHistory->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
    {
    // First, get the text (modulename) of the selected module.
    const char *c = this->ModulesHistory->GetValue ( );
    // Now, add it to the navigation list as though module were selected
    // straight from the modules choose menubutton.
    // the little checkbox in this pulldown menu should really track the current module.
    // but don't want to trigger extra events by setting it. ignore for now.
    this->SelectModule ( c );
    this->GetModuleNavigator()->AddModuleNameToNavigationList ( c );
//    this->RaiseModule ( c );
    }
  if ( entry == this->ModulesSearchEntry && event == vtkKWEntry::EntryValueChangedEvent )
    {
    // user types search term, when focus leaves entry widget,
    // execute the search and populate the "search" menu button with results.
    // user sees results when they click 'search'. A little weird. try for now and see
    // whether it serves the purpose or needs to change.
    // populate the module search menu with results from the search.
    const char *c = this->ModulesSearchEntry->GetValue ( );
    this->PopulateModuleSearchMenu ( c );
    }
  if ( menu == this->ModulesSearch->GetMenu() && event == vtkKWMenu::MenuItemInvokedEvent )
    {
    // select module chosen from the search menu
    const char *c = this->ModulesSearch->GetValue ( );
    this->SelectModule ( c );
    }
}

 

//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::RaiseModule ( const char *moduleName )
{
  if ( this->GetApplicationGUI() != NULL && moduleName != NULL )
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
            p->GetMainSlicerWindow()->SetStatusText ( mName );
            this->GetModulesMenuButton()->SetValue( mName );
            break;
           }
          m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ) );
        } // end while      
      } // end if ( app != NULL
    }
}





//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::PopulateHistoryListMenu  ( )
{

  if ( this->ModulesHistory != NULL )
    {
    // Delete the existing menu
    this->ModulesHistory->GetMenu()->DeleteAllItems ( );
    struct ModuleNameEntry *n = this->GetModuleNavigator()->GetModuleHistoryList();
    // and reconstruct it with current module history.
    while ( n != NULL )
      {
      this->ModulesHistory->GetMenu()->AddRadioButton ( n->ModuleName );
      n = n->Next;
      }
    }
}



//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::SelectModule ( const char *moduleName )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  
  if ( app )
    {
    if ( this->GetApplicationGUI() != NULL )
      {
      if ( moduleName != NULL )
        {
        // Exit current module
        char * currentModuleName = this->GetModuleNavigator()->GetCurrentModuleName();
        if ( currentModuleName != NULL )
          {
          vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName( currentModuleName );
          if (currentModule != NULL )
            {
            currentModule->Exit ( );
            }
          }
        // Enter selected module.
        vtkSlicerModuleGUI *currentModule = app->GetModuleGUIByName( moduleName );        
        if ( currentModule )
          {
          currentModule->Enter ( );
          
          this->RaiseModule ( moduleName );
          this->GetModuleNavigator()->AddModuleNameToHistoryList ( moduleName );
          this->PopulateHistoryListMenu ( );
          this->GetModuleNavigator()->AddModuleNameToNavigationList ( moduleName );
          }
        else
          {
          vtkErrorMacro ("ERROR no slicer module GUI found for " << moduleName<< "\n");
          }
        }
      }
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
      this->ModulesMenuButton->SetWidth ( 24 );
      this->ModulesMenuButton->IndicatorVisibilityOn ( );
      this->ModulesMenuButton->SetBalloonHelpString ("Select a Slicer module.");

      //--- Frame that contains module nav/search widgets
      this->ModuleNavigationFrame->SetParent ( appF );
      this->ModuleNavigationFrame->Create ( );

      this->ModulesSearchEntry->SetParent ( this->ModuleNavigationFrame );
      this->ModulesSearchEntry->Create ( );
      this->ModulesSearchEntry->SetValue ( "search" );
      this->ModulesSearchEntry->SetWidth ( 12 );
      this->ModulesSearchEntry->SetCommandTriggerToAnyChange();
      this->ModulesSearchEntry->SetBalloonHelpString ("Type the name of a module you want to select and click the 'search' button.");
      this->ModulesSearchEntry->SetForegroundColor ( 0.5, 0.5, 0.5);
      
      //--- Next and previous module button
      this->ModulesNext->SetParent ( this->ModuleNavigationFrame );
      this->ModulesNext->Create ( );
      this->ModulesNext->SetBorderWidth ( 0 );
      this->ModulesNext->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleNextIcon() );
      this->ModulesNext->SetBalloonHelpString ("Go to next module.");

      this->ModulesPrev->SetParent ( this->ModuleNavigationFrame );
      this->ModulesPrev->Create ( );
      this->ModulesPrev->SetBorderWidth ( 0 );
      this->ModulesPrev->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModulePrevIcon() );
      this->ModulesPrev->SetBalloonHelpString ("Go to previous module.");
        
      this->ModulesHistory->SetParent ( this->ModuleNavigationFrame );
      this->ModulesHistory->Create ( );
      this->ModulesHistory->SetBorderWidth ( 0 );
      this->ModulesHistory->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleHistoryIcon() );
      this->ModulesHistory->IndicatorVisibilityOff  ( );
      this->ModulesHistory->SetBalloonHelpString ("List all visited modules.");

      this->ModulesRefresh->SetParent ( this->ModuleNavigationFrame );
      this->ModulesRefresh->Create ( );
      this->ModulesRefresh->SetBorderWidth ( 0 );
      this->ModulesRefresh->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleRefreshIcon() );
      this->ModulesRefresh->SetBalloonHelpString ("Refresh the list of available modules.");

      this->ModulesSearch->SetParent ( this->ModuleNavigationFrame );
      this->ModulesSearch->Create ( );
      this->ModulesSearch->SetBorderWidth ( 0 );
      this->ModulesSearch->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleSearchIcon() );
      this->ModulesSearch->IndicatorVisibilityOff ( );
      this->ModulesSearch->SetBalloonHelpString ("Displays module search results for the text entered to the left (or use keyboard Ctrl+F).");

      //--- create a small label to show search context
      vtkKWLabel *colonLabel = vtkKWLabel::New ( );
      colonLabel->SetParent ( this->ModuleNavigationFrame );
      colonLabel->Create ( );
      colonLabel->SetText (":");
      
      //--- pack everything up.
      app->Script ( "grid %s -row 1 -column 0 -ipadx 0 -padx 0 -pady 0", this->ModulesLabel->GetWidgetName ( ) );
      app->Script ( "grid %s -row 1 -column 1 -ipady 0 -padx 0 -pady 0", this->ModulesMenuButton->GetWidgetName ( ) );
      app->Script ( "grid %s -row 0 -column 1 -sticky nsew -padx 0 -pady 0", this->ModuleNavigationFrame->GetWidgetName ( ) );

      app->Script ( "pack %s -side left -anchor c -padx 0 -pady 2", this->ModulesSearch->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -ipadx 0 -padx 1 -pady 2", colonLabel->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 2 -pady 2", this->ModulesSearchEntry->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesPrev->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesNext->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesHistory->GetWidgetName( ) );      
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesRefresh->GetWidgetName( ) );

      colonLabel->Delete ( );
    }
  }

}

//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::BuildGUI ( vtkKWToolbar *tb )
{
  
  //--- Populate the Slice Control Frame
  if ( tb != NULL )
    {
      //--- ALL modules menu button label
      this->ModulesLabel->SetParent ( tb->GetFrame() );
      this->ModulesLabel->Create ( );
      this->ModulesLabel->SetText ( "   Modules:");
      this->ModulesLabel->SetAnchorToWest ( );
      this->ModulesLabel->SetWidth ( 9 );
      tb->AddWidget ( this->ModulesLabel );

      //--- All modules menu button
      this->ModulesMenuButton->SetParent ( tb->GetFrame() );
      this->ModulesMenuButton->Create ( );
      this->ModulesMenuButton->SetWidth ( 24 );
      this->ModulesMenuButton->IndicatorVisibilityOn ( );
      this->ModulesMenuButton->SetBalloonHelpString ("Select a Slicer module.");
      tb->AddWidget ( this->ModulesMenuButton );
      
      this->ModulesPrev->SetParent ( tb->GetFrame() );
      this->ModulesPrev->Create ( );
      this->ModulesPrev->SetReliefToFlat();
      this->ModulesPrev->SetOverReliefToNone ( );
      this->ModulesPrev->SetBorderWidth ( 0 );
      this->ModulesPrev->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModulePrevIcon() );
      this->ModulesPrev->SetBalloonHelpString ("Go to previous module.");
      tb->AddWidget ( this->ModulesPrev );

      //--- Next and previous module button
      this->ModulesNext->SetParent ( tb->GetFrame());
      this->ModulesNext->Create ( );
      this->ModulesNext->SetReliefToFlat();
      this->ModulesNext->SetOverReliefToNone ( );
      this->ModulesNext->SetBorderWidth ( 0 );
      this->ModulesNext->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleNextIcon() );
      this->ModulesNext->SetBalloonHelpString ("Go to next module.");
      tb->AddWidget ( this->ModulesNext );

      this->ModulesHistory->SetParent ( tb->GetFrame() );
      this->ModulesHistory->Create ( );
      this->ModulesHistory->SetReliefToFlat();
      this->ModulesHistory->SetBorderWidth ( 0 );
      this->ModulesHistory->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleHistoryIcon() );
      this->ModulesHistory->IndicatorVisibilityOff  ( );
      this->ModulesHistory->SetBalloonHelpString ("List all visited modules.");
      tb->AddWidget ( this->ModulesHistory);

      this->ModulesRefresh->SetParent ( tb->GetFrame() );
      this->ModulesRefresh->Create ( );
      this->ModulesRefresh->SetReliefToFlat();
      this->ModulesRefresh->SetOverReliefToNone ( );
      this->ModulesRefresh->SetBorderWidth ( 0 );
      this->ModulesRefresh->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleRefreshIcon() );
      this->ModulesRefresh->SetBalloonHelpString ("Refresh the list of available modules.");
      tb->AddWidget ( this->ModulesRefresh);

      this->ModulesSearchEntry->SetParent ( tb->GetFrame() );
      this->ModulesSearchEntry->Create ( );
      this->ModulesSearchEntry->SetValue ( "search modules" );
      this->ModulesSearchEntry->SetWidth ( 15 );
      this->ModulesSearchEntry->SetCommandTriggerToAnyChange();
      this->ModulesSearchEntry->SetBalloonHelpString ("Type the name of a module you want to select and click the 'search' button.");
      this->ModulesSearchEntry->SetForegroundColor ( 0.5, 0.5, 0.5);
      tb->AddWidget ( this->ModulesSearchEntry );

      //--- create a small label to show search context
//      vtkKWLabel *dotdotLabel = vtkKWLabel::New ( );
//      dotdotLabel->SetParent ( tb->GetFrame() );
//      dotdotLabel->Create ( );
//      dotdotLabel->SetText (":");
//      tb->AddWidget ( dotdotLabel );
//      dotdotLabel->Delete ( );

      this->ModulesSearch->SetParent ( tb->GetFrame() );
      this->ModulesSearch->Create ( );
      this->ModulesSearch->SetReliefToFlat();
      this->ModulesSearch->SetBorderWidth ( 0 );
      this->ModulesSearch->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleSearchIcon() );
      this->ModulesSearch->IndicatorVisibilityOff ( );
      this->ModulesSearch->SetBalloonHelpString ("Displays module search results for the text entered to the left (or use keyboard Ctrl+F).");
      tb->AddWidget ( this->ModulesSearch);




  }


}
  




//---------------------------------------------------------------------------

void vtkSlicerModuleChooseGUI::PopulateModuleSearchMenu ( const char *searchString )
{
  vtkSlicerModuleGUI *m;
  typedef std::vector<std::string> ModuleSet;
  ModuleSet matchingModuleNames;

  const char *mname, *cname;
  char *uc_mname, *uc_cname;

  if ( searchString != NULL )
    {
    int len = strlen ( searchString );
    // don't search until string has a couple of charaters in it to prevent a total avalanche.
    // play with this and see what seems good behavior.
      // convert search string to upper case for search
      char  *uc_searchString = new char [len+1];
      strcpy ( uc_searchString, searchString );
      char *s = uc_searchString;
      convertToUpper( s );

      // Delete and recreate moduleSearchMenu
      // Include modules whose name contains the search term
      // and modules whose category contains the search term.
      if ( (this->GetApplication( )  != NULL ) ) 
        {
        vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
        if ( (app->GetModuleGUICollection ( ) != NULL) ) 
          {
          // Delete all items from menu
          this->GetModulesSearch()->GetMenu( )->DeleteAllItems();

          // find module names or categories containing the search string
          app->GetModuleGUICollection()->InitTraversal();
          m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
          while (m != NULL)
            {
            mname = m->GetUIPanel()->GetName();
            if ( mname != NULL)
              {
              len = strlen ( mname );
              uc_mname = new char [ len +1 ];
              strcpy (uc_mname, mname );
              s = uc_mname;
              convertToUpper( s );
              // does the GUIname contain the search term?
              if ( strstr(uc_mname, uc_searchString ) != NULL )
                {
                matchingModuleNames.push_back (mname);
                }
              }

            cname = m->GetCategory();
            if ( cname != NULL )
              {
              len = strlen (cname );
              uc_cname = new char [len +1 ];
              strcpy (uc_cname, cname );
              s = uc_cname;
              convertToUpper( s );

              // does the GUI category contain the search term?
              if ( strstr(uc_cname, uc_searchString ) != NULL )
                {
                matchingModuleNames.push_back (mname );
                }
              }
            m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
            delete [] uc_mname;
            delete [] uc_cname;
            }
          }

      
        // construct a menu of matching module names
        //
        ModuleSet::iterator mit;
        mit = matchingModuleNames.begin();
      
        while (mit != matchingModuleNames.end())
          {
          this->GetModulesSearch()->GetMenu()->AddRadioButton( (*mit).c_str() );
          mit++;
          }
        }
      delete [] uc_searchString;
      }
}



//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::Populate( )
{
  //const char* mName;
  vtkSlicerModuleGUI *m;

  typedef std::set<std::string> ModuleSet;
  typedef std::map<std::string, ModuleSet > CategoryToModuleVector;
  typedef CategoryToModuleVector::iterator CategoryIterator;

  CategoryToModuleVector categoryToModuleName;

  if ( (this->GetApplication( )  != NULL ) ) 
    {
    vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
    //
    //--- ALL modules pull-down menu 
    // - remove any existing items
    // - add one menu button per module
    // - set the Data Module as default
    //
    if ( (app->GetModuleGUICollection ( ) != NULL) ) 
      {
      this->GetModulesMenuButton()->GetMenu( )->DeleteAllItems();

      // Loop over the module guis in the list and determine which
      // guis are in which categories
      app->GetModuleGUICollection()->InitTraversal();
      m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
      while (m != NULL)
        {
        if (!m->GetCategory() || strcmp(m->GetCategory(), "") == 0)
          {
          categoryToModuleName["None"]
            .insert(m->GetUIPanel()->GetName());
          }
        else
          {
          categoryToModuleName[m->GetCategory()]
            .insert(m->GetUIPanel()->GetName());
          }

        m = vtkSlicerModuleGUI::SafeDownCast( app->GetModuleGUICollection( )->GetNextItemAsObject( ));
        }

      // construct a cascading menu of module guis
      //
      //


      // Have one menu that lists all the modules
      vtkKWMenu* all = vtkKWMenu::New();
      all->SetParent(this->GetModulesMenuButton()->GetMenu());
      all->Create();
      this->GetModulesMenuButton()->GetMenu()->AddCascade("All Modules", all);
      this->GetModulesMenuButton()->GetMenu()->AddSeparator();
      all->Delete();

      // first, put the uncategorized modules
      int index;
      ModuleSet::iterator mit;
      mit = categoryToModuleName["None"].begin();

      typedef std::map<std::string, std::string > AllMap;
      AllMap allMap;
      
      while (mit != categoryToModuleName["None"].end())
        {
        std::stringstream methodString;
        methodString << "SelectModule \"" << (*mit).c_str() << "\"";
        this->GetModulesMenuButton()->GetMenu( )
          ->AddCommand( (*mit).c_str(), this,
                        methodString.str().c_str() );
        allMap[(*mit).c_str()] = methodString.str();
        ++mit;
        }
      this->GetModulesMenuButton()->GetMenu()->AddSeparator();

      // add the rest of the menus
      CategoryIterator cit;
      cit = categoryToModuleName.begin();
      while (cit != categoryToModuleName.end())
        {
        if ((*cit).first != "None")
          {
          // tease apart the category to find the path and final
          // category
          std::vector<std::string> path;
          splitString((*cit).first, ".", path);

          // need to create a set of cascading menus
          vtkKWMenu *pos = this->GetModulesMenuButton()->GetMenu();
          for (unsigned int i=0; i < path.size(); ++i)
            {
            if (!pos->HasItem(path[i].c_str()))
              {
              // Need to make the itermediate menu
              vtkKWMenu* menu = vtkKWMenu::New();
              menu->SetParent( pos );
              menu->Create();
              
              pos->AddCascade( path[i].c_str(), menu );
              menu->Delete();
              }

            index = pos->GetIndexOfItem( path[i].c_str() );
            pos = pos->GetItemCascade(index);
            }
          // keep a handle on the last menu in cascading sequence
          vtkKWMenu *menu = pos;

          // add the items to the submenu
          ModuleSet::iterator mit;
          mit = (*cit).second.begin();
          while (mit != (*cit).second.end())
            {
            std::stringstream methodString;
            methodString << "SelectModule \"" << (*mit).c_str() << "\"";
            index = menu->AddCommand( (*mit).c_str(), this,
                                      methodString.str().c_str());

            allMap[(*mit).c_str()] = methodString.str();
            ++mit;
            }
          }
        
        ++cit;
        }

      // build the all modules meny in sorted order
      AllMap::iterator allIt = allMap.begin();
      int numLines = 0;
      while ( allIt != allMap.end() )
        {
        all->AddCommand( (*allIt).first.c_str(), this,
                         (*allIt).second.c_str());
        ++allIt;
        numLines++;
        if (numLines % 40 == 0)
          {
          all->SetItemColumnBreak(numLines - 1, 1);
          }
        }
      
      }
    //--- TODO: make the initial value be module user sets as "home"
    this->GetModulesMenuButton()->SetValue ("Data");
    }
  
}



