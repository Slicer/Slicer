// .NAME vtkQueryAtlasGUI
// .SECTION Description
// GUI for the Query Atlas


#ifndef __vtkQueryAtlasGUI_h
#define __vtkQueryAtlasGUI_h

#include "vtkQueryAtlasWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkQueryAtlasLogic.h"
#include "vtkQueryAtlasCollaboratorIcons.h"
#include "vtkQueryAtlasIcons.h"
#include "vtkIntArray.h"

class vtkKWPushButton;
class vtkKWPushButtonWithLabel;
class vtkKWMultiColumnList;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWCheckButton;
class vtkKWEntry;
class vtkKWEntryWithLabel;
class vtkKWLoadSaveButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWListBox;
class vtkKWListBoxWithScrollbars;
class vtkQueryAtlasUseSearchTermWidget;
class vtkQueryAtlasSearchTermWidget;


// Description:
// This class implements Slicer's QueryAtlas GUI
//

class VTK_QUERYATLAS_EXPORT vtkQueryAtlasGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkQueryAtlasGUI* New (  );
    vtkTypeRevisionMacro ( vtkQueryAtlasGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( Logic, vtkQueryAtlasLogic);

    // Description:
    // Get container for logos of all collaborators
    vtkGetObjectMacro ( CollaboratorIcons, vtkQueryAtlasCollaboratorIcons );

    // Description:
    // Get container for module icons
    vtkGetObjectMacro ( QueryAtlasIcons, vtkQueryAtlasIcons );
    
    // Annotation Options frame and widgets
    vtkGetObjectMacro (AnnotationTermSetMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro (QuerySceneVisibilityMenuButton, vtkKWMenuButton );

    // Querybuilder frame top widgets
    vtkGetObjectMacro (OtherButton, vtkKWPushButton );
    vtkGetObjectMacro (StructureButton, vtkKWPushButton );
    vtkGetObjectMacro (PopulationButton, vtkKWPushButton );
    vtkGetObjectMacro (SpeciesButton, vtkKWPushButton );
    // Querybuilder frame containing alternate sets of widgets
    vtkGetObjectMacro (SwitchQueryFrame, vtkKWFrame);
    
    // species panel
    vtkGetObjectMacro (SpeciesFrame, vtkKWFrame);
    vtkGetObjectMacro (SpeciesLabel, vtkKWLabel);
    vtkGetObjectMacro (SpeciesNoneButton, vtkKWCheckButton );
    vtkGetObjectMacro (SpeciesHumanButton, vtkKWCheckButton );
    vtkGetObjectMacro (SpeciesMouseButton, vtkKWCheckButton);
    vtkGetObjectMacro (SpeciesMacaqueButton, vtkKWCheckButton );
    
    // population panel
    vtkGetObjectMacro (PopulationFrame, vtkKWFrame );
    vtkGetObjectMacro (DiagnosisMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (HandednessMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (GenderMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (AgeMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (AddDiagnosisEntry, vtkKWEntryWithLabel );
    
    // structure panel
    vtkGetObjectMacro (StructureFrame, vtkKWFrame );
    vtkGetObjectMacro ( StructureMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro ( StructureListWidget, vtkQueryAtlasUseSearchTermWidget );

    // cell panel
    vtkGetObjectMacro ( OtherListWidget, vtkQueryAtlasUseSearchTermWidget );
    vtkGetObjectMacro (OtherFrame, vtkKWFrame);

    // search panel
    vtkGetObjectMacro ( DatabasesMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( SearchButton, vtkKWPushButton );
    vtkGetObjectMacro ( UseOtherTerms, vtkKWCheckButton );
    vtkGetObjectMacro ( UseStructureTerms, vtkKWCheckButton );    
    vtkGetObjectMacro ( UseGroupTerms, vtkKWCheckButton );
    vtkGetObjectMacro ( UseSpeciesTerms, vtkKWCheckButton );
    vtkGetObjectMacro (ResultsWithAnyButton, vtkKWRadioButton);
    vtkGetObjectMacro (ResultsWithAllButton, vtkKWRadioButton);
    vtkGetObjectMacro (ResultsWithExactButton, vtkKWRadioButton);

    //hierarchies panel
    vtkGetObjectMacro ( LocalSearchTermEntry, vtkKWEntry );
    vtkGetObjectMacro ( SynonymsMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( BIRNLexEntry, vtkKWEntry );
    vtkGetObjectMacro (BIRNLexIDEntry, vtkKWEntry );
    vtkGetObjectMacro ( NeuroNamesEntry, vtkKWEntry );
    vtkGetObjectMacro ( NeuroNamesIDEntry, vtkKWEntry );
    vtkGetObjectMacro ( UMLSCIDEntry, vtkKWEntry );
    vtkGetObjectMacro ( UMLSCNEntry, vtkKWEntry );

    vtkGetObjectMacro ( AddLocalTermButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddSynonymButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddBIRNLexStringButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddBIRNLexIDButton, vtkKWPushButton );    
    vtkGetObjectMacro ( AddNeuroNamesStringButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddNeuroNamesIDButton, vtkKWPushButton );    
    vtkGetObjectMacro ( AddUMLSCIDButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddUMLSCNButton, vtkKWPushButton );

    vtkGetObjectMacro ( BIRNLexHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( NeuroNamesHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( UMLSHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( SavedTerms, vtkQueryAtlasSearchTermWidget );

    // results frame?
    vtkGetObjectMacro ( CurrentResultsList, vtkKWMultiColumnListWithScrollbars );
    vtkGetObjectMacro ( AccumulatedResultsList, vtkKWMultiColumnListWithScrollbars );    
    
    vtkGetObjectMacro ( DeleteCurrentResultButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAllCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveCurrentSelectedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeselectAllCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SelectAllCurrentResultsButton, vtkKWPushButton );

    vtkGetObjectMacro ( DeselectAllAccumulatedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SelectAllAccumulatedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAccumulatedResultButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAllAccumulatedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveAccumulatedResultsButton, vtkKWLoadSaveButton );
    vtkGetObjectMacro ( LoadURIsButton, vtkKWLoadSaveButton );
    
    vtkGetMacro ( NumberOfColumns, int );

    // load frame
    vtkGetObjectMacro (FSasegSelector, vtkSlicerNodeSelectorWidget );
    vtkGetObjectMacro (FSbrainSelector, vtkSlicerNodeSelectorWidget );
    vtkGetObjectMacro (FSstatsSelector, vtkSlicerNodeSelectorWidget );    
    vtkGetObjectMacro (FSgoButton, vtkKWPushButtonWithLabel );
    vtkGetObjectMacro (LoadFIPSFSCatalogButton, vtkKWLoadSaveButtonWithLabel );
    vtkGetObjectMacro (QdecGetResultsButton, vtkKWLoadSaveButtonWithLabel );
    vtkGetObjectMacro (QdecScalarSelector, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (QdecGoButton, vtkKWPushButtonWithLabel);

    vtkGetObjectMacro ( BasicAnnotateButton, vtkKWPushButtonWithLabel );
    vtkGetObjectMacro (GeneralButton, vtkKWPushButton);
    vtkGetObjectMacro (GeneralFrame, vtkKWFrame);
    vtkGetObjectMacro (FIPSFSButton, vtkKWPushButton );
    vtkGetObjectMacro (QdecButton, vtkKWPushButton );
    vtkGetObjectMacro (FIPSFSFrame, vtkKWFrame );
    vtkGetObjectMacro (QdecFrame, vtkKWFrame );

    vtkGetMacro (ProcessingMRMLEvent, int);
    
    virtual void SetModuleLogic ( vtkSlicerLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), 
                       dynamic_cast<vtkQueryAtlasLogic*>(logic) ); }
    virtual void SetAndObserveModuleLogic ( vtkSlicerLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), 
                                 dynamic_cast<vtkQueryAtlasLogic*>(logic));}
    
    // Description:
    // This method builds the QueryAtlas module GUI
    virtual void BuildGUI ( ) ;
    virtual void BuildAcknowledgementPanel ( );
    virtual void BuildLoadAndConvertGUI ( );
    virtual void BuildAnnotationOptionsGUI ( );
    virtual void BuildOntologyGUI ( );
    virtual void BuildSearchTermGUI ( );
    virtual void BuildQueriesGUI ( );
    virtual void BuildDisplayAndNavigationGUI ( );

    // Description:
    // Clear all the widgets    
    virtual void ClearOntologyGUI ( );
    
    // Description:
    // Assign colors that encode cell label and modelID
    virtual vtkDataArray *AssignCellColorCode ( int numCells,
                                                    int nextCellIndex,
                                                    vtkDataArray *cellNumberColors );

    // Description:
    // Helper methods for building the complicated GUI

    virtual void UnpackLoaderContextFrames ( );
    virtual void PackLoaderContextFrame ( vtkKWFrame *f );
    virtual void BuildLoaderContextButtons ( vtkKWFrame *parent );
    virtual void ColorCodeLoaderContextButtons ( vtkKWPushButton *b );
    virtual void BuildLoaderContextFrames ( vtkKWFrame *parent );

    virtual void BuildFreeSurferFIPSFrame( );
    virtual void BuildQdecFrame();
    virtual void BuildGeneralAnnotateFrame ( );

    virtual void UnpackQueryBuilderContextFrames ( );
    virtual void PackQueryBuilderContextFrame ( vtkKWFrame *f );
    virtual void BuildQueryBuilderContextButtons ( vtkKWFrame *parent );
    virtual void ColorCodeContextButtons ( vtkKWPushButton *b );
    virtual void BuildQueryBuilderContextFrames ( vtkKWFrame *parent );

    virtual void BuildSpeciesFrame();
    virtual void BuildPopulationFrame();
    virtual void BuildStructureFrame ( );
    virtual void BuildOtherFrame();
    virtual void BuildDiagnosisMenu( vtkKWMenu *m );
    virtual void AddToDiagnosisMenu( vtkKWMenu *m, const char *diagnosis );
    virtual void BuildDatabasesMenu( vtkKWMenu *m );
                                    
    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );


    // Description:
    // Add/Remove observers on MRML
    virtual void AddMRMLObservers ( );
    virtual void TearDownGUI ( );
    
    // Description:
    // Get terms from the Diagosis panel
    virtual void GetDiagnosisTerms ( );
    // Description:
    // Get terms from the species panel
    virtual void GetSpeciesTerms ( );
    // Description:
    // Get terms from the structure panel
    virtual void GetStructureTerms ( );
    // Description:
    // Get terms from the other panel
    virtual void GetOtherTerms ( );

    virtual void AppendUniqueResult ( const char *r );
    virtual void AccumulateUniqueResult ( const char *r );
    virtual void DeleteAllResults(vtkKWMultiColumnList *l );
    virtual void DeleteSelectedResults(vtkKWMultiColumnList *l );
    virtual void CurrentResultsSelectionCommandCallback ( );
    virtual void AccumulatedResultsSelectionCommandCallback ( );
    
    //BTX
    std::vector<std::string> DiagnosisTerms;
    std::vector<std::string> SpeciesTerms;
    std::vector<std::string> StructureTerms;
    std::vector<std::string> OtherTerms;
    //ETX

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    virtual vtkIntArray* NewObservableEvents();

    // Description:
    // load the tcl routines into the interpreter (uses the 
    // Slicer3_BIN global variable to find the path to the 
    // pkgIndex.tcl file in the binary/installation tree
    void LoadTclPackage();

    // Description:
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    // Description:
    // methods for writing and reading bookmark files
    void WriteBookmarksCallback();
    void LoadBookmarksCallback();

    // Description:
    // method to load precomputed Qdec results
    // thru the QdecModule Logic
    void LoadQdecResultsCallback();

    // Description:
    // method to load an xcede catalog
    // containing FIPS and FreeSurfer results
    void LoadXcedeCatalogCallback();
    
    // Description:
    // populates the menu with loaded qdec scalar overlays
    void UpdateScalarOverlayMenu ( );

    // Description:
    // populates the model/annotation visibility menu with query models
    void UpdateAnnoVisibilityMenu ( );

    // Description:
    // method lets you set the visibility of annotations or models in the scene.
    // useful in case you want to peek under a model to see a slice plane.
    void ModifyQuerySceneVisibility();
    
    // Description:
    // Displays a selected scalar overlay on a loaded Qdec scene.
    void DisplayScalarOverlay();
    
    // Description:
    // Apply some conditioning to a volume that
    // appears to be a statistics volume (has 'stat' in
    // its name) upon load.
    virtual void AutoWinLevThreshStatisticsVolume ( vtkMRMLScalarVolumeNode *vnode );
    
 protected:
    vtkQueryAtlasGUI ( );
    virtual ~vtkQueryAtlasGUI ( );
    vtkQueryAtlasGUI ( const vtkQueryAtlasGUI& ); // Not implemented.
    void operator = ( const vtkQueryAtlasGUI& ); //Not implemented.

    // Module logic and mrml pointers
    vtkQueryAtlasLogic *Logic;

    vtkQueryAtlasCollaboratorIcons *CollaboratorIcons;
    vtkQueryAtlasIcons *QueryAtlasIcons;
    
    int ProcessingMRMLEvent;
    // load / configure frame
    vtkSlicerNodeSelectorWidget *FSasegSelector;
    vtkSlicerNodeSelectorWidget *FSbrainSelector;
    vtkSlicerNodeSelectorWidget *FSstatsSelector;
    vtkKWPushButtonWithLabel *FSgoButton;

    vtkKWLoadSaveButtonWithLabel *LoadFIPSFSCatalogButton;
    vtkKWLoadSaveButtonWithLabel *QdecGetResultsButton;
    vtkKWMenuButtonWithLabel *QdecScalarSelector;
    vtkKWPushButtonWithLabel *QdecGoButton;
    
    vtkKWPushButtonWithLabel *BasicAnnotateButton;
    
    vtkKWPushButton *GeneralButton;
    vtkKWPushButton *FIPSFSButton;
    vtkKWPushButton *QdecButton;
    vtkKWFrame *GeneralFrame;
    vtkKWFrame *FIPSFSFrame;
    vtkKWFrame *QdecFrame;

    // Annotation Options frame and widgets
    vtkKWMenuButton *AnnotationTermSetMenuButton;
    vtkKWMenuButton *QuerySceneVisibilityMenuButton;
    
    // ontology frame
    vtkKWEntry *LocalSearchTermEntry;
    vtkKWMenuButton *SynonymsMenuButton;
    vtkKWEntry *BIRNLexEntry;
    vtkKWEntry *BIRNLexIDEntry;
    vtkKWEntry *NeuroNamesEntry;
    vtkKWEntry *NeuroNamesIDEntry;
    vtkKWEntry *UMLSCIDEntry;
    vtkKWEntry *UMLSCNEntry;
    vtkKWPushButton *AddLocalTermButton;
    vtkKWPushButton *AddSynonymButton;
    vtkKWPushButton *AddBIRNLexStringButton;
    vtkKWPushButton *AddBIRNLexIDButton;
    vtkKWPushButton *AddNeuroNamesStringButton;
    vtkKWPushButton *AddNeuroNamesIDButton;
    vtkKWPushButton *AddUMLSCIDButton;    
    vtkKWPushButton *AddUMLSCNButton;    
    vtkKWPushButton *BIRNLexHierarchyButton;
    vtkKWPushButton *NeuroNamesHierarchyButton;
    vtkKWPushButton *UMLSHierarchyButton;
    vtkQueryAtlasSearchTermWidget *SavedTerms;
    
    int NumberOfColumns;
    
    // searchterm builder
    vtkKWPushButton *OtherButton;
    vtkKWPushButton *StructureButton;
    vtkKWPushButton *PopulationButton;
    vtkKWPushButton *SpeciesButton;
    vtkKWFrame *SwitchQueryFrame;
    
    // species frame
    vtkKWFrame *SpeciesFrame;
    vtkKWLabel *SpeciesLabel;
    vtkKWCheckButton *SpeciesNoneButton;
    vtkKWCheckButton *SpeciesHumanButton;
    vtkKWCheckButton *SpeciesMouseButton;
    vtkKWCheckButton *SpeciesMacaqueButton;

    // population frame
    vtkKWFrame *PopulationFrame;
    vtkKWMenuButtonWithLabel *DiagnosisMenuButton;
    vtkKWMenuButtonWithLabel *GenderMenuButton;
    vtkKWMenuButtonWithLabel *HandednessMenuButton;
    vtkKWMenuButtonWithLabel *AgeMenuButton;
    vtkKWEntryWithLabel *AddDiagnosisEntry;

    // structure frame
    vtkKWFrame *StructureFrame;
    vtkKWMenuButtonWithLabel *StructureMenuButton;
    vtkQueryAtlasUseSearchTermWidget *StructureListWidget;

    // querybuilder
    vtkKWPushButton *SearchButton;
    vtkKWMenuButton *DatabasesMenuButton;
    vtkKWRadioButton *ResultsWithAnyButton;
    vtkKWRadioButton *ResultsWithAllButton;
    vtkKWRadioButton *ResultsWithExactButton;
    vtkKWCheckButton *UseOtherTerms;
    vtkKWCheckButton *UseStructureTerms;
    vtkKWCheckButton *UseGroupTerms;
    vtkKWCheckButton *UseSpeciesTerms;
    
    // results frame
    vtkKWMultiColumnListWithScrollbars *CurrentResultsList;
    vtkKWMultiColumnListWithScrollbars *AccumulatedResultsList;

    vtkKWPushButton *DeleteCurrentResultButton;
    vtkKWPushButton *DeleteAllCurrentResultsButton;
    vtkKWPushButton *SaveCurrentResultsButton;
    vtkKWPushButton *SaveCurrentSelectedResultsButton;
    vtkKWPushButton *DeselectAllCurrentResultsButton;
    vtkKWPushButton *SelectAllCurrentResultsButton;

    vtkKWPushButton *DeselectAllAccumulatedResultsButton;
    vtkKWPushButton *SelectAllAccumulatedResultsButton;
    vtkKWPushButton *DeleteAccumulatedResultButton;
    vtkKWPushButton *DeleteAllAccumulatedResultsButton;
    vtkKWLoadSaveButton *SaveAccumulatedResultsButton;
    vtkKWLoadSaveButton *LoadURIsButton;

    // cell frame
    vtkKWFrame *OtherFrame;
    vtkQueryAtlasUseSearchTermWidget *OtherListWidget;

    void OpenOntologyBrowser();

    bool SceneClosing;
    
    //BTX
    // Description:
    // The column orders in the list box
    // add here as the need arises.
    enum
      {
        SelectionColumn = 0,
        SearchTermColumn = 1,
      };
    //ETX

    // DUMP ALL STATE HERE FOR NOW.
    // move all this to MRML Node
    int SearchOption;
    //BTX
    enum
      {
        And = 0,
        Or = 1,
        Quote = 2
      };
    //ETX

private:
};


#endif
