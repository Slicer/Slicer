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

#ifndef __qSlicerExtensionsInstallWidget_h
#define __qSlicerExtensionsInstallWidget_h

// Qt includes
#include <QWidget>

// CTK includes
#include <ctkErrorLogModel.h>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QNetworkReply;
class qSlicerExtensionsInstallWidgetPrivate;
class qSlicerExtensionsManagerModel;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsInstallWidget
  : public QWidget
{
  Q_OBJECT
  Q_PROPERTY(QString slicerRevision READ slicerRevision WRITE setSlicerRevision)
  Q_PROPERTY(QString slicerOs READ slicerOs WRITE setSlicerOs)
  Q_PROPERTY(QString slicerArch READ slicerArch WRITE setSlicerArch)
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsInstallWidget(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsInstallWidget();

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

  QString slicerRevision()const;
  void setSlicerRevision(const QString& revision);

  QString slicerOs()const;
  void setSlicerOs(const QString& os);

  QString slicerArch()const;
  void setSlicerArch(const QString& arch);

public slots:
  /// Refresh the web page associated with the widget
  void refresh();

  void onExtensionInstalled(const QString& extensionName);

  void onExtensionScheduledForUninstall(const QString& extensionName);

  void onExtensionCancelledScheduleForUninstall(const QString& extensionName);

  void onSlicerRequirementsChanged(const QString& revision,const QString& os,const QString& arch);

  void onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level);

  void onDownloadStarted(QNetworkReply* reply);

  void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

  void onDownloadFinished(QNetworkReply* reply);

protected slots:
  void initJavascript();
  void onLoadStarted();
  void onLoadFinished(bool ok);
  void onLinkClicked(const QUrl& url);

protected:
  QScopedPointer<qSlicerExtensionsInstallWidgetPrivate> d_ptr;

  /// Event filter used to capture WebView Show and Hide events in order to both set
  /// "document.webkitHidden" property and trigger the associated event.
  bool eventFilter(QObject *obj, QEvent *event);

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsInstallWidget);
  Q_DISABLE_COPY(qSlicerExtensionsInstallWidget);
};

#endif

