/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Yarmakovich, Isomics Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerVolumeRenderingModuleWidget_h
#define __qSlicerVolumeRenderingModuleWidget_h

// CTK includes
#include <ctkVTKObject.h>

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerVolumeRenderingModuleWidgetsExport.h"

class qSlicerVolumeRenderingPropertiesWidget;
class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLAnnotationROINode;
class vtkMRMLNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeRenderingDisplayNode;
class vtkMRMLVolumePropertyNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerVolumeRenderingModuleWidget :
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

  void addRenderingMethodWidget(const QString& methodClassName,
                                qSlicerVolumeRenderingPropertiesWidget* widget);

public slots:

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

  void onCurrentRenderingMethodChanged(int index);
  void onCurrentMemorySizeChanged(int index);
  void onCurrentQualityControlChanged(int index);
  void onCurrentFramerateChanged(double fps);

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
