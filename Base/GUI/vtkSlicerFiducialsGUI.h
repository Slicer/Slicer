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
class vtkKWChangeColorButton;
class vtkKWScaleWithEntry;
class vtkKWMenuButton;
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
    vtkGetObjectMacro ( ListMeasurementLabel, vtkKWLabel);
    vtkGetObjectMacro ( RenumberButton, vtkKWPushButton);
    vtkGetObjectMacro ( RenumberDialogue, vtkKWSimpleEntryDialog);
    vtkGetObjectMacro ( RenameButton, vtkKWPushButton);
    vtkGetObjectMacro ( RenameDialogue, vtkKWSimpleEntryDialog);
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
    vtkGetObjectMacro ( HideListToggle, vtkKWPushButton);
    vtkGetObjectMacro ( RemoveSelectedListButton, vtkKWPushButton );
    vtkGetObjectMacro ( ListColorButton, vtkKWChangeColorButton);
    vtkGetObjectMacro ( ListSelectedColorButton, vtkKWChangeColorButton);
    vtkGetObjectMacro ( ListSymbolScale, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListSymbolTypeMenu, vtkKWMenuButtonWithLabel);
    vtkGetObjectMacro ( ListNumberingSchemeMenu, vtkKWMenuButtonWithLabel);
    vtkGetObjectMacro ( ListTextScale, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListOpacity, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListAmbient, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListDiffuse, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListSpecular, vtkKWScaleWithEntry);
    vtkGetObjectMacro ( ListPower, vtkKWScaleWithEntry);

    vtkGetObjectMacro ( Logic, vtkSlicerFiducialsLogic);
    
    vtkGetObjectMacro ( MoveSelectedFiducialUpButton, vtkKWPushButton );
    vtkGetObjectMacro ( MoveSelectedFiducialDownButton, vtkKWPushButton );
    vtkGetObjectMacro ( Center3DViewOnSelectedFiducialButton, vtkKWPushButton );
    vtkGetObjectMacro ( AllLockMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( AllVisibilityMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( FiducialsListLockToggle, vtkKWPushButton );
    vtkGetObjectMacro ( HideOrExposeAllFiducialListsMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( ListVisibilityMenuButton, vtkKWMenuButton );
        
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
    virtual void BuildGUI ( void );
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

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
    // Getting the mrml fiducial list node id
    vtkGetStringMacro(FiducialListNodeID);

    // Description:
    // Set the fid list id, and update the widgets
    void SetFiducialListNodeID(char *id);
    
    // Description:
    // Which fiducial list node are we displaying in this gui?
    vtkSlicerNodeSelectorWidget* FiducialListSelectorWidget;

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
    // measurement labels
    void UpdateMeasurementLabels();

    // Description:
    // when right click on a row in the fid list, call this to jump the slices
    // to that point in RAS.
    void JumpSlicesCallback(int row, int col, int x, int y);

    
    // Description:
    // Modifies the lock state on all fiducial lists.
    void ModifyAllLock(int lockState);

    // Description:
    // Modifies the lock state on selected fiducial list.
    void ModifyListLock( int lockState);
    // Description:
    // Modifies the visibility state on all fiducials in all lists
    // but preserves list exposure setting
    void ModifyAllFiducialVisibility(int visibilityState);

    // Description:
     // Modifies the visibility state on all fiducials in the
     // selected list.
     void ModifyFiducialsInListVisibility(int visibilityState);
 
     // Description:
     // Modifies the exposure state on all fiducial  lists but
     // preserves fiducial visibility setting.
     void ModifyAllListExposure(int visibilityState);

     // Modifies the exposure state on selected fiducial list
     // but preserves fiducial visibility setting.
     void ModifyListExposure(int visibilityState);

     // Description
     // sets node state when visibility or lock cells
     // in the multicolumn list for individual  fiducials
     // are clicked on (selected)
     // for toggling. Modifies Nth Fiducial.
     void VisibilityOrLockToggleCallback ( );
     
     // Description:
     // modifies the multicolumn list if a fiducial point's
     // visibility changes
     void ModifyIndividualFiducialsVisibilityGUI();
     // Description:
     // modifies the multicolumn list if a fiducial list's lock
     // state changes.
     void ModifyIndividualFiducialsLockGUI( );
     // Description:
     // modifies the Selected List toggle button to
     // show the selected list's lock state.
     void ModifySelectedListLockGUI();
     // Description:
     // modifies the Selected List's hide/expose button to
     // show the selected list's exposure state.
     void ModifySelectedListExposureGUI();

protected:
    vtkSlicerFiducialsGUI ( );
    virtual ~vtkSlicerFiducialsGUI ( );
    
    // Module logic and mrml pointers
    vtkSlicerFiducialsLogic *Logic;

    // Description:
    // The ID of the fiducial list node that is currently displayed in the GUI
    char *FiducialListNodeID;

    // Widgets for the Fiducials module

    // Description:
    // Update this label with text about distance between selected fiducials
    vtkKWLabel *MeasurementLabel;

    // Description:
    // Update this label with text about the distance between selected
    // fiducials for the whole list
    vtkKWLabel *ListMeasurementLabel;

    // Description:
    // Renumber the fiducials in this list, starting from 0
    vtkKWPushButton *RenumberButton;

    // Description:
    // pop up dialogue to get the new fiducial starting number
    vtkKWSimpleEntryDialog *RenumberDialogue;

    // Description:
    // Rename the fiducials in this list, preserving ending numbers
    vtkKWPushButton *RenameButton;

    // Description:
    // pop up dialogue to get the new fid name
    vtkKWSimpleEntryDialog *RenameDialogue;

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
    // centers the 3d view on the selected fiducial
    vtkKWPushButton *Center3DViewOnSelectedFiducialButton;

    // Description:
    // list visibility, overrides individual point visibility
    vtkKWPushButton *HideListToggle;

    // Description:
    // removes selected fiducial list and its fiducials.
    vtkKWPushButton *RemoveSelectedListButton;

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
    // list actor ambient
    vtkKWScaleWithEntry *ListAmbient;

    // Description:
    // list actor diffuse
    vtkKWScaleWithEntry *ListDiffuse;
    
    // Description:
    // list actor specular
    vtkKWScaleWithEntry *ListSpecular;

    // Description:
    // list actor power
    vtkKWScaleWithEntry *ListPower;
    
    // Description:
    // display the points on the list
    vtkKWMultiColumnListWithScrollbars *MultiColumnList;

    // Description:
    // menu button whose menu exposes options for
    // locking or unlocking all fiducials in all fiducial lists.
    vtkKWMenuButton *AllLockMenuButton;

    // Description:
    // menu button whose menu exposes options for
    // setting visibility of  all fiducials in all fiducial lists.
    vtkKWMenuButton *AllVisibilityMenuButton;

    // Description:
    // push button to toggle
    // locking or unlocking all fiducials in a fiducial list.
    vtkKWPushButton *FiducialsListLockToggle;

    // Description:
    // menu button whose menu exposes options for
    // setting visibility of all fiducials in a fiducial list.
    vtkKWMenuButton *ListVisibilityMenuButton;

    // Description:
    // choose the numbering scheme for the next fiducial added in the list
    vtkKWMenuButtonWithLabel *ListNumberingSchemeMenu;

    // menu button whose menu exposes options
    // for hiding or exposing all fiducial lists.
    vtkKWMenuButton *HideOrExposeAllFiducialListsMenuButton;
  
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
        SelectedColumn = 0,
        VisibilityColumn = 1,
        NameColumn = 2,
        XColumn = 3,
        YColumn = 4,
        ZColumn = 5,
        OrWColumn = 6,
        OrXColumn = 7,
        OrYColumn = 8,
        OrZColumn = 9,
        LockColumn = 10,
        NumberOfColumns = 11,
    };
    //ETX

private:
    vtkSlicerFiducialsGUI ( const vtkSlicerFiducialsGUI& ); // Not implemented.
    void operator = ( const vtkSlicerFiducialsGUI& ); //Not implemented.
};


#endif
