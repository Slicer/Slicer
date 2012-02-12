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

#ifndef __qSlicerApplication_h
#define __qSlicerApplication_h

// Qt includes
#include <QPalette>

// CTK includes
#include <ctkPimpl.h>
#include <ctkSettingsDialog.h>

// QTCORE includes
#include "qSlicerCoreApplication.h"

// QTGUI includes
#include "qSlicerBaseQTGUIExport.h"

class QMainWindow;
class qSlicerApplicationPrivate;
class qSlicerCommandOptions;
class qSlicerIOManager;
#ifdef Slicer_USE_PYTHONQT
class qSlicerPythonManager;
#endif
class qSlicerLayoutManager;
class qSlicerWidget;

// MRML includes
class vtkMRMLNode;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplication : public qSlicerCoreApplication
{
  Q_OBJECT
public:

  typedef qSlicerCoreApplication Superclass;
  qSlicerApplication(int &argc, char **argv);
  virtual ~qSlicerApplication();

  /// Return a reference to the application singleton
  static qSlicerApplication* application();

  /// Get commandOptions
  Q_INVOKABLE qSlicerCommandOptions* commandOptions();

  /// Get IO Manager
  Q_INVOKABLE qSlicerIOManager* ioManager();

  #ifdef Slicer_USE_PYTHONQT
  /// Get Python Manager
  Q_INVOKABLE qSlicerPythonManager * pythonManager();
  #endif

  /// Set/Get layout manager
  Q_INVOKABLE qSlicerLayoutManager* layoutManager()const;
  void setLayoutManager(qSlicerLayoutManager* layoutManager);

  /// Return a pointer on the main window of the application if any.
  QMainWindow* mainWindow()const;

  /// TODO
  /// See http://doc.trolltech.com/4.6/qapplication.html#commitData
  /// and http://doc.trolltech.com/4.6/qsessionmanager.html#allowsInteraction
  //virtual void commitData(QSessionManager & manager);
  
  /// Enable/Disable tooltips
  void setToolTipsEnabled(bool enable);

  /// Return the best module name for a given node.
  /// \note qSlicerApplication is a temporary host for the function as it should be
  /// moved into a DataManager where module can register new node 
  /// types/modules
  QString nodeModule(vtkMRMLNode* node)const;

  ctkSettingsDialog* settingsDialog()const;

public slots:

  /// Utility function that retrieve the best module for a node and trigger
  /// its associated QAction which eventually opens the module.
  /// \note qSlicerApplication is a temporary host for the function as it should be
  /// moved into a DataManager where module can register new node 
  /// types/modules
  void openNodeModule(vtkMRMLNode* node);

  /// Popup a dialog asking the user if the application should be restarted.
  /// If no \a reason is given, the text will default to ""Are you sure you want to restart?"
  void confirmRestart(QString reason = QString());

protected slots:
  void onSettingDialogAccepted();

protected:
  /// Reimplemented from qSlicerCoreApplication
  virtual QSettings* newSettings(const QString& fileName = QString());
  virtual void handleCommandLineArguments();
  virtual void onSlicerApplicationLogicModified();

private:
  Q_DECLARE_PRIVATE(qSlicerApplication);
  Q_DISABLE_COPY(qSlicerApplication);
};

/// Apply the Slicer palette to the \c palette
/// Note also that the palette parameter is passed by reference and will be
/// updated using the native paletter and applying Slicer specific properties.
void  Q_SLICER_BASE_QTGUI_EXPORT qSlicerApplyPalette(QPalette& palette);

#endif
