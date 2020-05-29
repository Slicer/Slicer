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

#ifndef __qSlicerCPURayCastVolumeRenderingPropertiesWidget_h
#define __qSlicerCPURayCastVolumeRenderingPropertiesWidget_h

// Slicer includes
#include "qSlicerVolumeRenderingPropertiesWidget.h"
class qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate;
class vtkMRMLCPURayCastVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerCPURayCastVolumeRenderingPropertiesWidget
  : public qSlicerVolumeRenderingPropertiesWidget
{
  Q_OBJECT
public:
  typedef qSlicerVolumeRenderingPropertiesWidget Superclass;
  qSlicerCPURayCastVolumeRenderingPropertiesWidget(QWidget *parent=nullptr);
  ~qSlicerCPURayCastVolumeRenderingPropertiesWidget() override;

  vtkMRMLCPURayCastVolumeRenderingDisplayNode* mrmlCPURayCastDisplayNode();

public slots:
  void setRenderingTechnique(int index);

protected slots:
  void updateWidgetFromMRML() override;

protected:
  QScopedPointer<qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCPURayCastVolumeRenderingPropertiesWidget);
  Q_DISABLE_COPY(qSlicerCPURayCastVolumeRenderingPropertiesWidget);
};

#endif
