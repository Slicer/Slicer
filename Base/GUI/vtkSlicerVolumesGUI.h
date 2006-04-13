// .NAME vtkSlicerVolumesGUI 
// .SECTION Description
// Main Volumes GUI for slicer3.  Points to the VolumeLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.


#ifndef __vtkSlicerVolumesGUI_h
#define __vtkSlicerVolumesGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerVolumesLogic.h"
#include "vtkMRMLNode.h"
#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWFrame.h"

// Description:
// This class implements Slicer's Volumes GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerVolumesGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerVolumesGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerVolumesGUI, vtkSlicerModuleGUI );
    vtkGetObjectMacro ( LoadVolumeButton, vtkKWLoadSaveButton );
    //vtkGetObjectMacro ( Logic, vtkSlicerVolumesLogic);

    // Description:
    // Sets pointer to the module logic and adds observers.    
    //virtual void SetLogic ( vtkSlicerVolumesLogic *logic );

    // Description:
    // This method builds the Volumes module GUI
    virtual void BuildGUI ( );
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
    vtkSlicerVolumesGUI ( );
    ~vtkSlicerVolumesGUI ( );

    // Module logic
    //vtkSlicerVolumesLogic *Logic;

    // Widgets for the Volumes module
    vtkKWLoadSaveButton *LoadVolumeButton;

 private:
    vtkSlicerVolumesGUI ( const vtkSlicerVolumesGUI& ); // Not implemented.
    void operator = ( const vtkSlicerVolumesGUI& ); //Not implemented.
};

#endif
