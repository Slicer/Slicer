#ifndef __qMRMLThreeDView_p_h
#define __qMRMLThreeDView_p_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLThreeDView.h"

class vtkMRMLThreeDViewDisplayableManagerGroup;
class vtkMRMLViewNode;
class vtkMRMLCameraNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLThreeDViewPrivate: public QObject, public ctkPrivate<qMRMLThreeDView>
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLThreeDView);
  qMRMLThreeDViewPrivate();
  ~qMRMLThreeDViewPrivate();

  void setMRMLScene(vtkMRMLScene* scene);

  /// Loop over all CameraNode from the scene and return the one having
  /// its activeTag matching \a viewNode ID
//  vtkMRMLCameraNode* lookUpMRMLCameraNode(vtkMRMLViewNode* viewNode);

public slots:
  /// Handle MRML scene event
  void onSceneAboutToBeClosedEvent();
  void onSceneAboutToBeImportedEvent();
  void onSceneImportedEvent();
//  void onSceneRestoredEvent();

public:

  vtkMRMLThreeDViewDisplayableManagerGroup*  DisplayableManagerGroup;
  vtkMRMLScene*                              MRMLScene;
  vtkMRMLViewNode*                           MRMLViewNode;
};

#endif
