// .NAME vtkSlicerSliceGUI 
// .SECTION Description
// Main Slice GUI for slicer3.  Points to the SliceLogic and
// reflects changes in that logic back onto the UI.  Also routes changes
// from the GUI into the Logic to effect the user's desires.

#ifndef __vtkSlicerSliceGUI_h
#define __vtkSlicerSliceGUI_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkSlicerComponentGUI.h"
#include "vtkSlicerSliceLogic.h"
#include "vtkSlicerSliceWidgetCollection.h"

class vtkObject;
class vtkSlicerSliceWidget;
class vtkKWFrame;

// Description:
// This class implements Slicer's Slice GUI.
//
class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceGUI : public vtkSlicerComponentGUI
{
 public:
    static vtkSlicerSliceGUI* New (  );
    vtkTypeRevisionMacro ( vtkSlicerSliceGUI, vtkSlicerComponentGUI );
    void PrintSelf (ostream& os, vtkIndent indent);

    vtkGetObjectMacro ( SliceLogic, vtkSlicerSliceLogic);
    vtkSetObjectMacro ( SliceLogic, vtkSlicerSliceLogic);
    vtkGetObjectMacro ( SliceWidgets, vtkSlicerSliceWidgetCollection);
    vtkGetObjectMacro ( MainSlice0, vtkSlicerSliceWidget);
    vtkGetObjectMacro ( MainSlice1, vtkSlicerSliceWidget);
    vtkGetObjectMacro ( MainSlice2, vtkSlicerSliceWidget);
    
    virtual void AddSliceWidget ( vtkSlicerSliceWidget *w );
    vtkSlicerSliceWidget* GetSliceWidget ( int SliceWidgetNum );
    vtkSlicerSliceWidget* GetSliceWidget ( char *SliceWidgetColor );

    virtual void BuildGUI ( );
    virtual void BuildGUI ( vtkKWFrame *f1, vtkKWFrame *f2, vtkKWFrame *f3);
    virtual void AddGUIObservers ( );
    virtual void RemoveGUIObservers ( );
    virtual void AddLogicObservers ( );
    virtual void RemoveLogicObservers ( );

    virtual void ProcessLogicEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void ProcessGUIEvents ( vtkObject *caller, unsigned long event,
                                            void *callData );
    virtual void Enter ( );
    virtual void Exit ( );
    
 protected:
    vtkSlicerSliceGUI ( );
    ~vtkSlicerSliceGUI ( );

    vtkSlicerSliceLogic *SliceLogic;

    // Three slice widgets by default.
    vtkSlicerSliceWidget *MainSlice0;
    vtkSlicerSliceWidget *MainSlice1;
    vtkSlicerSliceWidget *MainSlice2;

    // Collection of the above and other slice widgets.
    vtkSlicerSliceWidgetCollection *SliceWidgets;
    
 private:
    vtkSlicerSliceGUI ( const vtkSlicerSliceGUI& ); // Not implemented.
    void operator = ( const vtkSlicerSliceGUI& ); //Not implemented.
}; 

#endif
