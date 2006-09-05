// .NAME vtkSlicerSlicesControlGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// SlicesControl GUI Panel in slicer3.

#ifndef __vtkSlicerSlicesControlGUI_h
#define __vtkSlicerSlicesControlGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"

class vtkSlicerApplicationGUI;
class vtkKWScale;
class vtkKWPushButton;

// Description:
// This class implements Slicer's SlicesControl Panel on Main GUI panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSlicesControlGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerSlicesControlGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSlicesControlGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get the widgets in the SlicesControlFrame
    vtkGetObjectMacro (ToggleAnnotationButton, vtkKWPushButton );
    vtkGetObjectMacro (ToggleFgBgButton, vtkKWPushButton );
    vtkGetObjectMacro (SliceFadeScale, vtkKWScale );
    vtkGetObjectMacro (SliceOpacityScale, vtkKWScale );

    // Description:
    // Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );
    
    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWFrame *appF ) ;
    
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
    // Describe the behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    
 protected:
    vtkSlicerSlicesControlGUI ( );
    virtual ~vtkSlicerSlicesControlGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;
        //Description:
    // Widgets for the SlicesControlFrame in the GUI
    vtkKWPushButton *ToggleAnnotationButton;
    vtkKWPushButton *ToggleFgBgButton;
    vtkKWScale *SliceFadeScale;
    vtkKWScale *SliceOpacityScale;
    
 private:
    vtkSlicerSlicesControlGUI ( const vtkSlicerSlicesControlGUI& ); // Not implemented.
    void operator = ( const vtkSlicerSlicesControlGUI& ); //Not implemented.
};


#endif
