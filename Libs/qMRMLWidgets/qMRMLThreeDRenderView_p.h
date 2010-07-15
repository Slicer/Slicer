#ifndef __qMRMLThreeDRenderView_p_h
#define __qMRMLThreeDRenderView_p_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLThreeDRenderView.h"

class vtkMRMLDisplayableManagerGroup;
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

  /// Loop over all CameraNode from the scene and return the one having
  /// its activeTag matching \a viewNode ID
//  vtkMRMLCameraNode* lookUpMRMLCameraNode(vtkMRMLViewNode* viewNode);

public slots:
  /// Handle MRML scene event
  void onSceneClosingEvent();
//  void onSceneClosedEvent();
  void onSceneLoadStartEvent();
  void onSceneLoadEndEvent();
//  void onSceneRestoredEvent();
//  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
//  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);

public:

  vtkMRMLDisplayableManagerGroup*          DisplayableManagerGroup;
  vtkMRMLScene*                            MRMLScene;
  vtkMRMLViewNode*                         MRMLViewNode;
};

#endif
