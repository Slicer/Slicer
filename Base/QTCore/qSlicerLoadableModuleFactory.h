/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerLoadableModuleFactory_h
#define __qSlicerLoadableModuleFactory_h

// SlicerQT includes
#include "qSlicerAbstractModule.h"

// qCTK includes
#include <qCTKPimpl.h>
#include <qCTKAbstractPluginFactory.h>

#include "qSlicerBaseQTCoreExport.h"

class qSlicerLoadableModuleFactoryPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerLoadableModuleFactory :
  public qCTKAbstractPluginFactory<qSlicerAbstractModule>
{
public:

  typedef qCTKAbstractPluginFactory<qSlicerAbstractModule> Superclass;
  qSlicerLoadableModuleFactory();
  virtual ~qSlicerLoadableModuleFactory(){}

  virtual void registerItems();


private:
  QCTK_DECLARE_PRIVATE(qSlicerLoadableModuleFactory);
};

#endif
