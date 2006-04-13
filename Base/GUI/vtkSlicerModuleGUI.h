#ifndef __vtkSlicerModuleGUI_h
#define __vtkSlicerModuleGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkSlicerComponentGUI.h"


// Description:
// This is a base class from which all SlicerModuleGUIs that include
// their GUI in Slicer's shared GUI panel are derived. SlicerModuleGUIs
// that don't populate that panel with their widgets can derive
// directly from vtkSlicerComponentGUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleGUI : public vtkSlicerComponentGUI
{

 public:
    static vtkSlicerModuleGUI* New ( ) { return NULL; } ;
    vtkTypeRevisionMacro ( vtkSlicerModuleGUI, vtkKWObject );

    // Description:
    // Get/Set pointers to the ApplicationLogic
    vtkGetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    vtkSetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );

 protected:

    // Description:
    // This user interface panel is populated with the GUI's widgets,
    // and is raised in Slicer's shared GUI panel when the module
    // is selected for use.
    vtkKWUserInterfacePanel *UIPanel;
    
    // constructor, destructor.
    vtkSlicerModuleGUI ( );
    ~vtkSlicerModuleGUI ( );
    
 private:
    vtkSlicerModuleGUI ( const vtkSlicerModuleGUI& ); // Not implemented.
    void operator = ( const vtkSlicerModuleGUI& ); // Not implemented.
};


#endif


