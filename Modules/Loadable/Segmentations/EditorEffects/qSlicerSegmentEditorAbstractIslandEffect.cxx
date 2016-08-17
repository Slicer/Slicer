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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "qSlicerSegmentEditorAbstractIslandEffect.h"

#include "vtkMRMLSegmentationNode.h"
#include "vtkMRMLSegmentEditorNode.h"

// Qt includes
#include <QCheckBox>
#include <QLabel>
#include <QSpinBox>

//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractIslandEffectPrivate methods

//-----------------------------------------------------------------------------
class qSlicerSegmentEditorAbstractIslandEffectPrivate: public QObject
{
  Q_DECLARE_PUBLIC(qSlicerSegmentEditorAbstractIslandEffect);
protected:
  qSlicerSegmentEditorAbstractIslandEffect* const q_ptr;
public:
  typedef QObject Superclass;
  qSlicerSegmentEditorAbstractIslandEffectPrivate(qSlicerSegmentEditorAbstractIslandEffect& object);
  ~qSlicerSegmentEditorAbstractIslandEffectPrivate();

public:
  QCheckBox* FullyConnectedCheckbox;
  QSpinBox* MinimumSizeSpinBox;
};

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractIslandEffectPrivate::qSlicerSegmentEditorAbstractIslandEffectPrivate(qSlicerSegmentEditorAbstractIslandEffect& object)
  : q_ptr(&object)
  , FullyConnectedCheckbox(NULL)
  , MinimumSizeSpinBox(NULL)
{
}

//-----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractIslandEffectPrivate::~qSlicerSegmentEditorAbstractIslandEffectPrivate()
{
}

//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// qSlicerSegmentEditorAbstractIslandEffect methods

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractIslandEffect::qSlicerSegmentEditorAbstractIslandEffect(QObject* parent)
 : Superclass(parent)
 , d_ptr( new qSlicerSegmentEditorAbstractIslandEffectPrivate(*this) )
{
}

//----------------------------------------------------------------------------
qSlicerSegmentEditorAbstractIslandEffect::~qSlicerSegmentEditorAbstractIslandEffect()
{
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractIslandEffect::setupOptionsFrame()
{
  Q_D(qSlicerSegmentEditorAbstractIslandEffect);

  d->FullyConnectedCheckbox = new QCheckBox("Fully connected");
  d->FullyConnectedCheckbox->setToolTip("When set, only pixels that share faces (not corners or edges) are considered connected.");
  this->addOptionsWidget(d->FullyConnectedCheckbox);

  QLabel* sizeLabel = new QLabel("Minimum size");
  sizeLabel->setToolTip("Minimum size of islands to be considered.");
  this->addOptionsWidget(sizeLabel);

  d->MinimumSizeSpinBox = new QSpinBox();
  d->MinimumSizeSpinBox->setMinimum(0);
  d->MinimumSizeSpinBox->setMaximum(VTK_INT_MAX);
  d->MinimumSizeSpinBox->setValue(1000);
  this->addOptionsWidget(d->MinimumSizeSpinBox);

  QObject::connect(d->FullyConnectedCheckbox, SIGNAL(clicked()), this, SLOT(updateMRMLFromGUI()));
  QObject::connect(d->MinimumSizeSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateMRMLFromGUI()));
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractIslandEffect::setMRMLDefaults()
{
  this->setCommonParameterDefault(this->fullyConnectedParameterName(), 1);
  this->setCommonParameterDefault(this->minimumSizeParameterName(), 1000);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractIslandEffect::updateGUIFromMRML()
{
  Q_D(qSlicerSegmentEditorAbstractIslandEffect);
  if (!this->active())
    {
    // updateGUIFromMRML is called when the effect is activated
    return;
    }

  d->FullyConnectedCheckbox->blockSignals(true);
  d->FullyConnectedCheckbox->setChecked(this->integerParameter(this->fullyConnectedParameterName()));
  d->FullyConnectedCheckbox->blockSignals(false);

  d->MinimumSizeSpinBox->blockSignals(true);
  d->MinimumSizeSpinBox->setValue(this->integerParameter(this->minimumSizeParameterName()));
  d->MinimumSizeSpinBox->blockSignals(false);
}

//-----------------------------------------------------------------------------
void qSlicerSegmentEditorAbstractIslandEffect::updateMRMLFromGUI()
{
  Q_D(qSlicerSegmentEditorAbstractIslandEffect);

  this->setCommonParameter(this->fullyConnectedParameterName(), (int)d->FullyConnectedCheckbox->isChecked());
  this->setCommonParameter(this->minimumSizeParameterName(), d->MinimumSizeSpinBox->value());
}
