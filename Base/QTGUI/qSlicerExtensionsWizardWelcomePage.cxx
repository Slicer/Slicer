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
#include <QSettings>
#include <QUrl>

// CTK includes
#include "ctkLogger.h"

// QtGUI includes
#include "qSlicerCoreApplication.h"
#include "qSlicerExtensionsWizardWelcomePage.h"
#include "ui_qSlicerExtensionsWizardWelcomePage.h"

static ctkLogger logger("org.commontk.libs.widgets.qSlicerExtensionsWizardWelcomePage");

// qSlicerExtensionsWizardWelcomePagePrivate

//-----------------------------------------------------------------------------
class qSlicerExtensionsWizardWelcomePagePrivate
  : public Ui_qSlicerExtensionsWizardWelcomePage
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsWizardWelcomePage);
protected:
  qSlicerExtensionsWizardWelcomePage* const q_ptr;

public:
  qSlicerExtensionsWizardWelcomePagePrivate(qSlicerExtensionsWizardWelcomePage& object);
  void init();
};

// --------------------------------------------------------------------------
qSlicerExtensionsWizardWelcomePagePrivate::qSlicerExtensionsWizardWelcomePagePrivate(qSlicerExtensionsWizardWelcomePage& object)
  :q_ptr(&object)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePagePrivate::init()
{
  Q_Q(qSlicerExtensionsWizardWelcomePage);

  this->setupUi(q);

  q->registerField("installEnabled", this->InstallExtensionsCheckBox);
  q->registerField("uninstallEnabled", this->UninstallExtensionsCheckBox);
  q->registerField("installPath", this->InstallPathDirectoryButton, "directory", SIGNAL(directoryChanged(const QString&)));
  q->registerField("extensionsURL", this->SearchURLComboBox, "currentText", SIGNAL(editTextChanged(const QString&)));
  
  QObject::connect(this->InstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->UninstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->SearchURLComboBox, SIGNAL(editTextChanged(const QString&)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->SearchURLComboBox, SIGNAL(currentIndexChanged(const QString&)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->DeleteTempPushButton, SIGNAL(clicked()),
                   q, SIGNAL(deleteTemporaryZipFiles()));
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizardWelcomePage::qSlicerExtensionsWizardWelcomePage(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsWizardWelcomePagePrivate(*this))
{
  Q_D(qSlicerExtensionsWizardWelcomePage);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsWizardWelcomePage::~qSlicerExtensionsWizardWelcomePage()
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePage::initializePage()
{
  Q_D(qSlicerExtensionsWizardWelcomePage);
  
  if (!qSlicerCoreApplication::application())
    {
    return;
    }

  d->InstallPathDirectoryButton->setDirectory(
    qSlicerCoreApplication::application()->extensionsPath());


  QString url("http://ext.slicer.org/ext/");
  url += qSlicerCoreApplication::application()->repositoryUrl();
  url += "/";
  url += qSlicerCoreApplication::application()->repositoryRevision();
  url += "-";
  url += qSlicerCoreApplication::application()->platform();

  QStringList urls = settings.value("Modules/ExtensionsUrls").toStringList();

  d->SearchURLComboBox->addItems(urls);
  d->SearchURLComboBox->setEditText(url);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardWelcomePage::validatePage()
{
  Q_D(qSlicerExtensionsWizardWelcomePage);

  QUrl searchURL(d->SearchURLComboBox->currentText());
  int index = d->SearchURLComboBox->findText(searchURL.toString());
  if (index == -1 && searchURL.isValid())
    {
    d->SearchURLComboBox->addItem(searchURL.toString());
    }

  QStringList urls;
  for (int i = 0; i < d->SearchURLComboBox->count(); ++i)
    {
    urls << d->SearchURLComboBox->itemText(i);
    }
  QSettings settings;
  settings.setValue("Modules/ExtensionsUrls", urls);
  
  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardWelcomePage::isComplete()const
{
  Q_D(const qSlicerExtensionsWizardWelcomePage);
  
  if (!d->InstallExtensionsCheckBox->isChecked() &&
      !d->UninstallExtensionsCheckBox->isChecked())
    {
    return false;
    }
  
  QUrl searchURL(d->SearchURLComboBox->currentText());

  if (d->InstallExtensionsCheckBox->isChecked() &&
      !searchURL.isValid())
    {
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardWelcomePage::deleteTemporaryZipFiles()
{
  Q_D(qSlicerExtensionsWizardWelcomePage);
  if (!qSlicerCoreApplication::application())
    {
    return;
    }
  QDir tempDir(qSlicerCoreApplication::application()->temporaryPath());
  foreach(QString zipFile, tempDir.entryList("*.zip"))
    {
    tempDir.remove(zipFile);
    }
}
