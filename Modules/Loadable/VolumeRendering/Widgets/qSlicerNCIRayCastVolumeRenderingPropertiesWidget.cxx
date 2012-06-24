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

// qSlicerNCIRayCastVolumeRendering includes
#include "qSlicerNCIRayCastVolumeRenderingPropertiesWidget.h"
#include "vtkMRMLNCIRayCastVolumeRenderingDisplayNode.h"
#include "ui_qSlicerNCIRayCastVolumeRenderingPropertiesWidget.h"

// MRML includes
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkImageData.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qSlicerNCIRayCastVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
protected:
  qSlicerNCIRayCastVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate(
    qSlicerNCIRayCastVolumeRenderingPropertiesWidget& object);
  virtual void setupUi(qSlicerNCIRayCastVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
};

// --------------------------------------------------------------------------
qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate
::qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate(
  qSlicerNCIRayCastVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate
::setupUi(qSlicerNCIRayCastVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qSlicerNCIRayCastVolumeRenderingPropertiesWidget::setupUi(widget);
  QObject::connect(this->DepthPeelingSliderWidget, SIGNAL(valueChanged(double)),
                   widget, SLOT(setDepthPeelingThreshold(double)));
  QObject::connect(this->DistColorBlendingSliderWidget, SIGNAL(valueChanged(double)),
                   widget, SLOT(setDistanceColorBlending(double)));
  QObject::connect(this->ICPEScaleSliderWidget, SIGNAL(valueChanged(double)),
                   widget, SLOT(setICPEScale(double)));
  QObject::connect(this->ICPESmoothnessSliderWidget, SIGNAL(valueChanged(double)),
                   widget, SLOT(setICPESmoothness(double)));
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
}

// --------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate
::populateRenderingTechniqueComboBox()
{
  this->RenderingTechniqueComboBox->clear();
  this->RenderingTechniqueComboBox->addItem(
    "Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueComboBox->addItem(
    "Composite Pseudo Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueComboBox->addItem(
    "Maximum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Minimum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Gradient Magnitude Opacity Modulation",
    vtkMRMLNCIRayCastVolumeRenderingDisplayNode::GradiantMagnitudeOpacityModulation);
  this->RenderingTechniqueComboBox->addItem(
    "Illustrative Context Preserving Exploration",
    vtkMRMLNCIRayCastVolumeRenderingDisplayNode::IllustrativeContextPreservingExploration);
}

//-----------------------------------------------------------------------------
// qSlicerNCIRayCastVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qSlicerNCIRayCastVolumeRenderingPropertiesWidget
::qSlicerNCIRayCastVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerNCIRayCastVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerNCIRayCastVolumeRenderingPropertiesWidget
::~qSlicerNCIRayCastVolumeRenderingPropertiesWidget()
{
}

//-----------------------------------------------------------------------------
vtkMRMLNCIRayCastVolumeRenderingDisplayNode*
qSlicerNCIRayCastVolumeRenderingPropertiesWidget::mrmlNCIRayCastDisplayNode()
{
  return vtkMRMLNCIRayCastVolumeRenderingDisplayNode::SafeDownCast(
    this->mrmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
  this->Superclass::updateWidgetFromMRML();
  if (!this->mrmlNCIRayCastDisplayNode())
    {
    return;
    }
  d->DepthPeelingSliderWidget->setValue(
    this->mrmlNCIRayCastDisplayNode() ? this->mrmlNCIRayCastDisplayNode()->GetDepthPeelingThreshold() : 0.);
  d->DistColorBlendingSliderWidget->setValue(
    this->mrmlNCIRayCastDisplayNode() ? this->mrmlNCIRayCastDisplayNode()->GetDistanceColorBlending() : 0.);
  d->ICPEScaleSliderWidget->setValue(
    this->mrmlNCIRayCastDisplayNode() ? this->mrmlNCIRayCastDisplayNode()->GetICPEScale() : 0.);
  d->ICPESmoothnessSliderWidget->setValue(
    this->mrmlNCIRayCastDisplayNode() ? this->mrmlNCIRayCastDisplayNode()->GetICPESmoothness() : 0.);
  int technique = this->mrmlNCIRayCastDisplayNode()->GetRaycastTechnique();
  int index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1)
    {
    index = 0;
    }
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget
::updateWidgetFromMRMLVolumeNode()
{
  Q_D(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
  vtkMRMLVolumeNode* volumeNode = this->mrmlVolumeNode();
  if (!volumeNode)
    {
    return;
    }
  // DepthPeelingThresholdSliderWidget depends on the scalar range of the volume
  // Set the range here before the display node is set and the
  // DepthPeelingThreshold slider value updated.
  vtkImageData* imageData = volumeNode ? volumeNode->GetImageData() : 0;
  if (imageData)
    {
    double range[2];
    imageData->GetScalarRange(range);
    bool oldBlockSignals =
      d->DepthPeelingSliderWidget->blockSignals(true);
    d->DepthPeelingSliderWidget->setRange(range[0], range[1]);
    d->DepthPeelingSliderWidget->blockSignals(oldBlockSignals);
    }
}

// --------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget::setDepthPeelingThreshold(double value)
{
  if (!this->mrmlNCIRayCastDisplayNode())
    {
    return;
    }
  this->mrmlNCIRayCastDisplayNode()->SetDepthPeelingThreshold(value);
}

// --------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget::setDistanceColorBlending(double value)
{
  if (!this->mrmlNCIRayCastDisplayNode())
    {
    return;
    }
  this->mrmlNCIRayCastDisplayNode()->SetDistanceColorBlending(value);
}

// --------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget::setICPEScale(double value)
{
  if (!this->mrmlNCIRayCastDisplayNode())
    {
    return;
    }
  this->mrmlNCIRayCastDisplayNode()->SetICPEScale(value);
}

// --------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget::setICPESmoothness(double value)
{
  if (!this->mrmlNCIRayCastDisplayNode())
    {
    return;
    }
  this->mrmlNCIRayCastDisplayNode()->SetICPESmoothness(value);
}

//-----------------------------------------------------------------------------
void qSlicerNCIRayCastVolumeRenderingPropertiesWidget
::setRenderingTechnique(int index)
{
  Q_D(qSlicerNCIRayCastVolumeRenderingPropertiesWidget);
  if (!this->mrmlNCIRayCastDisplayNode())
    {
    return;
    }
  int technique = d->RenderingTechniqueComboBox->itemData(index).toInt();
  this->mrmlNCIRayCastDisplayNode()->SetRaycastTechnique(technique);
}
