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
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QSettings>
#include <QTime>
#include <QWebFrame>

// CTK includes
#include <ctkPimpl.h>

// QtCore includes
#include <qSlicerPersistentCookieJar.h>

// QtGUI includes
#include "qSlicerExtensionsInstallWidget.h"
#include "qSlicerExtensionsManagerModel.h"
#include "ui_qSlicerExtensionsInstallWidget.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsInstallWidgetPrivate: public Ui_qSlicerExtensionsInstallWidget
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsInstallWidget);
protected:
  qSlicerExtensionsInstallWidget* const q_ptr;

public:
  qSlicerExtensionsInstallWidgetPrivate(qSlicerExtensionsInstallWidget& object);

  void init();

  /// Return the URL allowing to retrieve the extension list page
  /// associated with the current architecture, operating system and slicer revision.
  QUrl extensionsListUrl();

  void setFailurePage(const QUrl &faultyUrl);

  /// Convenient function to return the mainframe
  QWebFrame* mainFrame();

  /// Convenient function to evaluate JS in main frame context
  QString evalJS(const QString &js);

  /// Convenient method to set "document.webkitHidden" property
  void setDocumentWebkitHidden(bool value);

  qSlicerExtensionsManagerModel * ExtensionsManagerModel;

  QTime DownloadTime;

  QString SlicerRevision;
  QString SlicerOs;
  QString SlicerArch;
};

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidgetPrivate::qSlicerExtensionsInstallWidgetPrivate(qSlicerExtensionsInstallWidget& object)
  :q_ptr(&object)
{
  this->ExtensionsManagerModel = 0;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::init()
{
  Q_Q(qSlicerExtensionsInstallWidget);

  this->setupUi(q);
  this->WebView->installEventFilter(q);

  QNetworkAccessManager * networkAccessManager = this->WebView->page()->networkAccessManager();;
  Q_ASSERT(networkAccessManager);
  networkAccessManager->setCookieJar(new qSlicerPersistentCookieJar());

  QObject::connect(this->WebView, SIGNAL(loadStarted()),
                   q, SLOT(onLoadStarted()));

  QObject::connect(this->WebView, SIGNAL(loadFinished(bool)),
                   q, SLOT(onLoadFinished(bool)));

  QObject::connect(this->WebView, SIGNAL(loadProgress(int)),
                   this->ProgressBar, SLOT(setValue(int)));

  QObject::connect(this->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()),
                   q, SLOT(initJavascript()));

  this->WebView->settings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);

  this->ProgressBar->setVisible(false);

  this->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOn);

  this->WebView->page()->setLinkDelegationPolicy(QWebPage::DelegateExternalLinks);

  QObject::connect(this->WebView->page(), SIGNAL(linkClicked(QUrl)),
                   q, SLOT(onLinkClicked(QUrl)));
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsInstallWidgetPrivate::extensionsListUrl()
{
  QUrl url(this->ExtensionsManagerModel->serverUrlWithExtensionsStorePath());
  url.setQueryItems(QList<QPair<QString, QString> >()
                    << QPair<QString, QString>("layout", "empty")
                    << QPair<QString, QString>("os", this->SlicerOs)
                    << QPair<QString, QString>("arch", this->SlicerArch)
                    << QPair<QString, QString>("revision", this->SlicerRevision));
  return url;
}

// --------------------------------------------------------------------------
QWebFrame* qSlicerExtensionsInstallWidgetPrivate::mainFrame()
{
  return this->WebView->page()->mainFrame();
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionsInstallWidgetPrivate::evalJS(const QString &js)
{
  return this->mainFrame()->evaluateJavaScript(js).toString();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::setFailurePage(const QUrl& faultyUrl)
{
  QString html =
      "<style type='text/css'>"
      "  div.viewWrapperSlicer{"
      "    font-family:'Lucida Grande','Lucida Sans Unicode',helvetica,arial,Verdana,sans-serif;"
      "    font-size:13px;margin-left:8px;color:#777777;"
      "  }"
      "  div.extensionsHeader,div.extensionsBody{margin-right:10px;}"
      "  div.extensionsHeader{height:45px;border-bottom:1px solid #d0d0d0;}"
      "  div.extensionsTitle{float:left;font-size:24px;font-weight:bolder;margin-top:10px;}"
      "  div.extensionsBodyLeftColumn{float:left;width:230px;border-right:1px solid #d0d0d0;min-height:450px;}"
      "  div.extensionsBodyRightColumn{margin-left:230px;}"
      "</style>"
      "<div class='viewWrapperSlicer'>"
      "  <div class='extensionsHeader'>"
      "    <div class='extensionsTitle'>Slicer Extensions</div>"
      "  </div>"
      "  <div class='extensionsBody'>"
      "    <p>Failed to load extension page using the following URL:<br>%1</p>"
      "  </div>"
      "</div>";

  this->WebView->setHtml(html.arg(faultyUrl.toString()));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::setDocumentWebkitHidden(bool value)
{
  this->evalJS(QString("document.webkitHidden = %1").arg(value ? "true" : "false"));
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::qSlicerExtensionsInstallWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsInstallWidgetPrivate(*this))
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::~qSlicerExtensionsInstallWidget()
{
}

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsInstallWidget::extensionsManagerModel()const
{
  Q_D(const qSlicerExtensionsInstallWidget);
  return d->ExtensionsManagerModel;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsInstallWidget);

  if (this->extensionsManagerModel() == model)
    {
    return;
    }

  disconnect(this, SLOT(onExtensionInstalled(QString)));
  //disconnect(this, SLOT(onExtensionUninstalled(QString)));
  disconnect(this, SLOT(onExtensionScheduledForUninstall(QString)));
  disconnect(this, SLOT(onSlicerRequirementsChanged(QString,QString,QString)));
  disconnect(this, SLOT(onMessageLogged(QString,ctkErrorLogLevel::LogLevels)));
  disconnect(this, SLOT(onDownloadStarted(QNetworkReply*)));
  disconnect(this, SLOT(onDownloadFinished(QNetworkReply*)));

  d->ExtensionsManagerModel = model;

  if (model)
    {
    this->onSlicerRequirementsChanged(
          model->slicerRevision(), model->slicerOs(), model->slicerArch());

    QObject::connect(model, SIGNAL(extensionInstalled(QString)),
                     this, SLOT(onExtensionInstalled(QString)));

    QObject::connect(model, SIGNAL(extensionScheduledForUninstall(QString)),
                     this, SLOT(onExtensionScheduledForUninstall(QString)));

    QObject::connect(model, SIGNAL(extensionCancelledScheduleForUninstall(QString)),
                     this, SLOT(onExtensionCancelledScheduleForUninstall(QString)));

    QObject::connect(model, SIGNAL(slicerRequirementsChanged(QString,QString,QString)),
                     this, SLOT(onSlicerRequirementsChanged(QString,QString,QString)));

    QObject::connect(model, SIGNAL(messageLogged(QString,ctkErrorLogLevel::LogLevels)),
                     this, SLOT(onMessageLogged(QString,ctkErrorLogLevel::LogLevels)));

    QObject::connect(model, SIGNAL(downloadStarted(QNetworkReply*)),
                     this, SLOT(onDownloadStarted(QNetworkReply*)));

    QObject::connect(model, SIGNAL(downloadFinished(QNetworkReply*)),
                     this, SLOT(onDownloadFinished(QNetworkReply*)));
    }
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsInstallWidget, QString, slicerRevision, SlicerRevision)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, const QString&, setSlicerRevision, SlicerRevision)

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsInstallWidget, QString, slicerOs, SlicerOs)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, const QString&, setSlicerOs, SlicerOs)

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsInstallWidget, QString, slicerArch, SlicerArch)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, const QString&, setSlicerArch, SlicerArch)

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::refresh()
{
  Q_D(qSlicerExtensionsInstallWidget);
  if (!d->ExtensionsManagerModel)
    {
    return;
    }
  d->WebView->setUrl(d->extensionsListUrl());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'ScheduleUninstall')").arg(extensionName));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'CancelScheduledForUninstall')").arg(extensionName));
}

// -------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionCancelledScheduleForUninstall(const QString& extensionName)
{
  this->onExtensionInstalled(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onSlicerRequirementsChanged(const QString& revision,const QString& os,const QString& arch)
{
  this->setSlicerRevision(revision);
  this->setSlicerOs(os);
  this->setSlicerArch(arch);
  this->refresh();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level)
{
  Q_D(qSlicerExtensionsInstallWidget);

  QString delay = "2500";
  QString state;
  if (level == ctkErrorLogLevel::Warning)
    {
    delay = "10000";
    state = "warning";
    }
  else if(level == ctkErrorLogLevel::Critical || level == ctkErrorLogLevel::Fatal)
    {
    delay = "10000";
    state = "error";
    }

  d->evalJS(QString("midas.createNotice('%1', %2, '%3')").arg(text).arg(delay).arg(state));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onDownloadStarted(QNetworkReply* reply)
{
  Q_D(qSlicerExtensionsInstallWidget);
  connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
          SLOT(onDownloadProgress(qint64,qint64)));
  d->DownloadTime.start();
  d->ProgressBar->setVisible(true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
  Q_D(qSlicerExtensionsInstallWidget);

  // Calculate the download speed
  double speed = bytesReceived * 1000.0 / d->DownloadTime.elapsed();
  QString unit;
  if (speed < 1024)
    {
    unit = "bytes/sec";
    }
  else if (speed < 1024*1024) {
    speed /= 1024;
    unit = "kB/s";
    }
  else
    {
    speed /= 1024*1024;
    unit = "MB/s";
    }

  d->ProgressBar->setFormat(QString("%p% (%1 %2)").arg(speed, 3, 'f', 1).arg(unit));
  d->ProgressBar->setMaximum(bytesTotal);
  d->ProgressBar->setValue(bytesReceived);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onDownloadFinished(QNetworkReply* reply)
{
  Q_D(qSlicerExtensionsInstallWidget);
  Q_UNUSED(reply);
  d->ProgressBar->reset();
  d->ProgressBar->setVisible(false);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::initJavascript()
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->setDocumentWebkitHidden(!d->WebView->isVisible());
  d->mainFrame()->addToJavaScriptWindowObject("extensions_manager_model", d->ExtensionsManagerModel);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLoadStarted()
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->ProgressBar->setFormat("%p%");
  d->ProgressBar->setVisible(true);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLoadFinished(bool ok)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->ProgressBar->reset();
  d->ProgressBar->setVisible(false);
  if(!ok)
    {
    d->setFailurePage(d->extensionsListUrl().toString());
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLinkClicked(const QUrl& url)
{
  Q_D(qSlicerExtensionsInstallWidget);
  if(url.host() == this->extensionsManagerModel()->serverUrl().host())
    {
    d->WebView->load(url);
    }
  else
    {
    if(!QDesktopServices::openUrl(url))
      {
      qWarning() << "Failed to open url:" << url;
      }
    }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsInstallWidget::eventFilter(QObject* obj, QEvent* event)
{
  Q_D(qSlicerExtensionsInstallWidget);
  Q_ASSERT(d->WebView == obj);
  if (d->WebView == obj && !event->spontaneous() &&
      (event->type() == QEvent::Show || event->type() == QEvent::Hide))
    {
    d->setDocumentWebkitHidden(!d->WebView->isVisible());
    d->evalJS("$.event.trigger({type: 'webkitvisibilitychange'})"); // Assume jquery is available
    }
  return QObject::eventFilter(obj, event);
}
