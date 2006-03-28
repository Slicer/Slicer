#ifndef __vtkSlicerComponentGUI_h
#define __vtkSlicerComponentGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerLogic.h"

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
    // Get/Set pointers to the ApplicationLogic and MrmlScene.
    vtkGetObjectMacro ( Logic, vtkSlicerApplicationLogic );
    vtkSetObjectMacro ( Logic, vtkSlicerApplicationLogic );
    vtkGetObjectMacro ( Mrml, vtkMRMLScene );
    vtkSetObjectMacro ( Mrml, vtkMRMLScene );
    
    // Description:
    // Determines where the GUI should be packed.
    // May be more than one parent being assigned
    // in a derived class' redef of this method.
    virtual void SetParent ( vtkKWFrame *frame );
    
    // Description:
    // Specifies all widgets for this GUI
    virtual void BuildGUI ( );

    // Description:
    // Create observers on widgets defined in this class using the following paradigm:
    // this->AddCallbackCommandObserver ( ObservedWidget, vtkCommand::SomeEvent);
    virtual void AddGUIObservers ( ) { }
    // Description:
    // Create observers on logic in application layer using the vtk paradigm:
    // Logic->mylogic->AddObserver ( vtkCommand::ModifiedEvent, this->LogicCommand);
    virtual void AddLogicObservers ( ) { }
    // Description:
    // Create observers on logic in application layer using the vtk paradigm:
    // Mrml->thing->AddObserver ( vtkCommand::ModifiedEvent, this->MrmlCommand);
    virtual void AddMrmlObservers ( ) { }
    
    // Description:
    // propagate events generated in logic layer to GUI
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                void *callData );
    // Description:
    // propagate events generated in mrml layer to GUI
    virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                void *callData );
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                    void *callData );
    
    
 protected:
    // GUI's interface to the application layer;
    vtkSlicerApplicationLogic *Logic;
    vtkMRMLScene *Mrml;
    
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


