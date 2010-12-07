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
#include <QUrl>

// CTK includes
#include "ctkLogger.h"

// QtGUI includes
#include "qSlicerApplication.h"
#include "qSlicerExtensionsWizardOverviewPage.h"
#include "ui_qSlicerExtensionsWizardOverviewPage.h"

static ctkLogger logger("org.commontk.libs.widgets.qSlicerExtensionsWizardOverviewPage");

// qSlicerExtensionsWizardOverviewPagePrivate

//-----------------------------------------------------------------------------
class qSlicerExtensionsWizardOverviewPagePrivate
  : public Ui_qSlicerExtensionsWizardOverviewPage
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsWizardOverviewPage);
protected:
  qSlicerExtensionsWizardOverviewPage* const q_ptr;

public:
  qSlicerExtensionsWizardOverviewPagePrivate(qSlicerExtensionsWizardOverviewPage& object);
  void init();
};

// --------------------------------------------------------------------------
qSlicerExtensionsWizardOverviewPagePrivate::qSlicerExtensionsWizardOverviewPagePrivate(qSlicerExtensionsWizardOverviewPage& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPagePrivate::init()
{
  Q_Q(qSlicerExtensionsWizardOverviewPage);
  this->setupUi(q);
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizardOverviewPage::qSlicerExtensionsWizardOverviewPage(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsWizardOverviewPagePrivate(*this))
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizardOverviewPage::~qSlicerExtensionsWizardOverviewPage()
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardOverviewPage::initializePage()
{
  QString installPath = this->field("installPath").toString();
  QString extensionsURL = this->field("extensionsURL").toString();
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardOverviewPage::validatePage()
{
  Q_D(qSlicerExtensionsWizardOverviewPage);

  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardOverviewPage::isComplete()const
{
  Q_D(const qSlicerExtensionsWizardOverviewPage);
  
  return true;
}
