// .NAME vtkSlicerDataGUI
// .SECTION Description
// Main Data GUI and mediator methods for slicer3.

#ifndef __vtkSlicerDataGUI_h
#define __vtkSlicerDataGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerMRMLTreeWidget.h"
#include "vtkSlicerSceneSnapshotWidget.h"
#include "vtkSlicerRecordSnapshotWidget.h"
#include "vtkKWLabel.h"

//#include "vtkSlicerDataLogic.h"

// Description:
// This class implements Slicer's Data GUI
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerDataGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerDataGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerDataGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members (no Set methods required.)
    //vtkGetObjectMacro ( Logic, vtkSlicerDataLogic);
    //vtkGetObjectMacro ( MRMLNode, vtkMRMLNode );

    // Description:
    // API for setting MRMLNode, Logic and
    // for both setting and observing them.
    /*
      // classes not yet defined!
    void SetMRMLNode ( vtkMRMLNode *node )
        { this->SetMRML ( vtkObjectPointer( &this->MRMLNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLNode *node )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLNode), node ); }

    void SetModuleLogic ( vtkSlicerDataLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ) }
    void SetAndObserveModuleLogic ( vtkSlicerDataLogic *logic )
        { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ) }
    */
    
    // Description:
    // Methods for adding module-specific key bindings and
    // removing them.
    virtual void CreateModuleEventBindings ( );
    virtual void ReleaseModuleEventBindings ( );

    // Description:
    // This method builds the Data module's GUI
    virtual void BuildGUI ( ) ;

    // Descripgion:
    // This method releases references and key-bindings,
    // and optionally removes observers.
    virtual void TearDownGUI ( );
    
    // Description:
    // Add/Remove observers on widgets in the GUI
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    // Description:
    // Class's mediator methods for processing events invoked by
    // either the Logic, MRML or GUI.
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
    
    // Description:
    // Describe the behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerDataGUI ( );
    virtual ~vtkSlicerDataGUI ( );

    // Logic pointer for the Data module
    //vtkSlicerDataLogic *Logic;
    //vtkMRMLNode *MRMLNode;
    
    // Widgets for the Data module
  vtkSlicerMRMLTreeWidget *MRMLTreeWidget;
  vtkSlicerSceneSnapshotWidget *SceneSnapshotWidget;
  vtkSlicerRecordSnapshotWidget *RecordSnapshotWidget;
  vtkKWLabel *NACLabel;
  vtkKWLabel *NAMICLabel;
  vtkKWLabel *NCIGTLabel;
  vtkKWLabel *BIRNLabel;
  
 private:
    vtkSlicerDataGUI ( const vtkSlicerDataGUI& ); // Not implemented.
    void operator = ( const vtkSlicerDataGUI& ); //Not implemented.
};


#endif
