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

#include "vtkSlicerConfigure.h" // For Slicer_BUILD_WEBENGINE_SUPPORT

// Qt includes
#include <QDesktopServices>
#include <QFileDialog>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTimer>
#include <QTimerEvent>
#include <QToolButton>
#include <QUrlQuery>
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
# include <QWebEngineHistory>
# include <QWebEnginePage>
# include <QWebEngineView>
#endif

// CTK includes
#include <ctkSearchBox.h>
#include <ctkMessageBox.h>

// QtGUI includes
#include <qSlicerApplication.h>
#include "qSlicerExtensionsManagerWidget.h"
#include "qSlicerExtensionsManagerModel.h"
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
# include "qSlicerExtensionsServerWidget.h"
#endif
#include "ui_qSlicerExtensionsActionsWidget.h"
#include "ui_qSlicerExtensionsManagerWidget.h"
#include "ui_qSlicerExtensionsToolsWidget.h"

// --------------------------------------------------------------------------
namespace
{

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
QString jsQuote(QString text)
{
  // NOTE: This assumes that 'text' does not contain '\r' or other control characters
  static QRegExp reSpecialCharacters("([\'\"\\\\])");
  text.replace(reSpecialCharacters, "\\\\1").replace("\n", "\\n");
  return QString("\'%1\'").arg(text);
}
#endif

// --------------------------------------------------------------------------
void invalidateSizeHint(QToolButton* button)
{
  // Invalidate cached size hint of QToolButton... this seems to be necessary
  // to get the initially visible button to have the correct hint for having a
  // menu indicator included; otherwise the configure buttons end up with
  // different sizes, causing the UI to "jump" when switching tabs
  //
  // NOTE: This depends on some knowledge of the QToolButton internals;
  //       specifically, that changing the toolButtonStyle will invalidate the
  //       hint (given that we are toggling visibility of the text, it seems
  //       pretty safe to assume this will always do the trick)
  //
  // See https://bugreports.qt-project.org/browse/QTBUG-38949
  button->setToolButtonStyle(Qt::ToolButtonTextOnly);
  button->setToolButtonStyle(Qt::ToolButtonIconOnly);
}

//---------------------------------------------------------------------------
void setThemeIcon(QAbstractButton* button, const QString& name)
{
  // TODO: Can do this in the .ui once Qt 4.8 is required
  button->setIcon(QIcon::fromTheme(name, button->icon()));
}

//---------------------------------------------------------------------------
void setThemeIcon(QAction* action, const QString& name)
{
  // TODO: Can do this in the .ui once Qt 4.8 is required
  action->setIcon(QIcon::fromTheme(name, action->icon()));
}

// --------------------------------------------------------------------------
class qSlicerExtensionsActionsWidget
  : public QStackedWidget
  , public Ui_qSlicerExtensionsActionsWidget
{
public:
  qSlicerExtensionsActionsWidget(QWidget* parent = nullptr)
    : QStackedWidget(parent)
  {
    this->setupUi(this);
  }
};

// --------------------------------------------------------------------------
class qSlicerExtensionsToolsWidget
  : public QWidget
  , public Ui_qSlicerExtensionsToolsWidget
{
public:
  qSlicerExtensionsToolsWidget(QWidget* parent = nullptr)
    : QWidget(parent)
  {
    this->setupUi(this);

    setThemeIcon(this->ConfigureButton, "configure");
    setThemeIcon(this->CheckForUpdatesAction, "view-refresh");

    const QIcon searchIcon = QIcon::fromTheme("edit-find", QPixmap(":/Icons/Search.png"));
    const QIcon clearIcon = QIcon::fromTheme(this->layoutDirection() == Qt::LeftToRight ? "edit-clear-locationbar-rtl"
                                                                                        : "edit-clear-locationbar-ltr",
                                             this->SearchBox->clearIcon());

    const QFontMetrics fm = this->SearchBox->fontMetrics();
    const int searchWidth = 24 * fm.averageCharWidth() + 40;

    this->SearchBox->setClearIcon(clearIcon);
    this->SearchBox->setSearchIcon(searchIcon);
    this->SearchBox->setShowSearchIcon(true);
    this->SearchBox->setFixedWidth(searchWidth);

    // manage
    QMenu* configureMenu = new QMenu(this);
    configureMenu->addAction(this->CheckForUpdatesAction);
    configureMenu->addAction(this->AutoUpdateCheckAction);
    configureMenu->addAction(this->AutoUpdateInstallAction);
    configureMenu->addAction(this->AutoInstallDependenciesAction);
    configureMenu->addAction(this->EditBookmarksAction);
    configureMenu->addAction(this->OpenExtensionsCatalogWebsiteAction);

    this->ConfigureButton->setMenu(configureMenu);
    invalidateSizeHint(this->ConfigureButton);
  }
};

} // namespace

//-----------------------------------------------------------------------------
class qSlicerExtensionsManagerWidgetPrivate : public Ui_qSlicerExtensionsManagerWidget
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManagerWidget);

protected:
  qSlicerExtensionsManagerWidget* const q_ptr;

public:
  qSlicerExtensionsManagerWidgetPrivate(qSlicerExtensionsManagerWidget& object);
  void init();
  bool setBatchProcessing(bool newMode);

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  qSlicerExtensionsServerWidget* ExtensionsManageBrowser;
  qSlicerExtensionsServerWidget* ExtensionsServerWidget;
#endif

  qSlicerExtensionsToolsWidget* ToolsWidget;
  QString LastInstallWidgetSearchText;
  QUrl LastInstallWidgetUrl;
  int SearchTimerId;
  bool IsBatchProcessing{
    false
  }; // in the process of installing or updating multiple extensions - should not close the widget

  QMessageBox* MessageWidget{ nullptr };
  QStringList Messages;
  QTimer* MessageWidgetAutoCloseTimer;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerWidgetPrivate::qSlicerExtensionsManagerWidgetPrivate(qSlicerExtensionsManagerWidget& object)
  : q_ptr(&object)
  , ToolsWidget(nullptr)
  , SearchTimerId(0)
{
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  this->ExtensionsManageBrowser = nullptr;
  this->ExtensionsServerWidget = nullptr;
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerWidget);

  this->setupUi(q);

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  // Setup browser for "Install Extensions" tab
  this->ExtensionsServerWidget = new qSlicerExtensionsServerWidget(this->InstallExtensionsTab);
  this->ExtensionsServerWidget->setObjectName("ExtensionsServerWidget");
  this->InstallExtensionsTabLayout->addWidget(this->ExtensionsServerWidget);

  // Setup browser for "Manage Extensions" tab
  this->ExtensionsManageBrowser = new qSlicerExtensionsServerWidget();
  this->ExtensionsManageBrowser->setObjectName("ExtensionsManageBrowser");
  this->ManageExtensionsPager->addWidget(this->ExtensionsManageBrowser);
  this->ExtensionsManageBrowser->setBrowsingEnabled(false);
  this->ExtensionsManageBrowser->webView()->load(QUrl(/*no tr*/ "about:"));
#endif

  qSlicerExtensionsActionsWidget* actionsWidget = new qSlicerExtensionsActionsWidget;

  // Back and forward buttons
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  actionsWidget->ManageBackButton->setDefaultAction(
    this->ExtensionsManageBrowser->webView()->pageAction(QWebEnginePage::Back));
  actionsWidget->ManageForwardButton->setDefaultAction(
    this->ExtensionsManageBrowser->webView()->pageAction(QWebEnginePage::Forward));
  actionsWidget->InstallBackButton->setDefaultAction(
    this->ExtensionsServerWidget->webView()->pageAction(QWebEnginePage::Back));
  actionsWidget->InstallForwardButton->setDefaultAction(
    this->ExtensionsServerWidget->webView()->pageAction(QWebEnginePage::Forward));
#endif

  this->tabWidget->setCornerWidget(actionsWidget, Qt::TopLeftCorner);

  // Search field and configure button
  this->ToolsWidget = new qSlicerExtensionsToolsWidget;

  QObject::connect(this->ToolsWidget->AutoUpdateCheckAction, SIGNAL(toggled(bool)), q, SLOT(setAutoUpdateCheck(bool)));
  QObject::connect(
    this->ToolsWidget->AutoUpdateInstallAction, SIGNAL(toggled(bool)), q, SLOT(setAutoUpdateInstall(bool)));
  QObject::connect(
    this->ToolsWidget->AutoInstallDependenciesAction, SIGNAL(toggled(bool)), q, SLOT(setAutoInstallDependencies(bool)));

  this->tabWidget->setCornerWidget(this->ToolsWidget, Qt::TopRightCorner);

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)), actionsWidget, SLOT(setCurrentIndex(int)));
  QObject::connect(this->ExtensionsLocalWidget, SIGNAL(linkActivated(QUrl)), q, SLOT(onManageLinkActivated(QUrl)));

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  QObject::connect(
    this->ExtensionsManageBrowser->webView(), SIGNAL(urlChanged(QUrl)), q, SLOT(onManageUrlChanged(QUrl)));
  QObject::connect(this->ToolsWidget->SearchBox, SIGNAL(textEdited(QString)), q, SLOT(onSearchTextChanged(QString)));
  QObject::connect(
    this->ExtensionsServerWidget->webView(), SIGNAL(urlChanged(QUrl)), q, SLOT(onInstallUrlChanged(QUrl)));
#endif

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)), q, SLOT(onCurrentTabChanged(int)));
  QObject::connect(
    this->ToolsWidget->CheckForUpdatesAction, SIGNAL(triggered(bool)), q, SLOT(onCheckForUpdatesTriggered()));
  QObject::connect(
    this->ToolsWidget->EditBookmarksAction, SIGNAL(triggered(bool)), q, SLOT(onEditBookmarksTriggered()));
  qSlicerApplication* app = qSlicerApplication::application();
  QObject::connect(this->ToolsWidget->OpenExtensionsCatalogWebsiteAction,
                   SIGNAL(triggered(bool)),
                   app,
                   SLOT(openExtensionsCatalogWebsite()));
  QObject::connect(this->ToolsWidget->CheckForUpdatesButton, SIGNAL(clicked()), q, SLOT(onCheckForUpdatesTriggered()));
  QObject::connect(this->ToolsWidget->InstallUpdatesButton, SIGNAL(clicked()), q, SLOT(onInstallUpdatesTriggered()));
  QObject::connect(
    this->ToolsWidget->InstallBookmarkedButton, SIGNAL(clicked()), q, SLOT(onInstallBookmarkedTriggered()));
  QObject::connect(this->ToolsWidget->InstallFromFileButton, SIGNAL(clicked()), q, SLOT(onInstallFromFileTriggered()));

  this->MessageWidget = new QMessageBox(q);
  this->MessageWidget->setWindowTitle(qSlicerExtensionsManagerWidget::tr("Extensions Manager"));
  this->MessageWidgetAutoCloseTimer = new QTimer(q);
  this->MessageWidgetAutoCloseTimer->setInterval(500);
  this->MessageWidgetAutoCloseTimer->setSingleShot(true);
  QObject::connect(this->MessageWidgetAutoCloseTimer, SIGNAL(timeout()), this->MessageWidget, SLOT(accept()));
  QObject::connect(this->MessageWidget, SIGNAL(finished(int)), q, SLOT(onMessagesAcknowledged()));
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerWidgetPrivate::setBatchProcessing(bool newMode)
{
  Q_Q(qSlicerExtensionsManagerWidget);
  bool wasBatchProcessing = this->IsBatchProcessing;
  this->IsBatchProcessing = newMode;
  if (wasBatchProcessing != this->IsBatchProcessing)
  {
    if (!this->IsBatchProcessing)
    {
      QApplication::restoreOverrideCursor();
    }
    emit q->inBatchProcessing(this->IsBatchProcessing);
    if (this->IsBatchProcessing)
    {
      QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    }
  }
  return wasBatchProcessing;
}

// --------------------------------------------------------------------------
// qSlicerExtensionsManagerWidget methods

// --------------------------------------------------------------------------
qSlicerExtensionsManagerWidget::qSlicerExtensionsManagerWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsManagerWidgetPrivate(*this))
{
  Q_D(qSlicerExtensionsManagerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerWidget::~qSlicerExtensionsManagerWidget() {}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsManagerWidget::extensionsManagerModel() const
{
  Q_D(const qSlicerExtensionsManagerWidget);
  return d->ExtensionsLocalWidget->extensionsManagerModel();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsManagerWidget);

  if (this->extensionsManagerModel() == model)
  {
    return;
  }

  disconnect(this, SLOT(onModelUpdated()));
  disconnect(this, SLOT(onMessageLogged(QString, ctkErrorLogLevel::LogLevels)));

  d->ExtensionsLocalWidget->setExtensionsManagerModel(model);
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  d->ExtensionsManageBrowser->setExtensionsManagerModel(model);
  d->ExtensionsServerWidget->setExtensionsManagerModel(model);
#endif

  if (model)
  {
    this->updateAutoUpdateWidgetsFromModel();

    this->onModelUpdated();
    connect(model, SIGNAL(modelUpdated()), this, SLOT(onModelUpdated()));
    connect(model,
            SIGNAL(messageLogged(QString, ctkErrorLogLevel::LogLevels)),
            this,
            SLOT(onMessageLogged(QString, ctkErrorLogLevel::LogLevels)));
    connect(model, SIGNAL(extensionInstalled(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionUninstalled(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionScheduledForUpdate(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionCancelledScheduleForUpdate(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionBookmarkedChanged(QString, bool)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionUpdateAvailable(QString)), this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(autoUpdateSettingsChanged()), this, SLOT(updateAutoUpdateWidgetsFromModel()));

    // Ensure extension metadata is retrieved from the server or cache.
    // This is performed early during application startup, except the very first time after install,
    // because then the default extension server address is set later during startup.
    if (!model->lastUpdateTimeExtensionsMetadataFromServer().isValid())
    {
      model->updateExtensionsMetadataFromServer();
    }
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::updateAutoUpdateWidgetsFromModel()
{
  Q_D(qSlicerExtensionsManagerWidget);
  QSignalBlocker blocker1(d->ToolsWidget->AutoUpdateInstallAction);
  QSignalBlocker blocker2(d->ToolsWidget->AutoUpdateCheckAction);
  QSignalBlocker blocker3(d->ToolsWidget->AutoInstallDependenciesAction);
  d->ToolsWidget->AutoUpdateCheckAction->setChecked(this->extensionsManagerModel()->autoUpdateCheck());
  d->ToolsWidget->AutoUpdateInstallAction->setChecked(this->extensionsManagerModel()->autoUpdateInstall());
  d->ToolsWidget->AutoInstallDependenciesAction->setChecked(this->extensionsManagerModel()->autoInstallDependencies());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::refreshInstallWidget()
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->updateExtensionsMetadataFromServer();
  this->extensionsManagerModel()->updateModel();

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  d->ExtensionsServerWidget->refresh();
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onModelUpdated()
{
  Q_D(qSlicerExtensionsManagerWidget);

  int manageExtensionsTabIndex = d->tabWidget->indexOf(d->ManageExtensionsTab);
  int managedExtensionsCount = this->extensionsManagerModel()->managedExtensionsCount();

  // Get the list of extensions that have update available but not updated yet
  QStringList extensionsToUpdate = this->extensionsManagerModel()->availableUpdateExtensions();
  const QStringList& extensionsAlreadyScheduledForUpdate =
    this->extensionsManagerModel()->scheduledForUpdateExtensions();
  foreach (const QString& extensionName, extensionsAlreadyScheduledForUpdate)
  {
    extensionsToUpdate.removeAll(extensionName);
  }
  int extensionUpdates = extensionsToUpdate.size();
  if (extensionUpdates > 0)
  {
    d->ToolsWidget->InstallUpdatesButton->setText(tr("Update all (%1)").arg(extensionUpdates));
    d->ToolsWidget->InstallUpdatesButton->setVisible(true);
    d->ToolsWidget->CheckForUpdatesButton->setVisible(false);
  }
  else
  {
    d->ToolsWidget->InstallUpdatesButton->setVisible(false);
    d->ToolsWidget->CheckForUpdatesButton->setVisible(true);
    d->ToolsWidget->CheckForUpdatesButton->setEnabled(this->extensionsManagerModel()->managedExtensionsCount() > 0);
  }

  int foundNonInstalledBookmarkedExtension = 0;
  QStringList bookmarkedExtensions = this->extensionsManagerModel()->bookmarkedExtensions();
  foreach (const QString& extensionName, bookmarkedExtensions)
  {
    if (this->extensionsManagerModel()->isExtensionInstalled(extensionName))
    {
      // already installed
      continue;
    }
    const qSlicerExtensionsManagerModel::ExtensionMetadataType& metadataFromServer =
      this->extensionsManagerModel()->extensionMetadata(extensionName, qSlicerExtensionsManagerModel::MetadataServer);
    if (metadataFromServer["revision"].toString().isEmpty())
    {
      // not available on server
      continue;
    }
    foundNonInstalledBookmarkedExtension++;
  }
  if (foundNonInstalledBookmarkedExtension > 0)
  {
    d->ToolsWidget->InstallBookmarkedButton->setText(
      tr("Install bookmarked (%1)").arg(foundNonInstalledBookmarkedExtension));
    d->ToolsWidget->InstallBookmarkedButton->setEnabled(true);
  }
  else
  {
    d->ToolsWidget->InstallBookmarkedButton->setText(tr("Install bookmarked"));
    d->ToolsWidget->InstallBookmarkedButton->setEnabled(false);
  }

  d->tabWidget->setTabText(manageExtensionsTabIndex,
                           //: %1 represents the number of managed extensions
                           tr("Manage Extensions (%1)").arg(managedExtensionsCount));

  if (managedExtensionsCount == 0)
  {
    d->tabWidget->setTabEnabled(manageExtensionsTabIndex, false);
    d->tabWidget->setCurrentWidget(d->InstallExtensionsTab);
  }
  else
  {
    d->tabWidget->setTabEnabled(manageExtensionsTabIndex, true);
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onCheckForUpdatesTriggered()
{
  Q_D(qSlicerExtensionsManagerWidget);
  bool wasBatchProcessing = d->setBatchProcessing(true);
  this->extensionsManagerModel()->updateExtensionsMetadataFromServer(true, true);
  this->extensionsManagerModel()->checkForExtensionsUpdates();
  d->setBatchProcessing(wasBatchProcessing);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onEditBookmarksTriggered()
{
  Q_D(qSlicerExtensionsManagerWidget);
  bool ok = false;
  QStringList oldList = this->extensionsManagerModel()->bookmarkedExtensions();
  QString newStr = QInputDialog::getMultiLineText(
    this, tr("Bookmarked extensions"), tr("List of bookmarked extensions:"), oldList.join("\n"), &ok);
  if (!ok)
  {
    // Cancel clicked
    return;
  }
  // Split along whitespaces and common separator characters
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
  QStringList newList = newStr.split(QRegExp("\\s+|,|;"), Qt::SkipEmptyParts);
#else
  QStringList newList = newStr.split(QRegExp("\\s+|,|;"), QString::SkipEmptyParts);
#endif
  newList.removeDuplicates();

  // Update bookmarks
  QSettings settings;
  settings.setValue(/*no tr*/ "Extensions/Bookmarked", newList);
  this->extensionsManagerModel()->updateModel();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onCurrentTabChanged(int index)
{
  Q_D(qSlicerExtensionsManagerWidget);
  Q_UNUSED(index);
  if (d->tabWidget->currentWidget() == d->ManageExtensionsTab)
  {
    d->ToolsWidget->SearchBox->setEnabled(true);
  }
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  else if (d->tabWidget->currentWidget() == d->InstallExtensionsTab)
  {
    QWebEngineHistory* history = d->ExtensionsManageBrowser->webView()->history();
    if (history->canGoBack())
    {
      history->goToItem(history->items().first());
    }
    bool isCatalogPage = false;
    int serverAPI = this->extensionsManagerModel()->serverAPI();
    if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
    {
      isCatalogPage = d->LastInstallWidgetUrl.path().contains("/catalog");
    }
    else
    {
      qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
    }
    if (isCatalogPage)
    {
      d->ToolsWidget->SearchBox->setEnabled(true);
      // When URL is changed because user clicked on a link then we want the search text
      // to be reset. However, when user is entering text in the searchbox (it has the focus)
      // then we must not overwrite the search text.
      if (!d->ToolsWidget->SearchBox->hasFocus())
      {
        QString lastSearchTextLoaded;
        if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
        {
          lastSearchTextLoaded = QUrlQuery(d->LastInstallWidgetUrl).queryItemValue("q");
        }
        d->ToolsWidget->SearchBox->setText(lastSearchTextLoaded);
      }
    }
    else
    {
      d->ToolsWidget->SearchBox->setEnabled(false);
    }
  }
#endif
  this->processSearchTextChange();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onManageLinkActivated(const QUrl& link)
{
  Q_D(qSlicerExtensionsManagerWidget);
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  d->ManageExtensionsPager->setCurrentIndex(1);
  d->ExtensionsManageBrowser->webView()->load(link);
#else
  Q_UNUSED(link);
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onManageUrlChanged(const QUrl& newUrl)
{
  Q_D(qSlicerExtensionsManagerWidget);
  d->ManageExtensionsPager->setCurrentIndex(newUrl.scheme() == /*no tr*/ "about" ? 0 : 1);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onInstallUrlChanged(const QUrl& newUrl)
{
  Q_D(qSlicerExtensionsManagerWidget);
  // refresh tools widget state (it should be only enabled if browsing the appstore)
  bool isCatalogPage = false;
  int serverAPI = this->extensionsManagerModel()->serverAPI();
  QString lastSearchTextLoaded;
  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    lastSearchTextLoaded = QUrlQuery(newUrl).queryItemValue("q");
    isCatalogPage = newUrl.path().contains("/catalog");
  }
  if (isCatalogPage)
  {
    d->ToolsWidget->SearchBox->setEnabled(true);
    // When URL is changed because user clicked on a link then we want the search text
    // to be reset. However, when user is entering text in the searchbox (it has the focus)
    // then we must not overwrite the search text.
    if (!d->ToolsWidget->SearchBox->hasFocus())
    {
      d->ToolsWidget->SearchBox->setText(lastSearchTextLoaded);
    }
  }
  else
  {
    d->ToolsWidget->SearchBox->setEnabled(false);
  }
  d->LastInstallWidgetUrl = newUrl;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onSearchTextChanged(const QString& newText)
{
  Q_UNUSED(newText);
  Q_D(qSlicerExtensionsManagerWidget);
  if (d->SearchTimerId)
  {
    this->killTimer(d->SearchTimerId);
    d->SearchTimerId = 0;
  }
  d->SearchTimerId = this->startTimer(500);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::processSearchTextChange()
{
  Q_D(qSlicerExtensionsManagerWidget);
  const QString& searchText = d->ToolsWidget->SearchBox->text();
  if (d->tabWidget->currentWidget() == d->ManageExtensionsTab)
  {
    d->ExtensionsLocalWidget->setSearchText(searchText);
  }
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  else if (d->tabWidget->currentWidget() == d->InstallExtensionsTab)
  {
    if (searchText != d->LastInstallWidgetSearchText)
    {
      int serverAPI = this->extensionsManagerModel()->serverAPI();
      if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
      {
        d->ExtensionsServerWidget->webView()->page()->runJavaScript("app.search(" + jsQuote(searchText) + ");");
      }
      else
      {
        qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
      }
      d->LastInstallWidgetSearchText = searchText;
    }
  }
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::timerEvent(QTimerEvent* e)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (e->timerId() == d->SearchTimerId)
  {
    this->processSearchTextChange();
    this->killTimer(d->SearchTimerId);
    d->SearchTimerId = 0;
  }
  QObject::timerEvent(e);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onInstallUpdatesTriggered()
{
  Q_D(qSlicerExtensionsManagerWidget);
  // Save last update check time
  bool wasBatchProcessing = d->setBatchProcessing(true);
  QStringList extensionNames = this->extensionsManagerModel()->availableUpdateExtensions();
  foreach (const QString& extensionName, extensionNames)
  {
    this->extensionsManagerModel()->scheduleExtensionForUpdate(extensionName);
  }
  d->setBatchProcessing(wasBatchProcessing);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onInstallBookmarkedTriggered()
{
  Q_D(qSlicerExtensionsManagerWidget);
  bool wasBatchProcessing = d->setBatchProcessing(true);
  // Save last update check time
  QStringList extensionNames = this->extensionsManagerModel()->bookmarkedExtensions();
  foreach (const QString& extensionName, extensionNames)
  {
    if (this->extensionsManagerModel()->isExtensionInstalled(extensionName))
    {
      // already installed
      continue;
    }
    const qSlicerExtensionsManagerModel::ExtensionMetadataType& metadata =
      this->extensionsManagerModel()->extensionMetadata(extensionName);
    QString extensionId = metadata.value("extension_id").toString();
    if (extensionId.isEmpty())
    {
      // not available on the server
      continue;
    }
    // Do not install dependencies, because installing in incorrect order could result in showing
    // installation confirmation popups. The user may also intentionally not want to install
    // some dependencies.
    bool installDependencies = false;
    this->extensionsManagerModel()->downloadAndInstallExtensionByName(extensionName, installDependencies);
  }
  d->setBatchProcessing(wasBatchProcessing);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onInstallFromFileTriggered()
{
  Q_D(qSlicerExtensionsManagerWidget);
  const QStringList& archiveNames = QFileDialog::getOpenFileNames(
    this,
    tr("Select extension archive file(s)..."),
    QString(),
    tr("Archives") + " (*.zip *.7z *.tar *.tar.gz *.tgz *.tar.bz2 *.tar.xz);;" + tr("All files") + " (*)");
  if (archiveNames.empty())
  {
    return;
  }

  bool wasBatchProcessing = d->setBatchProcessing(true);
  qSlicerExtensionsManagerModel* const model = this->extensionsManagerModel();
  foreach (const QString& archiveName, archiveNames)
  {
    model->installExtension(archiveName);
  }
  d->setBatchProcessing(wasBatchProcessing);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerWidget::confirmClose()
{
  Q_D(qSlicerExtensionsManagerWidget);
  QStringList pendingOperations;
  if (this->extensionsManagerModel())
  {
    pendingOperations.append(this->extensionsManagerModel()->activeTasks());
  }
  if (pendingOperations.empty() && !d->IsBatchProcessing)
  {
    return true;
  }

  ctkMessageBox confirmDialog;
  confirmDialog.setText(
    tr("Install/uninstall/update operations are still in progress:") + "\n- " + pendingOperations.join("\n- ") + "\n\n"
    + tr("Click OK to wait for them to complete, or choose Ignore to close the Extensions Manager now."));
  confirmDialog.setIcon(QMessageBox::Question);
  confirmDialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
  bool closeConfirmed = (confirmDialog.exec() == QMessageBox::Ignore);
  return closeConfirmed;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::setAutoUpdateCheck(bool toggle)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setAutoUpdateCheck(toggle);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::setAutoUpdateInstall(bool toggle)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setAutoUpdateInstall(toggle);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::setAutoInstallDependencies(bool toggle)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (!this->extensionsManagerModel())
  {
    return;
  }
  this->extensionsManagerModel()->setAutoInstallDependencies(toggle);
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerWidget::isInBatchProcessing()
{
  Q_D(qSlicerExtensionsManagerWidget);
  return d->IsBatchProcessing;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (d->tabWidget->currentWidget() != d->ManageExtensionsTab)
  {
    // only display messages when we are in the extensions tab (the web view uses its own style)
    return;
  }

  bool show = false;
  int delayMsec = 2500; // show messages for 2.5 seconds by default
  QString color;
  if (level == ctkErrorLogLevel::Info)
  {
    show = true;
  }
  else if (level == ctkErrorLogLevel::Warning)
  {
    delayMsec = 10000; // show warning messages for 10 seconds
    color = "darkorange";
    show = true;
    if (d->MessageWidget->icon() != QMessageBox::Critical)
    {
      d->MessageWidget->setIcon(QMessageBox::Warning);
    }
  }
  else if (level == ctkErrorLogLevel::Critical || level == ctkErrorLogLevel::Fatal)
  {
    delayMsec = 10000; // show error messages for 10 seconds
    color = "red";
    show = true;
    d->MessageWidget->setIcon(QMessageBox::Critical);
  }
  if (d->MessageWidgetAutoCloseTimer->remainingTime() < delayMsec)
  {
    d->MessageWidgetAutoCloseTimer->start(delayMsec);
  }
  if (color.isEmpty())
  {
    d->Messages << text;
  }
  else
  {
    d->Messages << QString("<font color=\"%1\">%2</font>").arg(color).arg(text);
  }
  const int maxNumberOfDisplayedMessages = 12;
  if (d->Messages.size() > maxNumberOfDisplayedMessages)
  {
    d->Messages.pop_front();
  }
  d->MessageWidget->setText(d->Messages.join("<p>"));
  if (show)
  {
    d->MessageWidget->show();
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onMessagesAcknowledged()
{
  Q_D(qSlicerExtensionsManagerWidget);
  d->Messages.clear();
  d->MessageWidget->setIcon(QMessageBox::NoIcon);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::setFocusToSearchBox()
{
  Q_D(qSlicerExtensionsManagerWidget);
  d->ToolsWidget->SearchBox->setFocus();
}
