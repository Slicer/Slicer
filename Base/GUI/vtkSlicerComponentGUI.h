#ifndef __vtkSlicerComponentGUI_h
#define __vtkSlicerComponentGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerApplicationLogic.h"
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
    static vtkSlicerComponentGUI* New ( ) { return NULL; } ;
    vtkTypeRevisionMacro ( vtkSlicerComponentGUI, vtkKWObject );

    // Description:
    // Get/Set pointers to the ApplicationLogic
    vtkGetObjectMacro ( Logic, vtkSlicerApplicationLogic );
    vtkSetObjectMacro ( Logic, vtkSlicerApplicationLogic );
    vtkSetStringMacro ( GUIName );
    vtkGetStringMacro ( GUIName );
    
    // Description:
    // Specifies all widgets for this GUI
    // Define function in subclasses.
    virtual void BuildGUI ( ) = 0;

    // Description:
    // Create observers on widgets defined in this class
    // Define function in subclasses.
    virtual void AddGUIObservers ( ) = 0;
    // Description:
    // Create observers on logic in application layer 
    // Define function in subclasses.
    virtual void AddLogicObservers ( ) = 0;
    
    // Description:
    // Remove observers on logic, GUI
    // Define functions in subclasses.
    virtual void RemoveGUIObservers ( ) = 0;
    virtual void RemoveLogicObservers ( ) = 0;


    // Description:
    // propagate events generated in logic layer to GUI
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                         void *callData ) { }
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                       void *callData ) { }
    
    
 protected:
    // GUI's interface to the application layer;
    vtkSlicerApplicationLogic *Logic;
    char *GUIName;
    
    vtkSlicerGUIUpdate *GUICommand;
    vtkSlicerLogicUpdate *LogicCommand;

    // constructor, destructor.
    vtkSlicerComponentGUI ( );
    ~vtkSlicerComponentGUI ( );
    
 private:
    vtkSlicerComponentGUI ( const vtkSlicerComponentGUI& ); // Not implemented.
    void operator = ( const vtkSlicerComponentGUI& ); // Not implemented.
};


#endif


