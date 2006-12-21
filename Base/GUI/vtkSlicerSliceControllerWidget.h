#ifndef __vtkSlicerSliceControllerWidget_h
#define __vtkSlicerSliceControllerWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWScaleWithEntry.h"
#include "vtkKWEntryWithLabel.h"
#include "vtkKWMenuButtonWithSpinButtonsWithLabel.h"
#include "vtkKWMenuButton.h"
#include "vtkKWPushButton.h"
#include "vtkKWFrame.h"
#include "vtkKWTopLevel.h"

#include "vtkSlicerModuleCollapsibleFrame.h"
#include "vtkSlicerWidget.h"
#include "vtkSlicerNodeSelectorWidget.h"
#include "vtkSlicerSlicesControlIcons.h"
#include "vtkSlicerVisibilityIcons.h"

#include "vtkMRMLSliceNode.h"
#include "vtkMRMLSliceCompositeNode.h"

#include "vtkSlicerSliceLogic.h"

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
  vtkGetObjectMacro ( LinkButton, vtkKWPushButton );
  vtkGetObjectMacro ( VisibilityIcons, vtkSlicerVisibilityIcons );
  vtkGetObjectMacro ( SliceControlIcons, vtkSlicerSlicesControlIcons);
  vtkGetObjectMacro ( ScaleFrame, vtkKWFrame );
  vtkGetObjectMacro ( LabelOpacityButton, vtkKWPushButton );
  vtkGetObjectMacro ( LabelOpacityScale, vtkKWScaleWithEntry );
  vtkGetObjectMacro ( LabelOpacityTopLevel, vtkKWTopLevel );
  vtkGetObjectMacro ( FitToWindowButton, vtkKWPushButton );
  vtkGetObjectMacro ( VolumeDisplayMenuButton, vtkKWMenuButton );


  void RemoveWidgetObservers ( );
  void AddWidgetObservers ( );
  
  // Description:
  // Get/Set the Nodes
  vtkGetObjectMacro ( SliceCompositeNode, vtkMRMLSliceCompositeNode );
  void SetSliceCompositeNode (vtkMRMLSliceCompositeNode *scnode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->SliceCompositeNode, scnode );
    }
  vtkGetObjectMacro ( SliceNode, vtkMRMLSliceNode );
  void SetSliceNode (vtkMRMLSliceNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro(this->SliceNode, snode );
    }

  // Description:
  // slice logic controlling the slice to be manipulated
  vtkSetObjectMacro ( SliceLogic, vtkSlicerSliceLogic );
  vtkGetObjectMacro ( SliceLogic, vtkSlicerSliceLogic );

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

  // Description:
  // applies a background color to the controller
  virtual void ApplyColorCode ( double *c );
  virtual void ToggleSlicesLink  ( );
  virtual void LinkAllSlices();
  virtual void UnlinkAllSlices();
  virtual int AllSlicesLinked ( );
  virtual void HideLabelOpacityScaleAndEntry ( );
  virtual void PopUpLabelOpacityScaleAndEntry ( );

  // Description:
  // Shrink/Expand the widget
  virtual void Shrink();
  virtual void Expand();

  //BTX
  enum 
  {
    ExpandEvent = 15000,
    ShrinkEvent
  };
  //ETX

protected:
  vtkSlicerSliceControllerWidget ( );
  virtual ~vtkSlicerSliceControllerWidget ( );

  // Description:
  // Create the widget.
  virtual void CreateWidget( );

  // TODO: hook this up
  int ControllerStyle;

  vtkKWFrame *ContainerFrame;

  //
  // Slice controller subwidgets
  //
  vtkKWScaleWithEntry *OffsetScale;
  vtkKWMenuButtonWithSpinButtonsWithLabel *OrientationMenu;
  vtkSlicerNodeSelectorWidget *ForegroundSelector;
  vtkSlicerNodeSelectorWidget *BackgroundSelector;
  vtkSlicerNodeSelectorWidget *LabelSelector;
  vtkKWPushButton *VisibilityToggle;
  vtkKWPushButton *LinkButton;
  vtkKWPushButton *LabelOpacityButton;
  vtkKWScaleWithEntry *LabelOpacityScale;
  vtkKWTopLevel *LabelOpacityTopLevel;

  vtkSlicerVisibilityIcons *VisibilityIcons;
  vtkSlicerSlicesControlIcons *SliceControlIcons;
  vtkKWFrame *ScaleFrame;
  vtkKWPushButton *ColorCodeButton;
  vtkKWPushButton *FitToWindowButton;
  vtkKWMenuButton *VolumeDisplayMenuButton;

  //
  // Nodes
  //
  vtkMRMLSliceNode *SliceNode;
  vtkMRMLSliceCompositeNode *SliceCompositeNode;

  //
  // Logic
  //
  vtkSlicerSliceLogic *SliceLogic;

private:
  vtkSlicerSliceControllerWidget (const vtkSlicerSliceControllerWidget &); //Not implemented
  void operator=(const vtkSlicerSliceControllerWidget &);         //Not implemented

};

#endif

