// .NAME vtkSlicerTractographyDisplayGUI 
// .SECTION Description
// Main Tractography GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerTractographyDisplayGUI_h
#define __vtkSlicerTractographyDisplayGUI_h

#include "vtkSlicerTractographyDisplayWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
//#include "vtkSlicerTractographyLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"

//#include "vtkMRMLModelNode.h"

#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWFrame.h"

// Description:
// This class implements Slicer's Tractography GUI
//

class VTK_SLICERTRACTOGRAPHYDISPLAY_EXPORT vtkSlicerTractographyDisplayGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkSlicerTractographyDisplayGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerTractographyDisplayGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( LoadTractographyButton, vtkKWLoadSaveButton );
    //vtkGetObjectMacro ( Logic, vtkSlicerTractographyLogic);
    //vtkGetObjectMacro ( FiberBundleNode, vtkMRMLFiberBundleNode );
    
    // Description:
    // API for setting FiberBundleNode, Logic and
    // for both setting and observing them.
    /*
      // classes not yet defined.
    void SetMRMLNode ( vtkMRMLFiberBundleNode *node )
        { this->SetMRML ( vtkObjectPointer( &this->MRMLFiberBundleNode), node ); }
    void SetAndObserveMRMLNode ( vtkMRMLFiberBundleNode *node )
        { this->SetAndObserveMRML ( vtkObjectPointer( &this->MRMLFiberBundleNode), node ); }

    void SetModuleLogic ( vtkSlicerTractographyLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ) }
    void SetAndObserveModuleLogic ( vtkSlicerTractographyLogic *logic )
        { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ) }
    */
    /*
    void SetModuleLogic ( vtkSlicerTractographyLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkSlicerTractographyLogic *logic )
        { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    vtkGetObjectMacro( Logic, vtkSlicerTractographyLogic );
    */

    // Description:
    // This method builds the Tractography module GUI
    virtual void BuildGUI ( ) ;

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
    // Methods describe behavior at module enter and exit.
    virtual void Enter ( );
    virtual void Exit ( );

 protected:
    vtkSlicerTractographyDisplayGUI ( );
    virtual ~vtkSlicerTractographyDisplayGUI ( );

    // Module logic and mrml pointers (classes not yet built!)
    //vtkSlicerTractographyLogic *Logic;
    //vtkMRMLFiberBundleNode *FiberBundleNode;
    
    // Widgets for the Tractography module
    vtkKWLoadSaveButton *LoadTractographyButton;
    vtkKWLoadSaveButton *LoadTractographyDirectoryButton;
    vtkKWLoadSaveButton *SaveTractographyButton;
    vtkSlicerNodeSelectorWidget* FiberBundleSelectorWidget;

    //vtkSlicerFiberBundleDisplayWidget *FiberBundleDisplayWidget;

    //vtkSlicerClipTractographyWidget *ClipTractographyWidget;

 private:
    vtkSlicerTractographyDisplayGUI ( const vtkSlicerTractographyDisplayGUI& ); // Not implemented.
    void operator = ( const vtkSlicerTractographyDisplayGUI& ); //Not implemented.
};


#endif
