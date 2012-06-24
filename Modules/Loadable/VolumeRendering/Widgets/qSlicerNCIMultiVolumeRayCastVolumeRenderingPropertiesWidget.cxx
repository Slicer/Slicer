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

// qSlicerNCIMultiVolumeRayCastVolumeRendering includes
#include "qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget.h"
#include "vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode.h"
#include "ui_qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget.h"

// MRML includes
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkImageData.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget);
protected:
  qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate(
    qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget& object);
  virtual void setupUi(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
  void populateRenderingTechniqueFgComboBox();
};

// --------------------------------------------------------------------------
qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate
::qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate(
  qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate
::setupUi(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::setupUi(widget);

  QObject::connect(this->DepthPeelingSliderWidget, SIGNAL(valueChanged(double)),
                   widget, SLOT(setDepthPeelingThreshold(double)));
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
  this->populateRenderingTechniqueFgComboBox();
  QObject::connect(this->RenderingTechniqueFgComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechniqueFg(int)));
  QObject::connect(this->FusionComboBox, SIGNAL(currentIndexChanged(double)),
                   widget, SLOT(setFusion(int)));
  QObject::connect(this->BgFgRatioSliderWidget, SIGNAL(valueChanged(double)),
                   widget, SLOT(setBgFgRatioSliderWidget(double)));
}

// --------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate
::populateRenderingTechniqueComboBox()
{
  this->RenderingTechniqueComboBox->clear();
  this->RenderingTechniqueComboBox->addItem(
    "Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueComboBox->addItem(
    "Composite Pseudo Shading", vtkMRMLVolumeRenderingDisplayNode::CompositeEdgeColoring);
  this->RenderingTechniqueComboBox->addItem(
    "Maximum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Minimum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Gradient Magnitude Opacity Modulation",
    vtkMRMLVolumeRenderingDisplayNode::GradiantMagnitudeOpacityModulation);
  this->RenderingTechniqueComboBox->addItem(
    "Illustrative Context Preserving Exploration",
    vtkMRMLVolumeRenderingDisplayNode::IllustrativeContextPreservingExploration);
}

// --------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate
::populateRenderingTechniqueFgComboBox()
{
  this->RenderingTechniqueFgComboBox->clear();
  this->RenderingTechniqueFgComboBox->addItem(
    "Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueFgComboBox->addItem(
    "Composite Pseudo Shading", vtkMRMLVolumeRenderingDisplayNode::CompositeEdgeColoring);
  this->RenderingTechniqueFgComboBox->addItem(
    "Maximum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
  this->RenderingTechniqueFgComboBox->addItem(
    "Minimum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
  this->RenderingTechniqueFgComboBox->addItem(
    "Gradient Magnitude Opacity Modulation",
    vtkMRMLVolumeRenderingDisplayNode::GradiantMagnitudeOpacityModulation);
  this->RenderingTechniqueFgComboBox->addItem(
    "Illustrative Context Preserving Exploration",
    vtkMRMLVolumeRenderingDisplayNode::IllustrativeContextPreservingExploration);
}

//-----------------------------------------------------------------------------
// qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget
::qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget
::~qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget()
{
}

//-----------------------------------------------------------------------------
vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode*
qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::mrmlNCIMultiVolumeRayCastDisplayNode()
{
  return vtkMRMLNCIMultiVolumeRayCastVolumeRenderingDisplayNode::SafeDownCast(
    this->mrmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget);
   d->DepthPeelingSliderWidget->setValue(
    this->mrmlNCIMultiVolumeRayCastDisplayNode() ?
    this->mrmlNCIMultiVolumeRayCastDisplayNode()->GetDepthPeelingThreshold() : 0.);
 if (!this->mrmlNCIMultiVolumeRayCastDisplayNode())
    {
    return;
    }
  int technique = this->mrmlNCIMultiVolumeRayCastDisplayNode()->GetRaycastTechnique();
  int index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1)
    {
    index = 0;
    }
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
  int techniqueFg =
    this->mrmlNCIMultiVolumeRayCastDisplayNode()->GetRaycastTechniqueFg();
  int indexFg = d->RenderingTechniqueFgComboBox->findData(QVariant(techniqueFg));
  d->RenderingTechniqueFgComboBox->setCurrentIndex(indexFg);
  d->FusionComboBox->setCurrentIndex(
    this->mrmlNCIMultiVolumeRayCastDisplayNode()->GetMultiVolumeFusionMethod());
  d->BgFgRatioSliderWidget->setValue(
    this->mrmlNCIMultiVolumeRayCastDisplayNode()->GetBgFgRatio());
}

//-----------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget
::updateWidgetFromMRMLVolumeNode()
{
  Q_D(qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget);
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
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::setDepthPeelingThreshold(double value)
{
  if (!this->mrmlNCIMultiVolumeRayCastDisplayNode())
    {
    return;
    }
  this->mrmlNCIMultiVolumeRayCastDisplayNode()->SetDepthPeelingThreshold(value);
}

//-----------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::setRenderingTechnique(int index)
{
  if (!this->mrmlNCIMultiVolumeRayCastDisplayNode())
    {
    return;
    }

  this->mrmlNCIMultiVolumeRayCastDisplayNode()->SetRaycastTechnique(index);
}

//-----------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::setRenderingTechniqueFg(int index)
{
  if (!this->mrmlNCIMultiVolumeRayCastDisplayNode())
    {
    return;
    }

  this->mrmlNCIMultiVolumeRayCastDisplayNode()->SetRaycastTechniqueFg(index);
}

//-----------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::setFusion(int index)
{
  if (!this->mrmlNCIMultiVolumeRayCastDisplayNode())
    {
    return;
    }

  this->mrmlNCIMultiVolumeRayCastDisplayNode()->SetMultiVolumeFusionMethod(index);
}

//-----------------------------------------------------------------------------
void qSlicerNCIMultiVolumeRayCastVolumeRenderingPropertiesWidget::setBgFgRatio(double value)
{
  if (!this->mrmlNCIMultiVolumeRayCastDisplayNode())
    {
    return;
    }

  this->mrmlNCIMultiVolumeRayCastDisplayNode()->SetBgFgRatio(value);
}
