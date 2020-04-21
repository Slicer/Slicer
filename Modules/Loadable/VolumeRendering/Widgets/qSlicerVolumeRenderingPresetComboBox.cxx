/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// qSlicerVolumeRendering includes
#include "qSlicerVolumeRenderingPresetComboBox.h"
#include "ui_qSlicerVolumeRenderingPresetComboBox.h"
#include "qSlicerPresetComboBox.h"

// Qt includes
#include <QDebug>

// CTK includes
#include <ctkUtils.h>

// MRMLWidgets includes
#include <qMRMLSceneModel.h>

// MRML includes
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLVolumePropertyNode.h>

// VTK includes
#include <vtkWeakPointer.h>
#include <vtkImageData.h>
#include <vtkColorTransferFunction.h>
#include <vtkVolumeProperty.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingPresetComboBoxPrivate : public Ui_qSlicerVolumeRenderingPresetComboBox
{
  Q_DECLARE_PUBLIC(qSlicerVolumeRenderingPresetComboBox);
protected:
  qSlicerVolumeRenderingPresetComboBox* const q_ptr;

public:
  qSlicerVolumeRenderingPresetComboBoxPrivate(qSlicerVolumeRenderingPresetComboBox& object);
  virtual ~qSlicerVolumeRenderingPresetComboBoxPrivate();

  void init();
  void populatePresetsIcons();

  double OldPresetPosition;

  /// Volume property node controlling volume rendering transfer functions.
  /// Its content mirrors the currently selected preset node in the combobox.
  vtkWeakPointer<vtkMRMLVolumePropertyNode> VolumePropertyNode;
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingPresetComboBoxPrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingPresetComboBoxPrivate::qSlicerVolumeRenderingPresetComboBoxPrivate(
  qSlicerVolumeRenderingPresetComboBox& object)
  : q_ptr(&object)
  , OldPresetPosition(0.0)
  , VolumePropertyNode(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingPresetComboBoxPrivate::~qSlicerVolumeRenderingPresetComboBoxPrivate() = default;

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBoxPrivate::init()
{
  Q_Q(qSlicerVolumeRenderingPresetComboBox);

  this->Ui_qSlicerVolumeRenderingPresetComboBox::setupUi(q);

  QObject::connect(this->PresetComboBox, SIGNAL(nodeActivated(vtkMRMLNode*)), q, SLOT(applyPreset(vtkMRMLNode*)));
  QObject::connect(this->PresetComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), q, SIGNAL(currentNodeChanged(vtkMRMLNode*)));
  QObject::connect(this->PresetComboBox, SIGNAL(currentNodeIDChanged(QString)), q, SIGNAL(currentNodeIDChanged(QString)));

  QObject::connect(this->PresetOffsetSlider, SIGNAL(valueChanged(double)),
    q, SLOT(offsetPreset(double)));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(sliderPressed()),
    q, SLOT(startInteraction()));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(valueChanged(double)),
    q, SLOT(interaction()));
  QObject::connect(this->PresetOffsetSlider, SIGNAL(sliderReleased()),
    q, SLOT(endInteraction()));

  this->PresetComboBox->setMRMLScene(nullptr);
  this->PresetComboBox->setCurrentNode(nullptr);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBoxPrivate::populatePresetsIcons()
{
  Q_Q(qSlicerVolumeRenderingPresetComboBox);

  // This is a hack and doesn't work yet
  for (int i = 0; i < this->PresetComboBox->nodeCount(); ++i)
    {
    vtkMRMLNode* presetNode = this->PresetComboBox->nodeFromIndex(i);
    QIcon presetIcon(QString(":/presets/") + presetNode->GetName());
    if (!presetIcon.isNull())
      {
      qMRMLSceneModel* sceneModel = qobject_cast<qMRMLSceneModel*>(
        this->PresetComboBox->sortFilterProxyModel()->sourceModel() );
      sceneModel->setData(sceneModel->indexFromNode(presetNode), presetIcon, Qt::DecorationRole);
      }
    }
}


//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingPresetComboBox methods

// --------------------------------------------------------------------------
qSlicerVolumeRenderingPresetComboBox::qSlicerVolumeRenderingPresetComboBox(QWidget* parentWidget)
  : Superclass(parentWidget)
  , d_ptr(new qSlicerVolumeRenderingPresetComboBoxPrivate(*this))
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerVolumeRenderingPresetComboBox::~qSlicerVolumeRenderingPresetComboBox() = default;

// --------------------------------------------------------------------------
vtkMRMLNode* qSlicerVolumeRenderingPresetComboBox::currentNode()const
{
  Q_D(const qSlicerVolumeRenderingPresetComboBox);
  return d->PresetComboBox->currentNode();
}

// --------------------------------------------------------------------------
QString qSlicerVolumeRenderingPresetComboBox::currentNodeID()const
{
  Q_D(const qSlicerVolumeRenderingPresetComboBox);
  return d->PresetComboBox->currentNodeID();
}

// --------------------------------------------------------------------------
vtkMRMLVolumePropertyNode* qSlicerVolumeRenderingPresetComboBox::mrmlVolumePropertyNode()const
{
  Q_D(const qSlicerVolumeRenderingPresetComboBox);
  return d->VolumePropertyNode;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::setCurrentNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  d->PresetComboBox->setCurrentNode(node);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::setCurrentNodeID(const QString& nodeID)
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  d->PresetComboBox->setCurrentNodeID(nodeID);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::setMRMLVolumePropertyNode(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  d->VolumePropertyNode = vtkMRMLVolumePropertyNode::SafeDownCast(node);

  this->resetOffset();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::startInteraction()
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::StartInteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::endInteraction()
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::EndInteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::interaction()
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  vtkVolumeProperty* volumeProperty = d->VolumePropertyNode->GetVolumeProperty();
  if (volumeProperty)
    {
    volumeProperty->InvokeEvent(vtkCommand::InteractionEvent);
    }
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::offsetPreset(double newPosition)
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  emit presetOffsetChanged(newPosition - d->OldPresetPosition, 0., false);
  d->OldPresetPosition = newPosition;
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::resetOffset()
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);

  // Reset the slider position to the center.
  d->OldPresetPosition = 0.0;
  d->PresetOffsetSlider->setValue(0.0);
  this->updatePresetSliderRange();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::updatePresetSliderRange()
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  if (!d->VolumePropertyNode)
    {
    return;
    }

  if (!d->VolumePropertyNode->GetVolumeProperty())
    {
    return;
    }

  if (d->PresetOffsetSlider->slider()->isSliderDown())
    {
    // Do not change slider range while moving the slider
    return;
    }

  double effectiveRange[2] = { 0.0 };
  d->VolumePropertyNode->GetEffectiveRange(effectiveRange);
  if (effectiveRange[0] > effectiveRange[1])
    {
    if (!d->VolumePropertyNode->CalculateEffectiveRange())
      {
      return; // Do not use undefined effective range
      }
    d->VolumePropertyNode->GetEffectiveRange(effectiveRange);
    }
  double transferFunctionWidth = effectiveRange[1] - effectiveRange[0];

  bool wasBlocking = d->PresetOffsetSlider->blockSignals(true);
  d->PresetOffsetSlider->setRange(-transferFunctionWidth, transferFunctionWidth);
  d->PresetOffsetSlider->setSingleStep(ctk::closestPowerOfTen(transferFunctionWidth)/500.0);
  d->PresetOffsetSlider->setPageStep(d->PresetOffsetSlider->singleStep());
  d->PresetOffsetSlider->blockSignals(wasBlocking);
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::applyPreset(vtkMRMLNode* node)
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);

  if (this->signalsBlocked())
    {
    // Prevent the preset node from overwriting the active volume property node (thus reverting
    // changes in the transfer functions) when the widget's signals are blocked.
    // Needed to handle here, because if the inner combobox's signals are blocked, then the icon
    // is not updated.
    return;
    }

  vtkMRMLVolumePropertyNode* presetNode = vtkMRMLVolumePropertyNode::SafeDownCast(node);
  if (!presetNode || !d->VolumePropertyNode)
    {
    return;
    }

  if ( !presetNode->GetVolumeProperty()
    || !presetNode->GetVolumeProperty()->GetRGBTransferFunction()
    || presetNode->GetVolumeProperty()->GetRGBTransferFunction()->GetRange()[0] >
       presetNode->GetVolumeProperty()->GetRGBTransferFunction()->GetRange()[1] )
    {
    qCritical() << Q_FUNC_INFO << ": Invalid volume property preset node";
    return;
    }

  d->VolumePropertyNode->Copy(presetNode);

  this->resetOffset();
}

// --------------------------------------------------------------------------
bool qSlicerVolumeRenderingPresetComboBox::showIcons()const
{
  Q_D(const qSlicerVolumeRenderingPresetComboBox);
  return d->PresetComboBox->showIcons();
}

// --------------------------------------------------------------------------
void qSlicerVolumeRenderingPresetComboBox::setShowIcons(bool show)
{
  Q_D(qSlicerVolumeRenderingPresetComboBox);
  d->PresetComboBox->setShowIcons(show);
}
