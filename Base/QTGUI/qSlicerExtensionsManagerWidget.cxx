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
#include <QTimerEvent>
#include <QToolButton>
#include <QWebFrame>
#include <QWebView>

// CTK includes
#include <ctkSearchBox.h>

// QtGUI includes
#include "qSlicerExtensionsManagerWidget.h"
#include "qSlicerExtensionsManagerModel.h"
#include "ui_qSlicerExtensionsActionsWidget.h"
#include "ui_qSlicerExtensionsManagerWidget.h"

// --------------------------------------------------------------------------
namespace
{

QString jsQuote(QString text)
{
  // NOTE: This assumes that 'text' does not contain '\r' or other control characters
  static QRegExp reSpecialCharacters("([\'\"\\\\])");
  text.replace(reSpecialCharacters, "\\\\1").replace("\n", "\\n");
  return QString("\'%1\'").arg(text);
}

// --------------------------------------------------------------------------
class qSlicerExtensionsActionsWidget : public QStackedWidget, public Ui_qSlicerExtensionsActionsWidget
{
public:
  qSlicerExtensionsActionsWidget(QWidget * parent = 0) : QStackedWidget(parent)
  {
    this->setupUi(this);
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

  ctkSearchBox* searchText;
  QString lastSearchText;
  int searchTimerId;
};

// --------------------------------------------------------------------------
qSlicerExtensionsManagerWidgetPrivate::qSlicerExtensionsManagerWidgetPrivate(qSlicerExtensionsManagerWidget& object)
  :q_ptr(&object), searchTimerId(0)
{
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsManagerWidget);

  this->setupUi(q);

  this->ExtensionsManageBrowser->setBrowsingEnabled(false);

  // Back and forward buttons
  qSlicerExtensionsActionsWidget * actionsWidget = new qSlicerExtensionsActionsWidget;
  actionsWidget->ManageBackButton->setDefaultAction(this->ExtensionsManageBrowser->webView()->pageAction(QWebPage::Back));
  actionsWidget->ManageForwardButton->setDefaultAction(this->ExtensionsManageBrowser->webView()->pageAction(QWebPage::Forward));
  actionsWidget->InstallBackButton->setDefaultAction(this->ExtensionsInstallWidget->webView()->pageAction(QWebPage::Back));
  actionsWidget->InstallForwardButton->setDefaultAction(this->ExtensionsInstallWidget->webView()->pageAction(QWebPage::Forward));

  const QSize iconSize = this->tabWidget->iconSize();
  actionsWidget->ManageBackButton->setIconSize(iconSize);
  actionsWidget->ManageForwardButton->setIconSize(iconSize);
  actionsWidget->InstallBackButton->setIconSize(iconSize);
  actionsWidget->InstallForwardButton->setIconSize(iconSize);

  this->tabWidget->setCornerWidget(actionsWidget, Qt::TopLeftCorner);

  // Search field
  QWidget * searchWidget = new QWidget;
  QHBoxLayout * searchLayout = new QHBoxLayout(searchWidget);
  searchLayout->setContentsMargins(0, 0, 0, 0);

  QToolButton * configureButton = new QToolButton; // needed for vertical alignment
  configureButton->setMinimumSize(actionsWidget->InstallForwardButton->sizeHint());
  configureButton->setAutoRaise(true);
  configureButton->setEnabled(false);
  searchLayout->addWidget(configureButton);

  this->searchText = new ctkSearchBox;
  this->searchText->setClearIcon(QIcon::fromTheme("edit-clear-locationbar-rtl", this->searchText->clearIcon()));
  this->searchText->setSearchIcon(QIcon::fromTheme("edit-find", QPixmap(":/Icons/Search.png")));
  this->searchText->setShowSearchIcon(true);
  searchLayout->addWidget(this->searchText);

  searchWidget->setEnabled(false);

  this->tabWidget->setCornerWidget(searchWidget, Qt::TopRightCorner);

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)),
                   actionsWidget, SLOT(setCurrentIndex(int)));

  QObject::connect(this->ExtensionsManageWidget, SIGNAL(linkActivated(QUrl)),
                   q, SLOT(onManageLinkActivated(QUrl)));

  QObject::connect(this->ExtensionsManageBrowser->webView(),
                   SIGNAL(urlChanged(QUrl)),
                   q, SLOT(onManageUrlChanged(QUrl)));

  QObject::connect(this->searchText, SIGNAL(textEdited(QString)),
                   q, SLOT(onSearchTextChanged(QString)));

  QObject::connect(this->ExtensionsInstallWidget->webView(),
                   SIGNAL(urlChanged(QUrl)),
                   q, SLOT(onInstallUrlChanged(QUrl)));

  QObject::connect(this->tabWidget, SIGNAL(currentChanged(int)),
                   q, SLOT(onCurrentTabChanged(int)));
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
  d->ExtensionsManageBrowser->setExtensionsManagerModel(model);
  d->ExtensionsInstallWidget->setExtensionsManagerModel(model);

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
  d->ExtensionsInstallWidget->refresh();
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
void qSlicerExtensionsManagerWidget::onCurrentTabChanged(int index)
{
  Q_D(qSlicerExtensionsManagerWidget);

  d->tabWidget->cornerWidget(Qt::TopRightCorner)->setEnabled(
        d->tabWidget->widget(index) == d->InstallExtensionsTab);

  // TODO navigate 'manage browser' back to first page
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onManageLinkActivated(const QUrl& link)
{
  Q_D(qSlicerExtensionsManagerWidget);

  d->ManageExtensionsPager->setCurrentIndex(1);
  d->ExtensionsManageBrowser->webView()->load(link);
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
    d->searchText->setEnabled(true);
    d->lastSearchText = newUrl.queryItemValue("search");
    d->searchText->setText(d->lastSearchText);
    }
  else
    {
    d->searchText->setEnabled(false);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onSearchTextChanged(const QString& newText)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (d->searchTimerId)
    {
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
  if (newText != d->lastSearchText)
    {
    d->searchTimerId = this->startTimer(200);
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::timerEvent(QTimerEvent* e)
{
  Q_D(qSlicerExtensionsManagerWidget);
  if (e->timerId() == d->searchTimerId)
    {
    const QString& searchText = d->searchText->text();
    if (searchText != d->lastSearchText)
      {
      d->ExtensionsInstallWidget->webView()->page()->mainFrame()->evaluateJavaScript(
        "midas.slicerappstore.search = " + jsQuote(searchText) + ";"
        "midas.slicerappstore.applyFilter();");
      d->lastSearchText = searchText;
      }
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
  QObject::timerEvent(e);
}
