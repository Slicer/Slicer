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

#ifndef __qSlicerAppMainWindow_p_h
#define __qSlicerAppMainWindow_p_h

// Qt includes
#include <QQueue>
class QToolButton;

// Slicer includes
#include "qSlicerAppExport.h"
#include "qSlicerAppMainWindow.h"
#include <qSlicerIO.h>
#include "ui_qSlicerAppMainWindow.h"

class qSlicerModuleSelectorToolBar;
class qSlicerAppMainWindowPrivate;
class qSlicerLayoutManager;

//-----------------------------------------------------------------------------
class Q_SLICER_APP_EXPORT qSlicerAppMainWindowPrivate
  : public Ui_qSlicerAppMainWindow
{
  Q_DECLARE_PUBLIC(qSlicerAppMainWindow);
protected:
  qSlicerAppMainWindow* const q_ptr;

public:
  typedef qSlicerAppMainWindowPrivate Self;
  qSlicerAppMainWindowPrivate(qSlicerAppMainWindow& object);
  virtual ~qSlicerAppMainWindowPrivate();

  virtual void init();
  virtual void setupUi(QMainWindow * mainWindow);
  virtual void setupStatusBar();

  virtual void readSettings();
  virtual void writeSettings();

  virtual void setupRecentlyLoadedMenu(const QList<qSlicerIO::IOProperties>& fileProperties);

  virtual void filterRecentlyLoadedFileProperties();

  static QList<qSlicerIO::IOProperties> readRecentlyLoadedFiles();
  static void writeRecentlyLoadedFiles(const QList<qSlicerIO::IOProperties>& fileProperties);

  virtual bool confirmClose();

  void setErrorLogIconHighlighted(bool);

#ifdef Slicer_USE_PYTHONQT
  ctkPythonConsole*               PythonConsole;
#endif
  ctkErrorLogWidget*              ErrorLogWidget;
  QToolButton*                    ErrorLogToolButton;
  qSlicerModuleSelectorToolBar*   ModuleSelectorToolBar;
  QStringList                     FavoriteModules;
  qSlicerLayoutManager*           LayoutManager;
  QQueue<qSlicerIO::IOProperties> RecentlyLoadedFileProperties;

  QByteArray                      StartupState;
};

#endif
