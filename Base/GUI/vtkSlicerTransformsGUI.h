// .NAME vtkSlicerTransformsGUI 
// .SECTION Description
// Main Volumes GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerTransformsGUI_h
#define __vtkSlicerTransformsGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkSlicerTransformManagerWidget.h"
#include "vtkSlicerTransformEditorWidget.h"

#include "vtkKWFrame.h"

// Description:
// This class implements Slicer's Volumes GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerTransformsGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerTransformsGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerTransformsGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    // Description:
    // This method builds the Volumes module GUI
    virtual void BuildGUI ( );

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    // Description:
    // Describe behavior at module startup and exit.
    virtual void Enter ( );
    virtual void Exit ( );

 protected:
    vtkSlicerTransformsGUI ( );
    ~vtkSlicerTransformsGUI ( );
    
    // Widgets for the Volumes module
    vtkSlicerTransformManagerWidget *TransformManagerWidget;
    vtkSlicerTransformEditorWidget *TransformEditorWidget;

 private:
    vtkSlicerTransformsGUI ( const vtkSlicerTransformsGUI& ); // Not implemented.
    void operator = ( const vtkSlicerTransformsGUI& ); //Not implemented.
};

#endif
