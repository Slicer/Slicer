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

// Qt includes

// SlicerQt includes
#include "qSlicerModuleManager.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModuleFactoryManager.h"

// MRML includes

//-----------------------------------------------------------------------------
class qSlicerModuleManagerPrivate
{
public:

  /// Handle post-load initialization
  void onModuleLoaded(qSlicerAbstractCoreModule* module);

  typedef QHash<QString, qSlicerAbstractCoreModule*>::const_iterator ModuleListConstIterator;
  typedef QHash<QString, qSlicerAbstractCoreModule*>::iterator       ModuleListIterator;

  /// Store Pair<ModuleName, ModuleObject>
  QHash<QString, qSlicerAbstractCoreModule*> ModuleList;
  qSlicerModuleFactoryManager            ModuleFactoryManager;
};

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager(QObject* newParent)
  : Superclass(newParent), d_ptr(new qSlicerModuleManagerPrivate)
{
  //Q_D(qSlicerModuleManager);

  // The module manager should be instantiated only if a qSlicerCoreApplication exists
  Q_ASSERT(qSlicerCoreApplication::application());
}

//-----------------------------------------------------------------------------
qSlicerModuleManager::~qSlicerModuleManager()
{

}

//-----------------------------------------------------------------------------
void qSlicerModuleManager::printAdditionalInfo()
{
  Q_D(qSlicerModuleManager);
  qDebug() << "qSlicerModuleManager (" << this << ")";
  qDebug() << "ModuleList";

  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constBegin();
  while(iter != d->ModuleList.constEnd())
    {
    qDebug() << "Name:" << iter.key();
    iter.value()->printAdditionalInfo();
    ++iter;
    }
  d->ModuleFactoryManager.printAdditionalInfo();
}

//---------------------------------------------------------------------------
qSlicerModuleFactoryManager* qSlicerModuleManager::factoryManager()const
{
  Q_D(const qSlicerModuleManager);
  return const_cast<qSlicerModuleFactoryManager*>(&d->ModuleFactoryManager);
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::isLoaded(const QString& name)const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleList.contains(name);
}

//---------------------------------------------------------------------------
int qSlicerModuleManager::loadAllModules()
{
  int loadedCount = 0;
  foreach(const QString& name, this->factoryManager()->moduleNames())
    {
    bool loaded = this->loadModule(name);
    if (loaded)
      {
      ++loadedCount;
      }
    }
  return loadedCount;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::loadModule(const QString& name)
{
  Q_D(qSlicerModuleManager);

  // A module should be registered when attempting to load it
  //Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    return false;
    }

  // Check if module has been loaded already
  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.constFind(name);
  if (iter != d->ModuleList.constEnd())
    {
    return true;
    }

  // Instantiate the module if needed
  qSlicerAbstractCoreModule * _module = d->ModuleFactoryManager.instantiateModule(name);
  if (!_module)
    {
    return 0;
    }

  // Load the modules the module depends on.
  // There is no cycle check, so be careful
  foreach(const QString& dependency, _module->dependencies())
    {
    // no-op if the module is already loaded
    this->loadModule(dependency);
    }

  // Update internal Map
  d->ModuleList[name] = _module;

  // Initialize module
  _module->initialize(qSlicerCoreApplication::application()->applicationLogic());

  // Retrieve module title
  QString _moduleTitle = d->ModuleFactoryManager.moduleTitle(name);
  Q_ASSERT(!_moduleTitle.isEmpty());
  if (_moduleTitle.isEmpty())
    {
    qWarning() << "Failed to retrieve module title corresponding to module name: " << name;
    return 0;
    }

  // Set the MRML scene
  _module->setMRMLScene(qSlicerCoreApplication::application()->mrmlScene());

  // Module should also be aware if current MRML scene has changed
  this->connect(qSlicerCoreApplication::application(),
                SIGNAL(mrmlSceneChanged(vtkMRMLScene*)),
                _module,
                SLOT(setMRMLScene(vtkMRMLScene*)));

  // Handle post-load initialization
  emit this->moduleLoaded(_module);
  
  return true;
}

//---------------------------------------------------------------------------
bool qSlicerModuleManager::unLoadModule(const QString& name)
{
  Q_D(qSlicerModuleManager);

  // A module should be registered when attempting to unload it
  //Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    return false;
    }

  qSlicerModuleManagerPrivate::ModuleListConstIterator iter = d->ModuleList.find( name );
  if (iter == d->ModuleList.constEnd())
    {
    qWarning() << "Failed to unload module: " << name << " - Module wasn't loaded";
    return false;
    }

  qSlicerAbstractCoreModule * moduleToUnload = iter.value();
  Q_ASSERT(moduleToUnload);

  // Handle pre-unload
  emit this->moduleAboutToBeUnloaded(moduleToUnload);

  // Tells Qt to delete the object when appropriate
  moduleToUnload->deleteLater();

  // Remove the object from the list
  d->ModuleList.remove(iter.key());

  return true;
}

//---------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerModuleManager::module(const QString& name)
{
  Q_D(qSlicerModuleManager);

  // A module should be registered when attempting to obtain it
  // assert causes a crash on linux64 when this check fails
  //  Q_ASSERT(d->ModuleFactoryManager.isRegistered(name));
  if (!d->ModuleFactoryManager.isRegistered(name))
    {
    qDebug() << "The module" << name << "has not been registered.";
    qDebug() << "The following modules have been registered:"
             << d->ModuleFactoryManager.moduleNames();
    return 0;
    }
  if (!d->ModuleFactoryManager.isInstantiated(name))
    {
    qDebug() << "The module" << name << "has been registered but not instantiated.";
    qDebug() << "The following modules have been instantiated:"
             << d->ModuleFactoryManager.instantiatedModuleNames();
    return 0;
    }

  qSlicerModuleManagerPrivate::ModuleListConstIterator iter =
    d->ModuleList.find(name);
  if ( iter == d->ModuleList.constEnd() )
    {
    qDebug()<< "The module" << name << "has not been loaded.";
    qDebug() << "The following modules have been loaded:"
             << this->loadedModules();
    return 0;
    }
  return iter.value();
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleTitle(const QString& name) const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleFactoryManager.moduleTitle(name);
}

//---------------------------------------------------------------------------
QString qSlicerModuleManager::moduleName(const QString& title) const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleFactoryManager.moduleName(title);
}

//---------------------------------------------------------------------------
QStringList qSlicerModuleManager::moduleList() const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleFactoryManager.moduleNames();
}

//---------------------------------------------------------------------------
QStringList qSlicerModuleManager::loadedModules() const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleList.keys();
}
