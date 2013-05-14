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
  vtkTypeRevisionMacro(vtkMRMLAnnotationROIDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationROIDisplayableManager(){this->m_Focus="vtkMRMLAnnotationROINode";}
  virtual ~vtkMRMLAnnotationROIDisplayableManager();

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  virtual void PropagateMRMLToWidget2D(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);

  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);

  /// Handler for specific SliceView actions
  virtual void OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode * sliceNode);


  /// Update just the position for the widget, implemented by subclasses.
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

  /// Check, if the widget is displayable in the current slice geometry
  virtual bool IsWidgetDisplayable(vtkMRMLSliceNode *sliceNode, vtkMRMLAnnotationNode* node);

  /// Set mrml parent transform to widgets
  virtual void SetParentTransformToWidget(vtkMRMLAnnotationNode *node, vtkAbstractWidget *widget);

private:

  vtkMRMLAnnotationROIDisplayableManager(const vtkMRMLAnnotationROIDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationROIDisplayableManager&); /// Not Implemented

};

#endif
