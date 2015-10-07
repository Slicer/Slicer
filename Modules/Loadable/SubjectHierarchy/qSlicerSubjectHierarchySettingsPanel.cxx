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
#include "qSlicerSubjectHierarchySettingsPanel.h"
#include "ui_qSlicerSubjectHierarchySettingsPanel.h"

// --------------------------------------------------------------------------
// qSlicerSubjectHierarchySettingsPanelPrivate

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchySettingsPanelPrivate: public Ui_qSlicerSubjectHierarchySettingsPanel
{
  Q_DECLARE_PUBLIC(qSlicerSubjectHierarchySettingsPanel);
protected:
  qSlicerSubjectHierarchySettingsPanel* const q_ptr;

public:
  qSlicerSubjectHierarchySettingsPanelPrivate(qSlicerSubjectHierarchySettingsPanel& object);
  void init();
};

// --------------------------------------------------------------------------
// qSlicerSubjectHierarchySettingsPanelPrivate methods

// --------------------------------------------------------------------------
qSlicerSubjectHierarchySettingsPanelPrivate
::qSlicerSubjectHierarchySettingsPanelPrivate(qSlicerSubjectHierarchySettingsPanel& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerSubjectHierarchySettingsPanelPrivate::init()
{
  Q_Q(qSlicerSubjectHierarchySettingsPanel);

  this->setupUi(q);

  // Default values
  this->AutoCreateSubjectHierarchyEnabledCheckBox->setChecked(false);
  this->AutoDeleteSubjectHierarchyChildrenEnabledCheckBox->setChecked(false);

  // Register settings
  q->registerProperty("SubjectHierarchy/AutoCreateSubjectHierarchy", this->AutoCreateSubjectHierarchyEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Enable/disable automatic subject hierarchy creation", ctkSettingsPanel::OptionRequireRestart);
  q->registerProperty("SubjectHierarchy/AutoDeleteSubjectHierarchyChildren", this->AutoDeleteSubjectHierarchyChildrenEnabledCheckBox,
                      "checked", SIGNAL(toggled(bool)),
                      "Enable/disable automatic subject hierarchy children deletion", ctkSettingsPanel::OptionRequireRestart);

  // Actions to propagate to the application when settings are changed
  QObject::connect(this->AutoCreateSubjectHierarchyEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(enableAutoCreateSubjectHierarchy(bool)));
  QObject::connect(this->AutoDeleteSubjectHierarchyChildrenEnabledCheckBox, SIGNAL(toggled(bool)),
                   q, SLOT(enableAutoDeleteSubjectHierarchyChildren(bool)));

}

// --------------------------------------------------------------------------
// qSlicerSubjectHierarchySettingsPanel methods

// --------------------------------------------------------------------------
qSlicerSubjectHierarchySettingsPanel::qSlicerSubjectHierarchySettingsPanel(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSubjectHierarchySettingsPanelPrivate(*this))
{
  Q_D(qSlicerSubjectHierarchySettingsPanel);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerSubjectHierarchySettingsPanel::~qSlicerSubjectHierarchySettingsPanel()
{
}

// --------------------------------------------------------------------------
void qSlicerSubjectHierarchySettingsPanel::enableAutoCreateSubjectHierarchy(bool value)
{
  Q_UNUSED(value);
}

// --------------------------------------------------------------------------
void qSlicerSubjectHierarchySettingsPanel::enableAutoDeleteSubjectHierarchyChildren(bool value)
{
  Q_UNUSED(value);
}
