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

// qSlicerGPURayCastVolumeRendering includes
#include "qSlicerGPURayCastVolumeRenderingPropertiesWidget.h"
#include "vtkMRMLGPURayCastVolumeRenderingDisplayNode.h"
#include "ui_qSlicerGPURayCastVolumeRenderingPropertiesWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qSlicerGPURayCastVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qSlicerGPURayCastVolumeRenderingPropertiesWidget);
protected:
  qSlicerGPURayCastVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate(
    qSlicerGPURayCastVolumeRenderingPropertiesWidget& object);
  virtual ~qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate();

  virtual void setupUi(qSlicerGPURayCastVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
};

// --------------------------------------------------------------------------
qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate
::qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate(
  qSlicerGPURayCastVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate::
~qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate
::setupUi(qSlicerGPURayCastVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qSlicerGPURayCastVolumeRenderingPropertiesWidget::setupUi(widget);
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
}

// --------------------------------------------------------------------------
void qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate
::populateRenderingTechniqueComboBox()
{
  this->RenderingTechniqueComboBox->clear();
  this->RenderingTechniqueComboBox->addItem(
    "Composite With Shading", vtkMRMLVolumeRenderingDisplayNode::Composite);
  this->RenderingTechniqueComboBox->addItem(
    "Maximum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MaximumIntensityProjection);
  this->RenderingTechniqueComboBox->addItem(
    "Minimum Intensity Projection",
    vtkMRMLVolumeRenderingDisplayNode::MinimumIntensityProjection);
}

//-----------------------------------------------------------------------------
// qSlicerGPURayCastVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qSlicerGPURayCastVolumeRenderingPropertiesWidget
::qSlicerGPURayCastVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerGPURayCastVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qSlicerGPURayCastVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerGPURayCastVolumeRenderingPropertiesWidget
::~qSlicerGPURayCastVolumeRenderingPropertiesWidget()
{
}

//-----------------------------------------------------------------------------
vtkMRMLGPURayCastVolumeRenderingDisplayNode* qSlicerGPURayCastVolumeRenderingPropertiesWidget
::mrmlGPURayCastDisplayNode()
{
  return vtkMRMLGPURayCastVolumeRenderingDisplayNode::SafeDownCast(
    this->mrmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerGPURayCastVolumeRenderingPropertiesWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerGPURayCastVolumeRenderingPropertiesWidget);
  if (!this->mrmlGPURayCastDisplayNode())
    {
    return;
    }
  int technique = this->mrmlGPURayCastDisplayNode()->GetRaycastTechnique();
  int index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1)
    {
    index = 0;
    }
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qSlicerGPURayCastVolumeRenderingPropertiesWidget
::setRenderingTechnique(int index)
{
  Q_D(qSlicerGPURayCastVolumeRenderingPropertiesWidget);
  if (!this->mrmlGPURayCastDisplayNode())
    {
    return;
    }
  int technique = d->RenderingTechniqueComboBox->itemData(index).toInt();
  this->mrmlGPURayCastDisplayNode()->SetRaycastTechnique(technique);
}
