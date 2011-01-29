/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QSettings>

// CTK includes
#include <ctkLogger.h>

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsWizardRestartPage.h"
#include "ui_qSlicerExtensionsWizardRestartPage.h"

static ctkLogger logger("org.commontk.libs.widgets.qSlicerExtensionsWizardRestartPage");

// --------------------------------------------------------------------------
// qSlicerExtensionsWizardRestartPagePrivate

//-----------------------------------------------------------------------------
class qSlicerExtensionsWizardRestartPagePrivate
  : public Ui_qSlicerExtensionsWizardRestartPage
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsWizardRestartPage);
protected:
  qSlicerExtensionsWizardRestartPage* const q_ptr;

public:
  qSlicerExtensionsWizardRestartPagePrivate(qSlicerExtensionsWizardRestartPage& object);
  void init();

};

// --------------------------------------------------------------------------
qSlicerExtensionsWizardRestartPagePrivate::qSlicerExtensionsWizardRestartPagePrivate(qSlicerExtensionsWizardRestartPage& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardRestartPagePrivate::init()
{
  Q_Q(qSlicerExtensionsWizardRestartPage);
  this->setupUi(q);

  QObject::connect(this->RestartNowPushButton, SIGNAL(clicked()),
                   qSlicerCoreApplication::instance(), SLOT(restart()));
}

// --------------------------------------------------------------------------
// qSlicerExtensionsWizardRestartPage

// --------------------------------------------------------------------------
qSlicerExtensionsWizardRestartPage::qSlicerExtensionsWizardRestartPage(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsWizardRestartPagePrivate(*this))
{
  Q_D(qSlicerExtensionsWizardRestartPage);

  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizardRestartPage::~qSlicerExtensionsWizardRestartPage()
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardRestartPage::initializePage()
{
  QStringList extensions = this->field("installedExtensions").toStringList();

  QSettings settings;
  settings.setValue("Modules/Extensions", extensions);
}

