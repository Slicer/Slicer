/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerMRMLTreeModule_h
#define __qSlicerMRMLTreeModule_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerCoreModule.h"

#include "qSlicerBaseQTCoreModulesExport.h"

class qSlicerMRMLTreeModulePrivate;

class Q_SLICER_BASE_QTCOREMODULES_EXPORT qSlicerMRMLTreeModule :
  public qSlicerCoreModule
{
  Q_OBJECT
public:

  typedef qSlicerCoreModule Superclass;
  qSlicerMRMLTreeModule(QObject *parent=0);

  ///
  /// Display name for the module
  qSlicerGetTitleMacro("MRMLTree");

  ///
  /// Help text of the module
  virtual QString helpText()const;

  ///
  /// Acknowledgement of the module
  virtual QString acknowledgementText()const;

protected:
  ///
  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation();

  ///
  /// Create and return the logic associated to this module
  virtual vtkSlicerLogic* createLogic();

private:
  ///
  /// Pimpl declaration
  CTK_DECLARE_PRIVATE(qSlicerMRMLTreeModule);
};

#endif
