#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWWidget.h"
#include "vtkKWPushButton.h"
#include "vtkKWCheckButton.h"
#include "vtkKWRadioButton.h"
#include "vtkKWMenu.h"
#include "vtkKWLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWMultiColumnList.h"
#include "vtkKWMultiColumnListWithScrollbars.h"
#include "vtkKWEntry.h"

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
    this->SpeciesLabel = NULL;
    this->SpeciesHumanButton = NULL;
    this->SpeciesMouseButton = NULL;
    this->SpeciesMacaqueButton = NULL;

    this->PopulationFrame = NULL;
    this->DiagnosisMenuButton = NULL;
    this->GenderMenuButton = NULL;
    this->HandednessMenuButton = NULL;
    this->AgeMenuButton = NULL;
    this->DiagnosticsMenuButton = NULL;

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
    this->HierarchySearchTermEntry = NULL;
    this->HierarchySearchButton = NULL;
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
    if ( this->DiagnosticsMenuButton )
      {
      this->DiagnosticsMenuButton->SetParent ( NULL );
      this->DiagnosticsMenuButton->Delete();
      this->DiagnosticsMenuButton = NULL;
      }
    if ( this->SpeciesButton )
      {
      this->SpeciesButton->SetParent ( NULL );
      this->SpeciesButton->Delete();
      this->SpeciesButton = NULL;      
      }
    if ( this->SpeciesLabel )
      {
      this->SpeciesLabel->SetParent ( NULL );
      this->SpeciesLabel->Delete();
      this->SpeciesLabel = NULL;
      }
    if ( this->SpeciesHumanButton )
      {
      this->SpeciesHumanButton->SetParent ( NULL );
      this->SpeciesHumanButton->Delete();
      this->SpeciesHumanButton = NULL;
      }
    if ( this->SpeciesMouseButton )
      {
      this->SpeciesMouseButton->SetParent ( NULL );
      this->SpeciesMouseButton->Delete();
      this->SpeciesMouseButton = NULL;
      }
    if ( this->SpeciesMacaqueButton )
      {
      this->SpeciesMacaqueButton->SetParent ( NULL );
      this->SpeciesMacaqueButton->Delete();
      this->SpeciesMacaqueButton = NULL;
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
    if ( this->HierarchySearchTermEntry )
      {
      this->HierarchySearchTermEntry->SetParent ( NULL );
      this->HierarchySearchTermEntry->Delete();
      this->HierarchySearchTermEntry = NULL;
      }
    if ( this->HierarchySearchButton )
      {
      this->HierarchySearchButton->SetParent ( NULL);
      this->HierarchySearchButton->Delete();
      this->HierarchySearchButton = NULL;
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
//  this->SearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->ClearButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectNoneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->DatabasesMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ProteinButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CellButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PopulationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HierarchySearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosticsMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: AddGUIObservers\n");
  this->LoadSceneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->SearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ClearButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->SelectAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectNoneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->DatabasesMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->GeneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->ProteinButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CellButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PopulationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HierarchySearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosticsMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
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
  else if ( (b == this->BIRNLexHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    //    this->Script ( "if { [info exists ::QA(filename) ] } {QueryAtlasLaunchBirnLexHierarchy}");
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
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->GeneFrame);
    }
  else if ( (b == this->ProteinButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->ProteinFrame);
    }
  else if ( (b == this->CellButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->CellFrame);    
    }
  else if ( (b == this->StructureButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    }
  else if ( (b == this->PopulationButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->PopulationFrame );
    }
  else if ( (b == this->SpeciesButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->SpeciesFrame );
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
    loadFrame->ExpandFrame ( );
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
    queryFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  queryFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    this->BuildQueryBuilderContextButtons ( queryFrame->GetFrame() );
    
    /*
    // test
    vtkKWFrame *tabmaker = vtkKWFrame::New();
    tabmaker->SetParent ( page );
    tabmaker->Create();
    tabmaker->SetWidth ( 20 );
    tabmaker->SetHeight ( 20 );
    tabmaker->SetBackgroundColor ( 1.0, 0.0, 0.0);
    app->Script ( "place %s -relx 2 -rely 2 -anchor nw -in %s",
                  tabmaker->GetWidgetName(),
                  queryFrame->GetFrame()->GetWidgetName() );
    tabmaker->Raise();
    tabmaker->Delete();
    */

    this->SwitchQueryFrame = vtkKWFrame::New();
    this->SwitchQueryFrame->SetParent ( queryFrame->GetFrame() );
    this->SwitchQueryFrame->Create();

    //--- these are the frames that populate the shared frame;
    //--- they are packed/unpacked depending on which context button is pushed.
    this->BuildQueryBuilderContextFrames ( this->SwitchQueryFrame );
    this->BuildSpeciesFrame();
    this->BuildPopulationFrame();
    this->BuildStructureFrame();
    this->BuildCellFrame();
    this->BuildProteinFrame();
    this->BuildGeneFrame();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    app->Script ( "pack %s -side top -fill x -expand 1", this->SwitchQueryFrame->GetWidgetName() );
    
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
    this->BIRNLexHierarchyButton->SetWidth ( 30 );
    this->BIRNLexHierarchyButton->SetText ( "Launch BIRNLex viewer");
    this->BIRNLexHierarchyButton->SetBalloonHelpString ("browse for this term in BIRNLex.");

    vtkKWLabel *hl = vtkKWLabel::New();
    hl->SetParent ( hierarchyFrame->GetFrame() );
    hl->Create();
    hl->SetText ("term: ");
    this->HierarchySearchTermEntry = vtkKWEntry::New();
    this->HierarchySearchTermEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->HierarchySearchTermEntry->Create();
    this->HierarchySearchTermEntry->SetWidth (28);
    this->HierarchySearchTermEntry->SetValue ("");
    this->HierarchySearchButton = vtkKWPushButton::New();
    this->HierarchySearchButton->SetParent ( hierarchyFrame->GetFrame() );
    this->HierarchySearchButton->Create();
    this->HierarchySearchButton->SetWidth ( 30 );
    this->HierarchySearchButton->SetText ("view in hierarchy >>");

    app->Script ("grid %s -row 0 -columnspan 2 -sticky w -pady 2", 
                  this->BIRNLexHierarchyButton->GetWidgetName ( ));
    app->Script ("grid %s -row 1 -column 0 -sticky w -padx 0 -pady 2",
                 hl->GetWidgetName() );
    app->Script ("grid %s -row 1 -column 1 -sticky w -padx 2 -pady 2",
                 this->HierarchySearchTermEntry->GetWidgetName() );
    app->Script ("grid %s -row 2 -columnspan 2 -sticky w -pady 2",
                 this->HierarchySearchButton->GetWidgetName() );

    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  hierarchyFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());
    hl->Delete();

    // ---
    // SEARCH FRAME
    vtkSlicerModuleCollapsibleFrame *searchFrame = vtkSlicerModuleCollapsibleFrame::New();
    searchFrame->SetParent ( page);
    searchFrame->Create();
    searchFrame->SetLabelText ("Search");
    searchFrame->CollapseFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  searchFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    vtkKWLabel *sl = vtkKWLabel::New();
    sl->SetParent ( searchFrame->GetFrame() );
    sl->Create();
    sl->SetText ("databases: ");
    this->DatabasesMenuButton = vtkKWMenuButton::New();
    this->DatabasesMenuButton->SetParent ( searchFrame->GetFrame() );
    this->DatabasesMenuButton->Create();
    this->DatabasesMenuButton->SetWidth (25);    
    this->BuildDatabasesMenu(this->DatabasesMenuButton->GetMenu() );
    this->SearchButton = vtkKWPushButton::New();
    this->SearchButton->SetParent ( searchFrame->GetFrame() );
    this->SearchButton->Create();
    this->SearchButton->SetWidth ( 27);
    this->SearchButton->SetText ("search >>");
    app->Script ("grid %s -row 0 -column 0 -padx 0 -pady 2 -sticky w",
                 sl->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 1 -padx 0 -pady 2 -sticky w",
                 this->DatabasesMenuButton->GetWidgetName() );    
    app->Script ("grid %s -row 1 -column 1 -padx 0 -pady 2 -sticky w",
                 this->SearchButton->GetWidgetName() );    
    sl->Delete();

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
    searchFrame->Delete();
    queryFrame->Delete ( );
    hierarchyFrame->Delete();
    saveFrame->Delete();
    loadFrame->Delete ( );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildStructureFrame()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

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
    this->SearchTermMultiColumnList->SetHeight(3);
    this->SearchTermMultiColumnList->GetWidget()->SetSelectionTypeToCell ( );
    this->SearchTermMultiColumnList->GetWidget()->MovableRowsOff ( );
    this->SearchTermMultiColumnList->GetWidget()->MovableColumnsOff ( );

    this->SearchTermMultiColumnList->GetWidget()->AddColumn ( "Use");
    this->SearchTermMultiColumnList->GetWidget()->AddColumn ( "Search Terms" );

//    this->SearchTermMultiColumnList->GetWidget()->ColumnEditableOn ( this->SelectionColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnWidth (this->SelectionColumn, 5);
    this->SearchTermMultiColumnList->GetWidget()->SetColumnAlignmentToCenter ( this->SelectionColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnResizable ( this->SelectionColumn, 0 );

    this->SearchTermMultiColumnList->GetWidget()->ColumnEditableOn ( this->SearchTermColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnWidth (this->SearchTermColumn, 42);
    this->SearchTermMultiColumnList->GetWidget()->SetColumnAlignmentToLeft (this->SearchTermColumn );
    this->SearchTermMultiColumnList->GetWidget()->SetColumnResizable ( this->SearchTermColumn, 0 );
    app->Script ( "pack %s -side top -fill x -expand true", this->SearchTermMultiColumnList->GetWidgetName() );

    // add and pack bottom button frame
    vtkKWFrame *bbframe = vtkKWFrame::New ( );
    bbframe->SetParent ( this->StructureFrame );
    bbframe->Create ( );
    app->Script ( "pack %s -side top -fill x -expand n",  bbframe->GetWidgetName() );

    // add search, clear, select all, select none buttons
    this->SelectAllButton = vtkKWPushButton::New ( );
    this->SelectAllButton->SetParent ( bbframe );
    this->SelectAllButton->Create();
    this->SelectAllButton->SetWidth ( 12 );
    this->SelectAllButton->SetText ( "Use all" );
    this->SelectAllButton->SetBalloonHelpString ( "Marck all search terms for use");

    this->SelectNoneButton = vtkKWPushButton::New ( );
    this->SelectNoneButton->SetParent ( bbframe );
    this->SelectNoneButton->Create();
    this->SelectNoneButton->SetWidth ( 12 );
    this->SelectNoneButton->SetText ( "Use none" );
    this->SelectNoneButton->SetBalloonHelpString ( "Unmark all search terms.");

    this->ClearButton = vtkKWPushButton::New ( );
    this->ClearButton->SetParent ( bbframe );
    this->ClearButton->Create();
    this->ClearButton->SetWidth ( 12 );
    this->ClearButton->SetText ( "Clear all" );
    this->ClearButton->SetBalloonHelpString ( "Clear all search terms" );

    // add search terms and delete highlighted search terms buttons
    this->AddTermButton = vtkKWPushButton::New ( );
    this->AddTermButton->SetParent ( bbframe );
    this->AddTermButton->Create();
    this->AddTermButton->SetWidth ( 12 );
    this->AddTermButton->SetText ( "Add new" );
    this->AddTermButton->SetBalloonHelpString ( "Add a new search term" );
    
    this->DeleteTermButton = vtkKWPushButton::New ( );
    this->DeleteTermButton->SetParent ( bbframe );
    this->DeleteTermButton->Create();
    this->DeleteTermButton->SetWidth ( 12 );
    this->DeleteTermButton->SetText ( "Clear selected" );
    this->DeleteTermButton->SetBalloonHelpString ( "Delete highlighted search terms" );

    // grid buttons into place
    app->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2",
                  this->AddTermButton->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky e -padx 2  -pady 2",
                  this->SelectAllButton->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 2 -sticky w -padx 2 -pady 2",
                  this->SelectNoneButton->GetWidgetName() );
    
    app->Script ( "grid %s -row 1 -column 1 -sticky e -padx 2 -pady 2",
                  this->DeleteTermButton->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 2 -sticky w -padx 2 -pady 2",
                  this->ClearButton->GetWidgetName() );
    bbframe->Delete ( );
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildSpeciesFrame()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    this->SpeciesLabel = vtkKWLabel::New();
    this->SpeciesLabel->SetParent ( this->SpeciesFrame );
    this->SpeciesLabel->Create();
    this->SpeciesLabel->SetText( "species to include: ");
    
    this->SpeciesHumanButton = vtkKWCheckButton::New();
    this->SpeciesHumanButton->SetParent ( this->SpeciesFrame);
    this->SpeciesHumanButton->Create();
    this->SpeciesHumanButton->SetText ("human");
    this->SpeciesHumanButton->SetSelectedState ( 1 );
    
    this->SpeciesMouseButton = vtkKWCheckButton::New();
    this->SpeciesMouseButton->SetParent ( this->SpeciesFrame );
    this->SpeciesMouseButton->Create();
    this->SpeciesMouseButton->SetText("mouse");
    this->SpeciesMouseButton->SetSelectedState ( 0 );

    this->SpeciesMacaqueButton = vtkKWCheckButton::New();
    this->SpeciesMacaqueButton->SetParent ( this->SpeciesFrame);
    this->SpeciesMacaqueButton->Create();
    this->SpeciesMacaqueButton->SetText ("macaque");
    this->SpeciesMacaqueButton->SetSelectedState ( 0 );

    app->Script ( "grid %s -row 0 -column 0 -sticky w", this->SpeciesLabel->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky w", this->SpeciesHumanButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1 -sticky w", this->SpeciesMouseButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 1 -sticky w", this->SpeciesMacaqueButton->GetWidgetName() );
    
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildPopulationFrame()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    this->DiagnosisMenuButton = vtkKWMenuButtonWithLabel::New();
    this->DiagnosisMenuButton->SetParent ( this->PopulationFrame );
    this->DiagnosisMenuButton->Create ();
    this->DiagnosisMenuButton->SetLabelText ( "diagnoses: ");
    this->DiagnosisMenuButton->SetLabelPositionToLeft ( );
    this->DiagnosisMenuButton->GetLabel()->SetWidth (10);
    this->DiagnosisMenuButton->GetWidget()->SetWidth (30);
    this->BuildDiagnosisMenu ( this->DiagnosisMenuButton->GetWidget()->GetMenu() );
    this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddCommand ( "close" );    

    this->GenderMenuButton = vtkKWMenuButtonWithLabel::New();
    this->GenderMenuButton->SetParent ( this->PopulationFrame );
    this->GenderMenuButton->Create ();
    this->GenderMenuButton->SetLabelText ( "gender: ");
    this->GenderMenuButton->GetWidget()->SetWidth (30);
    this->GenderMenuButton->SetLabelPositionToLeft ( );
    this->GenderMenuButton->GetLabel()->SetWidth ( 10);
    this->GenderMenuButton->GetWidget()->GetMenu()->AddRadioButton ("n/a");
    this->GenderMenuButton->GetWidget()->GetMenu()->SelectItem ("n/a");
    this->GenderMenuButton->GetWidget()->GetMenu()->AddRadioButton ("M");
    this->GenderMenuButton->GetWidget()->GetMenu()->AddRadioButton ("F");
    this->GenderMenuButton->GetWidget()->GetMenu()->AddRadioButton ("mixed");
    this->GenderMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->GenderMenuButton->GetWidget()->GetMenu()->AddCommand ( "close" );    

    this->HandednessMenuButton = vtkKWMenuButtonWithLabel::New();
    this->HandednessMenuButton->SetParent ( this->PopulationFrame );
    this->HandednessMenuButton->Create ();
    this->HandednessMenuButton->SetLabelText ( "handedness: ");
    this->HandednessMenuButton->GetWidget()->SetWidth (30);
    this->HandednessMenuButton->GetLabel()->SetWidth (10);
    this->HandednessMenuButton->SetLabelPositionToLeft ( );
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddRadioButton ("n/a");
    this->HandednessMenuButton->GetWidget()->GetMenu()->SelectItem ("n/a");
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddRadioButton ("left");
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddRadioButton ("right");
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddRadioButton ("mixed");
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddCommand ("close");

    this->AgeMenuButton = vtkKWMenuButtonWithLabel::New();
    this->AgeMenuButton->SetParent ( this->PopulationFrame );
    this->AgeMenuButton->Create ();
    this->AgeMenuButton->SetLabelText ( "age ranges: ");
    this->AgeMenuButton->GetWidget()->SetWidth (30);
    this->AgeMenuButton->GetLabel()->SetWidth (10);
    this->AgeMenuButton->SetLabelPositionToLeft ( );
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("n/a");
    this->AgeMenuButton->GetWidget()->GetMenu()->SelectItem ("n/a");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("0-10");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("10-20");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("20-30");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("30-40");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("40-50");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("50-60");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("60-70");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("70-80");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("80-90");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("90-100");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton (">100");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->AgeMenuButton->GetWidget()->GetMenu()->AddCommand ( "close");    

    this->DiagnosticsMenuButton = vtkKWMenuButtonWithLabel::New();
    this->DiagnosticsMenuButton->SetParent ( this->PopulationFrame );
    this->DiagnosticsMenuButton->Create ();
    this->DiagnosticsMenuButton->SetLabelText ( "diagnostics: ");
    this->DiagnosticsMenuButton->GetWidget()->SetWidth (30);
    this->DiagnosticsMenuButton->GetLabel()->SetWidth (10);
    this->DiagnosticsMenuButton->SetLabelPositionToLeft ( );
    this->DiagnosticsMenuButton->GetWidget()->GetMenu()->AddRadioButton ("none");
    this->DiagnosticsMenuButton->GetWidget()->GetMenu()->SelectItem ( "none");
    this->DiagnosticsMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->DiagnosticsMenuButton->GetWidget()->GetMenu()->AddCommand ( "close");

    app->Script ( "pack %s %s %s %s %s -side top -padx 5 -pady 2 -anchor nw",
                  this->DiagnosisMenuButton->GetWidgetName(),
                  this->GenderMenuButton->GetWidgetName (),
                  this->HandednessMenuButton->GetWidgetName(),
                  this->AgeMenuButton->GetWidgetName(),
                  this->DiagnosticsMenuButton->GetWidgetName() );

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
void vtkQueryAtlasGUI::UnpackQueryBuilderContextFrames ( )
{
    this->Script ( "pack forget %s", this->GeneFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->ProteinFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->CellFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->StructureFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->PopulationFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->SpeciesFrame->GetWidgetName() );
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::PackQueryBuilderContextFrame ( vtkKWFrame *f )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  app->Script ( "pack %s -side top -anchor nw -expand 0 -fill x", f->GetWidgetName( ));
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextFrames ( vtkKWFrame *parent )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    
    this->StructureFrame = vtkKWFrame::New();
    this->StructureFrame->SetParent ( parent );
    this->StructureFrame->Create();
    
    this->PopulationFrame = vtkKWFrame::New();
    this->PopulationFrame->SetParent ( parent );
    this->PopulationFrame->Create();

    this->SpeciesFrame = vtkKWFrame::New();
    this->SpeciesFrame->SetParent ( parent );
    this->SpeciesFrame->Create();

    this->CellFrame = vtkKWFrame::New();
    this->CellFrame->SetParent ( parent );
    this->CellFrame->Create();

    this->ProteinFrame = vtkKWFrame::New();
    this->ProteinFrame->SetParent ( parent );
    this->ProteinFrame->Create();

    this->GeneFrame = vtkKWFrame::New();
    this->GeneFrame->SetParent ( parent );
    this->GeneFrame->Create();
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextButtons ( vtkKWFrame *parent )
{
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();
  this->Script ("pack %s -side top -anchor nw -fill none", f->GetWidgetName() );

  this->GeneButton = vtkKWPushButton::New();
  this->GeneButton->SetParent( f );
  this->GeneButton->Create();
  this->GeneButton->SetWidth (6 );
  this->GeneButton->SetText ( "gene" );

  this->ProteinButton = vtkKWPushButton::New();
  this->ProteinButton->SetParent ( f );
  this->ProteinButton->Create();
  this->ProteinButton->SetWidth ( 6 );
  this->ProteinButton->SetText ( "protein" );
    
  this->CellButton = vtkKWPushButton::New();
  this->CellButton->SetParent ( f );
  this->CellButton->Create();
  this->CellButton->SetWidth ( 6 );
  this->CellButton->SetText ( "cell ");
    
  this->StructureButton = vtkKWPushButton::New();
  this->StructureButton->SetParent ( f );
  this->StructureButton->Create();
  this->StructureButton->SetWidth ( 6 );
  this->StructureButton->SetText ( "brain");
    
  this->PopulationButton = vtkKWPushButton::New();
  this->PopulationButton->SetParent ( f );
  this->PopulationButton->Create();
  this->PopulationButton->SetWidth ( 6 );
  this->PopulationButton->SetText ( "group");
    
  this->SpeciesButton = vtkKWPushButton::New();
  this->SpeciesButton->SetParent ( f );
  this->SpeciesButton->Create();
  this->SpeciesButton->SetWidth ( 6 );
  this->SpeciesButton->SetText ( "species");    

  this->Script ( "pack %s %s %s %s %s %s -anchor nw -side left -fill none -padx 2 -pady 2",
                 this->GeneButton->GetWidgetName(),
                 this->ProteinButton->GetWidgetName(),
                 this->CellButton->GetWidgetName(),
                 this->StructureButton->GetWidgetName(),
                 this->PopulationButton->GetWidgetName(),
                 this->SpeciesButton->GetWidgetName() );
  f->Delete();
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildDatabasesMenu ( vtkKWMenu *m )
{
 this->DatabasesMenuButton->GetMenu()->AddCheckButton ("google");
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("wikipedia");
  this->DatabasesMenuButton->GetMenu()->AddSeparator();
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("pubmed");
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("jneurosci");
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("ibvd");
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("braininfo");  
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("metasearch");
  this->DatabasesMenuButton->GetMenu()->AddSeparator();
  this->DatabasesMenuButton->GetMenu()->AddCheckButton ("entrez");
  this->DatabasesMenuButton->GetMenu()->AddCommand ( "close");
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildDiagnosisMenu( vtkKWMenu *m )
{
  m->AddRadioButton ( "Normal" );
  m->SelectItem ("Normal");
  m->AddRadioButton ("Alzheimer's Disease");
  m->AddRadioButton ("Schizophrenia");
  m->AddRadioButton  ("Alcoholism");
  m->AddRadioButton  ("Dementia");
  m->AddRadioButton  ("Autism");
  m->AddRadioButton ( "Depression");
  m->AddRadioButton ("Traumatic Brain Injury");
  m->AddRadioButton ("OCD");
  m->AddRadioButton ("ADHD");
  m->AddRadioButton ("Epilepsy");
  m->AddRadioButton ("PDAPP Transgenic");
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
