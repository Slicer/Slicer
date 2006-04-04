// .NAME vtkSlicerModelsGUI 
// .SECTION Description
// Main Models GUI for slicer3.  Points to the ModelsLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.

#ifndef __vtkSlicerModelsGUI_h
#define __vtkSlicerModelsGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
//#include "vtkSlicerModelsLogic.h"
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

    //vtkGetObjectMacro ( ModelsLogic, vtkSlicerModelsLogic);
    //vtkSetObjectMacro ( ModelsLogic, vtkSlicerModelsLogic);
    vtkGetObjectMacro ( LoadModelButton, vtkKWLoadSaveButton );

    // Description:
    // This method builds a Slice GUI
    virtual void BuildGUI ( ) ;
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void RemoveLogicObservers ( );
    virtual void AddMrmlObservers ( );
    virtual void RemoveMrmlObservers ( );
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessMrmlEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );

 protected:
    vtkSlicerModelsGUI ( );
    ~vtkSlicerModelsGUI ( );

    // Widgets for the Models module
    //vtkSlicerModelsLogic *ModelsLogic;
    vtkKWLoadSaveButton *LoadModelButton;

 private:
    vtkSlicerModelsGUI ( const vtkSlicerModelsGUI& ); // Not implemented.
    void operator = ( const vtkSlicerModelsGUI& ); //Not implemented.
};


#endif
