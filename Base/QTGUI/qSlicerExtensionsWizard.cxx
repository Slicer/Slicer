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

// CTK includes
#include "ctkLogger.h"

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsWizard.h"
#include "qSlicerExtensionsWizardWelcomePage.h"
#include "qSlicerExtensionsWizardOverviewPage.h"
#include "ui_qSlicerExtensionsWizardRestartPage.h"

static ctkLogger logger("org.commontk.libs.widgets.qSlicerExtensionsWizard");

//-----------------------------------------------------------------------------
class qSlicerExtensionsWizardPrivate
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsWizard);
protected:
  qSlicerExtensionsWizard* const q_ptr;

public:
  qSlicerExtensionsWizardPrivate(qSlicerExtensionsWizard& object);
  void init();
  Ui_qSlicerExtensionsWizardRestartPage RestartPageUi;
};

// --------------------------------------------------------------------------
qSlicerExtensionsWizardPrivate::qSlicerExtensionsWizardPrivate(qSlicerExtensionsWizard& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardPrivate::init()
{
  Q_Q(qSlicerExtensionsWizard);

  qSlicerExtensionsWizardWelcomePage* welcome = new qSlicerExtensionsWizardWelcomePage;

  qSlicerExtensionsWizardOverviewPage* overview = new qSlicerExtensionsWizardOverviewPage;

  QWizardPage* restart = new QWizardPage;
  this->RestartPageUi.setupUi(restart);

  q->addPage(welcome);
  q->addPage(overview);
  q->addPage(restart);
  q->setWindowTitle("Slicer extensions management");
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizard::qSlicerExtensionsWizard(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsWizardPrivate(*this))
{
  Q_D(qSlicerExtensionsWizard);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizard::~qSlicerExtensionsWizard()
{
}
