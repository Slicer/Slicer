#ifndef __vtkSlicerSliceControllerWidget_h
#define __vtkSlicerSliceControllerWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithLabel.h"
#include "vtkKWFrame.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceControllerWidget : public vtkKWCompositeWidget
{
    
public:
  static vtkSlicerSliceControllerWidget* New ( );
  vtkTypeRevisionMacro ( vtkSlicerSliceControllerWidget, vtkKWCompositeWidget );
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
  // the SliceControllerWidget.
  vtkGetMacro ( ControllerStyle, int );
  vtkSetMacro ( ControllerStyle, int );

protected:
  vtkSlicerSliceControllerWidget ( );
  ~vtkSlicerSliceControllerWidget ( );

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  // Slice controller
  int ControllerStyle;
  vtkKWFrame *ControlFrame;
  vtkKWScaleWithEntry *OffsetScale;
  vtkKWEntryWithLabel *FieldOfViewEntry;
  vtkKWMenuButtonWithLabel *OrientationMenu;

private:
  vtkSlicerSliceControllerWidget (const vtkSlicerSliceControllerWidget &); //Not implemented
  void operator=(const vtkSlicerSliceControllerWidget &);         //Not implemented

};

#endif

