/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt/CoreModules
#include "qSlicerCoreModuleFactory.h"
#include "qSlicerCamerasModule.h"
#include "qSlicerDataModule.h"
#include "qSlicerMRMLTreeModule.h"
#include "qSlicerROIModule.h"
#include "qSlicerTransformsIO.h"
#include "qSlicerTransformsModule.h"
// FIXME:Move the following to the Models module (when it will be ready in Qt.)
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"
#include "qSlicerModelsIO.h"
#include "qSlicerScalarOverlayIO.h"
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
  d->registerCoreModule<qSlicerDataModule>();
  d->registerCoreModule<qSlicerMRMLTreeModule>();
  d->registerCoreModule<qSlicerROIModule>();
  d->registerCoreModule<qSlicerTransformsModule>();
  
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
