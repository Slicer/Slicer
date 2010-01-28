/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCLIExecutableModuleFactory.h"

// SlicerQT includes
#include "qSlicerCLIModule.h"
#include "qSlicerCLIModuleFactoryHelper.h"

// QT includes
#include <QStringList>

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactoryItem::qSlicerCLIExecutableModuleFactoryItem(const QString& itemKey,
  const QString& itemPath):Superclass(itemKey),Path(itemPath)
{
}

//-----------------------------------------------------------------------------
bool qSlicerCLIExecutableModuleFactoryItem::load()
{
  return false;
}

//-----------------------------------------------------------------------------
QString qSlicerCLIExecutableModuleFactoryItem::path()
{
  return this->Path;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModule* qSlicerCLIExecutableModuleFactoryItem::instanciator()
{
  qDebug() << "CmdLineExecutableModuleItem::instantiate - name:" << this->path();
  return 0;
}

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate:public qCTKPrivate<qSlicerCLIExecutableModuleFactory>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerCLIExecutableModuleFactory);
  qSlicerCLIExecutableModuleFactoryPrivate()
    {
    }
};

//-----------------------------------------------------------------------------
qSlicerCLIExecutableModuleFactory::qSlicerCLIExecutableModuleFactory():Superclass()
{
  QCTK_INIT_PRIVATE(qSlicerCLIExecutableModuleFactory);
}

//-----------------------------------------------------------------------------
void qSlicerCLIExecutableModuleFactory::registerItems()
{
  
}
