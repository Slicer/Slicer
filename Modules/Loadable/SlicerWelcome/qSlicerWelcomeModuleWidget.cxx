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
#include "qSlicerExtensionsManagerModel.h"
#endif
#ifdef Slicer_BUILD_APPLICATIONUPDATE_SUPPORT
#include "qSlicerApplicationUpdateManager.h"
#endif

// CTK includes
#include "ctkButtonGroup.h"

// qMRML includes
#include "qMRMLWidget.h"

class qSlicerAppMainWindow;

//-----------------------------------------------------------------------------
class qSlicerWelcomeModuleWidgetPrivate: public Ui_qSlicerWelcomeModuleWidget
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

  // Generate welcome modules translatable texts (without HTML tags)

  QString labelText = QString(
    "<html>"
        "<head></head>"
        "<body>"
            "<p align='center'>"
                "<span style='font-size:22pt; color:#afb7d5;'>%1</span>"
            "</p>"
        "</body>"
    "</html>"
  ).arg(qSlicerWelcomeModuleWidget::tr("Welcome"));

  QString OpenExtensionsManagerButtonText = qSlicerWelcomeModuleWidget::tr(
    "Raise the \"Extensions Manager\" wizard that allows to find, download and install "
    "Slicer extensions.\n\nAn extension is a delivery package bundling together one or more "
    "Slicer modules.\n\nAfter installing an extension, the associated modules will be available "
    "in the module selector.");

  QString feedbackTextBrowser = QString(
    "<table align='center' border='0' cellpadding='5' cellspacing='0'>"
        "<tr>"
            "<td style='vertical-align:middle;'>"
                "<p>"
                    "<a href='https://discourse.slicer.org'>"
                        "<img src='qrc:Icons/Chat.png' style='vertical-align: middle;' width='64'/>"
                    "</a>"
                "</p>"
            "</td>"
            "<td style='vertical-align:middle;'>"
                "<p>%1</p>"
            "</td>"
        "</tr>"
    "</table>"
    "<p>%2</p>"
  ).arg(
    //: %1 and %2 are styling arguments
    qSlicerWelcomeModuleWidget::tr(
      "Share your stories with us on the %1 Slicer forum %2 "
      "and let us know about how 3D Slicer has enabled your research."
    ).arg("<a href='https://discourse.slicer.org'>").arg("</a>")
  ).arg(
    //: %1 is a styling argument
    qSlicerWelcomeModuleWidget::tr(
      "We are always interested in improving 3D Slicer. To tell us about your problem or submit a "
      "bug report, open %1 Help -> Report a Bug."
    ).arg("<b>") + "</b>"
  );

  QString appTitle = QString("<b>%1</b>").arg(qSlicerWelcomeModuleWidget::tr("3D Slicer"));
  QString aboutTextBrowser = QString(
    "<p>%1</p>"
    "<p>%2</p>"
  ).arg(
    //: %1 is the app title, %2 and %3 are styling arguments
    qSlicerWelcomeModuleWidget::tr(
      "%1 is a free open source software platform for medical image processing and 3D visualization "
      "of image data. This module contains some basic information and useful links to get you started "
      "using Slicer. For more information, please visit our website %2 https://www.slicer.org %3"
    ).arg(appTitle).arg("<a href='https://www.slicer.org'>").arg("</a>.")
  ).arg(
    //: %1 is the app title, %2 and %3 are styling arguments
    qSlicerWelcomeModuleWidget::tr(
      "%1 is distributed under a BSD-style license; for details about the contribution and software "
      "license agreement, please see the %2 3D Slicer Software License Agreement %3 "
      "This software has been designed for research purposes only and has not been reviewed or approved "
      "by the Food and Drug Administration, or by any other agency."
    ).arg(appTitle).arg("<a href='https://github.com/Slicer/Slicer/blob/main/License.txt'>").arg("</a>.")
  );

  QString documentationTextBrowser = QString(
    "%1 %2 %3"
  ).arg(
    QString(
      "<p><b>%1</b></p>"
      "<ul>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/'>%1</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/getting_started.html#quick-start'>%2</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/get_help.html'>%3</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/user_interface.html'>%4</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/user_interface.html#mouse-keyboard-shortcuts'>%5</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/getting_started.html#tutorials'>%6</a></li>"
      "</ul>"
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Documentation")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Quick Start")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Get Help")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("User Interface")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Mouse Buttons, \"Hot-keys\" and Keyboard Shortcuts")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Browse Tutorials")
    )
  ).arg(
    QString(
      "<p><b>%1</b></p>"
      "<ul>"
        "<li><a href='https://discourse.slicer.org/'>%2</a></li>"
        "<li><a href='https://www.linkedin.com/feed/hashtag/?keywords=3dslicer'>%3</a></li>"
        "<li><a href='https://discourse.slicer.org/c/support/feature-requests/9'>%4</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/get_help.html#i-want-to-report-a-problem'>%5</a></li>"
      "</ul>"
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Contact Us")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Visit the Slicer Forum")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Join Us on LinkedIn")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Search Feature Requests")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Report a Bug")
    )
  ).arg(
    QString(
      "<p><b>%1</b></p>"
      "<ul>"
        "<li><a href='https://github.com/Slicer/Slicer/blob/main/License.txt'>%2</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/about.html#how-to-cite'>%3</a></li>"
        "<li><a href='https://scholar.google.com/scholar?&as_sdt=1,22&as_vis=1&q=(\"3D+Slicer\"+OR+\"slicer+org\"+OR+Slicer3D)+-Slic3r+&btnG='>%4</a></li>"
        "<li><a href='https://slicer.readthedocs.io/en/latest/user_guide/about.html#acknowledgments'>%5</a></li>"
      "</ul>"
    ).arg(
      qSlicerWelcomeModuleWidget::tr("About 3D Slicer")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("View License")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("How to Cite")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Slicer Publications")
    ).arg(
      qSlicerWelcomeModuleWidget::tr("Acknowledgments")
    )
  );

  QString acknowledgmentTextBrowser = QString(
    "<table align='center' cellpadding='5' cellspacing='0'>"
        "<tr>"
            "<td>"
                "<p align='center'>"
                    "<a href='https://www.na-mic.org/'>"
                        "<img src='qrc:Logos/NAMIC.png' style='vertical-align: middle;' width='80'/>"
                    "</a>"
                "</p>"
            "</td>"
            "<td>"
                "<p align='center'>"
                    "<a href='https://nac.spl.harvard.edu/'>"
                        "<img src='qrc:Logos/NAC.png' style='vertical-align: middle;'/>"
                    "</a>"
                "</p>"
            "</td>"
            "<td>"
                "<p align='center'>"
                    "<a href='https://www.ncigt.org/'>"
                        "<img src='qrc:Logos/NCIGT.png' style='vertical-align: middle;'/>"
                    "</a>"
                "</p>"
            "</td>"
            "<td>"
                "<p align='center'>"
                    "<a href='https://www.spl.harvard.edu/'>"
                        "<img src='qrc:Logos/SPL.png' style='vertical-align: middle;'/>"
                    "</a>"
                "</p>"
            "</td>"
        "</tr>"
    "</table>"
    "<p>%1</p>"
    "<p>%2</p>"
    "<p>%3</p>"
  ).arg(
    //: All arguments are for styling purposes
    qSlicerWelcomeModuleWidget::tr(
      "Development of 3D Slicer is supported by %1 NA-MIC %2, %3 NAC %2, %4 NCIGT %2, %5 SPL %2 "
      "and the %6 Slicer Community %2. See %7 https://www.slicer.org %2 for details."
    ).arg("<a href='https://www.na-mic.org/'>").arg("</a>").arg("<a href='https://nac.spl.harvard.edu/'>")
     .arg("<a href='https://www.ncigt.org/'>").arg("<a href='https://www.spl.harvard.edu'>")
     .arg("<a href='https://slicer.readthedocs.io/en/latest/user_guide/about.html#acknowledgments'>")
     .arg("<a href='https://www.slicer.org'>")
  ).arg(
    qSlicerWelcomeModuleWidget::tr(
      "We would also like to express our sincere thanks to members of the Slicer User Community who have helped us "
      "to design the contents of this Welcome Module, and whose feedback continues to improve functionality, usability "
      "and Slicer user experience."
    )
  ).arg(
    qSlicerWelcomeModuleWidget::tr(
      "This module was developed by Jean-Christophe Fillion-Robin and Julien Finet at Kitware, Inc., and by Wendy Plesniak, "
      "Sonia Pujol, Steve Pieper and Ron Kikinis at Brigham and Women's Hospital."
    )
  );

  this->label->setText(labelText);
  this->OpenExtensionsManagerButton->setToolTip(OpenExtensionsManagerButtonText);
  this->feedbackTextBrowser->setHtml(feedbackTextBrowser.replace(" </a>", "</a>"));
  this->aboutTextBrowser->setHtml(aboutTextBrowser.replace(" </a>", "</a>"));
  this->documentationTextBrowser->setHtml(documentationTextBrowser);
  this->acknowledgmentTextBrowser->setHtml(acknowledgmentTextBrowser.replace(" </a>", "</a>"));

  // Create the button group ensuring that only one collabsibleWidgetButton will be open at a time
  ctkButtonGroup * group = new ctkButtonGroup(widget);

  // Add all collabsibleWidgetButton to a button group
  QList<ctkCollapsibleButton*> collapsibles = widget->findChildren<ctkCollapsibleButton*>();
  foreach(ctkCollapsibleButton* collapsible, collapsibles)
  {
    group->addButton(collapsible);
  }

  // Update occurrences of documentation URLs
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  foreach(QWidget* widget, QWidgetList()
          << this->FeedbackCollapsibleWidget
          << this->WelcomeAndAboutCollapsibleWidget
          << this->OtherUsefulHintsCollapsibleWidget
          << this->AcknowledgmentCollapsibleWidget
          )
  {
    QTextBrowser* textBrowser = widget->findChild<QTextBrowser*>();
    if (!textBrowser)
    {
      continue;
    }
    QString html = textBrowser->toHtml();
    qSlicerUtils::replaceDocumentationUrlVersion(html,
      QUrl(app->documentationBaseUrl()).host(), app->documentationVersion());
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

  qSlicerApplication* app = qSlicerApplication::application();

  bool extensionUpdatesEnabled = false;
  bool applicationUpdatesEnabled = false;

#ifdef Slicer_BUILD_EXTENSIONMANAGER_SUPPORT
  if (app && app->revisionUserSettings()->value("Extensions/ManagerEnabled").toBool())
  {
    QObject::connect(d->OpenExtensionsManagerButton, SIGNAL(clicked()),
      qSlicerApplication::application(), SLOT(openExtensionsManagerDialog()));
    qSlicerExtensionsManagerModel* extensionsManagerModel = d->extensionsManagerModel();
    if (extensionsManagerModel)
    {
      extensionUpdatesEnabled = true;
      QObject::connect(extensionsManagerModel, SIGNAL(extensionUpdatesAvailable(bool)),
        this, SLOT(setExtensionUpdatesAvailable(bool)));

      if (!extensionsManagerModel->availableUpdateExtensions().empty())
      {
        this->setExtensionUpdatesAvailable(true);
      }

      QObject::connect(extensionsManagerModel, SIGNAL(autoUpdateSettingsChanged()),
        this, SLOT(onAutoUpdateSettingsChanged()));
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
    QObject::connect(d->ApplicationUpdateAvailableButton, SIGNAL(clicked()),
      qSlicerApplication::application(), SLOT(openApplicationDownloadWebsite()));
    QObject::connect(d->ApplicationUpdateStatusButton, SIGNAL(clicked()),
      qSlicerApplication::application(), SLOT(openApplicationDownloadWebsite()));
    qSlicerApplicationUpdateManager* applicationUpdateManager = d->applicationUpdateManager();
    if (applicationUpdateManager)
    {
      applicationUpdatesEnabled = true;
      QObject::connect(applicationUpdateManager, SIGNAL(updateAvailable(bool)),
        this, SLOT(setApplicationUpdateAvailable(bool)));
      if (applicationUpdateManager->isUpdateAvailable())
      {
        this->setApplicationUpdateAvailable(true);
      }

      QObject::connect(applicationUpdateManager, SIGNAL(autoUpdateCheckChanged()),
        this, SLOT(onAutoUpdateSettingsChanged()));
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
    QObject::connect(d->CheckForUpdatesAutomaticallyCheckBox, SIGNAL(stateChanged(int)),
      this, SLOT(onAutoUpdateCheckStateChanged(int)));

    QObject::connect(d->CheckForUpdatesNowButton, SIGNAL(clicked()),
      this, SLOT(checkForUpdates()));

    QObject::connect(d->ExtensionUpdatesStatusButton, SIGNAL(clicked()),
      qSlicerApplication::application(), SLOT(openExtensionsManagerDialog()));
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
      tr("%1 extension update is available", "%1 extension updates are available", availableUpdates.size()).arg(availableUpdates.size()));
    d->ExtensionUpdatesStatusButton->setToolTip(
      tr("Use Extensions Manager to update these extensions:")
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
