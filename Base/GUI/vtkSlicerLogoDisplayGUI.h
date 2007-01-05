// .NAME vtkSlicerLogoDisplayGUI
// .SECTION Description
// Main Data GUI and mediator methods for
// ViewControl GUI Panel in slicer3.

#ifndef __vtkSlicerLogoDisplayGUI_h
#define __vtkSlicerLogoDisplayGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerLogoIcons.h"

#include "vtkKWFrame.h"
#include "vtkKWPushButton.h"

class vtkSlicerApplicationGUI;
class vtkKWLabel;
class vtkSlicerViewControlIcons;
class vtkLogoWidget;

// Description:
// This class implements Slicer's Logo Display panel in the Main GUI panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerLogoDisplayGUI : public vtkSlicerComponentGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerLogoDisplayGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerLogoDisplayGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get widgets in the logo frame
    vtkGetObjectMacro (SlicerLogoLabel, vtkKWLabel );
    vtkGetObjectMacro (ModuleLogoLabel, vtkKWLabel );
    vtkGetObjectMacro (SlicerLogoIcons, vtkSlicerLogoIcons );
    vtkGetObjectMacro ( LogoWidget, vtkLogoWidget);

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
    vtkSlicerLogoDisplayGUI ( );
    virtual ~vtkSlicerLogoDisplayGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;
    // Description:
    // Logo-related widgets
    vtkLogoWidget *LogoWidget;
    vtkSlicerLogoIcons *SlicerLogoIcons;
    vtkKWLabel *SlicerLogoLabel;
    vtkKWLabel *ModuleLogoLabel;
    
 private:
    vtkSlicerLogoDisplayGUI ( const vtkSlicerLogoDisplayGUI& ); // Not implemented.
    void operator = ( const vtkSlicerLogoDisplayGUI& ); //Not implemented.
};


#endif
