/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

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

// Qt includes

// SlicerQT includes
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerUtils.h"

//-----------------------------------------------------------------------------
qSlicerCLILoadableModuleFactoryItem::qSlicerCLILoadableModuleFactoryItem(
  const QString& newTempDirectory) : TempDirectory(newTempDirectory)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerCLILoadableModuleFactoryItem::instanciator()
{
  // Using a scoped pointer ensures the memory will be cleaned if instantiator
  // fails before returning the module. See QScopedPointer::take()
  QScopedPointer<qSlicerCLIModule> module(new qSlicerCLIModule());

  // Resolves symbol
  const char* xmlDescription = const_cast<const char *>(reinterpret_cast<char*>(
    this->symbolAddress("XMLModuleDescription")));

  // Retrieve
  //if (!xmlDescription) { xmlDescription = xmlFunction ? (*xmlFunction)() : 0; }

  if (!xmlDescription)
    {
    if (this->verbose())
      {
      qWarning() << "Failed to retrieve Xml Description - Path:" << this->path();
      }
    return 0;
    }

  // Resolves symbol
  typedef int (*ModuleEntryPointType)(int argc, char* argv[]);
  ModuleEntryPointType moduleEntryPoint =
    reinterpret_cast<ModuleEntryPointType>(
      this->symbolAddress("ModuleEntryPoint"));

  if (!moduleEntryPoint)
    {
    if (this->verbose())
      {
      qWarning() << "Failed to retrieve Module Entry Point - Path:" << this->path();
      }
    return 0;
    }

  char buffer[256];
  // The entry point address must be encoded the same way it is decoded. As it
  // is decoded using  sscanf, it must be encoded using sprintf
  sprintf(buffer, "slicer:%p", moduleEntryPoint);
  module->setEntryPoint(QString(buffer));
  module->setModuleType("SharedObjectModule");

  module->setXmlModuleDescription(QString(xmlDescription));
  module->setTempDirectory(this->TempDirectory);
  module->setPath(this->path());
  module->setInstalled(qSlicerCLIModuleFactoryHelper::isInstalled(this->path()));

  return module.take();
}

//-----------------------------------------------------------------------------
// qSlicerCLILoadableModuleFactory
//-----------------------------------------------------------------------------
qSlicerCLILoadableModuleFactory::qSlicerCLILoadableModuleFactory()
{
  // Set the list of required symbols for CmdLineLoadableModule,
  // if one of these symbols can't be resolved, the library won't be registered.
  QStringList cmdLineModuleSymbols;
  cmdLineModuleSymbols << "XMLModuleDescription";
  cmdLineModuleSymbols << "ModuleEntryPoint";
  this->setSymbols(cmdLineModuleSymbols);

  this->TempDirectory = QDir::tempPath();
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModuleFactory::registerItems()
{
  QStringList modulePaths = qSlicerCLIModuleFactoryHelper::modulePaths();
  this->registerAllFileItems(modulePaths);
}

//-----------------------------------------------------------------------------
ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* qSlicerCLILoadableModuleFactory::
createFactoryFileBasedItem()
{
  return new qSlicerCLILoadableModuleFactoryItem(this->TempDirectory);
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModuleFactory::fileNameToKey(const QString& fileName)const
{
  return qSlicerUtils::extractModuleNameFromLibraryName(fileName);
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModuleFactory::setTempDirectory(const QString& newTempDirectory)
{
  this->TempDirectory = newTempDirectory;
}

//-----------------------------------------------------------------------------
bool qSlicerCLILoadableModuleFactory::isValidFile(const QFileInfo& file)const
{
  if (!Superclass::isValidFile(file))
    {
    return false;
    }
  return qSlicerUtils::isCLILoadableModule(file.absoluteFilePath());
}
