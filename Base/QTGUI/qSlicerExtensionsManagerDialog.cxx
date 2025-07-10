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

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QCloseEvent>
#include <QPushButton>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsManagerDialog.h"
#include "qSlicerExtensionsManagerModel.h"
#include "qSlicerSettingsExtensionsPanel.h"
#include "ui_qSlicerExtensionsManagerDialog.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsManagerDialogPrivate : public Ui_qSlicerExtensionsManagerDialog
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManagerDialog);

protected:
  qSlicerExtensionsManagerDialog* const q_ptr;

public:
  qSlicerExtensionsManagerDialogPrivate(qSlicerExtensionsManagerDialog& object);
  void init();
  void updateButtons();

  bool RestartRequested;

  QStringList PreviousModulesAdditionalPaths;
  QStringList PreviousExtensionsScheduledForUninstall;
  QVariantMap PreviousExtensionsScheduledForUpdate;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerDialogPrivate::qSlicerExtensionsManagerDialogPrivate(qSlicerExtensionsManagerDialog& object)
  : q_ptr(&object)
  , RestartRequested(false)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialogPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerDialog);

  this->setupUi(q);

  QObject::connect(this->ExtensionsManagerWidget, SIGNAL(inBatchProcessing(bool)), q, SLOT(onBatchProcessingChanged()));

  QPushButton* restartButton = this->ButtonBox->button(QDialogButtonBox::Ok);
  restartButton->setText(qSlicerExtensionsManagerDialog::tr("Restart"));

  q->setRestartRequested(false);

  // Assuming the dialog is instantiated prior any update of the settings,
  // keeping track of settings will allow us to display the "RestartRequestedLabel"
  // only if it applies. Note also that keep track of "EnvironmentVariables/PYTHONPATH'
  // isn't required, "Modules/AdditionalPaths" is enough to know if we should restart.
  QSettings* settings = qSlicerCoreApplication::application()->revisionUserSettings();
  // this->PreviousModulesAdditionalPaths contain the raw (relative or absolute) paths, not converted to absolute
  this->PreviousModulesAdditionalPaths = settings->value("Modules/AdditionalPaths").toStringList();
  this->PreviousExtensionsScheduledForUninstall = settings->value("Extensions/ScheduledForUninstall").toStringList();
  this->PreviousExtensionsScheduledForUpdate = settings->value("Extensions/ScheduledForUpdate").toMap();

  qSlicerSettingsExtensionsPanel* extensionsPanel = qobject_cast<qSlicerSettingsExtensionsPanel*>(
    qSlicerApplication::application()->settingsDialog()->panel("Extensions"));
  Q_ASSERT(extensionsPanel);
  if (extensionsPanel)
  {
    QObject::connect(extensionsPanel,
                     SIGNAL(extensionsServerUrlChanged(QString)),
                     this->ExtensionsManagerWidget,
                     SLOT(refreshInstallWidget()));
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialogPrivate::updateButtons()
{
  Q_Q(qSlicerExtensionsManagerDialog);
  Q_ASSERT(q->extensionsManagerModel());
  bool shouldRestart = false;
  qSlicerCoreApplication* coreApp = qSlicerCoreApplication::application();
  // this->PreviousModulesAdditionalPaths contain the raw (relative or absolute) paths, not converted to absolute
  if (this->PreviousModulesAdditionalPaths
        != coreApp->revisionUserSettings()->value("Modules/AdditionalPaths").toStringList()
      || this->PreviousExtensionsScheduledForUninstall
           != coreApp->revisionUserSettings()->value("Extensions/ScheduledForUninstall").toStringList()
      || this->PreviousExtensionsScheduledForUpdate
           != coreApp->revisionUserSettings()->value("Extensions/ScheduledForUpdate").toMap())
  {
    shouldRestart = true;
  }
  bool isInBatchMode = this->ExtensionsManagerWidget->isInBatchProcessing();

  this->ButtonBox->setEnabled(!isInBatchMode);
  q->setRestartRequested(shouldRestart);
}

// --------------------------------------------------------------------------
// qSlicerExtensionsManagerDialog methods

// --------------------------------------------------------------------------
qSlicerExtensionsManagerDialog::qSlicerExtensionsManagerDialog(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsManagerDialogPrivate(*this))
{
  Q_D(qSlicerExtensionsManagerDialog);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerDialog::~qSlicerExtensionsManagerDialog() = default;

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsManagerDialog::extensionsManagerModel() const
{
  Q_D(const qSlicerExtensionsManagerDialog);
  return d->ExtensionsManagerWidget->extensionsManagerModel();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsManagerDialog);

  if (this->extensionsManagerModel() == model)
  {
    return;
  }

  disconnect(this, SLOT(onModelUpdated()));

  d->ExtensionsManagerWidget->setExtensionsManagerModel(model);

  if (model)
  {
    this->onModelUpdated();
    connect(model, SIGNAL(modelUpdated()), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionInstalled(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionScheduledForUninstall(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionCancelledScheduleForUninstall(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionScheduledForUpdate(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionCancelledScheduleForUpdate(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionEnabledChanged(QString, bool)), this, SLOT(onModelUpdated()));
  }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerDialog::restartRequested() const
{
  Q_D(const qSlicerExtensionsManagerDialog);
  return d->RestartRequested;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::setRestartRequested(bool value)
{
  Q_D(qSlicerExtensionsManagerDialog);
  d->RestartRequested = value;
  d->RestartRequestedLabel->setVisible(value);
  d->ButtonBox->button(QDialogButtonBox::Ok)->setEnabled(value);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::onModelUpdated()
{
  Q_D(qSlicerExtensionsManagerDialog);
  d->updateButtons();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::onBatchProcessingChanged()
{
  Q_D(qSlicerExtensionsManagerDialog);
  d->updateButtons();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::closeEvent(QCloseEvent* event)
{
  Q_D(qSlicerExtensionsManagerDialog);
  if (d->ExtensionsManagerWidget->confirmClose())
  {
    event->accept(); // close window
  }
  else
  {
    event->ignore(); // ignore close event
  }
}

//-----------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::accept()
{
  Q_D(qSlicerExtensionsManagerDialog);
  if (d->ExtensionsManagerWidget->confirmClose())
  {
    Superclass::accept(); // close window
  }
}

//-----------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::reject()
{
  Q_D(qSlicerExtensionsManagerDialog);
  if (d->ExtensionsManagerWidget->confirmClose())
  {
    Superclass::reject(); // close window
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialog::setFocusToSearchBox()
{
  Q_D(qSlicerExtensionsManagerDialog);
  d->ExtensionsManagerWidget->setFocusToSearchBox();
}
