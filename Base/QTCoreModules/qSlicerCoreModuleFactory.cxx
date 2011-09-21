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

// SlicerQt/CoreModules includes
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerCamerasModule.h"
#include "qSlicerColorIO.h"
#include "qSlicerColorsModule.h"
#include "qSlicerDataModule.h"
#include "qSlicerEventBrokerModule.h"
#include "qSlicerFiducialsIO.h"
//#include "qSlicerROIModule.h"
#include "qSlicerTransformsIO.h"
#include "qSlicerTransformsModule.h"
// FIXME:Move the following to the Models module (when it will be ready in Qt.)
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerScalarOverlayIO.h"
#include "qSlicerSliceControllerModule.h"
// endofFIXME
#include "qSlicerUtils.h"
  
//-----------------------------------------------------------------------------
class qSlicerCoreModuleFactoryPrivate
{
  Q_DECLARE_PUBLIC(qSlicerCoreModuleFactory);
protected:
  qSlicerCoreModuleFactory* const q_ptr;

public:
  qSlicerCoreModuleFactoryPrivate(qSlicerCoreModuleFactory& object);

  ///
  /// Add a module class to the core module factory
  template<typename ClassType>
  void registerCoreModule();
};

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryPrivate methods

//-----------------------------------------------------------------------------
qSlicerCoreModuleFactoryPrivate::qSlicerCoreModuleFactoryPrivate(qSlicerCoreModuleFactory& object)
  : q_ptr(&object)
{
}

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerCoreModuleFactoryPrivate::registerCoreModule()
{
  Q_Q(qSlicerCoreModuleFactory);
  
  QString _moduleName;
  if (!q->registerQObject<ClassType>(_moduleName))
    {
    if (q->verbose())
      {
      qDebug() << "Failed to register module: " << _moduleName;
      }
    return;
    }
}

//-----------------------------------------------------------------------------
// qSlicerCoreModuleFactory methods

//-----------------------------------------------------------------------------
qSlicerCoreModuleFactory::qSlicerCoreModuleFactory()
  : d_ptr(new qSlicerCoreModuleFactoryPrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerCoreModuleFactory::~qSlicerCoreModuleFactory()
{
}

//-----------------------------------------------------------------------------
void qSlicerCoreModuleFactory::registerItems()
{
  Q_D(qSlicerCoreModuleFactory);
  d->registerCoreModule<qSlicerCamerasModule>();
  // Note: If the color module is disabled, the color logic
  //       won't be initialized.
  d->registerCoreModule<qSlicerColorsModule>();
  d->registerCoreModule<qSlicerDataModule>();
  d->registerCoreModule<qSlicerEventBrokerModule>();
  // vtkMRMLROINode are obsolete, one should use the Annotations module to
  // interact with nodes.
  //d->registerCoreModule<qSlicerROIModule>();
  d->registerCoreModule<qSlicerSliceControllerModule>();
  d->registerCoreModule<qSlicerTransformsModule>();
  
  qSlicerCoreApplication::application()->coreIOManager()
    ->registerIO(new qSlicerColorIO());
  qSlicerCoreApplication::application()->coreIOManager()
    ->registerIO(new qSlicerFiducialsIO());
  qSlicerCoreApplication::application()->coreIOManager()
    ->registerIO(new qSlicerTransformsIO());
  // FIXME: Move the following to the Models module (when it will be ready in Qt.)
  Q_ASSERT(qSlicerCoreApplication::application());
  if(!qSlicerCoreApplication::application())
    {
    return;
    }
  qSlicerCoreApplication::application()->coreIOManager()
    ->registerIO(new qSlicerScalarOverlayIO());
  // endofFIXME
}

//-----------------------------------------------------------------------------
QString qSlicerCoreModuleFactory::objectNameToKey(const QString& objectName)
{
  return qSlicerCoreModuleFactory::extractModuleName(objectName);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreModuleFactory::extractModuleName(const QString& className)
{
  return qSlicerUtils::extractModuleNameFromClassName(className);
}
