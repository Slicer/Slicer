#include <map>
#include <set>

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
    this->ModulesHistory = vtkKWPushButton::New ( );
    this->ModulesRefresh = vtkKWPushButton::New ( );
    this->ModulesSearch = vtkKWPushButton::New ( );
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
    if ( this->ModulesSearch) {
      this->ModulesSearch->SetParent ( NULL );
        this->ModulesSearch->Delete ( );
        this->ModulesSearch = NULL;
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
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::AddGUIObservers ( )
{
}


//---------------------------------------------------------------------------
void vtkSlicerModuleChooseGUI::ProcessGUIEvents ( vtkObject *caller,
                                          unsigned long event, void *callData )
{
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

      //--- Next and previous module button
      this->ModulesNext->SetParent ( appF );
      this->ModulesNext->Create ( );
      this->ModulesNext->SetBorderWidth ( 0 );
      this->ModulesNext->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleNextIcon() );
      this->ModulesNext->SetBalloonHelpString ("Go to next module.");

      this->ModulesPrev->SetParent ( appF );
      this->ModulesPrev->Create ( );
      this->ModulesPrev->SetBorderWidth ( 0 );
      this->ModulesPrev->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModulePrevIcon() );
      this->ModulesPrev->SetBalloonHelpString ("Go to previous module.");
        
      this->ModulesHistory->SetParent ( appF );
      this->ModulesHistory->Create ( );
      this->ModulesHistory->SetBorderWidth ( 0 );
      this->ModulesHistory->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleHistoryIcon() );
      this->ModulesHistory->SetBalloonHelpString ("List all visited modules.");

      this->ModulesRefresh->SetParent ( appF );
      this->ModulesRefresh->Create ( );
      this->ModulesRefresh->SetBorderWidth ( 0 );
      this->ModulesRefresh->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleRefreshIcon() );
      this->ModulesRefresh->SetBalloonHelpString ("Refresh the list of available modules.");

      this->ModulesSearch->SetParent ( appF );
      this->ModulesSearch->Create ( );
      this->ModulesSearch->SetBorderWidth ( 0 );
      this->ModulesSearch->SetImageToIcon ( this->SlicerModuleNavigationIcons->GetModuleSearchIcon() );
      this->ModulesSearch->SetBalloonHelpString ("Search for a module (or use keyboard Ctrl+F).");

      //--- pack everything up.
      app->Script ( "pack %s -side left -anchor n -padx 0 -ipadx 0 -pady 3", this->ModulesLabel->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor se -padx 0 -ipady 0 -pady 0", this->ModulesMenuButton->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesHistory->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesPrev->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesNext->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesSearch->GetWidgetName( ) );
      app->Script ( "pack %s -side left -anchor c -padx 1 -pady 2", this->ModulesRefresh->GetWidgetName( ) );
    }
  }

}



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
      while ( allIt != allMap.end() )
        {
        all->AddCommand( (*allIt).first.c_str(), this,
                         (*allIt).second.c_str());
        ++allIt;
        }
      
      }
    //--- TODO: make the initial value be module user sets as "home"
    this->GetModulesMenuButton()->SetValue ("Data");
    }
}



