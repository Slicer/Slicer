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
#include <QWidget>

// QtGUI includes
#include "qSlicerBaseQTGUIExport.h"

class qSlicerExtensionsManagerModel;
class qSlicerExtensionsManageWidgetPrivate;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerExtensionsManageWidget
  : public QWidget
{
  Q_OBJECT
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructor
  explicit qSlicerExtensionsManageWidget(QWidget* parent = 0);

  /// Destructor
  virtual ~qSlicerExtensionsManageWidget();

  Q_INVOKABLE qSlicerExtensionsManagerModel* extensionsManagerModel()const;
  Q_INVOKABLE void setExtensionsManagerModel(qSlicerExtensionsManagerModel* model);

public slots:
  void displayExtensionDetails(const QString& extensionName);

protected slots:
  void setExtensionEnabled(const QString& extensionName);
  void setExtensionDisabled(const QString& extensionName);
  void uninstallExtension(const QString& extensionName);
  void onModelUpdated();
  void onExtensionInstalled(const QString& extensionName);
  void onExtensionUninstalled(const QString& extensionName);
  void onModelExtensionEnabledChanged(const QString& extensionName, bool enabled);

protected:
  QScopedPointer<qSlicerExtensionsManageWidgetPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerExtensionsManageWidget);
  Q_DISABLE_COPY(qSlicerExtensionsManageWidget);
};

#endif

