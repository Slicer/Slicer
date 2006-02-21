#ifndef __vtkSlicerLogicUpdate_h
#define __vtkSlicerLogicUpdate_h

#include "vtkCallbackCommand.h"
#include "vtkSlicerComponentGUI.h"

class vtkObject;

class vtkSlicerLogicUpdate : public vtkCallbackCommand
{
 public:
    static vtkSlicerLogicUpdate* New ( )
        { return new vtkSlicerLogicUpdate; }
    vtkTypeRevisionMacro ( vtkSlicerLogicUpdate, vtkCallbackCommand );

    void SetGUI ( vtkSlicerComponentGUI *gui ) { this->GUI = gui; }
    vtkSlicerComponentGUI *GetGUI ( ) { return this->GUI; }
    void Execute ( vtkObject *caller, unsigned long event, void *callData )
        {
            this->GUI->UpdateLogicWithGUIEvents ( caller, event, callData );
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
