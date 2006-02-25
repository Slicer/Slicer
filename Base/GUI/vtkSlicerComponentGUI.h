#ifndef __vtkSlicerComponentGUI_h
#define __vtkSlicerComponentGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"

class vtkSlicerLogic;
class vtkSlicerApplicationLogic;
class vtkSlicerGUIUpdate;
class vtkSlicerLogicUpdate;
class vtkKWApplication;
class vtkKWFrame;


// Description:
// This is a base class from which all SlicerAdditionalGUIs are derived,
// including the main vtkSlicerApplicationGUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerComponentGUI : public vtkKWObject
{

 public:
    static vtkSlicerComponentGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerComponentGUI, vtkKWObject );

    // Description:
    // Determines where the GUI should be packed.
    // May be more than one parent being assigned
    // in a derived class' redef of this method.
    virtual void SetParent ( vtkKWFrame *frame );
    
    // Description:
    // Specifies all widgets for this GUI
    virtual void BuildGUI ( );

    // Description:
    // Set pointers to object in the application layer
    // which this gui component needs to know about.
    // May be more than one logic pointer being assigned
    // in a derived class' redef of this method.
    virtual void SetLogic ( vtkSlicerApplicationLogic *logic );

    // Description:
    // Create observers on widgets defined in this class using the following paradigm:
    // this->AddCallbackCommandObserver ( ObservedWidget, vtkCommand::SomeEvent);
    virtual void AddGUIObservers ( ) { }
    // Description:
    // Create observers on logic in application layer using the vtk paradigm:
    // Logic->mylogic->AddObserver ( vtkCommand::ModifiedEvent, this->LogicCommand);
    virtual void AddLogicObservers ( ) { }
    // propagate events generated in application layer to GUI
    virtual void UpdateGUIWithLogicEvents ( vtkObject *caller, unsigned long event,
                                void *callData );
    // alternative method to propagate events generated in GUI to app layer
    virtual void UpdateLogicWithGUIEvents ( vtkObject *caller, unsigned long event,
                                    void *callData );
    
    
 protected:
    // GUI's interface to the application layer;
    vtkSlicerApplicationLogic *Logic;
    vtkSlicerGUIUpdate *LogicCommand;
    // alternative Logic interface to the GUI layer.
    vtkSlicerLogicUpdate *GUICommand;
    // frame into which this GUI will be packed.
    vtkKWFrame *Parent;
    
    // constructor, destructor.
    vtkSlicerComponentGUI ( );
    ~vtkSlicerComponentGUI ( );
    
 private:
    vtkSlicerComponentGUI ( const vtkSlicerComponentGUI& ); // Not implemented.
    void operator = ( const vtkSlicerComponentGUI& ); // Not implemented.
};


#endif


