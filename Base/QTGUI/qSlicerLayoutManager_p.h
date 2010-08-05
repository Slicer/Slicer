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

class QLayout;
class QGridLayout;
class QButtonGroup;
class qMRMLSliceViewWidget;
class qMRMLThreeDView;
class vtkCollection;
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

  /// If needed, instantiate a slice viewer corresponding to \a sliceViewName
  QWidget* createSliceView(const QString& sliceViewName, vtkMRMLSliceNode* sliceNode);

  /// Delete slice viewer associated with \a sliceNode
  void removeSliceView(vtkMRMLSliceNode* sliceNode);

  /// If needed, instantiate a 3D Viewer corresponding to \a viewNode
  QWidget* createThreeDView(vtkMRMLViewNode* viewNode);

  /// Delete 3D Viewer associated with \a viewNode
  void removeThreeDView(vtkMRMLViewNode* viewNode);

  ///
  void initialize();

  /// Enable/disable paint event associated with the TargetWidget
  bool startUpdateLayout();
  void endUpdateLayout(bool updateEnabled);
  
  /// Hide and remove all widgets from the current layout
  void clearLayout(QLayout* layout);

  /// Convenient function allowing to get a reference to the renderView widget
  /// identified by \a renderViewName
  qMRMLThreeDView* threeDView(int id);

  /// Convenient function allowing to get a reference to the sliceView widget
  /// identified by \a sliceViewName
  qMRMLSliceViewWidget* sliceView(const QString& sliceViewName);

public slots:
  /// Handle MRML scene event
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);
  void onSceneImportedEvent();
  void onSceneAboutToBeClosedEvent();
  void onSceneClosedEvent();

  /// Handle Layout node event
  void onLayoutNodeModifiedEvent(vtkObject* layoutNode);

public:
  vtkMRMLScene*      MRMLScene;
  vtkMRMLLayoutNode* MRMLLayoutNode;
  int                CurrentViewArrangement;
  int                SavedCurrentViewArrangement;
  QGridLayout*       GridLayout;
  QWidget*           TargetWidget;
  QButtonGroup*      SliceControllerButtonGroup;
  vtkCollection*     MRMLSliceLogics;
  
  /// Prevent onLayoutNodeModifiedEvent to be called when
  /// the current MRML layout node is updated from one of the switchTo* slots.
  bool               UpdatingMRMLLayoutNode;

  QList<qMRMLThreeDView*>              ThreeDViewList;
  QList<vtkMRMLViewNode*>              MRMLViewNodeList;

  QHash<QString, qMRMLSliceViewWidget*>      SliceViewMap;
  QHash<vtkMRMLSliceNode*, QString>          MRMLSliceNodeToSliceViewName;
};

#endif
