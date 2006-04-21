// .NAME vtkSlicerSlicesGUI 
// .SECTION Description
// Main Slice GUI and mediator functions for slicer3.  
// Manages a collection of individual sliceGUIs, each of which
// contain a SliceViewer a SliceController, a pointer to
// SliceLogic and a pointer to a MRMLSliceNode.

#ifndef __vtkSlicerSlicesGUI_h
#define __vtkSlicerSlicesGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerSliceGUICollection.h"
#include "vtkMRMLSliceNode.h"
#include "vtkSlicerSliceLogic.h"

class vtkObject;
class vtkKWFrame;

// Description:
// This class implements Slicer's Slice GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSlicesGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerSlicesGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerSlicesGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent);

    // Description:
    // Get/Set methods for class
    vtkGetObjectMacro ( SliceGUICollection, vtkSlicerSliceGUICollection );

    // Description:
    // Get methods for three default SliceGUIs
    // (Each SliceGUI contains a SliceViewerWidget,
    // SliceControllerWidget, a SliceLogic pointer and
    // a SliceNode pointer.)
    vtkGetObjectMacro ( MainSliceGUI0, vtkSlicerOneSliceGUI );
    vtkGetObjectMacro ( MainSliceGUI1, vtkSlicerOneSliceGUI );
    vtkGetObjectMacro ( MainSliceGUI2, vtkSlicerOneSliceGUI );
    
    // Description:
    // API for setting a SliceNode or SliceLogic in one of the sliceGUIs
    // in the SliceGUICollection.
    void SetMRMLNode ( int sliceGUINum, vtkMRMLSliceNode *node)
        {
            vtkSlicerOneSliceGUI *g;
            g = vtkSlicerOneSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject (sliceGUINum ));
            g->SetMRMLNode ( node );
        }
    void SetAndObserveMRMLNode ( int sliceGUINum, vtkMRMLSliceNode *node )
        {
            vtkSlicerOneSliceGUI *g;
            g = vtkSlicerOneSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject (sliceGUINum ));
            g->SetAndObserveMRMLNode ( node );
        }
    void SetModuleLogic ( int sliceGUINum, vtkSlicerSliceLogic *logic )
        {
            vtkSlicerOneSliceGUI *g;
            g = vtkSlicerOneSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject (sliceGUINum ));
            g->SetModuleLogic ( logic );
        }
    void SetAndObserveModuleLogic ( int sliceGUINum, vtkSlicerSliceLogic *logic )
        {
            vtkSlicerOneSliceGUI *g;
            g = vtkSlicerOneSliceGUI::SafeDownCast(this->SliceGUICollection->GetItemAsObject (sliceGUINum ));
            g->SetAndObserveModuleLogic ( logic );
        }

    // Description:
    // Add/Remove a SliceGUI to the SliceGUI Collection.
    virtual void AddSliceGUI ( vtkSlicerOneSliceGUI *s );
    virtual void AddAndObserveSliceGUI ( vtkSlicerOneSliceGUI *s );
    virtual void RemoveSliceGUI (vtkSlicerOneSliceGUI *s );
    
    // Description:
    // Returns a pointer to a Slice GUI contained in the SliceGUICollection
    // Given its number (0,1,2...) or, if it's one of the first three Slice GUIs
    // given its color (r, y, g, or R, Y, G).
    vtkSlicerOneSliceGUI* GetSliceGUI ( int SliceGUINum );
    vtkSlicerOneSliceGUI* GetSliceGUI ( char *SliceGUIColor );

    // Description:
    // Build the SlicesGUI's UIPanel and three main SliceGUIs 
    virtual void BuildGUI ( vtkKWFrame *f1, vtkKWFrame *f2, vtkKWFrame *f3);

    // Description:
    // Add/Remove Observers on UIPanel widgets and SliceGUIs.
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );
    
    // Description:
    // Processes all events raised by the logic
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );
    // Description:
    // Processes all events raised by the GUI
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event, void *callData );
    // Description:
    // Processes all events raised by MRML
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

    // Description:
    // Functions that define and undefine module-specific behaviors.
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerSlicesGUI ( );
    ~vtkSlicerSlicesGUI ( );

    // Description:
    // Three slice widgets by default.
    vtkSlicerOneSliceGUI *MainSliceGUI0;
    vtkSlicerOneSliceGUI *MainSliceGUI1;
    vtkSlicerOneSliceGUI *MainSliceGUI2;

    // Description:
    // Collection of the above and other slice GUIs
    vtkSlicerSliceGUICollection *SliceGUICollection;

 private:
    vtkSlicerSlicesGUI ( const vtkSlicerSlicesGUI& ); // Not implemented.
    void operator = ( const vtkSlicerSlicesGUI& ); //Not implemented.
}; 

#endif
