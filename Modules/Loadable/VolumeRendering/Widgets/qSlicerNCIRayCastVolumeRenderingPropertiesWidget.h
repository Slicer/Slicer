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

#ifndef __qSlicerNCIRayCastVolumeRenderingPropertiesWidget_h
#define __qSlicerNCIRayCastVolumeRenderingPropertiesWidget_h

// SlicerQt includes
#include "qSlicerVolumeRenderingPropertiesWidget.h"
class qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate;
class vtkMRMLNCIRayCastVolumeRenderingDisplayNode;

/// \ingroup Slicer_QtModules_VolumeRendering
class Q_SLICER_MODULE_VOLUMERENDERING_WIDGETS_EXPORT qSlicerNCIRayCastVolumeRenderingPropertiesWidget
  : public qSlicerVolumeRenderingPropertiesWidget
{
  Q_OBJECT
public:
  typedef qSlicerVolumeRenderingPropertiesWidget Superclass;
  qSlicerNCIRayCastVolumeRenderingPropertiesWidget(QWidget *parent=0);
  virtual ~qSlicerNCIRayCastVolumeRenderingPropertiesWidget();

  vtkMRMLNCIRayCastVolumeRenderingDisplayNode* mrmlNCIRayCastDisplayNode();

protected slots:
  virtual void updateWidgetFromMRML();
  virtual void updateWidgetFromMRMLVolumeNode();
  void setDepthPeelingThreshold(double value);
  void setDistanceColorBlending(double value);
  void setICPEScale(double value);
  void setICPESmoothness(double value);
  void setRenderingTechnique(int index);

protected:
  QScopedPointer<qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
  Q_DISABLE_COPY(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
};

#endif
