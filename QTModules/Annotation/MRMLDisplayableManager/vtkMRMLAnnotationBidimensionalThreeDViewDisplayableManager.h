/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationBidimensionalDisplayableManager_h
#define __vtkMRMLAnnotationBidimensionalDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationThreeDViewDisplayableManager.h"

class vtkMRMLAnnotationBidimensionalNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationBidimensionalDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager :
    public vtkMRMLAnnotationThreeDViewDisplayableManager
{
public:

  static vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager, vtkMRMLAnnotationThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

protected:

  vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager(){this->m_Focus="vtkMRMLAnnotationBidimensionalNode";}
  virtual ~vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInThreeDRenderWindow(double x, double y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

private:

  vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager(const vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationBidimensionalThreeDViewDisplayableManager&); /// Not Implemented

};

#endif

