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
#include <QPushButton>

// QtGUI includes
#include "qSlicerCoreApplication.h"
#include "qSlicerExtensionsManagerDialog.h"
#include "qSlicerExtensionsManagerModel.h"
#include "ui_qSlicerExtensionsManagerDialog.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsManagerDialogPrivate: public Ui_qSlicerExtensionsManagerDialog
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManagerDialog);
protected:
  qSlicerExtensionsManagerDialog* const q_ptr;

public:
  qSlicerExtensionsManagerDialogPrivate(qSlicerExtensionsManagerDialog& object);
  void init();

  bool RestartRequested;

  QStringList PreviousModulesAdditionalPaths;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerDialogPrivate::qSlicerExtensionsManagerDialogPrivate(qSlicerExtensionsManagerDialog& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerDialogPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerDialog);

  this->setupUi(q);

  QPushButton * restartButton = this->ButtonBox->button(QDialogButtonBox::Ok);
  restartButton->setText("Restart");

  q->setRestartRequested(false);

  // Assuming the dialog is instantiated prior any update of the settings,
  // keeping track of settings will allow us to display the "RestartRequestedLabel"
  // only if it applies. Note also that keep track of "EnvironmentVariables/PYTHONPATH'
  // isn't required, "Modules/AdditionalPaths" is enough to know if we should restart.
  this->PreviousModulesAdditionalPaths = QSettings().value("Modules/AdditionalPaths").toStringList();
}

// --------------------------------------------------------------------------
// qSlicerExtensionsManagerDialog methods

// --------------------------------------------------------------------------
qSlicerExtensionsManagerDialog::qSlicerExtensionsManagerDialog(QWidget *_parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsManagerDialogPrivate(*this))
{
  Q_D(qSlicerExtensionsManagerDialog);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerDialog::~qSlicerExtensionsManagerDialog()
{
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsManagerDialog::extensionsManagerModel()const
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
    connect(model, SIGNAL(modelUpdated()),
            this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionInstalled(QString)),
            this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionUninstalled(QString)),
            this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionEnabledChanged(QString,bool)),
            this, SLOT(onModelUpdated()));
    }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerDialog::restartRequested()const
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
  Q_ASSERT(this->extensionsManagerModel());
  bool shouldRestart = false;
  if (d->PreviousModulesAdditionalPaths
      != QSettings().value("Modules/AdditionalPaths").toStringList())
    {
    shouldRestart = true;
    }
  this->setRestartRequested(shouldRestart);
}
