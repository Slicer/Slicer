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

// SlicerExecutionModule
#include <ModuleLogo.h>

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
    this->appendInstantiateErrorString(QString("CLI loadable: %1").arg(this->path()));
    this->appendInstantiateErrorString("Failed to retrieve Xml Description");
    return 0;
    }

  // Resolves symbol
  typedef int (*ModuleEntryPointType)(int argc, char* argv[]);
  ModuleEntryPointType moduleEntryPoint =
    reinterpret_cast<ModuleEntryPointType>(
      this->symbolAddress("ModuleEntryPoint"));

  if (!moduleEntryPoint)
    {
    this->appendInstantiateErrorString(QString("CLI loadable: %1").arg(this->path()));
    this->appendInstantiateErrorString("Failed to retrieve Module Entry Point");
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

  if (module->logo().isNull())
    {
    ModuleLogo logo;
    const char* logoImage = 0;
    int width = 0;
    int height = 0;
    int pixelSize = 0;
    unsigned long bufferLength = 0;
    if (this->symbolAddress("ModuleLogoImage"))
      {
      logoImage = reinterpret_cast<const char *>(this->symbolAddress("ModuleLogoImage"));
      width = *reinterpret_cast<int *>(this->symbolAddress("ModuleLogoWidth"));
      height = *reinterpret_cast<int *>(this->symbolAddress("ModuleLogoHeight"));
      pixelSize = *reinterpret_cast<int *>(this->symbolAddress("ModuleLogoPixelSize"));
      bufferLength = *reinterpret_cast<unsigned long*>(this->symbolAddress("ModuleLogoLength"));
      }
    else
      {
      typedef const char * (*ModuleLogoFunction)(int *width, int *height, int *pixel_size, unsigned long *bufferLength);
      ModuleLogoFunction logoFunction = reinterpret_cast<ModuleLogoFunction>(
        this->symbolAddress("GetModuleLogo"));
      logoImage = (*logoFunction)(&width, &height, &pixelSize, &bufferLength);
      }
    if (logoImage != 0)
      {
      logo.SetLogo(logoImage, width, height, pixelSize, bufferLength, 0);
      module->setLogo(logo);
      }
    }

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
  cmdLineModuleSymbols << "ModuleLogoImage";
  cmdLineModuleSymbols << "ModuleLogoWidth";
  cmdLineModuleSymbols << "ModuleLogoHeight";
  cmdLineModuleSymbols << "ModuleLogoPixelSize";
  cmdLineModuleSymbols << "ModuleLogoLength";
  cmdLineModuleSymbols << "GetModuleLogo";
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
