/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerNodeSelectorWidget - menu to select volumes from current mrml scene
// .SECTION Description
// Inherits most behavior from kw widget, but is specialized to observe
// the current mrml scene and update the entries of the pop up menu to correspond
// to the currently available volumes.  This widget also has a notion of the current selection
// that can be observed or set externally
//


#ifndef __vtkSlicerTransformEditorWidget_h
#define __vtkSlicerTransformEditorWidget_h

#include "vtkSlicerWidget.h"

class vtkSlicerNodeSelectorWidget;
class vtkKWPushButton;
class vtkKWMatrix4x4;
class vtkKWScaleWithEntry;
class vtkKWMenuButtonWithLabel;

class vtkTransform;
class vtkMatrix4x4;

class VTK_SLICER_BASE_GUI_EXPORT vtkSlicerTransformEditorWidget : public vtkSlicerWidget
{
  
public:
  static vtkSlicerTransformEditorWidget* New();
  vtkTypeRevisionMacro(vtkSlicerTransformEditorWidget,vtkKWCompositeWidget);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessWidgetEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents ( vtkObject *caller, unsigned long event, void *callData );
  
  // Description:
  // removes observers on widgets in the class
  virtual void RemoveWidgetObservers ( );
  
  void TransformChangedCallback(double);
  void TransformChangingCallback(double);

  void RotationLRChangedCallback(double);
  void RotationLRChangingCallback(double);
  void RotationPAChangedCallback(double);
  void RotationPAChangingCallback(double);
  void RotationISChangedCallback(double);
  void RotationISChangingCallback(double);

  void RotationChangedCallback(int axis, double value);
  void RotationChangingCallback(int axis, double value);


 protected:
  vtkSlicerTransformEditorWidget();
  virtual ~vtkSlicerTransformEditorWidget();

  // Description:
  // Create the widget.
  virtual void CreateWidget();

  void UpdateTranslationSliders();
  void ResetRotationSliders(int axis);

  void UpdateMatrix();

  vtkTransform* GetCurrentTransformInCoordinateSystem();

  vtkSlicerNodeSelectorWidget* TransformEditSelectorWidget;
  vtkKWMatrix4x4*              MatrixWidget;
  vtkKWPushButton*             IdentityButton;
  vtkKWPushButton*             InvertButton;

  vtkKWScaleWithEntry* TranslationScaleLR;
  vtkKWScaleWithEntry* TranslationScalePA;
  vtkKWScaleWithEntry* TranslationScaleIS;

  vtkKWScaleWithEntry* RotationScaleLR;
  vtkKWScaleWithEntry* RotationScalePA;
  vtkKWScaleWithEntry* RotationScaleIS;
  vtkKWMenuButtonWithLabel* RotationCoordinateSystemMenu;

  bool ProcessingCallback;

  int               RotationAxis;
  vtkMatrix4x4*     RotationMatrix;


private:


  vtkSlicerTransformEditorWidget(const vtkSlicerTransformEditorWidget&); // Not implemented
  void operator=(const vtkSlicerTransformEditorWidget&); // Not Implemented
};

#endif

