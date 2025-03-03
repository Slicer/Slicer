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

#ifndef __qSlicerApplicationHelper_h
#define __qSlicerApplicationHelper_h

// Qt includes
#include <QScopedPointer>
#include <QObject>
#include <QSplashScreen>

// Slicer includes
#include <qSlicerApplication.h>

#include "qSlicerBaseQTAppExport.h"

class ctkProxyStyle;
class qSlicerModuleFactoryManager;

class Q_SLICER_BASE_QTAPP_EXPORT qSlicerApplicationHelper : public QObject
{
  Q_OBJECT
public:
  typedef QObject Superclass;
  typedef qSlicerApplicationHelper Self;

  qSlicerApplicationHelper(QObject * parent = nullptr);
  ~qSlicerApplicationHelper() override;

  static void preInitializeApplication(const char* argv0, ctkProxyStyle* style);

  /// Perform initialization steps on the application.
  /// \return If return value is non-zero then the application will terminate using
  /// the returned value as exit code.
  template<typename SlicerMainWindowType>
  static int postInitializeApplication(
      qSlicerApplication& app,
      QScopedPointer<QSplashScreen>& splashScreen,
      QScopedPointer<SlicerMainWindowType>& window);

  static void setupModuleFactoryManager(qSlicerModuleFactoryManager * moduleFactoryManager);

  static void showMRMLEventLoggerWidget();

  /// Display a warning popup if rendering capabilities do not meet requirements.
  /// If user chooses not to continue then this method returns false.
  /// Known limitation: currently this method only works reliably on Windows operating system.
  /// See more information at https://issues.slicer.org/view.php?id=4252
  Q_INVOKABLE static bool checkRenderingCapabilities();

  /// Run command as administrator (UAC on Windows, sudo on linux)
  /// Currently only works on Windows.
  /// Returns error code (0=success).
  Q_INVOKABLE static int runAsAdmin(QString executable, QString parameters = QString(), QString workingDir = QString());

private:
  Q_DISABLE_COPY(qSlicerApplicationHelper);
};

#include "qSlicerApplicationHelper.txx"

#endif
