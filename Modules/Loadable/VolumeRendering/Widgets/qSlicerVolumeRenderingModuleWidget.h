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

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"
#include "qSlicerVolumeRenderingModuleWidgetsExport.h"

class qSlicerVolumeRenderingPropertiesWidget;
class qSlicerVolumeRenderingModuleWidgetPrivate;
class vtkMRMLDisplayableNode;
class vtkMRMLNode;
class vtkMRMLMarkupsROINode;
class vtkMRMLViewNode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumePropertyNode;
class vtkMRMLVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerVolumeRenderingModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT
  QVTK_OBJECT

public:
  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerVolumeRenderingModuleWidget(QWidget *parent=nullptr);
  ~qSlicerVolumeRenderingModuleWidget() override;

  Q_INVOKABLE vtkMRMLVolumeNode* mrmlVolumeNode()const;
  Q_INVOKABLE vtkMRMLDisplayableNode* mrmlROINode()const;
  Q_INVOKABLE vtkMRMLMarkupsROINode* mrmlMarkupsROINode()const;
  Q_INVOKABLE vtkMRMLVolumePropertyNode* mrmlVolumePropertyNode()const;
  Q_INVOKABLE vtkMRMLVolumeRenderingDisplayNode* mrmlDisplayNode()const;

  void addRenderingMethodWidget(const QString& methodClassName, qSlicerVolumeRenderingPropertiesWidget* widget);

  bool setEditedNode(vtkMRMLNode* node, QString role = QString(), QString context = QString()) override;
  double nodeEditable(vtkMRMLNode* node) override;

public slots:
  void setMRMLVolumeNode(vtkMRMLNode* node);
  void setMRMLROINode(vtkMRMLNode* node);
  void setMRMLVolumePropertyNode(vtkMRMLNode* node);

  void fitROIToVolume();

protected slots:
  void onCurrentMRMLVolumeNodeChanged(vtkMRMLNode* node);
  void onVisibilityChanged(bool);
  void onCropToggled(bool);

  void onCurrentMRMLROINodeChanged(vtkMRMLNode* node);
  void onCurrentMRMLVolumePropertyNodeChanged(vtkMRMLNode* node);
  void onNewVolumePropertyAdded(vtkMRMLNode* node);

  void onCurrentRenderingMethodChanged(int index);
  void onCurrentMemorySizeChanged();
  void onCurrentQualityControlChanged(int index);
  void onCurrentFramerateChanged(double fps);
  void onAutoReleaseGraphicsResourcesCheckBoxToggled(bool autoRelease);

  void updateWidgetFromMRML();
  void updateWidgetFromROINode();

  void synchronizeScalarDisplayNode();
  void setFollowVolumeDisplayNode(bool);
  void setIgnoreVolumesThreshold(bool ignore);

  void onThresholdChanged(bool threshold);
  void onROICropDisplayCheckBoxToggled(bool toggle);
  void onChartsExtentChanged();
  void onEffectiveRangeModified();

protected:
  void setup() override;

protected:
  QScopedPointer<qSlicerVolumeRenderingModuleWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerVolumeRenderingModuleWidget);
  Q_DISABLE_COPY(qSlicerVolumeRenderingModuleWidget);
};

#endif
