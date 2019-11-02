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

#ifndef __qSlicerWebWidget_h
#define __qSlicerWebWidget_h

// Qt includes
#include <QSslError>
#include <QWidget>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT_WITH_OPENSSL

class QNetworkReply;
class qSlicerWebWidgetPrivate;
class QUrl;
class QWebChannel;
#include <QWebEnginePage>
class QWebEngineView;

#ifdef QT_NO_SSL
struct QSslError{};
#endif

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebWidget
  : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool handleExternalUrlWithDesktopService READ handleExternalUrlWithDesktopService WRITE setHandleExternalUrlWithDesktopService)
  Q_PROPERTY(QStringList internalHosts READ internalHosts WRITE setInternalHosts)
  Q_PROPERTY(QString url READ url WRITE setUrl)
  friend class qSlicerWebEnginePage;
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerWebWidget(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerWebWidget() override;

  /// \brief Return true if external URL should be open with desktop service.
  ///
  /// Setting both this property to true and at least one internal host will
  /// ensure URLs not matching an internal host are open with the current desktop
  /// browser.
  ///
  /// \sa setInternalHost(const QString&)
  /// \sa QDesktopServices::openUrl
  bool handleExternalUrlWithDesktopService() const;
  void setHandleExternalUrlWithDesktopService(bool enable);

  /// \brief Set/Get internal host.
  ///
  /// \sa setHandleExternalUrlWithDesktopService(bool)
  QStringList internalHosts() const;
  void setInternalHosts(const QStringList& hosts);

//  QWebEngineProfile* profile()const;
//  void setProfile(QWebEngineProfile* profile);

  /// Return a reference to the QWebView used internally.
  Q_INVOKABLE QWebEngineView * webView();

  /// Convenient function to evaluate JS in main frame context
  /// from C++ or Python code
  Q_INVOKABLE QString evalJS(const QString &js);

  /// Convenience for setting the internal webView QUrl from a QString
  Q_INVOKABLE QString url();

  /// Convenience for setting the internal webView html from a QString
  Q_INVOKABLE void setHtml(const QString &html, const QUrl &baseUrl = QUrl());

public slots:

  /// Convenience for setting the internal webView QUrl from a QString
  void setUrl(const QString &url);

  void onDownloadStarted(QNetworkReply* reply);

  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

  void onDownloadFinished(QNetworkReply* reply);

signals:
  /// emited with result of evalJS
  void evalResult(QString js, QString result);

  /// signal passed through from QWebEngineView
  void loadStarted();
  void loadProgress(int progress);
  void loadFinished(bool ok);

protected slots:
  virtual void initJavascript();
  virtual void onLoadStarted();
  virtual void onLoadProgress(int progress);
  virtual void onLoadFinished(bool ok);
  void handleSslErrors(QNetworkReply* reply, const QList<QSslError> &errors);

protected:
  qSlicerWebWidget(qSlicerWebWidgetPrivate* pimpl, QWidget* parent = nullptr);
  QScopedPointer<qSlicerWebWidgetPrivate> d_ptr;

  /// Event filter used to capture WebView Show and Hide events in order to both set
  /// "document.webkitHidden" property and trigger the associated event.
  bool eventFilter(QObject *obj, QEvent *event) override;

  virtual bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame);

private:
  Q_DECLARE_PRIVATE(qSlicerWebWidget);
  Q_DISABLE_COPY(qSlicerWebWidget);
};

#endif
