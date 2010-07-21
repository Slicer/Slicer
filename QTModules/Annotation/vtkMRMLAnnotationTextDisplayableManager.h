/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkSlicerNodeSelectorWidget.h,v $
 Date:      $Date: 2006/01/08 04:48:05 $
 Version:   $Revision: 1.45 $

 =========================================================================auto=*/

#ifndef __vtkSlicerAnnotationTextManager_h
#define __vtkSlicerAnnotationTextManager_h

// AnnotationModule includes
#include "qSlicerAnnotationModuleExport.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>

class vtkMRMLAnnotationTextNode;
class vtkSlicerViewerWidget;
class vtkMRMLAnnotationTextDisplayNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkMRMLAnnotationLineDisplayNode;
class vtkTextWidget;

class Q_SLICER_QTMODULES_ANNOTATIONS_EXPORT vtkMRMLAnnotationTextDisplayableManager :
    public vtkMRMLAbstractDisplayableManager
{
public:

  static vtkMRMLAnnotationTextDisplayableManager *New();
  vtkTypeRevisionMacro(vtkMRMLAnnotationTextDisplayableManager, vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

protected:

  vtkMRMLAnnotationTextDisplayableManager();
  virtual ~vtkMRMLAnnotationTextDisplayableManager();

  virtual void Create();

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLEvents
  virtual void OnMRMLSceneAboutToBeClosedEvent();
  virtual void OnMRMLSceneClosedEvent();
  virtual void OnMRMLSceneAboutToBeImportedEvent();
  virtual void OnMRMLSceneImportedEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  void SetAndObserveTextNodes();

  void OnMRMLAnnotationTextNodeModifiedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationTextNodeTransformModifiedEvent(vtkMRMLNode* node);
  void OnMRMLAnnotationTextNodeLockModifiedEvent(vtkMRMLNode* node);

private:

  vtkMRMLAnnotationTextDisplayableManager(const vtkMRMLAnnotationTextDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationTextDisplayableManager&); /// Not Implemented

  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

};

#endif

