#ifndef __vtkSlicerDataGUI_h
#define __vtkSlicerDataGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerDataLogic.h"

// Description:
// This class implements Slicer's Data GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDataGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerDataGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerDataGUI, vtkSlicerModuleGUI );
    //vtkGetObjectMacro ( Logic, vtkSlicerDataLogic);

    // Description:
    // Sets pointer to the module logic and adds observers.
    //virtual void SetLogic ( vtkSlicerDataLogic *logic );

    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( ) ;
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    virtual void AddLogicObserver ( vtkSlicerModuleLogic *logic, int event );
    virtual void RemoveLogicObserver ( vtkSlicerModuleLogic *logic, int event );

    virtual void AddLogicObservers ( );
    virtual void RemoveLogicObservers ( );

    virtual void AddMRMLObserver ( vtkMRMLNode *node, int event );
    virtual void RemoveMRMLObserver ( vtkMRMLNode *node, int event );

    virtual void AddMRMLObservers ( );
    virtual void RemoveMRMLObservers ( );
    
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerDataGUI ( );
    ~vtkSlicerDataGUI ( );

    // Logic pointer for the Data module
    //vtkSlicerDataLogic *Logic;
    
    // Widgets for the Data module

 private:
    vtkSlicerDataGUI ( const vtkSlicerDataGUI& ); // Not implemented.
    void operator = ( const vtkSlicerDataGUI& ); //Not implemented.
};


#endif
