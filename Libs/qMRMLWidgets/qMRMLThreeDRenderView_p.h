#ifndef __qMRMLThreeDRenderView_p_h
#define __qMRMLThreeDRenderView_p_h

// Qt includes
#include <QList>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLThreeDRenderView.h"
#include "vtkMRMLThreeDRenderViewInteractorStyle.h"

// VTK includes
#include <vtkActor.h>
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>

class vtkActor;
class vtkBoundingBox;
class vtkFollower;
class vtkMRMLViewNode;
class vtkMRMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLThreeDRenderViewPrivate: public QObject, public ctkPrivate<qMRMLThreeDRenderView>
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLThreeDRenderView);
  qMRMLThreeDRenderViewPrivate();
  ~qMRMLThreeDRenderViewPrivate();

  void setMRMLScene(vtkMRMLScene* scene);
  void setMRMLViewNode(vtkMRMLViewNode* viewNode);

  ///
  void initialize(vtkMRMLViewNode* viewNode);

  /// Loop over all CameraNode from the scene and return the one having
  /// its activeTag matching \a viewNode ID
  vtkMRMLCameraNode* lookUpMRMLCameraNode(vtkMRMLViewNode* viewNode);
  
  void createAxis();
  void updateAxis();
  void addAxis();

public slots:
  /// Handle MRML scene event
  void onSceneClosingEvent();
  void onSceneCloseEvent();
  void onSceneLoadStartEvent();
  void onSceneLoadEndEvent();
  void onSceneRestoredEvent();
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);

  /// Handle Camera node event
  void onCameraNodeActiveTagModifiedEvent(vtkObject* cameraNode);

  /// Handle View node event
  void onViewNodeModifiedEvent(vtkObject* viewNode);

  /// Handle Displayable node event
  void onDisplayableNodeModifiedEvent(vtkObject* displayableNode);

  /// Handle ClipModels node event
  void onClipModelsNodeModifiedEvent(vtkObject* clipModelsNode);

public:
  QList<vtkSmartPointer<vtkFollower> > AxisLabelActors;
  
  vtkSmartPointer<vtkActor> BoxAxisActor;
  vtkBoundingBox*           BoxAxisBoundingBox;
  vtkMRMLScene*             MRMLScene;
  vtkMRMLViewNode*          MRMLViewNode;
  
  vtkWeakPointer<vtkMRMLThreeDRenderViewInteractorStyle>  InteractorStyle;
};

#endif
