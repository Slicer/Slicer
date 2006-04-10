// .NAME vtkSlicerVolumesGUI 
// .SECTION Description
// Main Volumes GUI for slicer3.  Points to the VolumeLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.


#ifndef __vtkSlicerVolumesGUI_h
#define __vtkSlicerVolumesGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
//#include "vtkSlicerVolumesLogic.h"
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
    
    //vtkGetObjectMacro ( VolumesLogic, vtkSlicerVolumesLogic);
    //vtkSetObjectMacro ( VolumesLogic, vtkSlicerVolumesLogic);

    // Description:
    // This method builds the Volumes module GUI
    virtual void BuildGUI ( );
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void RemoveLogicObservers ( );
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void Enter ( );
    virtual void Exit ( );

 protected:
    vtkSlicerVolumesGUI ( );
    ~vtkSlicerVolumesGUI ( );

    // Widgets for the Volumes module
    //vtkSlicerVolumesLogic *VolumesLogic;
    vtkKWLoadSaveButton *LoadVolumeButton;

 private:
    vtkSlicerVolumesGUI ( const vtkSlicerVolumesGUI& ); // Not implemented.
    void operator = ( const vtkSlicerVolumesGUI& ); //Not implemented.
};

#endif
