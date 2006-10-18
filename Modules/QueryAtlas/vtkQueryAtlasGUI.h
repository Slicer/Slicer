// .NAME vtkQueryAtlasGUI
// .SECTION Description
// GUI for the Query Atlas


#ifndef __vtkQueryAtlasGUI_h
#define __vtkQueryAtlasGUI_h

#include "vtkQueryAtlasWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerModuleLogic.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkQueryAtlasLogic.h"

// Description:
// This class implements Slicer's QueryAtlas GUI
//

class VTK_QUERYATLAS_EXPORT vtkQueryAtlasGUI : public vtkSlicerModuleGUI
{
 public:
    // Description:
    // Usual vtk class functions
    static vtkQueryAtlasGUI* New (  );
    vtkTypeRevisionMacro ( vtkQueryAtlasGUI, vtkSlicerModuleGUI );
    void PrintSelf ( ostream& os, vtkIndent indent );
    
    // Description:
    // Get methods on class members ( no Set methods required. )
    vtkGetObjectMacro ( Logic, vtkQueryAtlasLogic);
    
    void SetModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); }
    void SetAndObserveModuleLogic ( vtkQueryAtlasLogic *logic )
    { this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); }

    // Description:
    // This method builds the QueryAtlas module GUI
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
    vtkQueryAtlasGUI ( );
    virtual ~vtkQueryAtlasGUI ( );

    // Module logic and mrml pointers
    vtkQueryAtlasLogic *Logic;


private:
    vtkQueryAtlasGUI ( const vtkQueryAtlasGUI& ); // Not implemented.
    void operator = ( const vtkQueryAtlasGUI& ); //Not implemented.
};


#endif
