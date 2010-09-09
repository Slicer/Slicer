/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationROIDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationROIDisplayableManager_h
#define __vtkMRMLAnnotationROIDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManager.h"

class vtkMRMLAnnotationROINode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationROIDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationROIDisplayableManager :
    public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationROIDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationROIDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationROIDisplayableManager(){this->m_Focus="vtkMRMLAnnotationROINode";}
  virtual ~vtkMRMLAnnotationROIDisplayableManager(){}

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

  vtkMRMLAnnotationROIDisplayableManager(const vtkMRMLAnnotationROIDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationROIDisplayableManager&); /// Not Implemented

};

#endif

