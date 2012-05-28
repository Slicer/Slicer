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

#ifndef __qSlicerAppMainWindowCore_h
#define __qSlicerAppMainWindowCore_h

// Qt includes
#include <QObject>

// CTK includes
#include <ctkPimpl.h>
class ctkPythonConsole;
class ctkErrorLogWidget;

#include "qSlicerAppExport.h"

class qSlicerAppMainWindow;
class qSlicerAppMainWindowCorePrivate;

class Q_SLICER_APP_EXPORT qSlicerAppMainWindowCore : public QObject
{
  Q_OBJECT

public:
  typedef QObject Superclass;
  qSlicerAppMainWindowCore(qSlicerAppMainWindow *parent = 0);
  virtual ~qSlicerAppMainWindowCore();

#ifdef Slicer_USE_PYTHONQT
  ctkPythonConsole* pythonConsole()const;
#endif
  ctkErrorLogWidget* errorLogWidget()const;

public slots:
  ///
  /// Handle actions - See qSlicerAppMainWindow::setupMenuActions

  void onFileAddDataActionTriggered();
  void onFileLoadDataActionTriggered();
  void onFileImportSceneActionTriggered();
  void onFileLoadSceneActionTriggered();
  void onFileAddVolumeActionTriggered();
  void onFileAddTransformActionTriggered();
  void onFileSaveSceneActionTriggered();
  void onSDBSaveToDirectoryActionTriggered();
  void onSDBZipDirectoryActionTriggered();
  void onSDBZipToDCMActionTriggered();
  void onFileCloseSceneActionTriggered();
  void onEditUndoActionTriggered();
  void onEditRedoActionTriggered();
  void setLayout(int);
  void setLayoutNumberOfCompareViewRows(int);
  void setLayoutNumberOfCompareViewColumns(int);
  void onWindowErrorLogActionTriggered(bool show);
  void onWindowPythonInteractorActionTriggered(bool show);

  void onHelpKeyboardShortcutsActionTriggered();
  void onHelpBrowseTutorialsActionTriggered();
  void onHelpInterfaceDocumentationActionTriggered();
  void onHelpSlicerPublicationsActionTriggered();
  void onHelpVisualBlogActionTriggered();

  void onHelpReportBugOrFeatureRequestActionTriggered();
  void onHelpAboutSlicerAppActionTriggered();

protected:
  qSlicerAppMainWindow* widget() const;

protected:
  QScopedPointer<qSlicerAppMainWindowCorePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerAppMainWindowCore);
  Q_DISABLE_COPY(qSlicerAppMainWindowCore);
};

#endif
