// .NAME vtkSlicerModelsGUI 
// .SECTION Description
// Main Models GUI for slicer3.  Points to the ModelsLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.

#ifndef __vtkSlicerModelsGUI_h
#define __vtkSlicerModelsGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerModelsLogic.h"
#include "vtkMRMLNode.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWFrame.h"

// Description:
// This class implements Slicer's Models GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModelsGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerModelsGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerModelsGUI, vtkSlicerModuleGUI );
    vtkGetObjectMacro ( LoadModelButton, vtkKWLoadSaveButton );
    //vtkGetObjectMacro ( Logic, vtkSlicerModelsLogic);

    // Description:
    // Sets pointer to the module logic and adds observers
    //virtual void SetLogic ( vtkSlicerModelsLogic *logic );


    // Description:
    // This method builds the Models module GUI
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
    vtkSlicerModelsGUI ( );
    ~vtkSlicerModelsGUI ( );

    // Module logic
    //vtkSlicerModelsLogic *Logic;
    
    // Widgets for the Models module
    vtkKWLoadSaveButton *LoadModelButton;

 private:
    vtkSlicerModelsGUI ( const vtkSlicerModelsGUI& ); // Not implemented.
    void operator = ( const vtkSlicerModelsGUI& ); //Not implemented.
};


#endif
