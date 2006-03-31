#ifndef __vtkSlicerLogicUpdate_h
#define __vtkSlicerLogicUpdate_h

#include "vtkCallbackCommand.h"
#include "vtkSlicerComponentGUI.h"

#include "vtkSlicerBaseGUIWin32Header.h"

class vtkObject;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerLogicUpdate : public vtkCallbackCommand
{
 public:
    static vtkSlicerLogicUpdate* New ( )
        { return new vtkSlicerLogicUpdate; }
    vtkTypeRevisionMacro ( vtkSlicerLogicUpdate, vtkCallbackCommand );

    void SetGUI ( vtkSlicerComponentGUI *gui ) { this->GUI = gui; }
    vtkSlicerComponentGUI *GetGUI ( ) { return this->GUI; }
    void Execute ( vtkObject *caller, unsigned long event, void *callData )
        {
            this->GUI->ProcessLogicEvents ( caller, event, callData );
        }

 protected:
    vtkSlicerComponentGUI *GUI;

    vtkSlicerLogicUpdate ( );
    ~vtkSlicerLogicUpdate ( );

 private:
    vtkSlicerLogicUpdate ( const vtkSlicerLogicUpdate& ); // Not implemented.
    void operator = (const vtkSlicerLogicUpdate& ); // Not implemented.
    
};

#endif
