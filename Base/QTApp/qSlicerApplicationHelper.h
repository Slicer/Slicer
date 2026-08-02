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
#include <QElapsedTimer>
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

  qSlicerApplicationHelper(QObject* parent = nullptr);
  ~qSlicerApplicationHelper() override;

  static void preInitializeApplication(const char* argv0, ctkProxyStyle* style);

  /// Record the time when the process entry point is reached.
  /// Meant to be the first statement of main() or WinMain().
  /// Called from qSlicerApplicationMainWrapper.cxx, which every application includes into
  /// its own Main.cxx, so custom applications are measured without having to ask for it.
  ///
  /// It is used for measuring the time that it takes for the operating system loader
  /// to create the process, map the executable, load dynamic libraries that are required
  /// for the main application, initialize the C runtime, etc.
  static void recordProcessEntryPointTime();

  /// Perform initialization steps on the application.
  /// \return If return value is non-zero then the application will terminate using
  /// the returned value as exit code.
  template <typename SlicerMainWindowType>
  static int postInitializeApplication( //
    qSlicerApplication& app,
    QScopedPointer<QSplashScreen>& splashScreen,
    QScopedPointer<SlicerMainWindowType>& window);

  static void setupModuleFactoryManager(qSlicerModuleFactoryManager* moduleFactoryManager);

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

  /// Milliseconds from the creation of the process to the entry point, that is, what the
  /// loader spent before the application had any say. Stays 0 where the platform does not
  /// report when the process was created.
  ///
  /// Set once by recordProcessEntryPointTime(), read afterwards from the same thread that
  /// will report it, so it needs no synchronization of its own.
  static double TimeElapsedBeforeProcessEntryPointMs;

  /// Started by recordProcessEntryPointTime(), or, if that was never called, by
  /// preInitializeApplication(), so it is always running by the time anything reads it.
  ///
  /// A second clock, because the wall clock that dates the creation of the process is
  /// coarse and can be shifted by clock adjustments: the wall clock is read once, to
  /// obtain TimeElapsedBeforeProcessEntryPointMs, and every duration after the entry
  /// point is measured with this monotonic timer instead.
  static QElapsedTimer TimeElapsedAfterProcessEntryPointTimer;
};

#include "qSlicerApplicationHelper.hxx"

#endif
