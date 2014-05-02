/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationStickyDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationStickyDisplayableManager_h
#define __vtkMRMLAnnotationStickyDisplayableManager_h

// Annotation includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLAnnotationStickyNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationStickyDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationStickyDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAnnotationStickyDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationStickyDisplayableManager(){this->m_Focus="vtkMRMLAnnotationStickyNode";}
  virtual ~vtkMRMLAnnotationStickyDisplayableManager(){}

  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);

  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationStickyDisplayableManager(const vtkMRMLAnnotationStickyDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationStickyDisplayableManager&); /// Not Implemented

};

#endif
