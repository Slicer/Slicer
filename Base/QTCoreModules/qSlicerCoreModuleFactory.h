/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCoreModuleFactory_h
#define __qSlicerCoreModuleFactory_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkAbstractQObjectFactory.h>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerCoreModuleFactoryPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCoreModuleFactory :
  public ctkAbstractQObjectFactory<qSlicerAbstractModule>
{
public:

  typedef ctkAbstractQObjectFactory<qSlicerAbstractModule> Superclass;
  qSlicerCoreModuleFactory();
  virtual ~qSlicerCoreModuleFactory(){}

  ///
  /// \overload
  virtual void registerItems();

  ///
  /// \overload
  virtual QString objectNameToKey(const QString& objectName);

  /// Extract module name given a core module \a className
  /// For example: 
  ///  qSlicerCamerasModule -> cameras
  ///  qSlicerTransformsModule -> transforms
  static QString extractModuleName(const QString& className);

private:
  CTK_DECLARE_PRIVATE(qSlicerCoreModuleFactory);
};

#endif
