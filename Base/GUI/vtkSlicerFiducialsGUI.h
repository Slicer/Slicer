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
#include "vtkKWPushButtonWithLabel.h"
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
class vtkKWPushButtonWithLabel;
class vtkKWMessage;
class vtkSlicerVisibilityIcons;
class vtkKWChangeColorButton;
class vtkKWScaleWithEntry;
class vtkKWMenuButtonWithLabel;
class vtkKWLabel;
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
    vtkGetObjectMacro ( MeasurementLabel, vtkKWLabel);
    vtkGetObjectMacro ( AddFiducialButton, vtkKWPushButton);
    vtkGetObjectMacro ( RemoveFiducialButton, vtkKWPushButton);
    vtkGetObjectMacro ( RemoveAllFiducialsButton, vtkKWPushButton);
    vtkGetObjectMacro ( LockAllFiducialsButton, vtkKWPushButtonWithLabel);
    vtkGetObjectMacro ( UnlockAllFiducialsButton, vtkKWPushButtonWithLabel);
    vtkGetObjectMacro ( RemoveFiducialsInListButton, vtkKWPushButton);
    vtkGetObjectMacro ( SelectAllFiducialsButton, vtkKWPushButton);
    vtkGetObjectMacro ( SelectAllFiducialsInListButton, vtkKWPushButton);
    vtkGetObjectMacro ( DeselectAllFiducialsButton, vtkKWPushButton);
    vtkGetObjectMacro ( DeselectAllFiducialsInListButton, vtkKWPushButton);
    vtkGetObjectMacro ( VisibilityToggle, vtkKWPushButtonWithLabel);
    vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons);
    vtkGetObjectMacro ( ListColorButton, vtkKWChangeColorButton);
    vtkGetObjectMacro ( ListSelectedColorButton, vtkKWChangeColorButton);
    vtkGetObjectMacro ( ListSymbolScale, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListSymbolTypeMenu, vtkKWMenuButtonWithLabel);
    vtkGetObjectMacro ( ListTextScale, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListOpacity, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( Logic, vtkSlicerFiducialsLogic);
    vtkGetObjectMacro ( LockToggle, vtkKWPushButtonWithLabel );
    vtkGetObjectMacro ( MoveSelectedFiducialUpButton, vtkKWPushButton );
    vtkGetObjectMacro ( MoveSelectedFiducialDownButton, vtkKWPushButton );
    
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
    // from a new list, use this call. It calls SetGUIDisplayFrameFromList.
    virtual void SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode);
    // Description:
    // If just the display frame elements need to be updated from the list
    // (got a display modified event), call this method. The lock toggle is
    // updated as changing the locked state on the list throws a displayed
    // modified event.
    virtual void SetGUIDisplayFrameFromList(vtkMRMLFiducialListNode * activeFiducialListNode);
    
    // Description:
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( vtkMRMLNode *node );
    virtual void Enter ( ) { this->Enter(NULL); };
    virtual void Exit ( );

    // Description:
    // Set a value in the multi column list box at row,col to str
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

    // Description:
    // Called when the selected state of some fids have changed, to update the
    // measurement label
    void UpdateMeasurementLabel();

    // Description:
    // when right click on a row in the fid list, call this to jump the slices
    // to that point in RAS.
    void JumpSlicesCallback(int row, int col, int x, int y);
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

    // Description:
    // Update this label with text about distance between selected fiducials
    vtkKWLabel *MeasurementLabel;
    
    // Description:
    // add a point
    vtkKWPushButton *AddFiducialButton;
    // Description:
    // remove the last selected (multi column list definition of selected)
    // point
    vtkKWPushButton *RemoveFiducialButton;
    // Description:
    // remove all the fiducial points on this list
    vtkKWPushButton *RemoveFiducialsInListButton;
    // Description:
    // remove all fiducials on all lists
    vtkKWPushButton *RemoveAllFiducialsButton;
    // Description:
    // lock all fiducial lists
    vtkKWPushButtonWithLabel *LockAllFiducialsButton;
    // Description:
    // unlock all fiducial lists
    vtkKWPushButtonWithLabel *UnlockAllFiducialsButton;
    // Description:
    // select all fiducial points on this list
    vtkKWPushButton *SelectAllFiducialsInListButton;
    //Description:
    // selects all fiducials points on all lists.
    vtkKWPushButton *SelectAllFiducialsButton;
    // Description:
    // deselect all fiducial points on this list
    vtkKWPushButton *DeselectAllFiducialsInListButton;
    // Description:
    // deselects all fiducial point in all lists.
    vtkKWPushButton *DeselectAllFiducialsButton;

    // Description:
    // moves the selected fiducial up/down in the list.
    vtkKWPushButton *MoveSelectedFiducialUpButton;
    vtkKWPushButton *MoveSelectedFiducialDownButton;

    // Description:
    // list visibility, overrides individual point visibility
    vtkKWPushButtonWithLabel *VisibilityToggle;
    vtkSlicerVisibilityIcons *VisibilityIcons;

    // Description:
    // list colour
    vtkKWChangeColorButton *ListColorButton;

    // Description:
    // list selected fiducial colour
    vtkKWChangeColorButton *ListSelectedColorButton;

    // Description:
    // symbol scale
    vtkKWScaleWithEntry *ListSymbolScale;
    // Description:
    // symbol type
    vtkKWMenuButtonWithLabel *ListSymbolTypeMenu;
    // Description:
    // text scale
    vtkKWScaleWithEntry *ListTextScale;

    // Description:
    // opacity
    vtkKWScaleWithEntry *ListOpacity;

    // Description:
    // list is locked or not (won't move when mouse over it)
    vtkKWPushButtonWithLabel *LockToggle;
    
    // Description:
    // display the points on the list
    vtkKWMultiColumnListWithScrollbars *MultiColumnList;

    // Description:
    // contributing logo widgets
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
        VisibilityColumn = 2,
        XColumn = 3,
        YColumn = 4,
        ZColumn = 5,
        OrWColumn = 6,
        OrXColumn = 7,
        OrYColumn = 8,
        OrZColumn = 9,
        NumberOfColumns = 10,
    };
    //ETX

private:
    vtkSlicerFiducialsGUI ( const vtkSlicerFiducialsGUI& ); // Not implemented.
    void operator = ( const vtkSlicerFiducialsGUI& ); //Not implemented.
};


#endif
