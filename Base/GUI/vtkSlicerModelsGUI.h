///  vtkSlicerModelsGUI 
/// 
/// Main Models GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerModelsGUI_h
#define __vtkSlicerModelsGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerModelsLogic.h"
#include "vtkSlicerModelHierarchyLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerClipModelsWidget.h"

#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWLoadSaveButtonWithLabel.h"
#include "vtkKWFrame.h"
#include "vtkKWLabel.h"

/// Description:
/// This class implements Slicer's Models GUI
//
class vtkSlicerModelDisplayWidget;
class vtkSlicerModelHierarchyWidget;
class vtkSlicerModelInfoWidget;
class vtkSlicerModuleCollapsibleFrame;
//class vtkSlicerViewerWidget;
//class vtkSlicerViewerInteractorStyle;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerModelsGUI : public vtkSlicerModuleGUI
{
 public:
    /// 
    /// Usual vtk class functions
    static vtkSlicerModelsGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerModelsGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    /// 
    /// Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( LoadModelButton, vtkKWPushButton );
    vtkGetObjectMacro ( LoadScalarsButton, vtkKWPushButton );
    //vtkGetObjectMacro ( Logic, vtkSlicerModelsLogic);
    //vtkGetObjectMacro ( ModelNode, vtkMRMLModelNode );
    
    /// 
    /// API for setting ModelNode, Logic and
    /// for both setting and observing them.
    /*
      /// classes not yet defined.
    void SetMRMLNode ( vtkMRMLModelNode *node )
        { this->SetMRML ( vtkObjectPointer( &this->MRMLModelNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLModelNode *node )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLModelNode), node ); }

    void SetModuleLogic ( vtkSlicerModelsLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ) }
    void SetAndObserveModuleLogic ( vtkSlicerModelsLogic *logic )
        { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ) }
    */

    void SetModuleLogic ( vtkSlicerModelsLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    //BTX
    using vtkSlicerModuleGUI::SetModuleLogic; 
    //ETX
    void SetAndObserveModuleLogic ( vtkSlicerModelsLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    vtkGetObjectMacro( Logic, vtkSlicerModelsLogic );

    vtkGetObjectMacro( ModelHierarchyLogic, vtkSlicerModelHierarchyLogic );
    vtkSetObjectMacro( ModelHierarchyLogic, vtkSlicerModelHierarchyLogic );
    
    

    /// 
    /// This method builds the Models module GUI
    virtual void BuildGUI ( void );
    virtual void BuildGUI ( vtkKWFrame * f ) { this->Superclass::BuildGUI(f); }
    virtual void BuildGUI ( vtkKWFrame * f, double * bgColor ) { this->Superclass::BuildGUI(f,bgColor); }

    /// Descripgion:
    /// This method releases references and key-bindings,
    /// and optionally removes observers.
    virtual void TearDownGUI ( );

    /// 
    /// Methods for adding module-specific key bindings and
    /// removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );

    /// 
    /// Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

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
    /// Get/Set the main slicer viewer widget, for picking
///    vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
///    virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);
    
    /// 
    /// Get/Set the slicer interactorstyle, for picking
///    vtkGetObjectMacro(InteractorStyle, vtkSlicerViewerInteractorStyle);
///    virtual void SetInteractorStyle(vtkSlicerViewerInteractorStyle *interactorStyle);

    /// 
    /// subclass setting the application gui, so can set the viewer widget and
    /// interactor style
///    virtual void Init ( );

protected:
    vtkSlicerModelsGUI ( );
    virtual ~vtkSlicerModelsGUI ( );

    /// Module logic and mrml pointers (classes not yet built!)
    //vtkSlicerModelsLogic *Logic;
    //vtkMRMLModelNode *ModelNode;
    
    /// Widgets for the Models module
    vtkKWPushButton *LoadModelButton;
    vtkKWPushButton *LoadScalarsButton;
    vtkSlicerNodeSelectorWidget* ModelDisplaySelectorWidget;
    vtkKWLabel *NACLabel;
    vtkKWLabel *NAMICLabel;
    vtkKWLabel *NCIGTLabel;
    vtkKWLabel *BIRNLabel;

    vtkSlicerModelDisplayWidget *ModelDisplayWidget;

    vtkSlicerClipModelsWidget *ClipModelsWidget;

    vtkSlicerModelHierarchyWidget *ModelHierarchyWidget;

    vtkSlicerModelInfoWidget *ModelInfoWidget;

    vtkSlicerModelsLogic *Logic;
    
    vtkSlicerModelHierarchyLogic * ModelHierarchyLogic;

    vtkSlicerModuleCollapsibleFrame *ModelDisplayFrame;

    /// 
    /// A pointer back to the viewer widget, useful for picking
///    vtkSlicerViewerWidget *ViewerWidget;
    
    /// 
    /// A pointer to the interactor style, useful for picking
///    vtkSlicerViewerInteractorStyle *InteractorStyle;

private:
    vtkSlicerModelsGUI ( const vtkSlicerModelsGUI& ); /// Not implemented.
    void operator = ( const vtkSlicerModelsGUI& ); //Not implemented.
};

#endif
