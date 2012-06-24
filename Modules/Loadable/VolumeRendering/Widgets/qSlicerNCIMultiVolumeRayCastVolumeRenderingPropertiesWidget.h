/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget_h
#define __qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget_h

// SlicerQt includes
#include "qSlicerVolumeRenderingPropertiesWidget.h"
class qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate;
class vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget
  : public qSlicerVolumeRenderingPropertiesWidget
{
  Q_OBJECT
public:
  typedef qSlicerVolumeRenderingPropertiesWidget Superclass;
  qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget(QWidget *parent=0);
  virtual ~qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget();

  vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode* mrmlNCIMultiVolumeRayCastDisplayNode();
protected slots:
  virtual void updateWidgetFromMRML();
  virtual void updateWidgetFromMRMLVolumeNode();

  void setDepthPeelingThreshold(double value);
  void setRenderingTechnique(int index);
  void setRenderingTechniqueFg(int index);
  void setFusion(int index);
  void setBgFgRatio(double value);
protected:
  QScopedPointer<qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget);
  Q_DISABLE_COPY(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget);
};

#endif
