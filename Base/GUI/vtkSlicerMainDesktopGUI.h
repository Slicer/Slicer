#ifndef __vtkSlicerMainDesktopGUI_h
#define __vtkSlicerMainDesktopGUI_h

#include "vtkSlicerComponentGUI.h"

#include "vtkKWScale.h"

class vtkKWWindow;
class vtkKWFrame;
// for file browse
class vtkKWLoadSaveButtonWithLabel;
class vtkKWLoadSaveDialog;
// for image viewer
class vtkKWRenderWidget;
class vtkImageViewer2;
class vtkKWScale;
class vtkKWWIndowLevelPresetSelector;
class vtkSlicerApplicationGUI;

// Description:
// For now just a test. This is the class derived from vtkSlicerComponentGUI
// that will create a window and gui, set a pointer to logic classes,
// add observers on gui and logic, and define functions to exectute when
// events are observed.
//
// In this simple example, a scale widget event causes logic to be modified,
// and the logic state change causes the gui label text to update. Just
// demos the basic mechanisms. Will this fail in some obvious cases?
//
class vtkSlicerMainDesktopGUI : public vtkSlicerComponentGUI
{
 public:
    static vtkSlicerMainDesktopGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerMainDesktopGUI, vtkSlicerComponentGUI );

    virtual void MakeWindow ( );
    virtual void MakeWidgets ( );
    // overriding base class definitions
    virtual int BuildGUI ( vtkSlicerApplicationGUI *app );
    virtual void AddGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void ProcessCallbackCommandEvents (vtkObject *, unsigned long, void * );
    virtual void ProcessLogicEvents ( vtkObject *, unsigned long, void * );
    
 protected:
    // Basic window for Slicer's desktop expression.
    vtkKWWindow *Window;
    vtkSlicerMRMLLogic *MRMLLogic;
    // And widgets.
    vtkKWLoadSaveButtonWithLabel *FileBrowseButton;
    vtkImageViewer2 *ImageViewer;
    vtkKWWindowLevelPresetSelector *WindowLevelPresetSelector;
    vtkKWRenderWidget *RenderWidget;
    vtkKWScale *Scale;
    vtkKWFrame *Frame;

    vtkSlicerMainDesktopGUI ( );
    ~vtkSlicerMainDesktopGUI ( );

 private:
    vtkSlicerMainDesktopGUI ( const vtkSlicerMainDesktopGUI& ); //Not implemented.
    void operator = ( const vtkSlicerMainDesktopGUI& ); //Not implemented.
};

#endif
