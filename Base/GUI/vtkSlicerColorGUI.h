// .NAME vtkSlicerColorGUI 
// .SECTION Description
// Main Color GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerColorGUI_h
#define __vtkSlicerColorGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerColorLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"

// Description:
// This class implements Slicer's Color GUI
//
class vtkSlicerColorDisplayWidget;
class vtkKWMessage;
class vtkKWPushButton;
class vtkKWLoadSaveButtonWithLabel;
class vtkKWLoadSaveButton;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerColorGUI : public vtkSlicerModuleGUI
{
public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerColorGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerColorGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( Logic, vtkSlicerColorLogic);

    // Description:
    // API for setting Logic and
    // for both setting and observing them.        
    void SetModuleLogic ( vtkSlicerColorLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkSlicerColorLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the Color module GUI
    virtual void BuildGUI ( ) ;

    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // This method releases references and key-bindings,
    // and optionally removes observers.
    virtual void TearDownGUI ( );

    // Description:
    // Methods for adding module-specific key bindings and
    // removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    // Description:
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

    // Description:
    // Get methods on class members (no Set methods required)
    vtkGetObjectMacro ( LoadColorFileButton, vtkKWLoadSaveButtonWithLabel );
    
protected:
    vtkSlicerColorGUI ( );
    virtual ~vtkSlicerColorGUI ( );

    // Module logic and mrml pointers
    vtkSlicerColorLogic *Logic;
    
    // Description:
    // the widget that displays the colour node
    vtkSlicerColorDisplayWidget *ColorDisplayWidget;
    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;

    vtkKWLoadSaveButtonWithLabel *LoadColorFileButton;
    
private:
    vtkSlicerColorGUI ( const vtkSlicerColorGUI& ); // Not implemented.
    void operator = ( const vtkSlicerColorGUI& ); //Not implemented.
};


#endif
