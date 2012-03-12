#ifndef __qSlicerVolumeRenderingModuleWidget_h
#define __qSlicerVolumeRenderingModuleWidget_h

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerVolumeRenderingModuleExport.h"

class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLAnnotationROINode;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeRenderingDisplayNode;
class vtkMRMLVolumePropertyNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_QTMODULES_VOLUMERENDERING_EXPORT qSlicerVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT
public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeRenderingModuleWidget(QWidget *parent=0);
  virtual ~qSlicerVolumeRenderingModuleWidget();

  vtkMRMLScalarVolumeNode* mrmlVolumeNode()const;
  vtkMRMLVolumeRenderingDisplayNode* mrmlDisplayNode()const;
  vtkMRMLAnnotationROINode* mrmlROINode()const;
  vtkMRMLVolumePropertyNode* mrmlVolumePropertyNode()const;
  QList<vtkMRMLViewNode*> mrmlViewNodes()const;

public slots:

  /// 
  /// Set the MRML node of interest
  void setMRMLVolumeNode(vtkMRMLNode* node);

  void setMRMLDisplayNode(vtkMRMLNode* node);

  void setMRMLROINode(vtkMRMLNode* node);

  void setMRMLVolumePropertyNode(vtkMRMLNode* node);

  void addVolumeIntoView(vtkMRMLNode* node);

  void fitROIToVolume();
  
  void applyPreset(vtkMRMLNode* volumePropertyNode);
protected slots:
  void onCurrentMRMLVolumeNodeChanged(vtkMRMLNode* node);
  void onVisibilityChanged(bool);
  void onCropToggled(bool);

  void onCurrentMRMLDisplayNodeChanged(vtkMRMLNode* node);
  void onCurrentMRMLROINodeChanged(vtkMRMLNode* node);
  void onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode* node);
  void onCheckedViewNodesChanged();

  void onCurrentRenderingMethodChanged(int index);
  void onCurrentMemorySizeChanged(int index);
  void onCurrentQualityControlChanged(int index);
  void onCurrentFramerateChanged(double fps);
  void onCurrentRenderingTechniqueChanged(int index);

  void onCurrentDistanceColorBlendingChanged(double value);
  void onCurrentICPEScaleChanged(double value);
  void onCurrentICPESmoothnessChanged(double value);
  void onCurrentDepthPeelingThreshold(double value);
  void onCurrentRenderingTechniqueFgChanged(int index);
  void onCurrentFusionChanged(int index);
  void onCurrentBgFgRatioChanged(double value);

  void startInteraction();
  void endInteraction();
  void interaction();

  void offsetPreset(double newPosition);
  void resetOffset();
  void updatePresetSliderRange();
  void updateFromMRMLDisplayNode();

  void synchronizeScalarDisplayNode();
  void setFollowVolumeDisplayNode(bool);
  void setIgnoreVolumesThreshold(bool ignore);

  void onThresholdChanged(bool threshold);
  void onROICropDisplayCheckBoxToggled(bool toggle);

protected:
  QScopedPointer<qSlicerVolumeRenderingModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModuleWidget);
};

#endif
