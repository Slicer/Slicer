/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCoreModuleFactory.h"

// SlicerQT/CoreModules
#include "qSlicerTransformsModule.h"
#include "qSlicerCamerasModule.h"
  
//-----------------------------------------------------------------------------
class qSlicerCoreModuleFactoryPrivate:public qCTKPrivate<qSlicerCoreModuleFactory>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerCoreModuleFactory);
  qSlicerCoreModuleFactoryPrivate(){}

  // Description:
  // Add a module class to the core module factory
  template<typename ClassType>
  void registerCoreModule();
};

//-----------------------------------------------------------------------------
qSlicerCoreModuleFactory::qSlicerCoreModuleFactory():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerCoreModuleFactory);
}

//-----------------------------------------------------------------------------
void qSlicerCoreModuleFactory::registerItems()
{
  QCTK_D(qSlicerCoreModuleFactory);
  d->registerCoreModule<qSlicerTransformsModule>();
  d->registerCoreModule<qSlicerCamerasModule>();
}

//-----------------------------------------------------------------------------
// qSlicerModuleFactoryPrivate methods

//-----------------------------------------------------------------------------
template<typename ClassType>
void qSlicerCoreModuleFactoryPrivate::registerCoreModule()
{
  QCTK_P(qSlicerCoreModuleFactory);
  
  QString _moduleName;
  if (!p->registerQObject<ClassType>(_moduleName))
    {
    qDebug() << "Failed to register module: " << _moduleName; 
    return;
    }
}
