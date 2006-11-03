// .NAME vtkSlicerCamerasGUI 
// .SECTION Description
// Main Camera GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerCamerasGUI_h
#define __vtkSlicerCamerasGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"


#include "vtkMRMLCameraNode.h"

#include "vtkSlicerNodeSelectorWidget.h"

class vtkKWCheckButton;

// Description:
// This class implements Slicer's Camera GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerCamerasGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerCamerasGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerCamerasGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    // Description:
    // This method builds the Camera module GUI
    virtual void BuildGUI ( );

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );


    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    // Description:
    // Describe behavior at module startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    void UpdateCameraSelector();

 protected:
    vtkSlicerCamerasGUI ( );
    virtual ~vtkSlicerCamerasGUI ( );
    
    vtkKWCheckButton *ActiveCheckButton;
    vtkSlicerNodeSelectorWidget *CameraSelectorWidget;

 private:
    vtkSlicerCamerasGUI ( const vtkSlicerCamerasGUI& ); // Not implemented.
    void operator = ( const vtkSlicerCamerasGUI& ); //Not implemented.
};

#endif
