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


#ifndef __qSlicerCLILoadableModuleFactory_h
#define __qSlicerCLILoadableModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractLibraryFactory.h>

// Slicer includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCLIExport.h"

class ModuleDescription;
class ModuleLogo;
class qSlicerCLIModule;

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModuleFactoryItem
  : public ctkFactoryLibraryItem<qSlicerAbstractCoreModule>
{
public:
  typedef ctkFactoryLibraryItem<qSlicerAbstractCoreModule> Superclass;
  qSlicerCLILoadableModuleFactoryItem(const QString& newTempDirectory);
  bool load() override;

  static void loadLibraryAndResolveSymbols(
      void* libraryLoader,  ModuleDescription& desc);

protected:
  /// Return path of the expected XML file.
  QString xmlModuleDescriptionFilePath()const;

  qSlicerAbstractCoreModule* instanciator() override;
  QString resolveXMLModuleDescriptionSymbol();
  bool resolveSymbols(ModuleDescription& desc);
  static bool updateLogo(qSlicerCLILoadableModuleFactoryItem* item, ModuleLogo& logo);
private:
  QString TempDirectory;
};

class qSlicerCLILoadableModuleFactoryPrivate;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLILoadableModuleFactory :
  public ctkAbstractLibraryFactory<qSlicerAbstractCoreModule>
{
public:
  typedef ctkAbstractLibraryFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerCLILoadableModuleFactory();
  ~qSlicerCLILoadableModuleFactory() override;

  /// Reimplemented to scan the directory of the command line modules
  void registerItems() override;

  /// Extract module name given \a libraryName
  /// For example:
  ///  libThresholdLib.so -> threshold
  ///  libThresholdLib.{dylib, bundle, so} -> threshold
  ///  ThresholdLib.dll -> threshold
  /// \sa qSlicerUtils::extractModuleNameFromLibraryName
  QString fileNameToKey(const QString& fileName)const override;

  void setTempDirectory(const QString& newTempDirectory);

protected:
  ctkAbstractFactoryItem<qSlicerAbstractCoreModule>*
    createFactoryFileBasedItem() override;

  bool isValidFile(const QFileInfo& file)const override;

protected:

  QScopedPointer<qSlicerCLILoadableModuleFactoryPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerCLILoadableModuleFactory);
  Q_DISABLE_COPY(qSlicerCLILoadableModuleFactory);
};

#endif
