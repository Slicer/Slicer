/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationFiducialSliceViewDisplayableManager.h.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationFiducialSliceViewDisplayableManager_h
#define __vtkMRMLAnnotationFiducialSliceViewDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationSliceViewDisplayableManager.h"

class vtkMRMLAnnotationFiducialNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationFiducialSliceViewDisplayableManager :
    public vtkMRMLAnnotationSliceViewDisplayableManager
{
public:

  static vtkMRMLAnnotationFiducialSliceViewDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationFiducialSliceViewDisplayableManager, vtkMRMLAnnotationSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationFiducialSliceViewDisplayableManager(){this->m_Focus="vtkMRMLAnnotationFiducialNode";}
  virtual ~vtkMRMLAnnotationFiducialSliceViewDisplayableManager(){}

  /// Callback for click in slice view
  virtual void OnClickInSliceViewWindow(double x, double y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationFiducialSliceViewDisplayableManager(const vtkMRMLAnnotationFiducialSliceViewDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationFiducialSliceViewDisplayableManager&); /// Not Implemented

};

#endif

