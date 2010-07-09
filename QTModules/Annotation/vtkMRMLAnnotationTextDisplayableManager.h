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

  /// Alternative method to propagate events generated in GUI to logic / mrml
  virtual void ProcessMRMLEvents(vtkObject *caller, unsigned long event, void *callData);

  /// Add observers on node
  virtual void AddMRMLObservers();

  /// Remove observers on node, from AbstractDisplayableManager
  virtual void RemoveMRMLObservers();

  /// Update triggered from MRML node, from AbstractDisplayableManager
  virtual void UpdateFromMRML();

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLEvents
  virtual void OnMRMLSceneClosingEvent();
  virtual void OnMRMLSceneCloseEvent();
  virtual void OnMRMLSceneLoadStartEvent();
  virtual void OnMRMLSceneLoadEndEvent();
  virtual void OnMRMLSceneRestoredEvent();
  virtual void OnMRMLSceneNodeAddedEvent(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemovedEvent(vtkMRMLNode* node);

  /// Check scene to make sure that have a widget for each ruler node, and no extra widgets...
  void Update3DWidget(vtkMRMLAnnotationTextNode *activeNode);

  /// encapsulated 3d widgets for each ruler node
  //std::map<std::string, vtkTextWidget *> TextWidgets;
  vtkTextWidget * GetTextWidget(const char * nodeID);
  void AddTextWidget(vtkMRMLAnnotationTextNode *node);
  void RemoveTextWidget(vtkMRMLAnnotationTextNode *node);
  void RemoveTextWidgets();

  /// Get/set the viewer widget so can add a the ruler widget to it
  //vtkGetObjectMacro(ViewerWidget, vtkSlicerViewerWidget);
  //virtual void SetViewerWidget(vtkSlicerViewerWidget *viewerWidget);

  void UpdateWidget(vtkMRMLAnnotationTextNode *activeNode);

protected:

  vtkMRMLAnnotationTextDisplayableManager();
  virtual ~vtkMRMLAnnotationTextDisplayableManager();

  void UpdateWidgetInteractors();
  void UpdateLockUnlock(vtkMRMLAnnotationTextNode* textNode);

  /// the id of the mrml node currently displayed in the widget
  char* NodeID;

  /// flag set to 1 when updating a 3d widget (todo: set it to the index of the
  /// ruler node / 3d widget once have more than one)
  int Updating3DWidget;

  /// pointer to the viewer widget so can add props, request renders
  vtkSlicerViewerWidget *ViewerWidget;
  int ProcessingMRMLEvent;

private:

  vtkMRMLAnnotationTextDisplayableManager(const vtkMRMLAnnotationTextDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationTextDisplayableManager&); /// Not Implemented

};

#endif

