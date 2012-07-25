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

// qSlicerCPURayCastVolumeRendering includes
#include "qSlicerCPURayCastVolumeRenderingPropertiesWidget.h"
#include "vtkMRMLCPURayCastVolumeRenderingDisplayNode.h"
#include "ui_qSlicerCPURayCastVolumeRenderingPropertiesWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate
  : public Ui_qSlicerCPURayCastVolumeRenderingPropertiesWidget
{
  Q_DECLARE_PUBLIC(qSlicerCPURayCastVolumeRenderingPropertiesWidget);
protected:
  qSlicerCPURayCastVolumeRenderingPropertiesWidget* const q_ptr;

public:
  qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate(
    qSlicerCPURayCastVolumeRenderingPropertiesWidget& object);
  virtual ~qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate();

  virtual void setupUi(qSlicerCPURayCastVolumeRenderingPropertiesWidget*);
  void populateRenderingTechniqueComboBox();
};

// --------------------------------------------------------------------------
qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate
::qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate(
  qSlicerCPURayCastVolumeRenderingPropertiesWidget& object)
  : q_ptr(&object)
{
}

// --------------------------------------------------------------------------
qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate::
~qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate()
{
}

// --------------------------------------------------------------------------
void qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate
::setupUi(qSlicerCPURayCastVolumeRenderingPropertiesWidget* widget)
{
  this->Ui_qSlicerCPURayCastVolumeRenderingPropertiesWidget::setupUi(widget);
  this->populateRenderingTechniqueComboBox();
  QObject::connect(this->RenderingTechniqueComboBox, SIGNAL(currentIndexChanged(int)),
                   widget, SLOT(setRenderingTechnique(int)));
}

// --------------------------------------------------------------------------
void qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate
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
// qSlicerCPURayCastVolumeRenderingPropertiesWidget methods

//-----------------------------------------------------------------------------
qSlicerCPURayCastVolumeRenderingPropertiesWidget
::qSlicerCPURayCastVolumeRenderingPropertiesWidget(QWidget* parentWidget)
  : Superclass( parentWidget )
  , d_ptr( new qSlicerCPURayCastVolumeRenderingPropertiesWidgetPrivate(*this) )
{
  Q_D(qSlicerCPURayCastVolumeRenderingPropertiesWidget);
  d->setupUi(this);
}

//-----------------------------------------------------------------------------
qSlicerCPURayCastVolumeRenderingPropertiesWidget
::~qSlicerCPURayCastVolumeRenderingPropertiesWidget()
{
}

//-----------------------------------------------------------------------------
vtkMRMLCPURayCastVolumeRenderingDisplayNode* qSlicerCPURayCastVolumeRenderingPropertiesWidget
::mrmlCPURayCastDisplayNode()
{
  return vtkMRMLCPURayCastVolumeRenderingDisplayNode::SafeDownCast(
    this->mrmlVolumeRenderingDisplayNode());
}

//-----------------------------------------------------------------------------
void qSlicerCPURayCastVolumeRenderingPropertiesWidget
::updateWidgetFromMRML()
{
  Q_D(qSlicerCPURayCastVolumeRenderingPropertiesWidget);
  if (!this->mrmlCPURayCastDisplayNode())
    {
    return;
    }
  int technique = this->mrmlCPURayCastDisplayNode()->GetRaycastTechnique();
  int index = d->RenderingTechniqueComboBox->findData(QVariant(technique));
  if (index == -1)
    {
    index = 0;
    }
  d->RenderingTechniqueComboBox->setCurrentIndex(index);
}

//-----------------------------------------------------------------------------
void qSlicerCPURayCastVolumeRenderingPropertiesWidget
::setRenderingTechnique(int index)
{
  Q_D(qSlicerCPURayCastVolumeRenderingPropertiesWidget);
  if (!this->mrmlCPURayCastDisplayNode())
    {
    return;
    }
  int technique = d->RenderingTechniqueComboBox->itemData(index).toInt();
  this->mrmlCPURayCastDisplayNode()->SetRaycastTechnique(technique);
}
