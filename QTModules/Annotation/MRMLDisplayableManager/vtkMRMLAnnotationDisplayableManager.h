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
#include "vtkMRMLAnnotationClickCounter.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractThreeDViewDisplayableManager.h>

// VTK includes
#include <vtkSeedWidget.h>
#include <vtkHandleWidget.h>

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

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);


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
  virtual void OnClickInThreeDRenderWindow(double x, double y);
  /// Counter for clicks in 3D Render Window
  vtkMRMLAnnotationClickCounter* m_ClickCounter;

  /// Place a seed for widgets
  virtual void PlaceSeed(double x, double y);

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);

  /// SeedWidget for point placement
  vtkSeedWidget * m_SeedWidget;
  /// List of Handles for the SeedWidget
  std::vector<vtkHandleWidget*> m_HandleWidgetList;
  /// .. and its associated convenient typedef
  typedef std::vector<vtkHandleWidget*>::iterator HandleWidgetListIt;

  /// Check if it is the right displayManager
  virtual bool IsCorrectDisplayableManager();

  // Convert display to world coordinates
  virtual double * GetDisplayToWorldCoordinates(double x, double y);

  // Convert display to world coordinates
  virtual double * GetWorldToDisplayCoordinates(double r, double a, double s);

  /// Focus of this displayableManager
  const char* m_Focus;

private:

  vtkMRMLAnnotationDisplayableManager(const vtkMRMLAnnotationDisplayableManager&); /// Not implemented
  void operator=(const vtkMRMLAnnotationDisplayableManager&); /// Not Implemented

  virtual void OnInteractorStyleEvent(int eventid);

  //BTX
  class vtkInternal;
  vtkInternal * Internal;
  //ETX

  int m_DisableInteractorStyleEventsProcessing;

};

#endif

