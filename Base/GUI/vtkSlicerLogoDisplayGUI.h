///  vtkSlicerLogoDisplayGUI
/// 
/// Main Data GUI and mediator methods for
/// ViewControl GUI Panel in slicer3.

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

/// Description:
/// This class implements Slicer's Logo Display panel in the Main GUI panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerLogoDisplayGUI : public vtkSlicerComponentGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerLogoDisplayGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerLogoDisplayGUI, vtkSlicerComponentGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );

    /// 
    /// Get widgets in the logo frame
    vtkGetObjectMacro (SlicerLogoLabel, vtkKWLabel );
    vtkGetObjectMacro (ModuleLogoLabel, vtkKWLabel );
    vtkGetObjectMacro (SlicerLogoIcons, vtkSlicerLogoIcons );
    vtkGetObjectMacro ( LogoWidget, vtkLogoWidget);

    /// 
    /// Get the main slicer toolbars.
    vtkGetObjectMacro (ApplicationGUI, vtkSlicerApplicationGUI );
    virtual void SetApplicationGUI ( vtkSlicerApplicationGUI *appGUI );

    //BTX
    using vtkSlicerComponentGUI::BuildGUI; 
    //ETX
    
    /// 
    /// This method builds the Data module's GUI
    virtual void BuildGUI ( vtkKWFrame *appF ) ;
    
    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    /// 
    /// Class's mediator methods for processing events invoked by
    /// either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    //BTX
    using vtkSlicerComponentGUI::Enter; 
    //ETX
    
    /// 
    /// Describe the behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    
 protected:
    vtkSlicerLogoDisplayGUI ( );
    virtual ~vtkSlicerLogoDisplayGUI ( );

    vtkSlicerApplicationGUI *ApplicationGUI;
    /// 
    /// Logo-related widgets
    vtkLogoWidget *LogoWidget;
    vtkSlicerLogoIcons *SlicerLogoIcons;
    vtkKWLabel *SlicerLogoLabel;
    vtkKWLabel *ModuleLogoLabel;
    
 private:
    vtkSlicerLogoDisplayGUI ( const vtkSlicerLogoDisplayGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerLogoDisplayGUI& ); //Not implemented.
};


#endif
