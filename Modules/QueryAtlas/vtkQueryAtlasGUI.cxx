#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWWidget.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"

#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkQueryAtlasGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasGUI );
vtkCxxRevisionMacro ( vtkQueryAtlasGUI, "$Revision: 1.0 $");


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::vtkQueryAtlasGUI ( )
{
    this->Logic = NULL;
    this->LoadSceneButton = NULL;
    this->SelectAllButton = NULL;
    this->SelectNoneButton = NULL;
    this->SearchButton = NULL;
    this->ClearButton = NULL;
    this->AddTermButton = NULL;
    this->DeleteTermButton = NULL;    
    this->SearchTermMultiColumnList = NULL;
    this->SearchTargetMenuButton = NULL;
    this->NumberOfColumns = 2;
    
    //this->DebugOn();
}


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::~vtkQueryAtlasGUI ( )
{

    this->SetModuleLogic ( NULL );
    if ( this->LoadSceneButton )
      {
      this->LoadSceneButton->SetParent ( NULL );
      this->LoadSceneButton->Delete ( );
      this->LoadSceneButton = NULL;
      }
    if ( this->SearchButton )
      {
      this->SearchButton->SetParent ( NULL );
      this->SearchButton->Delete ( );
      this->SearchButton = NULL;
      }
    if ( this->ClearButton )
      {
      this->ClearButton->SetParent ( NULL );
      this->ClearButton->Delete ( );
      this->ClearButton = NULL;
      }
    if ( this->AddTermButton )
      {
      this->AddTermButton->SetParent ( NULL );
      this->AddTermButton->Delete ( );
      this->AddTermButton = NULL;
      }
    if ( this->DeleteTermButton )
      {
      this->DeleteTermButton->SetParent ( NULL );
      this->DeleteTermButton->Delete ( );
      this->DeleteTermButton = NULL;
      }

    if ( this->SelectAllButton )
      {
      this->SelectAllButton->SetParent ( NULL );
      this->SelectAllButton->Delete ( );
      this->SelectAllButton = NULL;
      }
    if ( this->SelectNoneButton )
      {
      this->SelectNoneButton->SetParent ( NULL );
      this->SelectNoneButton->Delete ( );
      this->SelectNoneButton = NULL;
      }
    
    if ( this->SearchTermMultiColumnList )
      {
      this->SearchTermMultiColumnList->SetParent ( NULL );
      this->SearchTermMultiColumnList->Delete ( );
      this->SearchTermMultiColumnList = NULL;
      }
    if ( this->SearchTargetMenuButton )
      {
      this->SearchTargetMenuButton->SetParent ( NULL );
      this->SearchTargetMenuButton->Delete ( );
      this->SearchTargetMenuButton = NULL;      
      }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "QueryAtlasGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";

    // print widgets?
    os << indent << "LoadSceneButton" << this->GetLoadSceneButton ( ) << "\n";
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::RemoveGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: RemoveGUIObservers\n");
  this->LoadSceneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->ClearButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectNoneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->SearchTargetMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: AddGUIObservers\n");
  this->LoadSceneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->SelectAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectNoneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->SearchTargetMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // nothing to do here yet...
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );

  if ( (b == this->LoadSceneButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    }
  else if ( (b == this->SearchButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    }
  else if ( (b == this->ClearButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->DeleteAllSearchTerms();
    }
  else if ( (b == this->AddTermButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->AddNewSearchTerm();
    }
  else if ( (b == this->DeleteTermButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->DeleteSelectedSearchTerms ();
    }
  else if ( (b == this->SelectAllButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->SelectAllSearchTerms ( );
    }
  else if ( (b == this->SelectNoneButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->DeselectAllSearchTerms ( );
    }  
    else if ( (m == this->SearchTargetMenuButton->GetMenu() ) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
    {
    }
    return;
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessLogicEvents ( vtkObject *caller,
                                              unsigned long event, void *callData )
{
    // Fill in
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessMRMLEvents ( vtkObject *caller,
                                             unsigned long event, void *callData )
{    
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::Enter ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: Enter\n");
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::Exit ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: Exit\n");
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildGUI ( )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  // Define your help text here.


    // ---
    // MODULE GUI FRAME 
    // configure a page for a model loading UI for now.
    // later, switch on the modulesButton in the SlicerControlGUI
    // ---
    // create a page
    this->UIPanel->AddPage ( "QueryAtlas", "QueryAtlas", NULL );

    const char *help = "The Query Atlas module allows interactive Google, Wikipedia, queries from within the 3D anatomical display.";
    const char *about = "This work was supported by BIRN, NA-MIC, NAC, NCIGT, and the Slicer Community. See http://www.slicer.org for details. ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    this->BuildHelpAndAboutFrame ( page, help, about );
    
    // ---
    // LOAD FRAME            
    vtkSlicerModuleCollapsibleFrame *loadFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    loadFrame->SetParent ( page );
    loadFrame->Create ( );
    loadFrame->SetLabelText ("Load Scene");
    loadFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  loadFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());
    
    // add button to load a scene. this is wrong widget, but for now let it sit.
    this->LoadSceneButton = vtkKWPushButton::New ( );
    this->LoadSceneButton->SetParent ( loadFrame->GetFrame() );
    this->LoadSceneButton->Create();
    this->LoadSceneButton->SetText ( "LoadScene" );
    this->LoadSceneButton->SetBalloonHelpString ( "Select all search terms for use");
    app->Script ( "pack %s -side top -padx 3 -pady 3", this->LoadSceneButton->GetWidgetName() );


    // ---
    // QUERY FRAME
    vtkSlicerModuleCollapsibleFrame *queryFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    queryFrame->SetParent ( page );
    queryFrame->Create ( );
    queryFrame->SetLabelText ("Query Options");
    queryFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  queryFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    // add and pack top button frame
    vtkKWFrame *tbframe = vtkKWFrame::New ( );
    tbframe->SetParent ( queryFrame->GetFrame() );
    tbframe->Create ( );
    app->Script ( "pack %s -side top -fill x -expand n",  tbframe->GetWidgetName() );

    // add search terms and delete highlighted search terms buttons
    this->AddTermButton = vtkKWPushButton::New ( );
    this->AddTermButton->SetParent ( tbframe );
    this->AddTermButton->Create();
    this->AddTermButton->SetWidth ( 15 );
    this->AddTermButton->SetText ( "Add search term" );
    this->AddTermButton->SetBalloonHelpString ( "Add a new search term" );

    this->DeleteTermButton = vtkKWPushButton::New ( );
    this->DeleteTermButton->SetParent ( tbframe );
    this->DeleteTermButton->Create();
    this->DeleteTermButton->SetWidth ( 15 );
    this->DeleteTermButton->SetText ( "Delete selected" );
    this->DeleteTermButton->SetBalloonHelpString ( "Delete highlighted search terms" );

    // grid buttons into place
    app->Script ( "grid %s -row 0 -column 0 -sticky e -padx 6 -pady 3",
                  this->AddTermButton->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky e -padx 6 -pady 3",
                  this->DeleteTermButton->GetWidgetName());
    
    // add multi-column list box for search terms
    // IDEA: click 'add term' button and get a new row in this widget.
    // widget (for now) had two columns, first has a radiobutton in it
    // for marking the row for use in search, second is an entry widget
    // in which user types a search term.
    // Delete button above works when a row is highlighted. how is
    // that accomplished? maybe a better way. Just delete rows with
    // radio button selected? (select for use and for deletion?)
    // grrr. i don't yet understand how this widget works.
    this->SearchTermMultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
    this->SearchTermMultiColumnList->SetParent ( queryFrame->GetFrame() );
    this->SearchTermMultiColumnList->Create ( );
//    this->SearchTermMultiColumnList->SetBalloonHelpString ("Use the 'Add new term' button to create a new row in this widget and then click in the 'Search term' column to enter your new search term. Select or disable the term's use in your search by selecting the checkbox next to it. The space for one search term is created by default." );
    this->SearchTermMultiColumnList->SetWidth(0);
    this->SearchTermMultiColumnList->SetHeight(1);
    this->SearchTermMultiColumnList->GetWidget()->SetSelectionTypeToCell ( );
    this->SearchTermMultiColumnList->GetWidget()->MovableRowsOff ( );
    this->SearchTermMultiColumnList->GetWidget()->MovableColumnsOff ( );

    this->SearchTermMultiColumnList->GetWidget()->AddColumn ( "Use");
//    this->SearchTermMultiColumnList->GetWidget()->ColumnEditableOn ( this->SelectionColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnWidth (this->SelectionColumn, 5);
    this->SearchTermMultiColumnList->GetWidget()->SetColumnAlignmentToCenter ( this->SelectionColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnResizable ( this->SelectionColumn, 0 );

    this->SearchTermMultiColumnList->GetWidget()->AddColumn ( "Search Terms" );
    this->SearchTermMultiColumnList->GetWidget()->ColumnEditableOn ( this->SearchTermColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnWidth (this->SearchTermColumn, 35);
    this->SearchTermMultiColumnList->GetWidget()->SetColumnAlignmentToLeft (this->SearchTermColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnResizable ( this->SearchTermColumn, 0 );

    // default search terms in list
    this->AddNewSearchTerm ( );
    app->Script ( "pack %s -side top -fill x -expand true", this->SearchTermMultiColumnList->GetWidgetName() );

    // add and pack bottom button frame
    vtkKWFrame *bbframe = vtkKWFrame::New ( );
    bbframe->SetParent ( queryFrame->GetFrame() );
    bbframe->Create ( );
    app->Script ( "pack %s -side top -fill x -expand n",  bbframe->GetWidgetName() );

    // add search, clear, select all, select none buttons
    this->SelectAllButton = vtkKWPushButton::New ( );
    this->SelectAllButton->SetParent ( bbframe );
    this->SelectAllButton->Create();
    this->SelectAllButton->SetWidth ( 10 );
    this->SelectAllButton->SetText ( "Mark all" );
    this->SelectAllButton->SetBalloonHelpString ( "Marck all search terms for use");

    this->SelectNoneButton = vtkKWPushButton::New ( );
    this->SelectNoneButton->SetParent ( bbframe );
    this->SelectNoneButton->Create();
    this->SelectNoneButton->SetWidth ( 10 );
    this->SelectNoneButton->SetText ( "Unmark all" );
    this->SelectNoneButton->SetBalloonHelpString ( "Unmark all search terms.");

    this->SearchButton = vtkKWPushButton::New ( );
    this->SearchButton->SetParent ( bbframe );
    this->SearchButton->Create();
    this->SearchButton->SetWidth ( 10 );
    this->SearchButton->SetText ( "Search" );
    this->SearchButton->SetBalloonHelpString ( "Start a search using the selected search terms" );

    this->ClearButton = vtkKWPushButton::New ( );
    this->ClearButton->SetParent ( bbframe );
    this->ClearButton->Create();
    this->ClearButton->SetWidth ( 10 );
    this->ClearButton->SetText ( "Clear" );
    this->SearchButton->SetBalloonHelpString ( "Clear all search terms" );

    // grid buttons into place.
    app->Script ( "grid %s -row 0 -column 0 -sticky e -padx 6 -pady 3",
                  this->SelectAllButton->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky w -padx 6 -pady 3",
                  this->SelectNoneButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 0 -sticky e -padx 6 -pady 3",
                  this->SearchButton->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 1 -sticky w -padx 6 -pady 3",
                  this->ClearButton->GetWidgetName() );

    // ---
    // DISPLAY FRAME
    vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayFrame->SetParent ( page );
    displayFrame->Create ( );
    displayFrame->SetLabelText ("Display Options");
    displayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    
    // deleting frame widgets
    tbframe->Delete ( );
    bbframe->Delete ( );
    displayFrame->Delete ( );
    queryFrame->Delete ( );
    loadFrame->Delete ( );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::SelectAllSearchTerms ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Select All SearchTerms event. \n");  
  int numrows = this->SearchTermMultiColumnList->GetWidget()->GetNumberOfRows();
  int i;
  for ( i = 0; i < numrows; i++ )
    {
    this->SearchTermMultiColumnList->GetWidget()->SetCellText ( i, this->SelectionColumn, "1" );
    }
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeselectAllSearchTerms ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Deselect All SearchTerms event. \n");  
  int numrows = this->SearchTermMultiColumnList->GetWidget()->GetNumberOfRows();
  int i;
  for ( i = 0; i < numrows; i++ )
    {
    this->SearchTermMultiColumnList->GetWidget()->SetCellText ( i, this->SelectionColumn, "0" );
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteAllSearchTerms ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Clear All SearchTerms event. \n");
  int numrows = this->SearchTermMultiColumnList->GetWidget()->GetNumberOfRows();
  // remove each row
  this->SearchTermMultiColumnList->GetWidget()->DeleteAllRows();
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddNewSearchTerm ( )
{
    // default search terms in list
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Adding New SearchTerms event. \n");
    int i = this->SearchTermMultiColumnList->GetWidget()->GetNumberOfRows();
    this->SearchTermMultiColumnList->GetWidget()->InsertCellTextAsInt ( i, this->SelectionColumn, 0 );
    this->SearchTermMultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton (i, this->SelectionColumn );
    this->SearchTermMultiColumnList->GetWidget()->InsertCellText (i, this->SearchTermColumn, "edit search term here" );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnEditWindowToEntry (this->SearchTermColumn);
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteSelectedSearchTerms ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: DeleteSearchTerm event\n");
  // check to see if should confirm
  const char * confirmDelete = ((vtkSlicerApplication *)this->GetApplication())->GetConfirmDelete();
  int confirmDeleteFlag = 0;
  if (confirmDelete != NULL &&
      strncmp(confirmDelete, "1", 1) == 0)
    {
    vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: confirm delete flag is 1\n");
    confirmDeleteFlag = 1;
    }
  else
    {
    vtkDebugMacro("Not confirming deletes, confirmDelete = '" << confirmDelete << "'\n");
    }
  // save state for undo hrm... not populating the mrml node yet with these search terms, so...
  //this->MRMLScene->SaveStateForUndo();
        
  // get the row that was last selected
  int numRows = this->SearchTermMultiColumnList->GetWidget()->GetNumberOfSelectedRows();
  if (numRows == 1)
    {
    int row[1];
    this->SearchTermMultiColumnList->GetWidget()->GetSelectedRows(row);

    if (confirmDeleteFlag)
      {
      // confirm that really want to remove this term
      std::cout << "Deleting search term " << row[0] << endl;
      }
            
    // then remove that row by index
    this->SearchTermMultiColumnList->GetWidget()->DeleteRow ( row[0] );
    }
  else
    {
    std::cerr << "Selected rows (" << numRows << ") not 1, just pick one to delete for now\n";
    return;
    }
}
