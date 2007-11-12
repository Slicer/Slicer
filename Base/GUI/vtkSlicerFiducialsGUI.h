// .NAME vtkSlicerFiducialsGUI 
// .SECTION Description
// Main Fiducials GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerFiducialsGUI_h
#define __vtkSlicerFiducialsGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerFiducialsLogic.h"
#include "vtkMRMLFiducial.h"
#include "vtkMRMLFiducialListNode.h"

#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"
#include "vtkSlicerVisibilityIcons.h"
#include "vtkKWChangeColorButton.h"

#include "vtkKWScaleWithEntry.h"
#include "vtkKWScale.h"
#include "vtkKWLabel.h"

// Description:
// This class implements Slicer's Fiducials GUI
//
//class vtkSlicerFiducialsDisplayWidget;
class vtkKWMultiColumnListWithScrollbars;
class vtkKWPushButton;
class vtkKWMessage;
class vtkSlicerVisibilityIcons;
class vtkKWChangeColorButton;
class vtkKWScaleWithEntry;
class vtkKWMenuButtonWithLabel;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerFiducialsGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerFiducialsGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerFiducialsGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( FiducialListSelectorWidget, vtkSlicerNodeSelectorWidget);
    vtkGetObjectMacro ( AddFiducialButton, vtkKWPushButton);
    vtkGetObjectMacro ( RemoveFiducialButton, vtkKWPushButton);
    vtkGetObjectMacro ( RemoveFiducialsButton, vtkKWPushButton);
    vtkGetObjectMacro ( SelectAllFiducialsButton, vtkKWPushButton);
    vtkGetObjectMacro ( DeselectAllFiducialsButton, vtkKWPushButton);
    vtkGetObjectMacro ( VisibilityToggle, vtkKWPushButton);
    vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons);
    vtkGetObjectMacro ( ListColorButton, vtkKWChangeColorButton);
    vtkGetObjectMacro ( ListSelectedColorButton, vtkKWChangeColorButton);
    vtkGetObjectMacro ( ListSymbolScale, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListSymbolTypeMenu, vtkKWMenuButtonWithLabel);
    vtkGetObjectMacro ( ListTextScale, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListOpacity, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( Logic, vtkSlicerFiducialsLogic);
    
    // Description:
    // API for setting FiducialListNode, Logic and
    // for both setting and observing them.
    
    /*
    void SetMRMLNode ( vtkMRMLFiducialListNode *node )
        { this->SetMRML ( vtkObjectPointer( &this->FiducialListNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLFiducialListNode *node )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->FiducialListNode), node ); }
    */
    
    void SetModuleLogic ( vtkSlicerFiducialsLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkSlicerFiducialsLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the Fiducials module GUI
    virtual void BuildGUI ( ) ;

    // Description:
    // This method releases references and key-bindings,
    // and optionally removes observers.
    virtual void TearDownGUI ( );

    // Description:
    // Methods for adding module-specific key bindings and
    // removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );

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
    // Once know that the GUI has to be cleared and updated to show elements
    // from a new list, use this call
    virtual void SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode);
    
    // Description:
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    virtual void UpdateElement(int row, int col, char * str);

    // Description:
    // Getting and setting the mrml fiducail list node id
    vtkGetStringMacro(FiducialListNodeID);
    //vtkSetStringMacro(FiducialListNodeID);
    void SetFiducialListNodeID(char *id);
    
    // Description:
    // Which fiducial list node are we displaying in this gui?
    vtkSlicerNodeSelectorWidget* FiducialListSelectorWidget;
    
    // Description:
    // Set the selected node, the fid list id, and update the widgets
    void SetFiducialListNode(vtkMRMLFiducialListNode *fiducialListNode);

    //BTX
    // Description:
    // FiducialListIDModifiedEvent is generated when the FiducialListNodeID is
    // changed
    enum
    {
        FiducialListIDModifiedEvent = 20000,
    };
    //ETX

    // Description:
    // Update the gui from the currently selected list, called on Enter
    void UpdateGUI();
    
 protected:
    vtkSlicerFiducialsGUI ( );
    virtual ~vtkSlicerFiducialsGUI ( );

    // Module logic and mrml pointers
    vtkSlicerFiducialsLogic *Logic;


    // Description:
    // The ID of the fiducial list node that is currently displayed in the GUI
    // TODO: probably redundant
    char *FiducialListNodeID;

    // Description:
    // The the fiducial list node that is currently displayed in the GUI
    vtkMRMLFiducialListNode *FiducialListNode;
    
    // Widgets for the Fiducials module
    // add a point
    vtkKWPushButton *AddFiducialButton;
    // remove the last selected (multi column list definition of selected)
    // point
    vtkKWPushButton *RemoveFiducialButton;
    // remove all the fiducial points on this list
    vtkKWPushButton *RemoveFiducialsButton;
    // select all fiducial points on this list
    vtkKWPushButton *SelectAllFiducialsButton;
    // deselect all fiducial points on this list
    vtkKWPushButton *DeselectAllFiducialsButton;
    
    // list visibility
    vtkKWPushButton *VisibilityToggle;
    vtkSlicerVisibilityIcons *VisibilityIcons;

    // list colour
    vtkKWChangeColorButton *ListColorButton;

    // list selected fiducial colour
    vtkKWChangeColorButton *ListSelectedColorButton;
    
    // symbol scale
    vtkKWScaleWithEntry *ListSymbolScale;
    // symbol type
    vtkKWMenuButtonWithLabel *ListSymbolTypeMenu;
    // text scale
    vtkKWScaleWithEntry *ListTextScale;

    // opacity
    vtkKWScaleWithEntry *ListOpacity;
    
    // display the points on the list
    vtkKWMultiColumnListWithScrollbars *MultiColumnList;
    // the columns that hold the
    // name, x, y, z, orientation x y z w, selected
    int NumberOfColumns;
    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;

    //BTX
    // Description:
    // The column orders in the list box
    enum
    {
        NameColumn = 0,
        SelectedColumn = 1,
        XColumn = 2,
        YColumn = 3,
        ZColumn = 4,
        OrWColumn = 5,
        OrXColumn = 6,
        OrYColumn = 7,
        OrZColumn = 8,
    };
    //ETX

private:
    vtkSlicerFiducialsGUI ( const vtkSlicerFiducialsGUI& ); // Not implemented.
    void operator = ( const vtkSlicerFiducialsGUI& ); //Not implemented.
};


#endif
