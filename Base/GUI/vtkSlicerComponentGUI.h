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
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerLogic.h"
#include "vtkObserverManager.h"


class vtkCallbackCommmand;
class vtkSlicerGUIUpdate;
class vtkSlicerLogicUpdate;
class vtkSlicerMRMLUpdate;
class vtkKWApplication;
class vtkKWFrame;


#include <string>

//BTX

#ifndef vtkSetMRMLNodeMacro
#define vtkSetMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetObject ( vtkObjectPointer( &(node)), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeMacro
#define vtkSetAndObserveMRMLNodeMacro(node,value)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &(node) ), (value) ); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

#ifndef vtkSetAndObserveMRMLNodeEventsMacro
#define vtkSetAndObserveMRMLNodeEventsMacro(node,value,events)  { \
  vtkObject *oldNode = (node); \
  this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &(node)), (value), (events)); \
  if ( oldNode != (node) ) \
    { \
    this->InvokeEvent (vtkCommand::ModifiedEvent); \
    } \
};
#endif

//ETX

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
  // Get Macro and Set Methods for ApplicationLogic: GUI class's interface to logic
  vtkGetObjectMacro ( ApplicationLogic, vtkSlicerApplicationLogic );
  virtual void SetApplicationLogic ( vtkSlicerApplicationLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->ApplicationLogic), logic );};
  void SetAndObserveApplicationLogic ( vtkSlicerApplicationLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->ApplicationLogic) , logic ); };

  // Description:
  // GetMacro for MRML scene: GUI class's interface to mrml.
  // To Set MRMLScene, use SetMRML method.
  vtkGetObjectMacro ( MRMLScene, vtkMRMLScene );
  
  // Description:
  // API for setting or setting and observing MRMLScene
  void SetMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLScene ( vtkMRMLScene *mrml )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObject ( vtkObjectPointer( &this->MRMLScene), mrml );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }

  void SetAndObserveMRMLSceneEvents ( vtkMRMLScene *mrml, vtkIntArray *events )
    {
    vtkObject *oldValue = this->MRMLScene;
    this->MRMLObserverManager->SetAndObserveObjectEvents ( vtkObjectPointer( &this->MRMLScene), mrml, events );
    if ( oldValue != this->MRMLScene )
      {
      this->InvokeEvent (vtkCommand::ModifiedEvent);
      }
    }
  
  void SetLogic ( vtkObject **logicPtr, vtkObject *logic );
  void SetAndObserveLogic ( vtkObject **logicPtr, vtkObject *logic );
  
  // Description:
  // Add/Remove observers on a GUI.
  virtual void AddGUIObservers ( ) { };
  virtual void RemoveGUIObservers ( ) { };
  
  // Description:
  // Callback commands so that tcl scripts can set Abort flag when
  // they process events
  vtkGetObjectMacro (LogicCallbackCommand, vtkCallbackCommand);
  vtkGetObjectMacro (GUICallbackCommand, vtkCallbackCommand);
  
  // Description:
  // Flags to avoid event loops
  // NOTE: don't use the SetMacro or it call modified itself and generate even more events!
  void SetInLogicCallbackFlag (int flag) {
    this->InLogicCallbackFlag = flag;
  }
  vtkGetMacro(InLogicCallbackFlag, int);
  void SetInMRMLCallbackFlag (int flag) {
    this->InMRMLCallbackFlag = flag;
  }
  vtkGetMacro(InMRMLCallbackFlag, int);
  void SetInGUICallbackFlag (int flag) {
    this->InGUICallbackFlag = flag;
    }
  vtkGetMacro(InGUICallbackFlag, int);
  
  // Description:
  // Get/Set the name of the GUI, used to find and raise a GUI.
  vtkSetStringMacro ( GUIName );
  vtkGetStringMacro ( GUIName );
  
  // Description:
  // Specifies all widgets for this GUI
  // Define function in subclasses.
  virtual void BuildGUI ( ) { };

  // Description:
  // Method in which references are released,
  // called before Delete().
  virtual void TearDownGUI ( ) { };
  
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
  //ETX
  
  // Description::
  // Holders for MRML, GUI and Logic callbacks
  vtkCallbackCommand *LogicCallbackCommand;
  vtkCallbackCommand *GUICallbackCommand;
  vtkCallbackCommand *MRMLCallbackCommand;
  
  // constructor, destructor.
  vtkSlicerComponentGUI ( );
  virtual ~vtkSlicerComponentGUI ( );
  
  // Description:
  // Flag to avoid event loops
  int InLogicCallbackFlag;
  int InMRMLCallbackFlag;
  int InGUICallbackFlag;
  
  vtkObserverManager *MRMLObserverManager;
  
private:
  vtkSlicerComponentGUI ( const vtkSlicerComponentGUI& ); // Not implemented.
  void operator = ( const vtkSlicerComponentGUI& ); // Not implemented.
};


#endif


