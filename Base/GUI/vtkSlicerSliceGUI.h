#ifndef __vtkSlicerVolumeSelectGUI_h
#define __vtkSlicerVolumeSelectGUI_h

#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerApplicationLogic.h"

class vtkKWWindow;
// for file browse
class vtkSlicerApplicationGUI;

// Description:
// Manages a popup menu of the currently available MRMLVolumes in the scene
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVolumeSelectGUI : public vtkSlicerComponentGUI
{
 public:
    static vtkSlicerVolumeSelectGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerVolumeSelectGUI, vtkSlicerComponentGUI );

    virtual void MakeWindow ( );
    virtual void MakeWidgets ( );
    // overriding base class definitions
    virtual int BuildGUI ( vtkSlicerApplicationGUI *app );
    virtual void AddGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void UpdateGUIWithLogicEvents ( vtkObject *, unsigned long, void * );
    // try using this:
    virtual void UpdateLogicWithGUIEvents ( vtkObject *, unsigned long, void * );
    // instead of this:
    virtual void ProcessCallbackCommandEvents (vtkObject *, unsigned long, void * );

    vtkGetObjectMacro (ApplicationLogic, vtkSlicerApplicationLogic);
    vtkSetObjectMacro (ApplicationLogic, vtkSlicerApplicationLogic);
    
 protected:
    // Basic window for Slicer's desktop expression.
    vtkKWWindow *Window;
    vtkSlicerApplicationLogic *ApplicationLogic;
    // And widgets.
    vtkKWLoadSaveButton *FileBrowseButton;
    vtkImageViewer2 *ImageViewer;
    vtkKWWindowLevelPresetSelector *WindowLevelPresetSelector;
    vtkKWRenderWidget *RenderWidget;
    vtkKWScale *Scale;

    vtkSlicerVolumeSelectGUI ( );
    ~vtkSlicerVolumeSelectGUI ( );

 private:
    vtkSlicerVolumeSelectGUI ( const vtkSlicerVolumeSelectGUI& ); //Not implemented.
    void operator = ( const vtkSlicerVolumeSelectGUI& ); //Not implemented.
};

#endif
