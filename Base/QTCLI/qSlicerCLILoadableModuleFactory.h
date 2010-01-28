/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLILoadableModuleFactory_h
#define __qSlicerCLILoadableModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCLIExport.h"

// qCTK includes
#include <qCTKPimpl.h>
#include <qCTKAbstractLibraryFactory.h>

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModuleFactoryItem : public qCTKFactoryLibraryItem<qSlicerAbstractModule>
{
public:
  typedef qCTKFactoryLibraryItem<qSlicerAbstractModule> Superclass;
  explicit qSlicerCLILoadableModuleFactoryItem(const QString& itemKey, const QString& itemPath);
  virtual ~qSlicerCLILoadableModuleFactoryItem(){}

protected:
  // To make the cast code cleaner to read.
  //typedef char * (*XMLModuleDescriptionFunction)();
  //typedef int (*ModuleEntryPoint)(int argc, char* argv[]);

  virtual qSlicerAbstractModule* instanciator();
};

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModuleFactoryPrivate;

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLILoadableModuleFactory :
  public qCTKAbstractLibraryFactory<qSlicerAbstractModule,qSlicerCLILoadableModuleFactoryItem>
{
public:

  typedef qCTKAbstractLibraryFactory<qSlicerAbstractModule,
                                     qSlicerCLILoadableModuleFactoryItem> Superclass;
  qSlicerCLILoadableModuleFactory();
  virtual ~qSlicerCLILoadableModuleFactory(){}

  virtual void registerItems();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCLILoadableModuleFactory);
};

#endif
