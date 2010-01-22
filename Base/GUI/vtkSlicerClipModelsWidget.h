#ifndef __vtkSlicerClipModelsWidget_h
#define __vtkSlicerClipModelsWidget_h

#include "vtkSlicerBaseGUIWin32Header.h"
#include "vtkKWCompositeWidget.h"
#include "vtkKWFrame.h"

#include "vtkSlicerWidget.h"
#include "vtkSlicerVisibilityIcons.h"

#include "vtkMRMLClipModelsNode.h"

class vtkSlicerNodeSelectorWidget;
class vtkKWMenuButtonWithLabel;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerClipModelsWidget : public vtkSlicerWidget
{
    
public:
  static vtkSlicerClipModelsWidget* New ( );
  vtkTypeRevisionMacro ( vtkSlicerClipModelsWidget, vtkKWCompositeWidget );
  void PrintSelf (ostream& os, vtkIndent indent);
    
  
  void RemoveWidgetObservers ( );
  void AddWidgetObservers ( );
  
  /// 
  /// Get/Set the Clip Nodes
  vtkGetObjectMacro ( ClipModelsNode, vtkMRMLClipModelsNode );
  void SetClipModelsNode (vtkMRMLClipModelsNode *snode)
    {
    vtkSetAndObserveMRMLNodeMacro( this->ClipModelsNode, snode );
    }


  /// 
  /// respond to events from subwidgets of this widget
  void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  /// 
  /// respond to changes in the mrml scene
  void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );

  /// 
  /// respond to changes in the slice logic
  void ProcessLogicEvents ( vtkObject *vtkNotUsed(caller),
                            unsigned long vtkNotUsed(event),
                            void *vtkNotUsed(callData) ){};


protected:
  vtkSlicerClipModelsWidget ( );
  virtual ~vtkSlicerClipModelsWidget ( );

  /// 
  /// Create the widget.
  virtual void CreateWidget( );

  void UpdateGUI();
  void UpdateMRML();
  void UpdateClipModelsNode();

  //
  /// Slice controller subwidgets
  //
  vtkKWMenuButtonWithLabel *RedSliceClipStateMenu;
  vtkKWMenuButtonWithLabel *YellowSliceClipStateMenu;  
  vtkKWMenuButtonWithLabel *GreenSliceClipStateMenu;
  
  vtkKWMenuButtonWithLabel *ClipTypeMenu;

  vtkSlicerNodeSelectorWidget *ClipModelsNodeSelector;
  //
  /// Nodes
  //
  vtkMRMLClipModelsNode *ClipModelsNode;

private:
  vtkSlicerClipModelsWidget (const vtkSlicerClipModelsWidget &); //Not implemented
  void operator=(const vtkSlicerClipModelsWidget &);         //Not implemented

};

#endif

