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
class qSlicerExtensionsToolsWidget : public QStackedWidget, public Ui_qSlicerExtensionsToolsWidget
{
public:
  qSlicerExtensionsToolsWidget(QWidget * parent = nullptr) : QStackedWidget(parent)
  {
    this->setupUi(this);

    setThemeIcon(this->ManageConfigureButton, "configure");
    setThemeIcon(this->InstallConfigureButton, "configure");
    setThemeIcon(this->RestoreConfigureButton, "configure");
    setThemeIcon(this->CheckForUpdatesAction, "view-refresh");

    const QIcon searchIcon =
      QIcon::fromTheme("edit-find", QPixmap(":/Icons/Search.png"));
    const QIcon clearIcon =
      QIcon::fromTheme(this->layoutDirection() == Qt::LeftToRight
                       ? "edit-clear-locationbar-rtl"
                       : "edit-clear-locationbar-ltr",
                       this->ManageSearchBox->clearIcon());

    const QFontMetrics fm = this->ManageSearchBox->fontMetrics();
    const int searchWidth = 24 * fm.averageCharWidth() + 40;

    this->ManageSearchBox->setClearIcon(clearIcon);
    this->ManageSearchBox->setSearchIcon(searchIcon);
    this->ManageSearchBox->setShowSearchIcon(true);
    this->ManageSearchBox->setFixedWidth(searchWidth);

    this->InstallSearchBox->setClearIcon(clearIcon);
    this->InstallSearchBox->setSearchIcon(searchIcon);
    this->InstallSearchBox->setShowSearchIcon(true);
    this->InstallSearchBox->setFixedWidth(searchWidth);

    this->RestoreSearchBox->setClearIcon(clearIcon);
    this->RestoreSearchBox->setSearchIcon(searchIcon);
    this->RestoreSearchBox->setShowSearchIcon(true);
    this->RestoreSearchBox->setFixedWidth(searchWidth);

    // manage
    QMenu * manageConfigureMenu = new QMenu(this);
    manageConfigureMenu->addAction(this->CheckForUpdatesAction);
    manageConfigureMenu->addAction(this->AutoUpdateAction);
    manageConfigureMenu->addSeparator();
    manageConfigureMenu->addAction(this->InstallFromFileAction);

    this->ManageConfigureButton->setMenu(manageConfigureMenu);
    invalidateSizeHint(this->ManageConfigureButton);

    // install
    QMenu * installConfigureMenu = new QMenu(this);
    installConfigureMenu->addAction(this->InstallFromFileAction);

    this->InstallConfigureButton->setMenu(installConfigureMenu);
    invalidateSizeHint(this->InstallConfigureButton);

    // restore
    this->RestoreConfigureMenu = new QMenu(this);

    this->RestoreConfigureButton->setMenu(this->RestoreConfigureMenu);
    invalidateSizeHint(this->RestoreConfigureButton);
  }

  QMenu* RestoreConfigureMenu;
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
  QString lastSearchText;
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
  this->toolsWidget->RestoreConfigureMenu->addAction(this->ExtensionsRestoreWidget->selectAllAction());
  this->toolsWidget->RestoreConfigureMenu->addAction(this->ExtensionsRestoreWidget->deselectAllAction());
  this->toolsWidget->RestoreConfigureMenu->addSeparator();
  this->toolsWidget->RestoreConfigureMenu->addAction(this->ExtensionsRestoreWidget->installSelectedAction());

  QSettings settings;
  this->toolsWidget->AutoUpdateAction->setChecked(
    settings.value("Extensions/AutoUpdate", false).toBool());

  this->tabWidget->setCornerWidget(this->toolsWidget, Qt::TopRightCorner);

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)),
                   actionsWidget, SLOT(setCurrentIndex(int)));
  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)),
                   this->toolsWidget, SLOT(setCurrentIndex(int)));

  QObject::connect(this->ExtensionsManageWidget, SIGNAL(linkActivated(QUrl)),
                   q, SLOT(onManageLinkActivated(QUrl)));

#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  QObject::connect(this->ExtensionsManageBrowser->webView(),
                   SIGNAL(urlChanged(QUrl)),
                   q, SLOT(onManageUrlChanged(QUrl)));

  QObject::connect(this->toolsWidget->InstallSearchBox,
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

  QObject::connect(this->toolsWidget->InstallFromFileAction,
                   SIGNAL(triggered(bool)),
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
  int numberOfInstalledExtensions = this->extensionsManagerModel()->numberOfInstalledExtensions();

  d->tabWidget->setTabText(manageExtensionsTabIndex,
                           QString("Manage Extensions (%1)").arg(numberOfInstalledExtensions));

  if (numberOfInstalledExtensions == 0)
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
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  QWebEngineHistory* history = d->ExtensionsManageBrowser->webView()->history();
  if (history->canGoBack())
    {
    history->goToItem(history->items().first());
    }
#endif
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
  if (newUrl.path().endsWith("/slicerappstore"))
    {
    d->toolsWidget->InstallSearchBox->setEnabled(true);
    // When URL is changed because user clicked on a link then we want the search text
    // to be reset. However, when user is entering text in the searchbox (it has the focus)
    // then we must not overwrite the search text.
    if (!d->toolsWidget->InstallSearchBox->hasFocus())
      {
      QString lastSearchTextLoaded = QUrlQuery(newUrl).queryItemValue("search");
      d->toolsWidget->InstallSearchBox->setText(lastSearchTextLoaded);
      }
    }
  else
    {
    d->toolsWidget->InstallSearchBox->setEnabled(false);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onSearchTextChanged(const QString& newText)
{
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  Q_D(qSlicerExtensionsManagerWidget);
  if (d->searchTimerId)
    {
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
  if (newText != d->lastSearchText)
    {
    d->searchTimerId = this->startTimer(500);
    }
#else
  Q_UNUSED(newText);
#endif
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::timerEvent(QTimerEvent* e)
{
#ifdef Slicer_BUILD_WEBENGINE_SUPPORT
  Q_D(qSlicerExtensionsManagerWidget);
  if (e->timerId() == d->searchTimerId)
    {
    const QString& searchText = d->toolsWidget->InstallSearchBox->text();
    if (searchText != d->lastSearchText)
      {
      d->ExtensionsInstallWidget->webView()->page()->runJavaScript(
        "midas.slicerappstore.search = " + jsQuote(searchText) + ";"
        "midas.slicerappstore.applyFilter();");
      d->lastSearchText = searchText;
      }
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
#endif
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
