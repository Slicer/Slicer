#ifndef __vtkSlicerGUIUpdate_h
#define __vtkSlicerGUIUpdate_h

#include "vtkCallbackCommand.h"
#include "vtkSlicerComponentGUI.h"

class vtkObject;

class vtkSlicerGUIUpdate : public vtkCallbackCommand
{
 public:
    static vtkSlicerGUIUpdate* New ( )
        { return new vtkSlicerGUIUpdate; }
    vtkTypeRevisionMacro ( vtkSlicerGUIUpdate, vtkCallbackCommand );

    void SetGUI ( vtkSlicerComponentGUI *gui ) { this->GUI = gui; }
    vtkSlicerComponentGUI *GetGUI ( ) { return this->GUI; }
    void Execute ( vtkObject *caller, unsigned long event, void *callData )
        {
            this->GUI->UpdateGUIWithLogicEvents ( caller, event, callData );
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
