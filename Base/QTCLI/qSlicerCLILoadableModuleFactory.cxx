/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCLILoadableModuleFactory.h"

// SlicerQT includes
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"
#include "qSlicerCoreApplication.h"

// QT includes
#include <QStringList>
#include <QDirIterator>

//-----------------------------------------------------------------------------
qSlicerCLILoadableModuleFactoryItem::qSlicerCLILoadableModuleFactoryItem(const QString& itemKey,
  const QString& itemPath):Superclass(itemKey, itemPath){}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerCLILoadableModuleFactoryItem::instanciator()
{
  qSlicerCLIModule * module = new qSlicerCLIModule();

  // Resolve symbols
  char* xmlDescription = (char*)this->symbolAddress("XMLModuleDescription");

  // Retrieve
  //if (!xmlDescription) { xmlDescription = xmlFunction ? (*xmlFunction)() : 0; }

  if (!xmlDescription)
    {
    qWarning() << "Failed to retrieve Xml Description - Path:" << this->path();
    delete module; // Clean memory
    return 0;
    }
  module->setXmlModuleDescription(xmlDescription);


  return module;
}

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModuleFactoryPrivate:public qCTKPrivate<qSlicerCLILoadableModuleFactory>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerCLILoadableModuleFactory);
  qSlicerCLILoadableModuleFactoryPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerCLILoadableModuleFactory::qSlicerCLILoadableModuleFactory():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerCLILoadableModuleFactory);
  
  // Set the list of required symbols for CmdLineLoadableModule,
  // if one of these symbols can't be resolved, the library won't be registered.
  QStringList cmdLineModuleSymbols;
  cmdLineModuleSymbols << "XMLModuleDescription";
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
      
      //this->registerLibrary(factory, it.fileInfo());
      if (fileInfo.isSymLink())
        {
        // symLinkTarget() handles links pointing to symlinks.
        // How about a symlink pointing to a symlink ?
        fileInfo = QFileInfo(fileInfo.symLinkTarget());
        }
      // Skip if current file isn't a library
      if (!QLibrary::isLibrary(fileInfo.fileName())) { continue; }
      
      qDebug() << "Attempt to register command line module:" << fileInfo.fileName();

      QString libraryName;
      if (!this->registerLibrary(fileInfo, libraryName))
        {
        qDebug() << "Failed to register module: " << libraryName; 
        return;
        }
      }
    }
}
