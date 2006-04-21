#ifndef __vtkSlicerSliceController_h
#define __vtkSlicerSliceController_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceController : public vtkKWCompositeWidget
{
    
 public:
    static vtkSlicerSliceController* New ( );
    vtkTypeRevisionMacro ( vtkSlicerSliceController, vtkKWCompositeWidget );
    void PrintSelf (ostream& os, vtkIndent indent);
    
    // Description:
    // Get/Set the Widgets in this composite widget.
    vtkGetObjectMacro ( OffsetScale, vtkKWScaleWithEntry );
    vtkGetObjectMacro ( FieldOfViewEntry, vtkKWEntryWithLabel );
    vtkGetObjectMacro ( OrientationMenu, vtkKWMenuButtonWithLabel );
    vtkGetObjectMacro ( ControlFrame, vtkKWFrame );
    vtkSetObjectMacro ( ControlFrame, vtkKWFrame );

    // Description:
    // TODO: Use this flag to determine how to display
    // the SliceController.
    vtkGetMacro ( ControllerStyle, int );
    vtkSetMacro ( ControllerStyle, int );

    // Description:
    // Create the Widget inside a parent frame.
    virtual void Create ( );

 protected:
    vtkSlicerSliceController ( );
    ~vtkSlicerSliceController ( );
    // Slice controller
    int ControllerStyle;
    vtkKWFrame *ControlFrame;
    vtkKWScaleWithEntry *OffsetScale;
    vtkKWEntryWithLabel *FieldOfViewEntry;
    vtkKWMenuButtonWithLabel *OrientationMenu;

 private:
    vtkSlicerSliceController (const vtkSlicerSliceController &); //Not implemented
    void operator=(const vtkSlicerSliceController &);         //Not implemented

};

#endif

