/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationAngleDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationAngleDisplayableManager_h
#define __vtkMRMLAnnotationAngleDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include "MRMLDisplayableManager/vtkMRMLAnnotationDisplayableManager.h"

class vtkMRMLAnnotationAngleNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkAngleWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationAngleDisplayableManager :
    public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationAngleDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationAngleDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationAngleDisplayableManager(){this->m_Focus="vtkMRMLAnnotationAngleNode";}
  virtual ~vtkMRMLAnnotationAngleDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInThreeDRenderWindow(double x, double y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Propagate properties of MRML node to widgets.
  virtual void SetWidget(vtkMRMLAnnotationNode* node);
  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

private:

  vtkMRMLAnnotationAngleDisplayableManager(const vtkMRMLAnnotationAngleDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationAngleDisplayableManager&); /// Not Implemented

};

#endif

