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

#ifndef __qSlicerGPURayCastVolumeRenderingPropertiesWidget_h
#define __qSlicerGPURayCastVolumeRenderingPropertiesWidget_h

// Slicer includes
#include "qSlicerVolumeRenderingPropertiesWidget.h"
class qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate;
class vtkMRMLGPURayCastVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerGPURayCastVolumeRenderingPropertiesWidget
  : public qSlicerVolumeRenderingPropertiesWidget
{
  Q_OBJECT
public:
  typedef qSlicerVolumeRenderingPropertiesWidget Superclass;
  qSlicerGPURayCastVolumeRenderingPropertiesWidget(QWidget *parent=nullptr);
  ~qSlicerGPURayCastVolumeRenderingPropertiesWidget() override;

  vtkMRMLGPURayCastVolumeRenderingDisplayNode* mrmlGPURayCastDisplayNode();

public slots:
  void setRenderingTechnique(int index);
  void setSurfaceSmoothing(bool on);

protected slots:
  void updateWidgetFromMRML() override;

protected:
  QScopedPointer<qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerGPURayCastVolumeRenderingPropertiesWidget);
  Q_DISABLE_COPY(qSlicerGPURayCastVolumeRenderingPropertiesWidget);
};

#endif
