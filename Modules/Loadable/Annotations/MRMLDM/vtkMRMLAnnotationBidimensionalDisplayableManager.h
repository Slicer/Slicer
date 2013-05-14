/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationBidimensionalDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationBidimensionalDisplayableManager_h
#define __vtkMRMLAnnotationBidimensionalDisplayableManager_h

// Annotation includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLAnnotationBidimensionalNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationBidimensionalDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationBidimensionalDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationBidimensionalDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationBidimensionalDisplayableManager, vtkMRMLAnnotationDisplayableManager);
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

  vtkMRMLAnnotationBidimensionalDisplayableManager(){this->m_Focus="vtkMRMLAnnotationBidimensionalNode";}
  virtual ~vtkMRMLAnnotationBidimensionalDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

private:

  vtkMRMLAnnotationBidimensionalDisplayableManager(const vtkMRMLAnnotationBidimensionalDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationBidimensionalDisplayableManager&); /// Not Implemented

};

#endif
