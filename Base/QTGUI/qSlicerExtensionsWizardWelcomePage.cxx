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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QProgressDialog>
#include <QUrl>

// qMidasAPI includes
#include <qMidasAPI.h>

// QtGUI includes
#include "qSlicerCoreApplication.h"
#include "qSlicerExtensionsWizardWelcomePage.h"
#include "ui_qSlicerExtensionsWizardWelcomePage.h"

//-----------------------------------------------------------------------------
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

  qMidasAPI             MidasAPI;
  QString               RetrieveS4extQueryUuid;
  QString               RetrieveExtensionInfosQueryUuid;
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
  q->registerField("installPath", this->InstallPathDirectoryButton, "directory", SIGNAL(directoryChanged(QString)));
  q->registerField("extensionsServerURL", this->ExtensionServerURL, "text", SIGNAL(textChanged(QString)));

  q->setProperty("retrievedListOfExtensionInfos", QVariant::fromValue(QList<QVariantMap>()));
  q->registerField("retrievedListOfExtensionInfos", q, "retrievedListOfExtensionInfos");

  QObject::connect(this->InstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->UninstallExtensionsCheckBox, SIGNAL(toggled(bool)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->ExtensionServerURL, SIGNAL(textChanged(QString)),
                   q, SIGNAL(completeChanged()));
  QObject::connect(this->DeleteTempPushButton, SIGNAL(clicked()),
                   q, SLOT(deleteTemporaryArchiveFiles()));

  qRegisterMetaType<QList<QVariantMap> >("QList<QVariantMap>");
  QObject::connect(&this->MidasAPI, SIGNAL(resultReceived(QUuid,QList<QVariantMap>)),
                   q, SLOT(onExtensionInfosReceived(QUuid,QList<QVariantMap>)));
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

  QString url("http://localhost/midas");
  d->ExtensionServerURL->setText(url);
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
  d->MidasAPI.setMidasUrl(d->ExtensionServerURL->text());

  QString revision = d->SlicerRevisionLineEdit->text();

  // Query CDash server
  qMidasAPI::ParametersType parameters;
  parameters["slicer_revision"] = revision;
  parameters["os"] = app->platform().split("-").at(0);
  parameters["arch"] = app->platform().split("-").at(1);

  d->RetrieveExtensionInfosQueryUuid =
      d->MidasAPI.query("midas.slicerpackages.extension.list", parameters);

  if (d->ProgressDialog->exec() == QDialog::Rejected)
    {
    return false; // Failed to retrieve list of files
    }

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

  QUrl searchURL(d->ExtensionServerURL->text());
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

#include <QDebug>

// --------------------------------------------------------------------------
void qSlicerExtensionsWizardWelcomePage::onExtensionInfosReceived(const QUuid& queryUuid,
                                                                  const QList<QVariantMap>& extensionInfos)
{
  Q_D(qSlicerExtensionsWizardWelcomePage);
  if (queryUuid == d->RetrieveExtensionInfosQueryUuid)
    {
    this->setProperty("retrievedListOfExtensionInfos", QVariant::fromValue(extensionInfos));
    d->RetrieveExtensionInfosQueryUuid.clear();
    }
  if (d->RetrieveExtensionInfosQueryUuid.isEmpty())
    {
    d->ProgressDialog->accept();
    }
}
