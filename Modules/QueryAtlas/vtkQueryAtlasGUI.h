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

class vtkKWPushButton;
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
    vtkGetObjectMacro (ModelVisibilityButton, vtkKWPushButton );
    vtkGetObjectMacro (AnnotationVisibilityButton, vtkKWPushButton );
    vtkGetObjectMacro (AnnotationTermSetMenuButton, vtkKWMenuButton );
    vtkGetMacro ( AnnotationVisibility, int );
    vtkGetMacro ( ModelVisibility, int );

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
    vtkGetObjectMacro (SpeciesNoneButton, vtkKWRadioButton );
    vtkGetObjectMacro (SpeciesHumanButton, vtkKWRadioButton );
    vtkGetObjectMacro (SpeciesMouseButton, vtkKWRadioButton);
    vtkGetObjectMacro (SpeciesMacaqueButton, vtkKWRadioButton );
    
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

    vtkGetObjectMacro ( AddLocalTermButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddSynonymButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddBIRNLexStringButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddBIRNLexIDButton, vtkKWPushButton );    
    vtkGetObjectMacro ( AddNeuroNamesStringButton, vtkKWPushButton );
    vtkGetObjectMacro ( AddNeuroNamesIDButton, vtkKWPushButton );    
    vtkGetObjectMacro ( AddUMLSCIDButton, vtkKWPushButton );

    vtkGetObjectMacro ( BIRNLexHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( NeuroNamesHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( UMLSHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( SavedTerms, vtkQueryAtlasSearchTermWidget );

    // results frame?
    vtkGetObjectMacro ( CurrentResultsList, vtkKWListBoxWithScrollbars );
    vtkGetObjectMacro ( DeleteCurrentResultButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAllCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveCurrentSelectedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeselectAllCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeselectAllAccumulatedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( AccumulatedResultsList, vtkKWListBoxWithScrollbars );    
    vtkGetObjectMacro ( DeleteAccumulatedResultButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAllAccumulatedResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveAccumulatedResultsButton, vtkKWLoadSaveButton );
    vtkGetObjectMacro ( LoadURIsButton, vtkKWLoadSaveButton );
    
    vtkGetMacro ( NumberOfColumns, int );

    // load frame
    vtkGetObjectMacro (FSasegSelector, vtkSlicerNodeSelectorWidget );
    vtkGetObjectMacro (FSmodelSelector, vtkSlicerNodeSelectorWidget );
    vtkGetObjectMacro (FSbrainSelector, vtkSlicerNodeSelectorWidget );
    vtkGetObjectMacro (FSstatsSelector, vtkSlicerNodeSelectorWidget );    
    vtkGetObjectMacro (FSgoButton, vtkKWPushButton );
    vtkGetObjectMacro (QdecGetResultsButton, vtkKWLoadSaveButtonWithLabel );
    vtkGetObjectMacro (QdecModelSelector, vtkSlicerNodeSelectorWidget );    
    vtkGetObjectMacro (QdecScalarSelector, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (QdecGoButton, vtkKWPushButton);

    vtkGetObjectMacro (FIPSFSButton, vtkKWPushButton );
    vtkGetObjectMacro (QdecButton, vtkKWPushButton );
    vtkGetObjectMacro (FIPSFSFrame, vtkKWFrame );
    vtkGetObjectMacro (QdecFrame, vtkKWFrame );

    vtkGetMacro (ProcessingMRMLEvent, int);
    
    void SetModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

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
    // Helper methods for building the complicated GUI

    virtual void UnpackLoaderContextFrames ( );
    virtual void PackLoaderContextFrame ( vtkKWFrame *f );
    virtual void BuildLoaderContextButtons ( vtkKWFrame *parent );
    virtual void ColorCodeLoaderContextButtons ( vtkKWPushButton *b );
    virtual void BuildLoaderContextFrames ( vtkKWFrame *parent );

    virtual void BuildFreeSurferFIPSFrame( );
    virtual void BuildQdecFrame();

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

    // Description:
    // Opens a url in browser when
    // the current results listbox
    // item is double-clicked
    virtual void OpenLinkFromCurrentList ( );
    // Description:
    // Opens a url in browser when
    // the accumulated results listbox
    // item is double-clicked
    virtual void OpenLinkFromAccumulatedList ( );
    
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

    // Description:
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    // Description:
    // methods for writing and reading bookmark files
    void WriteBookmarksCallback();
    void LoadBookmarksCallback();

 protected:
    vtkQueryAtlasGUI ( );
    virtual ~vtkQueryAtlasGUI ( );

    // Module logic and mrml pointers
    vtkQueryAtlasLogic *Logic;

    vtkQueryAtlasCollaboratorIcons *CollaboratorIcons;
    vtkQueryAtlasIcons *QueryAtlasIcons;
    
    int ProcessingMRMLEvent;
    // load / configure frame
    vtkSlicerNodeSelectorWidget *FSasegSelector;
    vtkSlicerNodeSelectorWidget *FSmodelSelector;
    vtkSlicerNodeSelectorWidget *FSbrainSelector;
    vtkSlicerNodeSelectorWidget *FSstatsSelector;
    vtkKWPushButton *FSgoButton;
    vtkKWLoadSaveButtonWithLabel *QdecGetResultsButton;
    vtkKWMenuButtonWithLabel *QdecScalarSelector;
    vtkSlicerNodeSelectorWidget *QdecModelSelector;
    vtkKWPushButton *QdecGoButton;
    
    vtkKWPushButton *FIPSFSButton;
    vtkKWPushButton *QdecButton;
    vtkKWFrame *FIPSFSFrame;
    vtkKWFrame *QdecFrame;

    // Annotation Options frame and widgets
    vtkKWPushButton *AnnotationVisibilityButton;
    vtkKWPushButton *ModelVisibilityButton;
    vtkKWMenuButton *AnnotationTermSetMenuButton;
    
    // ontology frame
    vtkKWEntry *LocalSearchTermEntry;
    vtkKWMenuButton *SynonymsMenuButton;
    vtkKWEntry *BIRNLexEntry;
    vtkKWEntry *BIRNLexIDEntry;
    vtkKWEntry *NeuroNamesEntry;
    vtkKWEntry *NeuroNamesIDEntry;
    vtkKWEntry *UMLSCIDEntry;
    vtkKWPushButton *AddLocalTermButton;
    vtkKWPushButton *AddSynonymButton;
    vtkKWPushButton *AddBIRNLexStringButton;
    vtkKWPushButton *AddBIRNLexIDButton;
    vtkKWPushButton *AddNeuroNamesStringButton;
    vtkKWPushButton *AddNeuroNamesIDButton;
    vtkKWPushButton *AddUMLSCIDButton;    
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
    vtkKWRadioButton *SpeciesNoneButton;
    vtkKWRadioButton *SpeciesHumanButton;
    vtkKWRadioButton *SpeciesMouseButton;
    vtkKWRadioButton *SpeciesMacaqueButton;

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
    
    // results frame
    vtkKWListBoxWithScrollbars *CurrentResultsList;
    vtkKWPushButton *DeleteCurrentResultButton;
    vtkKWPushButton *DeleteAllCurrentResultsButton;
    vtkKWPushButton *SaveCurrentResultsButton;
    vtkKWPushButton *SaveCurrentSelectedResultsButton;
    vtkKWPushButton *DeselectAllCurrentResultsButton;
    vtkKWPushButton *DeselectAllAccumulatedResultsButton;
    vtkKWListBoxWithScrollbars *AccumulatedResultsList;
    vtkKWPushButton *DeleteAccumulatedResultButton;
    vtkKWPushButton *DeleteAllAccumulatedResultsButton;

    vtkKWLoadSaveButton *SaveAccumulatedResultsButton;
    vtkKWLoadSaveButton *LoadURIsButton;

    // cell frame
    vtkKWFrame *OtherFrame;
    vtkQueryAtlasUseSearchTermWidget *OtherListWidget;


    void OpenBIRNLexBrowser();
    void OpenNeuroNamesBrowser();
    void OpenUMLSBrowser();

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
    int AnnotationVisibility;
    int ModelVisibility;
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
    vtkQueryAtlasGUI ( const vtkQueryAtlasGUI& ); // Not implemented.
    void operator = ( const vtkQueryAtlasGUI& ); //Not implemented.
};


#endif
