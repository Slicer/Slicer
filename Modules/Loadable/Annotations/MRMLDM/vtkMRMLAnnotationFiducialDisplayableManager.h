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
  vtkTypeRevisionMacro(vtkMRMLAnnotationFiducialDisplayableManager, vtkMRMLAnnotationDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLAnnotationFiducialDisplayableManager(){this->m_Focus="vtkMRMLAnnotationFiducialNode";}
  virtual ~vtkMRMLAnnotationFiducialDisplayableManager(){}

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

  /// set up an observer on the interactor style to watch for key press events
  virtual void AdditionnalInitializeStep();
  /// respond to the interactor style event
  virtual void OnInteractorStyleEvent(int eventid);

  // respond to control point modified events
  virtual void UpdatePosition(vtkAbstractWidget *widget, vtkMRMLNode *node);

  std::map<vtkMRMLNode*, int> NodeGlyphTypes;

  // clean up when scene closes
  virtual void OnMRMLSceneEndClose();

private:

  vtkMRMLAnnotationFiducialDisplayableManager(const vtkMRMLAnnotationFiducialDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationFiducialDisplayableManager&); /// Not Implemented

};

#endif
