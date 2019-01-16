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

#include "vtkSlicerConfigure.h" // For Slicer_USE_PYTHONQT_WITH_OPENSSL, Slicer_HAVE_WEBKIT_SUPPORT

class QNetworkReply;
class qSlicerWebWidgetPrivate;
class QUrl;
#ifdef Slicer_HAVE_WEBKIT_SUPPORT
class QWebView;
#else
class QWebChannel;
#include <QWebEnginePage>
class QWebEngineView;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(5, 3, 0))
#ifdef QT_NO_OPENSSL
struct QSslError{};
#endif
#else
#ifdef QT_NO_SSL
struct QSslError{};
#endif
#endif

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerWebWidget
  : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(bool handleExternalUrlWithDesktopService READ handleExternalUrlWithDesktopService WRITE setHandleExternalUrlWithDesktopService)
  Q_PROPERTY(QStringList internalHosts READ internalHosts WRITE setInternalHosts)
  friend class qSlicerWebEnginePage;
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerWebWidget(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerWebWidget();

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
#ifdef Slicer_HAVE_WEBKIT_SUPPORT
  Q_INVOKABLE QWebView * webView();
#else
  Q_INVOKABLE QWebEngineView * webView();
#endif

  /// Convenient function to evaluate JS in main frame context
  /// from C++ or Python code
  Q_INVOKABLE QString evalJS(const QString &js);

public slots:

  void onDownloadStarted(QNetworkReply* reply);

  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

  void onDownloadFinished(QNetworkReply* reply);

signals:
  /// emited with result of evalJS
  void evalResult(QString js, QString result);

protected slots:
  virtual void initJavascript();
  virtual void onLoadStarted();
  virtual void onLoadFinished(bool ok);
#ifdef Slicer_HAVE_WEBKIT_SUPPORT
  virtual void onLinkClicked(const QUrl& url);
#endif
  void handleSslErrors(QNetworkReply* reply, const QList<QSslError> &errors);

protected:
  qSlicerWebWidget(qSlicerWebWidgetPrivate* pimpl, QWidget* parent = 0);
  QScopedPointer<qSlicerWebWidgetPrivate> d_ptr;

  /// Event filter used to capture WebView Show and Hide events in order to both set
  /// "document.webkitHidden" property and trigger the associated event.
  bool eventFilter(QObject *obj, QEvent *event);

#ifndef Slicer_HAVE_WEBKIT_SUPPORT
  virtual bool acceptNavigationRequest(const QUrl & url, QWebEnginePage::NavigationType type, bool isMainFrame);
#endif

private:
  Q_DECLARE_PRIVATE(qSlicerWebWidget);
  Q_DISABLE_COPY(qSlicerWebWidget);
};

#endif
