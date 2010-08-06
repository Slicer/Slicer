/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLIExecutableModuleFactory_h
#define __qSlicerCLIExecutableModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerBaseQTCLIExport.h"

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractPluginFactory.h>

//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryItem : public ctkAbstractFactoryItem<qSlicerAbstractCoreModule>
{
public:
  typedef ctkAbstractFactoryItem<qSlicerAbstractCoreModule> Superclass;
  explicit qSlicerCLIExecutableModuleFactoryItem(const QString& itemPath);
  virtual ~qSlicerCLIExecutableModuleFactoryItem(){}

  ///
  virtual bool load();

  ///
  /// Return path associated with the executable module
  QString path()const;

protected:
  virtual qSlicerAbstractCoreModule* instanciator();

private:
  QString          Path;
};


//-----------------------------------------------------------------------------
class qSlicerCLIExecutableModuleFactoryPrivate;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLIExecutableModuleFactory :
  public ctkAbstractPluginFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractPluginFactory<qSlicerAbstractCoreModule> Superclass;
  
  qSlicerCLIExecutableModuleFactory();
  virtual ~qSlicerCLIExecutableModuleFactory(){}

  ///
  virtual void registerItems();

  ///
  ///   virtual QString fileNameToKey(const QString& fileName);

  /// Extract module name given \a executableName
  /// For example: 
  ///  Threshold.exe -> threshold
  ///  Threshold -> threshold
  static QString extractModuleName(const QString& executableName);

protected:
  virtual ctkAbstractFactoryItem<qSlicerAbstractCoreModule>* createFactoryPluginItem(
    const QFileInfo& plugin)const;
private:
  CTK_DECLARE_PRIVATE(qSlicerCLIExecutableModuleFactory);
};

#endif
