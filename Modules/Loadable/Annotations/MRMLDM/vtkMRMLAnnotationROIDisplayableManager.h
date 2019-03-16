/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationROIDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationROIDisplayableManager_h
#define __vtkMRMLAnnotationROIDisplayableManager_h

// Annotation includes
#include "vtkMRMLAnnotationDisplayableManager.h"

class vtkMRMLAnnotationROINode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationROIDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationROIDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationROIDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAnnotationROIDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLAnnotationROIDisplayableManager(){this->m_Focus="vtkMRMLAnnotationROINode";}
  ~vtkMRMLAnnotationROIDisplayableManager() override;

  /// Callback for click in RenderWindow
  void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) override;
  /// Create a widget.
  vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node) override;

  /// Gets called when widget was created
  void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node) override;

  /// Propagate properties of MRML node to widget.
  void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget) override;
  virtual void PropagateMRMLToWidget2D(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to MRML node.
  void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node) override;

  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  /// Handler for specific SliceView actions
  void OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode * sliceNode) override;


  /// Update just the position for the widget, implemented by subclasses.
  void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) override;

  /// Check, if the widget is displayable in the current slice geometry
  bool IsWidgetDisplayable(vtkMRMLSliceNode *sliceNode, vtkMRMLAnnotationNode* node) override;

  /// Set mrml parent transform to widgets
  void SetParentTransformToWidget(vtkMRMLAnnotationNode *node, vtkAbstractWidget *widget) override;

private:

  vtkMRMLAnnotationROIDisplayableManager(const vtkMRMLAnnotationROIDisplayableManager&) = delete;
  void operator=(const vtkMRMLAnnotationROIDisplayableManager&) = delete;

};

#endif
