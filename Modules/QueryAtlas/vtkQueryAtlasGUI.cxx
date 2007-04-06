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
#include "vtkKWListBox.h"
#include "vtkKWListBoxWithScrollbars.h"

#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkQueryAtlasGUI.h"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasGUI );
vtkCxxRevisionMacro ( vtkQueryAtlasGUI, "$Revision: 1.0 $");


#define _r 0.85
#define _g 0.85
#define _b 0.95

//---------------------------------------------------------------------------
vtkQueryAtlasGUI::vtkQueryAtlasGUI ( )
{
    this->Logic = NULL;
    this->LoadSceneButton = NULL;

    this->GeneButton = NULL;
    this->MiscButton = NULL;
    this->CellButton = NULL;
    this->StructureButton = NULL;
    this->PopulationButton = NULL;
    this->SpeciesButton = NULL;
    this->SwitchQueryFrame = NULL;

    this->GeneFrame = NULL;
    this->CellFrame = NULL;
    this->MiscFrame = NULL;

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
    this->StructureUseAllButton = NULL;
    this->StructureUseNoneButton = NULL;
    this->StructureClearAllButton = NULL;
    this->StructureAddTermButton = NULL;
    this->StructureClearTermButton = NULL;    
    this->StructureMultiColumnList = NULL;

    this->SearchButton = NULL;
    this->DatabasesMenuButton = NULL;

    this->CurrentResultsList = NULL;
    this->PastResultsList = NULL;
    this->DeleteAllCurrentResultsButton = NULL;
    this->DeleteCurrentResultButton = NULL;
    this->DeleteAllPastResultsButton = NULL;
    this->DeletePastResultButton = NULL;
    this->SaveCurrentResultsButton = NULL;
    this->SavePastResultsButton = NULL;

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
    if ( this->MiscButton )
      {
      this->MiscButton->SetParent ( NULL );
      this->MiscButton->Delete();      
      this->MiscButton = NULL;
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
    if ( this->MiscFrame)
      {
      this->MiscFrame->SetParent ( NULL );
      this->MiscFrame->Delete();
      this->MiscFrame = NULL;
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
    if ( this->StructureClearAllButton )
      {
      this->StructureClearAllButton->SetParent ( NULL );
      this->StructureClearAllButton->Delete ( );
      this->StructureClearAllButton = NULL;
      }
    if ( this->StructureAddTermButton )
      {
      this->StructureAddTermButton->SetParent ( NULL );
      this->StructureAddTermButton->Delete ( );
      this->StructureAddTermButton = NULL;
      }
    if ( this->StructureClearTermButton )
      {
      this->StructureClearTermButton->SetParent ( NULL );
      this->StructureClearTermButton->Delete ( );
      this->StructureClearTermButton = NULL;
      }

    if ( this->StructureUseAllButton )
      {
      this->StructureUseAllButton->SetParent ( NULL );
      this->StructureUseAllButton->Delete ( );
      this->StructureUseAllButton = NULL;
      }
    if ( this->StructureUseNoneButton )
      {
      this->StructureUseNoneButton->SetParent ( NULL );
      this->StructureUseNoneButton->Delete ( );
      this->StructureUseNoneButton = NULL;
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
    if ( this->StructureMultiColumnList )
      {
      this->StructureMultiColumnList->SetParent ( NULL );
      this->StructureMultiColumnList->Delete ( );
      this->StructureMultiColumnList = NULL;
      }
    if ( this->DatabasesMenuButton )
      {
      this->DatabasesMenuButton->SetParent ( NULL );
      this->DatabasesMenuButton->Delete ( );
      this->DatabasesMenuButton = NULL;      
      }

    if ( this->CurrentResultsList )
      {
      this->CurrentResultsList->SetParent(NULL);
      this->CurrentResultsList->Delete();
      this->CurrentResultsList = NULL;
      }
    if ( this->PastResultsList )
      {
      this->PastResultsList->SetParent(NULL);
      this->PastResultsList->Delete();
      this->PastResultsList = NULL;
      }
    if ( this->DeleteCurrentResultButton )
      {
      this->DeleteCurrentResultButton->SetParent(NULL);
      this->DeleteCurrentResultButton->Delete();
      this->DeleteCurrentResultButton = NULL;
      }
    if ( this->DeleteAllCurrentResultsButton )
      {
      this->DeleteAllCurrentResultsButton->SetParent(NULL);
      this->DeleteAllCurrentResultsButton->Delete();
      this->DeleteAllCurrentResultsButton = NULL;
      }
    if ( this->DeletePastResultButton )
      {
      this->DeletePastResultButton->SetParent(NULL);
      this->DeletePastResultButton->Delete();
      this->DeletePastResultButton = NULL;
      }
    if ( this->DeleteAllPastResultsButton )
      {
      this->DeleteAllPastResultsButton->SetParent(NULL);
      this->DeleteAllPastResultsButton->Delete();
      this->DeleteAllPastResultsButton = NULL;
      }
    if ( this->SaveCurrentResultsButton )
      {
      this->SaveCurrentResultsButton->SetParent(NULL);
      this->SaveCurrentResultsButton->Delete();
      this->SaveCurrentResultsButton = NULL;
      }
    if ( this->SavePastResultsButton )
      {
      this->SavePastResultsButton->SetParent(NULL);
      this->SavePastResultsButton->Delete();
      this->SavePastResultsButton = NULL;
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

  this->StructureButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureClearAllButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureAddTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureClearTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->StructureUseAllButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureUseNoneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->GeneButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MiscButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CellButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SpeciesButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesHumanButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMouseButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMacaqueButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->BIRNLexHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HierarchySearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HierarchySearchTermEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->PopulationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosticsMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->DatabasesMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->SearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->SaveCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SavePastResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeletePastResultButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllPastResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteCurrentResultButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CurrentResultsList->GetWidget()->RemoveObservers(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PastResultsList->GetWidget()->RemoveObservers(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: AddGUIObservers\n");
  this->LoadSceneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->StructureButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureClearAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureAddTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureClearTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->StructureUseAllButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureUseNoneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->GeneButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->MiscButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CellButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SpeciesButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesHumanButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMouseButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMacaqueButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->BIRNLexHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HierarchySearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HierarchySearchTermEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->PopulationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosticsMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->DatabasesMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->SearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->SaveCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SavePastResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeletePastResultButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllPastResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteCurrentResultButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CurrentResultsList->GetWidget()->AddObserver(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->PastResultsList->GetWidget()->AddObserver(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // nothing to do here yet...
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
  const char *context;
  
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
  else if ( (b == this->StructureClearAllButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->DeleteAllSearchTerms( context );
    }
  else if ( (b == this->StructureAddTermButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->AddNewSearchTerm( context );
    }
  else if ( (b == this->StructureClearTermButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->DeleteSelectedSearchTerms (context );
    }
  else if ( (b == this->StructureUseAllButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->SelectAllSearchTerms ( context );
    }
  else if ( (b == this->StructureUseNoneButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->DeselectAllSearchTerms ( context );
    }  
  else if ( (b == this->MiscButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->MiscFrame);
    this->ColorCodeContextButtons ( this->MiscButton );
    }
  else if ( (b == this->GeneButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->GeneFrame);
    this->ColorCodeContextButtons ( this->GeneButton );
    }
  else if ( (b == this->CellButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->CellFrame);    
    this->ColorCodeContextButtons ( this->CellButton );
    }
  else if ( (b == this->StructureButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    this->ColorCodeContextButtons ( this->StructureButton );
    }
  else if ( (b == this->PopulationButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->PopulationFrame );
    this->ColorCodeContextButtons ( this->PopulationButton );
    }
  else if ( (b == this->SpeciesButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->SpeciesFrame );
    this->ColorCodeContextButtons ( this->SpeciesButton );
    }

//    if ( (m == this->DatabasesMenuButton->GetMenu() ) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
//    {
//    }
    return;
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ColorCodeContextButtons ( vtkKWPushButton *b )
{
  this->MiscButton->SetBackgroundColor ( _r, _g, _b );
  this->GeneButton->SetBackgroundColor ( _r, _g, _b );
  this->CellButton->SetBackgroundColor ( _r, _g, _b );
  this->StructureButton->SetBackgroundColor ( _r, _g, _b );
  this->PopulationButton->SetBackgroundColor ( _r, _g, _b );
  this->SpeciesButton->SetBackgroundColor ( _r, _g, _b );
  b->SetBackgroundColor (1.0, 1.0, 1.0);
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
    this->BuildMiscFrame();
    this->BuildGeneFrame();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    app->Script ( "pack %s -side top -fill x -expand 1", this->SwitchQueryFrame->GetWidgetName() );
//    this->Script ( "place %s -relx 0 -rely 0 -anchor nw", this->SwitchQueryFrame->GetWidgetName());
    this->ColorCodeContextButtons ( this->StructureButton );
    
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

    vtkKWFrame *curF = vtkKWFrame::New();
    curF->SetParent ( resultsFrame->GetFrame() );
    curF->Create();
    vtkKWLabel *curL = vtkKWLabel::New();
    curL->SetParent ( curF );
    curL->Create();
    curL->SetWidth ( 45 );
    curL->SetText ( "Results of current search" );
    curL->SetBackgroundColor ( _r, _g, _b);
    this->CurrentResultsList = vtkKWListBoxWithScrollbars::New();
    this->CurrentResultsList->SetParent ( curF );
    this->CurrentResultsList->Create();
    this->CurrentResultsList->GetWidget()->SetWidth ( 45 );
    this->CurrentResultsList->GetWidget()->SetHeight (4 );
    this->CurrentResultsList->HorizontalScrollbarVisibilityOn();
    this->CurrentResultsList->VerticalScrollbarVisibilityOn();
    this->DeleteCurrentResultButton = vtkKWPushButton::New();
    this->DeleteCurrentResultButton->SetParent (curF);
    this->DeleteCurrentResultButton->Create();
    this->DeleteCurrentResultButton->SetText ( "Delete selected");
    this->DeleteAllCurrentResultsButton = vtkKWPushButton::New();
    this->DeleteAllCurrentResultsButton->SetParent (curF);
    this->DeleteAllCurrentResultsButton->Create();
    this->DeleteAllCurrentResultsButton->SetText("Delete all");
    this->SaveCurrentResultsButton = vtkKWPushButton::New();
    this->SaveCurrentResultsButton->SetParent (curF);
    this->SaveCurrentResultsButton->Create();    
    this->SaveCurrentResultsButton->SetText ("Reserve all");

    vtkKWFrame *pastF = vtkKWFrame::New();
    pastF->SetParent ( resultsFrame->GetFrame() );
    pastF->Create();
    vtkKWLabel *pastL = vtkKWLabel::New();
    pastL->SetParent ( pastF );
    pastL->Create();
    pastL->SetWidth ( 45 );
    pastL->SetText ( "Reserved search result bundles" );
    pastL->SetBackgroundColor ( 0.85, 0.85, 0.95 );
    this->PastResultsList = vtkKWListBoxWithScrollbars::New();
    this->PastResultsList->SetParent ( pastF );
    this->PastResultsList->Create();
    this->PastResultsList->GetWidget()->SetWidth ( 45 );
    this->PastResultsList->GetWidget()->SetHeight ( 4 );
    this->PastResultsList->HorizontalScrollbarVisibilityOn();
    this->PastResultsList->VerticalScrollbarVisibilityOn();
    this->DeletePastResultButton = vtkKWPushButton::New();
    this->DeletePastResultButton->SetParent (pastF);
    this->DeletePastResultButton->Create();
    this->DeletePastResultButton->SetText ("Delete selected");
    this->DeleteAllPastResultsButton = vtkKWPushButton::New();
    this->DeleteAllPastResultsButton->SetParent (pastF);
    this->DeleteAllPastResultsButton->Create();
    this->DeleteAllPastResultsButton->SetText ("Delete all");
    this->SavePastResultsButton = vtkKWPushButton::New();
    this->SavePastResultsButton->SetParent (pastF);
    this->SavePastResultsButton->Create();    
    this->SavePastResultsButton->SetText ("Save to file");

    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand true", curF->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand true", pastF->GetWidgetName() );

    app->Script ("grid %s -row 0 -columnspan 3 -sticky ew", curL->GetWidgetName() );
    app->Script ("grid %s -row 1 -columnspan 3 -sticky ew", this->CurrentResultsList->GetWidgetName() );
    app->Script ("grid %s -row 2 -column 0 -sticky ew -pady 4", this->DeleteCurrentResultButton->GetWidgetName() );    
    app->Script ("grid %s -row 2 -column 1 -sticky ew -pady 4 -padx 4", this->DeleteAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 2 -column 2 -sticky ew -pady 4", this->SaveCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 0 -weight 1", this->DeleteCurrentResultButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 1 -weight 1", this->DeleteAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 2 -weight 1", this->SaveCurrentResultsButton->GetWidgetName() );    

    app->Script ("grid %s -columnspan 3 -row 0 -sticky ew", pastL->GetWidgetName() );
    app->Script ("grid %s -columnspan 3 -row 1 -sticky ew", this->PastResultsList->GetWidgetName() );
    app->Script ("grid %s -row 2 -column 0 -sticky ew -pady 4", this->DeletePastResultButton->GetWidgetName() );    
    app->Script ("grid %s -row 2 -column 1 -sticky ew -pady 4 -padx 4", this->DeleteAllPastResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 2 -column 2 -sticky ew -pady 4", this->SavePastResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 0 -weight 1", this->DeletePastResultButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 1 -weight 1", this->DeleteAllPastResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 2 -weight 1", this->SavePastResultsButton->GetWidgetName() );    

    curL->Delete();
    pastL->Delete();
    curF->Delete();
    pastF->Delete();

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

    
    // deleting frame widgets
    displayFrame->Delete ( );
    resultsFrame->Delete ( );
    searchFrame->Delete();
    queryFrame->Delete ( );
    hierarchyFrame->Delete();
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
    this->StructureMultiColumnList = vtkKWMultiColumnListWithScrollbars::New ( );
    this->StructureMultiColumnList->SetParent ( this->StructureFrame );
    this->StructureMultiColumnList->Create ( );
//    this->StructureMultiColumnList->SetBalloonHelpString ("Use the 'Add new term' button to create a new row in this widget and then click in the 'Search term' column to enter your new search term. Select or disable the term's use in your search by selecting the checkbox next to it. The space for one search term is created by default." );
    this->StructureMultiColumnList->SetWidth(0);
    this->StructureMultiColumnList->SetHeight(3);
    this->StructureMultiColumnList->GetWidget()->SetSelectionTypeToCell ( );
    this->StructureMultiColumnList->GetWidget()->MovableRowsOff ( );
    this->StructureMultiColumnList->GetWidget()->MovableColumnsOff ( );

    this->StructureMultiColumnList->GetWidget()->AddColumn ( "Use");
    this->StructureMultiColumnList->GetWidget()->AddColumn ( "Search Terms" );

//    this->StructureMultiColumnList->GetWidget()->ColumnEditableOn ( this->SelectionColumn );
    this->StructureMultiColumnList->GetWidget()->SetColumnWidth (this->SelectionColumn, 5);
    this->StructureMultiColumnList->GetWidget()->SetColumnAlignmentToCenter ( this->SelectionColumn );
    this->StructureMultiColumnList->GetWidget()->SetColumnResizable ( this->SelectionColumn, 0 );

    this->StructureMultiColumnList->GetWidget()->ColumnEditableOn ( this->SearchTermColumn );
    this->StructureMultiColumnList->GetWidget()->SetColumnWidth (this->SearchTermColumn, 42);
    this->StructureMultiColumnList->GetWidget()->SetColumnAlignmentToLeft (this->SearchTermColumn );
    this->StructureMultiColumnList->GetWidget()->SetColumnResizable ( this->SearchTermColumn, 0 );
    app->Script ( "pack %s -side top -fill x -expand true", this->StructureMultiColumnList->GetWidgetName() );

    // add and pack bottom button frame
    vtkKWFrame *bbframe = vtkKWFrame::New ( );
    bbframe->SetParent ( this->StructureFrame );
    bbframe->Create ( );
    app->Script ( "pack %s -side top -fill x -expand n",  bbframe->GetWidgetName() );

    // add search, clear, select all, select none buttons
    this->StructureUseAllButton = vtkKWPushButton::New ( );
    this->StructureUseAllButton->SetParent ( bbframe );
    this->StructureUseAllButton->Create();
    this->StructureUseAllButton->SetWidth ( 12 );
    this->StructureUseAllButton->SetText ( "Use all" );
    this->StructureUseAllButton->SetBalloonHelpString ( "Marck all search terms for use");

    this->StructureUseNoneButton = vtkKWPushButton::New ( );
    this->StructureUseNoneButton->SetParent ( bbframe );
    this->StructureUseNoneButton->Create();
    this->StructureUseNoneButton->SetWidth ( 12 );
    this->StructureUseNoneButton->SetText ( "Use none" );
    this->StructureUseNoneButton->SetBalloonHelpString ( "Unmark all search terms.");

    this->StructureClearAllButton = vtkKWPushButton::New ( );
    this->StructureClearAllButton->SetParent ( bbframe );
    this->StructureClearAllButton->Create();
    this->StructureClearAllButton->SetWidth ( 12 );
    this->StructureClearAllButton->SetText ( "Clear all" );
    this->StructureClearAllButton->SetBalloonHelpString ( "Clear all search terms" );

    // add search terms and delete highlighted search terms buttons
    this->StructureAddTermButton = vtkKWPushButton::New ( );
    this->StructureAddTermButton->SetParent ( bbframe );
    this->StructureAddTermButton->Create();
    this->StructureAddTermButton->SetWidth ( 12 );
    this->StructureAddTermButton->SetText ( "Add new" );
    this->StructureAddTermButton->SetBalloonHelpString ( "Add a new search term" );
    
    this->StructureClearTermButton = vtkKWPushButton::New ( );
    this->StructureClearTermButton->SetParent ( bbframe );
    this->StructureClearTermButton->Create();
    this->StructureClearTermButton->SetWidth ( 12 );
    this->StructureClearTermButton->SetText ( "Clear selected" );
    this->StructureClearTermButton->SetBalloonHelpString ( "Delete highlighted search terms" );

    // grid buttons into place
    app->Script ( "grid %s -row 0 -column 0 -sticky e -padx 2 -pady 2",
                  this->StructureAddTermButton->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky e -padx 2  -pady 2",
                  this->StructureUseAllButton->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 2 -sticky w -padx 2 -pady 2",
                  this->StructureUseNoneButton->GetWidgetName() );
    
    app->Script ( "grid %s -row 1 -column 1 -sticky e -padx 2 -pady 2",
                  this->StructureClearTermButton->GetWidgetName());
    app->Script ( "grid %s -row 1 -column 2 -sticky w -padx 2 -pady 2",
                  this->StructureClearAllButton->GetWidgetName() );
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
void vtkQueryAtlasGUI::BuildMiscFrame()
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
    this->Script ( "pack forget %s", this->MiscFrame->GetWidgetName() );
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

    this->MiscFrame = vtkKWFrame::New();
    this->MiscFrame->SetParent ( parent );
    this->MiscFrame->Create();

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

  this->MiscButton = vtkKWPushButton::New();
  this->MiscButton->SetParent ( f );
  this->MiscButton->Create();
  this->MiscButton->SetWidth ( 6 );
  this->MiscButton->SetText ( "misc" );

  this->GeneButton = vtkKWPushButton::New();
  this->GeneButton->SetParent( f );
  this->GeneButton->Create();
  this->GeneButton->SetWidth (6 );
  this->GeneButton->SetText ( "gene" );
    
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
                 this->MiscButton->GetWidgetName(),
                 this->GeneButton->GetWidgetName(),
                 this->CellButton->GetWidgetName(),
                 this->StructureButton->GetWidgetName(),
                 this->PopulationButton->GetWidgetName(),
                 this->SpeciesButton->GetWidgetName() );

/*
  // test
  vtkKWLabel *tabmaker = vtkKWLabel::New();
  tabmaker->SetParent ( f );
  tabmaker->Create();
  tabmaker->SetWidth ( 20 );
  tabmaker->SetHeight ( 20 );
  this->Script ( "place %s -relx 2 -rely 2 -anchor nw -in %s",
                tabmaker->GetWidgetName(), f);
  tabmaker->Raise();
  tabmaker->Delete();
*/
  f->Delete();
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildDatabasesMenu ( vtkKWMenu *m )
{
  m->AddRadioButton ("google");
  m->SelectItem ("google");
  m->AddRadioButton ("wikipedia");
  m->AddSeparator();
  m->AddRadioButton ("pubmed");
  m->AddRadioButton ("jneurosci");
  m->AddRadioButton ("ibvd");
  m->AddRadioButton ("braininfo");  
  m->AddRadioButton ("metasearch");
  m->AddSeparator();
  m->AddRadioButton ("entrez");
  m->AddSeparator();
  m->AddCommand ( "close");
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
void vtkQueryAtlasGUI::SelectAllSearchTerms ( const char *c )
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Select All SearchTerms event. \n");  
  int numrows = this->StructureMultiColumnList->GetWidget()->GetNumberOfRows();
  int i;
  for ( i = 0; i < numrows; i++ )
    {
    this->StructureMultiColumnList->GetWidget()->SetCellText ( i, this->SelectionColumn, "1" );
    }
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeselectAllSearchTerms ( const char *c)
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Deselect All SearchTerms event. \n");  
  int numrows = this->StructureMultiColumnList->GetWidget()->GetNumberOfRows();
  int i;
  for ( i = 0; i < numrows; i++ )
    {
    this->StructureMultiColumnList->GetWidget()->SetCellText ( i, this->SelectionColumn, "0" );
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteAllSearchTerms ( const char *c)
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Clear All SearchTerms event. \n");
  int numrows = this->StructureMultiColumnList->GetWidget()->GetNumberOfRows();
  // remove each row
  this->StructureMultiColumnList->GetWidget()->DeleteAllRows();
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddNewSearchTerm ( const char *c)
{
    // default search terms in list
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Adding New SearchTerms event. \n");
    int i = this->StructureMultiColumnList->GetWidget()->GetNumberOfRows();
    this->StructureMultiColumnList->GetWidget()->InsertCellTextAsInt ( i, this->SelectionColumn, 0 );
    this->StructureMultiColumnList->GetWidget()->SetCellWindowCommandToCheckButton (i, this->SelectionColumn );
    this->StructureMultiColumnList->GetWidget()->InsertCellText (i, this->SearchTermColumn, "edit search term here" );
    this->StructureMultiColumnList->GetWidget()->SetColumnEditWindowToEntry (this->SearchTermColumn);
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteSelectedSearchTerms ( const char *c)
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
  int numRows = this->StructureMultiColumnList->GetWidget()->GetNumberOfSelectedRows();
  if (numRows == 1)
    {
    int row[1];
    this->StructureMultiColumnList->GetWidget()->GetSelectedRows(row);

    if (confirmDeleteFlag)
      {
      // confirm that really want to remove this term
      std::cout << "Deleting search term " << row[0] << endl;
      }
            
    // then remove that row by index
    this->StructureMultiColumnList->GetWidget()->DeleteRow ( row[0] );
    }
  else
    {
    vtkErrorMacro (<< "Selected rows (" << numRows << ") not 1, just pick one to delete for now\n");
    return;
    }
}
