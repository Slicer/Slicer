// .NAME vtkSlicerSliceGUI 
// .SECTION Description
// Main Slice GUI for slicer3.  Points to the SliceLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.

#ifndef __vtkSlicerSliceGUI_h
#define __vtkSlicerSliceGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerModuleGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerSliceWidgetCollection.h"
#include "vtkSlicerSliceLogicCollection.h"

class vtkObject;
class vtkSlicerSliceWidget;
class vtkKWFrame;

// Description:
// This class implements Slicer's Slice GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceGUI : public vtkSlicerModuleGUI
{
 public:
    static vtkSlicerSliceGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerSliceGUI, vtkSlicerModuleGUI );
    void PrintSelf (ostream& os, vtkIndent indent);

    // Description:
    // Get/Set methods for class
    vtkGetObjectMacro ( SliceWidgetCollection, vtkSlicerSliceWidgetCollection );
    vtkGetObjectMacro ( SliceLogicCollection, vtkSlicerSliceLogicCollection );
    vtkGetObjectMacro ( MainSlice0, vtkSlicerSliceWidget );
    vtkGetObjectMacro ( MainSlice1, vtkSlicerSliceWidget );
    vtkGetObjectMacro ( MainSlice2, vtkSlicerSliceWidget );
    
    // Description:
    // Adds a new Slice Widget to the Slice Widget Collection.
    virtual void AddSliceWidget ( vtkSlicerSliceWidget *w );
    // Description:
    // Returns a pointer to a Slice Widget contained in the SliceWidgetCollection
    // Given its number (0,1,2...) or, if it's one of the first three Slice widgets
    // given its color (r, y, g, or R, Y, G).
    vtkSlicerSliceWidget* GetSliceWidget ( int SliceWidgetNum );
    vtkSlicerSliceWidget* GetSliceWidget ( char *SliceWidgetColor );

    // Description:
    // Sets the logic pointer and adds observers for a specified Slice Widget.
    virtual void SetSliceLogic ( vtkSlicerSliceLogic *l , int SliceWidgetNum);
    // Description:
    // Adds a SliceWidget's logic pointer to the SliceLogic collection.
    virtual void AddSliceLogic ( vtkSlicerSliceLogic *l );
    // Description:
    // Returns a pointer to a Slice Logic contained in the SliceLogicCollection
    // Given its widget's number (0,1,2...) or, if it's belongs to one of the first
    // three Slice widgets, given its color (r, y, g, or R, Y, G).
    vtkSlicerSliceLogic *GetSliceLogic ( int SliceLogicNum );
    vtkSlicerSliceLogic * GetSliceLogic ( char *SliceWidgetColor );
    
    // Description:
    // Build the three main Slice Widgets and add observers onto them.
    virtual void BuildGUI ( vtkKWFrame *f1, vtkKWFrame *f2, vtkKWFrame *f3);
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );
    
    // Description:
    // Create/Remove observers on various module logic
    virtual void AddLogicObservers ( );
    virtual void RemoveLogicObservers ( );
    // Description:
    // Create/Remove observers on logic for a particular widget
    virtual void AddLogicObservers ( int widgetNumber );
    virtual void RemoveLogicObservers ( int widgetNumber );

    // Description:
    // Create/Remove an observer on a module logic for a specific event
    virtual void AddLogicObserver ( vtkSlicerSliceLogic* logic, int event );
    virtual void RemoveLogicObserver ( vtkSlicerSliceLogic* logic, int event );

    // Description:
    // Create/Remove observers on a mrml node
    virtual void AddMRMLObserver (vtkMRMLNode *node, int event );
    virtual void RemoveMRMLObserver (vtkMRMLNode *node, int event );

    // Description:
    // Create/Remove observers on mrml
    virtual void AddMRMLObservers ( );
    virtual void RemoveMRMLObservers ( );
    
    // Description:
    // Processes all events raised by the logic
    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    // Description:
    // Processes all events raised by the GUI
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    // Description:
    // Processes all events raised by MRML
    virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    // Description:
    // Functions that define and undefine module-specific behaviors.
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerSliceGUI ( );
    ~vtkSlicerSliceGUI ( );

    // Description:
    // Three slice widgets by default.
    vtkSlicerSliceWidget *MainSlice0;
    vtkSlicerSliceWidget *MainSlice1;
    vtkSlicerSliceWidget *MainSlice2;

    // Description:
    // Collection of the above and other slice widgets.
    // Slice widgets and their associated slice logics
    // must have the same ordinal number
    // in the SliceWidgetCollection and SliceLogicCollection.
    vtkSlicerSliceWidgetCollection *SliceWidgetCollection;
    // Description:
    // Collection of slice logic pointers for all slice widgets.
    vtkSlicerSliceLogicCollection *SliceLogicCollection;
    
 private:
    vtkSlicerSliceGUI ( const vtkSlicerSliceGUI& ); // Not implemented.
    void operator = ( const vtkSlicerSliceGUI& ); //Not implemented.
}; 

#endif
