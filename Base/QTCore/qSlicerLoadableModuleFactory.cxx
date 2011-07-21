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
#include <QDirIterator>
#include <QSettings>
#include <QStringList>

// SlicerQt includes
#include "qSlicerLoadableModuleFactory.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerUtils.h"

// VTKSYS includes
#include <vtksys/SystemTools.hxx>

// For:
//  - Slicer_QTLOADABLEMODULES_LIB_DIR
#include "vtkSlicerConfigure.h"

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactoryItem::qSlicerLoadableModuleFactoryItem()
{

}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerLoadableModuleFactoryItem::instanciator()
{
  qSlicerAbstractCoreModule * module =
      ctkFactoryPluginItem<qSlicerAbstractCoreModule>::instanciator();
  module->setPath(this->path());
  return module;
}

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
  Q_ASSERT(!app->slicerHome().isEmpty());
  
  QStringList defaultQTModulePaths;
#ifdef Slicer_QTLOADABLEMODULES_LIB_DIR
  defaultQTModulePaths << app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR;
#endif
  if (!app->intDir().isEmpty())
    {
    // On Win32, *both* paths have to be there, since scripts are installed
    // in the install location, and exec/libs are *automatically* installed
    // in intDir.
#ifdef Slicer_QTLOADABLEMODULES_LIB_DIR
    defaultQTModulePaths << app->slicerHome() + "/" + Slicer_QTLOADABLEMODULES_LIB_DIR + "/" + app->intDir();
#endif
    }

  QStringList additionalModulePaths = QSettings().value("Modules/AdditionalPaths").toStringList();
  QStringList qtModulePaths =  additionalModulePaths + defaultQTModulePaths;
  foreach(const QString& path, qtModulePaths)
    {
    QString currentPath(vtksys::SystemTools::GetEnv("PATH"));
    vtksys::SystemTools::PutEnv(QString("PATH=%1;%2").arg(path).arg(currentPath).toLatin1());
    app->addLibraryPath(path);
    }

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

  this->registerAllFileItems(d->modulePaths());
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleFactory::fileNameToKey(const QString& fileName)const
{
  return qSlicerLoadableModuleFactory::extractModuleName(fileName);
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleFactory::extractModuleName(const QString& libraryName)
{
  return qSlicerUtils::extractModuleNameFromLibraryName(libraryName);
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleFactoryItem* qSlicerLoadableModuleFactory::createFactoryFileBasedItem()
{
  return new qSlicerLoadableModuleFactoryItem();
}
