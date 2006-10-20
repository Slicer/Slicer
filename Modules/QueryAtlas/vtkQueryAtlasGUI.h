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
    vtkGetObjectMacro ( LoadSceneButton, vtkKWPushButton );
    vtkGetObjectMacro ( SearchButton, vtkKWPushButton );
    vtkGetObjectMacro ( ClearButton, vtkKWPushButton );
    vtkGetObjectMacro ( SelectAllButton, vtkKWPushButton );
    vtkGetObjectMacro ( SelectNoneButton, vtkKWPushButton );    
    vtkGetObjectMacro ( AddTermButton, vtkKWPushButton );    
    vtkGetObjectMacro ( DeleteTermButton, vtkKWPushButton );    
    vtkGetObjectMacro ( SearchTermMultiColumnList, vtkKWMultiColumnListWithScrollbars );
    vtkGetObjectMacro ( SearchTargetMenuButton, vtkKWMenuButton );
    vtkGetMacro ( NumberOfColumns, int );
    
    void SetModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the QueryAtlas module GUI
    virtual void BuildGUI ( ) ;

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

 protected:
    vtkQueryAtlasGUI ( );
    virtual ~vtkQueryAtlasGUI ( );

    // Module logic and mrml pointers
    vtkQueryAtlasLogic *Logic;

    // widgets
    vtkKWPushButton *LoadSceneButton;
    vtkKWPushButton *SearchButton;
    vtkKWPushButton *ClearButton;
    vtkKWPushButton *SelectAllButton;
    vtkKWPushButton *SelectNoneButton;
    vtkKWPushButton *AddTermButton;
    vtkKWPushButton *DeleteTermButton;
    vtkKWMultiColumnListWithScrollbars *SearchTermMultiColumnList;
    vtkKWMenuButton *SearchTargetMenuButton;
    int NumberOfColumns;

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
