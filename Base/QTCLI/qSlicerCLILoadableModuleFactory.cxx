/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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
#include <QStringList>
#include <QDirIterator>

// SlicerQT includes
#include "qSlicerCLILoadableModuleFactory.h"
#include "qSlicerCLILoadableModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreCommandOptions.h"
#include "qSlicerUtils.h"

//-----------------------------------------------------------------------------
qSlicerCLILoadableModuleFactoryItem::qSlicerCLILoadableModuleFactoryItem(
  const QString& itemPath)
  :Superclass(itemPath)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerCLILoadableModuleFactoryItem::instanciator()
{
  qSlicerCLILoadableModule * module = new qSlicerCLILoadableModule();

  // Resolves symbol
  char* xmlDescription = reinterpret_cast<char*>(
    this->symbolAddress("XMLModuleDescription"));

  // Retrieve
  //if (!xmlDescription) { xmlDescription = xmlFunction ? (*xmlFunction)() : 0; }

  if (!xmlDescription)
    {
    if (this->verbose())
      {
      qWarning() << "Failed to retrieve Xml Description - Path:" << this->path();
      }
    delete module; // Clean memory
    return 0;
    }

  // Resolves symbol
  qSlicerCLILoadableModule::ModuleEntryPointType moduleEntryPoint =
    reinterpret_cast<qSlicerCLILoadableModule::ModuleEntryPointType>(
      this->symbolAddress("ModuleEntryPoint"));

  if (!moduleEntryPoint)
    {
    if (this->verbose())
      {
      qWarning() << "Failed to retrieve Module Entry Point - Path:" << this->path();
      }
    delete module; // Clean memory
    return 0;
    }
  module->setEntryPoint(moduleEntryPoint);
  
  module->setXmlModuleDescription(xmlDescription);

  module->setTempDirectory(
    qSlicerCoreApplication::application()->coreCommandOptions()->tempDirectory());

  return module;
}

//-----------------------------------------------------------------------------
qSlicerCLILoadableModuleFactory::qSlicerCLILoadableModuleFactory():Superclass()
{
  // Set the list of required symbols for CmdLineLoadableModule,
  // if one of these symbols can't be resolved, the library won't be registered.
  QStringList cmdLineModuleSymbols;
  cmdLineModuleSymbols << "XMLModuleDescription";
  cmdLineModuleSymbols << "ModuleEntryPoint";
  this->setSymbols(cmdLineModuleSymbols);
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModuleFactory::registerItems()
{
  QStringList modulePaths = qSlicerCLIModuleFactoryHelper::modulePaths();
  
  if (modulePaths.isEmpty())
    {
    qWarning() << "No loadable command line module paths provided";
    return;
    }

  // Process one path at a time
  foreach (QString path, modulePaths)
    {
    QDirIterator it(path);
    while (it.hasNext())
      {
      it.next();
      QFileInfo fileInfo = it.fileInfo();
      // Skip if item isn't a file
      if (!fileInfo.isFile()) { continue; }
      
      if (fileInfo.isSymLink())
        {
        // symLinkTarget() handles links pointing to symlinks.
        // How about a symlink pointing to a symlink ?
        fileInfo = QFileInfo(fileInfo.symLinkTarget());
        }
      // Skip if current file isn't a library
      if (!QLibrary::isLibrary(fileInfo.fileName()))
        {
        continue;
        }

      if (this->verbose())
        {
        qDebug() << "Attempt to register command line module:" << fileInfo.fileName();
        }

      QString key = this->fileNameToKey(fileInfo.filePath());
      if (!this->registerLibrary(key, fileInfo))
        {
        if (this->verbose())
          {
          qWarning() << "Failed to register module: " << key;
          }
        continue;
        }
      }
    }
}

//-----------------------------------------------------------------------------
ctkFactoryLibraryItem<qSlicerAbstractCoreModule>* qSlicerCLILoadableModuleFactory::
createFactoryLibraryItem(const QFileInfo& libraryFile)const
{
  return new qSlicerCLILoadableModuleFactoryItem(libraryFile.filePath());
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModuleFactory::fileNameToKey(const QString& objectName)const
{
  return qSlicerCLILoadableModuleFactory::extractModuleName(objectName);
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModuleFactory::extractModuleName(const QString& libraryName)
{
  return qSlicerUtils::extractModuleNameFromLibraryName(libraryName);
}
