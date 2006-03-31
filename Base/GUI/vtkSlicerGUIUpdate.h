#ifndef __vtkSlicerGUIUpdate_h
#define __vtkSlicerGUIUpdate_h

#include "vtkCallbackCommand.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkSlicerBaseGUIWin32Header.h"

class vtkObject;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerGUIUpdate : public vtkCallbackCommand
{
 public:
    static vtkSlicerGUIUpdate* New ( )
        { return new vtkSlicerGUIUpdate; }
    vtkTypeRevisionMacro ( vtkSlicerGUIUpdate, vtkCallbackCommand );

    void SetGUI ( vtkSlicerComponentGUI *gui ) { this->GUI = gui; }
    vtkSlicerComponentGUI *GetGUI ( ) { return this->GUI; }
    void Execute ( vtkObject *caller, unsigned long event, void *callData )
        {
            this->GUI->ProcessGUIEvents ( caller, event, callData );
        }

 protected:
    vtkSlicerComponentGUI *GUI;

    vtkSlicerGUIUpdate ( );
    ~vtkSlicerGUIUpdate ( );

 private:
    vtkSlicerGUIUpdate ( const vtkSlicerGUIUpdate& ); // Not implemented.
    void operator = (const vtkSlicerGUIUpdate& ); // Not implemented.
    
};

#endif
