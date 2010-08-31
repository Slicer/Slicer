/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationSplineThreeDViewDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationSplineDisplayableManager_h
#define __vtkMRMLAnnotationSplineDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationThreeDViewDisplayableManager.h"

class vtkMRMLAnnotationSplineNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationSplineDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSplineThreeDViewDisplayableManager :
    public vtkMRMLAnnotationThreeDViewDisplayableManager
{
public:

  static vtkMRMLAnnotationSplineThreeDViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationSplineThreeDViewDisplayableManager, vtkMRMLAnnotationThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationSplineThreeDViewDisplayableManager(){this->m_Focus="vtkMRMLAnnotationSplineNode";}
  virtual ~vtkMRMLAnnotationSplineThreeDViewDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInThreeDRenderWindow(double x, double y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationSplineThreeDViewDisplayableManager(const vtkMRMLAnnotationSplineThreeDViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationSplineThreeDViewDisplayableManager&); /// Not Implemented

};

#endif

