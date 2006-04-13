#ifndef __vtkSlicerComponentGUI_h
#define __vtkSlicerComponentGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerModuleLogic.h"


class vtkSlicerGUIUpdate;
class vtkSlicerLogicUpdate;
class vtkSlicerMRMLUpdate;
class vtkKWApplication;
class vtkKWFrame;

// Description:
// This is a base class from which all SlicerAdditionalGUIs are derived,
// including the main vtkSlicerApplicationGUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerComponentGUI : public vtkKWObject
{

 public:
    static vtkSlicerComponentGUI* New ( ) { return NULL; } ;
    vtkTypeRevisionMacro ( vtkSlicerComponentGUI, vtkKWObject );

    // Description:
    // Get/Set pointer to the ApplicationLogic
    vtkGetObjectMacro ( ApplicationLogic, vtkSlicerApplicationLogic );
    // Description:
    // Get/Set pointer to MRML scene
    vtkGetObjectMacro ( MRMLScene, vtkMRMLScene );

    // Description:
    // Get/Set the name of the GUI, used to raise the GUI.
    vtkSetStringMacro ( GUIName );
    vtkGetStringMacro ( GUIName );
    
    // Description:
    // Sets pointer to the application logic and adds observers.
    virtual void SetApplicationLogic ( vtkSlicerApplicationLogic *logic );
    virtual void SetLogic ( vtkSlicerModuleLogic *logic );
    virtual void SetMRMLScene ( vtkMRMLScene *mrml );

    // Description:
    // Specifies all widgets for this GUI
    // Define function in subclasses.
    virtual void BuildGUI ( ) { }

    // Description:
    // Create/Remove observers on widgets defined in this class
    // Define function in subclasses.
    virtual void AddGUIObservers ( ) { }
    virtual void RemoveGUIObservers ( ) { }
    
    // Description:
    // Create/Remove observers on logic in application layer 
    virtual void AddApplicationLogicObservers ( ) { }
    virtual void RemoveApplicationLogicObservers ( ) { }

    // Description:
    // Create/Remove observers on various module logic
    virtual void AddLogicObservers ( ) { }
    virtual void RemoveLogicObservers ( ) { }

    // Description:
    // Create/Remove observers on an individual module logic, used when
    // logic pointers are changed.
    virtual void AddLogicObserver ( vtkSlicerModuleLogic *logic, int event ) { }
    virtual void RemoveLogicObserver ( vtkSlicerModuleLogic *logic, int event ) { }
    
    // Description:
    // Create/Remove observers on mrml in application layer
    virtual void AddMRMLObservers ( ) { }
    virtual void RemoveMRMLObservers ( ) { }
    
    // Description:
    // Create/Remove observers on individual mrml node
    virtual void AddMRMLObserver ( vtkMRMLNode *node, int event ) { }
    virtual void RemoveMRMLObserver ( vtkMRMLNode *node, int event ) { }

    // Description:
    // propagate events generated in logic layer to GUI
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                         void *callData ) { }
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                       void *callData ) { }
    
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                       void *callData ) { }

    // Description:
    // functions that define and undefine module-specific behaviors.
    virtual void Enter ( ) { }
    virtual void Exit ( ) { }

    
 protected:
    // GUI's interface to the application layer;
    vtkSlicerApplicationLogic *ApplicationLogic;
    vtkSlicerModuleLogic *Logic;
    vtkMRMLScene *MRMLScene;
    
    // GUI's name, used to raise GUI
    char *GUIName;
    
    // helper functions that execute ProcessLogicEvents,
    // ProcessGUIEvents and ProcessMRMLEvents.
    vtkSlicerGUIUpdate *GUICommand;
    vtkSlicerLogicUpdate *LogicCommand;
    vtkSlicerMRMLUpdate *MRMLCommand;

    // constructor, destructor.
    vtkSlicerComponentGUI ( );
    ~vtkSlicerComponentGUI ( );
    
 private:
    vtkSlicerComponentGUI ( const vtkSlicerComponentGUI& ); // Not implemented.
    void operator = ( const vtkSlicerComponentGUI& ); // Not implemented.
};


#endif


