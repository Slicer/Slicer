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
#include <QStyle>
#include <QUrlQuery>
#include <QWebEngineView>

// CTK includes
#include <ctkPimpl.h>

// QtGUI includes
#include "qSlicerExtensionsServerWidget.h"
#include "qSlicerExtensionsServerWidget_p.h"
#include "qSlicerExtensionsManagerModel.h"

// --------------------------------------------------------------------------
void ExtensionInstallWidgetWebChannelProxy::refresh()
{
  this->InstallWidget->refresh();
}

// --------------------------------------------------------------------------
qSlicerExtensionsServerWidgetPrivate::qSlicerExtensionsServerWidgetPrivate(qSlicerExtensionsServerWidget& object)
  : qSlicerWebWidgetPrivate(object)
  , q_ptr(&object)
  , BrowsingEnabled(true)
{
  Q_Q(qSlicerExtensionsServerWidget);
  this->ExtensionsManagerModel = nullptr;
  this->InstallWidgetForWebChannel = new ExtensionInstallWidgetWebChannelProxy;
  this->InstallWidgetForWebChannel->InstallWidget = q;
  this->HandleExternalUrlWithDesktopService = true;
}

// --------------------------------------------------------------------------
qSlicerExtensionsServerWidgetPrivate::~qSlicerExtensionsServerWidgetPrivate()
{
  delete this->InstallWidgetForWebChannel;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidgetPrivate::setFailurePage(const QStringList& errors)
{
  Q_Q(qSlicerExtensionsServerWidget);
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
    "    <div class='extensionsTitle'>"
    + qSlicerExtensionsServerWidget::tr("Slicer Extensions")
    + "</div>"
      "  </div>"
      "  <div class='extensionsBody'>"
      "    <!-- Following layout and associated CSS style are inspired from Mozilla error message. -->"
      "    <!-- It is originally covered by https://mozilla.org/MPL/2.0/ license -->"
      "    <!-- MPL 2.0 license is compatible with Slicer (BSD-like) license -->"
      "    <div class='error'>"
      "      <div id='errorTitle'><h1>"
    + qSlicerExtensionsServerWidget::tr("Extensions can not be installed.")
    + "</h1></div>"
      "      <div id='errorDescription'>"
      "        <ul>"
      "%1"
#ifdef Q_OS_MAC
      "          <li><b>"
    + qSlicerExtensionsServerWidget::tr(
      "Extensions manager requires <em>3D Slicer</em> to be installed. "
      "Open the disk image (.dmg) file, drag <em>Slicer.app</em> into the the <em>Applications</em> folder, "
      "and launch <em>3D Slicer</em> from the <em>Applications</em> folder.")
    + "</b> "
#else
      "          <li>"
    + qSlicerExtensionsServerWidget::tr("Check that <b>3D Slicer</b> is properly installed.")
    + " "
#endif
      "<a href='https://slicer.readthedocs.io/en/latest/user_guide/getting_started.html#installing-3d-slicer'>"
    + qSlicerExtensionsServerWidget::tr("Read more...")
    + "</a></li>"
      "        </ul>"
      "        <button id='errorTryAgain' onclick='window.extensions_install_widget.refresh();' autofocus='true'>"
    + qSlicerExtensionsServerWidget::tr("Try Again")
    + "</button>"
      "      </div>"
      "    </div>"
      "  </div>"
      "</div>";

  QStringList htmlErrors;
  foreach (const QString& error, errors)
  {
    htmlErrors << QString("<li>%1</li>").arg(error);
  }
  q->webView()->setHtml(html.arg(htmlErrors.join("/n")));
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidgetPrivate::updateTheme()
{
  Q_Q(qSlicerExtensionsServerWidget);
  this->setDarkThemeEnabled(q->style()->objectName().compare("Dark Slicer", Qt::CaseInsensitive) == 0);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidgetPrivate::setDarkThemeEnabled(bool enabled)
{
  Q_Q(qSlicerExtensionsServerWidget);
  if (!this->BrowsingEnabled)
  {
    return;
  }
  int serverAPI = this->ExtensionsManagerModel->serverAPI();
  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    q->evalJS(QString("app.$vuetify.theme.dark = %1;").arg(enabled ? "true" : "false"));
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidgetPrivate::initializeWebChannelTransport(QByteArray& webChannelScript)
{
  this->Superclass::initializeWebChannelTransport(webChannelScript);
  webChannelScript.append(" window.extensions_manager_model = channel.objects.extensions_manager_model;\n"
                          // See ExtensionInstallWidgetWebChannelProxy
                          " window.extensions_install_widget = channel.objects.extensions_install_widget;\n");
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidgetPrivate::initializeWebChannel(QWebChannel* webChannel)
{
  this->Superclass::initializeWebChannel(webChannel);
  webChannel->registerObject("extensions_install_widget", this->InstallWidgetForWebChannel);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidgetPrivate::registerExtensionsManagerModel(qSlicerExtensionsManagerModel* oldModel,
                                                                          qSlicerExtensionsManagerModel* newModel)
{
  Q_Q(qSlicerExtensionsServerWidget);
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
qSlicerExtensionsServerWidget::qSlicerExtensionsServerWidget(QWidget* _parent)
  : Superclass(new qSlicerExtensionsServerWidgetPrivate(*this), _parent)
{
  Q_D(qSlicerExtensionsServerWidget);
  d->init();
}

// --------------------------------------------------------------------------
qSlicerExtensionsServerWidget::~qSlicerExtensionsServerWidget() = default;

// --------------------------------------------------------------------------
qSlicerExtensionsManagerModel* qSlicerExtensionsServerWidget::extensionsManagerModel() const
{
  Q_D(const qSlicerExtensionsServerWidget);
  return d->ExtensionsManagerModel;
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::setExtensionsManagerModel(qSlicerExtensionsManagerModel* model)
{
  Q_D(qSlicerExtensionsServerWidget);
  if (this->extensionsManagerModel() == model)
  {
    return;
  }

  disconnect(this, SLOT(onExtensionInstalled(QString)));
  // disconnect(this, SLOT(onExtensionUninstalled(QString)));
  disconnect(this, SLOT(onExtensionScheduledForUninstall(QString)));
  disconnect(this, SLOT(onSlicerRequirementsChanged()));
  disconnect(this, SLOT(onMessageLogged(QString, ctkErrorLogLevel::LogLevels)));
  disconnect(this, SLOT(onDownloadStarted(QNetworkReply*)));
  disconnect(this, SLOT(onDownloadFinished(QNetworkReply*)));

  d->registerExtensionsManagerModel(
    /* oldModel= */ d->ExtensionsManagerModel, /* newModel= */ model);

  d->ExtensionsManagerModel = model;

  if (model)
  {
    this->onSlicerRequirementsChanged();

    QObject::connect(model, SIGNAL(extensionInstalled(QString)), this, SLOT(onExtensionInstalled(QString)));

    QObject::connect(
      model, SIGNAL(extensionScheduledForUninstall(QString)), this, SLOT(onExtensionScheduledForUninstall(QString)));

    QObject::connect(model,
                     SIGNAL(extensionCancelledScheduleForUninstall(QString)),
                     this,
                     SLOT(onExtensionCancelledScheduleForUninstall(QString)));

    QObject::connect(
      model, SIGNAL(slicerRequirementsChanged(QString, QString, QString)), this, SLOT(onSlicerRequirementsChanged()));

    QObject::connect(model,
                     SIGNAL(messageLogged(QString, ctkErrorLogLevel::LogLevels)),
                     this,
                     SLOT(onMessageLogged(QString, ctkErrorLogLevel::LogLevels)));

    QObject::connect(model, SIGNAL(downloadStarted(QNetworkReply*)), this, SLOT(onDownloadStarted(QNetworkReply*)));

    QObject::connect(model, SIGNAL(downloadFinished(QNetworkReply*)), this, SLOT(onDownloadFinished(QNetworkReply*)));
  }
}

// --------------------------------------------------------------------------
CTK_GET_CPP(qSlicerExtensionsServerWidget, bool, isBrowsingEnabled, BrowsingEnabled)
CTK_SET_CPP(qSlicerExtensionsServerWidget, bool, setBrowsingEnabled, BrowsingEnabled)

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::refresh()
{
  Q_D(qSlicerExtensionsServerWidget);
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
  this->webView()->setUrl(this->extensionsManagerModel()->extensionsListUrl());
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onExtensionInstalled(const QString& extensionName)
{
  Q_D(qSlicerExtensionsServerWidget);
  if (d->BrowsingEnabled)
  {
    int serverAPI = d->ExtensionsManagerModel->serverAPI();
    if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
    {
      this->evalJS(QString("app.setExtensionButtonState('%1', 'Installed');").arg(extensionName));
    }
    else
    {
      qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
    }
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onExtensionScheduledForUninstall(const QString& extensionName)
{
  Q_D(qSlicerExtensionsServerWidget);
  if (d->BrowsingEnabled)
  {
    int serverAPI = d->ExtensionsManagerModel->serverAPI();
    if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
    {
      this->evalJS(QString("app.setExtensionButtonState('%1', 'ScheduledForUninstall');").arg(extensionName));
    }
    else
    {
      qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
    }
  }
}

// -------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onExtensionCancelledScheduleForUninstall(const QString& extensionName)
{
  this->onExtensionInstalled(extensionName);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onSlicerRequirementsChanged()
{
  Q_D(qSlicerExtensionsServerWidget);
  if (d->BrowsingEnabled)
  {
    this->refresh();
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level)
{
  Q_D(qSlicerExtensionsServerWidget);
  if (!d->BrowsingEnabled)
  {
    return;
  }
  QString escapedText = QString(text).replace("'", "\\'");
  QString delay = "2500";
  QString state;
  if (level == ctkErrorLogLevel::Warning)
  {
    delay = "10000";
    state = "warning";
  }
  else if (level == ctkErrorLogLevel::Critical || level == ctkErrorLogLevel::Fatal)
  {
    delay = "10000";
    state = "error";
  }
  int serverAPI = d->ExtensionsManagerModel->serverAPI();
  if (serverAPI == qSlicerExtensionsManagerModel::Girder_v1)
  {
    this->evalJS(QString("app.createNotice('%1', %2, '%3')").arg(escapedText).arg(delay).arg(state));
  }
  else
  {
    qWarning() << Q_FUNC_INFO << " failed: missing implementation for serverAPI" << serverAPI;
  }
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onLoadStarted()
{
  this->Superclass::onLoadStarted();
  this->initJavascript();
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::initJavascript()
{
  Q_D(qSlicerExtensionsServerWidget);
  this->Superclass::initJavascript();
  // This is done in qSlicerExtensionsServerWidgetPrivate::initializeWebChannel()
  // and qSlicerExtensionsServerWidgetPrivate::registerExtensionsManagerModel()
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::onLoadFinished(bool ok)
{
  Q_D(qSlicerExtensionsServerWidget);
  this->Superclass::onLoadFinished(ok);
  if (!ok && d->NavigationRequestAccepted)
  {
    d->setFailurePage(QStringList() << QString("Failed to load extension page using this URL: <strong>%1</strong>")
                                         .arg(this->extensionsManagerModel()->extensionsListUrl().toString()));
  }
  if (ok)
  {
    d->updateTheme();
  }
}

// --------------------------------------------------------------------------
bool qSlicerExtensionsServerWidget::acceptNavigationRequest(const QUrl& url,
                                                            QWebEnginePage::NavigationType type,
                                                            bool isMainFrame)
{
  Q_D(qSlicerExtensionsServerWidget);
  d->InternalHosts = QStringList() << this->extensionsManagerModel()->frontendServerUrl().host();
  return Superclass::acceptNavigationRequest(url, type, isMainFrame);
}

// --------------------------------------------------------------------------
void qSlicerExtensionsServerWidget::changeEvent(QEvent* e)
{
  Q_D(qSlicerExtensionsServerWidget);
  switch (e->type())
  {
    case QEvent::StyleChange:
      d->updateTheme();
      break;
    default:
      break;
  }
  this->Superclass::changeEvent(e);
}
