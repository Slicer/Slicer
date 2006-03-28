#ifndef __vtkSlicerMrmlUpdate_h
#define __vtkSlicerMrmlUpdate_h

#include "vtkCallbackCommand.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkSlicerBaseGUIWin32Header.h"

class vtkObject;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerMrmlUpdate : public vtkCallbackCommand
{
 public:
    static vtkSlicerMrmlUpdate* New ( )
        { return new vtkSlicerMrmlUpdate; }
    vtkTypeRevisionMacro ( vtkSlicerMrmlUpdate, vtkCallbackCommand );

    void SetGUI ( vtkSlicerComponentGUI *gui ) { this->GUI = gui; }
    vtkSlicerComponentGUI *GetGUI ( ) { return this->GUI; }
    void Execute ( vtkObject *caller, unsigned long event, void *callData )
        {
            this->GUI->ProcessGUIEvents ( caller, event, callData );
        }

 protected:
    vtkSlicerComponentGUI *GUI;

    vtkSlicerMrmlUpdate ( );
    ~vtkSlicerMrmlUpdate ( );

 private:
    vtkSlicerMrmlUpdate ( const vtkSlicerMrmlUpdate& ); // Not implemented.
    void operator = (const vtkSlicerMrmlUpdate& ); // Not implemented.
    
};

#endif
