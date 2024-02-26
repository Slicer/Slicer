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
#include <QDebug>
#include <QDesktopServices>
#include <QMainWindow>
#include <QMessageBox>
#include <QSettings>
#include <QSignalMapper>
#include <QTextStream>

// Slicer includes
#include "vtkSlicerConfigure.h" // For Slicer_BUILD_DICOM_SUPPORT

// Slicer includes
#include "qSlicerWelcomeModuleWidget.h"
#include "ui_qSlicerWelcomeModuleWidget.h"
#include "qSlicerApplication.h"
#include "qSlicerIO.h"
#include "qSlicerIOManager.h"
#include "qSlicerLayoutManager.h"
#include "qSlicerModuleManager.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModulePanel.h"
#include "qSlicerUtils.h"
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
# include "qSlicerExtensionsManagerModel.h"
#endif
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
# include "qSlicerApplicationUpdateManager.h"
#endif

// CTK includes
#include "ctkButtonGroup.h"

// qMRML includes
#include "qMRMLWidget.h"

class qSlicerAppMainWindow;

//-----------------------------------------------------------------------------
class qSlicerWelcomeModuleWidgetPrivate : public Ui_qSlicerWelcomeModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerWelcomeModuleWidget);

protected:
  qSlicerWelcomeModuleWidget* const q_ptr;

public:
  qSlicerWelcomeModuleWidgetPrivate(qSlicerWelcomeModuleWidget& object);
  void setupUi(qSlicerWidget* widget);

  bool selectModule(const QString& moduleName);

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerExtensionsManagerModel* extensionsManagerModel();
#endif
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  qSlicerApplicationUpdateManager* applicationUpdateManager();
#endif

  QSignalMapper CollapsibleButtonMapper;

  QString CheckingForUpdatesText;
  QString NoUpdatesWereFoundText;
};

//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidgetPrivate::qSlicerWelcomeModuleWidgetPrivate(qSlicerWelcomeModuleWidget& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidgetPrivate::setupUi(qSlicerWidget* widget)
{
  Q_Q(qSlicerWelcomeModuleWidget);

  this->Ui_qSlicerWelcomeModuleWidget::setupUi(widget);

  // Make the "application update available" button at the top orange to make it stand out more.
  QPalette palette = q->palette();
  palette.setColor(this->ApplicationUpdateAvailableButton->foregroundRole(), QColor("orange"));
  this->ApplicationUpdateAvailableButton->setPalette(palette);
  this->ApplicationUpdateAvailableButton->hide();

  this->CheckingForUpdatesText = qSlicerWelcomeModuleWidget::tr("Checking for updates...");
  this->NoUpdatesWereFoundText = qSlicerWelcomeModuleWidget::tr("No updates were found.");

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  ctkButtonGroup* group = new ctkButtonGroup(widget);

  // Add all collabsibleWidgetButton to a button group
  QList<ctkCollapsibleButton*> collapsibles = widget->findChildren<ctkCollapsibleButton*>();
  foreach (ctkCollapsibleButton* collapsible, collapsibles)
  {
    group->addButton(collapsible);
  }

  // Update occurrences of documentation URLs
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  foreach (QWidget* widget,
           QWidgetList() << this->FeedbackCollapsibleWidget << this->WelcomeAndAboutCollapsibleWidget
                         << this->OtherUsefulHintsCollapsibleWidget << this->AcknowledgmentCollapsibleWidget)
  {
    QTextBrowser* textBrowser = widget->findChild<QTextBrowser*>();
    if (!textBrowser)
    {
      continue;
    }
    QString html = textBrowser->toHtml();
    qSlicerUtils::replaceDocumentationUrlVersion(
      html, QUrl(app->documentationBaseUrl()).host(), app->documentationVersion());
    textBrowser->setHtml(html);
  }
}

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
//-----------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerWelcomeModuleWidgetPrivate::extensionsManagerModel()
{
  qSlicerApplication* app = qSlicerApplication::application();
  if (!app || !app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool())
  {
    return nullptr;
  }
  return app->extensionsManagerModel();
}
#endif

#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
//-----------------------------------------------------------------------------
qSlicerApplicationUpdateManager* qSlicerWelcomeModuleWidgetPrivate::applicationUpdateManager()
{
  qSlicerApplication* app = qSlicerApplication::application();
  if (!app || !qSlicerApplicationUpdateManager::isApplicationUpdateEnabled())
  {
    return nullptr;
  }
  return app->applicationUpdateManager();
}
#endif

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidgetPrivate::selectModule(const QString& moduleName)
{
  Q_Q(qSlicerWelcomeModuleWidget);
  qSlicerModuleManager* moduleManager = qSlicerCoreApplication::application()->moduleManager();
  if (!moduleManager)
  {
    return false;
  }
  qSlicerAbstractCoreModule* module = moduleManager->module(moduleName);
  if (!module)
  {
    QMessageBox::warning(
      q,
      qSlicerWelcomeModuleWidget::tr("Raising %1 Module:").arg(moduleName),
      qSlicerWelcomeModuleWidget::tr("Unfortunately, this requested module is not available in this Slicer session."),
      QMessageBox::Ok);
    return false;
  }
  qSlicerLayoutManager* layoutManager = qSlicerApplication::application()->layoutManager();
  if (!layoutManager)
  {
    return false;
  }
  layoutManager->setCurrentModule(moduleName);
  return true;
}

//-----------------------------------------------------------------------------
// qSlicerWelcomeModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::qSlicerWelcomeModuleWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerWelcomeModuleWidgetPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerWelcomeModuleWidget::~qSlicerWelcomeModuleWidget() = default;

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setup()
{
  Q_D(qSlicerWelcomeModuleWidget);
  d->setupUi(this);

  connect(d->LoadDicomDataButton, SIGNAL(clicked()), this, SLOT(loadDicomData()));
  connect(d->LoadNonDicomDataButton, SIGNAL(clicked()), this, SLOT(loadNonDicomData()));
  connect(d->LoadSampleDataButton, SIGNAL(clicked()), this, SLOT(loadRemoteSampleData()));
  connect(d->EditApplicationSettingsButton, SIGNAL(clicked()), this, SLOT(editApplicationSettings()));
  connect(d->ExploreLoadedDataPushButton, SIGNAL(clicked()), this, SLOT(exploreLoadedData()));

#ifndef Slicer_BUILD_DICOM_SUPPORT
  d->LoadDicomDataButton->hide();
#endif

  qSlicerApplication* app = qSlicerApplication::application();

  bool extensionUpdatesEnabled = false;
  bool applicationUpdatesEnabled = false;

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  if (app && app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool())
  {
    QObject::connect(d->OpenExtensionsManagerButton,
                     SIGNAL(clicked()),
                     qSlicerApplication::application(),
                     SLOT(openExtensionsManagerDialog()));
    qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
    if (extensionsManagerModel)
    {
      extensionUpdatesEnabled = true;
      QObject::connect(extensionsManagerModel,
                       SIGNAL(extensionUpdatesAvailable(bool)),
                       this,
                       SLOT(setExtensionUpdatesAvailable(bool)));

      if (!extensionsManagerModel->availableUpdateExtensions().empty())
      {
        this->setExtensionUpdatesAvailable(true);
      }

      QObject::connect(
        extensionsManagerModel, SIGNAL(autoUpdateSettingsChanged()), this, SLOT(onAutoUpdateSettingsChanged()));
    }
  }
  else
  {
    d->OpenExtensionsManagerButton->hide();
  }
#else
  d->OpenExtensionsManagerButton->hide();
#endif

#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  if (app && qSlicerApplicationUpdateManager::isApplicationUpdateEnabled())
  {
    QObject::connect(d->ApplicationUpdateAvailableButton,
                     SIGNAL(clicked()),
                     qSlicerApplication::application(),
                     SLOT(openApplicationDownloadWebsite()));
    QObject::connect(d->ApplicationUpdateStatusButton,
                     SIGNAL(clicked()),
                     qSlicerApplication::application(),
                     SLOT(openApplicationDownloadWebsite()));
    qSlicerApplicationUpdateManager* applicationUpdateManager = d->applicationUpdateManager();
    if (applicationUpdateManager)
    {
      applicationUpdatesEnabled = true;
      QObject::connect(
        applicationUpdateManager, SIGNAL(updateAvailable(bool)), this, SLOT(setApplicationUpdateAvailable(bool)));
      if (applicationUpdateManager->isUpdateAvailable())
      {
        this->setApplicationUpdateAvailable(true);
      }

      QObject::connect(
        applicationUpdateManager, SIGNAL(autoUpdateCheckChanged()), this, SLOT(onAutoUpdateSettingsChanged()));
    }
  }
#endif

  if (!extensionUpdatesEnabled && !applicationUpdatesEnabled)
  {
    d->AutomaticUpdatesCollapsibleWidget->hide();
  }

  // Initialize AutoUpdate button state
  this->onAutoUpdateSettingsChanged();

  if (extensionUpdatesEnabled || applicationUpdatesEnabled)
  {
    QObject::connect(d->CheckForUpdatesAutomaticallyCheckBox,
                     SIGNAL(stateChanged(int)),
                     this,
                     SLOT(onAutoUpdateCheckStateChanged(int)));

    QObject::connect(d->CheckForUpdatesNowButton, SIGNAL(clicked()), this, SLOT(checkForUpdates()));

    QObject::connect(d->ExtensionUpdatesStatusButton,
                     SIGNAL(clicked()),
                     qSlicerApplication::application(),
                     SLOT(openExtensionsManagerDialog()));
  }

  this->Superclass::setup();

  d->FeedbackCollapsibleWidget->setCollapsed(false);
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::editApplicationSettings()
{
  qSlicerApplication::application()->openSettingsDialog();
}

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::loadDicomData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  return d->selectModule("DICOM");
}

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::loadNonDicomData()
{
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  if (!ioManager)
  {
    return false;
  }
  return ioManager->openAddDataDialog();
}

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::loadRemoteSampleData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  return d->selectModule("SampleData");
}

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidget::exploreLoadedData()
{
  Q_D(qSlicerWelcomeModuleWidget);
  return d->selectModule("Data");
}

//---------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setExtensionUpdatesAvailable(bool isAvailable)
{
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  Q_D(qSlicerWelcomeModuleWidget);

  // Check if there was a change
  const char extensionUpdateAvailablePropertyName[] = "extensionUpdateAvailable";
  if (d->OpenExtensionsManagerButton->property(extensionUpdateAvailablePropertyName).toBool() != isAvailable)
  {
    // changed
    d->OpenExtensionsManagerButton->setProperty(extensionUpdateAvailablePropertyName, isAvailable);
    if (isAvailable)
    {
      d->OpenExtensionsManagerButton->setIcon(QIcon(":/Icons/ExtensionNotificationIcon.png"));
    }
    else
    {
      d->OpenExtensionsManagerButton->setIcon(QIcon(":/Icons/ExtensionDefaultIcon.png"));
    }
  }

  QStringList availableUpdates;
  qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
  if (extensionsManagerModel)
  {
    availableUpdates = extensionsManagerModel->availableUpdateExtensions();
  }

  if (availableUpdates.empty())
  {
    d->ExtensionUpdatesStatusButton->setEnabled(false);
    d->ExtensionUpdatesStatusButton->setText(d->NoUpdatesWereFoundText);
  }
  else
  {
    d->ExtensionUpdatesStatusButton->setEnabled(true);
    d->ExtensionUpdatesStatusButton->setText(
      tr("%1 extension update is available", "%1 extension updates are available", availableUpdates.size())
        .arg(availableUpdates.size()));
    d->ExtensionUpdatesStatusButton->setToolTip(tr("Use Extensions Manager to update these extensions:")
                                                + QString("\n- ") + availableUpdates.join("\n- "));
  }
#else
  Q_UNUSED(isAvailable);
#endif
}

//---------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::setApplicationUpdateAvailable(bool update)
{
  Q_UNUSED(update);
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  Q_D(qSlicerWelcomeModuleWidget);
  // Check if there was a change
  QString latestVersion;
  if (qSlicerApplicationUpdateManager::isApplicationUpdateEnabled())
  {
    qSlicerApplicationUpdateManager* applicationUpdateManager = d->applicationUpdateManager();
    if (applicationUpdateManager && applicationUpdateManager->isUpdateAvailable())
    {
      latestVersion = applicationUpdateManager->latestReleaseVersion();
    }
  }

  if (latestVersion.isEmpty())
  {
    d->ApplicationUpdateAvailableButton->hide();
    d->ApplicationUpdateStatusButton->setEnabled(false);
    d->ApplicationUpdateStatusButton->setText(d->NoUpdatesWereFoundText);
    d->ApplicationUpdateStatusButton->setToolTip("");
  }
  else
  {
    QString buttonText = tr("New application version is available: %1").arg(latestVersion);
    d->ApplicationUpdateAvailableButton->setText(buttonText);
    d->ApplicationUpdateAvailableButton->show();
    d->ApplicationUpdateStatusButton->setText(buttonText);
    d->ApplicationUpdateStatusButton->setEnabled(true);
    d->ApplicationUpdateStatusButton->setToolTip(d->ApplicationUpdateAvailableButton->toolTip());
  }
#endif
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::checkForUpdates()
{
  Q_D(qSlicerWelcomeModuleWidget);

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
  if (extensionsManagerModel)
  {
    d->ExtensionUpdatesStatusButton->setText(d->CheckingForUpdatesText);
    d->ExtensionUpdatesStatusButton->setEnabled(false);
    // wait for completion so that checkForExtensionsUpdates works from the updated metadata
    extensionsManagerModel->updateExtensionsMetadataFromServer(true, true);
    extensionsManagerModel->checkForExtensionsUpdates();
  }
#endif

#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  qSlicerApplicationUpdateManager* applicationUpdateManager = d->applicationUpdateManager();
  if (applicationUpdateManager)
  {
    d->ApplicationUpdateStatusButton->setEnabled(false);
    d->ApplicationUpdateStatusButton->setText(d->CheckingForUpdatesText);
    d->ApplicationUpdateStatusButton->setToolTip("");
    d->ApplicationUpdateAvailableButton->hide();
    applicationUpdateManager->checkForUpdate(true, false);
  }
#endif
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::onAutoUpdateCheckStateChanged(int state)
{
  Q_D(qSlicerWelcomeModuleWidget);
  bool autoUpdate = (state != Qt::Unchecked);

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
  if (extensionsManagerModel)
  {
    extensionsManagerModel->setAutoUpdateCheck(autoUpdate);
  }
#endif

#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  qSlicerApplicationUpdateManager* applicationUpdateManager = d->applicationUpdateManager();
  if (applicationUpdateManager)
  {
    applicationUpdateManager->setAutoUpdateCheck(autoUpdate);
  }
#endif
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::onAutoUpdateSettingsChanged()
{
  Q_D(qSlicerWelcomeModuleWidget);
  bool extensionAutoUpdateCheckEnabled = false;
  bool applicationAutoUpdateCheckEnabled = false;

  qSlicerApplication* app = qSlicerApplication::application();

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  if (app && app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool())
  {
    qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
    if (extensionsManagerModel)
    {
      if (extensionsManagerModel->autoUpdateCheck())
      {
        extensionAutoUpdateCheckEnabled = true;
      }
    }
  }
#endif

#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
  if (qSlicerApplicationUpdateManager::isApplicationUpdateEnabled())
  {
    qSlicerApplicationUpdateManager* applicationUpdateManager = d->applicationUpdateManager();
    if (applicationUpdateManager)
    {
      if (applicationUpdateManager->autoUpdateCheck())
      {
        applicationAutoUpdateCheckEnabled = true;
      }
    }
  }
#endif

  QSignalBlocker blocker(d->CheckForUpdatesAutomaticallyCheckBox);
  if (extensionAutoUpdateCheckEnabled && applicationAutoUpdateCheckEnabled)
  {
    d->CheckForUpdatesAutomaticallyCheckBox->setCheckState(Qt::Checked);
    d->CheckForUpdatesAutomaticallyCheckBox->setTristate(false);
  }
  else if (!extensionAutoUpdateCheckEnabled && !applicationAutoUpdateCheckEnabled)
  {
    d->CheckForUpdatesAutomaticallyCheckBox->setTristate(false);
    d->CheckForUpdatesAutomaticallyCheckBox->setCheckState(Qt::Unchecked);
  }
  else
  {
    d->CheckForUpdatesAutomaticallyCheckBox->setCheckState(Qt::PartiallyChecked);
  }
}
