#ifndef __vtkSlicerMainDesktopGUI_h
#define __vtkSlicerMainDesktopGUI_h

#include "vtkSlicerComponentGUI.h"

#include "vtkKWScale.h"

class vtkKWWindow;
class vtkKWFrame;
class vtkKWLabel;
class vtkKWScale;
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

    // Basic window for Slicer's desktop expression.
    vtkKWWindow *myWindow;
    vtkKWScale *myScale;
    vtkKWFrame *myFrame;
    vtkKWLabel *myLabel;

    virtual void MakeWindow ( );
    virtual void MakeWidgets ( );
    // overriding base class definitions
    virtual int BuildGUI ( vtkSlicerApplicationGUI *app );
    virtual void AddGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void ProcessCallbackCommandEvents (vtkObject *, unsigned long, void * );
    virtual void ProcessLogicEvents ( vtkObject *, unsigned long, void * );
    
 protected:
    vtkSlicerMainDesktopGUI ( );
    ~vtkSlicerMainDesktopGUI ( );

 private:
    vtkSlicerMainDesktopGUI ( const vtkSlicerMainDesktopGUI& ); //Not implemented.
    void operator = ( const vtkSlicerMainDesktopGUI& ); //Not implemented.
};

#endif
