/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerLoadableModuleFactory.h"

// SlicerQT includes
//#include "qSlicerCLIModule.h"
#include "qSlicerCoreApplication.h"

// QT includes
#include <QStringList>
#include <QDirIterator>

// For:
//  - Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR
#include "vtkSlicerConfigure.h"
  
//-----------------------------------------------------------------------------
class qSlicerLoadableModuleFactoryPrivate:public qCTKPrivate<qSlicerLoadableModuleFactory>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerLoadableModuleFactory);
  qSlicerLoadableModuleFactoryPrivate()
    {
    }

  ///
  /// Return a list of module paths
  QStringList modulePaths() const;
};

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactory::qSlicerLoadableModuleFactory():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerLoadableModuleFactory);
}

//-----------------------------------------------------------------------------
void qSlicerLoadableModuleFactory::registerItems()
{
  QCTK_D(qSlicerLoadableModuleFactory);

  QStringList modulePaths = d->modulePaths();
  
  if (modulePaths.isEmpty())
    {
    qWarning() << "No loadable module paths provided";
    return;
    }
  qWarning() << "d->ModulePaths:" << modulePaths;

  // Process one path at a time
  foreach (QString path, modulePaths)
    {
    QDirIterator it(path);
    while (it.hasNext())
      {
      it.next();
      QFileInfo fileInfo = it.fileInfo();
      //qDebug() << "Verifying loadable module:" << fileInfo.fileName();
      
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

      qDebug() << "Attempt to register loadable module:" << fileInfo.fileName();

      QString libraryName;
      if (!this->registerLibrary(fileInfo, libraryName))
        {
        qDebug() << "Failed to register module: " << libraryName; 
        return;
        }
      }
    }
}

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleFactoryPrivate Methods

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableModuleFactoryPrivate::modulePaths() const
{
  qSlicerCoreApplication* app = qSlicerCoreApplication::application();
  Q_ASSERT(app);

  // slicerHome shouldn't be empty
  Q_ASSERT(!app->slicerHome().isEmpty());
  
  // On Win32, *both* paths have to be there, since scripts are installed
  // in the install location, and exec/libs are *automatically* installed
  // in intDir.
  QStringList defaultQTModulePaths;
  defaultQTModulePaths << app->slicerHome() + "/"
                                             + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR;

   if (!app->intDir().isEmpty())
     {
     defaultQTModulePaths << app->slicerHome() + "/" + Slicer3_INSTALL_QTLOADABLEMODULES_LIB_DIR + "/" + app->intDir();
     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QStringList qtModulePaths = /*userModulePaths + PathSep + */defaultQTModulePaths;
  foreach(const QString& path, qtModulePaths)
    {
    app->addLibraryPath(path);
    }

//   foreach (QString path, app->libraryPaths())
//     {
//     qDebug() << "libraryPath:" << path;
//     }

  //qDebug() << "qtModulePaths:" << qtModulePaths;
  
  return qtModulePaths; 
}
