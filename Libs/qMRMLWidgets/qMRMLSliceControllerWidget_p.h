#ifndef __qMRMLSliceControllerWidget_p_h
#define __qMRMLSliceControllerWidget_p_h

/// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLSliceControllerWidget.h"
#include "ui_qMRMLSliceControllerWidget.h"

// MRMLLogic includes
#include <vtkMRMLSliceLogic.h>

/// VTK includes
#include <vtkSmartPointer.h>
#include <vtkWeakPointer.h>
#include <vtkImageData.h>

class QAction;
class ctkVTKSliceView;
class vtkMRMLSliceNode;
class vtkObject;

//-----------------------------------------------------------------------------
class qMRMLSliceControllerWidgetPrivate: public QObject,
                                   public ctkPrivate<qMRMLSliceControllerWidget>,
                                   public Ui_qMRMLSliceControllerWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:
  CTK_DECLARE_PUBLIC(qMRMLSliceControllerWidget);
  qMRMLSliceControllerWidgetPrivate();
  ~qMRMLSliceControllerWidgetPrivate();

  void setupUi(qMRMLWidget* widget);

  void setupMoreOptionMenu();

public slots:
  /// Update widget state using the associated MRML slice node
  void updateWidgetFromMRMLSliceNode();

  /// Called after a foregound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onForegroundLayerNodeSelected(vtkMRMLNode* node);

  /// Called after a backgound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onBackgroundLayerNodeSelected(vtkMRMLNode* node);

  /// Called after a backgound layer volume node is selected
  /// using the associated qMRMLNodeComboBox
  void onLabelMapNodeSelected(vtkMRMLNode* node);
  
  /// Called after the SliceLogic is modified
  void onSliceLogicModifiedEvent();

  /// Called after the ImageData associated with the SliceLogic is modified
  void onImageDataModifiedEvent();

  void toggleControllerWidgetGroupVisibility();

public:
  vtkMRMLSliceNode*                   MRMLSliceNode;
  vtkMRMLSliceCompositeNode*          MRMLSliceCompositeNode;
  vtkSmartPointer<vtkMRMLSliceLogic>  SliceLogic;
  vtkWeakPointer<vtkImageData>        ImageData;
  QString                             SliceOrientation;
  QHash<QString, QString>             SliceOrientationToDescription;
  QSize                               VTKSliceViewSize;
  QString                             SliceViewName;
  QButtonGroup*                       ControllerButtonGroup;

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
