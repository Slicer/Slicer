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
#include "vtkSlicerVersionConfigure.h"

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
#include "qSlicerExtensionsManagerModel.h"

// CTK includes
#include "ctkButtonGroup.h"

// qMRML includes
#include "qMRMLWidget.h"

class qSlicerAppMainWindow;

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_SlicerWelcome
class qSlicerWelcomeModuleWidgetPrivate: public Ui_qSlicerWelcomeModuleWidget
{
  Q_DECLARE_PUBLIC(qSlicerWelcomeModuleWidget);
protected:
  qSlicerWelcomeModuleWidget* const q_ptr;
public:
  qSlicerWelcomeModuleWidgetPrivate(qSlicerWelcomeModuleWidget& object);
  void setupUi(qSlicerWidget* widget);

  bool selectModule(const QString& moduleName);

  qSlicerExtensionsManagerModel* extensionsManagerModel();

  QSignalMapper CollapsibleButtonMapper;
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

  // QLabel's pixmap property loads the base image (ignores high-resolution @2x versions),
  // therefore we need to retrieve and set the best icon version manually.
  this->label->setPixmap(qMRMLWidget::pixmapFromIcon(QIcon(":/Images/WelcomeLogo.png")));

  this->UpdatesStatusLabel->hide();
  this->UpdatesFoundOpenExtensionsManagerButton->hide();

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  ctkButtonGroup * group = new ctkButtonGroup(widget);

  // Add all collabsibleWidgetButton to a button group
  QList<ctkCollapsibleButton*> collapsibles = widget->findChildren<ctkCollapsibleButton*>();
  foreach(ctkCollapsibleButton* collapsible, collapsibles)
    {
    group->addButton(collapsible);
    }

  // Lazily set the fitted browser source to avoid overhead when the module
  // is loaded.
  this->FeedbackCollapsibleWidget->setProperty("source", ":HTML/Feedback.html");
  this->WelcomeAndAboutCollapsibleWidget->setProperty("source", ":HTML/About.html");
  this->OtherUsefulHintsCollapsibleWidget->setProperty("source", ":HTML/OtherUsefulHints.html");
  this->AcknowledgmentCollapsibleWidget->setProperty("source", ":HTML/Acknowledgment.html");

  foreach(QWidget* widget, QWidgetList()
          << this->FeedbackCollapsibleWidget
          << this->WelcomeAndAboutCollapsibleWidget
          << this->OtherUsefulHintsCollapsibleWidget
          << this->AcknowledgmentCollapsibleWidget
          )
    {
    this->CollapsibleButtonMapper.setMapping(widget, widget);
    QObject::connect(widget, SIGNAL(contentsCollapsed(bool)),
                     &this->CollapsibleButtonMapper, SLOT(map()));
    }

  QObject::connect(&this->CollapsibleButtonMapper, SIGNAL(mapped(QWidget*)),
                   q, SLOT(loadSource(QWidget*)));
}

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

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::loadSource(QWidget* widget)
{
  // Lookup fitted browser
  ctkFittedTextBrowser* fittedTextBrowser =
      widget->findChild<ctkFittedTextBrowser*>();
  Q_ASSERT(fittedTextBrowser);
  if (fittedTextBrowser->source().isEmpty())
    {
    // Read content
    QString url = widget->property("source").toString();
    QFile source(url);
    if(!source.open(QIODevice::ReadOnly))
      {
      qWarning() << Q_FUNC_INFO << ": Failed to read" << url;
      return;
      }
    QTextStream in(&source);
    QString html = in.readAll();
    source.close();

    qSlicerCoreApplication* app = qSlicerCoreApplication::application();

    // Update occurrences of documentation URLs
    html = qSlicerUtils::replaceDocumentationUrlVersion(html,
      QUrl(app->documentationBaseUrl()).host(), app->documentationVersion());

    fittedTextBrowser->setHtml(html);
    }
}

//-----------------------------------------------------------------------------
bool qSlicerWelcomeModuleWidgetPrivate::selectModule(const QString& moduleName)
{
  Q_Q(qSlicerWelcomeModuleWidget);
  qSlicerModuleManager * moduleManager = qSlicerCoreApplication::application()->moduleManager();
  if (!moduleManager)
    {
    return false;
    }
  qSlicerAbstractCoreModule * module = moduleManager->module(moduleName);
  if(!module)
    {
    QMessageBox::warning(
          q, qSlicerWelcomeModuleWidget::tr("Raising %1 Module:").arg(moduleName),
          qSlicerWelcomeModuleWidget::tr("Unfortunately, this requested module is not available in this Slicer session."),
          QMessageBox::Ok);
    return false;
    }
  qSlicerLayoutManager * layoutManager = qSlicerApplication::application()->layoutManager();
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

  connect(d->LoadDicomDataButton, SIGNAL(clicked()),
          this, SLOT (loadDicomData()));
  connect(d->LoadNonDicomDataButton, SIGNAL(clicked()),
          this, SLOT (loadNonDicomData()));
  connect(d->LoadSampleDataButton, SIGNAL(clicked()),
          this, SLOT (loadRemoteSampleData()));
  connect(d->EditApplicationSettingsButton, SIGNAL(clicked()),
          this, SLOT (editApplicationSettings()));
  connect(d->ExploreLoadedDataPushButton, SIGNAL(clicked()),
          this, SLOT (exploreLoadedData()));

#ifndef Slicer_BUILD_DICOM_SUPPORT
  d->LoadDicomDataButton->hide();
#endif

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  qSlicerApplication* app = qSlicerApplication::application();
  bool extensionsManagerEnabled = app && app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool();
  if (extensionsManagerEnabled)
    {
    QObject::connect(d->OpenExtensionsManagerButton, SIGNAL(clicked()),
      qSlicerApplication::application(), SLOT(openExtensionsManagerDialog()));
    qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
    if (extensionsManagerModel)
      {
      QObject::connect(extensionsManagerModel, SIGNAL(extensionUpdatesAvailable(bool)),
        this, SLOT(setExtensionUpdatesAvailable(bool)));
      this->setExtensionUpdatesAvailable(!extensionsManagerModel->availableUpdateExtensions().empty());

      if (!extensionsManagerModel->autoUpdateCheck())
        {
        // if automatic update check is disabled then updates may be available, we just don't know
        d->UpdatesStatusLabel->hide();
        }

      d->CheckForUpdatesButton->setCheckState(extensionsManagerModel->autoUpdateCheck() ? Qt::Checked : Qt::Unchecked);

      QObject::connect(extensionsManagerModel, SIGNAL(autoUpdateSettingsChanged()),
        this, SLOT(onAutoUpdateSettingsChanged()));
      }

    QObject::connect(d->CheckForUpdatesButton, SIGNAL(checkBoxToggled(bool)),
      this, SLOT(onAutoUpdateCheckToggled(bool)));

    QObject::connect(d->CheckForUpdatesButton, SIGNAL(clicked()),
      this, SLOT(checkForUpdates()));

    QObject::connect(d->UpdatesFoundOpenExtensionsManagerButton, SIGNAL(clicked()),
      qSlicerApplication::application(), SLOT(openExtensionsManagerDialog()));
    }
  else
    {
    d->OpenExtensionsManagerButton->hide();
    d->AutomaticUpdatesCollapsibleWidget->hide(); // only extensions can be updated now
    }
#else
  d->OpenExtensionsManagerButton->hide();
  d->AutomaticUpdatesCollapsibleWidget->hide(); // only extensions can be updated now
#endif

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
  qSlicerIOManager *ioManager = qSlicerApplication::application()->ioManager();
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
void qSlicerWelcomeModuleWidget::setExtensionUpdatesAvailable(bool updateAvailable)
{
#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  Q_D(qSlicerWelcomeModuleWidget);

  // Check if there was a change
  const char extensionUpdateAvailablePropertyName[] = "extensionUpdateAvailable";
  if (d->OpenExtensionsManagerButton->property(extensionUpdateAvailablePropertyName).toBool() != updateAvailable)
    {
    // changed
    d->OpenExtensionsManagerButton->setProperty(extensionUpdateAvailablePropertyName, updateAvailable);
    if (updateAvailable)
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
    d->UpdatesStatusLabel->show();
    d->UpdatesStatusLabel->setText(tr("No extension updates were found."));
    d->UpdatesFoundOpenExtensionsManagerButton->hide();
    }
  else
    {
    d->UpdatesStatusLabel->hide();
    d->UpdatesFoundOpenExtensionsManagerButton->setText(tr(" %1 extension updates were found").arg(availableUpdates.size()));
    d->UpdatesFoundOpenExtensionsManagerButton->setToolTip(
      tr("Use Extensions Manager to update these extensions:")
      + QString("\n- ") + availableUpdates.join("\n- "));
    d->UpdatesFoundOpenExtensionsManagerButton->show();
    }
#else
  Q_UNUSED(updateAvailable);
#endif
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::checkForUpdates()
{
  Q_D(qSlicerWelcomeModuleWidget);
  qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
  if (!extensionsManagerModel)
    {
    return;
    }

  d->UpdatesStatusLabel->setText(tr("Checking for updates..."));
  d->UpdatesStatusLabel->show();
  d->UpdatesFoundOpenExtensionsManagerButton->hide();
  extensionsManagerModel->updateExtensionsMetadataFromServer(true, true);
  extensionsManagerModel->checkForExtensionsUpdates();
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::onAutoUpdateCheckToggled(bool autoUpdate)
{
  Q_D(qSlicerWelcomeModuleWidget);
  qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
  if (!extensionsManagerModel)
    {
    return;
    }
  extensionsManagerModel->setAutoUpdateCheck(autoUpdate);
}

//-----------------------------------------------------------------------------
void qSlicerWelcomeModuleWidget::onAutoUpdateSettingsChanged()
{
  Q_D(qSlicerWelcomeModuleWidget);
  qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
  if (!extensionsManagerModel)
    {
    return;
    }
  QSignalBlocker blocker(d->UpdatesFoundOpenExtensionsManagerButton);
  d->CheckForUpdatesButton->setCheckState(extensionsManagerModel->autoUpdateCheck() ? Qt::Checked : Qt::Unchecked);
}
