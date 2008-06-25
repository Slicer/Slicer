// .NAME vtkSlicerSliceGUI 
// .SECTION Description
// Individual Slice GUI and mediator functions for slicer3.
// Contains a SliceViewer, a Slice Controller, a pointer to
// SliceLogic and a pointer to a MRMLSliceNode.


#ifndef __vtkSlicerSliceGUI_h
#define __vtkSlicerSliceGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerSliceViewer.h"
#include "vtkSlicerSliceLogic.h"

#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"

#include "vtkImageMapper.h"
#include "vtkKWFrame.h"

class vtkObject;
class vtkKWFrame;
class vtkSlicerSliceControllerWidget;

// Description:
// This class implements Slicer's Slice GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceGUI : public vtkSlicerComponentGUI
{
 public:
    static vtkSlicerSliceGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerSliceGUI, vtkSlicerComponentGUI );
    void PrintSelf (ostream& os, vtkIndent indent);

    // Description:
    // Get methods for three default SliceGUIs
    // (Each SliceGUI contains a SliceViewerWidget,
    // SliceControllerWidget, a SliceLogic pointer and
    // a SliceNode pointer.)
    vtkGetObjectMacro ( SliceGUIFrame, vtkKWFrame );
    vtkGetObjectMacro ( SliceViewer, vtkSlicerSliceViewer );
    vtkGetObjectMacro ( SliceController, vtkSlicerSliceControllerWidget );
    vtkGetMacro ( ControllerStyle, int );
    vtkSetMacro ( ControllerStyle, int );
    vtkGetObjectMacro ( Logic, vtkSlicerSliceLogic );
    vtkGetObjectMacro ( SliceNode, vtkMRMLSliceNode );
    
    // Description:
    // API for setting SliceNode, SliceLogic and
    // for both setting and observing them.
    void SetMRMLNode ( vtkMRMLSliceNode *node )
        { vtkSetMRMLNodeMacro ( this->SliceNode, node ); }
    void SetAndObserveMRMLNode ( vtkMRMLSliceNode *node )
        { vtkSetAndObserveMRMLNodeMacro (this->SliceNode, node ); }

    void SetModuleLogic ( vtkSlicerSliceLogic *logic )
        { 
        this->SetLogic ( vtkObjectPointer (&this->Logic), logic ); 
        this->SetupViewerAndController();
        }

    void SetAndObserveModuleLogic ( vtkSlicerSliceLogic *logic )
        { 
        this->SetAndObserveLogic ( vtkObjectPointer (&this->Logic), logic ); 
        this->SetupViewerAndController();
        }

  void SetupViewerAndController();

    // Description:
    // Build the SlicesGUI's UIPanel and three main SliceGUIs 
    virtual void BuildGUI ( vtkKWFrame *f );
    virtual void BuildGUI ( vtkKWFrame *f, double *bgColor );

    // Description:
    // Show and hide the GUI, pack in frame given by f
    virtual void PackGUI ( vtkKWFrame *f );
    virtual void UnpackGUI ( );
    virtual void GridGUI ( vtkKWFrame *f, int row, int col);
    virtual void UngridGUI ( );
        virtual void GridSpanGUI( vtkKWFrame *f, int row, int col, int rowspan, int colspan);

    // Description:
    // Add/Remove Observers on UIPanel widgets and SliceGUIs.
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );

    //virtual void RemoveMRMLObservers ( );
    
    // Description:
    // Processes all events raised by the logic
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    // Description:
    // Processes all events raised by the GUI
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    // Description:
    // Processes all events raised by MRML
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    virtual void RemoveMRMLObservers();
    
    // Description:
    // Utility for accessing the event that is most recently triggered on the GUI
    vtkGetStringMacro ( CurrentGUIEvent );
    vtkSetStringMacro ( CurrentGUIEvent );
    
    // Description:
    // Allow scripts using to 'swallow' events (actual callback command
    // isn't correctly wrapped in vtk to allow access)
    void SetGUICommandAbortFlag ( int flag );

    // Description:
    // Flag saying that one of the observers wants to grab the events.
    // This doesn't prevent other observers from seeing the events, but they
    // can check the flag and decide not to process the event if some other
    // observer has grabbed it.  Observers can use the value of this flag to
    // do state mangement (the observer is responsible for creating a unique id)
    vtkGetStringMacro ( GrabID );
    vtkSetStringMacro ( GrabID );

    // Description:
    // Functions that define and undefine module-specific behaviors.
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerSliceGUI ( );
    virtual ~vtkSlicerSliceGUI ( );

    // Description:
    // A Slice Viewer, a slice Controller and a containing frame
    vtkSlicerSliceViewer *SliceViewer;
    vtkSlicerSliceControllerWidget *SliceController;
    vtkKWFrame *SliceGUIFrame;

    int ControllerStyle;
    vtkSlicerSliceLogic *Logic;
    vtkMRMLSliceNode *SliceNode;

    char *CurrentGUIEvent;
    char *GrabID;

//BTX
  std::vector<vtkMRMLDisplayNode *> DisplayNodes;
//ETX

 private:
    vtkSlicerSliceGUI ( const vtkSlicerSliceGUI& ); // Not implemented.
    void operator = ( const vtkSlicerSliceGUI& ); //Not implemented.
}; 

#endif
