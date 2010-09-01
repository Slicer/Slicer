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

// SlicerQt/CoreModules includes
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerCamerasModule.h"
#include "qSlicerColorIO.h"
#include "qSlicerColorsModule.h"
#include "qSlicerDataModule.h"
#include "qSlicerFiducialsIO.h"
#include "qSlicerMRMLTreeModule.h"
#include "qSlicerROIModule.h"
#include "qSlicerTransformsIO.h"
#include "qSlicerTransformsModule.h"
// FIXME:Move the following to the Models module (when it will be ready in Qt.)
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerModelsIO.h"
#include "qSlicerScalarOverlayIO.h"
#include "qSlicerSliceControllerModule.h"
// endofFIXME
  
//-----------------------------------------------------------------------------
class qSlicerCoreModuleFactoryPrivate: public ctkPrivate<qSlicerCoreModuleFactory>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerCoreModuleFactory);
  qSlicerCoreModuleFactoryPrivate(){}

  ///
  /// Add a module class to the core module factory
  template<typename ClassType>
  void registerCoreModule();
};

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryPrivate methods

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerCoreModuleFactoryPrivate::registerCoreModule()
{
  CTK_P(qSlicerCoreModuleFactory);
  
  QString _moduleName;
  if (!p->registerQObject<ClassType>(_moduleName))
    {
    if (p->verbose())
      {
      qDebug() << "Failed to register module: " << _moduleName;
      }
    return;
    }
}

//-----------------------------------------------------------------------------
// qSlicerCoreModuleFactory methods

//-----------------------------------------------------------------------------
qSlicerCoreModuleFactory::qSlicerCoreModuleFactory():Superclass()
{
  CTK_INIT_PRIVATE(qSlicerCoreModuleFactory);
}

//-----------------------------------------------------------------------------
void qSlicerCoreModuleFactory::registerItems()
{
  CTK_D(qSlicerCoreModuleFactory);
  d->registerCoreModule<qSlicerCamerasModule>();
  // if you disable the color module, it would not initialize the color logic
  d->registerCoreModule<qSlicerColorsModule>();
  d->registerCoreModule<qSlicerDataModule>();
  //d->registerCoreModule<qSlicerMRMLTreeModule>();
  d->registerCoreModule<qSlicerROIModule>();
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
    ->registerIO(new qSlicerModelsIO());
  qSlicerCoreApplication::application()->coreIOManager()
    ->registerIO(new qSlicerScalarOverlayIO());
  // endofFIXME
}

//-----------------------------------------------------------------------------
QString qSlicerCoreModuleFactory::objectNameToKey(const QString& objectName)
{
  return Self::extractModuleName(objectName);
}

//-----------------------------------------------------------------------------
QString qSlicerCoreModuleFactory::extractModuleName(const QString& className)
{
  QString moduleName = className;
  
  // Remove prefix 'qSlicer' if needed
  if (moduleName.indexOf("qSlicer") == 0)
    {
    moduleName.remove(0, 7);
    }

  // Remove suffix 'Module' if needed
  int index = moduleName.lastIndexOf("Module");
  if (index == (moduleName.size() - 6))
    {
    moduleName.remove(index, 6);
    }

  return moduleName.toLower();
}
