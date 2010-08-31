/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationRulerThreeDViewDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationRulerDisplayableManager_h
#define __vtkMRMLAnnotationRulerDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationThreeDViewDisplayableManager.h"

class vtkMRMLAnnotationRulerNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationRulerDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationRulerThreeDViewDisplayableManager :
    public vtkMRMLAnnotationThreeDViewDisplayableManager
{
public:

  static vtkMRMLAnnotationRulerThreeDViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationRulerThreeDViewDisplayableManager, vtkMRMLAnnotationThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationRulerThreeDViewDisplayableManager(){this->m_Focus="vtkMRMLAnnotationRulerNode";}
  virtual ~vtkMRMLAnnotationRulerThreeDViewDisplayableManager(){}

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

  vtkMRMLAnnotationRulerThreeDViewDisplayableManager(const vtkMRMLAnnotationRulerThreeDViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationRulerThreeDViewDisplayableManager&); /// Not Implemented

};

#endif

