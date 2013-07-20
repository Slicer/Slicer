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
#include <QWebFrame>
#include <QWebView>

// CTK includes
#include <ctkPimpl.h>

// QtGUI includes
#include "qSlicerExtensionsInstallWidget.h"
#include "qSlicerExtensionsManagerModel.h"

//-----------------------------------------------------------------------------
class qSlicerExtensionsInstallWidgetPrivate
{
  Q_DECLARE_PUBLIC(qSlicerExtensionsInstallWidget);
protected:
  qSlicerExtensionsInstallWidget* const q_ptr;

public:
  qSlicerExtensionsInstallWidgetPrivate(qSlicerExtensionsInstallWidget& object);

  /// Return the URL allowing to retrieve the extension list page
  /// associated with the current architecture, operating system and slicer revision.
  QUrl extensionsListUrl();

  void setFailurePage(const QUrl &faultyUrl);

  qSlicerExtensionsManagerModel * ExtensionsManagerModel;

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
void qSlicerExtensionsInstallWidgetPrivate::setFailurePage(const QUrl& faultyUrl)
{
  Q_Q(qSlicerExtensionsInstallWidget);
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

  q->webView()->setHtml(html.arg(faultyUrl.toString()));
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::qSlicerExtensionsInstallWidget(QWidget* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionsInstallWidgetPrivate(*this))
{
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
  this->webView()->setUrl(d->extensionsListUrl());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionInstalled(const QString& extensionName)
{
  this->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'ScheduleUninstall')").arg(extensionName));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionScheduledForUninstall(const QString& extensionName)
{
  this->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'CancelScheduledForUninstall')").arg(extensionName));
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

  this->evalJS(QString("midas.createNotice('%1', %2, '%3')").arg(text).arg(delay).arg(state));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::initJavascript()
{
  Q_D(qSlicerExtensionsInstallWidget);
  this->Superclass::initJavascript();
  this->webView()->page()->mainFrame()->addToJavaScriptWindowObject(
        "extensions_manager_model", d->ExtensionsManagerModel);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLoadFinished(bool ok)
{
  Q_D(qSlicerExtensionsInstallWidget);
  this->Superclass::onLoadFinished(ok);
  if(!ok)
    {
    d->setFailurePage(d->extensionsListUrl().toString());
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLinkClicked(const QUrl& url)
{
  if(url.host() == this->extensionsManagerModel()->serverUrl().host())
    {
    this->Superclass::onLinkClicked(url);
    }
  else
    {
    if(!QDesktopServices::openUrl(url))
      {
      qWarning() << "Failed to open url:" << url;
      }
    }
}
