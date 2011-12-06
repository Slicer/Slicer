/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationSplineDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationSplineDisplayableManager_h
#define __vtkMRMLAnnotationSplineDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationsModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManager.h"

class vtkMRMLAnnotationSplineNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationSplineDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationSplineDisplayableManager :
    public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationSplineDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationSplineDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationSplineDisplayableManager(){this->m_Focus="vtkMRMLAnnotationSplineNode";}
  virtual ~vtkMRMLAnnotationSplineDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationSplineDisplayableManager(const vtkMRMLAnnotationSplineDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationSplineDisplayableManager&); /// Not Implemented

};

#endif

