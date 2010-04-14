///  vtkSlicerColorGUI 
/// 
/// Main Color GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerColorGUI_h
#define __vtkSlicerColorGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerColorLogic.h"

#include "vtkKWFrame.h"
#include "vtkKWLabel.h"

/// Description:
/// This class implements Slicer's Color GUI
//
class vtkSlicerColorDisplayWidget;
class vtkSlicerColorEditWidget;
class vtkKWMessage;
class vtkKWPushButton;
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerColorGUI : public vtkSlicerModuleGUI
{
public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerColorGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerColorGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    /// 
    /// Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( Logic, vtkSlicerColorLogic);

    /// 
    /// API for setting Logic and
    /// for both setting and observing them.        
    void SetModuleLogic ( vtkSlicerColorLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    //BTX
    using vtkSlicerModuleGUI::SetModuleLogic;
    //ETX
    void SetAndObserveModuleLogic ( vtkSlicerColorLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    /// 
    /// This method builds the Color module GUI
    virtual void BuildGUI ( void );
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

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
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    /// 
    /// Methods describe behavior at module enter and exit.
    virtual void Enter ( vtkMRMLNode *node );
    virtual void Enter ( ) { this->Enter(NULL); };
    virtual void Exit ( );

    /// 
    /// Add Slicer mrml colour table nodes to the application's color picker dialog as swatches. Returns 1 on failure, 0 on success.
    int AddLUTsToColorDialog();

    /// 
    /// the widget that displays info about the colour node
    vtkGetObjectMacro(ColorDisplayWidget, vtkSlicerColorDisplayWidget);

    ///
    /// update the viewer widget on the sub widgets
    void SetActiveViewer(vtkSlicerViewerWidget *activeViewer );

protected:
    vtkSlicerColorGUI ( );
    virtual ~vtkSlicerColorGUI ( );

    /// Module logic and mrml pointers
    vtkSlicerColorLogic *Logic;
    
    /// 
    /// the widget that displays the colour node
    vtkSlicerColorDisplayWidget *ColorDisplayWidget;

    /// 
    /// the widget that allows the user to edit a color node
    vtkSlicerColorEditWidget *ColorEditWidget;

    /// 
    /// Acknowledgement icons
    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;
    
private:
    vtkSlicerColorGUI ( const vtkSlicerColorGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerColorGUI& ); //Not implemented.
};


#endif
