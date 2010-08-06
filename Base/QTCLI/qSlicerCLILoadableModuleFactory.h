/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCLILoadableModuleFactory_h
#define __qSlicerCLILoadableModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractLibraryFactory.h>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCLIExport.h"

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModuleFactoryItem : public ctkFactoryLibraryItem<qSlicerAbstractCoreModule>
{
public:
  // Convenient typedef
  typedef ctkFactoryLibraryItem<qSlicerAbstractCoreModule> Superclass;
  
  explicit qSlicerCLILoadableModuleFactoryItem(const QString& itemPath);
  virtual ~qSlicerCLILoadableModuleFactoryItem(){}

protected:
  /// Convenient typedef
  typedef qSlicerCLILoadableModuleFactoryItem Self;
  //typedef char * (*XMLModuleDescriptionFunction)();

  virtual qSlicerAbstractCoreModule* instanciator();
};

//-----------------------------------------------------------------------------
class Q_SLICER_BASE_QTCLI_EXPORT qSlicerCLILoadableModuleFactory :
  public ctkAbstractLibraryFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractLibraryFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerCLILoadableModuleFactory();

  /// Reimplemented to scan the directory of the command line modules
  virtual void registerItems();

  ///
  QString fileNameToKey(const QString& fileName)const;

  ///
  /// Extract module name given \a libraryName
  /// \sa qSlicerUtils::extractModuleNameFromLibraryName
  static QString extractModuleName(const QString& libraryName);
protected: 
  virtual ctkFactoryLibraryItem<qSlicerAbstractCoreModule>* createFactoryLibraryItem(
    const QFileInfo& libraryFile)const;
};

#endif
