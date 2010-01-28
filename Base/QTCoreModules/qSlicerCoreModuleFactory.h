/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCoreModuleFactory_h
#define __qSlicerCoreModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractModule.h"
#include "qSlicerBaseQTCoreModulesExport.h"

// qCTK includes
#include <qCTKPimpl.h>
#include <qCTKAbstractQObjectFactory.h>

class qSlicerCoreModuleFactoryPrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCoreModuleFactory :
  public qCTKAbstractQObjectFactory<qSlicerAbstractModule>
{
public:

  typedef qCTKAbstractQObjectFactory<qSlicerAbstractModule> Superclass;
  qSlicerCoreModuleFactory();
  virtual ~qSlicerCoreModuleFactory(){}

  virtual void registerItems();

private:
  QCTK_DECLARE_PRIVATE(qSlicerCoreModuleFactory);
};

#endif
