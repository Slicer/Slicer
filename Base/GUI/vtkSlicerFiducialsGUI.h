///  vtkSlicerFiducialsGUI 
/// 
/// Main Fiducials GUI and mediator methods for slicer3. 


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

/// Description:
/// This class implements Slicer's Fiducials GUI
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
    /// 
    /// Usual vtk class functions
    static vtkSlicerFiducialsGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerFiducialsGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    /// 
    /// Get methods on class members ( no Set methods required. )
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
        
    /// 
    /// API for setting FiducialListNode, Logic and
    /// for both setting and observing them.
    
    /*
    void SetMRMLNode ( vtkMRMLFiducialListNode *node )
        { this->SetMRML ( vtkObjectPointer( &this->FiducialListNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLFiducialListNode *node )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->FiducialListNode), node ); }
    */
    
    void SetModuleLogic ( vtkSlicerFiducialsLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    //BTX
    using vtkSlicerModuleGUI::SetModuleLogic;
    //ETX
    void SetAndObserveModuleLogic ( vtkSlicerFiducialsLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    /// 
    /// This method builds the Fiducials module GUI
    virtual void BuildGUI ( void );
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

    /// 
    /// This method releases references and key-bindings,
    /// and optionally removes observers.
    virtual void TearDownGUI ( );

    /// 
    /// Methods for adding module-specific key bindings and
    /// removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );

    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Add/Remove observers on the mrml scene
    void AddMRMLObservers();
    void RemoveMRMLObservers();

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    /// 
    /// Once know that the GUI has to be cleared and updated to show elements
    /// from a new list, use this call. It calls SetGUIDisplayFrameFromList.
    virtual void SetGUIFromList(vtkMRMLFiducialListNode * activeFiducialListNode);
    ///
    /// Update a single row of the table, called when a new fid is added, and
    /// in a loop from SetGUIFromList. If newRowFlag is true, just set it,
    /// otherwise compare against the old value. Returns if row is out of
    /// bounds or fidList is null, or n is out of bounds. Need to use the
    /// fidList GetNthFiducialX calls rather than passing in a fid since
    /// GetNthFiducial is protected on the fiducial list node. If
    /// updateMeasurementsFlag is true, call UpdateMeasurements (should be
    /// false when called in a loop). 
    virtual void UpdateRowFromNthFiducial(int row, vtkMRMLFiducialListNode *fidList, int n, bool newRowFlag, bool updateMeasurementsFlag);
    
    /// 
    /// If just the display frame elements need to be updated from the list
    /// (got a display modified event), call this method. The lock toggle is
    /// updated as changing the locked state on the list throws a displayed
    /// modified event.
    virtual void SetGUIDisplayFrameFromList(vtkMRMLFiducialListNode * activeFiducialListNode);
    
    /// 
    /// Methods describe behavior at module enter and exit.
    virtual void Enter ( vtkMRMLNode *node );
    virtual void Enter ( ) { this->Enter(NULL); };
    virtual void Exit ( );

    /// 
    /// Set a value in the multi column list box at row,col to str
    virtual void UpdateElement(int row, int col, char * str);

    /// 
    /// Getting the mrml fiducial list node id
    vtkGetStringMacro(FiducialListNodeID);

    /// 
    /// Set the fid list id, and update the widgets
    void SetFiducialListNodeID(char *id);

    //BTX
    /// 
    /// Which fiducial list node are we displaying in this gui?
    vtkSlicerNodeSelectorWidget* FiducialListSelectorWidget;
    
    /// 
    /// FiducialListIDModifiedEvent is generated when the FiducialListNodeID is
    /// changed
    enum
    {
        FiducialListIDModifiedEvent = 20000,
    };
    //ETX

    /// 
    /// Update the gui from the currently selected list, called on Enter
    void UpdateGUI();

    /// 
    /// Called when the selected state of some fids have changed, to update the
    /// measurement labels
    void UpdateMeasurementLabels();

    /// 
    /// when right click on a row in the fid list, call this to jump the slices
    /// to that point in RAS.
    void JumpSlicesCallback(int row, int col, int x, int y);

    
    /// 
    /// Modifies the lock state on all fiducial lists.
    void ModifyAllLock(int lockState);

    /// 
    /// Modifies the lock state on selected fiducial list.
    void ModifyListLock( int lockState);
    /// 
    /// Modifies the visibility state on all fiducials in all lists
    /// but preserves list exposure setting
    void ModifyAllFiducialVisibility(int visibilityState);

    /// 
     /// Modifies the visibility state on all fiducials in the
     /// selected list.
     void ModifyFiducialsInListVisibility(int visibilityState);
 
     /// 
     /// Modifies the exposure state on all fiducial  lists but
     /// preserves fiducial visibility setting.
     void ModifyAllListExposure(int visibilityState);

     /// Modifies the exposure state on selected fiducial list
     /// but preserves fiducial visibility setting.
     void ModifyListExposure(int visibilityState);

     /// Description
     /// sets node state when visibility or lock cells
     /// in the multicolumn list for individual  fiducials
     /// are clicked on (selected)
     /// for toggling. Modifies Nth Fiducial.
     void VisibilityOrLockToggleCallback ( );
     
     /// 
     /// modifies the multicolumn list if a fiducial point's
     /// visibility changes
     void ModifyIndividualFiducialsVisibilityGUI();
     /// 
     /// modifies the multicolumn list if a fiducial list's lock
     /// state changes.
     void ModifyIndividualFiducialsLockGUI( );
     /// 
     /// modifies the Selected List toggle button to
     /// show the selected list's lock state.
     void ModifySelectedListLockGUI();
     /// 
     /// modifies the Selected List's hide/expose button to
     /// show the selected list's exposure state.
     void ModifySelectedListExposureGUI();
  
protected:
    vtkSlicerFiducialsGUI ( );
    virtual ~vtkSlicerFiducialsGUI ( );

    /// Module logic and mrml pointers
    vtkSlicerFiducialsLogic *Logic;

    /// 
    /// The ID of the fiducial list node that is currently displayed in the GUI
    char *FiducialListNodeID;

    /// Widgets for the Fiducials module

    /// 
    /// Update this label with text about distance between selected fiducials
    vtkKWLabel *MeasurementLabel;

    /// 
    /// Update this label with text about the distance between selected
    /// fiducials for the whole list
    vtkKWLabel *ListMeasurementLabel;

    /// 
    /// Renumber the fiducials in this list, starting from 0
    vtkKWPushButton *RenumberButton;

    /// 
    /// pop up dialogue to get the new fiducial starting number
    vtkKWSimpleEntryDialog *RenumberDialogue;

    /// 
    /// Rename the fiducials in this list, preserving ending numbers
    vtkKWPushButton *RenameButton;

    /// 
    /// pop up dialogue to get the new fid name
    vtkKWSimpleEntryDialog *RenameDialogue;

    /// 
    /// add a point
    vtkKWPushButton *AddFiducialButton;
    /// 
    /// remove the last selected (multi column list definition of selected)
    /// point
    vtkKWPushButton *RemoveFiducialButton;
    /// 
    /// remove all the fiducial points on this list
    vtkKWPushButton *RemoveFiducialsInListButton;
    /// 
    /// remove all fiducials on all lists
    vtkKWPushButton *RemoveAllFiducialsButton;
    /// 
    /// lock all fiducial lists
    vtkKWPushButtonWithLabel *LockAllFiducialsButton;
    /// 
    /// unlock all fiducial lists
    vtkKWPushButtonWithLabel *UnlockAllFiducialsButton;
    /// 
    /// select all fiducial points on this list
    vtkKWPushButton *SelectAllFiducialsInListButton;
    //Description:
    /// selects all fiducials points on all lists.
    vtkKWPushButton *SelectAllFiducialsButton;
    /// 
    /// deselect all fiducial points on this list
    vtkKWPushButton *DeselectAllFiducialsInListButton;
    /// 
    /// deselects all fiducial point in all lists.
    vtkKWPushButton *DeselectAllFiducialsButton;

    /// 
    /// moves the selected fiducial up/down in the list.
    vtkKWPushButton *MoveSelectedFiducialUpButton;
    vtkKWPushButton *MoveSelectedFiducialDownButton;

    /// 
    /// centers the 3d view on the selected fiducial
    vtkKWPushButton *Center3DViewOnSelectedFiducialButton;

    /// 
    /// list visibility, overrides individual point visibility
    vtkKWPushButton *HideListToggle;

    /// 
    /// removes selected fiducial list and its fiducials.
    vtkKWPushButton *RemoveSelectedListButton;

    /// 
    /// list colour
    vtkKWChangeColorButton *ListColorButton;

    /// 
    /// list selected fiducial colour
    vtkKWChangeColorButton *ListSelectedColorButton;

    /// 
    /// symbol scale
    vtkKWScaleWithEntry *ListSymbolScale;

    /// 
    /// symbol type
    vtkKWMenuButtonWithLabel *ListSymbolTypeMenu;
    /// 
    /// text scale
    vtkKWScaleWithEntry *ListTextScale;

    /// 
    /// opacity
    vtkKWScaleWithEntry *ListOpacity;

    /// 
    /// list actor ambient
    vtkKWScaleWithEntry *ListAmbient;

    /// 
    /// list actor diffuse
    vtkKWScaleWithEntry *ListDiffuse;
    
    /// 
    /// list actor specular
    vtkKWScaleWithEntry *ListSpecular;

    /// 
    /// list actor power
    vtkKWScaleWithEntry *ListPower;
    
    /// 
    /// display the points on the list
    vtkKWMultiColumnListWithScrollbars *MultiColumnList;

    /// 
    /// menu button whose menu exposes options for
    /// locking or unlocking all fiducials in all fiducial lists.
    vtkKWMenuButton *AllLockMenuButton;

    /// 
    /// menu button whose menu exposes options for
    /// setting visibility of  all fiducials in all fiducial lists.
    vtkKWMenuButton *AllVisibilityMenuButton;

    /// 
    /// push button to toggle
    /// locking or unlocking all fiducials in a fiducial list.
    vtkKWPushButton *FiducialsListLockToggle;

    /// 
    /// menu button whose menu exposes options for
    /// setting visibility of all fiducials in a fiducial list.
    vtkKWMenuButton *ListVisibilityMenuButton;

    /// 
    /// choose the numbering scheme for the next fiducial added in the list
    vtkKWMenuButtonWithLabel *ListNumberingSchemeMenu;

    /// menu button whose menu exposes options
    /// for hiding or exposing all fiducial lists.
    vtkKWMenuButton *HideOrExposeAllFiducialListsMenuButton;
  
    /// 
    /// contributing logo widgets
    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;

    //BTX
    /// 
    /// The column orders in the list box
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
    vtkSlicerFiducialsGUI ( const vtkSlicerFiducialsGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerFiducialsGUI& ); //Not implemented.
};


#endif
