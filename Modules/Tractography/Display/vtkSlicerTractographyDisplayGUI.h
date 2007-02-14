// .NAME vtkSlicerTractographyDisplayGUI 
// .SECTION Description
// Main Tractography GUI and mediator methods for slicer3. 


#ifndef __vtkSlicerTractographyDisplayGUI_h
#define __vtkSlicerTractographyDisplayGUI_h

#include "vtkSlicerTractographyDisplay.h"
#include "vtkSlicerModuleGUI.h"

#include "vtkSlicerFiberBundleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerFiberBundleDisplayWidget.h"
#include "vtkSlicerAllFiberBundlesDisplayWidget.h"

#include "vtkKWLoadSaveButton.h"
#include "vtkKWLoadSaveDialog.h"
#include "vtkKWFrame.h"

// Description:
// This class implements Slicer's Tractography display GUI
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
    // Get the categorization of the module.
    const char *GetCategory() const
        { return "Tractography"; }
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( LoadTractographyButton, vtkKWLoadSaveButton );

    // Description:
    // Get logic object for this GUI  (handles load/save of fiber bundles, manages bundles)
    vtkGetObjectMacro ( Logic, vtkSlicerFiberBundleLogic);

    // Description:
    // Set logic object for this GUI (handles load/save of fiber bundles, manages bundles)
    vtkSetObjectMacro ( Logic, vtkSlicerFiberBundleLogic);

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

    // Module logic and mrml pointers 
    vtkSlicerFiberBundleLogic *Logic;
    
    // Widgets for the Tractography module
    vtkKWLoadSaveButton *LoadTractographyButton;
    vtkKWLoadSaveButton *LoadTractographyDirectoryButton;
    vtkKWLoadSaveButton *SaveTractographyButton;
    vtkSlicerNodeSelectorWidget* FiberBundleSelectorWidget;

    vtkSlicerAllFiberBundlesDisplayWidget *AllFiberBundlesDisplayWidget;
  
    vtkSlicerFiberBundleDisplayWidget *FiberBundleDisplayWidget;

 private:
    vtkSlicerTractographyDisplayGUI ( const vtkSlicerTractographyDisplayGUI& ); // Not implemented.
    void operator = ( const vtkSlicerTractographyDisplayGUI& ); //Not implemented.
};


#endif
