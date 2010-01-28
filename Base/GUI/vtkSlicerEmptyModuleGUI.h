#ifndef __vtkSlicerEmptyModuleGUI_h
#define __vtkSlicerEmptyModuleGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"

/// Description:
/// This class implements an Empty module used to obtain geometry of the module panel
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerEmptyModuleGUI : public vtkSlicerModuleGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerEmptyModuleGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerEmptyModuleGUI, vtkSlicerModuleGUI );
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
    vtkSlicerEmptyModuleGUI ( );
    virtual ~vtkSlicerEmptyModuleGUI ( );

 private:
    vtkSlicerEmptyModuleGUI ( const vtkSlicerEmptyModuleGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerEmptyModuleGUI& ); //Not implemented.
};

#endif
