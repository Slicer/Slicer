#ifndef __qMRMLSliceViewWidget_p_h
#define __qMRMLSliceViewWidget_p_h

/// Qt includes
#include <QList>

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceViewWidget.h"
#include "ui_qMRMLSliceViewWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class vtkMRMLSliceNode;
class vtkObject;
class QAction;

//-----------------------------------------------------------------------------
class qMRMLSliceViewWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLSliceViewWidget>,
                                   public Ui_qMRMLSliceViewWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLSliceViewWidget);
  qMRMLSliceViewWidgetPrivate();
  ~qMRMLSliceViewWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

  void setMRMLScene(vtkMRMLScene* newScene);
  void setMRMLSliceNode(vtkMRMLSliceNode* newSliceNode);
  void setImageData(vtkImageData* newImageData);

  void setupMoreOptionMenu();

public slots:
  /// Handle MRML scene event
  void onSceneClosingEvent();
  void onSceneCloseEvent();
  void onSceneLoadStartEvent();
  void onSceneLoadEndEvent();
  void onSceneRestoredEvent();
  void onNodeAddedEvent(vtkObject* scene, vtkObject* node);
  void onNodeRemovedEvent(vtkObject* scene, vtkObject* node);

  /// Update widget state using the associated MRML slice node
  void updateWidgetFromMRMLSliceNode();

  /// Triggered after a foregound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onForegroundLayerNodeSelected(vtkMRMLNode* node);

  /// Triggered after a backgound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onBackgroundLayerNodeSelected(vtkMRMLNode* node);

  /// Triggered after a backgound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onLabelMapNodeSelected(vtkMRMLNode* node);
  
  /// Triggered after the SliceLogic is modified
  void onSliceLogicModifiedEvent();

  /// Triggered after the ImageData associated with the SliceLogic is modified
  void onImageDataModifiedEvent();

public:
  vtkMRMLSliceNode*                   MRMLSliceNode;
  vtkMRMLSliceCompositeNode*          MRMLSliceCompositeNode;
  vtkSmartPointer<vtkMRMLSliceLogic>  SliceLogic;
  vtkWeakPointer<vtkImageData>        ImageData;
  QHash<QString, QString>             SliceOrientationToDescription;
  QString                             SliceViewName;

  QAction* actionFitToWindow;
  QAction* actionTotateToVolumePlane;
  QAction* actionAdjustLabelMapOpacity;
  QAction* actionShowLabelVolumeOutlines;
  QAction* actionShowReformatWidget;
  
  QAction* actionCompositingAlphaBlend;
  QAction* actionCompositingRevserseAlphaBlend;
  QAction* actionCompositingAdd;
  QAction* actionCompositingSubtract;

  QAction* actionSliceSpacingModeAutomatic;
  QAction* actionSliceSpacingModeManual;

  QAction* actionLightView1x1;
  QAction* actionLightView1x2;
  QAction* actionLightView1x3;
  QAction* actionLightView1x4;
  QAction* actionLightView1x6;
  QAction* actionLightView1x8;
  QAction* actionLightView2x2;
  QAction* actionLightView3x3;
  QAction* actionLightView6x6;
  
  QAction* actionAdjustDisplayForegroundVolume;
  QAction* actionAdjustDisplayBackgroundVolume;
  QAction* actionAdjustDisplayLabelMapVolume;
  
};

#endif
