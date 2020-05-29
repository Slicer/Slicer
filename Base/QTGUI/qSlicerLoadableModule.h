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

#ifndef __qSlicerLoadableModule_h
#define __qSlicerLoadableModule_h

// CTK includes
#include <ctkPimpl.h>

// Slicer includes
#include "qSlicerAbstractModule.h"

#include "qSlicerBaseQTGUIExport.h"

class qSlicerLoadableModulePrivate;
class qSlicerCorePythonManager;

class Q_SLICER_BASE_QTGUI_EXPORT qSlicerLoadableModule : public qSlicerAbstractModule
{
  Q_OBJECT

public:

  typedef qSlicerLoadableModule Self;
  typedef qSlicerAbstractModule Superclass;
  qSlicerLoadableModule(QObject *parent=nullptr);
  ~qSlicerLoadableModule() override;

  /// Return help/acknowledgement text
  QString helpText()const override;
  QString acknowledgementText()const override;

  /// \brief Import python extensions associated with \a modulePath.
  ///
  /// \a modulePath can either be the path to the module library or the
  /// directory containing the module library.
  ///
  /// Python extensions corresponds to files matching the following wildcard expression:
  /// <ul>
  ///   <li>vtkSlicer*ModuleLogic.py</li>
  ///   <li>vtkSlicer*ModuleMRML.py</li>
  ///   <li>vtkSlicer*ModuleMRMLDisplayableManager.py</li>
  ///   <li>qSlicer*PythonQt.* python</li>
  /// </ul>
  ///
  /// These files are searched within the \a modulePath directory minus the \a IntDir
  /// if it applies.
  ///
  /// \sa qSlicerCoreApplication::intDir(), qSlicerCoreApplication::corePythonManager()
  static bool importModulePythonExtensions(qSlicerCorePythonManager * pythonManager,
                                           const QString& intDir,
                                           const QString& modulePath,
                                           bool isEmbedded=false);

  /// Set \a module identified by \a moduleName has an attribute of "slicer.modules" module dictionary.
  /// qSlicerCoreApplication::corePythonManager()
  static bool addModuleToSlicerModules(qSlicerCorePythonManager * pythonManager,
                                       qSlicerAbstractModule *module,
                                       const QString& moduleName);

  /// Set \a moduleName has an attribute of "slicer.moduleNames" module dictionary.
  /// qSlicerCoreApplication::corePythonManager()
  static bool addModuleNameToSlicerModuleNames(qSlicerCorePythonManager * pythonManager,
                                               const QString& moduleName);

protected:
  void setup() override;

protected:
  QScopedPointer<qSlicerLoadableModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerLoadableModule);
  Q_DISABLE_COPY(qSlicerLoadableModule);
};

Q_DECLARE_INTERFACE(qSlicerLoadableModule,
                     "org.slicer.modules.loadable.qSlicerLoadableModule/1.0");

#endif
