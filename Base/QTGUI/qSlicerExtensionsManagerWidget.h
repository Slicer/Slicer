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

#ifndef __qSlicerExtensionsManagerWidget_h
#define __qSlicerExtensionsManagerWidget_h

// CTK includes
#include <ctkErrorLogLevel.h>

// Qt includes
#include <QUrl>
#include <QWidget>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsManagerWidgetPrivate;
class qSlicerExtensionsManagerModel;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsManagerWidget
  : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsManagerWidget(QWidget* parent = nullptr);

  /// Destructor
  ~qSlicerExtensionsManagerWidget() override;

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

public slots:
  void refreshInstallWidget();

protected slots:
  void onModelUpdated();

  void onCurrentTabChanged(int index);

  void onManageLinkActivated(const QUrl& link);
  void onManageUrlChanged(const QUrl& newUrl);
  void onInstallUrlChanged(const QUrl& newUrl);
  void onSearchTextChanged(const QString& newText);

  void onCheckForUpdatesTriggered();
  void onInstallFromFileTriggered();
  void onMessageLogged(const QString& text, ctkErrorLogLevel::LogLevels level);

protected:
  void timerEvent(QTimerEvent*) override;

  QScopedPointer<qSlicerExtensionsManagerWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManagerWidget);
  Q_DISABLE_COPY(qSlicerExtensionsManagerWidget);
};

#endif
