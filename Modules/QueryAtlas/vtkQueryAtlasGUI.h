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

    // Load scene
    vtkGetObjectMacro ( LoadSceneButton, vtkKWPushButton );

    // querybuilder
    vtkGetObjectMacro (GeneButton, vtkKWPushButton );
    vtkGetObjectMacro (ProteinButton, vtkKWPushButton );
    vtkGetObjectMacro (CellButton, vtkKWPushButton );
    vtkGetObjectMacro (StructureButton, vtkKWPushButton );
    vtkGetObjectMacro (PopulationButton, vtkKWPushButton );
    vtkGetObjectMacro (SpeciesButton, vtkKWPushButton );
    vtkGetObjectMacro (SwitchQueryFrame, vtkKWFrame);
    
    // species 
    vtkGetObjectMacro (SpeciesFrame, vtkKWFrame);
    vtkGetObjectMacro (SpeciesMenuButton, vtkKWMenuButton );
    
    // population
    vtkGetObjectMacro (PopulationFrame, vtkKWFrame );
    vtkGetObjectMacro (DiagnosisMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (HandednessMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (GenderMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (AgeMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro (ClinicalTestMenuButton, vtkKWMenuButtonWithLabel );
    
    // structure
    vtkGetObjectMacro (StructureFrame, vtkKWFrame );
    vtkGetObjectMacro ( StructureMenuButton, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro ( ClearButton, vtkKWPushButton );
    vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton );
    vtkGetObjectMacro ( SelectNoneButton, vtkKWPushButton );    
    vtkGetObjectMacro ( AddTermButton, vtkKWPushButton );    
    vtkGetObjectMacro ( DeleteTermButton, vtkKWPushButton );    
    vtkGetObjectMacro ( SearchTermMultiColumnList, vtkKWMultiColumnListWithScrollbars );

    // cell 
    vtkGetObjectMacro (CellFrame, vtkKWFrame);
    // protein
    vtkGetObjectMacro (ProteinFrame, vtkKWFrame );
    // genes
    vtkGetObjectMacro (GeneFrame, vtkKWFrame );

    // querymaker
    vtkGetObjectMacro ( DatabasesMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( SearchButton, vtkKWPushButton );
    
    //hierarchies
    vtkGetObjectMacro ( SPLHierarchyButton, vtkKWPushButton );
    vtkGetObjectMacro ( BIRNLexHierarchyButton, vtkKWPushButton );


    
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
    virtual void BuildQueryBuilderContextButtons ( vtkKWFrame *parent );
    virtual void BuildQueryBuilderContextFrames ( vtkKWFrame *parent );
    virtual void BuildSpeciesFrame();
    virtual void BuildPopulationFrame();
    virtual void BuildStructureFrame ( );
    virtual void BuildCellFrame();
    virtual void BuildProteinFrame();
    virtual void BuildGeneFrame();
    
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
    virtual void AddNewSearchTerm ( );
    virtual void DeleteSelectedSearchTerms ( );
    virtual void SelectAllSearchTerms ( );
    virtual void DeselectAllSearchTerms ( );
    virtual void DeleteAllSearchTerms ( );

    

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
    int NumberOfColumns;
    
    // querybuilder
    vtkKWPushButton *GeneButton;
    vtkKWPushButton *ProteinButton;
    vtkKWPushButton *CellButton;
    vtkKWPushButton *StructureButton;
    vtkKWPushButton *PopulationButton;
    vtkKWPushButton *SpeciesButton;
    vtkKWFrame *SwitchQueryFrame;
    
    // species frame
    vtkKWFrame *SpeciesFrame;
    vtkKWMenuButton *SpeciesMenuButton;

    // population frame
    vtkKWFrame *PopulationFrame;
    vtkKWMenuButtonWithLabel *DiagnosisMenuButton;
    vtkKWMenuButtonWithLabel *GenderMenuButton;
    vtkKWMenuButtonWithLabel *HandednessMenuButton;
    vtkKWMenuButtonWithLabel *AgeMenuButton;
    vtkKWMenuButtonWithLabel *ClinicalTestMenuButton;

    // structure frame
    vtkKWFrame *StructureFrame;
    vtkKWMenuButtonWithLabel *StructureMenuButton;
    vtkKWMultiColumnListWithScrollbars *SearchTermMultiColumnList;
    vtkKWPushButton *ClearButton;
    vtkKWPushButton *SelectAllButton;
    vtkKWPushButton *SelectNoneButton;
    vtkKWPushButton *AddTermButton;
    vtkKWPushButton *DeleteTermButton;

    // querymaker
    vtkKWPushButton *SearchButton;
    vtkKWMenuButton *DatabasesMenuButton;

    // cell frame
    vtkKWFrame *CellFrame;
    // protein frame
    vtkKWFrame *ProteinFrame;
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
