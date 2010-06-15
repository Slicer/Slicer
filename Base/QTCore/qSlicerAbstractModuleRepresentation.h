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
class qSlicerAbstractModule;
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
  virtual QAction* showModuleAction()  { return 0; }

  ///
  /// Set/Get module name
  virtual void setName(const QString& moduleName);
  QString name()const;

protected:
  ///
  /// All inialization code should be done in the setup
  virtual void setup() = 0;

  ///
  /// Set/Get associated Logic
  void setLogic(vtkSlicerLogic* logic);
  vtkSlicerLogic* logic() const;

private:
  CTK_DECLARE_PRIVATE(qSlicerAbstractModuleRepresentation);
  void setModule(qSlicerAbstractModule* module);
  /// Give access to qSlicerAbstractModule to the method qSlicerAbstractModuleRepresentation::setLogic
  friend class qSlicerAbstractModule;
};

#endif
