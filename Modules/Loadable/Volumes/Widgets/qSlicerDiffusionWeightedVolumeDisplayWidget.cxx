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

#include "qSlicerDiffusionWeightedVolumeDisplayWidget.h"
#include "ui_qSlicerDiffusionWeightedVolumeDisplayWidget.h"

// Qt includes

// MRML includes
#include "vtkMRMLDiffusionWeightedVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"

// VTK includes
#include <vtkImageData.h>

// STD includes

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate
  : public Ui_qSlicerDiffusionWeightedVolumeDisplayWidget
{
  Q_DECLARE_PUBLIC(qSlicerDiffusionWeightedVolumeDisplayWidget);
protected:
  qSlicerDiffusionWeightedVolumeDisplayWidget* const q_ptr;
public:
  qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate(qSlicerDiffusionWeightedVolumeDisplayWidget& object);
  ~qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate();
  void init();
  vtkWeakPointer<vtkMRMLDiffusionWeightedVolumeNode> VolumeNode;
};

//-----------------------------------------------------------------------------
qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate
::qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate(
  qSlicerDiffusionWeightedVolumeDisplayWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate
::~qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate::init()
{
  Q_Q(qSlicerDiffusionWeightedVolumeDisplayWidget);

  this->setupUi(q);

  QObject::connect(this->DWIComponentSlider, SIGNAL(valueChanged(int)),
                   q, SLOT(setDWIComponent(int)));
}

// --------------------------------------------------------------------------
qSlicerDiffusionWeightedVolumeDisplayWidget
::qSlicerDiffusionWeightedVolumeDisplayWidget(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerDiffusionWeightedVolumeDisplayWidgetPrivate(*this))
{
  Q_D(qSlicerDiffusionWeightedVolumeDisplayWidget);
  d->init();

  // disable as there is not MRML Node associated with the widget
  this->setEnabled(false);
}

// --------------------------------------------------------------------------
qSlicerDiffusionWeightedVolumeDisplayWidget
::~qSlicerDiffusionWeightedVolumeDisplayWidget() = default;

// --------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeNode* qSlicerDiffusionWeightedVolumeDisplayWidget
::volumeNode()const
{
  Q_D(const qSlicerDiffusionWeightedVolumeDisplayWidget);
  return d->VolumeNode;
}

// --------------------------------------------------------------------------
vtkMRMLDiffusionWeightedVolumeDisplayNode* qSlicerDiffusionWeightedVolumeDisplayWidget
::volumeDisplayNode()const
{
  vtkMRMLDiffusionWeightedVolumeNode* volumeNode = this->volumeNode();
  return volumeNode ? vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(
    volumeNode->GetDisplayNode()) : nullptr;
}

// --------------------------------------------------------------------------
void qSlicerDiffusionWeightedVolumeDisplayWidget::setMRMLVolumeNode(vtkMRMLNode* node)
{
  this->setMRMLVolumeNode(vtkMRMLDiffusionWeightedVolumeNode::SafeDownCast(node));
}

// --------------------------------------------------------------------------
void qSlicerDiffusionWeightedVolumeDisplayWidget
::setMRMLVolumeNode(vtkMRMLDiffusionWeightedVolumeNode* volumeNode)
{
  Q_D(qSlicerDiffusionWeightedVolumeDisplayWidget);

  vtkMRMLDiffusionWeightedVolumeDisplayNode* oldVolumeDisplayNode = this->volumeDisplayNode();

  qvtkReconnect(oldVolumeDisplayNode, volumeNode ? volumeNode->GetDisplayNode() :nullptr,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromDisplayNode()));
  qvtkReconnect(d->VolumeNode, volumeNode,
                vtkCommand::ModifiedEvent,
                this, SLOT(updateWidgetFromVolumeNode()));

  d->VolumeNode = volumeNode;
  d->ScalarVolumeDisplayWidget->setMRMLVolumeNode(volumeNode);

  this->updateWidgetFromVolumeNode();
  this->updateWidgetFromDisplayNode();
}

// --------------------------------------------------------------------------
void qSlicerDiffusionWeightedVolumeDisplayWidget::updateWidgetFromVolumeNode()
{
  Q_D(qSlicerDiffusionWeightedVolumeDisplayWidget);
  this->setEnabled(d->VolumeNode != nullptr);
  if (!d->VolumeNode)
    {
    return;
    }
  int maxRange = d->VolumeNode->GetImageData() ?
    d->VolumeNode->GetImageData()->GetNumberOfScalarComponents() - 1 : 0;
  // we save the component here, as changing the range of the slider/spinbox
  // can change the component value. We want to set it back.
  vtkMRMLDiffusionWeightedVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();

  int component = displayNode ? displayNode->GetDiffusionComponent() : d->DWIComponentSlider->value();
  bool sliderWasBlocking = d->DWIComponentSlider->blockSignals(true);
  bool spinBoxWasBlocking = d->DWIComponentSpinBox->blockSignals(true);
  d->DWIComponentSlider->setRange(0, maxRange);
  d->DWIComponentSpinBox->setRange(0, maxRange);
  d->DWIComponentSlider->blockSignals(sliderWasBlocking);
  d->DWIComponentSpinBox->blockSignals(spinBoxWasBlocking);
  d->DWIComponentSlider->setValue(component);
}

// --------------------------------------------------------------------------
void qSlicerDiffusionWeightedVolumeDisplayWidget::updateWidgetFromDisplayNode()
{
  Q_D(qSlicerDiffusionWeightedVolumeDisplayWidget);
  vtkMRMLDiffusionWeightedVolumeDisplayNode* displayNode =
    this->volumeDisplayNode();
  if (!displayNode)
    {
    return;
    }
  d->DWIComponentSlider->setValue(displayNode->GetDiffusionComponent());
}


//----------------------------------------------------------------------------
void qSlicerDiffusionWeightedVolumeDisplayWidget::setDWIComponent(int component)
{
  vtkMRMLDiffusionWeightedVolumeDisplayNode* volumeDisplayNode = this->volumeDisplayNode();
  if (!volumeDisplayNode)
    {
    return;
    }
  volumeDisplayNode->SetDiffusionComponent(component);
}
