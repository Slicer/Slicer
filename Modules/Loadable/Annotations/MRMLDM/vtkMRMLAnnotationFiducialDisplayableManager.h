/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationFiducialDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.5 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationFiducialDisplayableManager_h
#define __vtkMRMLAnnotationFiducialDisplayableManager_h

// Annotation includes
#include "vtkMRMLAnnotationDisplayableManager.h"
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"

class vtkMRMLAnnotationFiducialNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationFiducialDisplayableManager
  : public vtkMRMLAnnotationDisplayableManager
{
public:

  static vtkMRMLAnnotationFiducialDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAnnotationFiducialDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLAnnotationFiducialDisplayableManager(){this->m_Focus="vtkMRMLAnnotationFiducialNode";}
  ~vtkMRMLAnnotationFiducialDisplayableManager() override = default;

  /// Callback for click in RenderWindow
  void OnClickInRenderWindow(double x, double y, const char *associatedNodeID) override;
  /// Create a widget.
  vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node) override;

  /// Gets called when widget was created
  void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node) override;

  /// Propagate properties of MRML node to widget.
  void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget) override;
  /// Propagate properties of widget to MRML node.
  void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node) override;

  /// set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// respond to the interactor style event
  void OnInteractorStyleEvent(int eventid) override;

  // respond to control point modified events
  void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node) override;

  std::map<vtkMRMLNode*, int> NodeGlyphTypes;

  // clean up when scene closes
  void OnMRMLSceneEndClose() override;

private:

  vtkMRMLAnnotationFiducialDisplayableManager(const vtkMRMLAnnotationFiducialDisplayableManager&) = delete;
  void operator=(const vtkMRMLAnnotationFiducialDisplayableManager&) = delete;

};

#endif
