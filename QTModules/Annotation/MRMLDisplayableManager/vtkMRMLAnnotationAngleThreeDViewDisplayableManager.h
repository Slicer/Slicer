/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationAngleThreeDViewDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationAngleDisplayableManager_h
#define __vtkMRMLAnnotationAngleDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationThreeDViewDisplayableManager.h"

class vtkMRMLAnnotationAngleNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkAngleWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationAngleThreeDViewDisplayableManager :
    public vtkMRMLAnnotationThreeDViewDisplayableManager
{
public:

  static vtkMRMLAnnotationAngleThreeDViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationAngleThreeDViewDisplayableManager, vtkMRMLAnnotationThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

protected:

  vtkMRMLAnnotationAngleThreeDViewDisplayableManager(){this->m_Focus="vtkMRMLAnnotationAngleNode";}
  virtual ~vtkMRMLAnnotationAngleThreeDViewDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInThreeDRenderWindow(double x, double y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

private:

  vtkMRMLAnnotationAngleThreeDViewDisplayableManager(const vtkMRMLAnnotationAngleThreeDViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationAngleThreeDViewDisplayableManager&); /// Not Implemented

};

#endif

