/*=========================================================================

  Module:    $RCSfile: vtkKWVolumePropertyWidget.h,v $

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSlicerVolumePropertyWidget - a transfer function widget
// .SECTION Description
// This class contains the UI components and methods to edit a 
// ColorTransferFunction in concert with a PiecewiseFunction for opacity.
// New control points can be added by clicking with the left mouse button
// and they can be removed by dragging them out of the window.

#ifndef __vtkSlicerVolumePropertyWidget_h
#define __vtkSlicerVolumePropertyWidget_h

#include "vtkVolumeRenderingReplacements.h"
#include "vtkKWCompositeWidget.h"
#include "vtkSlicerWidget.h"

class vtkDataSet;
class vtkKWCheckButton;
class vtkKWColorTransferFunctionEditor;
class vtkKWHSVColorSelector;
class vtkKWHistogramSet;
class vtkKWFrame;
class vtkKWFrameWithLabel;
class vtkKWMenuButtonWithLabel;
class vtkKWScaleWithEntrySetWithLabel;
class vtkKWMenuButton;
class vtkKWPiecewiseFunctionEditor;
class vtkKWScalarComponentSelectionWidget;
class vtkKWScaleWithEntry;
class vtkKWVolumeMaterialPropertyWidget;
class vtkVolumeProperty;

class VTK_VOLUMERENDERINGREPLACEMENTS_EXPORT vtkSlicerVolumePropertyWidget : public vtkSlicerWidget
{
public:
  static vtkSlicerVolumePropertyWidget* New();
  void PrintSelf(ostream& os, vtkIndent indent);
  vtkTypeMacro(vtkSlicerVolumePropertyWidget,vtkSlicerWidget);
//  vtkTypeRevisionMacro(vtkSlicerVolumePropertyWidget,vtkKWCompositeWidget);

  // Description:
  // Get/Set the transfer function mapping scalar value to color
  vtkGetObjectMacro(VolumeProperty, vtkVolumeProperty);
  virtual void SetVolumeProperty(vtkVolumeProperty*);

  // Description:
  // The data this volume property is used for. 
  // Will be used to get the scalar range of the transfer functions for
  // example.
  vtkGetObjectMacro(DataSet, vtkDataSet);
  virtual void SetDataSet(vtkDataSet*);

  // Description:
  // Set/Get the histograms for the data this volume property is used for.
  // Obviously those histograms must have been computed using the same
  // data as the DataSet Ivar above.
  vtkGetObjectMacro(HistogramSet, vtkKWHistogramSet);
  virtual void SetHistogramSet(vtkKWHistogramSet*);

  // Description:
  // Set/Get the current component controlled by the widget
  virtual void SetSelectedComponent(int);
  vtkGetMacro(SelectedComponent, int);

  // Description:
  // Set/Get the window/level for those transfer functions that support
  // this mode (SetInteractiveWindowLevel will trigger interactive events)
  // IsInWindowLevelMode return true if part of this widget is in
  // window/level mode.
  virtual void SetWindowLevel(float window, float level);
  virtual void SetInteractiveWindowLevel(float window, float level);
  virtual int IsInWindowLevelMode();

  // Description:
  // Set/Get if the widget is in "Interactive Apply" mode, i.e.
  // changes will be propagated during most/all interactions, not just
  // at the end of them.
  vtkBooleanMacro(InteractiveApplyMode, int);
  vtkSetMacro(InteractiveApplyMode, int);
  vtkGetMacro(InteractiveApplyMode, int);

  // Description:
  // Set/Get the 'Interactive Apply' button visibility
  vtkBooleanMacro(InteractiveApplyButtonVisibility, int);
  virtual void SetInteractiveApplyButtonVisibility(int);
  vtkGetMacro(InteractiveApplyButtonVisibility, int);

  // Description:
  // Set/Get the scalar opacity unit distance visibility
  vtkBooleanMacro(ScalarOpacityUnitDistanceVisibility, int);
  virtual void SetScalarOpacityUnitDistanceVisibility(int);
  vtkGetMacro(ScalarOpacityUnitDistanceVisibility, int);

  // Description:
  // Set/Get the HSV color selector visibility
  vtkBooleanMacro(HSVColorSelectorVisibility, int);
  virtual void SetHSVColorSelectorVisibility(int);
  vtkGetMacro(HSVColorSelectorVisibility, int);

  // Description:
  // Set/Get the component selection widget visibility
  vtkBooleanMacro(ComponentSelectionVisibility, int);
  virtual void SetComponentSelectionVisibility(int);
  vtkGetMacro(ComponentSelectionVisibility, int);

  // Description:
  // Set/Get the interpolation type widget visibility
  vtkBooleanMacro(InterpolationTypeVisibility, int);
  virtual void SetInterpolationTypeVisibility(int);
  vtkGetMacro(InterpolationTypeVisibility, int);

  // Description:
  // Get the bottom frame, a frame that is always packed at the bottom of
  // the whole widget, and can be used to add more user-defined widgets
  vtkGetObjectMacro(BottomFrame, vtkKWFrame);

  // Description:
  // Get the main edit frame, so that it can be collapsed manually
  vtkGetObjectMacro(EditorFrame, vtkKWFrameWithLabel);

  // Description:
  // Set/Get the material widget + enable shading visibility
  vtkBooleanMacro(MaterialPropertyVisibility, int);
  virtual void SetMaterialPropertyVisibility(int);
  vtkGetMacro(MaterialPropertyVisibility, int);

  // Description:
  // Set the material property button position to the top frame (default)
  // or to the scalar opacity user frame, or color user frame.
  //BTX
  enum
  {
    MaterialPropertyPositionTop,
    MaterialPropertyPositionBottomFrame,
    MaterialPropertyPositionScalarOpacityUserFrame,
    MaterialPropertyPositionScalarColorUserFrame
  };
  //ETX
  virtual void SetMaterialPropertyPosition(int);
  vtkGetMacro(MaterialPropertyPosition, int);
  virtual void SetMaterialPropertyPositionToTop();
  virtual void SetMaterialPropertyPositionToBottomFrame();
  virtual void SetMaterialPropertyPositionToScalarOpacityUserFrame();
  virtual void SetMaterialPropertyPositionToScalarColorUserFrame();
  
  // Description:
  // Set/Get the gradient opacity function visibility
  vtkBooleanMacro(GradientOpacityFunctionVisibility, int);
  virtual void SetGradientOpacityFunctionVisibility(int);
  vtkGetMacro(GradientOpacityFunctionVisibility, int);

  // Description:
  // Set/Get the component weight visibility
  vtkBooleanMacro(ComponentWeightsVisibility, int);
  virtual void SetComponentWeightsVisibility(int);
  vtkGetMacro(ComponentWeightsVisibility, int);

  // Description:
  // If true, an "Enable Shading" checkbox will be displayed and will
  // control the shading flag of all components at once 
  // (based on the first one). If false, the shading flag will be available
  // on a per-component basis in the shading dialog.
  vtkBooleanMacro(EnableShadingForAllComponents, int);
  virtual void SetEnableShadingForAllComponents(int);
  vtkGetMacro(EnableShadingForAllComponents, int);

  // Description:
  // Update the whole UI depending on the value of the Ivars
  virtual void Update();

  // Description:
  // Set/Get if the scalar color transfer function should be used to color
  // each point in the scalar opacity editor. This also makes sure that
  // any changes made to one or the other will be propagated accordingly. 
  // This alszo enables the user to pick a color by double-clicking on
  // the scalar opacity points.
  vtkBooleanMacro(UseScalarColorFunctionInScalarOpacityEditor, int);
  virtual void SetUseScalarColorFunctionInScalarOpacityEditor(int);
  vtkGetMacro(UseScalarColorFunctionInScalarOpacityEditor, int);

  // Description:
  // Merge both scalar opacity and color editors.
  // This will put both editors in synchronized/locked mode, the scalar
  // color function in the editor will be hidden (only the color ramp will
  // remain), and the opacity editor will use the scalar color function
  // to color its points. Double clicking on a point will popup a color
  // selector. 
  virtual void MergeScalarOpacityAndColorEditors();

  // Description:
  // Specifies commands to associate with the widget. 
  // 'VolumePropertyChangedCommand' is invoked when the volume property has
  // changed (i.e. at the end of the user interaction), whereas 
  // 'VolumePropertyChangingCommand' is invoked when the volume property is
  // changing (i.e. during the user interaction itself).
  // The need for a '...ChangedCommand' and '...ChangingCommand' can be
  // explained as follows: the former can be used to be notified about any
  // changes made to this widget *after* the corresponding user interaction has
  // been performed (say, after releasing the mouse button that was dragging
  // a slider, or after clicking on a checkbutton). The later can be set
  // *additionally* to be notified about the intermediate changes that
  // occur *during* the corresponding user interaction (say, *while* dragging
  // a slider). While setting '...ChangedCommand' is enough to be notified
  // about any changes, setting '...ChangingCommand' is an application-specific
  // choice that is likely to depend on how fast you want (or can) answer to
  // rapid changes occuring during a user interaction, if any.
  // The 'object' argument is the object that will have the method called on
  // it. The 'method' argument is the name of the method to be called and any
  // arguments in string form. If the object is NULL, the method is still
  // evaluated as a simple command. 
  virtual void SetVolumePropertyChangedCommand(
    vtkObject *object,const char *method);
  virtual void SetVolumePropertyChangingCommand(
    vtkObject *object,const char *method);

  // Description:
  // Set/Get whether the above commands should be called or not.
  vtkSetMacro(DisableCommands, int);
  vtkGetMacro(DisableCommands, int);
  vtkBooleanMacro(DisableCommands, int);

  // Description:
  // Set the method used to compute the whole range of the various transfer
  // function editors. If set to Data, the scalar range of the data is used.
  // If no data is available or set to FunctionPoints, the range of the
  // function points already in the editor is used. If set to 
  // DataAndFunctionPointsthe the largest of the two ranges mentioned 
  // previously is used.
  //BTX
  enum
  {
    WholeRangeComputationMethodData                  = 1,
    WholeRangeComputationMethodFunctionPoints        = 2,
    WholeRangeComputationMethodDataAndFunctionPoints = 3
  };
  //ETX
  virtual void SetWholeRangeComputationMethod(int);
  vtkGetMacro(WholeRangeComputationMethod, int);
  virtual void SetWholeRangeComputationMethodToData();
  virtual void SetWholeRangeComputationMethodToFunctionPoints();
  virtual void SetWholeRangeComputationMethodToDataAndFunctionPoints();
  
  // Description:
  // Access the objects
  vtkGetObjectMacro(ScalarOpacityFunctionEditor, vtkKWPiecewiseFunctionEditor);
  vtkGetObjectMacro(ScalarColorFunctionEditor, vtkKWColorTransferFunctionEditor);
  vtkGetObjectMacro(GradientOpacityFunctionEditor, vtkKWPiecewiseFunctionEditor);
  vtkGetObjectMacro(ScalarOpacityUnitDistanceScale, vtkKWScaleWithEntry);
  vtkGetObjectMacro(HSVColorSelector, vtkKWHSVColorSelector);
  vtkGetObjectMacro(MaterialPropertyWidget, vtkKWVolumeMaterialPropertyWidget);
 
  // Description:
  // Pack or repack the UI
  virtual void Pack();

  // Description:
  // Update the "enable" state of the object and its internal parts.
  // Depending on different Ivars (this->Enabled, the application's 
  // Limited Edition Mode, etc.), the "enable" state of the object is updated
  // and propagated to its internal parts/subwidgets. This will, for example,
  // enable/disable parts of the widget UI, enable/disable the visibility
  // of 3D widgets, etc.
  virtual void UpdateEnableState();

  // Description:
  // Callbacks. Internal, do not use.
  virtual void SelectedComponentCallback(int);
  virtual void InterpolationTypeCallback(int type);
  virtual void EnableShadingCallback(int state);
  virtual void MaterialPropertyChangedCallback();
  virtual void MaterialPropertyChangingCallback();
  virtual void InteractiveApplyCallback(int state);
  virtual void ScalarOpacityFunctionChangedCallback();
  virtual void ScalarOpacityFunctionChangingCallback();
  virtual void DoubleClickOnScalarOpacityPointCallback(int id);
  virtual void WindowLevelModeCallback(int mode);
  virtual void LockOpacityAndColorCallback(int state);
  virtual void ScalarOpacityUnitDistanceChangedCallback(double value);
  virtual void ScalarOpacityUnitDistanceChangingCallback(double value);
  virtual void RGBTransferFunctionChangedCallback();
  virtual void RGBTransferFunctionChangingCallback();
  virtual void RGBTransferFunctionSelectionChangedCallback();
  virtual void EnableGradientOpacityCallback(int val);
  virtual void GradientOpacityFunctionChangedCallback();
  virtual void GradientOpacityFunctionChangingCallback();
  virtual void HSVColorSelectionChangedCallback(double h, double s, double v);
  virtual void HSVColorSelectionChangingCallback(double h, double s, double v);
  virtual void ComponentWeightChangedCallback(int index, double value);
  virtual void ComponentWeightChangingCallback(int index, double value);

protected:
  vtkSlicerVolumePropertyWidget();
  ~vtkSlicerVolumePropertyWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  vtkVolumeProperty *VolumeProperty;
  vtkDataSet        *DataSet;
  vtkKWHistogramSet *HistogramSet;

  int   SelectedComponent;
  int   DisableCommands;
  int   EnableShadingForAllComponents;
  int   MaterialPropertyPosition;
  int   WholeRangeComputationMethod;
  int   InteractiveApplyMode;
  int   InteractiveApplyButtonVisibility;
  int   ScalarOpacityUnitDistanceVisibility;
  int   HSVColorSelectorVisibility;
  int   ComponentSelectionVisibility;
  int   InterpolationTypeVisibility;
  int   MaterialPropertyVisibility;
  int   GradientOpacityFunctionVisibility;
  int   ComponentWeightsVisibility;
  int   UseScalarColorFunctionInScalarOpacityEditor;

  // Commands

  char  *VolumePropertyChangedCommand;
  char  *VolumePropertyChangingCommand;

  virtual void InvokeObjectMethodCommand(const char *command);
  virtual void InvokeVolumePropertyChangedCommand();
  virtual void InvokeVolumePropertyChangingCommand();
  
  // GUI

  vtkKWFrameWithLabel                 *EditorFrame;
  vtkKWHSVColorSelector               *HSVColorSelector;
  vtkKWFrame                          *InnerLeftFrame;
  vtkKWScalarComponentSelectionWidget *ComponentSelectionWidget;
  vtkKWMenuButtonWithLabel            *InterpolationTypeOptionMenu;
  vtkKWVolumeMaterialPropertyWidget   *MaterialPropertyWidget;
  vtkKWCheckButton                    *EnableShadingCheckButton;
  vtkKWCheckButton                    *InteractiveApplyCheckButton;
  vtkKWPiecewiseFunctionEditor        *ScalarOpacityFunctionEditor;
  vtkKWScaleWithEntry                 *ScalarOpacityUnitDistanceScale;
  vtkKWColorTransferFunctionEditor    *ScalarColorFunctionEditor;
  vtkKWCheckButton                    *LockOpacityAndColorCheckButton;
  vtkKWPiecewiseFunctionEditor        *GradientOpacityFunctionEditor;
  vtkKWMenuButton                     *EnableGradientOpacityOptionMenu;
  vtkKWScaleWithEntrySetWithLabel     *ComponentWeightScaleSet;
  vtkKWFrame                          *BottomFrame;

  int                                 LockOpacityAndColor[VTK_MAX_VRCOMP];
  int                                 WindowLevelMode[VTK_MAX_VRCOMP];

  // Are the components independent of each other?

  virtual int GetIndependentComponents();
  
  // Update HSV selector

  virtual void UpdateHSVColorSelectorFromScalarColorFunctionEditor();

  // This methods will be overriden in subclasses so that something
  // different than the DataSet ivar will be used to compute the
  // corresponding items
  virtual int GetNumberOfComponents();
  virtual int GetDataSetScalarRange(int comp, double range[2]);
  virtual int GetDataSetAdjustedScalarRange(int comp, double range[2]);
  virtual const char* GetDataSetScalarName();
  virtual int GetDataSetScalarOpacityUnitDistanceRangeAndResolution(
    double range[2], double *resolution);

private:
  vtkSlicerVolumePropertyWidget(const vtkSlicerVolumePropertyWidget&); // Not implemented
  void operator=(const vtkSlicerVolumePropertyWidget&); // Not implemented
};
#endif
