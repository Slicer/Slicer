#ifndef __vtkSlicerMRMLUpdate_h
#define __vtkSlicerMRMLUpdate_h

#include "vtkCallbackCommand.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkSlicerBaseGUIWin32Header.h"

class vtkObject;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerMRMLUpdate : public vtkCallbackCommand
{
 public:
    static vtkSlicerMRMLUpdate* New ( )
        { return new vtkSlicerMRMLUpdate; }
    vtkTypeRevisionMacro ( vtkSlicerMRMLUpdate, vtkCallbackCommand );

    void SetGUI ( vtkSlicerComponentGUI *gui ) { this->GUI = gui; }
    vtkSlicerComponentGUI *GetGUI ( ) { return this->GUI; }
    void Execute ( vtkObject *caller, unsigned long event, void *callData )
        {
            this->GUI->ProcessMRMLEvents ( caller, event, callData );
        }

 protected:
    vtkSlicerComponentGUI *GUI;

    vtkSlicerMRMLUpdate ( );
    ~vtkSlicerMRMLUpdate ( );

 private:
    vtkSlicerMRMLUpdate ( const vtkSlicerMRMLUpdate& ); // Not implemented.
    void operator = (const vtkSlicerMRMLUpdate& ); // Not implemented.
    
};

#endif
