// .NAME vtkSlicerComponentGUI 
// .SECTION Description
// Base class from which GUI classes derive. If a Slicer module's GUI
// will populate Slicer's main shared UIPanel, then it should be derived
// from vtkSlicerModuleGUI (which derives from this class but includes
// a widget that packs into the main shared UIPanel). If a Slicer module's
// GUI does *not* populate Slicer's main shared UIPanel, but will populate
// other panels in Slicer's window or another toplevel window with widgets, 
// then the module can be subclassed directly from this class.

#ifndef __vtkSlicerComponentGUI_h
#define __vtkSlicerComponentGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkSlicerLogic.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerLogic.h"


class vtkCallbackCommmand;
class vtkSlicerGUIUpdate;
class vtkSlicerLogicUpdate;
class vtkSlicerMRMLUpdate;
class vtkKWApplication;
class vtkKWFrame;

#ifndef vtkObjectPointer
#define vtkObjectPointer( xx ) (reinterpret_cast <vtkObject **>( (xx) ))
#endif

// Description:
// This is a base class from which all SlicerAdditionalGUIs are derived,
// including the main vtkSlicerApplicationGUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerComponentGUI : public vtkKWObject
{

 public:
    // The usual vtk class functions
    static vtkSlicerComponentGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerComponentGUI, vtkKWObject );
    void PrintSelf ( ostream& os, vtkIndent indent );

    // Description:
    // Get Macro for ApplicationLogic: GUI class's interface to logic
    // To Set ApplicationLogic, use SetLogic method.
    vtkGetObjectMacro ( ApplicationLogic, vtkSlicerApplicationLogic );

    // Description:
    // GetMacro for MRML scene: GUI class's interface to mrml.
    // To Set MRMLScene, use SetMRML method.
    vtkGetObjectMacro ( MRMLScene, vtkMRMLScene );

    // Description:
    // API for setting MRMLScene, ApplicationLogic and
    // for both setting and observing them.
    void SetMRMLScene ( vtkMRMLScene *mrml )
        { this->SetMRML ( vtkObjectPointer( &this->MRMLScene), mrml ); }
    void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLScene), mrml ); }
    virtual void SetApplicationLogic ( vtkSlicerApplicationLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->ApplicationLogic), logic ); }
    void SetAndObserveApplicationLogic ( vtkSlicerApplicationLogic *logic )
        { this->SetAndObserveLogic ( vtkObjectPointer (&this->ApplicationLogic), logic ); }
    
    // Description:
    // Add/Remove observers on a GUI.
    virtual void AddGUIObservers ( ) { };
    virtual void RemoveGUIObservers ( ) { };
    // Description:
    // Remove all observers on MRML scene, nodes and logic
    virtual void RemoveMRMLNodeObservers ( ) { };
    virtual void RemoveLogicObservers ( ) { };
    
    // Description:
    // Get/Set the name of the GUI, used to find and raise a GUI.
    vtkSetStringMacro ( GUIName );
    vtkGetStringMacro ( GUIName );
    
    // Description:
    // Specifies all widgets for this GUI
    // Define function in subclasses.
    virtual void BuildGUI ( ) { };

    // Description:
    // propagate events generated in logic layer to GUI
    virtual void ProcessLogicEvents ( vtkObject * /*caller*/,
      unsigned long /*event*/, void * /*callData*/ ) { };
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessGUIEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };
    
    // Description:
    // alternative method to propagate events generated in GUI to logic / mrml
    virtual void ProcessMRMLEvents ( vtkObject * /*caller*/, 
      unsigned long /*event*/, void * /*callData*/ ) { };

    // Description:
    // functions that define and undefine module-specific behaviors.
    virtual void Enter ( ) { };
    virtual void Exit ( ) { };

 protected:
    // GUI's interface to the application layer;
    vtkSlicerApplicationLogic *ApplicationLogic;
    vtkMRMLScene *MRMLScene;
    
    // GUI's name, used to raise GUI
    char *GUIName;

    //BTX
    // a shared set of functions that call the
    // virtual ProcessMRMLEvents, ProcessLogicEvents,
    // and ProcessGUIEvents methods in the
    // subclasses, if they are defined.
    static void MRMLCallback( vtkObject *__caller,
                              unsigned long eid, void *__clientData, void *callData );
    static void LogicCallback( vtkObject *__caller,
                              unsigned long eid, void *__clientData, void *callData );
    static void GUICallback( vtkObject *__caller,
                              unsigned long eid, void *__clientData, void *callData );    
    
    // functions that set MRML and Logic pointers for the GUI class,
    // either with or without adding/removing observers on them.
    void SetMRML ( vtkObject **nodePtr, vtkObject *node );
    void SetAndObserveMRML ( vtkObject **nodePtr, vtkObject *node );
    void SetLogic ( vtkObject **logicPtr, vtkObject *logic );
    void SetAndObserveLogic ( vtkObject **logicPtr, vtkObject *logic );
    //ETX
    
    // Description::
    // Holders for MRML, GUI and Logic callbacks
    vtkCallbackCommand *MRMLCallbackCommand;
    vtkCallbackCommand *LogicCallbackCommand;
    vtkCallbackCommand *GUICallbackCommand;
    
    // constructor, destructor.
    vtkSlicerComponentGUI ( );
    ~vtkSlicerComponentGUI ( );
    
 private:
    vtkSlicerComponentGUI ( const vtkSlicerComponentGUI& ); // Not implemented.
    void operator = ( const vtkSlicerComponentGUI& ); // Not implemented.
};


#endif


