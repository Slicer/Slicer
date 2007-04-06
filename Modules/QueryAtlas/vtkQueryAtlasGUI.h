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

class vtkKWPushButton;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWMenuButton;
class vtkKWMenuButtonWithLabel;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWCheckButton;
class vtkKWEntry;
class vtkKWListBox;
class vtkKWListBoxWithScrollbars;

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

    // Load scene frame and widgets
    vtkGetObjectMacro ( LoadSceneButton, vtkKWPushButton );

    // Querybuilder frame top widgets
    vtkGetObjectMacro (GeneButton, vtkKWPushButton );
    vtkGetObjectMacro (MiscButton, vtkKWPushButton );
    vtkGetObjectMacro (CellButton, vtkKWPushButton );
    vtkGetObjectMacro (StructureButton, vtkKWPushButton );
    vtkGetObjectMacro (PopulationButton, vtkKWPushButton );
    vtkGetObjectMacro (SpeciesButton, vtkKWPushButton );
    // Querybuilder frame containing alternate sets of widgets
    vtkGetObjectMacro (SwitchQueryFrame, vtkKWFrame);
    
    // species panel
    vtkGetObjectMacro (SpeciesFrame, vtkKWFrame);
    vtkGetObjectMacro (SpeciesLabel, vtkKWLabel);
    vtkGetObjectMacro (SpeciesHumanButton, vtkKWCheckButton );
    vtkGetObjectMacro (SpeciesMouseButton, vtkKWCheckButton);
    vtkGetObjectMacro (SpeciesMacaqueButton, vtkKWCheckButton );
    
    // population panel
    vtkGetObjectMacro (PopulationFrame, vtkKWFrame );
    vtkGetObjectMacro (DiagnosisMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (HandednessMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (GenderMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (AgeMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (DiagnosticsMenuButton, vtkKWMenuButtonWithLabel );
    
    // structure panel
    vtkGetObjectMacro (StructureFrame, vtkKWFrame );
    vtkGetObjectMacro ( StructureMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro ( StructureClearAllButton, vtkKWPushButton );
    vtkGetObjectMacro ( StructureUseAllButton, vtkKWPushButton );
    vtkGetObjectMacro ( StructureUseNoneButton, vtkKWPushButton );    
    vtkGetObjectMacro ( StructureAddTermButton, vtkKWPushButton );    
    vtkGetObjectMacro ( StructureClearTermButton, vtkKWPushButton );    
    vtkGetObjectMacro ( StructureMultiColumnList, vtkKWMultiColumnListWithScrollbars );

    // cell panel
    vtkGetObjectMacro (CellFrame, vtkKWFrame);
    // protein panel
    vtkGetObjectMacro (MiscFrame, vtkKWFrame );
    // genes panel
    vtkGetObjectMacro (GeneFrame, vtkKWFrame );

    // querymaker panel
    vtkGetObjectMacro ( DatabasesMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( SearchButton, vtkKWPushButton );
    
    //hierarchies panel
    vtkGetObjectMacro ( SPLHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( BIRNLexHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( HierarchySearchTermEntry, vtkKWEntry );
    vtkGetObjectMacro ( HierarchySearchButton, vtkKWPushButton );

    vtkGetObjectMacro ( CurrentResultsList, vtkKWListBoxWithScrollbars );
    vtkGetObjectMacro ( DeleteCurrentResultButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAllCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SaveCurrentResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( PastResultsList, vtkKWListBoxWithScrollbars );    
    vtkGetObjectMacro ( DeletePastResultButton, vtkKWPushButton );
    vtkGetObjectMacro ( DeleteAllPastResultsButton, vtkKWPushButton );
    vtkGetObjectMacro ( SavePastResultsButton, vtkKWPushButton );
    
    vtkGetMacro ( NumberOfColumns, int );
    
    void SetModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the QueryAtlas module GUI
    virtual void BuildGUI ( ) ;
    // Description:
    // Helper methods for building the complicated GUI
    virtual void UnpackQueryBuilderContextFrames ( );
    virtual void PackQueryBuilderContextFrame ( vtkKWFrame *f );
    virtual void BuildQueryBuilderContextButtons ( vtkKWFrame *parent );
    virtual void ColorCodeContextButtons ( vtkKWPushButton *b );
    virtual void BuildQueryBuilderContextFrames ( vtkKWFrame *parent );
    virtual void BuildSpeciesFrame();
    virtual void BuildPopulationFrame();
    virtual void BuildStructureFrame ( );
    virtual void BuildCellFrame();
    virtual void BuildMiscFrame();
    virtual void BuildGeneFrame();
    virtual void BuildDiagnosisMenu( vtkKWMenu *m );
    virtual void BuildDatabasesMenu( vtkKWMenu *m );
                                    
    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

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
    // Methods to add and delete search terms.
    virtual void AddNewSearchTerm ( const char *context );
    virtual void DeleteSelectedSearchTerms ( const char *context );
    virtual void SelectAllSearchTerms ( const char *context );
    virtual void DeselectAllSearchTerms ( const char *context );
    virtual void DeleteAllSearchTerms ( const char *context);

    

 protected:
    vtkQueryAtlasGUI ( );
    virtual ~vtkQueryAtlasGUI ( );

    // Module logic and mrml pointers
    vtkQueryAtlasLogic *Logic;

    // load scene
    vtkKWPushButton *LoadSceneButton;

    // hierarchies frame
    vtkKWPushButton *SPLHierarchyButton;
    vtkKWPushButton *BIRNLexHierarchyButton;
    vtkKWEntry *HierarchySearchTermEntry;
    vtkKWPushButton *HierarchySearchButton;
    int NumberOfColumns;
    
    // querybuilder
    vtkKWPushButton *GeneButton;
    vtkKWPushButton *MiscButton;
    vtkKWPushButton *CellButton;
    vtkKWPushButton *StructureButton;
    vtkKWPushButton *PopulationButton;
    vtkKWPushButton *SpeciesButton;
    vtkKWFrame *SwitchQueryFrame;
    
    // species frame
    vtkKWFrame *SpeciesFrame;
    vtkKWLabel *SpeciesLabel;
    vtkKWCheckButton *SpeciesHumanButton;
    vtkKWCheckButton *SpeciesMouseButton;
    vtkKWCheckButton *SpeciesMacaqueButton;

    // population frame
    vtkKWFrame *PopulationFrame;
    vtkKWMenuButtonWithLabel *DiagnosisMenuButton;
    vtkKWMenuButtonWithLabel *GenderMenuButton;
    vtkKWMenuButtonWithLabel *HandednessMenuButton;
    vtkKWMenuButtonWithLabel *AgeMenuButton;
    vtkKWMenuButtonWithLabel *DiagnosticsMenuButton;

    // structure frame
    vtkKWFrame *StructureFrame;
    vtkKWMenuButtonWithLabel *StructureMenuButton;
    vtkKWMultiColumnListWithScrollbars *StructureMultiColumnList;
    vtkKWPushButton *StructureClearAllButton;
    vtkKWPushButton *StructureUseAllButton;
    vtkKWPushButton *StructureUseNoneButton;
    vtkKWPushButton *StructureAddTermButton;
    vtkKWPushButton *StructureClearTermButton;

    // querymaker
    vtkKWPushButton *SearchButton;
    vtkKWMenuButton *DatabasesMenuButton;

    // results frame
    vtkKWListBoxWithScrollbars *CurrentResultsList;
    vtkKWPushButton *DeleteCurrentResultButton;
    vtkKWPushButton *DeleteAllCurrentResultsButton;
    vtkKWPushButton *SaveCurrentResultsButton;
    vtkKWListBoxWithScrollbars *PastResultsList;
    vtkKWPushButton *DeletePastResultButton;
    vtkKWPushButton *DeleteAllPastResultsButton;
    vtkKWPushButton *SavePastResultsButton;

    // cell frame
    vtkKWFrame *CellFrame;
    // protein frame
    vtkKWFrame *MiscFrame;
    // gene grame
    vtkKWFrame *GeneFrame;

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

private:
    vtkQueryAtlasGUI ( const vtkQueryAtlasGUI& ); // Not implemented.
    void operator = ( const vtkQueryAtlasGUI& ); //Not implemented.
};


#endif
