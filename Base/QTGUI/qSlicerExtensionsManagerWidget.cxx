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
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QTimerEvent>
#include <QToolButton>
#include <QUrlQuery>
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
#include <QWebEngineHistory>
#include <QWebEnginePage>
#include <QWebEngineView>
#endif

// CTK includes
#include <ctkSearchBox.h>
#include <ctkMessageBox.h>

// QtGUI includes
#include "qSlicerExtensionsManagerWidget.h"
#include "qSlicerExtensionsManagerModel.h"
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
#include "qSlicerExtensionsInstallWidget.h"
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
void invalidateSizeHint(QToolButton * button)
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
class qSlicerExtensionsActionsWidget : public QStackedWidget, public Ui_qSlicerExtensionsActionsWidget
{
public:
  qSlicerExtensionsActionsWidget(QWidget * parent = nullptr) : QStackedWidget(parent)
  {
    this->setupUi(this);
  }
};

// --------------------------------------------------------------------------
class qSlicerExtensionsToolsWidget : public QWidget, public Ui_qSlicerExtensionsToolsWidget
{
public:
  qSlicerExtensionsToolsWidget(QWidget * parent = nullptr) : QWidget(parent)
  {
    this->setupUi(this);

    setThemeIcon(this->ConfigureButton, "configure");
    setThemeIcon(this->CheckForUpdatesAction, "view-refresh");

    const QIcon searchIcon =
      QIcon::fromTheme("edit-find", QPixmap(":/Icons/Search.png"));
    const QIcon clearIcon =
      QIcon::fromTheme(this->layoutDirection() == Qt::LeftToRight
                       ? "edit-clear-locationbar-rtl"
                       : "edit-clear-locationbar-ltr",
                       this->SearchBox->clearIcon());

    const QFontMetrics fm = this->SearchBox->fontMetrics();
    const int searchWidth = 24 * fm.averageCharWidth() + 40;

    this->SearchBox->setClearIcon(clearIcon);
    this->SearchBox->setSearchIcon(searchIcon);
    this->SearchBox->setShowSearchIcon(true);
    this->SearchBox->setFixedWidth(searchWidth);

    // manage
    QMenu * configureMenu = new QMenu(this);
    configureMenu->addAction(this->CheckForUpdatesAction);
    configureMenu->addAction(this->AutoUpdateAction);

    this->ConfigureButton->setMenu(configureMenu);
    invalidateSizeHint(this->ConfigureButton);
  }
};

}

//-----------------------------------------------------------------------------
class qSlicerExtensionsManagerWidgetPrivate: public Ui_qSlicerExtensionsManagerWidget
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsManagerWidget);
protected:
  qSlicerExtensionsManagerWidget* const q_ptr;

public:
  qSlicerExtensionsManagerWidgetPrivate(qSlicerExtensionsManagerWidget& object);
  void init();

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  qSlicerExtensionsInstallWidget* ExtensionsManageBrowser;
  qSlicerExtensionsInstallWidget* ExtensionsInstallWidget;
#endif

  qSlicerExtensionsToolsWidget* toolsWidget;
  QString lastInstallWidgetSearchText;
  QUrl lastInstallWidgetUrl;
  int searchTimerId;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerWidgetPrivate::qSlicerExtensionsManagerWidgetPrivate(qSlicerExtensionsManagerWidget& object)
  : q_ptr(&object)
  , toolsWidget(nullptr)
  , searchTimerId(0)
{
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  this->ExtensionsManageBrowser = nullptr;
  this->ExtensionsInstallWidget = nullptr;
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerWidget);

  this->setupUi(q);

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  // Setup browser for "Install Extensions" tab
  this->ExtensionsInstallWidget = new qSlicerExtensionsInstallWidget(this->InstallExtensionsTab);
  this->ExtensionsInstallWidget->setObjectName("ExtensionsInstallWidget");
  this->InstallExtensionsTabLayout->addWidget(this->ExtensionsInstallWidget);

  // Setup browser for "Manage Extensions" tab
  this->ExtensionsManageBrowser = new qSlicerExtensionsInstallWidget();
  this->ExtensionsManageBrowser->setObjectName("ExtensionsManageBrowser");
  this->ManageExtensionsPager->addWidget(this->ExtensionsManageBrowser);
  this->ExtensionsManageBrowser->setBrowsingEnabled(false);
  this->ExtensionsManageBrowser->webView()->load(QUrl("about:"));
#endif

  qSlicerExtensionsActionsWidget * actionsWidget = new qSlicerExtensionsActionsWidget;

  // Back and forward buttons
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  actionsWidget->ManageBackButton->setDefaultAction(this->ExtensionsManageBrowser->webView()->pageAction(QWebEnginePage::Back));
  actionsWidget->ManageForwardButton->setDefaultAction(this->ExtensionsManageBrowser->webView()->pageAction(QWebEnginePage::Forward));
  actionsWidget->InstallBackButton->setDefaultAction(this->ExtensionsInstallWidget->webView()->pageAction(QWebEnginePage::Back));
  actionsWidget->InstallForwardButton->setDefaultAction(this->ExtensionsInstallWidget->webView()->pageAction(QWebEnginePage::Forward));
#endif

  this->tabWidget->setCornerWidget(actionsWidget, Qt::TopLeftCorner);

  // Search field and configure button
  this->toolsWidget = new qSlicerExtensionsToolsWidget;

  QSettings settings;
  this->toolsWidget->AutoUpdateAction->setChecked(
    settings.value("Extensions/AutoUpdate", false).toBool());

  this->tabWidget->setCornerWidget(this->toolsWidget, Qt::TopRightCorner);

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)),
                   actionsWidget, SLOT(setCurrentIndex(int)));

  QObject::connect(this->ExtensionsManageWidget, SIGNAL(linkActivated(QUrl)),
                   q, SLOT(onManageLinkActivated(QUrl)));

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  QObject::connect(this->ExtensionsManageBrowser->webView(),
                   SIGNAL(urlChanged(QUrl)),
                   q, SLOT(onManageUrlChanged(QUrl)));

  QObject::connect(this->toolsWidget->SearchBox,
                   SIGNAL(textEdited(QString)),
                   q, SLOT(onSearchTextChanged(QString)));

  QObject::connect(this->ExtensionsInstallWidget->webView(),
                   SIGNAL(urlChanged(QUrl)),
                   q, SLOT(onInstallUrlChanged(QUrl)));
#endif

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)),
                   q, SLOT(onCurrentTabChanged(int)));

  QObject::connect(this->toolsWidget->CheckForUpdatesAction,
                   SIGNAL(triggered(bool)),
                   q, SLOT(onCheckForUpdatesTriggered()));

  QObject::connect(this->toolsWidget->InstallFromFileButton,
                   SIGNAL(clicked()),
                   q, SLOT(onInstallFromFileTriggered()));
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
qSlicerExtensionsManagerWidget::~qSlicerExtensionsManagerWidget()
{
  Q_D(qSlicerExtensionsManagerWidget);

  QSettings settings;
  settings.setValue("Extensions/AutoUpdate",
                    d->toolsWidget->AutoUpdateAction->isChecked());
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsManagerWidget::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsManagerWidget);
  return d->ExtensionsManageWidget->extensionsManagerModel();
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

  d->ExtensionsManageWidget->setExtensionsManagerModel(model);
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  d->ExtensionsManageBrowser->setExtensionsManagerModel(model);
  d->ExtensionsInstallWidget->setExtensionsManagerModel(model);
#endif
  d->ExtensionsRestoreWidget->setExtensionsManagerModel(model);

  if (model)
    {
    this->onModelUpdated();
    connect(model, SIGNAL(modelUpdated()),
            this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionInstalled(QString)),
            this, SLOT(onModelUpdated()));
    connect(model, SIGNAL(extensionUninstalled(QString)),
            this, SLOT(onModelUpdated()));
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::refreshInstallWidget()
{
  Q_D(qSlicerExtensionsManagerWidget);
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  d->ExtensionsInstallWidget->refresh();
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onModelUpdated()
{
  Q_D(qSlicerExtensionsManagerWidget);

  int manageExtensionsTabIndex = d->tabWidget->indexOf(d->ManageExtensionsTab);
  int installedExtensionsCount = this->extensionsManagerModel()->installedExtensionsCount();

  d->tabWidget->setTabText(manageExtensionsTabIndex,
                           QString("Manage Extensions (%1)").arg(installedExtensionsCount));

  if (installedExtensionsCount == 0)
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

  const bool autoUpdate = d->toolsWidget->AutoUpdateAction->isChecked();
  this->extensionsManagerModel()->checkForUpdates(autoUpdate);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onCurrentTabChanged(int index)
{
  Q_D(qSlicerExtensionsManagerWidget);
  Q_UNUSED(index);
  if (d->tabWidget->currentWidget() == d->ManageExtensionsTab)
    {
    d->toolsWidget->SearchBox->setEnabled(true);
    }
  else if (d->tabWidget->currentWidget() == d->restoreExtensionsTab)
    {
    d->toolsWidget->SearchBox->setEnabled(true);
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
    if (serverAPI == qSlicerExtensionsManagerModel::Midas_v1)
      {
      isCatalogPage = d->lastInstallWidgetUrl.path().endsWith("/slicerappstore");
      }
    else if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
      {
      isCatalogPage = d->lastInstallWidgetUrl.path().contains("/catalog");
      }
    else
      {
      qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
      }
    if (isCatalogPage)
      {
      d->toolsWidget->SearchBox->setEnabled(true);
      // When URL is changed because user clicked on a link then we want the search text
      // to be reset. However, when user is entering text in the searchbox (it has the focus)
      // then we must not overwrite the search text.
      if (!d->toolsWidget->SearchBox->hasFocus())
        {
        QString lastSearchTextLoaded;
        if (serverAPI == qSlicerExtensionsManagerModel::Midas_v1)
          {
          lastSearchTextLoaded = QUrlQuery(d->lastInstallWidgetUrl).queryItemValue("search");
          }
        else if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
          {
          lastSearchTextLoaded = QUrlQuery(d->lastInstallWidgetUrl).queryItemValue("q");
          }
        d->toolsWidget->SearchBox->setText(lastSearchTextLoaded);
        }
      }
    else
      {
      d->toolsWidget->SearchBox->setEnabled(false);
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
  d->ManageExtensionsPager->setCurrentIndex(newUrl.scheme() == "about" ? 0 : 1);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onInstallUrlChanged(const QUrl& newUrl)
{
  Q_D(qSlicerExtensionsManagerWidget);
  // refresh tools widget state (it should be only enabled if browsing the appstore)
  bool isCatalogPage = false;
  int serverAPI = this->extensionsManagerModel()->serverAPI();
  QString lastSearchTextLoaded;
  if (serverAPI == qSlicerExtensionsManagerModel::Midas_v1)
    {
    lastSearchTextLoaded = QUrlQuery(newUrl).queryItemValue("search");
    isCatalogPage = newUrl.path().endsWith("/slicerappstore");
    }
  else if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
    {
    lastSearchTextLoaded = QUrlQuery(newUrl).queryItemValue("q");
    isCatalogPage = newUrl.path().contains("/catalog");
    }
  if (isCatalogPage)
    {
    d->toolsWidget->SearchBox->setEnabled(true);
    // When URL is changed because user clicked on a link then we want the search text
    // to be reset. However, when user is entering text in the searchbox (it has the focus)
    // then we must not overwrite the search text.
    if (!d->toolsWidget->SearchBox->hasFocus())
      {
      d->toolsWidget->SearchBox->setText(lastSearchTextLoaded);
      }
    }
  else
    {
    d->toolsWidget->SearchBox->setEnabled(false);
    }
  d->lastInstallWidgetUrl = newUrl;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onSearchTextChanged(const QString& newText)
{
  Q_UNUSED(newText);
  Q_D(qSlicerExtensionsManagerWidget);
  if (d->searchTimerId)
    {
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
  d->searchTimerId = this->startTimer(500);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::processSearchTextChange()
{
  Q_D(qSlicerExtensionsManagerWidget);
  const QString& searchText = d->toolsWidget->SearchBox->text();
  if (d->tabWidget->currentWidget() == d->ManageExtensionsTab)
    {
    d->ExtensionsManageWidget->setSearchText(searchText);
    }
  else if (d->tabWidget->currentWidget() == d->restoreExtensionsTab)
    {
    d->ExtensionsRestoreWidget->setSearchText(searchText);
    }
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  else if (d->tabWidget->currentWidget() == d->InstallExtensionsTab)
    {
    if (searchText != d->lastInstallWidgetSearchText)
      {
      int serverAPI = this->extensionsManagerModel()->serverAPI();
      if (serverAPI == qSlicerExtensionsManagerModel::Midas_v1)
        {
        d->ExtensionsInstallWidget->webView()->page()->runJavaScript(
          "midas.slicerappstore.search = " + jsQuote(searchText) + ";"
          "midas.slicerappstore.applyFilter();");
        }
      else if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
        {
        d->ExtensionsInstallWidget->webView()->page()->runJavaScript(
          "app.search(" + jsQuote(searchText) + ");");
        }
      else
        {
        qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
        }
      d->lastInstallWidgetSearchText = searchText;
      }
    }
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::timerEvent(QTimerEvent* e)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (e->timerId() == d->searchTimerId)
    {
    this->processSearchTextChange();
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
  QObject::timerEvent(e);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onInstallFromFileTriggered()
{
  const QString& archiveName =
    QFileDialog::getOpenFileName(
      this, "Select extension archive file...", QString(),
      "Archives (*.zip *.7z *.tar *.tar.gz *.tgz *.tar.bz2 *.tar.xz);;"
      "All files (*)");

  if (!archiveName.isEmpty())
    {
    qSlicerExtensionsManagerModel* const model = this->extensionsManagerModel();

    connect(model, SIGNAL(messageLogged(QString,ctkErrorLogLevel::LogLevels)),
            this, SLOT(onMessageLogged(QString,ctkErrorLogLevel::LogLevels)));
    this->extensionsManagerModel()->installExtension(archiveName);

    disconnect(model, SIGNAL(messageLogged(QString,ctkErrorLogLevel::LogLevels)),
               this, SLOT(onMessageLogged(QString,ctkErrorLogLevel::LogLevels)));
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onMessageLogged(
  const QString& text, ctkErrorLogLevel::LogLevels levels)
{
  if (levels >= ctkErrorLogLevel::Error)
    {
    QMessageBox::critical(this, "Install extension", text);
    }
  else if (levels >= ctkErrorLogLevel::Warning)
    {
    QMessageBox::warning(this, "Install extension", text);
    }
  else
    {
    QMessageBox::information(this, "Install extension", text);
    }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsManagerWidget::confirmClose()
{
  Q_D(qSlicerExtensionsManagerWidget);
  QStringList pendingOperations;
  int numberOfPendingRestoreOperations = d->ExtensionsRestoreWidget->pendingOperationsCount();
  if (numberOfPendingRestoreOperations > 0)
    {
    pendingOperations.append(tr("restore %1 extension(s)").arg(numberOfPendingRestoreOperations));
    }
  if (this->extensionsManagerModel())
    {
    pendingOperations.append(this->extensionsManagerModel()->activeTasks());
    }
  if (pendingOperations.empty())
    {
    return true;
    }

  ctkMessageBox confirmDialog;
  confirmDialog.setText(tr("Install/uninstall operations are still in progress:\n- ")
    + pendingOperations.join("\n- ")
    + tr("\n\nClick OK to wait for them to complete, or choose Ignore to close the Extensions Manager now."));
  confirmDialog.setIcon(QMessageBox::Question);
  confirmDialog.setStandardButtons(QMessageBox::Ok | QMessageBox::Ignore);
  bool closeConfirmed = (confirmDialog.exec() == QMessageBox::Ignore);
  return closeConfirmed;
}
