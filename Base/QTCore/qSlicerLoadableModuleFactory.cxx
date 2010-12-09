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
#include <QDirIterator>
#include <QSettings>
#include <QStringList>

// SlicerQt includes
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerUtils.h"

// For:
//  - Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR
#include "vtkSlicerConfigure.h"
  
//-----------------------------------------------------------------------------
class qSlicerLoadableModuleFactoryPrivate
{
public:
  ///
  /// Return a list of module paths
  QStringList modulePaths() const;
};

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
                                             + Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR;

   if (!app->intDir().isEmpty())
     {
     defaultQTModulePaths << app->slicerHome() + "/" + Slicer_INSTALL_QTLOADABLEMODULES_LIB_DIR + "/" + app->intDir();
     }

  // add the default modules directory (based on the slicer
  // installation or build tree) to the user paths
  QStringList qtModulePaths = /*userModulePaths + PathSep + */defaultQTModulePaths;
  foreach(const QString& path, qtModulePaths)
    {
    app->addLibraryPath(path);
    }

  QSettings settings;
  foreach(const QString& path, settings.value("Modules/Extensions", QStringList()).toStringList())
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

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleFactory Methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactory::qSlicerLoadableModuleFactory()
  : d_ptr(new qSlicerLoadableModuleFactoryPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactory::~qSlicerLoadableModuleFactory()
{
}

//-----------------------------------------------------------------------------
void qSlicerLoadableModuleFactory::registerItems()
{
  Q_D(qSlicerLoadableModuleFactory);

  QStringList modulePaths = d->modulePaths();
  
  if (modulePaths.isEmpty())
    {
    qWarning() << "No loadable module paths provided";
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
      //qDebug() << "Verifying loadable module:" << fileInfo.fileName();
      
      // Skip if item isn't a file
      if (!fileInfo.isFile()) { continue; }
      
      if (fileInfo.isSymLink())
        {
        // symLinkTarget() handles links pointing to symlinks.
        // How about a symlink pointing to a symlink ?
        fileInfo = QFileInfo(fileInfo.symLinkTarget());
        }
      // Skip if current file isn't a library
      if (!QLibrary::isLibrary(fileInfo.fileName())) { continue; }

      if (this->verbose())
        {
        qDebug() << "Attempt to register loadable module:" << fileInfo.fileName();
        }

      QString libraryName = this->fileNameToKey(fileInfo.fileName());
      if (!this->registerLibrary(libraryName, fileInfo))
        {
        if (this->verbose())
          {
          qDebug() << "Failed to register module: " << libraryName;
          }
        continue;
        }
      }
    }
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleFactory::fileNameToKey(const QString& fileName)
{
  return qSlicerLoadableModuleFactory::extractModuleName(fileName);
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleFactory::extractModuleName(const QString& libraryName)
{
  return qSlicerUtils::extractModuleNameFromLibraryName(libraryName);
}
