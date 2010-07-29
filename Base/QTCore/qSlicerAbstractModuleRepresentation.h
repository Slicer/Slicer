/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#ifndef __qSlicerAbstractModuleRepresentation_h
#define __qSlicerAbstractModuleRepresentation_h

// CTK includes
#include <ctkPimpl.h>

// SlicerQt includes
#include "qSlicerObject.h"

#include "qSlicerBaseQTCoreExport.h"

/// class vtkSlicerApplicationLogic;
class vtkSlicerLogic;
class qSlicerAbstractCoreModule;
class QAction;
class qSlicerAbstractModuleRepresentationPrivate;

class Q_SLICER_BASE_QTCORE_EXPORT qSlicerAbstractModuleRepresentation : virtual public qSlicerObject
{
public:

  typedef qSlicerObject Superclass;
  qSlicerAbstractModuleRepresentation();
  virtual ~qSlicerAbstractModuleRepresentation();

  ///
  /// Return the action allowing to show the module
  /// 0 by default
  virtual QAction* createAction();

  ///
  /// Set/Get module name
  QString moduleName()const;

protected:
  ///
  /// All inialization code should be done in the setup
  virtual void setup() = 0;

  /// Return the logic of the module
  vtkSlicerLogic* logic() const;

  const qSlicerAbstractCoreModule* module()const;

private:
  CTK_DECLARE_PRIVATE(qSlicerAbstractModuleRepresentation);

  /// Give access to qSlicerAbstractModule to the method
  /// qSlicerAbstractModuleRepresentation::setModule
  friend class qSlicerAbstractCoreModule;
  void setModule(qSlicerAbstractCoreModule* module);
};

#endif
