/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerCamerasModule_h
#define __qSlicerCamerasModule_h

/// SlicerQT includes
#include "qSlicerAbstractCoreModule.h"

/// qCTK includes
#include <qCTKPimpl.h>

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerAbstractModuleWidget; 
class qSlicerCamerasModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerCamerasModule :
  public qSlicerAbstractCoreModule
{
  Q_OBJECT
public:
  typedef qSlicerAbstractCoreModule Superclass;
  qSlicerCamerasModule(QObject *parent=0);

  qSlicerGetTitleMacro("Cameras");

  /// Return help/acknowledgement text
  virtual QString helpText()const;
  virtual QString acknowledgementText()const;

protected:

  /// 
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleWidget * createWidgetRepresentation();

  /// 
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();
  
private:
  QCTK_DECLARE_PRIVATE(qSlicerCamerasModule);
};

#endif
