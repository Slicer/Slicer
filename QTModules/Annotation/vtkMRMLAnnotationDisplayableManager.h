/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.1 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationDisplayableManager_h
#define __vtkMRMLAnnotationDisplayableManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

class vtkMRMLAnnotationNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkAbstractWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationDisplayableManager :
    public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:

  static vtkMRMLAnnotationDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  // Get the coordinates of a click in the RenderWindow
  void OnClickInThreeDRenderWindowGetCoordinates();


protected:

  vtkMRMLAnnotationDisplayableManager();
  virtual ~vtkMRMLAnnotationDisplayableManager();

  virtual void Create();

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLEvents
  virtual void OnMRMLSceneAboutToBeClosedEvent();
  virtual void OnMRMLSceneClosedEvent();
  virtual void OnMRMLSceneAboutToBeImportedEvent();
  virtual void OnMRMLSceneImportedEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  /// Observe all associated nodes.
  void SetAndObserveNodes();

  /// Preset functions for certain events.
  void OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node);

  /// Get the widget of a node.
  vtkAbstractWidget * GetWidget(vtkMRMLAnnotationNode * node);


  /// Callback for click in RenderWindow
  virtual void OnClickInThreeDRenderWindow(float x, float y);
  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Propagate properties of MRML node to widgets.
  virtual void SetWidget(vtkMRMLAnnotationNode* node);

private:

  vtkMRMLAnnotationDisplayableManager(const vtkMRMLAnnotationDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationDisplayableManager&); /// Not Implemented

  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif

