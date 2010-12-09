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
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProgressDialog>
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

  QNetworkAccessManager NetworkManager;
  QProgressDialog*      ProgressDialog;
};

// --------------------------------------------------------------------------
qSlicerExtensionsWizardWelcomePagePrivate::qSlicerExtensionsWizardWelcomePagePrivate(qSlicerExtensionsWizardWelcomePage& object)
  :q_ptr(&object)
{
  this->ProgressDialog = 0;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePagePrivate::init()
{
  Q_Q(qSlicerExtensionsWizardWelcomePage);

  this->setupUi(q);
  this->DeleteTempPushButton->setIcon(q->style()->standardIcon(QStyle::SP_TrashIcon));
  this->ProgressDialog = new QProgressDialog(q);
  this->ProgressDialog->setLabelText("Download manifest");
  this->ProgressDialog->setRange(0,0);

  q->registerField("installEnabled", this->InstallExtensionsCheckBox);
  q->registerField("uninstallEnabled", this->UninstallExtensionsCheckBox);
  q->registerField("installPath", this->InstallPathDirectoryButton, "directory", SIGNAL(directoryChanged(const QString&)));
  q->registerField("extensionsURL", this->SearchURLComboBox, "currentText", SIGNAL(editTextChanged(const QString&)));
  q->setProperty("manifestFile", QString());
  q->registerField("manifestFile", q, "manifestFile");

  QObject::connect(this->InstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->UninstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->SearchURLComboBox, SIGNAL(editTextChanged(const QString&)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->SearchURLComboBox, SIGNAL(currentIndexChanged(const QString&)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->DeleteTempPushButton, SIGNAL(clicked()),
                   q, SLOT(deleteTemporaryZipFiles()));
  
  QObject::connect(&this->NetworkManager, SIGNAL(finished(QNetworkReply*)),
                   q, SLOT(downloadFinished(QNetworkReply*)));
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
  url += qSlicerCoreApplication::application()->repositoryBranch();
  url += "/";
  url += qSlicerCoreApplication::application()->repositoryRevision();
  url += "-";
  url += qSlicerCoreApplication::application()->platform();

  QSettings settings;
  QStringList urls = settings.value("Modules/ExtensionsUrls").toStringList();

  d->SearchURLComboBox->addItems(urls);
  d->SearchURLComboBox->setEditText(url);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardWelcomePage::validatePage()
{
  Q_D(qSlicerExtensionsWizardWelcomePage);

  // Make sure the Extensions directory exists.
  QString extensionsPath = this->field("installPath").toString();
  QDir::root().mkpath(extensionsPath);
  
  // Set manifestFile as a dynamic property
  this->setProperty("manifestFile", extensionsPath + "/manifest.html");

  // Download manifest file that contains the list of the remote extensions
  QString manifestURL = d->SearchURLComboBox->currentText();
  if (manifestURL.right(1) != "/")
    {
    manifestURL += "/";
    }
  QUrl manifestUrl(manifestURL);
  QNetworkRequest request(manifestUrl);
  // When the NetworkManager will be done, it will close the progress dialog
  d->NetworkManager.get(request);
  if (d->ProgressDialog->exec() == QDialog::Rejected)
    {
    // Failed to download the file
    return false;
    }
  
  // Save the url into the favorite box so it can be used next time
  int index = d->SearchURLComboBox->findText(manifestURL);
  if (index == -1)
    {
    d->SearchURLComboBox->addItem(manifestURL);
    }

  // Save all the favorite urls for next sessions
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
void qSlicerExtensionsWizardWelcomePage::deleteTemporaryZipFiles()
{
  if (!qSlicerCoreApplication::application())
    {
    return;
    }
  QDir tempDir(qSlicerCoreApplication::application()->temporaryPath());
  foreach(QString zipFile, tempDir.entryList(QStringList() << "*.zip"))
    {
    tempDir.remove(zipFile);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePage::downloadFinished(QNetworkReply* reply)
{
  Q_D(qSlicerExtensionsWizardWelcomePage);
  if (reply->error())
    {
    qWarning() << "Failed downloading: " << reply->url().toString();
    d->ProgressDialog->cancel();
    return;
    }
  QString fileName = this->field("manifestFile").toString();
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
    {
    qWarning() << "Could not open " << fileName << " for writing: %s" << file.errorString();
    d->ProgressDialog->cancel();
    return;
    }

  file.write(reply->readAll());
  file.close();

  d->ProgressDialog->accept();
}
