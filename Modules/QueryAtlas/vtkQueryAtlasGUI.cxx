#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWWidget.h"
#include "vtkKWPushButton.h"
#include "vtkKWMenu.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
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

    this->GeneButton = NULL;
    this->ProteinButton = NULL;
    this->CellButton = NULL;
    this->StructureButton = NULL;
    this->PopulationButton = NULL;
    this->SpeciesButton = NULL;
    this->SwitchQueryFrame = NULL;

    this->GeneFrame = NULL;
    this->CellFrame = NULL;
    this->ProteinFrame = NULL;

    this->SpeciesFrame = NULL;
    this->SpeciesMenuButton = NULL;

    this->PopulationFrame = NULL;
    this->DiagnosisMenuButton = NULL;
    this->GenderMenuButton = NULL;
    this->HandednessMenuButton = NULL;
    this->AgeMenuButton = NULL;
    this->ClinicalTestMenuButton = NULL;

    this->StructureFrame = NULL;
    this->StructureMenuButton = NULL;
    this->SelectAllButton = NULL;
    this->SelectNoneButton = NULL;
    this->ClearButton = NULL;
    this->AddTermButton = NULL;
    this->DeleteTermButton = NULL;    
    this->SearchTermMultiColumnList = NULL;

    this->SearchButton = NULL;
    this->DatabasesMenuButton = NULL;

    this->NumberOfColumns = 2;


    this->SPLHierarchyButton = NULL;
    this->BIRNLexHierarchyButton = NULL;
    
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
    if ( this->GeneButton )
      {
      this->GeneButton->SetParent ( NULL );
      this->GeneButton->Delete();
      this->GeneButton = NULL;      
      }
    if ( this->CellButton )
      {
      this->CellButton->SetParent ( NULL );
      this->CellButton->Delete();
      this->CellButton = NULL;   
      }
    if ( this->ProteinButton )
      {
      this->ProteinButton->SetParent ( NULL );
      this->ProteinButton->Delete();      
      this->ProteinButton = NULL;
      }
    if ( this->StructureButton )
      {
      this->StructureButton->SetParent ( NULL );
      this->StructureButton->Delete();
      this->StructureButton = NULL;
      }
    if ( this->PopulationButton )
      {
      this->PopulationButton->SetParent ( NULL );
      this->PopulationButton->Delete();
      this->PopulationButton = NULL;      
      }
    if ( this->DiagnosisMenuButton )
      {
      this->DiagnosisMenuButton->SetParent ( NULL );
      this->DiagnosisMenuButton->Delete();
      this->DiagnosisMenuButton = NULL;
      }
    if ( this->GenderMenuButton )
      {
      this->GenderMenuButton->SetParent ( NULL );
      this->GenderMenuButton->Delete();
      this->GenderMenuButton = NULL;
      }
    if ( this->HandednessMenuButton )
      {
      this->HandednessMenuButton->SetParent ( NULL );
      this->HandednessMenuButton->Delete();
      this->HandednessMenuButton = NULL;
      }
    if ( this->AgeMenuButton )
      {
      this->AgeMenuButton->SetParent ( NULL );
      this->AgeMenuButton->Delete();
      this->AgeMenuButton  = NULL;
      }
    if ( this->ClinicalTestMenuButton )
      {
      this->ClinicalTestMenuButton->SetParent ( NULL );
      this->ClinicalTestMenuButton->Delete();
      this->ClinicalTestMenuButton = NULL;
      }
    if ( this->SpeciesButton )
      {
      this->SpeciesButton->SetParent ( NULL );
      this->SpeciesButton->Delete();
      this->SpeciesButton = NULL;      
      }
    if ( this->SpeciesMenuButton )
      {
      this->SpeciesMenuButton->SetParent ( NULL );
      this->SpeciesMenuButton->Delete();
      this->SpeciesMenuButton = NULL;
      }
    
    if ( this->SwitchQueryFrame)
      {
      this->SwitchQueryFrame->SetParent ( NULL );
      this->SwitchQueryFrame->Delete();
      this->SwitchQueryFrame = NULL;
      }

    if ( this->SpeciesFrame)
      {
      this->SpeciesFrame->SetParent ( NULL );
      this->SpeciesFrame->Delete();
      this->SpeciesFrame = NULL;
      }
    if ( this->PopulationFrame )
      {
      this->PopulationFrame->SetParent ( NULL );
      this->PopulationFrame->Delete();
      this->PopulationFrame = NULL;
      }
    if ( this->StructureFrame )
      {
      this->StructureFrame->SetParent ( NULL );
      this->StructureFrame->Delete();
      this->StructureFrame = NULL;      
      }
    if ( this->CellFrame )
      {
      this->CellFrame->SetParent ( NULL );
      this->CellFrame->Delete();
      this->CellFrame = NULL;      
      }
    if ( this->ProteinFrame)
      {
      this->ProteinFrame->SetParent ( NULL );
      this->ProteinFrame->Delete();
      this->ProteinFrame = NULL;
      }
    if ( this->GeneFrame)
      {
      this->GeneFrame->SetParent ( NULL );
      this->GeneFrame->Delete();
      this->GeneFrame = NULL;
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
    if ( this->StructureMenuButton)
      {
      this->StructureMenuButton->SetParent ( NULL );
      this->StructureMenuButton->Delete();
      this->StructureMenuButton = NULL;
      }
    if ( this->SPLHierarchyButton )
      {
      this->SPLHierarchyButton->SetParent ( NULL );
      this->SPLHierarchyButton->Delete();
      this->SPLHierarchyButton = NULL;
      }
    if ( this->BIRNLexHierarchyButton )
      {
      this->BIRNLexHierarchyButton->SetParent ( NULL );
      this->BIRNLexHierarchyButton->Delete();
      this->BIRNLexHierarchyButton= NULL;
      }
    if ( this->SearchTermMultiColumnList )
      {
      this->SearchTermMultiColumnList->SetParent ( NULL );
      this->SearchTermMultiColumnList->Delete ( );
      this->SearchTermMultiColumnList = NULL;
      }
    if ( this->DatabasesMenuButton )
      {
      this->DatabasesMenuButton->SetParent ( NULL );
      this->DatabasesMenuButton->Delete ( );
      this->DatabasesMenuButton = NULL;      
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
  this->SPLHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->DatabasesMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ProteinButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CellButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PopulationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  this->SPLHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->DatabasesMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->GeneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ProteinButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CellButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PopulationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

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
    this->Script ( "source ../../../Slicer3/Modules/QueryAtlas/Tcl/QueryAtlas.tcl; QueryAtlasInit");
    }
  else if ( (b == this->SPLHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "if { [info exists ::QA(filename) ] } {QueryAtlasLaunchSPLHierarchy}");
    }
  else if ( (b == this->BIRNLexHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
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
  else if ( (b == this->GeneButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "raise %s", this->GeneFrame->GetWidgetName() );
    }
  else if ( (b == this->ProteinButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "raise %s", this->ProteinFrame->GetWidgetName() );
    }
  else if ( (b == this->CellButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "raise %s", this->CellFrame->GetWidgetName() );
    }
  else if ( (b == this->StructureButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "raise %s", this->StructureFrame->GetWidgetName() );
    }
  else if ( (b == this->PopulationButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "raise %s", this->PopulationFrame->GetWidgetName() );
    }
  else if ( (b == this->SpeciesButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "raise %s", this->SpeciesFrame->GetWidgetName() );
    }

//    if ( (m == this->DatabasesMenuButton->GetMenu() ) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
//    {
//    }
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
    loadFrame->SetLabelText ("Load 3D Query Scene");
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


    // -------------------------------------------------------------------------------------------------
    // ---
    // BUILD QUERY FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *queryFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    queryFrame->SetParent ( page );
    queryFrame->Create ( );
    queryFrame->SetLabelText ("Query Builder");
    queryFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  queryFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    this->BuildQueryBuilderContextButtons ( queryFrame->GetFrame() );

    this->SwitchQueryFrame = vtkKWFrame::New();
    this->SwitchQueryFrame->SetParent ( queryFrame->GetFrame() );
    this->SwitchQueryFrame->Create();
    app->Script ( "pack %s -side top -anchor w -fill both", this->SwitchQueryFrame->GetWidgetName() );
    this->BuildQueryBuilderContextFrames ( this->SwitchQueryFrame );

    this->BuildSpeciesFrame();
    this->BuildPopulationFrame();
    this->BuildStructureFrame();
    this->BuildCellFrame();
    this->BuildProteinFrame();
    this->BuildGeneFrame();

    // -------------------------------------------------------------------------------------------------
    // ---
    // HIERARCHY FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *hierarchyFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    hierarchyFrame->SetParent ( page );
    hierarchyFrame->Create ( );
    hierarchyFrame->SetLabelText ("Hierarchy Search Options");
    hierarchyFrame->CollapseFrame ( );

    this->BIRNLexHierarchyButton = vtkKWPushButton::New();
    this->BIRNLexHierarchyButton->SetParent ( hierarchyFrame->GetFrame() );
    this->BIRNLexHierarchyButton->Create();
    this->BIRNLexHierarchyButton->SetWidth ( 14 );
    this->BIRNLexHierarchyButton->SetText ( "BIRNLex ");
    this->BIRNLexHierarchyButton->SetBalloonHelpString ("browse for this term in BIRNLex.");

    this->SPLHierarchyButton = vtkKWPushButton::New();
    this->SPLHierarchyButton->SetParent ( hierarchyFrame->GetFrame() );
    this->SPLHierarchyButton->Create();
    this->SPLHierarchyButton->SetWidth ( 14 );
    this->SPLHierarchyButton->SetText ( "SPL Brain Atlas");
    this->SPLHierarchyButton->SetBalloonHelpString ("browse for this term in the SPL brain anatomy hierarchy.");

    app->Script ( "pack %s -side top -anchor nw -fill none -padx 2 -pady 2",
                  this->BIRNLexHierarchyButton->GetWidgetName ( ));
    app->Script ( "pack %s -side top -anchor nw -fill none -padx 2 -pady 2",
                  this->SPLHierarchyButton->GetWidgetName());
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  hierarchyFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    // ---
    // RESULTS FRAME
    vtkSlicerModuleCollapsibleFrame *resultsFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    resultsFrame->SetParent ( page );
    resultsFrame->Create ( );
    resultsFrame->SetLabelText ("Query Results Manager");
    resultsFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  resultsFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    // ---
    // DISPLAY FRAME
    vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayFrame->SetParent ( page );
    displayFrame->Create ( );
    displayFrame->SetLabelText ("3D Display & Navigation");
    displayFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    // ---
    // SAVE FRAME
    vtkSlicerModuleCollapsibleFrame *saveFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    saveFrame->SetParent ( page );
    saveFrame->Create ( );
    saveFrame->SetLabelText ("Save Display Options");
    saveFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  saveFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    
    // deleting frame widgets
    displayFrame->Delete ( );
    resultsFrame->Delete ( );
    queryFrame->Delete ( );
    hierarchyFrame->Delete();
    saveFrame->Delete();
    loadFrame->Delete ( );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildStructureFrame()
{
    // add and pack top button frame
    vtkKWFrame *tbframe = vtkKWFrame::New ( );
    tbframe->SetParent ( this->StructureFrame );
    tbframe->Create ( );
    this->Script ( "pack %s -side top -fill x -expand n",  tbframe->GetWidgetName() );
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
    this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 6 -pady 3",
                  this->AddTermButton->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1 -sticky e -padx 6 -pady 3",
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
    this->SearchTermMultiColumnList->SetParent ( this->StructureFrame );
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
    this->Script ( "pack %s -side top -fill x -expand true", this->SearchTermMultiColumnList->GetWidgetName() );

    // add and pack bottom button frame
    vtkKWFrame *bbframe = vtkKWFrame::New ( );
    bbframe->SetParent ( this->StructureFrame );
    bbframe->Create ( );
    this->Script ( "pack %s -side top -fill x -expand n",  bbframe->GetWidgetName() );

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
    this->ClearButton->SetBalloonHelpString ( "Clear all search terms" );

    // grid buttons into place.
    this->Script ( "grid %s -row 0 -column 0 -sticky e -padx 6 -pady 3",
                  this->SelectAllButton->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1 -sticky w -padx 6 -pady 3",
                  this->SelectNoneButton->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 0 -sticky e -padx 6 -pady 3",
                  this->SearchButton->GetWidgetName());
    this->Script ( "grid %s -row 1 -column 1 -sticky w -padx 6 -pady 3",
                  this->ClearButton->GetWidgetName() );

    tbframe->Delete ( );
    bbframe->Delete ( );

}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildSpeciesFrame()
{
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildPopulationFrame()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildCellFrame()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildProteinFrame()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildGeneFrame()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextFrames ( vtkKWFrame *parent )
{
    this->StructureFrame = vtkKWFrame::New();
    this->StructureFrame->SetParent ( parent );
    this->StructureFrame->Create();
    this->Script ( "place %s -relx 0 -rely 0 -relwidth 1.0 -relheight 1.0", this->StructureFrame->GetWidgetName( ) );
    this->Script ( "raise %s", this->StructureFrame->GetWidgetName() );
    
    this->PopulationFrame = vtkKWFrame::New();
    this->PopulationFrame->SetParent ( parent );
    this->PopulationFrame->Create();
    this->Script ( "place %s -relx 0 -rely 0 -relwidth 1.0 -relheight 1.0", this->PopulationFrame->GetWidgetName( ) );

    this->SpeciesFrame = vtkKWFrame::New();
    this->SpeciesFrame->SetParent ( parent );
    this->SpeciesFrame->Create();
    this->Script ( "place %s -relx 0 -rely 0 -relwidth 1.0 -relheight 1.0", this->SpeciesFrame->GetWidgetName( ) );

    this->CellFrame = vtkKWFrame::New();
    this->CellFrame->SetParent ( parent );
    this->CellFrame->Create();
    this->Script ( "place %s -relx 0 -rely 0 -relwidth 1.0 -relheight 1.0", this->CellFrame->GetWidgetName( ) );

    this->ProteinFrame = vtkKWFrame::New();
    this->ProteinFrame->SetParent ( parent );
    this->ProteinFrame->Create();
    this->Script ( "place %s -relx 0 -rely 0 -relwidth 1.0 -relheight 1.0", this->ProteinFrame->GetWidgetName( ) );

    this->GeneFrame = vtkKWFrame::New();
    this->GeneFrame->SetParent ( parent );
    this->GeneFrame->Create();
    this->Script ( "place %s -relx 0 -rely 0 -relwidth 1.0 -relheight 1.0", this->GeneFrame->GetWidgetName( ) );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextButtons ( vtkKWFrame *parent )
{
    this->GeneButton = vtkKWPushButton::New();
    this->GeneButton->SetParent(parent );
    this->GeneButton->Create();
    this->GeneButton->SetWidth (10 );
    this->GeneButton->SetText ( "gene" );

    this->ProteinButton = vtkKWPushButton::New();
    this->ProteinButton->SetParent ( parent );
    this->ProteinButton->Create();
    this->ProteinButton->SetWidth ( 10 );
    this->ProteinButton->SetText ( "protein" );
    
    this->CellButton = vtkKWPushButton::New();
    this->CellButton->SetParent ( parent );
    this->CellButton->Create();
    this->CellButton->SetWidth ( 10 );
    this->CellButton->SetText ( "cell ");
    
    this->StructureButton = vtkKWPushButton::New();
    this->StructureButton->SetParent ( parent );
    this->StructureButton->Create();
    this->StructureButton->SetWidth ( 10 );
    this->StructureButton->SetText ( "brain");
    
    this->PopulationButton = vtkKWPushButton::New();
    this->PopulationButton->SetParent ( parent );
    this->PopulationButton->Create();
    this->PopulationButton->SetWidth ( 10 );
    this->PopulationButton->SetText ( "population");
    
    this->SpeciesButton = vtkKWPushButton::New();
    this->SpeciesButton->SetParent (parent );
    this->SpeciesButton->Create();
    this->SpeciesButton->SetWidth ( 10 );
    this->SpeciesButton->SetText ( "species");    

    this->Script ( "pack %s %s %s %s %s %s -anchor nw -side left -fill none -padx 2 -pady 2",
                  this->GeneButton->GetWidgetName(),
                  this->ProteinButton->GetWidgetName(),
                  this->CellButton->GetWidgetName(),
                  this->StructureButton->GetWidgetName(),
                  this->PopulationButton->GetWidgetName(),
                  this->SpeciesButton->GetWidgetName() );

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
    vtkErrorMacro (<< "Selected rows (" << numRows << ") not 1, just pick one to delete for now\n");
    return;
    }
}
