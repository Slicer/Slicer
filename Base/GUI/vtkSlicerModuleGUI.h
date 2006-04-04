#ifndef __vtkSlicerModuleGUI_h
#define __vtkSlicerModuleGUI_h

#include "vtkObject.h"
#include "vtkKWObject.h"
#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerApplicationLogic.h"
#include "vtkMRMLScene.h"
#include "vtkSlicerLogic.h"
#include "vtkKWUserInterfacePanel.h"
#include "vtkSlicerComponentGUI.h"

class vtkSlicerGUIUpdate;
class vtkSlicerLogicUpdate;
class vtkSlicerMrmlUpdate;
class vtkKWApplication;
class vtkKWFrame;


// Description:
// This is a base class from which all SlicerAdditionalGUIs are derived,
// including the main vtkSlicerApplicationGUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModuleGUI : public vtkSlicerComponentGUI
{

 public:
    static vtkSlicerModuleGUI* New ( );
    vtkTypeRevisionMacro ( vtkSlicerModuleGUI, vtkKWObject );

    // Description:
    // Get/Set pointers to the ApplicationLogic and MrmlScene.
    vtkGetObjectMacro ( Logic, vtkSlicerApplicationLogic );
    vtkSetObjectMacro ( Logic, vtkSlicerApplicationLogic );
    vtkGetObjectMacro ( Mrml, vtkMRMLScene );
    vtkSetObjectMacro ( Mrml, vtkMRMLScene );
    vtkGetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    vtkSetObjectMacro ( UIPanel, vtkKWUserInterfacePanel );
    vtkGetMacro (ModuleUIPageID, int);
    vtkSetMacro (ModuleUIPageID, int);
    
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
    // Remove observers on logic, GUI and Mrml
    virtual void RemoveGUIObservers ( ) { }
    virtual void RemoveLogicObservers ( ) { }
    virtual void RemoveMrmlObservers ( ) { }


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
    
    vtkSlicerGUIUpdate *GUICommand;
    vtkSlicerLogicUpdate *LogicCommand;
    vtkSlicerMrmlUpdate *MrmlCommand;

    vtkKWUserInterfacePanel *UIPanel;
    int ModuleUIPageID;
    // frame into which this GUI's widgets will be packed.
    vtkKWFrame *Parent;
    
    // constructor, destructor.
    vtkSlicerModuleGUI ( );
    ~vtkSlicerModuleGUI ( );
    
 private:
    vtkSlicerModuleGUI ( const vtkSlicerModuleGUI& ); // Not implemented.
    void operator = ( const vtkSlicerModuleGUI& ); // Not implemented.
};


#endif


