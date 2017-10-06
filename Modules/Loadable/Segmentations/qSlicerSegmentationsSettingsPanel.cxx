/*==============================================================================

  Program: 3D Slicer

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

// Qt includes
#include <QSettings>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerSegmentationsSettingsPanel.h"
#include "ui_qSlicerSegmentationsSettingsPanel.h"

// --------------------------------------------------------------------------
// qSlicerSegmentationsSettingsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSegmentationsSettingsPanelPrivate: public Ui_qSlicerSegmentationsSettingsPanel
{
  Q_DECLARE_PUBLIC(qSlicerSegmentationsSettingsPanel);
protected:
  qSlicerSegmentationsSettingsPanel* const q_ptr;

public:
  qSlicerSegmentationsSettingsPanelPrivate(qSlicerSegmentationsSettingsPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSegmentationsSettingsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSegmentationsSettingsPanelPrivate
::qSlicerSegmentationsSettingsPanelPrivate(qSlicerSegmentationsSettingsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSegmentationsSettingsPanelPrivate::init()
{
  Q_Q(qSlicerSegmentationsSettingsPanel);

  this->setupUi(q);

  // Default values
  this->AutoOpacitiesCheckBox->setChecked(true);

  // Register settings
  q->registerProperty("Segmentations/AutoOpacities", this->AutoOpacitiesCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Automatically set opacities of the segments based on which contains which, so that no segment obscures another", ctkSettingsPanel::OptionNone);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->AutoOpacitiesCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(setAutoOpacities(bool)));
}

// --------------------------------------------------------------------------
// qSlicerSegmentationsSettingsPanel methods

// --------------------------------------------------------------------------
qSlicerSegmentationsSettingsPanel::qSlicerSegmentationsSettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSegmentationsSettingsPanelPrivate(*this))
{
  Q_D(qSlicerSegmentationsSettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSegmentationsSettingsPanel::~qSlicerSegmentationsSettingsPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSegmentationsSettingsPanel::setAutoOpacities(bool on)
{
  Q_UNUSED(on);
}
