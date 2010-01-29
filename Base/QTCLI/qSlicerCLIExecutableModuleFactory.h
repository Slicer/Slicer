/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIExecutableModuleFactory_h
#define __qSlicerCLIExecutableModuleFactory_h

/// SlicerQT includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCLIExport.h"

/// qCTK includes
#include <qCTKPimpl.h>
#include <qCTKAbstractPluginFactory.h>

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryItem : public qCTKAbstractFactoryItem<qSlicerAbstractModule>
{
public:
  typedef qCTKAbstractFactoryItem<qSlicerAbstractModule> Superclass; 
  explicit qSlicerCLIExecutableModuleFactoryItem(const QString& itemKey, const QString& itemPath);
  virtual ~qSlicerCLIExecutableModuleFactoryItem(){}

  virtual bool load();
  QString path();

protected:
  virtual qSlicerAbstractModule* instanciator();

private:
  QString          Path;
};


//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIExecutableModuleFactory :
  public qCTKAbstractPluginFactory<qSlicerAbstractModule,qSlicerCLIExecutableModuleFactoryItem>
{
public:

  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule,
                                    qSlicerCLIExecutableModuleFactoryItem> Superclass;
  qSlicerCLIExecutableModuleFactory();
  virtual ~qSlicerCLIExecutableModuleFactory(){}

  virtual void registerItems();


private:
  QCTK_DECLARE_PRIVATE(qSlicerCLIExecutableModuleFactory);
};

#endif
