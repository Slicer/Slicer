#ifndef __vtkSlicerApplicationGUI_h
#define __vtkSlicerApplicationGUI_h

#include "vtkSlicerGUI.h"

class vtkKWApplication;
class vtkSlicerComponentGUI;
class vtkSlicerGUICollection;
class vtkKWWindowBase;
class vtkCollection;


// Description:
// Contains a vtkKWApplication object, 
// and a collection of vtkSlicerGUIUnits.

//
class vtkSlicerApplicationGUI : public vtkSlicerGUI
{
 public:
    static vtkSlicerApplicationGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerApplicationGUI, vtkSlicerGUI );

    virtual void SetKwApplication ( vtkKWApplication *app ) ;
    vtkKWApplication *GetKwApplication ( );
    
    // Description:
    // This method collects GUIs added to Slicer.
    virtual void AddGUI ( vtkSlicerComponentGUI *gui );
    // Description:
    // Sets application behavior.
    virtual void ConfigureApplication ( );
    // Description:
    // Starts up the application with no window
    virtual int StartApplication ( );
    // Description:
    // Adds a new window to the application
    virtual void AddWindow ( vtkKWWindowBase *win );
    // Description:
    // Closes all application windows
    virtual void CloseWindows ( );

    vtkGetMacro ( NumGUIs, int );
    
 protected:
    vtkSlicerApplicationGUI ( );
    ~vtkSlicerApplicationGUI ( );

    // Tcl_Interp *interp; 

    // Description:
    // Application widget
    vtkKWApplication *KWapp;

    // Description:
    // Collections of widgets
    vtkSlicerGUICollection *GUICollection;

    // Description:
    // numbers of widgets
    int NumGUIs;
    
 private:
    vtkSlicerApplicationGUI ( const vtkSlicerApplicationGUI& ); // Not implemented.
    void operator = ( const vtkSlicerGUI& ); //Not implemented.
}; 

#endif
