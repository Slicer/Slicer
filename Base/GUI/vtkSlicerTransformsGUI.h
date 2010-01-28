///  vtkSlicerTransformsGUI 
/// 
/// Main Volumes GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerTransformsGUI_h
#define __vtkSlicerTransformsGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkSlicerTransformManagerWidget.h"
#include "vtkSlicerTransformEditorWidget.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"

class vtkKWLoadSaveButton;

/// Description:
/// This class implements Slicer's Volumes GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerTransformsGUI : public vtkSlicerModuleGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerTransformsGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerTransformsGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent );
    
    /// 
    /// This method builds the Volumes module GUI
    virtual void BuildGUI ( );
    //BTX
    using vtkSlicerComponentGUI::BuildGUI; 
    //ETX

    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

       /// 
    /// This method releases references and key-bindings,
    /// and optionally removes observers.
    virtual void TearDownGUI ( );

        /// 
    /// Methods for adding module-specific key bindings and
    /// removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );
    
    /// 
    /// Class's mediator methods for processing events invoked by
    /// either the Logic, MRML or GUI.
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// 
    /// Describe behavior at module startup and exit.
    virtual void Enter ( vtkMRMLNode *node );
    virtual void Enter ( ) { this->Enter(NULL); };
    virtual void Exit ( );

 protected:
    vtkSlicerTransformsGUI ( );
    virtual ~vtkSlicerTransformsGUI ( );
    
    /// Widgets for the Volumes module
    vtkSlicerTransformEditorWidget *TransformEditorWidget;
    vtkKWLoadSaveButton *LoadButton;
    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;

 private:
    vtkSlicerTransformsGUI ( const vtkSlicerTransformsGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerTransformsGUI& ); //Not implemented.
};

#endif
