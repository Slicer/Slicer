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
  vtkTypeMacro(vtkMRMLAnnotationBidimensionalDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget) override;
  /// Propagate properties of widget to MRML node.
  void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node) override;
  // update just the position, called from PropagateMRMLToWidget and in
  // response to slice node modified events
  void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) override;

protected:

  vtkMRMLAnnotationBidimensionalDisplayableManager(){this->m_Focus="vtkMRMLAnnotationBidimensionalNode";}
  ~vtkMRMLAnnotationBidimensionalDisplayableManager() override = default;

  /// Callback for click in RenderWindow
  void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) override;
  /// Create a widget.
  vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node) override;

  /// Gets called when widget was created
  void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node) override;

private:

  vtkMRMLAnnotationBidimensionalDisplayableManager(const vtkMRMLAnnotationBidimensionalDisplayableManager&) = delete;
  void operator=(const vtkMRMLAnnotationBidimensionalDisplayableManager&) = delete;

};

#endif
