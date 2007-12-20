#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkCommand.h"

#include "vtkKWWidget.h"
#include "vtkKWPushButton.h"
#include "vtkKWPushButtonWithLabel.h"
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
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWMessageDialog.h"
#include "vtkKWTopLevel.h"

#include "vtkSlicerModelsGUI.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerColorLogic.h"
#include "vtkSlicerColorGUI.h"
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
#include "vtkSlicerWindow.h"

#include "vtkQdecModuleGUI.h"
#include "vtkQdecModuleLogic.h"


#include "vtkMRMLModelNode.h"
#include "vtkMRMLModelDisplayNode.h"
#include "vtkMRMLColorNode.h"
#include "vtkMRMLScalarVolumeNode.h"
#include "vtkMRMLScalarVolumeDisplayNode.h"
#include "vtkMRMLProceduralColorNode.h"

// for path manipulation
#include "itksys/SystemTools.hxx"


#define QUERIES_FRAME
#define SEARCHTERM_FRAME
#define RESULTS_FRAME
#define LOAD_FRAME
#define ANNO_FRAME
#define ONTOLOGY_FRAME

//---------------------------------------------------------------------------
vtkStandardNewMacro (vtkQueryAtlasGUI );
vtkCxxRevisionMacro ( vtkQueryAtlasGUI, "$Revision: 1.0 $");


#define _br 0.945
#define _bg 0.945
#define _bb 0.975

#define _fr 0.75
#define _fg 0.75
#define _fb 0.75


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::vtkQueryAtlasGUI ( )
{
    this->Logic = NULL;

    this->CollaboratorIcons = NULL;
    this->QueryAtlasIcons = NULL;
    this->ProcessingMRMLEvent = 0;
    this->SceneClosing = false;
    
#ifdef SEARCHTERM_FRAME
    //---
    // master category switch
    //---
    this->OtherButton = NULL;
    this->StructureButton = NULL;
    this->PopulationButton = NULL;
    this->SpeciesButton = NULL;
    this->SwitchQueryFrame = NULL;

    //---
    // search term other frame
    //---    
    this->OtherFrame = NULL;
    this->OtherListWidget = NULL;

    //---
    // search term species frame
    //---    
    this->SpeciesFrame = NULL;
    this->SpeciesLabel = NULL;
    this->SpeciesNoneButton = NULL;
    this->SpeciesHumanButton = NULL;
    this->SpeciesMouseButton = NULL;
    this->SpeciesMacaqueButton = NULL;

    //---
    // search term population frame
    //---
    this->PopulationFrame = NULL;
    this->DiagnosisMenuButton = NULL;
    this->GenderMenuButton = NULL;
    this->HandednessMenuButton = NULL;
    this->AgeMenuButton = NULL;
    this->AddDiagnosisEntry = NULL;

    //---
    // search term structure frame
    //---
    this->StructureFrame = NULL;
    this->StructureMenuButton = NULL;
    this->StructureListWidget = NULL;
#endif

    //---
    // annotation frame
    //---    
#ifdef ANNO_FRAME
    this->AnnotationTermSetMenuButton = NULL;
    this->QuerySceneVisibilityMenuButton = NULL;
#endif
    
    //---
    // query frame
    //---    
#ifdef QUERIES_FRAME
    this->SearchButton = NULL;
    this->UseStructureTerms = NULL;
    this->UseOtherTerms = NULL;
    this->UseGroupTerms = NULL;
    this->UseSpeciesTerms = NULL;
    this->DatabasesMenuButton = NULL;
    this->ResultsWithAnyButton = NULL;
    this->ResultsWithAllButton = NULL;
    this->ResultsWithExactButton = NULL;    
#endif

    //---
    // results frame
    //---
#ifdef RESULTS_FRAME
    this->CurrentResultsList = NULL;
    this->AccumulatedResultsList = NULL;
    this->DeleteAllCurrentResultsButton = NULL;
    this->DeleteCurrentResultButton = NULL;
    this->DeselectAllCurrentResultsButton = NULL;
    this->SelectAllCurrentResultsButton = NULL;
    this->DeselectAllAccumulatedResultsButton = NULL;
    this->SelectAllAccumulatedResultsButton = NULL;
    this->DeleteAllAccumulatedResultsButton = NULL;
    this->DeleteAccumulatedResultButton = NULL;
    this->SaveCurrentResultsButton = NULL;
    this->SaveCurrentSelectedResultsButton = NULL;
    this->SaveAccumulatedResultsButton = NULL;
    this->LoadURIsButton = NULL;
#endif
    this->NumberOfColumns = 2;

    //---
    // ontology frame
    //---
#ifdef ONTOLOGY_FRAME
    this->LocalSearchTermEntry = NULL;
    this->SynonymsMenuButton = NULL;
    this->BIRNLexEntry = NULL;
    this->BIRNLexIDEntry = NULL;
    this->NeuroNamesEntry = NULL;
    this->NeuroNamesIDEntry = NULL;
    this->UMLSCIDEntry = NULL;
    this->UMLSCNEntry = NULL;
    this->AddLocalTermButton = NULL;
    this->AddSynonymButton = NULL;
    this->AddBIRNLexStringButton = NULL;
    this->AddBIRNLexIDButton = NULL;
    this->AddNeuroNamesStringButton = NULL;
    this->AddNeuroNamesIDButton = NULL;
    this->AddUMLSCIDButton = NULL;
    this->AddUMLSCNButton = NULL;
    this->BIRNLexHierarchyButton = NULL;
    this->NeuroNamesHierarchyButton = NULL;
    this->UMLSHierarchyButton = NULL;
    this->SavedTerms = NULL;
#endif

    //---
    // load frame
    //---    
#ifdef LOAD_FRAME
    this->FIPSFSButton = NULL;
    this->FIPSFSFrame = NULL;
    this->BasicAnnotateButton = NULL;
    this->GeneralFrame = NULL;
    this->GeneralButton = NULL;
    this->QdecButton = NULL;
    this->QdecFrame = NULL;
    this->FSasegSelector = NULL;
    this->FSbrainSelector = NULL;
    this->FSstatsSelector = NULL;
    this->FSgoButton = NULL;
    this->QdecGetResultsButton = NULL;
    this->LoadFIPSFSCatalogButton = NULL;
    this->QdecScalarSelector = NULL;
    this->QdecGoButton = NULL;
#endif
}


//---------------------------------------------------------------------------
vtkQueryAtlasGUI::~vtkQueryAtlasGUI ( )
{

  vtkDebugMacro("vtkQueryAtlasGUI: Tearing down Tcl callbacks \n");
  this->Script ( "QueryAtlasTearDown" );

  this->RemoveMRMLObservers ( );
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
#ifdef LOAD_FRAME
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
    if ( this->BasicAnnotateButton)
      {
      this->BasicAnnotateButton->SetParent ( NULL );
      this->BasicAnnotateButton->Delete();
      this->BasicAnnotateButton = NULL;      
      }
    if ( this->GeneralFrame)
      {
      this->GeneralFrame->SetParent ( NULL);
      this->GeneralFrame->Delete();
      this->GeneralFrame = NULL;
      }
    if ( this->GeneralButton )
      {
      this->GeneralButton->SetParent ( NULL );
      this->GeneralButton->Delete();
      this->GeneralButton = NULL;
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
    if ( this->FSgoButton )
      {
      this->FSgoButton->SetParent ( NULL );
      this->FSgoButton->Delete();
      this->FSgoButton = NULL;
      }
    if ( this->QdecGoButton )
      {
      this->QdecGoButton->SetParent ( NULL );
      this->QdecGoButton->Delete();
      this->QdecGoButton = NULL;
      }
    if ( this->FSstatsSelector )
      {
      this->FSstatsSelector->SetParent ( NULL );
      this->FSstatsSelector->Delete();
      this->FSstatsSelector = NULL;
      }
    if ( this->FSbrainSelector )
      {
      this->FSbrainSelector->SetParent ( NULL );
      this->FSbrainSelector->Delete();
      this->FSbrainSelector = NULL;
      }
    if ( this->FSasegSelector )
      {
      this->FSasegSelector->SetParent ( NULL );
      this->FSasegSelector->Delete();
      this->FSasegSelector = NULL;
      }
    if ( this->LoadFIPSFSCatalogButton )
      {
      this->LoadFIPSFSCatalogButton->SetParent ( NULL );
      this->LoadFIPSFSCatalogButton->Delete();
      this->LoadFIPSFSCatalogButton = NULL;      
      }
    if ( this->QdecGetResultsButton )
      {
      this->QdecGetResultsButton->SetParent ( NULL );
      this->QdecGetResultsButton->Delete();
      this->QdecGetResultsButton = NULL;
      }
    if ( this->QdecScalarSelector )
      {
      this->QdecScalarSelector->SetParent ( NULL );
      this->QdecScalarSelector->Delete();
      this->QdecScalarSelector = NULL;
      }
#endif


    //---
    // annotation frame
    //---
#ifdef ANNO_FRAME
    if ( this->AnnotationTermSetMenuButton )
      {
      this->AnnotationTermSetMenuButton->SetParent ( NULL );
      this->AnnotationTermSetMenuButton->Delete();
      this->AnnotationTermSetMenuButton = NULL;      
      }
    if ( this->QuerySceneVisibilityMenuButton )
      {
      this->QuerySceneVisibilityMenuButton->SetParent ( NULL );
      this->QuerySceneVisibilityMenuButton->Delete();
      this->QuerySceneVisibilityMenuButton = NULL;      
      }
#endif
    
    //---
    // search term frame
    //---
#ifdef SEARCHTERM_FRAME
    if ( this->SwitchQueryFrame)
      {
      this->SwitchQueryFrame->SetParent ( NULL );
      this->SwitchQueryFrame->Delete();
      this->SwitchQueryFrame = NULL;
      }
    if ( this->OtherButton )
      {
      this->OtherButton->SetParent ( NULL );
      this->OtherButton->Delete();
      this->OtherButton = NULL;   
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
    // search term population panel
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
    // search term species panel
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
    // search term structure panel
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
    // search term structure panel
    //---
    if ( this->StructureListWidget )
      {
      this->StructureListWidget->SetParent ( NULL );
      this->StructureListWidget->Delete ( );
      this->StructureListWidget = NULL;
      }
    //---
    // search term substructure panel
    //---
    if ( this->OtherFrame )
      {
      this->OtherFrame->SetParent ( NULL );
      this->OtherFrame->Delete();
      this->OtherFrame = NULL;      
      }
    if ( this->OtherListWidget )
      {
      this->OtherListWidget->SetParent ( NULL );
      this->OtherListWidget->Delete();
      this->OtherListWidget = NULL;      
      }

#endif

    //---
    // query panel
    //---
#ifdef QUERIES_FRAME
    if ( this->UseOtherTerms )
      {
      this->UseOtherTerms->SetParent ( NULL );
      this->UseOtherTerms->Delete();
      this->UseOtherTerms = NULL;
      }
    if ( this->UseStructureTerms )
      {
      this->UseStructureTerms->SetParent ( NULL);
      this->UseStructureTerms->Delete();
      this->UseStructureTerms = NULL;      
      }
    if ( this->UseGroupTerms )
      {
      this->UseGroupTerms->SetParent ( NULL );
      this->UseGroupTerms->Delete();
      this->UseGroupTerms = NULL;
      }
    if ( this->UseSpeciesTerms )
      {
      this->UseSpeciesTerms->SetParent ( NULL );
      this->UseSpeciesTerms->Delete();
      this->UseSpeciesTerms = NULL;      
      }
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
#endif


    //---
    // ontology frame
    //---
#ifdef ONTOLOGY_FRAME
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
    if ( this->UMLSCNEntry)
      {
      this->UMLSCNEntry->SetParent(NULL);
      this->UMLSCNEntry->Delete();
      this->UMLSCNEntry = NULL;
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
    if ( this->AddUMLSCNButton)
      {
      this->AddUMLSCNButton->SetParent( NULL );
      this->AddUMLSCNButton->Delete();
      this->AddUMLSCNButton = NULL;
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
#endif


    //---
    // results panel
    //---
#ifdef RESULTS_FRAME
    if ( this->CurrentResultsList )
      {
      this->CurrentResultsList->SetParent(NULL);
      this->CurrentResultsList->Delete();
      this->CurrentResultsList = NULL;
      }
    if ( this->AccumulatedResultsList )
      {
      this->AccumulatedResultsList->SetParent(NULL);
      this->AccumulatedResultsList->Delete();
      this->AccumulatedResultsList = NULL;
      }
    if ( this->DeselectAllCurrentResultsButton )
      {
      this->DeselectAllCurrentResultsButton->SetParent ( NULL );      
      this->DeselectAllCurrentResultsButton->Delete();
      this->DeselectAllCurrentResultsButton = NULL;      
      }
    if ( this->SelectAllCurrentResultsButton )
      {
      this->SelectAllCurrentResultsButton->SetParent ( NULL );
      this->SelectAllCurrentResultsButton->Delete();
      this->SelectAllCurrentResultsButton = NULL;
      }
    if ( this->SelectAllAccumulatedResultsButton )
      {
      this->SelectAllAccumulatedResultsButton->SetParent ( NULL );
      this->SelectAllAccumulatedResultsButton->Delete();
      this->SelectAllAccumulatedResultsButton = NULL;      
      }
    if ( this->DeselectAllAccumulatedResultsButton )
      {
      this->DeselectAllAccumulatedResultsButton->SetParent ( NULL );      
      this->DeselectAllAccumulatedResultsButton->Delete();
      this->DeselectAllAccumulatedResultsButton = NULL;      
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
    if ( this->DeleteAccumulatedResultButton )
      {
      this->DeleteAccumulatedResultButton->SetParent(NULL);
      this->DeleteAccumulatedResultButton->Delete();
      this->DeleteAccumulatedResultButton = NULL;
      }
    if ( this->DeleteAllAccumulatedResultsButton )
      {
      this->DeleteAllAccumulatedResultsButton->SetParent(NULL);
      this->DeleteAllAccumulatedResultsButton->Delete();
      this->DeleteAllAccumulatedResultsButton = NULL;
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
    if ( this->SaveAccumulatedResultsButton )
      {
      this->SaveAccumulatedResultsButton->SetParent(NULL);
      this->SaveAccumulatedResultsButton->Delete();
      this->SaveAccumulatedResultsButton = NULL;
      }
    if ( this->LoadURIsButton )
      {
      this->LoadURIsButton->SetParent ( NULL );
      this->LoadURIsButton->Delete();
      this->LoadURIsButton = NULL;
      }
#endif

}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::OpenOntologyBrowser()
{
  this->Script ( "QueryAtlasLaunchOntologyBrowser" );
}




//---------------------------------------------------------------------------
vtkDataArray* vtkQueryAtlasGUI::AssignCellColorCode (
                                                     int numCells,
                                                     int nextCellIndex,
                                                     vtkDataArray *cellNumberColors)
{
  int val, r, g, b;
  int mult = 256*256;
  cellNumberColors->Initialize();
  cellNumberColors->SetNumberOfComponents (4);
  if ( nextCellIndex >= 0 )
    {
    for ( int i=0; i<numCells; i++ )
      {
      val = i + nextCellIndex;
      val = val+1;
      r = val / (mult);
      val = val % (mult);
      g = val / 256;
      b = val % 256;
      cellNumberColors->InsertNextTuple4 ( (double)r, (double)g, (double)b, 255.0 );
      }
    }
  return (cellNumberColors);
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
#ifdef LOAD_FRAME
    os << indent << "FSasegSelector: " << this->GetFSasegSelector ( ) << "\n";    
    os << indent << "FSbrainSelector: " << this->GetFSbrainSelector ( ) << "\n";    
    os << indent << "FSstatsSelector: " << this->GetFSstatsSelector ( ) << "\n";    
    os << indent << "FSgoButton: " << this->GetFSgoButton() << "\n";
    os << indent << "QdecGoButton: " << this->GetQdecGoButton() << "\n";
    os << indent << "QdecGetResultsButton: " << this->GetQdecGetResultsButton ( ) << "\n";    
    os << indent << "LoadFIPSFSCatalogButton: " << this->GetLoadFIPSFSCatalogButton ( ) << "\n";    
    os << indent << "QdecScalarSelector: " << this->GetQdecScalarSelector ( ) << "\n";    
#endif
    
    //---
    // ontology frame
    //---
#ifdef ONTOLOGY_FRAME
    os << indent << "LoadSearchTermEntry" << this->GetLocalSearchTermEntry ( ) << "\n";    
    os << indent << "SynonymsMenuButton" << this->GetSynonymsMenuButton ( ) << "\n";    
    os << indent << "BIRNLexEntry" << this->GetBIRNLexEntry ( ) << "\n";    
    os << indent << "BIRNLexIDEntry" << this->GetBIRNLexIDEntry ( ) << "\n";    
    os << indent << "NeuroNamesEntry" << this->GetNeuroNamesEntry ( ) << "\n";    
    os << indent << "NeuroNamesIDEntry" << this->GetNeuroNamesIDEntry ( ) << "\n";    
    os << indent << "UMLSCIDEntry" << this->GetUMLSCIDEntry ( ) << "\n";    
    os << indent << "UMLSCNEntry" << this->GetUMLSCNEntry ( ) << "\n";    
    os << indent << "AddLocalTermButton" << this->GetAddLocalTermButton ( ) << "\n";    
    os << indent << "AddSynonymButton" << this->GetAddSynonymButton ( ) << "\n";    
    os << indent << "AddBIRNLexStringButton" << this->GetAddBIRNLexStringButton ( ) << "\n";    
    os << indent << "AddBIRNLexIDButton" << this->GetAddBIRNLexIDButton ( ) << "\n";    
    os << indent << "AddNeuroNamesStringButton" << this->GetAddNeuroNamesStringButton ( ) << "\n";    
    os << indent << "AddNeuroNamesIDButton" << this->GetAddNeuroNamesIDButton ( ) << "\n";    
    os << indent << "AddUMLSCIDButton" << this->GetAddUMLSCIDButton ( ) << "\n";    
    os << indent << "AddUMLSCNButton" << this->GetAddUMLSCNButton ( ) << "\n";    
    os << indent << "BIRNLexHierarchyButton" << this->GetBIRNLexHierarchyButton ( ) << "\n";    
    os << indent << "NeuroNamesHierarchyButton" << this->GetNeuroNamesHierarchyButton ( ) << "\n";    
    os << indent << "UMLSHierarchyButton" << this->GetUMLSHierarchyButton ( ) << "\n";    
    os << indent << "SavedTerms" << this->GetSavedTerms() << "\n";
    os << indent << "AddDiagnosisEntry" << this->GetAddDiagnosisEntry() << "\n";
    os << indent << "ResultsWithExactButton" <<  this->GetResultsWithExactButton() << "\n";
    os << indent << "ResultsWithAnyButton" <<  this->GetResultsWithAnyButton() << "\n";
    os << indent << "ResultsWithAllButton" <<  this->GetResultsWithAllButton() << "\n";
#endif
    
    //---
    // TODO: finish this method!
    //---
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::RemoveGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: RemoveGUIObservers\n");

  //--- all the KWLoadSaveDialogs
  this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->RemoveObservers(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->RemoveObservers(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadURIsButton->GetLoadSaveDialog()->RemoveObservers(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);

  this->BasicAnnotateButton->GetWidget()->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FIPSFSButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeneralButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );    
  this->QdecButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSasegSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSbrainSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSstatsSelector->RemoveObservers ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSgoButton->GetWidget()->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QdecGoButton->GetWidget()->RemoveObservers ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QdecScalarSelector->GetWidget()->GetMenu()->RemoveObservers ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->AddLocalTermButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddSynonymButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddBIRNLexStringButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddBIRNLexIDButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddNeuroNamesStringButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddNeuroNamesIDButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddUMLSCIDButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddUMLSCNButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->SavedTerms->RemoveWidgetObservers();
  this->SavedTerms->RemoveObservers(vtkQueryAtlasSearchTermWidget::ReservedTermsEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->StructureButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->RemoveWidgetObservers();
  this->OtherButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OtherListWidget->RemoveWidgetObservers();
  this->SpeciesButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesNoneButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesHumanButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMouseButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMacaqueButton->RemoveObservers(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->AnnotationTermSetMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QuerySceneVisibilityMenuButton->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->BIRNLexHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NeuroNamesHierarchyButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LocalSearchTermEntry->RemoveObservers(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->PopulationButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->RemoveObservers(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddDiagnosisEntry->GetWidget()->RemoveObservers ( vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SearchButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );

#ifdef RESULTS_FRAME
  this->DeselectAllCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeselectAllAccumulatedResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllAccumulatedResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveCurrentSelectedResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAccumulatedResultButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllAccumulatedResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteCurrentResultButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllCurrentResultsButton->RemoveObservers(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
#endif

}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddGUIObservers ( )
{
  vtkDebugMacro("vtkQueryAtlasGUI: AddGUIObservers\n");

  //--- all the KWLoadSaveDialogs
  this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->AddObserver(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->AddObserver(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->AddObserver(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
  this->LoadURIsButton->GetLoadSaveDialog()->AddObserver(vtkKWTopLevel::WithdrawEvent, (vtkCommand *)this->GUICallbackCommand);
    
  this->BasicAnnotateButton->GetWidget()->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->FIPSFSButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GeneralButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->QdecButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSasegSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSbrainSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSstatsSelector->AddObserver ( vtkSlicerNodeSelectorWidget::NodeSelectedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->FSgoButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QdecGoButton->GetWidget()->AddObserver ( vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QdecScalarSelector->GetWidget()->GetMenu()->AddObserver ( vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->AddLocalTermButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddSynonymButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddBIRNLexStringButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddBIRNLexIDButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddNeuroNamesStringButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddNeuroNamesIDButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddUMLSCIDButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->AddUMLSCNButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );  
  this->SavedTerms->AddWidgetObservers();
  this->SavedTerms->AddObserver(vtkQueryAtlasSearchTermWidget::ReservedTermsEvent, (vtkCommand *)this->GUICallbackCommand );  

  this->StructureButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->StructureListWidget->AddWidgetObservers();
  this->OtherButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->OtherListWidget->AddWidgetObservers();
  this->SpeciesButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesNoneButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesHumanButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMouseButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SpeciesMacaqueButton->AddObserver(vtkKWCheckButton::SelectedStateChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->AnnotationTermSetMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->QuerySceneVisibilityMenuButton->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
  this->BIRNLexHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->NeuroNamesHierarchyButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->LocalSearchTermEntry->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->PopulationButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DiagnosisMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->HandednessMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->GenderMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AgeMenuButton->GetWidget()->GetMenu()->AddObserver(vtkKWMenu::MenuItemInvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->AddDiagnosisEntry->GetWidget()->AddObserver(vtkKWEntry::EntryValueChangedEvent, (vtkCommand *)this->GUICallbackCommand );

  this->SearchButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  
#ifdef RESULTS_FRAME
  this->DeselectAllCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeselectAllAccumulatedResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SelectAllAccumulatedResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->SaveCurrentSelectedResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAccumulatedResultButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllAccumulatedResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteCurrentResultButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
  this->DeleteAllCurrentResultsButton->AddObserver(vtkKWPushButton::InvokedEvent, (vtkCommand *)this->GUICallbackCommand );
#endif
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
  vtkSlicerNodeSelectorWidget *sel = vtkSlicerNodeSelectorWidget::SafeDownCast ( caller );
  vtkQueryAtlasSearchTermWidget *stw = vtkQueryAtlasSearchTermWidget::SafeDownCast (caller );
  vtkKWLoadSaveDialog *lsdialog = vtkKWLoadSaveDialog::SafeDownCast ( caller );
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkMRMLNode *node;

  if ( app ==  NULL )
    {
    return;
    }

  
  //--- check all LoadSaveDialogs
  if ( lsdialog != NULL )
    {
    if ( (lsdialog == this->LoadURIsButton->GetLoadSaveDialog() ) && (event == vtkKWTopLevel::WithdrawEvent ) )
      {
      this->LoadBookmarksCallback();
      }
    else if  ( (lsdialog == this->SaveAccumulatedResultsButton->GetLoadSaveDialog() ) && (event == vtkKWTopLevel::WithdrawEvent ) )
      {
      this->WriteBookmarksCallback();
      }
    else if  ( (lsdialog == this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog() ) && (event == vtkKWTopLevel::WithdrawEvent ) )
      {
      this->LoadXcedeCatalogCallback();
      }
    else if  ( (lsdialog == this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog() ) && (event == vtkKWTopLevel::WithdrawEvent ) )
      {
      this->LoadQdecResultsCallback();
      }    
    }

  if ( (stw = this->SavedTerms) && (event == vtkQueryAtlasSearchTermWidget::ReservedTermsEvent ))
    {
    // test
    int num = this->SavedTerms->GetNumberOfReservedTerms();
    const char *term;
    for (int i=0; i<num; i++)
      {
      term = this->SavedTerms->GetNthReservedTerm ( i );
      this->StructureListWidget->AddNewSearchTerm ( term );
      }
    }

  
  if ( sel != NULL )
    {
    if ((sel == this->FSasegSelector ) && ( event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) )
      {
      node = sel->GetSelected();
      if ( node != NULL )
        {
        this->Script ( "QueryAtlasSetAnnotatedLabelMap" );
        }
      }
    else if ((sel == this->FSbrainSelector ) && ( event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) )
      {
      node = sel->GetSelected();
      if ( node != NULL )
        {
        this->Script ( "QueryAtlasSetAnatomical" );
        }
      }
    else if ((sel == this->FSstatsSelector ) && ( event == vtkSlicerNodeSelectorWidget::NodeSelectedEvent ) )
      {
      node = sel->GetSelected();
      if ( node != NULL )
        {
        this->Script ( "QueryAtlasSetStatistics" );
        }
      }
    }
  

  //---
  //--- Process All Entry events
  //---
  if ( e != NULL )
    {
    if ( this->AddDiagnosisEntry )
      {
      if ( (e == this->AddDiagnosisEntry->GetWidget() ) && ( event == vtkKWEntry::EntryValueChangedEvent ))
        {
        if ( strcmp (this->AddDiagnosisEntry->GetWidget()->GetValue(), "" ) ) 
          {
          this->AddToDiagnosisMenu ( this->DiagnosisMenuButton->GetWidget()->GetMenu(),
                                     this->AddDiagnosisEntry->GetWidget()->GetValue() );
          }
        }
      }

    if ( (e == this->LocalSearchTermEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if ( this->LocalSearchTermEntry->GetValue() )
        {
        if ( strcmp ( this->LocalSearchTermEntry->GetValue(), "" )) 
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s local", this->LocalSearchTermEntry->GetValue() );
          }
        }
      }
    else if ( (e == this->BIRNLexEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if (this->BIRNLexEntry->GetValue() )
        {
        if ( strcmp ( this->BIRNLexEntry->GetValue(), "" ))
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s BIRN_String", this->BIRNLexEntry->GetValue() );
          }
        }
      }
    else if ( (e == this->BIRNLexIDEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if ( this->BIRNLexIDEntry->GetValue() )
        {
        if ( strcmp (this->BIRNLexIDEntry->GetValue(), "" ))
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s BIRN_ID", this->BIRNLexIDEntry->GetValue() );
          }
        }
      }
    else if ( (e == this->NeuroNamesEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if (this->NeuroNamesEntry->GetValue() )
        {
        if  (strcmp (this->NeuroNamesEntry->GetValue(), "" ))
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s NN", this->NeuroNamesEntry->GetValue() );
          }
        }
      }
    else if ( (e == this->NeuroNamesIDEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if ( this->NeuroNamesIDEntry->GetValue() )
        {
        if ( strcmp (this->NeuroNamesIDEntry->GetValue(), "" ))
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s NN_ID", this->NeuroNamesIDEntry->GetValue() );
          }
        }
      }
    else if ( (e == this->UMLSCIDEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if ( this->UMLSCIDEntry->GetValue() )
        {
        if ( strcmp (this->UMLSCIDEntry->GetValue(), "" ))
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s UMLS_CID", this->UMLSCIDEntry->GetValue() );
          }
        }
      }
    else if ( (e == this->UMLSCNEntry) && (event == vtkKWEntry::EntryValueChangedEvent) )
      {
      if ( this->UMLSCNEntry->GetValue() )
        {
        if ( strcmp (this->UMLSCNEntry->GetValue(), "" ))
          {
          this->Script ("QueryAtlasPopulateOntologyInformation %s UMLS_CN", this->UMLSCNEntry->GetValue() );
          }
        }
      }
    }

  //---
  //--- Process All PushButton events
  //---
  if ( b != NULL )
    {
    if ( this->FSgoButton)
      {
      if ( (b == this->FSgoButton->GetWidget()) && (event == vtkKWPushButton::InvokedEvent ) )
        {
        this->Script ( "QueryAtlasInitialize FIPSFreeSurfer NULL" );
        // do this here again in case node added events haven't triggered
        // the update of this menu, which they seem not to do for command
        // line loading.
        this->Script ( "QueryAtlasAnnotationVisibility on");
        this->UpdateAnnoVisibilityMenu();
        }
      }
    if ( this->QdecGoButton )
      {
      if ( (b == this->QdecGoButton->GetWidget()) && (event == vtkKWPushButton::InvokedEvent ) )
        {
        vtkQdecModuleLogic *qLogic = vtkQdecModuleGUI::SafeDownCast(app->GetModuleGUIByName("QdecModule"))->GetLogic();
        if ( qLogic != NULL )
          {
          std::vector<std::string> pathcomponents;
          std::string subjDir = qLogic->GetSubjectsDirectory();
          itksys::SystemTools::SplitPath(subjDir.c_str(), pathcomponents);
          std::string avgDir = qLogic->GetAverageSubject ( );
          pathcomponents.push_back(avgDir.c_str() );
          pathcomponents.push_back("label" );
          std::string labelDir = "";
          labelDir = itksys::SystemTools::JoinPath(pathcomponents);
          this->Script ( "QueryAtlasInitialize Qdec \"%s\"", labelDir.c_str() );
          // do this here again in case node added events haven't triggered
          // the update of this menu, which they seem not to do for command
          // line loading
          this->Script ( "QueryAtlasAnnotationVisibility on");
          this->UpdateAnnoVisibilityMenu();
          }
        }
      }
    if ( (b == this->NeuroNamesHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      //--- get last clicked (or typed) structure from the LocalSearchTermEntry
      const char *structureLabel =  this->NeuroNamesEntry->GetValue();
      if ( strcmp (structureLabel, "" ))
        {
        this->Script ( "QueryAtlasSendOntologyCommand  \"%s\" NN", structureLabel  );
        }

      }
    else if ( (b == this->BIRNLexHierarchyButton) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      //--- get last clicked (or typed) structure from the LocalSearchTermEntry
      const char *structureLabel =  this->BIRNLexEntry->GetValue();
      if ( strcmp (structureLabel, "" ))
        {
        this->Script ( "QueryAtlasSendOntologyCommand  \"%s\" BIRN", structureLabel  );
        }
      }
    else if ( (b == this->AddLocalTermButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->LocalSearchTermEntry->GetValue() );
      }
    else if ( (b == this->AddSynonymButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->SynonymsMenuButton->GetValue() );
      }
    else if ( (b == this->AddBIRNLexStringButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->BIRNLexEntry->GetValue() );
      }
    else if ( (b == this->AddBIRNLexIDButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->BIRNLexIDEntry->GetValue() );
      }
    else if ( (b == this->AddNeuroNamesStringButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->NeuroNamesEntry->GetValue() );
      }
    else if ( (b == this->AddNeuroNamesIDButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->NeuroNamesIDEntry->GetValue() );
      }
    else if ( (b == this->AddUMLSCIDButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->UMLSCIDEntry->GetValue() );
      }
    else if ( (b == this->AddUMLSCNButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->SavedTerms->AddTerm (this->UMLSCNEntry->GetValue() );
      }
    else if ( (b == this->SearchButton) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      if ( this->DatabasesMenuButton->IsCreated() )
        {
        this->Script ( "QueryAtlasFormURLsForTargets");
        }
      }
    else if ( (b == this->FIPSFSButton) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->UnpackLoaderContextFrames();
      this->PackLoaderContextFrame ( this->FIPSFSFrame );
      this->ColorCodeLoaderContextButtons ( this->FIPSFSButton );
      }
    else if ( (b == this->BasicAnnotateButton->GetWidget()) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->Script ( "QueryAtlasInitialize NULL NULL" );
      this->Script ( "QueryAtlasAnnotationVisibility on");
      this->UpdateAnnoVisibilityMenu();
      }
    else if ( (b == this->GeneralButton) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->UnpackLoaderContextFrames();
      this->PackLoaderContextFrame ( this->GeneralFrame );
      this->ColorCodeLoaderContextButtons ( this->GeneralButton );
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
    else if ( (b == this->OtherButton) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->UnpackQueryBuilderContextFrames();
      this->PackQueryBuilderContextFrame ( this->OtherFrame);    
      this->ColorCodeContextButtons ( this->OtherButton );
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

    else if ( (b == this->DeselectAllCurrentResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      int num = this->CurrentResultsList->GetWidget()->GetNumberOfRows();
      for ( int i=0; i<num; i++ )
        {
        this->CurrentResultsList->GetWidget()->DeselectCell(i,1);
        }
      }
    else if ( (b == this->DeleteCurrentResultButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedResults( this->CurrentResultsList->GetWidget() );
      }
    else if ( (b == this->DeleteAllCurrentResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllResults( this->CurrentResultsList->GetWidget() ); 
      }
    else if ( (b == this->SaveCurrentResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      int num = this->CurrentResultsList->GetWidget()->GetNumberOfRows();
      for ( int i=0; i<num; i++ )
        {
        std::string result (this->CurrentResultsList->GetWidget()->GetCellText(i,1));
        this->AccumulateUniqueResult (result.c_str() );
        }
      }
    else if ( (b == this->SaveCurrentSelectedResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      int num = this->CurrentResultsList->GetWidget()->GetNumberOfRows();
      for ( int i=0; i<num; i++ )
        {
        if ( this->CurrentResultsList->GetWidget()->IsRowSelected(i) )
          {
          std::string result (this->CurrentResultsList->GetWidget()->GetCellText(i,1));
          this->AccumulateUniqueResult (result.c_str());
          }
        }
      }
    else if (( b== this->SelectAllCurrentResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      int num = this->CurrentResultsList->GetWidget()->GetNumberOfRows();
      for ( int i=0; i<num; i++ )
        {
        this->CurrentResultsList->GetWidget()->SelectCell(i,1);
        }
      }

    else if ( (b == this->SelectAllAccumulatedResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      int num = this->AccumulatedResultsList->GetWidget()->GetNumberOfRows();
      for ( int i=0; i<num; i++ )
        {
        this->AccumulatedResultsList->GetWidget()->SelectCell(i,1);
        }
      }
    else if ( (b == this->DeselectAllAccumulatedResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      int num = this->AccumulatedResultsList->GetWidget()->GetNumberOfRows();
      for ( int i=0; i<num; i++ )
        {
        this->AccumulatedResultsList->GetWidget()->DeselectCell(i,1);
        }
      }
    else if ( (b == this->DeleteAccumulatedResultButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteSelectedResults ( this->AccumulatedResultsList->GetWidget() );
      }
    else if ( (b == this->DeleteAllAccumulatedResultsButton ) && (event == vtkKWPushButton::InvokedEvent ) )
      {
      this->DeleteAllResults ( this->AccumulatedResultsList->GetWidget() );
      }
    }

  //---
  //--- Process menu selections
  //---
  // no need to do anything here; we'll just grab the widget values when we need them with tcl
  if ( m != NULL )
    {
    if ( (m==this->QuerySceneVisibilityMenuButton->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent) )
      {
      this->ModifyQuerySceneVisibility();
      }
    else if ( this->AnnotationTermSetMenuButton )
      {
      if (( m== this->AnnotationTermSetMenuButton->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent ) )
        {
        const char *val = this->AnnotationTermSetMenuButton->GetValue();
        if ( !strcmp( val, "local identifier" ) )
          {
          this->Script ( "QueryAtlasSetAnnotationTermSet local" );
          }
        else if (!strcmp( val, "BIRNLex String" ) )
          {
          this->Script ( "QueryAtlasSetAnnotationTermSet BIRNLex" );
          }
        else if (!strcmp( val, "NeuroNames String" ) )
          {
          this->Script ( "QueryAtlasSetAnnotationTermSet NeuroNames" );
          }
        else if (!strcmp( val, "UMLS CName" ) )
          {
          this->Script ( "QueryAtlasSetAnnotationTermSet UMLS" );
          }
        else if (!strcmp( val, "IBVD" ) )
          {
          this->Script ( "QueryAtlasSetAnnotationTermSet IBVD" );
          }
        }
      }
    if ( this->QdecScalarSelector )
      {
      if (( m == this->QdecScalarSelector->GetWidget()->GetMenu()) && (event == vtkKWMenu::MenuItemInvokedEvent ))
        {
        this->DisplayScalarOverlay();
        }
      }
    }

  //---
  //--- Process Checkbuttons
  //---
  if ( c != NULL )
    {
    if ((c == this->SpeciesNoneButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
      {
      if (this->SpeciesNoneButton->GetSelectedState() == 1 )
        {
        this->SpeciesHumanButton->SetSelectedState(0);
        this->SpeciesMouseButton->SetSelectedState(0);
        this->SpeciesMacaqueButton->SetSelectedState(0);
        }
      }
    if ((c == this->SpeciesHumanButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
      {
      if (this->SpeciesNoneButton->GetSelectedState() == 1 && this->SpeciesHumanButton->GetSelectedState() == 1 )
        {
        this->SpeciesNoneButton->SetSelectedState(0);
        }
      }
    if ((c == this->SpeciesMouseButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
      {
      if (this->SpeciesNoneButton->GetSelectedState() == 1 && this->SpeciesMouseButton->GetSelectedState() == 1 )
        {
        this->SpeciesNoneButton->SetSelectedState(0);
        }
      }
    if ((c == this->SpeciesMacaqueButton) && (event == vtkKWCheckButton::SelectedStateChangedEvent))
      {
      if (this->SpeciesNoneButton->GetSelectedState() == 1 && this->SpeciesMacaqueButton->GetSelectedState() == 1 )
        {
        this->SpeciesNoneButton->SetSelectedState(0);
        }
      }
    }

  return;
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::LoadXcedeCatalogCallback ( )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );
  const char *filen;

  // get file from dialog  
  filen = this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->GetFileName();
  if ( filen != NULL )
    {
    itksys::SystemTools::ConvertToUnixOutputPath( filen );
    std::string fl(filen);

    if ( this->GetMRMLScene() && fl.find(".xcat") != std::string::npos )
      {
      this->Script ( "XcedeCatalogImport %s", filen);
      this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      }

    if (  this->GetMRMLScene()->GetErrorCode() != 0 ) 
      {
      if ( app->GetApplicationGUI() != NULL )
        {
        //--- display error message: there was an error during scene load.
        vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
        dialog->SetParent (  app->GetApplicationGUI()->GetMainSlicerWindow() );
        dialog->SetStyleToMessage();
        std::string msg = this->GetMRMLScene()->GetErrorMessage();
        dialog->SetText(msg.c_str());
        dialog->Create ( );
        dialog->Invoke();
        dialog->Delete();
        }
      }
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AutoWinLevThreshStatisticsVolume ( vtkMRMLScalarVolumeNode *vnode )
{
  int i;
  double win, level, upT, lowT;
  
  //--- look at the nnode's name; if it contains the substring "stat"
  //--- then assume this is a statistics file and auto win/lev/thresh it.
  if ( vnode != NULL )
    {
    std::string nname ( vnode->GetName() );
    if ( nname != "" )
      {
      i=nname.find ( "stat", 0 );
      if ( i != string::npos )
        {
        vtkMRMLScalarVolumeDisplayNode *dnode = vnode->GetScalarVolumeDisplayNode();
        if ( dnode != NULL ) 
          {
          dnode->SetAutoThreshold (0);
          dnode->SetAutoWindowLevel (0);
          win = dnode->GetWindow();
          level = dnode->GetLevel();
          upT = dnode->GetUpperThreshold();
          lowT = dnode->GetLowerThreshold();

          //--- set window... a guess
          dnode->SetWindow ( win/2.6 );
          win = dnode->GetWindow();
          dnode->SetLevel ( upT - (win/2.0) );
          
          //--- set lower threshold
          dnode->SetLowerThreshold ( upT - ( (upT-lowT)/2.5));
          dnode->SetUpperThreshold ( upT );

          //-- apply the settings
          dnode->SetApplyThreshold(1);
          dnode->SetAutoThreshold( 0 );
          }
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::LoadQdecResultsCallback ( )
{
  
  int retval = -1;
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast( this->GetApplication() );

  if ( app == NULL )
    {
    return;
    }
  const char *filen = this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->GetFileName();
  if ( filen != NULL )
    {
    itksys::SystemTools::ConvertToUnixOutputPath( filen );

    //--- and load results thru qdecModule Logic, which we get thru the GUI.
    //--- TODO: build a direct way of getting logics, w/o requiring gui-route.
    vtkQdecModuleGUI *qGUI = vtkQdecModuleGUI::SafeDownCast(app->GetModuleGUIByName("QdecModule"));
    if ( qGUI != NULL )
      {
      retval = qGUI->LoadProjectFile ( filen );
      }

    //--- if results appear to have loaded...
    //--- make sure scene agrees with that.

    if ( retval >= 0 )
      {
      this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->SaveLastPathToRegistry("OpenPath");
      if (  this->GetMRMLScene()->GetErrorCode() != 0 ) 
        {
        if ( app->GetApplicationGUI() != NULL )
          {
          vtkKWMessageDialog *dialog = vtkKWMessageDialog::New();
          dialog->SetParent (  app->GetApplicationGUI()->GetMainSlicerWindow() );
          dialog->SetStyleToMessage();
          std::string msg = this->GetMRMLScene()->GetErrorMessage();
          dialog->SetText(msg.c_str());
          dialog->Create ( );
          dialog->Invoke();
          dialog->Delete();
          }
        }
      }
    // update Scalar overlay menu
    this->UpdateScalarOverlayMenu();
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ModifyQuerySceneVisibility()
{
  int state;
  if ( this->QuerySceneVisibilityMenuButton != NULL )
    {
    if ( this->QuerySceneVisibilityMenuButton->GetMenu() != NULL )
      {
      //--- update annotation visibility
      state =  this->QuerySceneVisibilityMenuButton->GetMenu()->GetItemSelectedState("annotations");
      if ( state == 1 )
        {
        this->Script ( "QueryAtlasAnnotationVisibility on");
        }
      else
        {
        this->Script ( "QueryAtlasAnnotationVisibility off");
        }
      int n = this->QuerySceneVisibilityMenuButton->GetMenu()->GetNumberOfItems();
      int m = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLModelNode" );

      const char *menuText;
      vtkMRMLModelNode *mnode;
      vtkMRMLModelDisplayNode *dnode;

      //--- process models (don't include separators and 'close' command
      //--- at beginning and end of menu
      for ( int i=2; i<n-2; i++)
        {
        menuText = this->QuerySceneVisibilityMenuButton->GetMenu()->GetItemLabel ( i );
        //--- search all models for this modelID
        for ( int j=0; j < m; j++)
          {
          mnode = vtkMRMLModelNode::SafeDownCast (this->GetMRMLScene()->GetNthNodeByClass ( j, "vtkMRMLModelNode" ));
          if ( mnode != NULL )
            {
            std::string name (mnode->GetName() );
            std::string mid (mnode->GetID() );
            std::string menuitem = name + " (" + mid + ")";
            if ( !(strcmp ( menuitem.c_str(), menuText )) )
              {
              this->QuerySceneVisibilityMenuButton->GetMenu()->SetItemStateToNormal(menuitem.c_str() );
              state =  this->QuerySceneVisibilityMenuButton->GetMenu()->GetItemSelectedState(menuitem.c_str() );
              // if selected state is different from node's state, then change node's state.
              dnode = vtkMRMLModelDisplayNode::SafeDownCast (mnode->GetDisplayNode() );
              if ( dnode != NULL )
                {
                if ( state != dnode->GetVisibility() )
                  {
                  this->GetMRMLScene()->SaveStateForUndo( dnode );
                  dnode->SetVisibility ( state );
                  this->Script ( "QueryAtlasSetQueryModelVisibility %s %d", mid.c_str(), state);
                  break;
                  }
                }
              }
            }
          }
        }
      }
    }
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::UpdateAnnoVisibilityMenu ( )
{

    if ( this->QuerySceneVisibilityMenuButton != NULL )
      {
      //--- clear it out
      int state = this->QuerySceneVisibilityMenuButton->GetMenu()->GetItemSelectedState ( "annotations" );
      this->QuerySceneVisibilityMenuButton->GetMenu()->DeleteAllItems();    
      //--- now reconstruct it
      this->QuerySceneVisibilityMenuButton->GetMenu()->AddCheckButton ( "annotations" );
      if ( state == 1 )
        {
        this->QuerySceneVisibilityMenuButton->GetMenu()->SelectItem ( "annotations" );
        }
      else if ( state == 0 )
        {
        this->QuerySceneVisibilityMenuButton->GetMenu()->DeselectItem ( "annotations" );
        }
      this->QuerySceneVisibilityMenuButton->GetMenu()->AddSeparator();
      // search the scene for any models and add them to this list.
      int numModels = this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLModelNode");
      for ( int i=0; i < numModels; i++ )
        {
        vtkMRMLModelNode *mnode = vtkMRMLModelNode::SafeDownCast ( this->GetMRMLScene()->GetNthNodeByClass ( i, "vtkMRMLModelNode" ));
        if ( mnode != NULL )
          {
          std::string name (mnode->GetName() );
          std::string mid (mnode->GetID() );
          std::string menuitem = name + " (" + mid + ")";
          if ( (!(strcmp (name.c_str(), "lh.pial"))) || (!(strcmp (name.c_str(), "rh.pial")))
               || (!(strcmp (name.c_str(), "lh.inflated")))
               || (!(strcmp (name.c_str(), "rh.inflated"))) )
            {
            this->QuerySceneVisibilityMenuButton->GetMenu()->AddCheckButton ( menuitem.c_str() );
            //--- set the model's initial visibility according to the display node.
            vtkMRMLModelDisplayNode *dnode = vtkMRMLModelDisplayNode::SafeDownCast( mnode->GetDisplayNode() );
            if ( dnode )
              {
              int v = dnode->GetVisibility ( );
              if ( v )
                {
                this->QuerySceneVisibilityMenuButton->GetMenu()->SelectItem ( menuitem.c_str() );
                }
              else
                {
                this->QuerySceneVisibilityMenuButton->GetMenu()->DeselectItem ( menuitem.c_str() );
                }
              }
            }
          }
        }
      this->QuerySceneVisibilityMenuButton->GetMenu()->AddSeparator();
      this->QuerySceneVisibilityMenuButton->GetMenu()->AddCommand ( "close" );    
      }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::UpdateScalarOverlayMenu ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  vtkQdecModuleLogic *qLogic;
  if ( app )
    {
    if ( vtkQdecModuleGUI::SafeDownCast ( app->GetModuleGUIByName("QdecModule")) != NULL )
      {
      qLogic = vtkQdecModuleGUI::SafeDownCast(app->GetModuleGUIByName("QdecModule"))->GetLogic();
      }

    if ( (this->QdecScalarSelector != NULL) && (qLogic != NULL) )
      {
      this->QdecScalarSelector->GetWidget()->GetMenu()->DeleteAllItems();
      int numQuestions = qLogic->GetNumberOfQuestions();
      for ( int i=0; i<numQuestions; i++ )
        {
        this->QdecScalarSelector->GetWidget()->GetMenu()->AddRadioButton(qLogic->GetQuestion(i).c_str());
        }

      //--- if the labels are now on the model, add the menuitem
      vtkMRMLModelNode *mnode = qLogic->GetModelNode();
      if ( mnode != NULL )
        {
        std::string lutName ("QueryLUT_" );
        lutName = lutName + mnode->GetID();
        // find the query lut by name
        int n= this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLColorNode");
        for ( int j=0; j< n; j++ )
          {
          vtkMRMLColorNode *cnode = vtkMRMLColorNode::SafeDownCast( this->GetMRMLScene()->GetNthNodeByClass ( j, "vtkMRMLColorNode"));
          if ( cnode != NULL )
            {
            if ( !(strcmp(cnode->GetName(), lutName.c_str() )) )
              {
              // looks like the node is in the scene; so add the menuitem.
              this->QdecScalarSelector->GetWidget()->GetMenu()->AddRadioButton("labels");
              }
            }
          }
        }
      }
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DisplayScalarOverlay ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast (this->GetApplication() );
  vtkQdecModuleLogic *qLogic;
  vtkSlicerModelsLogic *mLogic;
  if ( app )
    {
    if ( vtkQdecModuleGUI::SafeDownCast(app->GetModuleGUIByName("QdecModule")) != NULL )
      {
      qLogic = vtkQdecModuleGUI::SafeDownCast(app->GetModuleGUIByName("QdecModule"))->GetLogic();
      }
    if ( vtkSlicerModelsGUI::SafeDownCast(app->GetModuleGUIByName("Models")) != NULL )
      {
      mLogic = vtkSlicerModelsGUI::SafeDownCast(app->GetModuleGUIByName("Models"))->GetLogic();
      }
    if ( (this->QdecScalarSelector->IsCreated()) && (qLogic != NULL) && (mLogic != NULL) )
      {
      if ( (strcmp(this->QdecScalarSelector->GetWidget()->GetValue(), "None") != 0)  && (qLogic != NULL ) )
        {
        const char *cselection = this->QdecScalarSelector->GetWidget()->GetValue();
        // trigger display change on the model
        vtkMRMLModelNode *mnode = qLogic->GetModelNode();
        if ( mnode != NULL)
          {
          if ( !(strcmp( cselection,"labels" )) )
            {
            // find the query lut by name
            std::string lutName ("QueryLUT_" );
            lutName = lutName + mnode->GetID();
            int n= this->GetMRMLScene()->GetNumberOfNodesByClass ( "vtkMRMLColorNode");
            for ( int j=0; j< n; j++ )
              {
              // get the color node that goes with that name
              vtkMRMLColorNode *cnode = vtkMRMLColorNode::SafeDownCast( this->GetMRMLScene()->GetNthNodeByClass ( j, "vtkMRMLColorNode"));
              if ( cnode != NULL )
                {
                if ( !(strcmp(cnode->GetName(), lutName.c_str() )) )
                  {
                  vtkDebugMacro("Setting the active scalars on " << mnode->GetName() << " to " << cselection );
                  qLogic->GetModelNode()->SetActiveScalars(cselection, NULL);
                  qLogic->GetModelNode()->GetModelDisplayNode()->SetActiveScalarName( cselection );
                  qLogic->GetModelNode()->GetModelDisplayNode()->SetAndObserveColorNodeID(cnode->GetID() );
                  }
                }
              }
            }
          else
            {
            std::string scalarName = qLogic->GetQuestionScalarName( cselection );
            vtkDebugMacro("Got question scalar name from logic: " << scalarName.c_str());
            vtkDebugMacro("Setting the active scalars on " << qLogic->GetModelNode()->GetName() << " to " << scalarName.c_str());
            qLogic->GetModelNode()->SetActiveScalars(scalarName.c_str(), NULL);
            qLogic->GetModelNode()->GetModelDisplayNode()->SetActiveScalarName(scalarName.c_str());
            // the color node has the same name as the scalar array name to facilitate
            // this pairing, find the ID by querying the mrml scene
            std::string colorID = "none";
            if (this->GetApplication() && this->GetApplicationGUI()->GetMRMLScene())
              {
              vtkCollection *colorNodes =  this->GetApplicationGUI()->GetMRMLScene()->GetNodesByName(scalarName.c_str());
              if (colorNodes)
                {
                int numberOfNodes = colorNodes->GetNumberOfItems();
                if (numberOfNodes > 0)
                  {
                  // take the first one
                  colorID = vtkMRMLProceduralColorNode::SafeDownCast(colorNodes->GetItemAsObject(0))->GetID();
                  }
                else
                  {
                  vtkErrorMacro("vtkQueryAtlasGUI Cannot find a color node with the name " << scalarName.c_str());
                  }
                colorNodes->RemoveAllItems();
                colorNodes->Delete();
                }
              else
                {
                vtkErrorMacro("vtkQueryAtlasGUI cannot find procedural color nodes to check for the one associated with scalar " << scalarName.c_str());
                }         
              } else { vtkErrorMacro("No application or scene, can't find matching color node"); }
            if (strcmp(colorID.c_str(), "none") != 0)
              {
              // use this node id
              if (strcmp(qLogic->GetModelNode()->GetModelDisplayNode()->GetColorNodeID(), colorID.c_str()) != 0)
                {
                vtkDebugMacro("Setting the model's display node color node id to " << colorID.c_str());
                qLogic->GetModelNode()->GetModelDisplayNode()->SetAndObserveColorNodeID(colorID.c_str());
                }
              else { vtkDebugMacro("Model's display node color node is already set to " << colorID.c_str()); }
              }
            else
              {
              vtkErrorMacro("Qdec Module gui unable to find matching color node for scalar array " << scalarName.c_str());
              }
            }
          }
        else
          {
          vtkErrorMacro("Qdec Module Logic has no record of a model node, can't switch scalars");
          }
        }
      }
    }
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::WriteBookmarksCallback ()
{
  // get file from dialog
  const char *filen;
  
  filen = this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->GetFileName();
  if ( filen != NULL )
    {
    itksys::SystemTools::ConvertToUnixOutputPath( filen );
    this->Script( "QueryAtlasWriteFirefoxBookmarkFile \"%s\"", filen );
    }
  this->SaveAccumulatedResultsButton->SetText ( "" );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::LoadBookmarksCallback ()
{
  // get file from dialog
  const char *filen;
  filen = this->LoadURIsButton->GetLoadSaveDialog()->GetFileName();
  if ( filen != NULL )
    {
    itksys::SystemTools::ConvertToUnixOutputPath( filen );
    this->Script( "QueryAtlasLoadFirefoxBookmarkFile \"%s\"", filen );
    this->LoadURIsButton->SetText ( "" );
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ColorCodeContextButtons ( vtkKWPushButton *b )
{
#ifdef SEARCHTERM_FRAME
  this->OtherButton->SetBackgroundColor ( _br, _bg, _bb );
  this->StructureButton->SetBackgroundColor ( _br, _bg, _bb );
  this->PopulationButton->SetBackgroundColor ( _br, _bg, _bb );
  this->SpeciesButton->SetBackgroundColor ( _br, _bg, _bb );

  this->OtherButton->SetForegroundColor ( _fr, _fg, _fb );
  this->StructureButton->SetForegroundColor ( _fr, _fg, _fb );
  this->PopulationButton->SetForegroundColor ( _fr, _fg, _fb );
  this->SpeciesButton->SetForegroundColor ( _fr, _fg, _fb );

  b->SetBackgroundColor (1.0, 1.0, 1.0);
  b->SetForegroundColor (0.0, 0.0, 0.0);
#endif
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
  if (this->ProcessingMRMLEvent != 0 )
    {
    return;
    }
  this->ProcessingMRMLEvent = event;
  vtkDebugMacro("processing event " << event);
   

  //--- has a node been added?
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
       && (event == vtkMRMLScene::NodeAddedEvent ))
    {
    vtkMRMLScalarVolumeNode *node = vtkMRMLScalarVolumeNode::SafeDownCast ( (vtkObjectBase *)callData );
    if ( node != NULL )
      {
      //--- apply ballpark threshold if the node appears to be a statistics volume.
      AutoWinLevThreshStatisticsVolume ( node );
      }
    this->Script ( "QueryAtlasNodeAddedUpdate" );
    this->UpdateScalarOverlayMenu();
    this->UpdateAnnoVisibilityMenu();
    }

  //--- has a node been deleted?
  if ( vtkMRMLScene::SafeDownCast(caller) == this->MRMLScene 
       && (event == vtkMRMLScene::NodeRemovedEvent ) )
    {
    this->Script ( "QueryAtlasNodeRemovedUpdate");
    this->UpdateScalarOverlayMenu();
    this->UpdateAnnoVisibilityMenu();
    }
  
  //--- is the scene closing?
  if (event == vtkMRMLScene::SceneCloseEvent )
    {
    this->SceneClosing = true;
    // clean up and reset globals.
    this->Script ("QueryAtlasTearDown" );
    this->ClearOntologyGUI();
    this->Script("QueryAtlasInitializeGlobals");
    // empty menus.
    this->QdecScalarSelector->GetWidget()->GetMenu()->DeleteAllItems();
    this->UpdateAnnoVisibilityMenu();
    }
  else 
    {
    this->SceneClosing = false;
    }
  this->ProcessingMRMLEvent = 0;
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::Enter ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: Enter\n");
    this->Script ( "QueryAtlasCullOldModelAnnotations");
    this->Script ( "QueryAtlasCullOldLabelMapAnnotations");
    this->Script ( "QueryAtlasAddInteractorObservers" );
    this->UpdateScalarOverlayMenu();
    this->UpdateAnnoVisibilityMenu();
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::Exit ( )
{
    vtkDebugMacro("vtkQueryAtlasGUI: Exit\n");
    this->Script ( "QueryAtlasRemoveInteractorObservers" );
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AddMRMLObservers()
{
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::RemoveMRMLObservers()
{
  this->SetAndObserveMRMLScene ( NULL );
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

    const char *help = "The (Generation 1) Query Atlas module allows interactive queries to a number of informational resources (Google, Wikipedia, BrainInfo, IBVD, Journal of Neuroscience, and Pubmed) from within the 3D anatomical display. These queries take advantage, where appropriate,  of the QueryAtlas's controlled vocabulary, which maps anatomical terms to different formal naming systems. More advanced query building and the ability to collect and preview web links to information, and save valuable ones to a bookmarks file is avilable in the GUI panel. This module requires the use of Mozilla Firefox as your web browser; you can point Slicer to this application through the Application Settings interface (View->Application Settings) and its location will be saved in Slicer's Application Registry for future reference.";
    const char *about = "This research was supported by Grant 5 MOI RR 000827 to the FIRST BIRN and Grant 1 U24 RR021992 to the FBIRN Biomedical Informatics Research Network (BIRN, http://www.nbirn.net), that is funded by the National Center for Research Resources (NCRR) at the National Institutes of Health (NIH). This work was also supported by NA-MIC, NAC, NCIGT. NeuroNames ontology and URI resources are provided courtesy of BrainInfo, University of Washington (http://www.braininfo.org).                                                                                                                                                                                      ";
    vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    this->QueryAtlasIcons = vtkQueryAtlasIcons::New();
    this->BuildHelpAndAboutFrame ( page, help, about );
    this->BuildAcknowledgementPanel ( );
#ifdef LOAD_FRAME
    this->BuildLoadAndConvertGUI ( );
#endif
#ifdef ANNO_FRAME
    this->BuildAnnotationOptionsGUI ( );
#endif
#ifdef ONTOLOGY_FRAME
    this->BuildOntologyGUI ( );
#endif
#ifdef SEARCHTERM_FRAME
    this->BuildSearchTermGUI ( );
#endif
#ifdef QUERIES_FRAME
    this->BuildQueriesGUI ( );
#endif
//    this->BuildDisplayAndNavigationGUI ( );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildAcknowledgementPanel ( )
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
    
    vtkKWLabel *aspl = vtkKWLabel::New();
    aspl->SetParent ( this->GetLogoFrame() );
    aspl->Create();
    aspl->SetImageToIcon ( this->QueryAtlasIcons->GetSPLlogo() );
    
    app->Script ("grid %s -row 0 -column 0 -padx 2 -pady 2 -sticky w",  abirn->GetWidgetName());
    app->Script ("grid %s -row 0 -column 1 -padx 2 -pady 2 -sticky w", anamic->GetWidgetName());
    app->Script ("grid %s -row 0 -column 2 -padx 2 -pady 2 -sticky w",  anac->GetWidgetName());
    app->Script ("grid %s -row 1 -column 0 -padx 2 -pady 2 -sticky w",  aigt->GetWidgetName());                 
    app->Script ("grid %s -row 1 -column 1 -padx 2 -pady 2 -sticky w",  aspl->GetWidgetName());
    app->Script ("grid %s -row 1 -column 2 -padx 2 -pady 2 -sticky w",  abi->GetWidgetName());

    aspl->Delete();
    abirn->Delete();
    anac->Delete();
    anamic->Delete();
    aigt->Delete();
    abi->Delete();
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
    convertFrame->SetLabelText ("Scene Setup");
    convertFrame->ExpandFrame ( );
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 4 -in %s",
                  convertFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    this->BuildLoaderContextButtons ( convertFrame->GetFrame() );
    
    vtkKWFrame *switcher = vtkKWFrame::New();
    switcher->SetParent ( convertFrame->GetFrame() );
    switcher->Create();
    
    this->BuildLoaderContextFrames ( switcher );
    this->BuildFreeSurferFIPSFrame ( );
    this->BuildQdecFrame ( );
    this->BuildGeneralAnnotateFrame ( );
    this->PackLoaderContextFrame ( this->GeneralFrame );
    app->Script ( "pack %s -side top -fill x -expand 1 -pady 0", switcher->GetWidgetName() );

    this->ColorCodeLoaderContextButtons ( this->GeneralButton );
    switcher->Delete();
    convertFrame->Delete();
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildFreeSurferFIPSFrame( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  
    this->LoadFIPSFSCatalogButton = vtkKWLoadSaveButtonWithLabel::New() ;
    this->LoadFIPSFSCatalogButton->SetParent ( this->FIPSFSFrame );
    this->LoadFIPSFSCatalogButton->Create();
    this->LoadFIPSFSCatalogButton->GetWidget()->SetImageToIcon ( app->GetApplicationGUI()->GetApplicationToolbar()->GetSlicerToolbarIcons()->GetLoadSceneIcon() );   
    this->LoadFIPSFSCatalogButton->GetWidget()->SetBorderWidth(0);
    this->LoadFIPSFSCatalogButton->GetWidget()->SetReliefToFlat ( );
    this->LoadFIPSFSCatalogButton->SetBalloonHelpString ( "Load FIPS/FreeSurfer Xcede catalog" );
    this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Load FIPS/FreeSurfer Xcede catalog");
    this->LoadFIPSFSCatalogButton->GetLabel()->SetText( "Load catalog: ");
    this->LoadFIPSFSCatalogButton->GetLabel()->SetWidth ( 18 );
    this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOff();
    this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->SaveDialogOff();
    this->LoadFIPSFSCatalogButton->GetWidget()->GetLoadSaveDialog()->SetFileTypes ( "{ {Xcede catalog} {*.xcat} }");
    this->LoadFIPSFSCatalogButton->SetBalloonHelpString("Load a FIPS/FreeSurfer study from an Xcede catalog.");
    this->Script ( "pack %s -side top -anchor nw -padx 6 -pady 4",
                  this->LoadFIPSFSCatalogButton->GetWidgetName());

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
  this->FSbrainSelector->SetLabelText( "Anatomical volume: ");
  this->FSbrainSelector->SetBalloonHelpString("Select a volume (FreeSurfer brain.mgz) from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                  this->FSbrainSelector->GetWidgetName());

  this->FSasegSelector = vtkSlicerNodeSelectorWidget::New() ;
  this->FSasegSelector->SetParent ( this->FIPSFSFrame );
  this->FSasegSelector->Create ( );
  this->FSasegSelector->SetNodeClass("vtkMRMLVolumeNode", "LabelMap", "1", NULL);
  this->FSasegSelector->SetMRMLScene(this->GetMRMLScene());
  this->FSasegSelector->SetBorderWidth(2);
  this->FSasegSelector->SetPadX(2);
  this->FSasegSelector->SetPadY(2);
  this->FSasegSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->FSasegSelector->GetWidget()->GetWidget()->SetWidth(20);
  this->FSasegSelector->GetLabel()->SetWidth(18);
  this->FSasegSelector->SetLabelText( "Annotated labelmap: ");
  this->FSasegSelector->SetBalloonHelpString("Select an annotated label map (FreeSurfer aparc+aseg) from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                  this->FSasegSelector->GetWidgetName());

  this->FSstatsSelector = vtkSlicerNodeSelectorWidget::New() ;
  this->FSstatsSelector->SetParent ( this->FIPSFSFrame );
  this->FSstatsSelector->Create ( );
  this->FSstatsSelector->SetNodeClass("vtkMRMLVolumeNode", NULL, NULL, NULL);
  this->FSstatsSelector->SetMRMLScene(this->GetMRMLScene());
  this->FSstatsSelector->SetBorderWidth(2);
  this->FSstatsSelector->SetPadX(2);
  this->FSstatsSelector->SetPadY(2);
  this->FSstatsSelector->GetWidget()->GetWidget()->IndicatorVisibilityOff();
  this->FSstatsSelector->GetWidget()->GetWidget()->SetWidth(20);
  this->FSstatsSelector->GetLabel()->SetWidth(18);
  this->FSstatsSelector->SetLabelText( "Statistics: ");
  this->FSstatsSelector->SetBalloonHelpString("Select a statistical overlay volume from the current  scene.");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                  this->FSstatsSelector->GetWidgetName());
  
  this->FSgoButton = vtkKWPushButtonWithLabel::New();
  this->FSgoButton->SetParent ( this->FIPSFSFrame );
  this->FSgoButton->Create();
  this->FSgoButton->GetWidget()->SetImageToIcon ( this->QueryAtlasIcons->GetSetUpIcon() );
  this->FSgoButton->GetWidget()->SetBorderWidth(0);
  this->FSgoButton->GetWidget()->SetReliefToFlat ( );
  this->FSgoButton->GetLabel()->SetText ("Set up annotations: ");
  this->FSgoButton->GetLabel()->SetWidth ( 18 );
  this->FSgoButton->SetBalloonHelpString ("Create interactive annotations for models and anatomical volume" );
  this->Script ( "pack %s -side top -anchor nw -padx 6 -pady 2",
                  this->FSgoButton->GetWidgetName());
  
  this->ProcessGUIEvents ( this->FSbrainSelector, vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );
  this->ProcessGUIEvents ( this->FSasegSelector, vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );
  this->ProcessGUIEvents ( this->FSstatsSelector, vtkSlicerNodeSelectorWidget::NodeSelectedEvent, NULL );
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildGeneralAnnotateFrame ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast ( this->GetApplication() );

  this->BasicAnnotateButton = vtkKWPushButtonWithLabel::New ( );
  this->BasicAnnotateButton->SetParent ( this->GeneralFrame );
  this->BasicAnnotateButton->Create();
  this->BasicAnnotateButton->GetWidget()->SetImageToIcon ( this->QueryAtlasIcons->GetSetUpIcon() );
  this->BasicAnnotateButton->GetWidget()->SetBorderWidth(0);
  this->BasicAnnotateButton->GetWidget()->SetReliefToFlat();
  this->BasicAnnotateButton->GetLabel()->SetText("Set up annotations: ");
  this->BasicAnnotateButton->GetLabel()->SetWidth ( 18 );
  this->BasicAnnotateButton->SetBalloonHelpString ("Create interactive annotations for any models and label maps in the scene (currently supports FreeSurfer models only)");
  this->Script ( "pack %s -side top -anchor nw -padx 6 -pady 2",
                 this->BasicAnnotateButton->GetWidgetName());
}

//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQdecFrame ( )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    this->QdecGetResultsButton = vtkKWLoadSaveButtonWithLabel::New() ;
    this->QdecGetResultsButton->SetParent ( this->QdecFrame );
    this->QdecGetResultsButton->Create();
    this->QdecGetResultsButton->GetWidget()->SetImageToIcon ( app->GetApplicationGUI()->GetApplicationToolbar()->GetSlicerToolbarIcons()->GetLoadSceneIcon() );   
    this->QdecGetResultsButton->GetWidget()->SetBorderWidth(0);
    this->QdecGetResultsButton->GetWidget()->SetReliefToFlat ( );
    this->QdecGetResultsButton->SetBalloonHelpString ( "Load Qdec results" );
    //this->QdecGetResultsButton->GetWidget()->SetCommand ( this, "" );
    this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->SetTitle("Load Qdec results");
    this->QdecGetResultsButton->GetLabel()->SetText( "Load Qdec results: ");
    this->QdecGetResultsButton->GetLabel()->SetWidth ( 18 );
    this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->ChooseDirectoryOff();
    this->QdecGetResultsButton->GetWidget()->GetLoadSaveDialog()->SaveDialogOff();
    //this->QdecGetResultsButton->GetLoadSaveDialog()->SetFileTypes ( "");
    this->QdecGetResultsButton->SetBalloonHelpString("Load all results from previous Qdec analysis (select a qdec directory).");
    this->Script ( "pack %s -side top -anchor nw -padx 6 -pady 4",
                  this->QdecGetResultsButton->GetWidgetName());

    this->QdecGoButton = vtkKWPushButtonWithLabel::New();
    this->QdecGoButton->SetParent ( this->QdecFrame );
    this->QdecGoButton->Create();
    this->QdecGoButton->GetWidget()->SetImageToIcon ( this->QueryAtlasIcons->GetSetUpIcon() );
    this->QdecGoButton->GetWidget()->SetBorderWidth(0);
    this->QdecGoButton->GetWidget()->SetReliefToFlat();
    this->QdecGoButton->GetLabel()->SetText("Set up annotations: ");
    this->QdecGoButton->GetLabel()->SetWidth ( 18 );
    this->QdecGoButton->SetBalloonHelpString ("Create interactive annotations for models");
    this->Script ( "pack %s -side top -anchor nw -padx 6 -pady 2",
                   this->QdecGoButton->GetWidgetName());

    this->QdecScalarSelector = vtkKWMenuButtonWithLabel::New();
    this->QdecScalarSelector->SetParent ( this->QdecFrame );
    this->QdecScalarSelector->Create ( );
    this->QdecScalarSelector->SetBorderWidth(2);
    this->QdecScalarSelector->SetPadX(2);
    this->QdecScalarSelector->SetPadY(2);
    this->QdecScalarSelector->GetWidget()->SetImageToIcon ( this->QueryAtlasIcons->GetSelectOverlayIcon() );
    this->QdecScalarSelector->GetWidget()->SetBorderWidth ( 0 );
    this->QdecScalarSelector->GetWidget()->SetReliefToFlat();
    this->QdecScalarSelector->GetWidget()->IndicatorVisibilityOff();
    this->QdecScalarSelector->GetLabel()->SetWidth(18);
    this->QdecScalarSelector->GetLabel()->SetText( "Select overlay: ");
    this->QdecScalarSelector->SetBalloonHelpString("select a scalar overlay for this model.");
    this->Script ( "pack %s -side top -anchor nw -padx 2 -pady 2",
                  this->QdecScalarSelector->GetWidgetName());

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

    //--- widget that lets you choose what term set to
    //--- display in the annotations in the 3D viewer
    vtkKWLabel *l = vtkKWLabel::New();
    l->SetParent ( annotationFrame->GetFrame() );
    l->Create ( );
    l->SetText ( "annotation term set: " );
    this->AnnotationTermSetMenuButton = vtkKWMenuButton::New();
    this->AnnotationTermSetMenuButton->SetParent ( annotationFrame->GetFrame() );
    this->AnnotationTermSetMenuButton->Create();
    this->AnnotationTermSetMenuButton->SetWidth ( 24 );
    this->AnnotationTermSetMenuButton->GetMenu()->AddRadioButton ("local identifier");
    this->AnnotationTermSetMenuButton->GetMenu()->AddRadioButton ("BIRNLex String");
    this->AnnotationTermSetMenuButton->GetMenu()->AddRadioButton ("NeuroNames String");
    this->AnnotationTermSetMenuButton->GetMenu()->AddRadioButton ("IBVD");
    this->AnnotationTermSetMenuButton->GetMenu()->AddRadioButton ("UMLS CName");
    this->AnnotationTermSetMenuButton->GetMenu()->AddSeparator();
    this->AnnotationTermSetMenuButton->GetMenu()->AddCommand ( "close" );    
    this->AnnotationTermSetMenuButton->GetMenu()->SelectItem ("local identifier");
    this->AnnotationTermSetMenuButton->SetBalloonHelpString ( "Select term set used to display annotations in the 3D viewer." );

    //--- widget that permits toggling of annotations and
    //--- query model visibility in the 3D viewer.
    vtkKWLabel *vl = vtkKWLabel::New();
    vl->SetParent ( annotationFrame->GetFrame() );
    vl->Create();
    vl->SetText ( "scene visibility: " );
    this->QuerySceneVisibilityMenuButton = vtkKWMenuButton::New();
    this->QuerySceneVisibilityMenuButton->SetParent ( annotationFrame->GetFrame() );
    this->QuerySceneVisibilityMenuButton->Create();
    this->QuerySceneVisibilityMenuButton->SetWidth ( 24 );
    this->QuerySceneVisibilityMenuButton->GetMenu()->AddCheckButton ( "annotations" );
    this->QuerySceneVisibilityMenuButton->GetMenu()->SelectItem ("annotations");
    this->QuerySceneVisibilityMenuButton->GetMenu()->AddSeparator();
    this->QuerySceneVisibilityMenuButton->GetMenu()->AddSeparator();
    this->QuerySceneVisibilityMenuButton->GetMenu()->AddCommand ( "close" );    
    this->QuerySceneVisibilityMenuButton->SetValue ( "(models + annotations)" );
    this->QuerySceneVisibilityMenuButton->SetBalloonHelpString ( "Toggle the visibility of models and annotations in the 3D viewer." );

    app->Script ( "grid %s -row 0 -column 0 -sticky nse -padx 2 -pady 2",
                  l->GetWidgetName() );
    app->Script ( "grid %s -row 0 -column 1 -sticky wns -padx 2 -pady 2",
                  this->AnnotationTermSetMenuButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 0 -sticky nse -padx 2 -pady 2",
                  vl->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1 -sticky wns -padx 2 -pady 2",
                  this->QuerySceneVisibilityMenuButton->GetWidgetName() );
    app->Script ( "pack %s -side top -anchor nw -fill x -expand y -padx 2 -pady 2 -in %s",
                  annotationFrame->GetWidgetName(), 
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    l->Delete();
    vl->Delete();
    annotationFrame->Delete();
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ClearOntologyGUI ( )
{
  if ( this->LocalSearchTermEntry->IsCreated() )
    {
    this->LocalSearchTermEntry->SetValue ( "" );
    }
  if ( this->SynonymsMenuButton->IsCreated() )
    {
    this->SynonymsMenuButton->GetMenu()->DeleteAllItems();
    this->SynonymsMenuButton->SetValue ( "" );
    }
  if ( this->BIRNLexEntry->IsCreated() )
    {
    this->BIRNLexEntry->SetValue ( "" );
    }
  if ( this->BIRNLexIDEntry->IsCreated() )
    {
    this->BIRNLexIDEntry->SetValue ( "" );
    }
  if ( this->NeuroNamesEntry->IsCreated() )
    {
    this->NeuroNamesEntry->SetValue ( "" );
    }
  if ( this->NeuroNamesIDEntry->IsCreated() )
    {    
    this->NeuroNamesIDEntry->SetValue ( "" );
    }
  if ( this->UMLSCNEntry->IsCreated() )
    {
    this->UMLSCNEntry->SetValue ( "" );
    }
  if ( this->UMLSCIDEntry->IsCreated() )
    {
    this->UMLSCIDEntry->SetValue ( "" );
    }
  if ( this->SavedTerms->IsCreated() )
    {
    this->SavedTerms->GetMultiColumnList()->GetWidget()->DeleteAllRows();
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildOntologyGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    // -------------------------------------------------------------------------------------------------
    // ---
    // ONTOLOGY SEARCH FRAME
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
    this->LocalSearchTermEntry->ReadOnlyOn();
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
    this->BIRNLexEntry->ReadOnlyOn();
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
    this->BIRNLexHierarchyButton->SetBalloonHelpString ("View in BIRNLex ontology browser (click once to launch, and click again to select term).");

    // forthrow (BIRNLex ID )
    vtkKWLabel *birnidLabel = vtkKWLabel::New();
    birnidLabel->SetParent ( hierarchyFrame->GetFrame() );
    birnidLabel->Create();
    birnidLabel->SetText ("BIRNLex ID: ");
    this->BIRNLexIDEntry = vtkKWEntry::New();
    this->BIRNLexIDEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->BIRNLexIDEntry->Create();
    this->BIRNLexIDEntry->SetValue("");
    this->BIRNLexIDEntry->ReadOnlyOn();
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
    this->NeuroNamesEntry->ReadOnlyOn();
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
    this->NeuroNamesHierarchyButton->SetImageToIcon ( this->QueryAtlasIcons->GetOntologyBrowserIcon() );
    this->NeuroNamesHierarchyButton->SetBorderWidth ( 0 );
    this->NeuroNamesHierarchyButton->SetReliefToFlat();
    this->NeuroNamesHierarchyButton->SetBalloonHelpString ("View in NeuroNames ontology browser (click once to launch, and click again to select term).");

    // sixth row (NeuroNames ID)
    vtkKWLabel *nnidLabel = vtkKWLabel::New();    
    nnidLabel->SetParent ( hierarchyFrame->GetFrame() );
    nnidLabel->Create();
    nnidLabel->SetText ("NeuroNames ID: ");
    this->NeuroNamesIDEntry = vtkKWEntry::New();
    this->NeuroNamesIDEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->NeuroNamesIDEntry->Create();
    this->NeuroNamesIDEntry->SetValue ("");
    this->NeuroNamesIDEntry->ReadOnlyOn();
    this->AddNeuroNamesIDButton = vtkKWPushButton::New();
    this->AddNeuroNamesIDButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddNeuroNamesIDButton->Create();
    this->AddNeuroNamesIDButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddNeuroNamesIDButton->SetBorderWidth(0);
    this->AddNeuroNamesIDButton->SetReliefToFlat();
    this->AddNeuroNamesIDButton->SetBalloonHelpString ("Save this term for building queries.");
    
    // seventh row (UMLSCN)
    vtkKWLabel *umlscnLabel = vtkKWLabel::New();
    umlscnLabel->SetParent ( hierarchyFrame->GetFrame() );
    umlscnLabel->Create();
    umlscnLabel->SetText ("UMLS: ");
    this->UMLSCNEntry = vtkKWEntry::New();
    this->UMLSCNEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->UMLSCNEntry->Create();
    this->UMLSCNEntry->SetValue ("");
    this->UMLSCNEntry->ReadOnlyOn();
    this->AddUMLSCNButton = vtkKWPushButton::New();
    this->AddUMLSCNButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddUMLSCNButton->Create();
    this->AddUMLSCNButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddUMLSCNButton->SetBorderWidth(0);
    this->AddUMLSCNButton->SetReliefToFlat();
    this->AddUMLSCNButton->SetBalloonHelpString ("Save this term for building queries.");
    this->UMLSHierarchyButton = vtkKWPushButton::New();
    this->UMLSHierarchyButton->SetParent ( hierarchyFrame->GetFrame() );
    this->UMLSHierarchyButton->Create();
    this->UMLSHierarchyButton->SetImageToIcon ( this->QueryAtlasIcons->GetOntologyBrowserDisabledIcon() );
    this->UMLSHierarchyButton->SetBorderWidth ( 0 );
    this->UMLSHierarchyButton->SetReliefToFlat();
    this->UMLSHierarchyButton->SetBalloonHelpString ("View in UMLS ontology browser.");

    // eighth row (UMLSCID)
    vtkKWLabel *umlsLabel = vtkKWLabel::New();
    umlsLabel->SetParent ( hierarchyFrame->GetFrame() );
    umlsLabel->Create();
    umlsLabel->SetText ("UMLS CID: ");
    this->UMLSCIDEntry = vtkKWEntry::New();
    this->UMLSCIDEntry->SetParent ( hierarchyFrame->GetFrame() );
    this->UMLSCIDEntry->Create();
    this->UMLSCIDEntry->SetValue ("");
    this->UMLSCIDEntry->ReadOnlyOn();
    this->AddUMLSCIDButton = vtkKWPushButton::New();
    this->AddUMLSCIDButton->SetParent ( hierarchyFrame->GetFrame() );
    this->AddUMLSCIDButton->Create();
    this->AddUMLSCIDButton->SetImageToIcon ( this->QueryAtlasIcons->GetAddIcon() );
    this->AddUMLSCIDButton->SetBorderWidth(0);
    this->AddUMLSCIDButton->SetReliefToFlat();
    this->AddUMLSCIDButton->SetBalloonHelpString ("Save this term for building queries.");
    
    // ninth row (listbox saved terms)
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
    app->Script ( "grid %s -row 6 -column 0 -sticky e -padx 0 -pady 1", umlscnLabel->GetWidgetName() );
    app->Script ( "grid %s -row 7 -column 0 -sticky e -padx 0 -pady 1", umlsLabel->GetWidgetName() );
    app->Script ( "grid %s -row 8 -column 0 -sticky ne -padx 0 -pady 1", termsLabel->GetWidgetName() );

    app->Script ( "grid %s -row 0 -column 1 -sticky ew -padx 2 -pady 1", this->LocalSearchTermEntry->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 1 -sticky ew -padx 2 -pady 1", this->SynonymsMenuButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 1 -sticky ew -padx 2 -pady 1", this->BIRNLexEntry->GetWidgetName() );
    app->Script ( "grid %s -row 3 -column 1 -sticky ew -padx 2 -pady 1", this->BIRNLexIDEntry->GetWidgetName() );
    app->Script ( "grid %s -row 4 -column 1 -sticky ew -padx 2 -pady 1", this->NeuroNamesEntry->GetWidgetName() );
    app->Script ( "grid %s -row 5 -column 1 -sticky ew -padx 2 -pady 1", this->NeuroNamesIDEntry->GetWidgetName() );
    app->Script ( "grid %s -row 6 -column 1 -sticky ew -padx 2 -pady 1", this->UMLSCNEntry->GetWidgetName() );
    app->Script ( "grid %s -row 7 -column 1 -sticky ew -padx 2 -pady 1", this->UMLSCIDEntry->GetWidgetName() );
    app->Script ( "grid %s -row 8 -column 1 -sticky ew -columnspan 2 -padx 2 -pady 1", f->GetWidgetName() );
    app->Script ( "pack %s -side top -fill x -expand true -padx 0 -pady 0", this->SavedTerms->GetWidgetName() );
    f->Delete();
    
    app->Script ( "grid %s -row 0 -column 2 -padx 2 -pady 1", this->AddLocalTermButton->GetWidgetName() );
    app->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 1", this->AddSynonymButton->GetWidgetName() );
    app->Script ( "grid %s -row 2 -column 2 -padx 2 -pady 1", this->AddBIRNLexStringButton->GetWidgetName() );
    app->Script ( "grid %s -row 3 -column 2 -padx 2 -pady 1", this->AddBIRNLexIDButton->GetWidgetName() );
    app->Script ( "grid %s -row 4 -column 2 -padx 2 -pady 1", this->AddNeuroNamesStringButton->GetWidgetName() );
    app->Script ( "grid %s -row 5 -column 2 -padx 2 -pady 1", this->AddNeuroNamesIDButton->GetWidgetName() );
    app->Script ( "grid %s -row 6 -column 2 -padx 2 -pady 1", this->AddUMLSCNButton->GetWidgetName() );
    app->Script ( "grid %s -row 7 -column 2 -padx 2 -pady 1", this->AddUMLSCIDButton->GetWidgetName() );

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
    umlscnLabel->Delete();
    termsLabel->Delete();
    hierarchyFrame->Delete();
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildSearchTermGUI ( )
{

  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );
    // -------------------------------------------------------------------------------------------------
    // ---
    // BUILD SEARCH TERMS FRAME
    // ---
    // -------------------------------------------------------------------------------------------------
    vtkSlicerModuleCollapsibleFrame *queryFrame = vtkSlicerModuleCollapsibleFrame::New ( );
    queryFrame->SetParent ( page );
    queryFrame->Create ( );
    queryFrame->SetLabelText ("Search Terms");
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
    this->BuildOtherFrame();
    this->PackQueryBuilderContextFrame ( this->StructureFrame );
    app->Script ( "pack %s -side top -fill x -expand 1", this->SwitchQueryFrame->GetWidgetName() );
    this->ColorCodeContextButtons ( this->StructureButton );

    queryFrame->Delete();
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueriesGUI ( )
{
  vtkSlicerApplication *app = vtkSlicerApplication::SafeDownCast(this->GetApplication());
  vtkKWWidget *page = this->UIPanel->GetPageWidget ( "QueryAtlas" );

    vtkSlicerModuleCollapsibleFrame *searchFrame = vtkSlicerModuleCollapsibleFrame::New();
    searchFrame->SetParent ( page);
    searchFrame->Create();
    searchFrame->SetLabelText ("Build Queries");
    searchFrame->CollapseFrame();
    app->Script ( "pack %s -side top -anchor nw -fill x -padx 2 -pady 2 -in %s",
                  searchFrame->GetWidgetName(),
                  this->UIPanel->GetPageWidget("QueryAtlas")->GetWidgetName());

    vtkKWFrame *TF = vtkKWFrame::New();
    TF->SetParent ( searchFrame->GetFrame() );
    TF->Create();
    vtkKWFrame *MF = vtkKWFrame::New();
    MF->SetParent ( searchFrame->GetFrame() );
    MF->Create();
    vtkKWFrame *BF = vtkKWFrame::New();
    BF->SetParent ( searchFrame->GetFrame() );
    BF->Create();    
    this->Script ( "pack %s -side top -padx 0 -pady 0 -fill x -expand y", TF->GetWidgetName() );
    this->Script ( "pack %s -side top -padx 0 -pady 0 -fill x -expand y", MF->GetWidgetName() );
    this->Script ( "pack %s -side top -padx 0 -pady 0 -fill x -expand y", BF->GetWidgetName() );

    // ---
    // Top frame QUERY TARGET WIDGETS
    // ---
    vtkKWLabel *sl = vtkKWLabel::New();
    sl->SetParent ( TF);
    sl->Create();
    sl->SetWidth ( 15 );
    sl->SetText ("search target: ");
    
    this->DatabasesMenuButton = vtkKWMenuButton::New();
    this->DatabasesMenuButton->SetParent ( TF );
    this->DatabasesMenuButton->Create();
    this->DatabasesMenuButton->SetWidth (20);    
    this->DatabasesMenuButton->SetBalloonHelpString ( "Select a target for search." );
    this->BuildDatabasesMenu(this->DatabasesMenuButton->GetMenu() );

    this->SearchButton = vtkKWPushButton::New();
    this->SearchButton->SetParent ( TF );
    this->SearchButton->Create();
    this->SearchButton->SetImageToIcon ( this->QueryAtlasIcons->GetSearchIcon() );
    this->SearchButton->SetBorderWidth ( 0 );
    this->SearchButton->SetReliefToFlat();
    this->SearchButton->SetBalloonHelpString ( "Perform a search on the selected target" );

    app->Script ("grid %s -row 4 -column 0 -padx 0 -pady 2 -sticky w",
                 sl->GetWidgetName() );
    app->Script ("grid %s -row 4 -column 1 -padx 2 -pady 2 -sticky w",
                 this->DatabasesMenuButton->GetWidgetName() );
    app->Script ("grid %s -row 4 -column 2 -padx 2 -pady 2 -sticky w",
                 this->SearchButton->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 0 -weight 0", sl->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 1 -weight 0", this->DatabasesMenuButton->GetWidgetName() );
    app->Script ( "grid columnconfigure %s 2 -weight 0", this->SearchButton->GetWidgetName() );


    // ---
    // Middle frame QUERY RESULTS MANAGER WIDGETS
    // ---

    vtkKWFrame *curF = vtkKWFrame::New();
    curF->SetParent ( MF );
    curF->Create();
    vtkKWFrame *topcurF = vtkKWFrame::New();
    topcurF->SetParent ( MF );
    topcurF->Create();

    this->CurrentResultsList = vtkKWMultiColumnListWithScrollbars::New();
    this->CurrentResultsList->SetParent ( topcurF );
    this->CurrentResultsList->Create();
    this->CurrentResultsList->GetWidget()->SetSelectionModeToMultiple();
    this->CurrentResultsList->GetWidget()->SetWidth ( 0 );
    this->CurrentResultsList->GetWidget()->SetHeight (4 );
    this->CurrentResultsList->GetWidget()->SetSelectionTypeToCell();
    this->CurrentResultsList->GetWidget()->MovableRowsOn();
    this->CurrentResultsList->GetWidget()->MovableColumnsOff();    
    this->CurrentResultsList->HorizontalScrollbarVisibilityOn();
    this->CurrentResultsList->VerticalScrollbarVisibilityOn();
    this->CurrentResultsList->GetWidget()->SortArrowVisibilityOff();
    
    this->CurrentResultsList->GetWidget()->AddColumn ( "view" );
    this->CurrentResultsList->GetWidget()->ColumnEditableOff ( 0 );
    this->CurrentResultsList->GetWidget()->ColumnResizableOff ( 0 );
    this->CurrentResultsList->GetWidget()->ColumnStretchableOff ( 0 );
    this->CurrentResultsList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( 0 );

    this->CurrentResultsList->GetWidget()->AddColumn ( "Working search results" );    
    this->CurrentResultsList->GetWidget()->ColumnEditableOff ( 1 );
    this->CurrentResultsList->GetWidget()->SetColumnWidth ( 1, 0 );
    this->CurrentResultsList->GetWidget()->SetColumnSortModeToAscii ( 1 );
    this->CurrentResultsList->GetWidget()->SetSelectionCommand ( this, "CurrentResultsSelectionCommandCallback");

    this->DeselectAllCurrentResultsButton = vtkKWPushButton::New();
    this->DeselectAllCurrentResultsButton->SetParent (curF);
    this->DeselectAllCurrentResultsButton->Create();
    this->DeselectAllCurrentResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetDeselectAllIcon() );
    this->DeselectAllCurrentResultsButton->SetBorderWidth ( 0 );
    this->DeselectAllCurrentResultsButton->SetReliefToFlat();    
    this->DeselectAllCurrentResultsButton->SetBalloonHelpString ( "Deselect all results");

    this->SelectAllCurrentResultsButton = vtkKWPushButton::New();
    this->SelectAllCurrentResultsButton->SetParent (curF);
    this->SelectAllCurrentResultsButton->Create();
    this->SelectAllCurrentResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetSelectAllIcon() );
    this->SelectAllCurrentResultsButton->SetBorderWidth ( 0 );
    this->SelectAllCurrentResultsButton->SetReliefToFlat();    
    this->SelectAllCurrentResultsButton->SetBalloonHelpString ( "Select all results");

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

    //---
    //--- BottomFrame ACCUMULATED RESULTS WIDGETS
    //---
    vtkKWFrame *pastF = vtkKWFrame::New();
    pastF->SetParent ( BF );
    pastF->Create();
    vtkKWFrame *toppastF = vtkKWFrame::New();
    toppastF->SetParent ( BF );
    toppastF->Create();

    this->AccumulatedResultsList = vtkKWMultiColumnListWithScrollbars::New();
    this->AccumulatedResultsList->SetParent ( toppastF );
    this->AccumulatedResultsList->Create();
    this->AccumulatedResultsList->GetWidget()->SetSelectionModeToMultiple();
    this->AccumulatedResultsList->GetWidget()->SetWidth ( 0 );
    this->AccumulatedResultsList->GetWidget()->SetHeight (4 );
    this->AccumulatedResultsList->GetWidget()->SetSelectionTypeToCell();
    this->AccumulatedResultsList->GetWidget()->MovableRowsOn();
    this->AccumulatedResultsList->GetWidget()->MovableColumnsOff();    
    this->AccumulatedResultsList->HorizontalScrollbarVisibilityOn();
    this->AccumulatedResultsList->VerticalScrollbarVisibilityOn();
    this->AccumulatedResultsList->GetWidget()->SortArrowVisibilityOff ( );

    this->AccumulatedResultsList->GetWidget()->AddColumn ( "view" );
    this->AccumulatedResultsList->GetWidget()->ColumnEditableOff ( 0 );
    this->AccumulatedResultsList->GetWidget()->ColumnResizableOff ( 0 );
    this->AccumulatedResultsList->GetWidget()->ColumnStretchableOff ( 0 );
    this->AccumulatedResultsList->GetWidget()->ColumnStretchableOff ( 0 );
    this->AccumulatedResultsList->GetWidget()->SetColumnFormatCommandToEmptyOutput ( 0 );

    this->AccumulatedResultsList->GetWidget()->AddColumn ( "Reserved search results" );    
    this->AccumulatedResultsList->GetWidget()->ColumnEditableOff ( 1 );
    this->AccumulatedResultsList->GetWidget()->SetColumnWidth ( 1, 0 );
    this->AccumulatedResultsList->GetWidget()->SetColumnSortModeToAscii ( 1 );
    this->AccumulatedResultsList->GetWidget()->SetSelectionCommand ( this, "AccumulatedResultsSelectionCommandCallback");

    this->DeleteAccumulatedResultButton = vtkKWPushButton::New();
    this->DeleteAccumulatedResultButton->SetParent (pastF);
    this->DeleteAccumulatedResultButton->Create();
    this->DeleteAccumulatedResultButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearSelectedIcon ( ) );
    this->DeleteAccumulatedResultButton->SetBorderWidth ( 0 );
    this->DeleteAccumulatedResultButton->SetReliefToFlat ( );    
    this->DeleteAccumulatedResultButton->SetBalloonHelpString ("Delete selected");

    this->DeselectAllAccumulatedResultsButton = vtkKWPushButton::New();
    this->DeselectAllAccumulatedResultsButton->SetParent (pastF);
    this->DeselectAllAccumulatedResultsButton->Create();
    this->DeselectAllAccumulatedResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetDeselectAllIcon() );
    this->DeselectAllAccumulatedResultsButton->SetBorderWidth ( 0 );
    this->DeselectAllAccumulatedResultsButton->SetReliefToFlat();    
    this->DeselectAllAccumulatedResultsButton->SetBalloonHelpString ( "Deselect all results");

    this->SelectAllAccumulatedResultsButton = vtkKWPushButton::New();
    this->SelectAllAccumulatedResultsButton->SetParent (pastF);
    this->SelectAllAccumulatedResultsButton->Create();
    this->SelectAllAccumulatedResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetSelectAllIcon() );
    this->SelectAllAccumulatedResultsButton->SetBorderWidth ( 0 );
    this->SelectAllAccumulatedResultsButton->SetReliefToFlat();    
    this->SelectAllAccumulatedResultsButton->SetBalloonHelpString ( "Select all results");

    this->DeleteAllAccumulatedResultsButton = vtkKWPushButton::New();
    this->DeleteAllAccumulatedResultsButton->SetParent (pastF);
    this->DeleteAllAccumulatedResultsButton->Create();
    this->DeleteAllAccumulatedResultsButton->SetImageToIcon ( this->QueryAtlasIcons->GetClearAllIcon (  ) );
    this->DeleteAllAccumulatedResultsButton->SetBorderWidth ( 0 );
    this->DeleteAllAccumulatedResultsButton->SetReliefToFlat();    
    this->DeleteAllAccumulatedResultsButton->SetBalloonHelpString ("Delete all");

    this->SaveAccumulatedResultsButton = vtkKWLoadSaveButton::New();
    this->SaveAccumulatedResultsButton->SetParent (pastF);
    this->SaveAccumulatedResultsButton->Create();    
    this->SaveAccumulatedResultsButton->SetImageToIcon (  app->GetApplicationGUI()->GetApplicationToolbar()->GetSlicerToolbarIcons()->GetSaveSceneIcon() );   
    this->SaveAccumulatedResultsButton->SetBorderWidth ( 0 );
    this->SaveAccumulatedResultsButton->SetReliefToFlat();    
    this->SaveAccumulatedResultsButton->SetBalloonHelpString ("Save links to file");
    this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->SetTitle("Save Firefox bookmarks file");
    this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->ChooseDirectoryOff();
    this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->SaveDialogOn();
    this->SaveAccumulatedResultsButton->GetLoadSaveDialog()->SetFileTypes ( "{ {Bookmark file} {*.html} }");

    this->LoadURIsButton = vtkKWLoadSaveButton::New();
    this->LoadURIsButton->SetParent ( pastF);
    this->LoadURIsButton->Create();
    this->LoadURIsButton->SetImageToIcon ( app->GetApplicationGUI()->GetApplicationToolbar()->GetSlicerToolbarIcons()->GetLoadSceneIcon() );   
    this->LoadURIsButton->SetBorderWidth(0);
    this->LoadURIsButton->SetReliefToFlat ( );
    this->LoadURIsButton->SetBalloonHelpString ( "Load links from file" );
    this->LoadURIsButton->GetLoadSaveDialog()->SetTitle("Load Firefox bookmarks file");
    this->LoadURIsButton->GetLoadSaveDialog()->ChooseDirectoryOff();
    this->LoadURIsButton->GetLoadSaveDialog()->SaveDialogOff();
    this->LoadURIsButton->GetLoadSaveDialog()->SetFileTypes ( "*.html");

    app->Script( "pack %s -side top -padx 0 -pady 2 -fill both -expand 1", topcurF->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 0 -fill both -expand 1", this->CurrentResultsList->GetWidgetName() );

    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand 1", curF->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 0 -sticky ew -pady 4 -padx 3", this->DeselectAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 1 -sticky ew -pady 4 -padx 3", this->SelectAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 2 -sticky ew -pady 4 -padx 3", this->DeleteCurrentResultButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 3 -sticky ew -pady 4 -padx 3", this->DeleteAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 4 -sticky ew -pady 4 -padx 3", this->SaveCurrentSelectedResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 5 -sticky ew -pady 4 -padx 3", this->SaveCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 0 -weight 1", this->DeselectAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 1 -weight 1", this->SelectAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 2 -weight 1", this->DeleteCurrentResultButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 3 -weight 1", this->DeleteAllCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 4 -weight 1", this->SaveCurrentResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 5 -weight 1", this->SaveCurrentSelectedResultsButton->GetWidgetName() );    

    app->Script( "pack %s -side top -padx 0 -pady 2 -fill both -expand true", toppastF->GetWidgetName() );
    app->Script ("pack %s -side top -padx 0 -pady 0 -fill both -expand true", this->AccumulatedResultsList->GetWidgetName() );

    app->Script ("pack %s -side top -padx 0 -pady 2 -fill x -expand 1", pastF->GetWidgetName() );
    app->Script ("grid %s -row 0 -column 0 -sticky ew -pady 4 -padx 3", this->DeselectAllAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 1 -sticky ew -pady 4 -padx 3", this->SelectAllAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 2 -sticky ew -pady 4 -padx 3", this->DeleteAccumulatedResultButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 3 -sticky ew -pady 4 -padx 3", this->DeleteAllAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 4 -sticky ew -pady 4 -padx 3", this->LoadURIsButton->GetWidgetName() );    
    app->Script ("grid %s -row 0 -column 5 -sticky ew -pady 4 -padx 3", this->SaveAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 0 -weight 1", this->DeselectAllAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 1 -weight 1", this->SelectAllAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 2 -weight 1", this->DeleteAccumulatedResultButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 3 -weight 1", this->DeleteAllAccumulatedResultsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 4 -weight 1", this->LoadURIsButton->GetWidgetName() );    
    app->Script ("grid columnconfigure %s 5 -weight 1", this->SaveAccumulatedResultsButton->GetWidgetName() );    

    topcurF->Delete();
    curF->Delete();
    toppastF->Delete();
    pastF->Delete();
    sl->Delete();
    BF->Delete();
    MF->Delete();
    TF->Delete();
    searchFrame->Delete();
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteAllResults ( vtkKWMultiColumnList *l )
{
  l->DeleteAllRows();
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::DeleteSelectedResults ( vtkKWMultiColumnList *l )
{
  int numRows;
  int row[1000];

  numRows = l->GetSelectedRows ( row );
  while ( numRows != 0 )
    {
      l->DeleteRow ( row[0] );
      numRows = l->GetSelectedRows ( row );
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AppendUniqueResult ( const char *r )
{

  const char *url="";
  int unique = 1;
  vtkKWMultiColumnList *l = this->CurrentResultsList->GetWidget();
  
  //--- determine uniqueness of entry
  if ( l )
    {
    int N = l->GetNumberOfRows();
    for ( int i=0; i < N; i++ )
      {
      url = l->GetCellText ( i, 1);
      if ( !(strcmp (r, url)) )
        {
        unique = 0;
        break;
        }
      }

    //--- add a unique url
    if ( unique == 1 )
      {
      //--- add image in viewer column
      l->InsertCellText (N, 1, r );
      l->SetCellBackgroundColor ( N, 0, 1.0, 1.0, 1.0 );
      l->SetCellBackgroundColor ( N, 1, 1.0, 1.0, 1.0 );
      l->SetCellImageToIcon ( N, 0, this->QueryAtlasIcons->GetWebIcon() );
      }
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AccumulateUniqueResult ( const char *r )
{

  const char *url="";

  int unique = 1;
  vtkKWMultiColumnList *l = this->AccumulatedResultsList->GetWidget();

  if ( l )
    {
    //--- determine uniqueness of entry
    int N = l->GetNumberOfRows();

    for ( int i=0; i < N; i++ )
      {
      url = l->GetCellText ( i, 1);
      if ( !(strcmp (r, url)) )
        {
        unique = 0;
        break;
        }
      }

    //--- add a unique url
    if ( unique == 1 )
      {
      //--- add image in viewer column
      l->InsertCellText (N, 1, r );
      l->SetCellBackgroundColor ( N, 0, 1.0, 1.0, 1.0 );
      l->SetCellBackgroundColor ( N, 1, 1.0, 1.0, 1.0 );
      l->SetCellImageToIcon ( N, 0, this->QueryAtlasIcons->GetWebIcon() );
      }
    }
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::CurrentResultsSelectionCommandCallback ( )
{

  // loop thru the first column and see if anything is selected
  vtkKWMultiColumnList *l = this->CurrentResultsList->GetWidget();
  int numRows = l->GetNumberOfRows();
  int s;
  if ( l )
    {
    for ( int i=0; i<numRows; i++ )
      {
      s = l->IsCellSelected(i,0);
      if ( s )
        {
        //-- launch browser with url
        if ( l->GetCellText(i,1) != NULL )
          {
          this->Script ("QueryAtlasOpenLink \"%s\"", l->GetCellText(i,1) );
          }

        //-- now deselect the cell
        l->DeselectCell (i,0);
        break;
        }
      }
    }
}





//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::AccumulatedResultsSelectionCommandCallback ( )
{

  // loop thru the first column and see if anything is selected
  vtkKWMultiColumnList *l = this->AccumulatedResultsList->GetWidget();
  int numRows = l->GetNumberOfRows();
  int s;
  if ( l )
    {
    for ( int i=0; i<numRows; i++ )
      {
      s = l->IsCellSelected(i, 0 );
      if ( s )
        {
        //-- launch browser with url
        if ( l->GetCellText(i, 1) != NULL )
          {
          this->Script ("QueryAtlasOpenLink \"%s\"", l->GetCellText(i,1) );
          }

        //-- now deselect the cell
        l->DeselectCell (i, 0);
        break;
        }
      }
    }
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
    this->SpeciesMacaqueButton->SetSelectedState( 0 );

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
    this->HandednessMenuButton->GetWidget()->GetMenu()->AddRadioButton ("both");
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
    this->Script ( "pack forget %s", this->GeneralFrame->GetWidgetName() );
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
  this->FIPSFSButton->SetBalloonHelpString ( "Load, annotate and display FIPS+FreeSurfer datasets" );

  this->QdecButton = vtkKWPushButton::New();
  this->QdecButton->SetParent ( f );
  this->QdecButton->Create();
  this->QdecButton->SetWidth ( 12 );
  this->QdecButton->SetText ( "Qdec");
  this->QdecButton->SetBalloonHelpString ( "Load, annotate and display a Qdec dataset" );

  this->GeneralButton = vtkKWPushButton::New();
  this->GeneralButton->SetParent ( f );
  this->GeneralButton->Create();
  this->GeneralButton->SetWidth ( 12) ;
  this->GeneralButton->SetText ( "Basic" );
  this->GeneralButton->SetBalloonHelpString ( "Annotate models and/or label maps" );
  this->Script ( "pack %s %s %s -anchor nw -side left -fill none -padx 2 -pady 0",
                 this->GeneralButton->GetWidgetName(),
                 this->FIPSFSButton->GetWidgetName(),
                 this->QdecButton->GetWidgetName() );
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

    this->GeneralFrame = vtkKWFrame::New();
    this->GeneralFrame->SetParent ( parent );
    this->GeneralFrame->Create();
    this->GeneralFrame->SetReliefToGroove();
    this->GeneralFrame->SetBorderWidth ( 1 );    
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::ColorCodeLoaderContextButtons ( vtkKWPushButton *b )
{
  this->FIPSFSButton->SetBackgroundColor ( _br, _bg, _bb );
  this->QdecButton->SetBackgroundColor ( _br, _bg, _bb );
  this->GeneralButton->SetBackgroundColor (_br, _bg, _bb );
                                           
  this->FIPSFSButton->SetForegroundColor ( _fr, _fg, _fb );
  this->QdecButton->SetForegroundColor ( _fr, _fg, _fb );
  this->GeneralButton->SetForegroundColor( _fr, _fg, _fb );

  b->SetBackgroundColor (1.0, 1.0, 1.0);
  b->SetForegroundColor (0.0, 0.0, 0.0);
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildOtherFrame()
{
    vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();

    // add multi-column list box for search terms

    this->OtherListWidget = vtkQueryAtlasUseSearchTermWidget::New();
    this->OtherListWidget->SetParent ( this->OtherFrame );
    this->OtherListWidget->Create ( );
    this->OtherListWidget->GetMultiColumnList()->GetWidget()->SetHeight(3);
//    int i = this->OtherListWidget->GetMultiColumnList()->GetWidget()->GetColumnIndexWithName ( "Search terms" );
//    this->OtherListWidget->GetMultiColumnList()->GetWidget()->SetColumnName ( i, "Other search terms");
    app->Script ( "pack %s -side top -fill x -expand true", this->OtherListWidget->GetWidgetName() );

}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::UnpackQueryBuilderContextFrames ( )
{
    this->Script ( "pack forget %s", this->OtherFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->StructureFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->PopulationFrame->GetWidgetName() );
    this->Script ( "pack forget %s", this->SpeciesFrame->GetWidgetName() );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::PackQueryBuilderContextFrame ( vtkKWFrame *f )
{
  vtkSlicerApplication *app = (vtkSlicerApplication *)this->GetApplication();
  if ( app )
    {
    app->Script ( "pack %s -side top -anchor nw -expand 0 -fill x", f->GetWidgetName( ));
    }
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextFrames ( vtkKWFrame *parent )
{
    
    this->StructureFrame = vtkKWFrame::New();
    this->StructureFrame->SetParent ( parent );
    this->StructureFrame->Create();
    
    this->PopulationFrame = vtkKWFrame::New();
    this->PopulationFrame->SetParent ( parent );
    this->PopulationFrame->Create();
    this->PopulationFrame->SetReliefToGroove();
    this->PopulationFrame->SetBorderWidth ( 1 );    

    this->SpeciesFrame = vtkKWFrame::New();
    this->SpeciesFrame->SetParent ( parent );
    this->SpeciesFrame->Create();
    this->SpeciesFrame->SetReliefToGroove();
    this->SpeciesFrame->SetBorderWidth ( 1 );

    this->OtherFrame = vtkKWFrame::New();
    this->OtherFrame->SetParent ( parent );
    this->OtherFrame->Create();
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildQueryBuilderContextButtons ( vtkKWFrame *parent )
{

    vtkKWFrame *f = vtkKWFrame::New();
    f->SetParent ( parent );
    f->Create();
    this->Script ("pack %s -side top -anchor nw -fill none", f->GetWidgetName() );

    this->UseOtherTerms = vtkKWCheckButton::New();
    this->UseOtherTerms->SetParent ( f );
    this->UseOtherTerms->Create();
    this->UseOtherTerms->SetText ( "" );
    this->UseOtherTerms->SetSelectedState ( 0 );
    this->UseOtherTerms->SetBalloonHelpString ("Use these terms in the search");

    this->UseStructureTerms = vtkKWCheckButton::New();
    this->UseStructureTerms->SetParent ( f );
    this->UseStructureTerms->Create();
    this->UseStructureTerms->SetText ( "" );
    this->UseStructureTerms->SetSelectedState ( 1 );
    this->UseStructureTerms->SetBalloonHelpString ("Use these terms in the search");

    this->UseGroupTerms = vtkKWCheckButton::New();
    this->UseGroupTerms->SetParent ( f );
    this->UseGroupTerms->Create();
    this->UseGroupTerms->SetText ( "" );
    this->UseGroupTerms->SetSelectedState ( 0 );
    this->UseGroupTerms->SetBalloonHelpString ("Use these terms in the search");

    this->UseSpeciesTerms = vtkKWCheckButton::New();
    this->UseSpeciesTerms->SetParent ( f );
    this->UseSpeciesTerms->Create();
    this->UseSpeciesTerms->SetText ( "" );
    this->UseSpeciesTerms->SetSelectedState ( 0 );
    this->UseSpeciesTerms->SetBalloonHelpString ("Use these terms in the search");

    vtkKWLabel *useL = vtkKWLabel::New();
    useL->SetParent ( f );
    useL->Create();
    useL->SetText( "use: ");

    this->Script ( "grid %s -row 0 -column 0 -padx 0 -pady 1 -sticky e", useL->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 1 -padx 4 -pady 1 -sticky ew", this->UseOtherTerms->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 2 -padx 4 -pady 1 -sticky ew", this->UseStructureTerms->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 3 -padx 4 -pady 1 -sticky ew", this->UseGroupTerms->GetWidgetName() );
    this->Script ( "grid %s -row 0 -column 4 -padx 4 -pady 1 -sticky ew", this->UseSpeciesTerms->GetWidgetName() );

    // for now this will be the "other" term repository...
    // when we flesh this out with multiscale categories,
    // 
    this->OtherButton = vtkKWPushButton::New();
    this->OtherButton->SetParent ( f );
    this->OtherButton->Create();
    this->OtherButton->SetWidth ( 8 );
    this->OtherButton->SetText ( "other");
    this->OtherButton->SetBalloonHelpString ("specify and select other search terms");
    
    this->StructureButton = vtkKWPushButton::New();
    this->StructureButton->SetParent ( f );
    this->StructureButton->Create();
    this->StructureButton->SetWidth ( 8);
    this->StructureButton->SetText ( "structure");
    this->StructureButton->SetBalloonHelpString ("specify and select structure search terms");
    
    this->PopulationButton = vtkKWPushButton::New();
    this->PopulationButton->SetParent ( f );
    this->PopulationButton->Create();
    this->PopulationButton->SetWidth ( 8 );
    this->PopulationButton->SetText ( "group");
    this->PopulationButton->SetBalloonHelpString ("Choose population search terms");
    
    this->SpeciesButton = vtkKWPushButton::New();
    this->SpeciesButton->SetParent ( f );
    this->SpeciesButton->Create();
    this->SpeciesButton->SetWidth ( 8 );
    this->SpeciesButton->SetText ( "species");    
    this->SpeciesButton->SetBalloonHelpString ("Choose species search terms");

    vtkKWLabel *chooseL = vtkKWLabel::New();
    chooseL->SetParent ( f );
    chooseL->Create();
    chooseL->SetText ( "choose: ");
    
    this->Script ( "grid %s -row 1 -column 0 -padx 0 -pady 0 -sticky e", chooseL->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 1 -padx 2 -pady 0 -sticky ew", this->OtherButton->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 2 -padx 2 -pady 0 -sticky ew", this->StructureButton->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 3 -padx 2 -pady 0 -sticky ew", this->PopulationButton->GetWidgetName() );
    this->Script ( "grid %s -row 1 -column 4 -padx 2 -pady 0 -sticky ew", this->SpeciesButton->GetWidgetName() );

    this->Script ( "grid columnconfigure %s 0 -weight 0", useL->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 1 -weight 0", this->UseOtherTerms->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 2 -weight 0", this->UseStructureTerms->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 3 -weight 0", this->UseGroupTerms->GetWidgetName() );
    this->Script ( "grid columnconfigure %s 4 -weight 0", this->UseSpeciesTerms->GetWidgetName() );

    useL->Delete();
    chooseL->Delete();
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
  m->AddRadioButton ("PubMedCentral");
  m->AddRadioButton ("JNeurosci");
  m->AddRadioButton ("Metasearch");
  m->AddRadioButton ("PLoSone");
  m->AddSeparator();
  m->AddRadioButton ("IBVD");
  m->AddRadioButton ("BrainInfo");
  m->AddSeparator();
  m->AddCommand ( "close");
}


//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::BuildDiagnosisMenu( vtkKWMenu *m )
{
  m->DeleteAllItems( );
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
void vtkQueryAtlasGUI::AddToDiagnosisMenu ( vtkKWMenu *m, const char *diagnosis )
{
  this->BuildDiagnosisMenu ( m );
  m->AddRadioButton ( diagnosis );
  m->SelectItem ( diagnosis );
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::GetDiagnosisTerms ( )
{

  const char *term;
  
  this->DiagnosisTerms.clear();
  //-- get diagnosis
  term = this->GetDiagnosisMenuButton()->GetWidget()->GetValue();
  this->DiagnosisTerms.push_back ( std::string(term) );

  //-- get gender
  term = this->GetGenderMenuButton()->GetWidget()->GetValue();
  this->DiagnosisTerms.push_back ( std::string(term) );

  //-- get age
  term = this->GetAgeMenuButton()->GetWidget()->GetValue();
  this->DiagnosisTerms.push_back ( std::string(term) );
  
  //-- get handedness
  term = this->GetHandednessMenuButton()->GetWidget()->GetValue();
  this->DiagnosisTerms.push_back ( std::string(term) );

}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::GetSpeciesTerms ( )
{
  this->SpeciesTerms.clear();
  if ( this->SpeciesHumanButton->GetSelectedState() )
    {
    this->SpeciesTerms.push_back ( std::string ("human" ) );
    }
  if ( this->SpeciesMouseButton->GetSelectedState() )
    {
    this->SpeciesTerms.push_back ( std::string ("mouse" ) );
    }
  if ( this->SpeciesMacaqueButton->GetSelectedState() )
    {
    this->SpeciesTerms.push_back ( std::string ("macaque" ) );
    }
}




//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::GetStructureTerms ( )
{

  this->StructureTerms.clear();
  // this counts the number of selected items instead of the number of CHECKED items
  int num = this->StructureListWidget->GetNumberOfSearchTermsToUse();
  for ( int i = 0; i < num; i++ )
    {
    this->StructureTerms.push_back ( std::string (this->StructureListWidget->GetNthSearchTermToUse ( i ) ) );
    }
}



//---------------------------------------------------------------------------
void vtkQueryAtlasGUI::GetOtherTerms ( )
{

  this->OtherTerms.clear();
  int num = this->StructureListWidget->GetNumberOfSearchTermsToUse();
  for ( int i = 0; i < num; i++ )
    {
    this->StructureTerms.push_back ( std::string (this->StructureListWidget->GetNthSearchTermToUse ( i ) ) );
    }

}



