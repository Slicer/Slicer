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
#include "vtkKWEntryWithLabel.h"
#include "vtkKWListBox.h"
#include "vtkKWListBoxWithScrollbars.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWLoadSaveButton.h"

#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerApplication.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkSlicerToolbarIcons.h"
#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerSliceControllerWidget.h"
#include "vtkSlicerSliceGUI.h"
#include "vtkSlicerToolbarGUI.h"
#include "vtkQueryAtlasGUI.h"
#include "vtkQueryAtlasUseSearchTermWidget.h"
#include "vtkQueryAtlasSearchTermWidget.h"
#include "vtkSlicerPopUpHelpWidget.h"

// for path manipulation
#include "itksys/SystemTools.hxx"

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasGUI );
vtkCxxRevisionMacro ( vtkQueryAtlasGUI, "$Revision: 1.0 $");


#define _br 0.85
#define _bg 0.85
#define _bb 0.95

#define _fr 0.5
#define _fg 0.5
#define _fb 0.5


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::vtkQueryAtlasGUI ( )
{
    this->Logic = NULL;

    this->CollaboratorIcons = NULL;
    this->QueryAtlasIcons = NULL;
    this->AnnotationVisibility = 1;
    this->ModelVisibility = 1;
    
    //---
    // master category switch
    //---
    this->SubStructureButton = NULL;
    this->StructureButton = NULL;
    this->PopulationButton = NULL;
    this->SpeciesButton = NULL;
    this->SwitchQueryFrame = NULL;

    //---
    // query builder substructure frame
    //---    
    this->SubStructureFrame = NULL;
    this->SubStructureListWidget = NULL;

    //---
    // query builder species frame
    //---    
    this->SpeciesFrame = NULL;
    this->SpeciesLabel = NULL;
    this->SpeciesNoneButton = NULL;
    this->SpeciesHumanButton = NULL;
    this->SpeciesMouseButton = NULL;
    this->SpeciesMacaqueButton = NULL;

    //---
    // query builder popuation frame
    //---
    this->PopulationFrame = NULL;
    this->DiagnosisMenuButton = NULL;
    this->GenderMenuButton = NULL;
    this->HandednessMenuButton = NULL;
    this->AgeMenuButton = NULL;
    this->AddDiagnosisEntry = NULL;

    //---
    // query builder structure frame
    //---
    this->StructureFrame = NULL;
    this->StructureMenuButton = NULL;
    this->StructureListWidget = NULL;

    //---
    // annotation frame
    //---    
    this->AnnotationVisibilityButton = NULL;
    this->AnnotationNomenclatureMenuButton = NULL;
    this->ModelVisibilityButton = NULL;
    
    //---
    // search frame
    //---    
    this->SearchButton = NULL;
    this->DatabasesMenuButton = NULL;
    this->ResultsWithAnyButton = NULL;
    this->ResultsWithAllButton = NULL;
    this->ResultsWithExactButton = NULL;    

    //---
    // results frame
    //---
    this->CurrentResultsList = NULL;
    this->PastResultsList = NULL;
    this->DeleteAllCurrentResultsButton = NULL;
    this->DeleteCurrentResultButton = NULL;
    this->DeleteAllPastResultsButton = NULL;
    this->DeletePastResultButton = NULL;
    this->SaveCurrentResultsButton = NULL;
    this->SaveCurrentSelectedResultsButton = NULL;
    this->SavePastResultsButton = NULL;
    this->LoadURIsButton = NULL;

    this->NumberOfColumns = 2;

    //---
    // ontology frame
    //---
    this->LocalSearchTermEntry = NULL;
    this->SynonymsMenuButton = NULL;
    this->BIRNLexEntry = NULL;
    this->BIRNLexIDEntry = NULL;
    this->NeuroNamesEntry = NULL;
    this->NeuroNamesIDEntry = NULL;
    this->UMLSCIDEntry = NULL;
    this->AddLocalTermButton = NULL;
    this->AddSynonymButton = NULL;
    this->AddBIRNLexStringButton = NULL;
    this->AddBIRNLexIDButton = NULL;
    this->AddNeuroNamesStringButton = NULL;
    this->AddNeuroNamesIDButton = NULL;
    this->AddUMLSCIDButton = NULL;
    this->BIRNLexHierarchyButton = NULL;
    this->NeuroNamesHierarchyButton = NULL;
    this->UMLSHierarchyButton = NULL;
    this->SavedTerms = NULL;

    //---
    // load frame
    //---    
    this->FIPSFSButton = NULL;
    this->FIPSFSFrame = NULL;
    this->QdecButton = NULL;
    this->QdecFrame = NULL;
    this->FSbrainSelector = NULL;
    this->FSoverlaySelector = NULL;
    this->FSmodelSelector = NULL;
    this->FStransformSelector = NULL;
    this->QdecModelSelector = NULL;
}


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::~vtkQueryAtlasGUI ( )
{

    this->SetModuleLogic ( NULL );
    //---
    // help and acknowledgment frame
    //---
    if ( this->CollaboratorIcons )
      {
      this->CollaboratorIcons->Delete();
      this->CollaboratorIcons = NULL;
      }
    if ( this->QueryAtlasIcons )
      {
      this->QueryAtlasIcons->Delete();
      this->QueryAtlasIcons = NULL;
      }

    //---
    // load frame
    //---
    if ( this->FIPSFSButton )
      {
      this->FIPSFSButton->SetParent ( NULL );
      this->FIPSFSButton->Delete();
      this->FIPSFSButton = NULL;
      }
    if ( this->FIPSFSFrame )
      {
      this->FIPSFSFrame->SetParent ( NULL );
      this->FIPSFSFrame->Delete();
      this->FIPSFSFrame = NULL;      
      }
    if ( this->QdecButton )
      {
      this->QdecButton->SetParent ( NULL );
      this->QdecButton->Delete();      
      this->QdecButton = NULL;      
      }
    if ( this->QdecFrame )
      {
      this->QdecFrame->SetParent ( NULL );
      this->QdecFrame->Delete();
      this->QdecFrame = NULL;      
      }
    if ( this->FSbrainSelector )
      {
      this->FSbrainSelector->SetParent ( NULL );
      this->FSbrainSelector->Delete();
      this->FSbrainSelector = NULL;
      }
    if ( this->FSoverlaySelector )
      {
      this->FSoverlaySelector->SetParent ( NULL );
      this->FSoverlaySelector->Delete();
      this->FSoverlaySelector = NULL;
      }
    if ( this->FSmodelSelector )
      {
      this->FSmodelSelector->SetParent ( NULL );
      this->FSmodelSelector->Delete();
      this->FSmodelSelector = NULL;
      }
    if ( this->FStransformSelector )
      {
      this->FStransformSelector->SetParent ( NULL );
      this->FStransformSelector->Delete();
      this->FStransformSelector = NULL;
      }
    if ( this->QdecModelSelector )
      {
      this->QdecModelSelector->SetParent ( NULL );
      this->QdecModelSelector->Delete();
      this->QdecModelSelector = NULL;
      }

    //---
    // annotation frame
    //---
    if ( this->ModelVisibilityButton )
      {
      this->ModelVisibilityButton->SetParent ( NULL );
      this->ModelVisibilityButton->Delete();
      this->ModelVisibilityButton = NULL;
      }
    if ( this->AnnotationVisibilityButton )
      {
      this->AnnotationVisibilityButton->SetParent ( NULL );
      this->AnnotationVisibilityButton->Delete();
      this->AnnotationVisibilityButton = NULL;
      }
    if ( this->AnnotationNomenclatureMenuButton )
      {
      this->AnnotationNomenclatureMenuButton->SetParent ( NULL );
      this->AnnotationNomenclatureMenuButton->Delete();
      this->AnnotationNomenclatureMenuButton = NULL;      
      }
    
    //---
    // query builder frame
    //---
    if ( this->SwitchQueryFrame)
      {
      this->SwitchQueryFrame->SetParent ( NULL );
      this->SwitchQueryFrame->Delete();
      this->SwitchQueryFrame = NULL;
      }
    if ( this->SubStructureButton )
      {
      this->SubStructureButton->SetParent ( NULL );
      this->SubStructureButton->Delete();
      this->SubStructureButton = NULL;   
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

    //---
    // query builder population panel
    //---
    if ( this->PopulationFrame )
      {
      this->PopulationFrame->SetParent ( NULL );
      this->PopulationFrame->Delete();
      this->PopulationFrame = NULL;
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
    if ( this->AddDiagnosisEntry )
      {
      this->AddDiagnosisEntry->SetParent ( NULL );
      this->AddDiagnosisEntry->Delete();
      this->AddDiagnosisEntry = NULL;
      }
    //---
    // query builder species panel
    //---
    if ( this->SpeciesFrame)
      {
      this->SpeciesFrame->SetParent ( NULL );
      this->SpeciesFrame->Delete();
      this->SpeciesFrame = NULL;
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
    if ( this->SpeciesNoneButton )
      {
      this->SpeciesNoneButton->SetParent ( NULL );
      this->SpeciesNoneButton->Delete();
      this->SpeciesNoneButton = NULL;      
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


    //---
    // search panel
    //---
    if ( this->SearchButton )
      {
      this->SearchButton->SetParent ( NULL );
      this->SearchButton->Delete ( );
      this->SearchButton = NULL;
      }
    if ( this->DatabasesMenuButton )
      {
      this->DatabasesMenuButton->SetParent ( NULL );
      this->DatabasesMenuButton->Delete ( );
      this->DatabasesMenuButton = NULL;      
      }
    if ( this->ResultsWithAnyButton )
      {
      this->ResultsWithAnyButton->SetParent ( NULL );
      this->ResultsWithAnyButton->Delete();
      this->ResultsWithAnyButton = NULL;
      }
    if ( this->ResultsWithAllButton )
      {
      this->ResultsWithAllButton->SetParent ( NULL );
      this->ResultsWithAllButton->Delete();
      this->ResultsWithAllButton = NULL;      
      }
    if ( this->ResultsWithExactButton )
      {
      this->ResultsWithExactButton->SetParent ( NULL );
      this->ResultsWithExactButton->Delete();
      this->ResultsWithExactButton = NULL;      
      }

    //---
    // query builder structure panel
    //---
    if ( this->StructureFrame )
      {
      this->StructureFrame->SetParent ( NULL );
      this->StructureFrame->Delete();
      this->StructureFrame = NULL;      
      }
    if ( this->StructureMenuButton)
      {
      this->StructureMenuButton->SetParent ( NULL );
      this->StructureMenuButton->Delete();
      this->StructureMenuButton = NULL;
      }

    //---
    // ontology frame
    //---
    if ( this->LocalSearchTermEntry )
      {
      this->LocalSearchTermEntry->SetParent ( NULL );
      this->LocalSearchTermEntry->Delete();
      this->LocalSearchTermEntry = NULL;
      }
    if ( this->SynonymsMenuButton )
      {
      this->SynonymsMenuButton->SetParent ( NULL );
      this->SynonymsMenuButton->Delete();
      this->SynonymsMenuButton = NULL;      
      }
    if ( this->BIRNLexEntry )
      {
      this->BIRNLexEntry->SetParent ( NULL );
      this->BIRNLexEntry->Delete();
      this->BIRNLexEntry = NULL;      
      }
    if ( this->BIRNLexIDEntry )
      {
      this->BIRNLexIDEntry->SetParent ( NULL );
      this->BIRNLexIDEntry->Delete();
      this->BIRNLexIDEntry = NULL;      
      }
    if ( this->NeuroNamesEntry )
      {
      this->NeuroNamesEntry->SetParent ( NULL );
      this->NeuroNamesEntry->Delete ( );
      this->NeuroNamesEntry = NULL;      
      }
    if ( this->NeuroNamesIDEntry )
      {
      this->NeuroNamesIDEntry->SetParent ( NULL );
      this->NeuroNamesIDEntry->Delete ( );
      this->NeuroNamesIDEntry = NULL;      
      }
    if ( this->UMLSCIDEntry )
      {
      this->UMLSCIDEntry->SetParent ( NULL );
      this->UMLSCIDEntry->Delete ( );
      this->UMLSCIDEntry = NULL;      
      }
    if ( this->AddLocalTermButton )
      {
      this->AddLocalTermButton->SetParent ( NULL );
      this->AddLocalTermButton->Delete ( );
      this->AddLocalTermButton = NULL;      
      }
    if ( this->AddSynonymButton )
      {
      this->AddSynonymButton->SetParent ( NULL );
      this->AddSynonymButton->Delete ();
      this->AddSynonymButton = NULL;      
      }
    if ( this->AddBIRNLexStringButton )
      {
      this->AddBIRNLexStringButton->SetParent ( NULL );
      this->AddBIRNLexStringButton->Delete ( );
      this->AddBIRNLexStringButton = NULL;      
      }
    if ( this->AddBIRNLexIDButton)
      {
      this->AddBIRNLexIDButton->SetParent ( NULL );
      this->AddBIRNLexIDButton->Delete ( );
      this->AddBIRNLexIDButton = NULL;      
      }
    if ( this->AddNeuroNamesStringButton )
      {
      this->AddNeuroNamesStringButton->SetParent ( NULL );
      this->AddNeuroNamesStringButton->Delete ();
      this->AddNeuroNamesStringButton = NULL;      
      }
    if ( this->AddNeuroNamesIDButton )
      {    //---
      this->AddNeuroNamesIDButton->SetParent ( NULL );
      this->AddNeuroNamesIDButton->Delete ( );
      this->AddNeuroNamesIDButton = NULL;      
      }
    if ( this->AddUMLSCIDButton )
      {
      this->AddUMLSCIDButton->SetParent ( NULL );
      this->AddUMLSCIDButton->Delete ();
      this->AddUMLSCIDButton = NULL;      
      }
    if ( this->NeuroNamesHierarchyButton)
      {
      this->NeuroNamesHierarchyButton->SetParent ( NULL );
      this->NeuroNamesHierarchyButton->Delete();
      this->NeuroNamesHierarchyButton = NULL;      
      }
    if ( this->BIRNLexHierarchyButton )
      {
      this->BIRNLexHierarchyButton->SetParent ( NULL );
      this->BIRNLexHierarchyButton->Delete();
      this->BIRNLexHierarchyButton= NULL;
      }
    if ( this->UMLSHierarchyButton )
      {
      this->UMLSHierarchyButton->SetParent ( NULL );
      this->UMLSHierarchyButton->Delete();
      this->UMLSHierarchyButton= NULL;
      }
    if ( this->SavedTerms )
      {
      this->SavedTerms->SetParent ( NULL );
      this->SavedTerms->Delete();
      this->SavedTerms = NULL;
      }

    //---
    // query builder structure panel
    //---
    if ( this->StructureListWidget )
      {
      this->StructureListWidget->SetParent ( NULL );
      this->StructureListWidget->Delete ( );
      this->StructureListWidget = NULL;
      }

    //---
    // results panel
    //---
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
    if ( this->SaveCurrentSelectedResultsButton )
      {
      this->SaveCurrentSelectedResultsButton->SetParent(NULL);
      this->SaveCurrentSelectedResultsButton->Delete();
      this->SaveCurrentSelectedResultsButton = NULL;
      }

    if ( this->SavePastResultsButton )
      {
      this->SavePastResultsButton->SetParent(NULL);
      this->SavePastResultsButton->Delete();
      this->SavePastResultsButton = NULL;
      }
    if ( this->LoadURIsButton )
      {
      this->LoadURIsButton->SetParent ( NULL );
      this->LoadURIsButton->Create();
      this->LoadURIsButton = NULL;
      }

    //---
    // query builder substructure panel
    //---
    if ( this->SubStructureFrame )
      {
      this->SubStructureFrame->SetParent ( NULL );
      this->SubStructureFrame->Delete();
      this->SubStructureFrame = NULL;      
      }
    if ( this->SubStructureListWidget )
      {
      this->SubStructureListWidget->SetParent ( NULL );
      this->SubStructureListWidget->Delete();
      this->SubStructureListWidget = NULL;      
      }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::OpenBIRNLexBrowser()
{
  this->Script ( "QueryAtlasLaunchBirnLexHierarchy" );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::OpenNeuroNamesBrowser()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::OpenUMLSBrowser()
{
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::PrintSelf ( ostream& os, vtkIndent indent )
{
    this->vtkObject::PrintSelf ( os, indent );

    os << indent << "QueryAtlasGUI: " << this->GetClassName ( ) << "\n";
    os << indent << "Logic: " << this->GetLogic ( ) << "\n";

    //---
    // load frame
    //---
    os << indent << "FSbrainSelector: " << this->GetFSbrainSelector ( ) << "\n";    
    os << indent << "FSoverlaySelector: " << this->GetFSoverlaySelector ( ) << "\n";    
    os << indent << "FSmodelSelector: " << this->GetFSmodelSelector ( ) << "\n";    
    os << indent << "FStransformSelector: " << this->GetFStransformSelector ( ) << "\n";    
    os << indent << "QdecModelSelector: " << this->GetQdecModelSelector ( ) << "\n";    

    //---
    // ontology frame
    //---
    os << indent << "LoadSearchTermEntry" << this->GetLocalSearchTermEntry ( ) << "\n";    
    os << indent << "SynonymsMenuButton" << this->GetSynonymsMenuButton ( ) << "\n";    
    os << indent << "BIRNLexEntry" << this->GetBIRNLexEntry ( ) << "\n";    
    os << indent << "BIRNLexIDEntry" << this->GetBIRNLexIDEntry ( ) << "\n";    
    os << indent << "NeuroNamesEntry" << this->GetNeuroNamesEntry ( ) << "\n";    
    os << indent << "NeuroNamesIDEntry" << this->GetNeuroNamesIDEntry ( ) << "\n";    
    os << indent << "UMLSCIDEntry" << this->GetUMLSCIDEntry ( ) << "\n";    
    os << indent << "AddLocalTermButton" << this->GetAddLocalTermButton ( ) << "\n";    
    os << indent << "AddSynonymButton" << this->GetAddSynonymButton ( ) << "\n";    
    os << indent << "AddBIRNLexStringButton" << this->GetAddBIRNLexStringButton ( ) << "\n";    
    os << indent << "AddBIRNLexIDButton" << this->GetAddBIRNLexIDButton ( ) << "\n";    
    os << indent << "AddNeuroNamesStringButton" << this->GetAddNeuroNamesStringButton ( ) << "\n";    
    os << indent << "AddNeuroNamesIDButton" << this->GetAddNeuroNamesIDButton ( ) << "\n";    
    os << indent << "AddUMLSCIDButton" << this->GetAddUMLSCIDButton ( ) << "\n";    
    os << indent << "BIRNLexHierarchyButton" << this->GetBIRNLexHierarchyButton ( ) << "\n";    
    os << indent << "NeuroNamesHierarchyButton" << this->GetNeuroNamesHierarchyButton ( ) << "\n";    
    os << indent << "UMLSHierarchyButton" << this->GetUMLSHierarchyButton ( ) << "\n";    
    os << indent << "SavedTerms" << this->GetSavedTerms() << "\n";
    os << indent << "AddDiagnosisEntry" << this->GetAddDiagnosisEntry() << "\n";
    os << indent << "ResultsWithExactButton" <<  this->GetResultsWithExactButton() << "\n";
    os << indent << "ResultsWithAnyButton" <<  this->GetResultsWithAnyButton() << "\n";
    os << indent << "ResultsWithAllButton" <<  this->GetResultsWithAllButton() << "\n";
    //---
    // TODO: finish this method!
    //---
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::RemoveGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: RemoveGUIObservers\n");

  this->FIPSFSButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QdecButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->StructureButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetClearAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetAddNewButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->StructureListWidget->GetUseAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetUseNoneButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SubStructureButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetClearAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetAddNewButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetClearSelectedButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->SubStructureListWidget->GetUseAllButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetUseNoneButton()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SpeciesButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesNoneButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesHumanButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMouseButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMacaqueButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ModelVisibilityButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AnnotationVisibilityButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->BIRNLexHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NeuroNamesHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LocalSearchTermEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->PopulationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddDiagnosisEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->DatabasesMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->SearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->SaveCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveCurrentSelectedResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SavePastResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadURIsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeletePastResultButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllPastResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteCurrentResultButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CurrentResultsList->GetWidget()->RemoveObservers(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->PastResultsList->GetWidget()->RemoveObservers(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: AddGUIObservers\n");
  this->FIPSFSButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QdecButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->StructureButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetAddNewButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->StructureListWidget->GetUseAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->GetUseNoneButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SubStructureButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetClearAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetAddNewButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetClearSelectedButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->SubStructureListWidget->GetUseAllButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SubStructureListWidget->GetUseNoneButton()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SpeciesButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesNoneButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesHumanButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMouseButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMacaqueButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->ModelVisibilityButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AnnotationVisibilityButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->BIRNLexHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NeuroNamesHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LocalSearchTermEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->PopulationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddDiagnosisEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->DatabasesMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->SearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->SaveCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveCurrentSelectedResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SavePastResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LoadURIsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeletePastResultButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllPastResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteCurrentResultButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->CurrentResultsList->GetWidget()->AddObserver(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
//  this->PastResultsList->GetWidget()->AddObserver(vtkKWListBox::ListBoxSelectionChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ProcessGUIEvents ( vtkObject *caller,
                                            unsigned long event, void *callData )
{
    // nothing to do here yet...
  vtkKWPushButton *b = vtkKWPushButton::SafeDownCast ( caller );
  vtkKWMenu *m = vtkKWMenu::SafeDownCast ( caller );
  vtkKWCheckButton *c = vtkKWCheckButton::SafeDownCast ( caller );
  vtkKWListBox *lb = vtkKWListBox::SafeDownCast ( caller );
  vtkKWEntry *e  = vtkKWEntry::SafeDownCast ( caller);
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

  const char *context;
  int index;
  
  if ( (e == this->AddDiagnosisEntry->GetWidget() ) && ( event == vtkKWEntry::EntryValueChangedEvent ))
    {
    }
  else if ( (b == this->NeuroNamesHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    }
  else if ( (b == this->BIRNLexHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    //--- TODO: check to see if BIRNLexBrowser is open.
    
    //--- Will open if it's not alreay open.
    this->OpenBIRNLexBrowser();
    //--- get last clicked (or typed) structure from the LocalSearchTermEntry
    const char *structureLabel =  this->LocalSearchTermEntry->GetValue();
    if ( !strcmp (structureLabel, "" ))
      {
      structureLabel = "BIRNLex_subset";
      }
    this->Script ( "QueryAtlasSendHierarchyCommand  %s", structureLabel );
    }
  else if ( (b == this->SearchButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "QueryAtlasFormURLsForTargets");
    }
  else if ( (b == this->AnnotationVisibilityButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    if ( this->AnnotationVisibility == 1 )
      {
      // turn off automatic annotations in the main viewer
      vtkKWIcon *i = app->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceController()->GetVisibilityIcons()->GetInvisibleIcon();
      this->AnnotationVisibilityButton->SetImageToIcon ( i );
      this->AnnotationVisibility = 0;
     //      this->Script ( "" );
      }
    else
      {
      // turn on automatic annotations in main viewer
      vtkKWIcon *i = app->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceController()->GetVisibilityIcons()->GetVisibleIcon();
      this->AnnotationVisibilityButton->SetImageToIcon ( i );
      this->AnnotationVisibility = 1;
     //      this->Script ( "" );
      }
    }
  else if ( (b == this->ModelVisibilityButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    if ( this->ModelVisibility == 1 )
      {
      // turn off automatic annotations in the main viewer
      vtkKWIcon *i = app->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceController()->GetVisibilityIcons()->GetInvisibleIcon();
      this->ModelVisibilityButton->SetImageToIcon ( i );
      this->ModelVisibility = 0;
     //      this->Script ( "" );
      }
    else
      {
      // turn on automatic annotations in main viewer
      vtkKWIcon *i = app->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceController()->GetVisibilityIcons()->GetVisibleIcon();
      this->ModelVisibilityButton->SetImageToIcon ( i );
      this->ModelVisibility = 1;
     //      this->Script ( "" );
      }
    }

  else if ( (b == this->FIPSFSButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackLoaderContextFrames();
    this->PackLoaderContextFrame ( this->FIPSFSFrame );
    this->ColorCodeLoaderContextButtons ( this->FIPSFSButton );
    }
  else if ( (b == this->QdecButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackLoaderContextFrames();
    this->PackLoaderContextFrame ( this->QdecFrame );
    this->ColorCodeLoaderContextButtons ( this->QdecButton );
    }
  else if ( (b == this->StructureButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    this->ColorCodeContextButtons ( this->StructureButton );
    }
  else if ( (b == this->StructureListWidget->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->DeleteAllSearchTerms( context );
    }
  else if ( (b == this->StructureListWidget->GetAddNewButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->AddNewSearchTerm( context );
    }
  else if ( (b == this->StructureListWidget->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->DeleteSelectedSearchTerms (context );
    }
  else if ( (b == this->StructureListWidget->GetUseAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->SelectAllSearchTerms ( context );
    }
  else if ( (b == this->StructureListWidget->GetUseNoneButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "structure";
    this->DeselectAllSearchTerms ( context );
    }  
  else if ( (b == this->SubStructureButton) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->UnpackQueryBuilderContextFrames();
    this->PackQueryBuilderContextFrame ( this->SubStructureFrame);    
    this->ColorCodeContextButtons ( this->SubStructureButton );
    }
  else if ( (b == this->SubStructureListWidget->GetClearAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "substructure";
    this->DeleteAllSearchTerms( context );
    }
  else if ( (b == this->SubStructureListWidget->GetAddNewButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "substructure";
    this->AddNewSearchTerm( context );
    }
  else if ( (b == this->SubStructureListWidget->GetClearSelectedButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "substructure";
    this->DeleteSelectedSearchTerms (context );
    }
  else if ( (b == this->SubStructureListWidget->GetUseAllButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "substructure";
    this->SelectAllSearchTerms ( context );
    }
  else if ( (b == this->SubStructureListWidget->GetUseNoneButton()) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    context = "substructure";
    this->DeselectAllSearchTerms ( context );
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
  else if ( (b == this->DeleteCurrentResultButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    index = this->CurrentResultsList->GetWidget()->GetSelectionIndex();
    if ( index >= 0)
      {
      this->CurrentResultsList->GetWidget()->DeleteRange( index, index );
      }
    }
  else if ( (b == this->DeleteAllCurrentResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->CurrentResultsList->GetWidget()->DeleteAll();
    }
  else if ( (b == this->SaveCurrentResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script ( "QueryAtlasBundleSearchResults");
    }
  else if ( (b == this->SaveCurrentSelectedResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
//    this->Script ( "QueryAtlasBundleSearchResults");
    }
  else if ( (b == this->DeletePastResultButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    index = this->PastResultsList->GetWidget()->GetSelectionIndex();
    if ( index >= 0)
      {
      this->PastResultsList->GetWidget()->DeleteRange( index, index );
      }
    }
  else if ( (b == this->DeleteAllPastResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->PastResultsList->GetWidget()->DeleteAll();
    }
  else if ( (b == this->SavePastResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
    this->Script( "QueryAtlasSaveLinkBundlesToFile");
    }
  else if ( (b == this->LoadURIsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
    {
//    this->Script(""); load links from a file and put in bottom list.
    }
  if ((lb = this->CurrentResultsList->GetWidget()) && (event == vtkKWListBox::ListBoxSelectionChangedEvent ))
    {
    this->Script ("QueryAtlasOpenLink");
    }
  else if ((lb = this->PastResultsList->GetWidget()) && (event == vtkKWListBox::ListBoxSelectionChangedEvent ))
    {
    }
  // no need to do anything here; we'll just get the widget values with tcl.
  if (( m== this->DiagnosisMenuButton->GetWidget()->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
    {
    }
  else if (( m== this->GenderMenuButton->GetWidget()->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
    {
    }
  else if (( m== this->HandednessMenuButton->GetWidget()->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
    {
    }
  else if (( m== this->AgeMenuButton->GetWidget()->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
    {
    }
  if ((c == this->SpeciesNoneButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    }
  if ((c == this->SpeciesHumanButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    }
  if ((c == this->SpeciesMouseButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    }
  if ((c == this->SpeciesMacaqueButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
    {
    }
    return;
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ColorCodeContextButtons ( vtkKWPushButton *b )
{
  this->SubStructureButton->SetBackgroundColor ( _br, _bg, _bb );
  this->StructureButton->SetBackgroundColor ( _br, _bg, _bb );
  this->PopulationButton->SetBackgroundColor ( _br, _bg, _bb );
  this->SpeciesButton->SetBackgroundColor ( _br, _bg, _bb );

  this->SubStructureButton->SetForegroundColor ( _fr, _fg, _fb );
  this->StructureButton->SetForegroundColor ( _fr, _fg, _fb );
  this->PopulationButton->SetForegroundColor ( _fr, _fg, _fb );
  this->SpeciesButton->SetForegroundColor ( _fr, _fg, _fb );

  b->SetBackgroundColor (1.0, 1.0, 1.0);
  b->SetForegroundColor (0.0, 0.0, 0.0);
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

    const char *help = "The (Generation 1) Query Atlas module allows interactive Google, Wikipedia, queries from within the 3D anatomical display.";
    const char *about = "This research was supported by Grant 5 MOI RR 000827 to the FIRST BIRN and Grant 1 U24 RR021992 to the FBIRN Biomedical Informatics Research Network (BIRN, http://www.nbirn.net), that is funded by the National Center for Research Resources (NCRR) at the National Institutes of Health (NIH). This work was also supported by NA-MIC, NAC, NCIGT. NeuroNames ontology and URI resources are provided courtesy of BrainInfo, Neuroscience Division, National Primate Research Center, University of Washington (http://www.braininfo.org).                                                                                                                                                                                      ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    this->QueryAtlasIcons = vtkQueryAtlasIcons::New();
    this->BuildHelpAndAboutFrame ( page, help, about );
    this->BuildHelpAndAcknowledgementGUI ( );
    this->BuildLoadAndConvertGUI ( );
    this->BuildAnnotationOptionsGUI ( );
    this->BuildOntologyGUI ( );
    this->BuildQueryGUI ( );
    this->BuildSearchGUI ( );
    this->BuildResultsManagerGUI ( );
    this->BuildDisplayAndNavigationGUI ( );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildHelpAndAcknowledgementGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());


    vtkKWLabel *abirn = vtkKWLabel::New();
    abirn->SetParent ( this->GetLogoFrame() );
    abirn->Create();
    abirn->SetImageToIcon ( this->GetAcknowledgementIcons()->GetBIRNLogo() );

    vtkKWLabel *anac = vtkKWLabel::New();
    anac->SetParent ( this->GetLogoFrame() );
    anac->Create();
    anac->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNACLogo() );

    vtkKWLabel *anamic = vtkKWLabel::New();
    anamic->SetParent ( this->GetLogoFrame() );
    anamic->Create();
    anamic->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNAMICLogo() );

    vtkKWLabel *aigt = vtkKWLabel::New();
    aigt->SetParent ( this->GetLogoFrame() );
    aigt->Create();
    aigt->SetImageToIcon ( this->GetAcknowledgementIcons()->GetNCIGTLogo() );

    this->CollaboratorIcons = vtkQueryAtlasCollaboratorIcons::New();

    vtkKWLabel *abi = vtkKWLabel::New();
    abi->SetParent ( this->GetLogoFrame() );
    abi->Create();
    abi->SetImageToIcon ( this->GetCollaboratorIcons()->GetBrainInfoLogo() );
    
    app->Script ("grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w",  abirn->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w", anamic->GetWidgetName());
    app->Script ("grid %s -row 0 -column 2 -padx 2 -pady 2 -sticky w",  anac->GetWidgetName());
    app->Script ("grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  aigt->GetWidgetName());                 
    app->Script ("grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  abi->GetWidgetName());                 
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildLoadAndConvertGUI ( )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );

    //---
    // LOAD AND CONVERSION FRAME 
    //---
    vtkSlicerModuleCollapsibleFrame *convertFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    convertFrame->SetParent ( page );
    convertFrame->Create ( );
    convertFrame->SetLabelText ("Load & Configure");
//    convertFrame->ExpandFrame ( );
    convertFrame->CollapseFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  convertFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    this->BuildLoaderContextButtons ( convertFrame->GetFrame() );
    
    vtkKWFrame *switcher = vtkKWFrame::New();
    switcher->SetParent ( convertFrame->GetFrame() );
    switcher->Create();
    
    this->BuildLoaderContextFrames ( switcher );
    this->BuildFreeSurferFIPSFrame ( );
    this->BuildQdecFrame ( );
    this->PackLoaderContextFrame ( this->FIPSFSFrame );
    app->Script ( "pack %s -side top -fill x -expand 1", switcher->GetWidgetName() );
    this->ColorCodeLoaderContextButtons ( this->FIPSFSButton );
    switcher->Delete();
    convertFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildFreeSurferFIPSFrame( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
    this->FSbrainSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->FSbrainSelector->SetParent ( this->FIPSFSFrame );
    this->FSbrainSelector->Create ( );
    this->FSbrainSelector->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->FSbrainSelector->SetMRMLScene(this->GetMRMLScene());
    this->FSbrainSelector->SetBorderWidth(2);
    this->FSbrainSelector->SetPadX(2);
    this->FSbrainSelector->SetPadY(2);
    this->FSbrainSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->FSbrainSelector->GetWidget()->GetWidget()->SetWidth(20);
    this->FSbrainSelector->GetLabel()->SetWidth(18);
    this->FSbrainSelector->SetLabelText( "FreeSurfer anatomical: ");
    this->FSbrainSelector->SetBalloonHelpString("select a volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                   this->FSbrainSelector->GetWidgetName());

    this->FSoverlaySelector = vtkSlicerNodeSelectorWidget::New() ;
    this->FSoverlaySelector->SetParent ( this->FIPSFSFrame );
    this->FSoverlaySelector->Create ( );
    this->FSoverlaySelector->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->FSoverlaySelector->SetMRMLScene(this->GetMRMLScene());
    this->FSoverlaySelector->SetBorderWidth(2);
    this->FSoverlaySelector->SetPadX(2);
    this->FSoverlaySelector->SetPadY(2);
    this->FSoverlaySelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->FSoverlaySelector->GetWidget()->GetWidget()->SetWidth(20);
    this->FSoverlaySelector->GetLabel()->SetWidth(18);
    this->FSoverlaySelector->SetLabelText( "Statistical volume: ");
    this->FSoverlaySelector->SetBalloonHelpString("select a volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->FSoverlaySelector->GetWidgetName());


    this->FSmodelSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->FSmodelSelector->SetParent ( this->FIPSFSFrame );
    this->FSmodelSelector->Create ( );
    this->FSmodelSelector->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->FSmodelSelector->SetMRMLScene(this->GetMRMLScene());
    this->FSmodelSelector->SetBorderWidth(2);
    this->FSmodelSelector->SetPadX(2);
    this->FSmodelSelector->SetPadY(2);
    this->FSmodelSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->FSmodelSelector->GetWidget()->GetWidget()->SetWidth(20);
    this->FSmodelSelector->GetLabel()->SetWidth(18);
    this->FSmodelSelector->SetLabelText( "Annotated model: ");
    this->FSmodelSelector->SetBalloonHelpString("select a volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->FSmodelSelector->GetWidgetName());

    this->FStransformSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->FStransformSelector->SetParent ( this->FIPSFSFrame );
    this->FStransformSelector->Create ( );
    this->FStransformSelector->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->FStransformSelector->SetMRMLScene(this->GetMRMLScene());
    this->FStransformSelector->SetBorderWidth(2);
    this->FStransformSelector->SetPadX(2);
    this->FStransformSelector->SetPadY(2);
    this->FStransformSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->FStransformSelector->GetWidget()->GetWidget()->SetWidth(20);
    this->FStransformSelector->GetLabel()->SetWidth(18);
    this->FStransformSelector->SetLabelText( "Registration transform: ");
    this->FStransformSelector->SetBalloonHelpString("select a volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->FStransformSelector->GetWidgetName());


}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQdecFrame ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  

    this->QdecModelSelector = vtkSlicerNodeSelectorWidget::New() ;
    this->QdecModelSelector->SetParent ( this->QdecFrame );
    this->QdecModelSelector->Create ( );
    this->QdecModelSelector->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
    this->QdecModelSelector->SetMRMLScene(this->GetMRMLScene());
    this->QdecModelSelector->SetBorderWidth(2);
    this->QdecModelSelector->SetPadX(2);
    this->QdecModelSelector->SetPadY(2);
    this->QdecModelSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
    this->QdecModelSelector->GetWidget()->GetWidget()->SetWidth(20);
    this->QdecModelSelector->GetLabel()->SetWidth(18);
    this->QdecModelSelector->SetLabelText( "Annotated model: ");
    this->QdecModelSelector->SetBalloonHelpString("select a volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2",
                  this->QdecModelSelector->GetWidgetName());

    //get uri from model, load annotations from a relative path??

}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildAnnotationOptionsGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    // -------------------------------------------------------------------------------------------------
    // ---
    // ANNOTATION OPTIONS FRAME

    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *annotationFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    annotationFrame->SetParent ( page );
    annotationFrame->Create ();
    annotationFrame->SetLabelText ( "Annotation & Display Options" );
    annotationFrame->CollapseFrame ( );

    vtkKWLabel *annoLabel = vtkKWLabel::New();
    annoLabel->SetParent ( annotationFrame->GetFrame() );
    annoLabel->Create();
    annoLabel->SetText ("annotation visibility: " );

    this->AnnotationVisibilityButton = vtkKWPushButton::New();
    this->AnnotationVisibilityButton->SetParent ( annotationFrame->GetFrame() );
    this->AnnotationVisibilityButton->Create();
    // get the icon this way; don't seem to admit baseGUI scope.
    // TODO: move common icons up into applicationGUI for easy access.
    vtkKWIcon *i = app->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceController()->GetVisibilityIcons()->GetVisibleIcon();
    this->AnnotationVisibilityButton->SetImageToIcon ( i );
    this->AnnotationVisibilityButton->SetBorderWidth ( 0 );
    this->AnnotationVisibilityButton->SetReliefToFlat();    
    this->AnnotationVisibilityButton->SetBalloonHelpString ( "Toggle annotation visibility." );

    vtkKWLabel *modelLabel = vtkKWLabel::New();
    modelLabel->SetParent ( annotationFrame->GetFrame() );
    modelLabel->Create();
    modelLabel->SetText ("model visibility: " );

    this->ModelVisibilityButton = vtkKWPushButton::New();
    this->ModelVisibilityButton->SetParent ( annotationFrame->GetFrame() );
    this->ModelVisibilityButton->Create();
    // get the icon this way; don't seem to admit baseGUI scope.
    // TODO: move common icons up into applicationGUI for easy access.
    i = app->GetApplicationGUI()->GetMainSliceGUI0()->GetSliceController()->GetVisibilityIcons()->GetVisibleIcon();
    this->ModelVisibilityButton->SetImageToIcon ( i );
    this->ModelVisibilityButton->SetBorderWidth ( 0 );
    this->ModelVisibilityButton->SetReliefToFlat();    
    this->ModelVisibilityButton->SetBalloonHelpString ( "Toggle model visibility." );

    vtkKWLabel *l = vtkKWLabel::New();
    l->SetParent ( annotationFrame->GetFrame() );
    l->Create ( );
    l->SetText ( "term set: " );

    this->AnnotationNomenclatureMenuButton = vtkKWMenuButton::New();
    this->AnnotationNomenclatureMenuButton->SetParent ( annotationFrame->GetFrame() );
    this->AnnotationNomenclatureMenuButton->Create();
    this->AnnotationNomenclatureMenuButton->SetWidth ( 25 );
    this->AnnotationNomenclatureMenuButton->GetMenu()->AddRadioButton ("local identifier");
    this->AnnotationNomenclatureMenuButton->GetMenu()->AddRadioButton ("BIRNLex String");
    this->AnnotationNomenclatureMenuButton->GetMenu()->AddRadioButton ("NeuroNames String");
    this->AnnotationNomenclatureMenuButton->GetMenu()->AddRadioButton ("UMLS CID");
    this->AnnotationNomenclatureMenuButton->GetMenu()->AddSeparator();
    this->AnnotationNomenclatureMenuButton->GetMenu()->AddCommand ( "close" );    
    this->AnnotationNomenclatureMenuButton->GetMenu()->SelectItem ("local identifier");

    app->Script ( "grid %s -row 0 -column 0 -sticky nse -padx 2 -pady 2",
                  l->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky wns -padx 2 -pady 2",
                  this->AnnotationNomenclatureMenuButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 0 -sticky ens -padx 2 -pady 2",
                  annoLabel->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1  -sticky wns -padx 2 -pady 2",
                  this->AnnotationVisibilityButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 0   -sticky ens -padx 2 -pady 2",
                  modelLabel->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 1   -sticky wns -padx 2 -pady 2",
                  this->ModelVisibilityButton->GetWidgetName() );

    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 4 -pady 2 -in %s",
                  annotationFrame->GetWidgetName(), 
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    l->Delete();
    annoLabel->Delete();
    modelLabel->Delete();
    annotationFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildOntologyGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    // -------------------------------------------------------------------------------------------------
    // ---
    // HIERARCHY SEARCH FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *hierarchyFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    hierarchyFrame->SetParent ( page );
    hierarchyFrame->Create ( );
    hierarchyFrame->SetLabelText ("Ontology Mapping");
    hierarchyFrame->CollapseFrame ( );

    // first row (local terms)
    vtkKWLabel *localLabel = vtkKWLabel::New();
    localLabel->SetParent ( hierarchyFrame->GetFrame() );
    localLabel->Create();
    localLabel->SetText ("local term: ");
    this->LocalSearchTermEntry = vtkKWEntry::New();
    this->LocalSearchTermEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->LocalSearchTermEntry->Create();
    this->LocalSearchTermEntry->SetValue ("");
    this->AddLocalTermButton = vtkKWPushButton::New();
    this->AddLocalTermButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddLocalTermButton->Create();
    this->AddLocalTermButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddLocalTermButton->SetBorderWidth ( 0 );
    this->AddLocalTermButton->SetReliefToFlat();
    this->AddLocalTermButton->SetBalloonHelpString ("Save this term for building queries.");
    
    // second row (synonyms)
    vtkKWLabel *synLabel = vtkKWLabel::New();
    synLabel->SetParent ( hierarchyFrame->GetFrame() );
    synLabel->Create();
    synLabel->SetText ("synonyms: ");
    this->SynonymsMenuButton = vtkKWMenuButton::New();
    this->SynonymsMenuButton->SetParent ( hierarchyFrame->GetFrame() );
    this->SynonymsMenuButton->Create();
    this->SynonymsMenuButton->IndicatorVisibilityOn();
    this->AddSynonymButton = vtkKWPushButton::New();
    this->AddSynonymButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddSynonymButton->Create();
    this->AddSynonymButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddSynonymButton->SetBorderWidth ( 0 );
    this->AddSynonymButton->SetReliefToFlat();
    this->AddSynonymButton->SetBalloonHelpString ("Save this term for building queries.");

    // third row (BIRNLex)
    vtkKWLabel *birnLabel = vtkKWLabel::New();
    birnLabel->SetParent ( hierarchyFrame->GetFrame() );
    birnLabel->Create();
    birnLabel->SetText ("BIRNLex: ");
    this->BIRNLexEntry = vtkKWEntry::New();
    this->BIRNLexEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->BIRNLexEntry->Create();
    this->BIRNLexEntry->SetValue ("");
    this->AddBIRNLexStringButton = vtkKWPushButton::New();
    this->AddBIRNLexStringButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddBIRNLexStringButton->Create();
    this->AddBIRNLexStringButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddBIRNLexStringButton->SetBorderWidth(0);
    this->AddBIRNLexStringButton->SetReliefToFlat();
    this->AddBIRNLexStringButton->SetBalloonHelpString ("Save this term for building queries.");
    this->BIRNLexHierarchyButton = vtkKWPushButton::New();
    this->BIRNLexHierarchyButton->SetParent ( hierarchyFrame->GetFrame() );
    this->BIRNLexHierarchyButton->Create();
    this->BIRNLexHierarchyButton->SetImageToIcon ( this->QueryAtlasIcons->GetOntologyBrowserIcon() );
    this->BIRNLexHierarchyButton->SetBorderWidth ( 0 );
    this->BIRNLexHierarchyButton->SetReliefToFlat ( );
    this->BIRNLexHierarchyButton->SetBalloonHelpString ("Launch ontology browser.");

    // forthrow (BIRNLex ID )
    vtkKWLabel *birnidLabel = vtkKWLabel::New();
    birnidLabel->SetParent ( hierarchyFrame->GetFrame() );
    birnidLabel->Create();
    birnidLabel->SetText ("BIRNLex ID: ");
    this->BIRNLexIDEntry = vtkKWEntry::New();
    this->BIRNLexIDEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->BIRNLexIDEntry->Create();
    this->BIRNLexIDEntry->SetValue("");
    this->AddBIRNLexIDButton = vtkKWPushButton::New();
    this->AddBIRNLexIDButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddBIRNLexIDButton->Create();
    this->AddBIRNLexIDButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddBIRNLexIDButton->SetBorderWidth ( 0 );
    this->AddBIRNLexIDButton->SetReliefToFlat ( );
    this->AddBIRNLexIDButton->SetBalloonHelpString ("Save this term for building queries.");

    // fifth row (NeuroNames)
    vtkKWLabel *nnLabel = vtkKWLabel::New();
    nnLabel->SetParent ( hierarchyFrame->GetFrame() );
    nnLabel->Create();
    nnLabel->SetText ("NeuroNames: ");
    this->NeuroNamesEntry = vtkKWEntry::New();
    this->NeuroNamesEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->NeuroNamesEntry->Create();
    this->NeuroNamesEntry->SetValue ("");
    this->AddNeuroNamesStringButton = vtkKWPushButton::New();
    this->AddNeuroNamesStringButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddNeuroNamesStringButton->Create();
    this->AddNeuroNamesStringButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddNeuroNamesStringButton->SetBorderWidth (0 );
    this->AddNeuroNamesStringButton->SetReliefToFlat( );
    this->AddNeuroNamesStringButton->SetBalloonHelpString ("Save this term for building queries.");
    this->NeuroNamesHierarchyButton = vtkKWPushButton::New();
    this->NeuroNamesHierarchyButton->SetParent ( hierarchyFrame->GetFrame() );
    this->NeuroNamesHierarchyButton->Create();
    this->NeuroNamesHierarchyButton->SetImageToIcon ( this->QueryAtlasIcons->GetOntologyBrowserDisabledIcon() );
    this->NeuroNamesHierarchyButton->SetBorderWidth ( 0 );
    this->NeuroNamesHierarchyButton->SetReliefToFlat();
    this->NeuroNamesHierarchyButton->SetBalloonHelpString ("Launch ontology browser.");

    // sixth row (NeuroNames ID)
    vtkKWLabel *nnidLabel = vtkKWLabel::New();    
    nnidLabel->SetParent ( hierarchyFrame->GetFrame() );
    nnidLabel->Create();
    nnidLabel->SetText ("NeuroNames ID: ");
    this->NeuroNamesIDEntry = vtkKWEntry::New();
    this->NeuroNamesIDEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->NeuroNamesIDEntry->Create();
    this->NeuroNamesIDEntry->SetValue ("");
    this->AddNeuroNamesIDButton = vtkKWPushButton::New();
    this->AddNeuroNamesIDButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddNeuroNamesIDButton->Create();
    this->AddNeuroNamesIDButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddNeuroNamesIDButton->SetBorderWidth(0);
    this->AddNeuroNamesIDButton->SetReliefToFlat();
    this->AddNeuroNamesIDButton->SetBalloonHelpString ("Save this term for building queries.");
    // seventh row (UMLS)
    vtkKWLabel *umlsLabel = vtkKWLabel::New();
    umlsLabel->SetParent ( hierarchyFrame->GetFrame() );
    umlsLabel->Create();
    umlsLabel->SetText ("UMLS CID: ");
    this->UMLSCIDEntry = vtkKWEntry::New();
    this->UMLSCIDEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->UMLSCIDEntry->Create();
    this->UMLSCIDEntry->SetValue ("");
    this->AddUMLSCIDButton = vtkKWPushButton::New();
    this->AddUMLSCIDButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddUMLSCIDButton->Create();
    this->AddUMLSCIDButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddUMLSCIDButton->SetBorderWidth(0);
    this->AddUMLSCIDButton->SetReliefToFlat();
    this->AddUMLSCIDButton->SetBalloonHelpString ("Save this term for building queries.");
    this->UMLSHierarchyButton = vtkKWPushButton::New();
    this->UMLSHierarchyButton->SetParent ( hierarchyFrame->GetFrame() );
    this->UMLSHierarchyButton->Create();
    this->UMLSHierarchyButton->SetImageToIcon ( this->QueryAtlasIcons->GetOntologyBrowserDisabledIcon() );
    this->UMLSHierarchyButton->SetBorderWidth ( 0 );
    this->UMLSHierarchyButton->SetReliefToFlat();
    this->UMLSHierarchyButton->SetBalloonHelpString ("Launch ontology browser.");
    
    // eighth row (listbox saved terms)
    vtkKWLabel *termsLabel = vtkKWLabel::New();
    termsLabel->SetParent (hierarchyFrame->GetFrame() );
    termsLabel->Create();
    termsLabel->SetText ( "(structure) ");
    
    vtkKWFrame *f = vtkKWFrame::New();
    f->SetParent ( hierarchyFrame->GetFrame() );
    f->Create();
    
    this->SavedTerms = vtkQueryAtlasSearchTermWidget::New();
    this->SavedTerms->SetParent ( f );
    this->SavedTerms->Create();
//    int i = this->SavedTerms->GetMultiColumnList()->GetWidget()->GetColumnIndexWithName ( "Search terms" );
//    this->SavedTerms->GetMultiColumnList()->GetWidget()->SetColumnName ( i, "Saved terms");
    this->SavedTerms->GetMultiColumnList()->GetWidget()->SetHeight(3);
    
    //---
    // grid and pack up
    //---
    app->Script ( "grid columnconfigure %s 0 -weight 0", hierarchyFrame->GetFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 1", hierarchyFrame->GetFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 0", hierarchyFrame->GetFrame()->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 3 -weight 0", hierarchyFrame->GetFrame()->GetWidgetName() );

    app->Script ( "grid %s -row 0 -column 0 -sticky e -padx 0 -pady 1", localLabel->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 0 -sticky e -padx 0 -pady 1", synLabel->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 0 -sticky e -padx 0 -pady 1", birnLabel->GetWidgetName() );
    app->Script ( "grid %s -row 3 -column 0 -sticky e -padx 0 -pady 1", birnidLabel->GetWidgetName() );
    app->Script ( "grid %s -row 4 -column 0 -sticky e -padx 0 -pady 1", nnLabel->GetWidgetName() );
    app->Script ( "grid %s -row 5 -column 0 -sticky e -padx 0 -pady 1", nnidLabel->GetWidgetName() );
    app->Script ( "grid %s -row 6 -column 0 -sticky e -padx 0 -pady 1", umlsLabel->GetWidgetName() );
    app->Script ( "grid %s -row 7 -column 0 -sticky ne -padx 0 -pady 1", termsLabel->GetWidgetName() );

    app->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 1", this->LocalSearchTermEntry->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1 -sticky ew -padx 2 -pady 1", this->SynonymsMenuButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 1 -sticky ew -padx 2 -pady 1", this->BIRNLexEntry->GetWidgetName() );
    app->Script ( "grid %s -row 3 -column 1 -sticky ew -padx 2 -pady 1", this->BIRNLexIDEntry->GetWidgetName() );
    app->Script ( "grid %s -row 4 -column 1 -sticky ew -padx 2 -pady 1", this->NeuroNamesEntry->GetWidgetName() );
    app->Script ( "grid %s -row 5 -column 1 -sticky ew -padx 2 -pady 1", this->NeuroNamesIDEntry->GetWidgetName() );
    app->Script ( "grid %s -row 6 -column 1 -sticky ew -padx 2 -pady 1", this->UMLSCIDEntry->GetWidgetName() );
    app->Script ( "grid %s -row 7 -column 1 -sticky ew -columnspan 2 -padx 2 -pady 1", f->GetWidgetName() );
    app->Script ( "pack %s -side top -fill x -expand true -padx 0 -pady 0", this->SavedTerms->GetWidgetName() );
    f->Delete();
    
    app->Script ( "grid %s -row 0 -column 2 -padx 2 -pady 1", this->AddLocalTermButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 1", this->AddSynonymButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 2 -padx 2 -pady 1", this->AddBIRNLexStringButton->GetWidgetName() );
    app->Script ( "grid %s -row 3 -column 2 -padx 2 -pady 1", this->AddBIRNLexIDButton->GetWidgetName() );
    app->Script ( "grid %s -row 4 -column 2 -padx 2 -pady 1", this->AddNeuroNamesStringButton->GetWidgetName() );
    app->Script ( "grid %s -row 5 -column 2 -padx 2 -pady 1", this->AddNeuroNamesIDButton->GetWidgetName() );
    app->Script ( "grid %s -row 6 -column 2 -padx 2 -pady 1", this->AddUMLSCIDButton->GetWidgetName() );

    app->Script ( "grid %s -row 2 -column 3 -padx 2 -pady 1", this->BIRNLexHierarchyButton->GetWidgetName() );
    app->Script ( "grid %s -row 4 -column 3 -padx 2 -pady 1", this->NeuroNamesHierarchyButton->GetWidgetName() );
    app->Script ( "grid %s -row 6 -column 3 -padx 2 -pady 1", this->UMLSHierarchyButton->GetWidgetName() );

    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2 -in %s",
                  hierarchyFrame->GetWidgetName(), 
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());


    //---
    // clean up.
    //---
    localLabel->Delete();
    synLabel->Delete();
    birnLabel->Delete();
    birnidLabel->Delete();
    nnLabel->Delete();
    nnidLabel->Delete();
    umlsLabel->Delete();
    termsLabel->Delete();
    hierarchyFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
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
    this->BuildSubStructureFrame();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    app->Script ( "pack %s -side top -fill x -expand 1", this->SwitchQueryFrame->GetWidgetName() );
//    this->Script ( "place %s -relx 0 -rely 0 -anchor nw", this->SwitchQueryFrame->GetWidgetName());
    this->ColorCodeContextButtons ( this->StructureButton );
    queryFrame->Delete();

}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildSearchGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );

    // -------------------------------------------------------------------------------------------------
    // ---
    // SEARCH FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *searchFrame = vtkSlicerModuleCollapsibleFrame::New();
    searchFrame->SetParent ( page);
    searchFrame->Create();
    searchFrame->SetLabelText ("Search");
    searchFrame->CollapseFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  searchFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    vtkKWFrame *f = vtkKWFrame::New();
    f->SetParent ( searchFrame->GetFrame() );
    f->Create();
    this->ResultsWithAnyButton = vtkKWRadioButton::New();
    this->ResultsWithAnyButton->SetParent ( f );
    this->ResultsWithAnyButton->Create();
    this->ResultsWithAnyButton->SetImageToIcon ( this->QueryAtlasIcons->GetWithAnyIcon() );
    this->ResultsWithAnyButton->SetSelectImageToIcon ( this->QueryAtlasIcons->GetWithAnySelectedIcon() );
    this->ResultsWithAnyButton->SetBorderWidth ( 0 );
    this->ResultsWithAnyButton->SetReliefToFlat ( );
    this->ResultsWithAnyButton->SetSelectedState ( 1 );
    this->ResultsWithAnyButton->IndicatorVisibilityOff();
    this->ResultsWithAnyButton->SetValueAsInt ( vtkQueryAtlasGUI::Or );
    this->ResultsWithAnyButton->SetBalloonHelpString ( "Search for results that include any of the search terms." );

    this->ResultsWithAllButton = vtkKWRadioButton::New();
    this->ResultsWithAllButton->SetParent ( f );
    this->ResultsWithAllButton->Create();
    this->ResultsWithAllButton->SetImageToIcon ( this->QueryAtlasIcons->GetWithAllIcon() );
    this->ResultsWithAllButton->SetImageToIcon ( this->QueryAtlasIcons->GetWithAllDisabledIcon() );
    this->ResultsWithAllButton->SetBorderWidth ( 0 );
    this->ResultsWithAllButton->SetReliefToFlat ( );
    this->ResultsWithAllButton->SetStateToDisabled();
    this->ResultsWithAllButton->SetValueAsInt ( vtkQueryAtlasGUI::And );
    this->ResultsWithAllButton->SetVariableName ( this->ResultsWithAnyButton->GetVariableName() );
    this->ResultsWithAllButton->SetBalloonHelpString ( "Search for results that include all of the search terms (disabled)." );

    this->ResultsWithExactButton = vtkKWRadioButton::New();
    this->ResultsWithExactButton->SetParent ( f );
    this->ResultsWithExactButton->Create();
    this->ResultsWithExactButton->SetImageToIcon ( this->QueryAtlasIcons->GetWithExactIcon() );
    this->ResultsWithExactButton->SetImageToIcon ( this->QueryAtlasIcons->GetWithExactDisabledIcon() );
    this->ResultsWithExactButton->SetBorderWidth ( 0 );
    this->ResultsWithExactButton->SetReliefToFlat ( );
    this->ResultsWithExactButton->SetStateToDisabled();
    this->ResultsWithExactButton->SetValueAsInt ( vtkQueryAtlasGUI::Quote );
    this->ResultsWithExactButton->SetVariableName ( this->ResultsWithAnyButton->GetVariableName() );
    this->ResultsWithExactButton->SetBalloonHelpString ( "Search for results that include any of the exact search term (disabled)." );

    vtkKWLabel *sl = vtkKWLabel::New();
    sl->SetParent ( searchFrame->GetFrame() );
    sl->Create();
    sl->SetText ("search target: ");
    this->DatabasesMenuButton = vtkKWMenuButton::New();
    this->DatabasesMenuButton->SetParent ( searchFrame->GetFrame() );
    this->DatabasesMenuButton->Create();
    this->DatabasesMenuButton->SetWidth (25);    
    this->BuildDatabasesMenu(this->DatabasesMenuButton->GetMenu() );
    this->SearchButton = vtkKWPushButton::New();
    this->SearchButton->SetParent ( f );
    this->SearchButton->Create();
    this->SearchButton->SetImageToIcon ( this->QueryAtlasIcons->GetSearchIcon() );
    this->SearchButton->SetBorderWidth ( 0 );
    this->SearchButton->SetReliefToFlat();
    this->SearchButton->SetBalloonHelpString ( "Perform a search" );
    app->Script ( "pack %s %s %s %s -side left -anchor w -padx 2 -pady 2",
                  this->ResultsWithAnyButton->GetWidgetName(),
                  this->ResultsWithAllButton->GetWidgetName(),
                  this->ResultsWithExactButton->GetWidgetName(),
                  this->SearchButton->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 0 -padx 0 -pady 2 -sticky w",
                 sl->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 1 -padx 0 -pady 2 -sticky w",
                 this->DatabasesMenuButton->GetWidgetName() );    
    app->Script ("grid %s -row 1 -column 1 -padx 0 -pady 2 -sticky w",
                 f->GetWidgetName() );

    f->Delete();
    sl->Delete();
    searchFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildResultsManagerGUI ()
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    // -------------------------------------------------------------------------------------------------
    // ---
    // QUERY RESULTS MANAGER FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
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
    vtkKWFrame *topcurF = vtkKWFrame::New();
    topcurF->SetParent ( resultsFrame->GetFrame() );
    topcurF->Create();
    vtkKWLabel *curL = vtkKWLabel::New();
    curL->SetParent ( topcurF );
    curL->Create();
    curL->SetWidth ( 45 );
    curL->SetText ( "Latest search results" );
    curL->SetBackgroundColor ( _br, _bg, _bb);
    this->CurrentResultsList = vtkKWListBoxWithScrollbars::New();
    this->CurrentResultsList->SetParent ( topcurF );
    this->CurrentResultsList->Create();
    this->CurrentResultsList->GetWidget()->SetWidth ( 45 );
    this->CurrentResultsList->GetWidget()->SetHeight (4 );
    this->CurrentResultsList->HorizontalScrollbarVisibilityOn();
    this->CurrentResultsList->VerticalScrollbarVisibilityOn();

    this->DeleteCurrentResultButton = vtkKWPushButton::New();
    this->DeleteCurrentResultButton->SetParent (curF);
    this->DeleteCurrentResultButton->Create();
    this->DeleteCurrentResultButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearSelectedIcon() );
    this->DeleteCurrentResultButton->SetBorderWidth ( 0 );
    this->DeleteCurrentResultButton->SetReliefToFlat();    
    this->DeleteCurrentResultButton->SetBalloonHelpString ( "Delete selected results");

    this->DeleteAllCurrentResultsButton = vtkKWPushButton::New();
    this->DeleteAllCurrentResultsButton->SetParent (curF);
    this->DeleteAllCurrentResultsButton->Create();
    this->DeleteAllCurrentResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearAllIcon() );
    this->DeleteAllCurrentResultsButton->SetBorderWidth ( 0 );
    this->DeleteAllCurrentResultsButton->SetReliefToFlat();    
    this->DeleteAllCurrentResultsButton->SetBalloonHelpString ("Delete all results ");

    this->SaveCurrentResultsButton = vtkKWPushButton::New();
    this->SaveCurrentResultsButton->SetParent (curF);
    this->SaveCurrentResultsButton->Create();
    this->SaveCurrentResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetReserveURIsIcon() );
    this->SaveCurrentResultsButton->SetBorderWidth ( 0 );
    this->SaveCurrentResultsButton->SetReliefToFlat();    
    this->SaveCurrentResultsButton->SetBalloonHelpString ("Reserve all results");

    this->SaveCurrentSelectedResultsButton = vtkKWPushButton::New();
    this->SaveCurrentSelectedResultsButton->SetParent (curF);
    this->SaveCurrentSelectedResultsButton->Create();
    this->SaveCurrentSelectedResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetReserveSelectedURIsIcon() );
    this->SaveCurrentSelectedResultsButton->SetBorderWidth ( 0 );
    this->SaveCurrentSelectedResultsButton->SetReliefToFlat();    
    this->SaveCurrentSelectedResultsButton->SetBalloonHelpString ("Reserve selected results");


    vtkKWFrame *pastF = vtkKWFrame::New();
    pastF->SetParent ( resultsFrame->GetFrame() );
    pastF->Create();
    vtkKWFrame *toppastF = vtkKWFrame::New();
    toppastF->SetParent ( resultsFrame->GetFrame() );
    toppastF->Create();
    vtkKWLabel *pastL = vtkKWLabel::New();
    pastL->SetParent ( toppastF );
    pastL->Create();
    pastL->SetWidth ( 45 );
    pastL->SetText ( "Reserved search results" );
    pastL->SetBackgroundColor ( 0.85, 0.85, 0.95 );
    this->PastResultsList = vtkKWListBoxWithScrollbars::New();
    this->PastResultsList->SetParent ( toppastF );
    this->PastResultsList->Create();
    this->PastResultsList->GetWidget()->SetWidth ( 45 );
    this->PastResultsList->GetWidget()->SetHeight ( 4 );
    this->PastResultsList->HorizontalScrollbarVisibilityOn();
    this->PastResultsList->VerticalScrollbarVisibilityOn();

    this->DeletePastResultButton = vtkKWPushButton::New();
    this->DeletePastResultButton->SetParent (pastF);
    this->DeletePastResultButton->Create();
    this->DeletePastResultButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearSelectedIcon ( ) );
    this->DeletePastResultButton->SetBorderWidth ( 0 );
    this->DeletePastResultButton->SetReliefToFlat ( );    
    this->DeletePastResultButton->SetBalloonHelpString ("Delete selected");

    this->DeleteAllPastResultsButton = vtkKWPushButton::New();
    this->DeleteAllPastResultsButton->SetParent (pastF);
    this->DeleteAllPastResultsButton->Create();
    this->DeleteAllPastResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearAllIcon (  ) );
    this->DeleteAllPastResultsButton->SetBorderWidth ( 0 );
    this->DeleteAllPastResultsButton->SetReliefToFlat();    
    this->DeleteAllPastResultsButton->SetBalloonHelpString ("Delete all");

    this->SavePastResultsButton = vtkKWPushButton::New();
    this->SavePastResultsButton->SetParent (pastF);
    this->SavePastResultsButton->Create();    
    this->SavePastResultsButton->SetImageToIcon (  app->GetApplicationGUI()->GetApplicationToolbar()->GetSlicerToolbarIcons()->GetSaveSceneIcon() );   
    this->SavePastResultsButton->SetBorderWidth ( 0 );
    this->SavePastResultsButton->SetReliefToFlat();    
    this->SavePastResultsButton->SetBalloonHelpString ("Save links to file");

    this->LoadURIsButton = vtkKWPushButton::New();
    this->LoadURIsButton->SetParent ( pastF);
    this->LoadURIsButton->Create();
    this->LoadURIsButton->SetImageToIcon ( app->GetApplicationGUI()->GetApplicationToolbar()->GetSlicerToolbarIcons()->GetLoadSceneIcon() );   
    this->LoadURIsButton->SetBorderWidth(0);
    this->LoadURIsButton->SetReliefToFlat ( );
    this->LoadURIsButton->SetBalloonHelpString ( "Load links from file" );

    app->Script( "pack %s -side top -padx 0 -pady 2 -fill both -expand 1", topcurF->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand 1", curL->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 0 -fill both -expand 1", this->CurrentResultsList->GetWidgetName() );

    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand 1", curF->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 0 -sticky ew -pady 4 -padx 3", this->DeleteCurrentResultButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 1 -sticky ew -pady 4 -padx 3", this->DeleteAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 2 -sticky ew -pady 4 -padx 3", this->SaveCurrentSelectedResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 3 -sticky ew -pady 4 -padx 3", this->SaveCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 0 -weight 1", this->DeleteCurrentResultButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 1 -weight 1", this->DeleteAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 2 -weight 1", this->SaveCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 3 -weight 1", this->SaveCurrentSelectedResultsButton->GetWidgetName() );    

    app->Script( "pack %s -side top -padx 0 -pady 2 -fill both -expand 1", toppastF->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand 1", pastL->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 0 -fill both -expand 1", this->PastResultsList->GetWidgetName() );

    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand 1", pastF->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 0 -sticky ew -pady 4 -padx 3", this->LoadURIsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 1 -sticky ew -pady 4 -padx 3", this->SavePastResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 2 -sticky ew -pady 4 -padx 3", this->DeletePastResultButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 3 -sticky ew -pady 4 -padx 3", this->DeleteAllPastResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 0 -weight 1", this->DeletePastResultButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 1 -weight 1", this->DeleteAllPastResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 2 -weight 1", this->SavePastResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 3 -weight 1", this->LoadURIsButton->GetWidgetName() );    

    curL->Delete();
    pastL->Delete();
    topcurF->Delete();
    toppastF->Delete();
    curF->Delete();
    pastF->Delete();
    resultsFrame->Delete();

}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildDisplayAndNavigationGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    // -------------------------------------------------------------------------------------------------
    // ---
    // 3D DISPLAY AND NAVIGATION FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *displayFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    displayFrame->SetParent ( page );
    displayFrame->Create ( );
    displayFrame->SetLabelText ("3D Display & Navigation");
    displayFrame->CollapseFrame ( );
    // for now supress this frame.
    /*
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  displayFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());
    */
     // ---
    // Source tcl files
    app->Script ( "source $::env(SLICER_HOME)/../Slicer3/Modules/QueryAtlas/Tcl/QueryAtlas.tcl; QueryAtlasInit");
    displayFrame->Delete();
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
    this->StructureListWidget = vtkQueryAtlasUseSearchTermWidget::New ( );
    this->StructureListWidget->SetParent ( this->StructureFrame );
    this->StructureListWidget->Create ( );
//    int i = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetColumnIndexWithName ( "Search terms" );
//    this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetColumnName ( i, "Structure terms");
    app->Script ( "pack %s -side top -fill x -expand true", this->StructureListWidget->GetWidgetName() );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildSpeciesFrame()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    this->SpeciesLabel = vtkKWLabel::New();
    this->SpeciesLabel->SetParent ( this->SpeciesFrame );
    this->SpeciesLabel->Create();
    this->SpeciesLabel->SetText( "species to include: ");
    
    this->SpeciesNoneButton = vtkKWCheckButton::New();
    this->SpeciesNoneButton->SetParent ( this->SpeciesFrame);
    this->SpeciesNoneButton->Create();
    this->SpeciesNoneButton->SetText ("don't specify");
    this->SpeciesNoneButton->SetSelectedState ( 1 );
    
    this->SpeciesHumanButton = vtkKWCheckButton::New();
    this->SpeciesHumanButton->SetParent ( this->SpeciesFrame);
    this->SpeciesHumanButton->Create();
    this->SpeciesHumanButton->SetText ("human");
    this->SpeciesHumanButton->SetSelectedState ( 0 );
    
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
    app->Script ( "grid %s -row 0 -column 1 -sticky w", this->SpeciesNoneButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1 -sticky w", this->SpeciesHumanButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 1 -sticky w", this->SpeciesMouseButton->GetWidgetName() );
    app->Script ( "grid %s -row 3 -column 1 -sticky w", this->SpeciesMacaqueButton->GetWidgetName() );
    
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
    this->DiagnosisMenuButton->GetLabel()->SetWidth (12);
    this->DiagnosisMenuButton->GetWidget()->SetWidth (28);
    this->BuildDiagnosisMenu ( this->DiagnosisMenuButton->GetWidget()->GetMenu() );
    this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddCommand ( "close" );

    this->AddDiagnosisEntry = vtkKWEntryWithLabel::New();
    this->AddDiagnosisEntry->SetParent ( this->PopulationFrame );
    this->AddDiagnosisEntry->Create();
    this->AddDiagnosisEntry->SetLabelText ( "add diagnosis: " );
    this->AddDiagnosisEntry->SetLabelPositionToLeft ( );
    this->AddDiagnosisEntry->GetLabel()->SetWidth(12);
    this->AddDiagnosisEntry->GetWidget()->SetWidth (35 );

    this->GenderMenuButton = vtkKWMenuButtonWithLabel::New();
    this->GenderMenuButton->SetParent ( this->PopulationFrame );
    this->GenderMenuButton->Create ();
    this->GenderMenuButton->SetLabelText ( "gender: ");
    this->GenderMenuButton->GetWidget()->SetWidth (28);
    this->GenderMenuButton->SetLabelPositionToLeft ( );
    this->GenderMenuButton->GetLabel()->SetWidth ( 12);
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
    this->HandednessMenuButton->GetWidget()->SetWidth (28);
    this->HandednessMenuButton->GetLabel()->SetWidth (12);
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
    this->AgeMenuButton->GetWidget()->SetWidth (28);
    this->AgeMenuButton->GetLabel()->SetWidth (12);
    this->AgeMenuButton->SetLabelPositionToLeft ( );
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("n/a");
    this->AgeMenuButton->GetWidget()->GetMenu()->SelectItem ("n/a");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("neonate");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("infant");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("child");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("adolescent");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("adult");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddRadioButton ("elderly");
    this->AgeMenuButton->GetWidget()->GetMenu()->AddSeparator();
    this->AgeMenuButton->GetWidget()->GetMenu()->AddCommand ( "close");    

    app->Script ( "pack %s %s %s %s %s -side top -padx 5 -pady 2 -anchor nw",
                  this->DiagnosisMenuButton->GetWidgetName(),
                  this->AddDiagnosisEntry->GetWidgetName(),
                  this->GenderMenuButton->GetWidgetName (),
                  this->HandednessMenuButton->GetWidgetName(),
                  this->AgeMenuButton->GetWidgetName());
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::UnpackLoaderContextFrames ( )
{
    this->Script ( "pack forget %s", this->FIPSFSFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->QdecFrame->GetWidgetName() );
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::PackLoaderContextFrame ( vtkKWFrame *f )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  app->Script ( "pack %s -side top -anchor nw -expand 0 -fill x -padx 2 -pady 0", f->GetWidgetName( ));
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildLoaderContextButtons ( vtkKWFrame *parent )
{
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();
  this->Script ("pack %s -side top -anchor nw -fill x -expand n", f->GetWidgetName() );

  this->FIPSFSButton = vtkKWPushButton::New();
  this->FIPSFSButton->SetParent ( f );
  this->FIPSFSButton->Create();
  this->FIPSFSButton->SetWidth ( 12 );
  this->FIPSFSButton->SetText ( "FIPS+FreeSurfer" );

  this->QdecButton = vtkKWPushButton::New();
  this->QdecButton->SetParent ( f );
  this->QdecButton->Create();
  this->QdecButton->SetWidth ( 12 );
  this->QdecButton->SetText ( "Qdec");

  this->Script ( "pack %s %s -anchor nw -side left -fill none -padx 2 -pady 2",
                 this->FIPSFSButton->GetWidgetName(),
                 this->QdecButton->GetWidgetName() );

  vtkSlicerPopUpHelpWidget *helpy = vtkSlicerPopUpHelpWidget::New();
  helpy->SetParent ( f  );
  helpy->Create ( );
  helpy->SetHelpText (" This is some sample help text. This is some more sample help text. Not so very helpful yet." );
  helpy->SetHelpTitle ( "Testing Popup Help" );
  this->Script ( "pack %s -side right -anchor ne -padx 2 -pady 2 -ipady 10", helpy->GetWidgetName() );
  helpy->Delete();

  f->Delete();
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildLoaderContextFrames ( vtkKWFrame *parent )
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
    
    this->FIPSFSFrame = vtkKWFrame::New();
    this->FIPSFSFrame->SetParent ( parent );
    this->FIPSFSFrame->Create();
    this->FIPSFSFrame->SetReliefToGroove();
    this->FIPSFSFrame->SetBorderWidth ( 1 );
    
    this->QdecFrame = vtkKWFrame::New();
    this->QdecFrame->SetParent ( parent );
    this->QdecFrame->Create();
    this->QdecFrame->SetReliefToGroove();
    this->QdecFrame->SetBorderWidth ( 1 );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ColorCodeLoaderContextButtons ( vtkKWPushButton *b )
{
  this->FIPSFSButton->SetBackgroundColor ( _br, _bg, _bb );
  this->QdecButton->SetBackgroundColor ( _br, _bg, _bb );

  this->FIPSFSButton->SetForegroundColor ( _fr, _fg, _fb );
  this->QdecButton->SetForegroundColor ( _fr, _fg, _fb );

  b->SetBackgroundColor (1.0, 1.0, 1.0);
  b->SetForegroundColor (0.0, 0.0, 0.0);
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildSubStructureFrame()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // add multi-column list box for search terms

    this->SubStructureListWidget = vtkQueryAtlasUseSearchTermWidget::New();
    this->SubStructureListWidget->SetParent ( this->SubStructureFrame );
    this->SubStructureListWidget->Create ( );
    this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->SetHeight(3);
//    int i = this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetColumnIndexWithName ( "Search terms" );
//    this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->SetColumnName ( i, "Other search terms");
    app->Script ( "pack %s -side top -fill x -expand true", this->SubStructureListWidget->GetWidgetName() );

}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::UnpackQueryBuilderContextFrames ( )
{
    this->Script ( "pack forget %s", this->SubStructureFrame->GetWidgetName() );
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

    this->SubStructureFrame = vtkKWFrame::New();
    this->SubStructureFrame->SetParent ( parent );
    this->SubStructureFrame->Create();

}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextButtons ( vtkKWFrame *parent )
{
  vtkKWFrame *f = vtkKWFrame::New();
  f->SetParent ( parent );
  f->Create();
  this->Script ("pack %s -side top -anchor nw -fill none", f->GetWidgetName() );

  // for now this will be the "other" term repository...
  // when we flesh this out with multiscale categories,
  // 
  this->SubStructureButton = vtkKWPushButton::New();
  this->SubStructureButton->SetParent ( f );
  this->SubStructureButton->Create();
  this->SubStructureButton->SetWidth ( 10 );
  this->SubStructureButton->SetText ( "other");
    
  this->StructureButton = vtkKWPushButton::New();
  this->StructureButton->SetParent ( f );
  this->StructureButton->Create();
  this->StructureButton->SetWidth ( 10);
  this->StructureButton->SetText ( "structure");
    
  this->PopulationButton = vtkKWPushButton::New();
  this->PopulationButton->SetParent ( f );
  this->PopulationButton->Create();
  this->PopulationButton->SetWidth ( 10 );
  this->PopulationButton->SetText ( "group");
    
  this->SpeciesButton = vtkKWPushButton::New();
  this->SpeciesButton->SetParent ( f );
  this->SpeciesButton->Create();
  this->SpeciesButton->SetWidth ( 10 );
  this->SpeciesButton->SetText ( "species");    

  this->Script ( "pack %s %s %s %s -anchor nw -side left -fill none -padx 2 -pady 2",
                 this->SubStructureButton->GetWidgetName(),
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
  m->AddRadioButton ("all");
  m->SelectItem ("all");
  m->AddSeparator();
  m->AddRadioButton ("Google");
  m->AddRadioButton ("Wikipedia");
  m->AddSeparator();
  m->AddRadioButton ("PubMed");
  m->AddRadioButton ("JNeurosci");
  m->AddRadioButton ("PLoS");
  m->AddSeparator();
  m->AddRadioButton ("Metasearch");
  m->AddRadioButton ("Entrez");
  m->AddSeparator();
  m->AddRadioButton ("IBVD");
  m->AddRadioButton ("BrainInfo");
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
  int i;
  int numrows;
  
  if ( !(strcmp (c, "structure") ))
    {
      numrows = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      for ( i = 0; i < numrows; i++ )
        {
        this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetCellText ( i, this->SelectionColumn, "1" );
        }
    }
  else if ( !(strcmp (c, "substructure")))
    {
    numrows = this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
    for ( i = 0; i < numrows; i++ )
      {
      this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->SetCellText ( i, this->SelectionColumn, "1" );
      }
    }
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeselectAllSearchTerms ( const char *c)
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Deselect All SearchTerms event. \n");  
  int i;
  int numrows;

  if ( !(strcmp(c, "structure")))
    {
      numrows = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      for ( i = 0; i < numrows; i++ )
        {
        this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetCellText ( i, this->SelectionColumn, "0" );
        }
    }
  else if ( !(strcmp(c, "substructure")))
    {
      numrows = this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      for ( i = 0; i < numrows; i++ )
        {
        this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->SetCellText ( i, this->SelectionColumn, "0" );
        }
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteAllSearchTerms ( const char *c)
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Clear All SearchTerms event. \n");
  int numrows;
  // remove each row

  if ( !(strcmp(c, "structure")))
    {
      numrows = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      this->StructureListWidget->GetMultiColumnList()->GetWidget()->DeleteAllRows();
    }
  else if ( !(strcmp(c, "substructure")))
    {
      numrows = this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->DeleteAllRows();
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddNewStructureSearchTerm ( const char *term )
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Adding New SearchTerms event. \n");
  int i = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
  this->StructureListWidget->GetMultiColumnList()->GetWidget()->InsertCellTextAsInt ( i, this->SelectionColumn, 0 );
  this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton (i, this->SelectionColumn );
  this->StructureListWidget->GetMultiColumnList()->GetWidget()->InsertCellText (i, this->SearchTermColumn, "edit search term here" );
  this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry (this->SearchTermColumn);
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddNewSearchTerm ( const char *c)
{
    // default search terms in list
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: Adding New SearchTerms event. \n");
  int i;
  if ( !(strcmp(c, "structure")))
    {
      i = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      this->StructureListWidget->GetMultiColumnList()->GetWidget()->InsertCellTextAsInt ( i, this->SelectionColumn, 0 );
      this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton (i, this->SelectionColumn );
      this->StructureListWidget->GetMultiColumnList()->GetWidget()->InsertCellText (i, this->SearchTermColumn, "edit search term here" );
      this->StructureListWidget->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry (this->SearchTermColumn);
    }
  else if ( !(strcmp(c, "substructure")))
    {
      i = this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfRows();
      this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->InsertCellTextAsInt ( i, this->SelectionColumn, 0 );
      this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->SetCellWindowCommandToCheckButton (i, this->SelectionColumn );
      this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->InsertCellText (i, this->SearchTermColumn, "edit search term here" );
      this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->SetColumnEditWindowToEntry (this->SearchTermColumn);
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteSelectedSearchTerms ( const char *c)
{
  vtkDebugMacro("vtkQueryAtlasGUI: ProcessGUIEvent: DeleteSearchTerm event\n");
  int numRows;
  int row[1];
  // get the row that was last selected and remove by index
  if ( !(strcmp(c, "structure")))
    {
      numRows = this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfSelectedRows();
      if (numRows == 1)
        {
        this->StructureListWidget->GetMultiColumnList()->GetWidget()->GetSelectedRows(row);
        this->StructureListWidget->GetMultiColumnList()->GetWidget()->DeleteRow ( row[0] );
        }
      else
        {
        vtkErrorMacro (<< "Selected rows (" << numRows << ") not 1, just pick one to delete for now\n");
        return;
        }
    }
  else if ( !(strcmp(c, "substructure")))
    {
      numRows = this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetNumberOfSelectedRows();
      if (numRows == 1)
        {
        this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->GetSelectedRows(row);
        this->SubStructureListWidget->GetMultiColumnList()->GetWidget()->DeleteRow ( row[0] );
        }
      else
        {
        vtkErrorMacro (<< "Selected rows (" << numRows << ") not 1, just pick one to delete for now\n");
        return;
        }
    }
}
