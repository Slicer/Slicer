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

#ifndef __qSlicerWebWidget_p_h
#define __qSlicerWebWidget_p_h

// Qt includes
#include <QtGlobal>
#include <QTime>
#include <QWebEngineCertificateError>
#include <QWebEnginePage>
class QWebEngineProfile;
class QWebEngineDownloadItem;

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"
#include "qSlicerWebPythonProxy.h"
#include "qSlicerWebWidget.h"
#include "ui_qSlicerWebWidget.h"

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebEnginePage: public QWebEnginePage
{
  friend class qSlicerWebWidget;
  friend class qSlicerWebWidgetPrivate;
public:
  qSlicerWebEnginePage(QWebEngineProfile *profile, QObject *parent = nullptr);
  ~qSlicerWebEnginePage() override;

protected:
  bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame) override
  {
    Q_ASSERT(this->WebWidget);
    return this->WebWidget->acceptNavigationRequest(url, type, isMainFrame);
  }

  bool webEnginePageAcceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame)
  {
    return this->QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
  }

  QWebEnginePage *createWindow(QWebEnginePage::WebWindowType type) override
  {
    Q_UNUSED(type);
    qWarning() << "qSlicerWebEnginePage: createWindow not implemented";
    return nullptr;
  }

  bool certificateError(const QWebEngineCertificateError &certificateError) override
  {
    qDebug() << "[SSL] [" << qPrintable(certificateError.url().host().trimmed()) << "]"
             << qPrintable(certificateError.errorDescription());
    return false;
  }
private:
  qSlicerWebWidget* WebWidget;
};

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebWidgetPrivate: public QObject, Ui_qSlicerWebWidget
{
  Q_OBJECT
  Q_DECLARE_PUBLIC(qSlicerWebWidget);
protected:
  qSlicerWebWidget* const q_ptr;

public:
  qSlicerWebWidgetPrivate(qSlicerWebWidget& object);
  ~qSlicerWebWidgetPrivate() override;

  virtual void init();

  /// \brief Update \c profile injecting a qtwebchannel script.
  ///
  /// A QWebEngineScript named ``qwebchannel_appended.js`` is created by
  /// copying the Qt resource ``qrc:/qtwebchannel/qwebchannel.js``. It is
  /// initialized with the following properties:
  /// * WorldId set to QWebEngineScript::MainWorld.
  /// * InjectionPoint set to QWebEngineScript::DocumentCreation.
  /// * RunsOnSubFrames set to false.
  ///
  /// Prior creating the script the function updateWebChannelScript(QByteArray)
  /// is called to allow further customization by sub-classes by appending additional
  /// script content.
  virtual void initializeWebEngineProfile(QWebEngineProfile* profile);

  /// \brief Append additional script content to ``qwebchannel_appended.js``.
  ///
  /// The default implementation instantiates a ``QWebChannel`` JS object and
  /// call initializeWebChannelTransport() to provide derived classes with an
  /// opportunity to further customize the WebChannelTransport initialization
  /// callback code.
  ///
  /// \sa initializeWebEngineProfile(QWebEngineProfile*)
  virtual void updateWebChannelScript(QByteArray& /* webChannelScript */);

  /// \brief Append additional script content to the WebChannelTransport initialization
  /// callback associated with the default QWebChannel.
  ///
  /// \sa updateWebChannelScript()
  virtual void initializeWebChannelTransport(QByteArray& /* webChannelScript */);

  virtual void initializeWebChannel(QWebChannel* /* webChannel */);

protected slots:
  virtual void handleDownload(QWebEngineDownloadItem *download);

public:
  /// Convenient method to set "document.webkitHidden" property
  void setDocumentWebkitHidden(bool value);

protected slots:
  void onAppAboutToQuit();

public:

  QTime DownloadTime;
  bool HandleExternalUrlWithDesktopService;
  bool NavigationRequestAccepted;
  QStringList InternalHosts;
  qSlicerWebEnginePage* WebEnginePage;
  QWebEngineView* WebView;
  QWebChannel* WebChannel;
  qSlicerWebPythonProxy* PythonProxy;

private:
  Q_DISABLE_COPY(qSlicerWebWidgetPrivate);
};

#endif
