/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationAngleDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationAngleDisplayableManager_h
#define __vtkMRMLAnnotationAngleDisplayableManager_h

// Annotations includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLAnnotationAngleNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkAngleWidget;

class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationAngleDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationAngleDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAnnotationAngleDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

  // update just the position, called from PropagateMRMLToWidget and in
  // response to slice node modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

protected:

  vtkMRMLAnnotationAngleDisplayableManager(){this->m_Focus="vtkMRMLAnnotationAngleNode";}
  virtual ~vtkMRMLAnnotationAngleDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

private:

  vtkMRMLAnnotationAngleDisplayableManager(const vtkMRMLAnnotationAngleDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationAngleDisplayableManager&); /// Not Implemented

};

#endif
