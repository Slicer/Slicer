#ifndef __vtkSlicerSliceControllerWidget_h
#define __vtkSlicerSliceControllerWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWPushButton.h"

#include "vtkSlicerWidget.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerVisibilityIcons.h"

#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerSliceControllerWidget : public vtkSlicerWidget
{
    
public:
  static vtkSlicerSliceControllerWidget* New ( );
  vtkTypeRevisionMacro ( vtkSlicerSliceControllerWidget, vtkKWCompositeWidget );
  void PrintSelf (ostream& os, vtkIndent indent);
    
  // Description:
  // Get/Set the Widgets in this composite widget.
  vtkGetObjectMacro ( OffsetScale, vtkKWScaleWithEntry );
  vtkGetObjectMacro ( OrientationMenu, vtkKWMenuButtonWithSpinButtonsWithLabel );
  vtkGetObjectMacro ( ForegroundSelector, vtkSlicerNodeSelectorWidget );
  vtkGetObjectMacro ( BackgroundSelector, vtkSlicerNodeSelectorWidget );
  vtkGetObjectMacro ( LabelSelector, vtkSlicerNodeSelectorWidget );
  vtkGetObjectMacro ( VisibilityToggle, vtkKWPushButton );
  vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons );

  void RemoveWidgetObservers ( );
  void AddWidgetObservers ( );
  
  // Description:
  // Get/Set the Nodes
  vtkGetObjectMacro ( SliceCompositeNode, vtkMRMLSliceCompositeNode );
  void SetSliceCompositeNode (vtkMRMLSliceCompositeNode *scnode)
    {
    this->SetAndObserveMRML( vtkObjectPointer(&this->SliceCompositeNode), scnode );
    }
  vtkGetObjectMacro ( SliceNode, vtkMRMLSliceNode );
  void SetSliceNode (vtkMRMLSliceNode *snode)
    {
    this->SetAndObserveMRML( vtkObjectPointer(&this->SliceNode), snode );
    }

  // Description:
  // TODO: Use this flag to determine how to display
  // the SliceControllerWidget.
  vtkGetMacro ( ControllerStyle, int );
  vtkSetMacro ( ControllerStyle, int );
  
  // Description:
  // respond to events from subwidgets of this widget
  void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // respond to changes in the mrml scene
  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  // Description:
  // respond to changes in the slice logic
  void ProcessLogicEvents ( vtkObject *caller, unsigned long event, void *callData );

protected:
  vtkSlicerSliceControllerWidget ( );
  ~vtkSlicerSliceControllerWidget ( );

  // Description:
  // Create the widget.
  virtual void CreateWidget( );

  // TODO: hook this up
  int ControllerStyle;

  //
  // Slice controller subwidgets
  //
  vtkKWScaleWithEntry *OffsetScale;
  vtkKWMenuButtonWithSpinButtonsWithLabel *OrientationMenu;
  vtkSlicerNodeSelectorWidget *ForegroundSelector;
  vtkSlicerNodeSelectorWidget *BackgroundSelector;
  vtkSlicerNodeSelectorWidget *LabelSelector;
  vtkKWPushButton *VisibilityToggle;
  vtkSlicerVisibilityIcons *VisibilityIcons;
  
  //
  // Nodes
  //
  vtkMRMLSliceNode *SliceNode;
  vtkMRMLSliceCompositeNode *SliceCompositeNode;

private:
  vtkSlicerSliceControllerWidget (const vtkSlicerSliceControllerWidget &); //Not implemented
  void operator=(const vtkSlicerSliceControllerWidget &);         //Not implemented

};

#endif

