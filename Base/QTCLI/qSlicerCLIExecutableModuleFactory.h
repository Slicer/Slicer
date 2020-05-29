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

#ifndef __qSlicerCLIExecutableModuleFactory_h
#define __qSlicerCLIExecutableModuleFactory_h

// Slicer includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerBaseQTCLIExport.h"
class qSlicerCLIModule;

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractPluginFactory.h>

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryItem
  : public ctkAbstractFactoryFileBasedItem<qSlicerAbstractCoreModule>
{
public:
  qSlicerCLIExecutableModuleFactoryItem(const QString& newTempDirectory);
  bool load() override;
  void uninstantiate() override;
protected:
  /// Return path of the expected XML file.
  QString xmlModuleDescriptionFilePath();

  qSlicerAbstractCoreModule* instanciator() override;
  QString runCLIWithXmlArgument();
private:
  QString TempDirectory;
  qSlicerCLIModule* CLIModule;
};

class qSlicerCLIExecutableModuleFactoryPrivate;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIExecutableModuleFactory :
  public ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule>
{
public:
  typedef ctkAbstractFileBasedFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerCLIExecutableModuleFactory();
  qSlicerCLIExecutableModuleFactory(const QString& tempDir);
  ~qSlicerCLIExecutableModuleFactory() override;

  void registerItems() override;

  /// Extract module name given \a executableName
  /// For example:
  ///  Threshold.exe -> threshold
  ///  Threshold -> threshold
  QString fileNameToKey(const QString& fileName)const override;

  void setTempDirectory(const QString& newTempDirectory);

protected:
  bool isValidFile(const QFileInfo& file)const override;

  ctkAbstractFactoryItem<qSlicerAbstractCoreModule>*
    createFactoryFileBasedItem() override;

protected:

  QScopedPointer<qSlicerCLIExecutableModuleFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLIExecutableModuleFactory);
  Q_DISABLE_COPY(qSlicerCLIExecutableModuleFactory);
};

#endif
