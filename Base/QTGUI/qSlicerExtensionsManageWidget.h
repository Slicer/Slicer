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

#ifndef __qSlicerExtensionsManageWidget_h
#define __qSlicerExtensionsManageWidget_h

// Qt includes
#include <QListWidget>
#include <QUrl>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsItemDelegate;
class qSlicerExtensionsManagerModel;
class qSlicerExtensionsManageWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsManageWidget
  : public QListWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QListWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsManageWidget(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerExtensionsManageWidget() override;

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

signals:
  void linkActivated(const QUrl& link);

public slots:
  void displayExtensionDetails(const QString& extensionName);

protected slots:
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
  void onExtensionScheduledForUninstall(const QString& extensionName);
  void onExtensionCancelledScheduleForUninstall(const QString& extensionName);
  void setExtensionUpdateScheduled(const QString& extensionName);
  void setExtensionUpdateCanceled(const QString& extensionName);
  void setExtensionUpdateDownloadProgress(
    const QString& extensionName, qint64 received, qint64 total);
  void onModelExtensionEnabledChanged(const QString& extensionName, bool enabled);

protected:
  QScopedPointer<qSlicerExtensionsManageWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManageWidget);
  Q_DISABLE_COPY(qSlicerExtensionsManageWidget);

  friend class qSlicerExtensionsItemDelegate;
};

#endif

