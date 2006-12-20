// .NAME vtkSlicerModuleChooseGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// Module selection frame in Slicer main application GUI

#ifndef __vtkSlicerModuleChooseGUI_h
#define __vtkSlicerModuleChooseGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

class vtkSlicerApplicationGUI;
class vtkSlicerModuleNavigationIcons;
class vtkSlicerModuleNavigator;
class vtkKWPushButton;
class vtkKWLabel;
class vtkKWMenuButton;
class vtkKWFrame;
class vtkKWEntry;
class vtkKWToolbar;

// Description:
// This class implements Slicer's Module Selection GUI panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleChooseGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerModuleChooseGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerModuleChooseGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get the widgets in the ModuleChooseFrame
    vtkGetObjectMacro ( ModulesMenuButton, vtkKWMenuButton );
    vtkGetObjectMacro ( ModulesLabel, vtkKWLabel );
    vtkGetObjectMacro ( ModulesPrev, vtkKWPushButton );
    vtkGetObjectMacro ( ModulesNext, vtkKWPushButton );    
    vtkGetObjectMacro (ModulesHistory, vtkKWMenuButton );
    vtkGetObjectMacro (ModulesRefresh, vtkKWPushButton );
    vtkGetObjectMacro (ModulesSearch, vtkKWMenuButton );
    vtkGetObjectMacro (ModulesSearchEntry, vtkKWEntry );
    vtkGetObjectMacro (ModuleNavigationFrame, vtkKWFrame );
    vtkGetObjectMacro ( SlicerModuleNavigationIcons, vtkSlicerModuleNavigationIcons );
    vtkGetObjectMacro (ModuleNavigator, vtkSlicerModuleNavigator );

    // Description:
    // Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );

    // Description:
    // Populate the menu of modules
    void Populate();
  
    
    // Description:
    // Raise module's panel.
    void RaiseModule ( const char *moduleName );
    // Description:
    // Raise module's panel and add to navigation history.
    void SelectModule ( const char *moduleName );

    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWToolbar *tb ) ;
    
    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWFrame *appF ) ;
    
    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Add any newly visited modules to the ModulesHistory menu
    virtual void PopulateHistoryListMenu ( );
    
    // Descripton:
    // Search available modules names and categories for those
    // matching a searchString, case insensitive.
    void PopulateModuleSearchMenu ( const char *searchString );

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    // Description:
    // Describe the behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    
 protected:
    vtkSlicerModuleChooseGUI ( );
    virtual ~vtkSlicerModuleChooseGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;

    // Description:
    // Widgets for the modules GUI panels
    vtkKWMenuButton *ModulesMenuButton;
    vtkKWLabel *ModulesLabel;
    vtkKWPushButton *ModulesPrev;
    vtkKWPushButton *ModulesNext;
    vtkKWMenuButton *ModulesHistory;
    vtkKWPushButton *ModulesRefresh;
    vtkKWMenuButton *ModulesSearch;
    vtkSlicerModuleNavigationIcons *SlicerModuleNavigationIcons;
    vtkKWFrame *ModuleNavigationFrame;
    vtkKWEntry *ModulesSearchEntry;
    vtkSlicerModuleNavigator *ModuleNavigator;
    
 private:
    vtkSlicerModuleChooseGUI ( const vtkSlicerModuleChooseGUI& ); // Not implemented.
    void operator = ( const vtkSlicerModuleChooseGUI& ); //Not implemented.
};


#endif
