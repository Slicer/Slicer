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

  // Back and forward buttons
  QWidget * actionsWidget = new QWidget;
  QHBoxLayout * actionsLayout = new QHBoxLayout(actionsWidget);
  actionsLayout->setContentsMargins(0, 0, 0, 0);
  QToolButton * backButton = new QToolButton;
  backButton->setDefaultAction(this->ExtensionsInstallWidget->webView()->pageAction(QWebPage::Back));
  actionsLayout->addWidget(backButton);
  QToolButton * forwardButton = new QToolButton;
  forwardButton->setDefaultAction(this->ExtensionsInstallWidget->webView()->pageAction(QWebPage::Forward));
  actionsLayout->addWidget(forwardButton);

  int size = this->tabWidget->height();
  backButton->setIconSize(QSize(size, size));
  forwardButton->setIconSize(QSize(size, size));

  actionsWidget->setEnabled(false);

  this->tabWidget->setCornerWidget(actionsWidget, Qt::TopLeftCorner);

  // Search field
  QWidget * searchWidget = new QWidget;
  QHBoxLayout * searchLayout = new QHBoxLayout(searchWidget);
  searchLayout->setContentsMargins(0, 0, 0, 0);

  QToolButton * configureButton = new QToolButton; // needed for vertical alignment
  configureButton->setMinimumSize(forwardButton->sizeHint());
  configureButton->setAutoRaise(true);
  configureButton->setEnabled(false);
  searchLayout->addWidget(configureButton);

  this->searchText = new ctkSearchBox;
  this->searchText->setClearIcon(QIcon::fromTheme("edit-clear-locationbar-rtl", this->searchText->clearIcon()));
  this->searchText->setSearchIcon(QIcon::fromTheme("edit-find", QPixmap(":/Icons/Search.png")));
  this->searchText->setShowSearchIcon(true);
  searchLayout->addWidget(this->searchText);

  this->tabWidget->setCornerWidget(searchWidget, Qt::TopRightCorner);

  QObject::connect(this->searchText, SIGNAL(textEdited(QString)),
                   q, SLOT(onSearchTextChanged(QString)));

  QObject::connect(this->ExtensionsInstallWidget->webView(),
                   SIGNAL(urlChanged(QUrl)),
                   q, SLOT(onUrlChanged(QUrl)));

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
  d->tabWidget->cornerWidget(Qt::TopLeftCorner)->setEnabled(
        d->tabWidget->widget(index) == d->InstallExtensionsTab);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsManagerWidget::onUrlChanged(const QUrl& newUrl)
{
  Q_D(qSlicerExtensionsManagerWidget);
  d->lastSearchText = newUrl.queryItemValue("search");
  d->searchText->setText(d->lastSearchText);
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
      }
    this->killTimer(d->searchTimerId);
    d->searchTimerId = 0;
    }
  QObject::timerEvent(e);
}
