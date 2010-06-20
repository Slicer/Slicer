#ifndef __qSlicerLayoutManager_p_h
#define __qSlicerLayoutManager_p_h

/// Qt includes
#include <QHash>

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qSlicerLayoutManager.h"

// VTK includes
#include <vtkSmartPointer.h>

// Logic includes
#include "vtkSlicerSliceLogic.h"

class QGridLayout;
class qSlicerSliceViewWidget;
class qMRMLThreeDRenderView;
class vtkObject;
class vtkMRMLLayoutNode;
class vtkMRMLViewNode;
class vtkMRMLSliceNode;

//-----------------------------------------------------------------------------
class qSlicerLayoutManagerPrivate: public QObject, public ctkPrivate<qSlicerLayoutManager>
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qSlicerLayoutManager);
  qSlicerLayoutManagerPrivate();
  ~qSlicerLayoutManagerPrivate();

  void setMRMLScene(vtkMRMLScene* scene);

  /// Instanciate a slice viewer
  QWidget* createSliceView(const QString& sliceViewName, vtkMRMLSliceNode* sliceNode);

  /// Instanciate a 3D viewer
  QWidget* createThreeDRenderView(const QString& renderViewName, vtkMRMLViewNode* viewNode);

  ///
  void initialize();

  /// Enable/disable paint event associated with the TargetWidget
  bool startUpdateLayout();
  void endUpdateLayout(bool updateEnabled);
  
  /// Hide and remove all widgets from the current layout
  void clearLayout();

  /// Convenient function allowing to get a reference to the renderView widget
  /// identified by \a renderViewName
  qMRMLThreeDRenderView* threeDRenderView(const QString& renderViewName);

  /// Convenient function allowing to get a reference to the sliceView widget
  /// identified by \a sliceViewName
  qSlicerSliceViewWidget* sliceView(const QString& sliceViewName);

public slots:
  /// Handle MRML scene event
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
//   void onSceneCloseEvent();

  /// Handle Layout node event
  void onLayoutNodeModifiedEvent(vtkObject* layoutNode);

public:
  vtkMRMLScene*      MRMLScene;
  vtkMRMLLayoutNode* MRMLLayoutNode;
  int                CurrentViewArrangement;
  QGridLayout*       GridLayout;
  QWidget*           TargetWidget;
  
  /// Prevent onLayoutNodeModifiedEvent to be called when
  /// the current MRML layout node is updated from one of the switchTo* slots.
  bool               UpdatingMRMLLayoutNode;

  QHash<QString, qMRMLThreeDRenderView*>                 ThreeDRenderViewMap;
  QHash<QString, qSlicerSliceViewWidget*>                SliceViewMap;
};

#endif
