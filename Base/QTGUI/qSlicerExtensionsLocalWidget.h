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

#ifndef __qSlicerExtensionsLocalWidget_h
#define __qSlicerExtensionsLocalWidget_h

// Qt includes
#include <QListWidget>
#include <QUrl>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsItemDelegate;
class qSlicerExtensionsManagerModel;
class qSlicerExtensionsLocalWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsLocalWidget : public QListWidget
{
  Q_OBJECT
  Q_PROPERTY(QString searchText READ searchText WRITE setSearchText)
public:
  /// Superclass typedef
  typedef QListWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsLocalWidget(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerExtensionsLocalWidget() override;

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel() const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

  /// Allow filtering of extensions list
  QString searchText() const;

signals:
  void linkActivated(const QUrl& link);

public slots:
  void displayExtensionDetails(const QString& extensionName);
  void setSearchText(const QString& newText);
  // Refresh after application settings changed (server URL, autoupdate settings)
  void refresh();

protected slots:
  void addBookmark(const QString& extensionName);
  void removeBookmark(const QString& extensionName);
  void installExtension(const QString& extensionName);
  void setExtensionEnabled(const QString& extensionName);
  void setExtensionDisabled(const QString& extensionName);
  void setExtensionUpdateAvailable(const QString& extensionName);
  void scheduleExtensionForUninstall(const QString& extensionName);
  void cancelExtensionScheduledForUninstall(const QString& extensionName);
  void scheduleExtensionForUpdate(const QString& extensionName);
  void cancelExtensionScheduledForUpdate(const QString& extensionName);
  void onModelUpdated();
  void onIconDownloadComplete(const QString& extensionName);
  void onLinkActivated(const QString& link);
  void onExtensionInstalled(const QString& extensionName);
  void onExtensionUninstalled(const QString& extensionName);
  void onExtensionMetadataUpdated(const QString& extensionName);
  void onExtensionBookmarkedChanged(const QString& extensionName, bool bookmarked);
  void onExtensionScheduledForUninstall(const QString& extensionName);
  void onExtensionCancelledScheduleForUninstall(const QString& extensionName);
  void setExtensionUpdateScheduled(const QString& extensionName);
  void setExtensionUpdateCanceled(const QString& extensionName);
  void setExtensionUpdateDownloadProgress(const QString& extensionName, qint64 received, qint64 total);
  void setExtensionInstallDownloadProgress(const QString& extensionName, qint64 received, qint64 total);
  void onModelExtensionEnabledChanged(const QString& extensionName, bool enabled);

protected:
  QScopedPointer<qSlicerExtensionsLocalWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsLocalWidget);
  Q_DISABLE_COPY(qSlicerExtensionsLocalWidget);

  friend class qSlicerExtensionsItemDelegate;
};

#endif
