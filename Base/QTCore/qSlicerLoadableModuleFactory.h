/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerLoadableModuleFactory_h
#define __qSlicerLoadableModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractPluginFactory.h>

// SlicerQt includes
#include "qSlicerAbstractCoreModule.h"

#include "qSlicerBaseQTCoreExport.h"

class qSlicerLoadableModuleFactoryPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerLoadableModuleFactory :
  public ctkAbstractPluginFactory<qSlicerAbstractCoreModule>
{
public:

  typedef ctkAbstractPluginFactory<qSlicerAbstractCoreModule> Superclass;
  qSlicerLoadableModuleFactory();

  ///
  virtual void registerItems();

  ///
  virtual QString fileNameToKey(const QString& fileName);

  ///
  /// Extract module name given \a libraryName
  /// \sa qSlicerUtils::extractModuleNameFromLibraryName
  static QString extractModuleName(const QString& libraryName);

private:
  CTK_DECLARE_PRIVATE(qSlicerLoadableModuleFactory);
};

#endif
