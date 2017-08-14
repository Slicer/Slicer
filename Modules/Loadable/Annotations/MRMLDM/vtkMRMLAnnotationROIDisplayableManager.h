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
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

protected:

  vtkMRMLAnnotationROIDisplayableManager(){this->m_Focus="vtkMRMLAnnotationROINode";}
  virtual ~vtkMRMLAnnotationROIDisplayableManager();

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) VTK_OVERRIDE;
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node) VTK_OVERRIDE;

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node) VTK_OVERRIDE;

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget) VTK_OVERRIDE;
  virtual void PropagateMRMLToWidget2D(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node) VTK_OVERRIDE;

  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) VTK_OVERRIDE;

  /// Handler for specific SliceView actions
  virtual void OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode * sliceNode) VTK_OVERRIDE;


  /// Update just the position for the widget, implemented by subclasses.
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) VTK_OVERRIDE;

  /// Check, if the widget is displayable in the current slice geometry
  virtual bool IsWidgetDisplayable(vtkMRMLSliceNode *sliceNode, vtkMRMLAnnotationNode* node) VTK_OVERRIDE;

  /// Set mrml parent transform to widgets
  virtual void SetParentTransformToWidget(vtkMRMLAnnotationNode *node, vtkAbstractWidget *widget) VTK_OVERRIDE;

private:

  vtkMRMLAnnotationROIDisplayableManager(const vtkMRMLAnnotationROIDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationROIDisplayableManager&); /// Not Implemented

};

#endif
