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
#include <QHash>
#include <QProgressDialog>
#include <QSettings>
#include <QUrl>
#include <QVariant>

// qCDashAPI includes
#include <qCDashAPI.h>

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

  qCDashAPI             CDashAPI;
  QString               RetrieveS4extQueryUuid;
  QString               RetrievePackageQueryUuid;
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
  q->registerField("extensionsServerURL", this->SearchURLComboBox, "currentText", SIGNAL(editTextChanged(const QString&)));

  q->setProperty("retrievedListOfS4extFiles", QVariant::fromValue(QList<QVariantMap>()));
  q->registerField("retrievedListOfS4extFiles", q, "retrievedListOfS4extFiles");

  q->setProperty("retrievedListOfPackageFiles", QVariant::fromValue(QList<QVariantMap>()));
  q->registerField("retrievedListOfPackageFiles", q, "retrievedListOfPackageFiles");

  QObject::connect(this->InstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->UninstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->SearchURLComboBox, SIGNAL(editTextChanged(const QString&)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->SearchURLComboBox, SIGNAL(currentIndexChanged(const QString&)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->DeleteTempPushButton, SIGNAL(clicked()),
                   q, SLOT(deleteTemporaryArchiveFiles()));
  
  qRegisterMetaType<QList<QVariantMap> >("QList<QVariantMap>");
  QObject::connect(&this->CDashAPI, SIGNAL(projectFilesReceived(const QString&, const QList<QVariantMap>&)),
                   q, SLOT(onProjectFilesReceived(const QString&, const QList<QVariantMap>&)));
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
  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  if (!app)
    {
    return;
    }

  d->InstallPathDirectoryButton->setDirectory(app->extensionsPath());

  QString url("http://www.cdash.org/slicer4");
  //QStringList urls = QSettings().value("Modules/ExtensionsUrls").toStringList();
  //d->SearchURLComboBox->addItems(urls);
  d->SearchURLComboBox->setEditText(url);
  d->PlatformArchitectureValueLabel->setText(app->platform());
  d->SlicerRevisionLineEdit->setText(app->repositoryRevision());
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsWizardWelcomePage::validatePage()
{
  Q_D(qSlicerExtensionsWizardWelcomePage);

  // Make sure the Extensions directory exists.
  QString extensionsPath = this->field("installPath").toString();
  QDir::root().mkpath(extensionsPath);

  qSlicerCoreApplication * app = qSlicerCoreApplication::application();
  d->CDashAPI.setUrl(d->SearchURLComboBox->currentText());

  QString revision = d->SlicerRevisionLineEdit->text();

  // Prepare matching pattern arguments
  QString retrieveS4extPattern("%1-%2-.*.s4ext");
  retrieveS4extPattern = retrieveS4extPattern.arg(revision).arg(app->platform());

  QString retrievePackagePattern("%1-%2-.*.tar.gz");
  retrievePackagePattern = retrievePackagePattern.arg(revision).arg(app->platform());

  // Query CDash server
  d->CDashAPI.setLogLevel(qCDashAPI::SILENT);
  d->RetrieveS4extQueryUuid = d->CDashAPI.queryProjectFiles("Slicer4", retrieveS4extPattern);

  d->CDashAPI.setLogLevel(qCDashAPI::SILENT);
  d->RetrievePackageQueryUuid = d->CDashAPI.queryProjectFiles("Slicer4", retrievePackagePattern);


  if (d->ProgressDialog->exec() == QDialog::Rejected)
    {
    return false; // Failed to retrieve list of files
    }
  
  // Save the url into the favorite box so it can be used next time
//  int index = d->SearchURLComboBox->findText(manifestURL);
//  if (index == -1)
//    {
//    d->SearchURLComboBox->addItem(manifestURL);
//    }

  // Save all the favorite urls for next sessions
//  QStringList urls;
//  for (int i = 0; i < d->SearchURLComboBox->count(); ++i)
//    {
//    urls << d->SearchURLComboBox->itemText(i);
//    }

  //QSettings().setValue("Modules/ExtensionsUrls", urls);

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
  if (d->InstallExtensionsCheckBox->isChecked() && !searchURL.isValid())
    {
    return false;
    }
  return true;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePage::deleteTemporaryArchiveFiles()
{
  if (!qSlicerCoreApplication::application())
    {
    return;
    }
  QDir tempDir(qSlicerCoreApplication::application()->temporaryPath());
  foreach(const QString& file, tempDir.entryList(QStringList() << "*.tar.gz"))
    {
    tempDir.remove(file);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePage::onProjectFilesReceived(const QString& queryUuid,
                                                                const QList<QVariantMap>& files)
{
  Q_D(qSlicerExtensionsWizardWelcomePage);
  // Set manifestFile as a dynamic property
  if (queryUuid == d->RetrieveS4extQueryUuid)
    {
    this->setProperty("retrievedListOfS4extFiles", QVariant::fromValue(files));
    d->RetrieveS4extQueryUuid.clear();
    }
  else if (queryUuid == d->RetrievePackageQueryUuid)
    {
    this->setProperty("retrievedListOfPackageFiles", QVariant::fromValue(files));
    d->RetrievePackageQueryUuid.clear();
    }
  if (d->RetrieveS4extQueryUuid.isEmpty() && d->RetrievePackageQueryUuid.isEmpty())
    {
    d->ProgressDialog->accept();
    }
}
