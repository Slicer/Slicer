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
#include <QUrlQuery>
#include <QWebEngineView>

// CTK includes
#include <ctkPimpl.h>

// QtGUI includes
#include "qSlicerExtensionsInstallWidget.h"
#include "qSlicerExtensionsInstallWidget_p.h"
#include "qSlicerExtensionsManagerModel.h"

// --------------------------------------------------------------------------
void ExtensionInstallWidgetWebChannelProxy::refresh()
{
  this->InstallWidget->refresh();
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidgetPrivate::qSlicerExtensionsInstallWidgetPrivate(qSlicerExtensionsInstallWidget& object)
  : qSlicerWebWidgetPrivate(object),
    q_ptr(&object),
    BrowsingEnabled(true)
{
  Q_Q(qSlicerExtensionsInstallWidget);
  this->ExtensionsManagerModel = nullptr;
  this->InstallWidgetForWebChannel = new ExtensionInstallWidgetWebChannelProxy;
  this->InstallWidgetForWebChannel->InstallWidget = q;
  this->HandleExternalUrlWithDesktopService = true;
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidgetPrivate::~qSlicerExtensionsInstallWidgetPrivate()
{
  delete this->InstallWidgetForWebChannel;
}

// --------------------------------------------------------------------------
QUrl qSlicerExtensionsInstallWidgetPrivate::extensionsListUrl()
{
     QUrl url(this->ExtensionsManagerModel->serverUrlWithExtensionsStorePath());
     //HS Uncomment the following line for debugging and comment above
     //QUrl url("http://10.171.2.133:8080/slicerappstore");
     QUrlQuery urlQuery;
     urlQuery.setQueryItems(
        QList<QPair<QString, QString> >()
        << QPair<QString, QString>("layout", "empty")
        << QPair<QString, QString>("os", this->SlicerOs)
        << QPair<QString, QString>("arch", this->SlicerArch)
        << QPair<QString, QString>("revision", this->SlicerRevision));
        //HS Uncomment the following line for debugging and comment above
        //<< QPair<QString, QString>("revision", "19291"));
     url.setQuery(urlQuery);
     return url;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::setFailurePage(const QStringList& errors)
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
      "  div.error{"
      "      position: relative;"
      "      min-width: 13em; max-width: 52em; margin: 4em auto;"
      "      border: 1px solid threedshadow; border-radius: 10px 10px 10px 10px;"
      "      padding: 3em;"
      "      -webkit-padding-start: 30px;"
      "      background: url('qrc:Icons/ExtensionError.svg') no-repeat scroll left 0px content-box border-box;"
      "     }"
      "   #errorTitle, #errorDescription {-webkit-margin-start:80px;}"
      "   #errorTitle h1 {margin:0px 0px 0.6em;}"
      "   #errorDescription ul{"
      "     list-style: square outside none;"
      "     margin: 0px; -webkit-margin-start: 1.5em; padding: 0px;"
      "     }"
      "   #errorDescription ul > li{margin-bottom: 0.5em;}"
      "   #errorTryAgain{margin-top: 2em;}"
      "</style>"
      "<div class='viewWrapperSlicer'>"
      "  <div class='extensionsHeader'>"
      "    <div class='extensionsTitle'>Slicer Extensions</div>"
      "  </div>"
      "  <div class='extensionsBody'>"
      "    <!-- Following layout and associated CSS style are inspired from Mozilla error message. -->"
      "    <!-- It is originally covered by http://mozilla.org/MPL/2.0/ license -->"
      "    <!-- MPL 2.0 license is compatible with Slicer (BSD-like) license -->"
      "    <div class='error'>"
      "      <div id='errorTitle'><h1>Ooops. Extensions can not be installed !</h1></div>"
      "      <div id='errorDescription'>"
      "        <ul>"
      "%1"
      "          <li>Check that <b>3D Slicer</b> is properly installed. "
      "<a href='http://www.slicer.org/slicerWiki/index.php/Documentation/Slicer/Install'>Read more ?</a></li>"
      "        </ul>"
      "        <button id='errorTryAgain' onclick='window.extensions_install_widget.refresh();' autofocus='true'>Try Again</button>"
      "      </div>"
      "    </div>"
      "  </div>"
      "</div>";

  QStringList htmlErrors;
  foreach(const QString& error, errors)
    {
    htmlErrors << QString("<li>%1</li>").arg(error);
    }
  q->webView()->setHtml(html.arg(htmlErrors.join("/n")));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::initializeWebChannelTransport(QByteArray& webChannelScript)
{
  this->Superclass::initializeWebChannelTransport(webChannelScript);
  webChannelScript.append(
      " window.extensions_manager_model = channel.objects.extensions_manager_model;\n"
      // See ExtensionInstallWidgetWebChannelProxy
      " window.extensions_install_widget = channel.objects.extensions_install_widget;\n"
      );
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::initializeWebChannel(QWebChannel* webChannel)
{
  this->Superclass::initializeWebChannel(webChannel);
  webChannel->registerObject(
        "extensions_install_widget", this->InstallWidgetForWebChannel);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidgetPrivate::registerExtensionsManagerModel(
    qSlicerExtensionsManagerModel* oldModel, qSlicerExtensionsManagerModel* newModel)
{
  Q_Q(qSlicerExtensionsInstallWidget);
  QWebChannel* webChannel = q->webView()->page()->webChannel();
  if (oldModel)
    {
    webChannel->deregisterObject(oldModel);
    }
  if (newModel)
    {
    webChannel->registerObject("extensions_manager_model", newModel);
    }
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::qSlicerExtensionsInstallWidget(QWidget* _parent)
  : Superclass(new qSlicerExtensionsInstallWidgetPrivate(*this), _parent)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsInstallWidget::~qSlicerExtensionsInstallWidget() = default;

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

  d->registerExtensionsManagerModel(
        /* oldModel= */ d->ExtensionsManagerModel, /* newModel= */ model);

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
CTK_GET_CPP(qSlicerExtensionsInstallWidget, bool, isBrowsingEnabled, BrowsingEnabled)
CTK_SET_CPP(qSlicerExtensionsInstallWidget, bool, setBrowsingEnabled, BrowsingEnabled)

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::refresh()
{
  Q_D(qSlicerExtensionsInstallWidget);
  if (!d->ExtensionsManagerModel)
    {
    return;
    }
  QStringList errors = this->extensionsManagerModel()->checkInstallPrerequisites();
  if (!errors.empty())
    {
    d->setFailurePage(errors);
    return;
    }
  this->webView()->setUrl(d->extensionsListUrl());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsInstallWidget);
  if(d->BrowsingEnabled)
    {
    this->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'ScheduleUninstall')").arg(extensionName));
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsInstallWidget);
  if(d->BrowsingEnabled)
    {
    this->evalJS(QString("midas.slicerappstore.setExtensionButtonState('%1', 'CancelScheduledForUninstall')").arg(extensionName));
    }
}

// -------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onExtensionCancelledScheduleForUninstall(const QString& extensionName)
{
  this->onExtensionInstalled(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onSlicerRequirementsChanged(const QString& revision,const QString& os,const QString& arch)
{
  Q_D(qSlicerExtensionsInstallWidget);
  this->setSlicerRevision(revision);
  this->setSlicerOs(os);
  this->setSlicerArch(arch);
  if (d->BrowsingEnabled)
    {
    this->refresh();
    }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level)
{
  Q_D(qSlicerExtensionsInstallWidget);
  if(!d->BrowsingEnabled)
    {
    return;
    }
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
void qSlicerExtensionsInstallWidget::onLoadStarted()
{
  this->Superclass::onLoadStarted();
  this->initJavascript();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::initJavascript()
{
  Q_D(qSlicerExtensionsInstallWidget);
  this->Superclass::initJavascript();
  // This is done in qSlicerExtensionsInstallWidgetPrivate::initializeWebChannel()
  // and qSlicerExtensionsInstallWidgetPrivate::registerExtensionsManagerModel()
}

// --------------------------------------------------------------------------
void qSlicerExtensionsInstallWidget::onLoadFinished(bool ok)
{
  Q_D(qSlicerExtensionsInstallWidget);
  this->Superclass::onLoadFinished(ok);
  if(!ok && d->NavigationRequestAccepted)
    {
    d->setFailurePage(QStringList() << QString("Failed to load extension page using this URL: <strong>%1</strong>")
                      .arg(d->extensionsListUrl().toString()));
    }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsInstallWidget::acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame)
{
  Q_D(qSlicerExtensionsInstallWidget);
  d->InternalHosts = QStringList() << this->extensionsManagerModel()->serverUrl().host();
  return Superclass::acceptNavigationRequest(url, type, isMainFrame);
}
