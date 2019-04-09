/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

 Module:    $RCSfile: vtkMRMLAnnotationDisplayableManager.h,v $
 Date:      $Date: 2010/07/26 04:48:05 $
 Version:   $Revision: 1.2 $

 =========================================================================auto=*/

#ifndef __vtkMRMLAnnotationDisplayableManager_h
#define __vtkMRMLAnnotationDisplayableManager_h

// Annotations includes
#include "vtkSlicerAnnotationsModuleMRMLDisplayableManagerExport.h"
class vtkMRMLAnnotationClickCounter;
class vtkMRMLAnnotationDisplayNode;
class vtkMRMLAnnotationDisplayableManagerHelper;
class vtkMRMLAnnotationLineDisplayNode;
class vtkMRMLAnnotationNode;
class vtkMRMLAnnotationPointDisplayNode;
class vtkSlicerViewerWidget;

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>

// MRML includes
class vtkMRMLSliceNode;

// VTK includes
class vtkAbstractWidget;
class vtkHandleWidget;
class vtkSeedWidget;

/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT
vtkMRMLAnnotationDisplayableManager
  : public vtkMRMLAbstractDisplayableManager
{
public:

  static vtkMRMLAnnotationDisplayableManager *New();
  vtkTypeMacro(vtkMRMLAnnotationDisplayableManager, vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // the following functions must be public to be accessible by the callback
  /// Propagate properties of MRML node to widget.
  virtual void PropagateMRMLToWidget(vtkMRMLAnnotationNode* node, vtkAbstractWidget * widget);
  /// Propagate properties of widget to MRML node.
  virtual void PropagateWidgetToMRML(vtkAbstractWidget * widget, vtkMRMLAnnotationNode* node);
  /// Check if this is a 2d SliceView displayable manager, returns true if so,
  /// false otherwise. Checks return from GetSliceNode for non null, which means
  /// it's a 2d displayable manager
  virtual bool Is2DDisplayableManager();
  /// Get the sliceNode, if registered. This would mean it is a 2D SliceView displayableManager.
  vtkMRMLSliceNode * GetSliceNode();

  /// Check if the displayCoordinates are inside the viewport and if not, correct the displayCoordinates
  void RestrictDisplayCoordinatesToViewport(double* displayCoordinates);

  /// Check if there are real changes between two sets of displayCoordinates
  bool GetDisplayCoordinatesChanged(double * displayCoordinates1, double * displayCoordinates2);

  /// Check if there are real changes between two sets of worldCoordinates
  bool GetWorldCoordinatesChanged(double * worldCoordinates1, double * worldCoordinates2);

  /// Convert display to world coordinates
  void GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates);
  void GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates);

  /// Convert world coordinates to local using mrml parent transform
  virtual void GetWorldToLocalCoordinates(vtkMRMLAnnotationNode *node,
                                  double *worldCoordinates, double *localCoordinates);

  /// Set mrml parent transform to widgets
  virtual void SetParentTransformToWidget(vtkMRMLAnnotationNode *vtkNotUsed(node), vtkAbstractWidget *vtkNotUsed(widget)){}

  /// Set/Get the 2d scale factor to divide 3D scale by to show 2D elements appropriately (usually set to 300)
  vtkSetMacro(ScaleFactor2D, double);
  vtkGetMacro(ScaleFactor2D, double);

  /// Return true if in lightbox mode - there is a slice node that has layout
  /// grid columns or rows > 1
  bool IsInLightboxMode();

  /// Gets the world coordinate of the annotation node, transforms it to
  /// display coordinates.
  /// Defaults to returning the 0th control point's light box index. Returns
  /// -1 if not in lightbox mode.
  int GetLightboxIndex(vtkMRMLAnnotationNode *node, int controlPointIndex = 0);

  /// Set up data structures for an annotation node.  Returns false on failure
  /// or if it's already set up. Can be called to reinitialise a node's widgets
  /// after calling RemoveWidgetAndNode on the Helper
  /// \sa vtkMRMLAnnotationDisplayableManagerHelper::RemoveWidgetAndNode()
  bool AddAnnotation(vtkMRMLAnnotationNode *node);

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

protected:

  vtkMRMLAnnotationDisplayableManager();
  ~vtkMRMLAnnotationDisplayableManager() override;

  void ProcessMRMLNodesEvents(vtkObject *caller, unsigned long event, void *callData) override;

  void Create() override;

  /// wrap the superclass render request in a check for batch processing
  virtual void RequestRender();

  /// Remove MRML observers
  void RemoveMRMLObservers() override;

  /// Called from RequestRender method if UpdateFromMRMLRequested is true
  /// \sa RequestRender() SetUpdateFromMRMLRequested()
  void UpdateFromMRML() override;

  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Called after the corresponding MRML event is triggered, from AbstractDisplayableManager
  /// \sa ProcessMRMLSceneEvents
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;

  /// Called after the corresponding MRML View container was modified
  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Handler for specific SliceView actions
  virtual void OnMRMLSliceNodeModifiedEvent(vtkMRMLSliceNode * sliceNode);

  /// Check, if the widget is displayable in the current slice geometry
  virtual bool IsWidgetDisplayable(vtkMRMLSliceNode * sliceNode, vtkMRMLAnnotationNode* node);

  /// Observe one node
  void SetAndObserveNode(vtkMRMLAnnotationNode *annotationNode);
  /// Observe all associated nodes.
  void SetAndObserveNodes();

  /// Observe the interaction node.
  void AddObserversToInteractionNode();
  void RemoveObserversFromInteractionNode();

  /// Observe the selection node for:
  ///    * vtkMRMLSelectionNode::UnitModifiedEvent
  /// events to update the unit of the annotation nodes.
  /// \sa RemoveObserversFromSelectionNode(), AddObserversToInteractionNode(),
  /// OnMRMLSelectionNodeUnitModifiedEvent()
  void AddObserversToSelectionNode();
  void RemoveObserversFromSelectionNode();

  /// Preset functions for certain events.
  virtual void OnMRMLAnnotationNodeModifiedEvent(vtkMRMLNode* node);
  virtual void OnMRMLAnnotationNodeTransformModifiedEvent(vtkMRMLNode* node);
  virtual void OnMRMLAnnotationNodeLockModifiedEvent(vtkMRMLNode* node);
  virtual void OnMRMLAnnotationDisplayNodeModifiedEvent(vtkMRMLNode *node);
  virtual void OnMRMLAnnotationControlPointModifiedEvent(vtkMRMLNode *node);
  virtual void OnMRMLSelectionNodeUnitModifiedEvent(vtkMRMLSelectionNode*) {}

  //
  // Handling of interaction within the RenderWindow
  //

  // Get the coordinates of a click in the RenderWindow
  void OnClickInRenderWindowGetCoordinates();
  /// Callback for click in RenderWindow
  virtual void OnClickInRenderWindow(double x, double y, const char *associatedNodeID = nullptr);
  /// Counter for clicks in Render Window
  vtkMRMLAnnotationClickCounter* m_ClickCounter;

  /// Update just the position for the widget, implemented by subclasses.
  virtual void UpdatePosition(vtkAbstractWidget *vtkNotUsed(widget), vtkMRMLNode *vtkNotUsed(node)) {}
  //
  // Seeds for widget placement
  //

  /// Place a seed for widgets
  virtual void PlaceSeed(double x, double y);
  /// Return the placed seeds
  vtkHandleWidget * GetSeed(int index);

  //
  // Coordinate Conversions
  //

  /// Convert display to world coordinates
//  void GetDisplayToWorldCoordinates(double x, double y, double * worldCoordinates);
//  void GetDisplayToWorldCoordinates(double * displayCoordinates, double * worldCoordinates);

  /// Convert display to world coordinates
  void GetWorldToDisplayCoordinates(double r, double a, double s, double * displayCoordinates);
  void GetWorldToDisplayCoordinates(double * worldCoordinates, double * displayCoordinates);

  /// Convert display to viewport coordinates
  void GetDisplayToViewportCoordinates(double x, double y, double * viewportCoordinates);
  void GetDisplayToViewportCoordinates(double *displayCoordinates, double * viewportCoordinates);

  //
  // Widget functionality
  //

  /// Create a widget.
  virtual vtkAbstractWidget * CreateWidget(vtkMRMLAnnotationNode* node);
  /// Gets called when widget was created
  virtual void OnWidgetCreated(vtkAbstractWidget * widget, vtkMRMLAnnotationNode * node);
  /// Get the widget of a node.
  vtkAbstractWidget * GetWidget(vtkMRMLAnnotationNode * node);

  /// Check if it is the right displayManager
  bool IsCorrectDisplayableManager();

  /// Return true if this displayable manager supports(can manage) that node,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(const char*), IsCorrectDisplayableManager()
  virtual bool IsManageable(vtkMRMLNode* node);
  /// Return true if this displayable manager supports(can manage) that node class,
  /// false otherwise.
  /// Can be reimplemented to add more conditions.
  /// \sa IsManageable(vtkMRMLNode*), IsCorrectDisplayableManager()
  virtual bool IsManageable(const char* nodeClassName);

  /// Focus of this displayableManager is set to a specific annotation type when inherited
  const char* m_Focus;

  /// Disable processing when updating is in progress.
  int m_Updating;

  /// Respond to interactor style events
  void OnInteractorStyleEvent(int eventid) override;

  /// Accessor for internal flag that disables interactor style event processing
  vtkGetMacro(DisableInteractorStyleEventsProcessing, int);

  vtkMRMLAnnotationDisplayableManagerHelper * Helper;

  double LastClickWorldCoordinates[4];

private:

  vtkMRMLAnnotationDisplayableManager(const vtkMRMLAnnotationDisplayableManager&) = delete;
  void operator=(const vtkMRMLAnnotationDisplayableManager&) = delete;


  int DisableInteractorStyleEventsProcessing;

  /// Scale factor for 2d windows
  double ScaleFactor2D;
};

#endif
