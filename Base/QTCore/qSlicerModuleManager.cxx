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

// Slicer includes
#include "qSlicerModuleManager.h"
#include "qSlicerCoreApplication.h"
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerModuleFactoryManager.h"

// MRML includes

//-----------------------------------------------------------------------------
class qSlicerModuleManagerPrivate
{
public:
  qSlicerModuleFactoryManager* ModuleFactoryManager;
};

//-----------------------------------------------------------------------------
qSlicerModuleManager::qSlicerModuleManager(QObject* newParent)
  : Superclass(newParent), d_ptr(new qSlicerModuleManagerPrivate)
{
  Q_D(qSlicerModuleManager);
  d->ModuleFactoryManager = new qSlicerModuleFactoryManager(this);
  connect(d->ModuleFactoryManager, SIGNAL(moduleLoaded(QString)),
          this, SIGNAL(moduleLoaded(QString)));
  connect(d->ModuleFactoryManager, SIGNAL(moduleAboutToBeUnloaded(QString)),
          this, SIGNAL(moduleAboutToBeUnloaded(QString)));
}

//-----------------------------------------------------------------------------
qSlicerModuleManager::~qSlicerModuleManager() = default;

//-----------------------------------------------------------------------------
void qSlicerModuleManager::printAdditionalInfo()
{
  Q_D(qSlicerModuleManager);
  qDebug() << "qSlicerModuleManager (" << this << ")";
  d->ModuleFactoryManager->printAdditionalInfo();
}

//---------------------------------------------------------------------------
qSlicerModuleFactoryManager* qSlicerModuleManager::factoryManager()const
{
  Q_D(const qSlicerModuleManager);
  return const_cast<qSlicerModuleFactoryManager*>(d->ModuleFactoryManager);
}

//---------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerModuleManager::module(const QString& name)const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleFactoryManager->loadedModule(name);
}

//---------------------------------------------------------------------------
QStringList qSlicerModuleManager::modulesNames()const
{
  Q_D(const qSlicerModuleManager);
  return d->ModuleFactoryManager->loadedModuleNames();
}
