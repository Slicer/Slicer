/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerAbstractModuleWidget.h"

// SlicerQT includes
#include "qSlicerModuleLogic.h"

// SlicerLogic includes
// #include "vtkSlicerApplicationLogic.h"

//-----------------------------------------------------------------------------
struct qSlicerAbstractModuleWidgetPrivate: public qCTKPrivate<qSlicerAbstractModuleWidget>
{
  QString Name;
  
  // Since by design qSlicerModuleLogic isn't a QObject, Logic couldn't be a QPointer.
  // We are also sure it won't be dangling pointer. Indeed, qSlicerAbstractModuleWidget
  // is repsonsible of creating widget representation and the corresponding logic.
  qSlicerModuleLogic*  Logic;
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerAbstractModuleWidget, const QString&, setName, Name);
QCTK_GET_CXX(qSlicerAbstractModuleWidget, QString, name, Name);

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::initialize(/*vtkSlicerApplicationLogic* appLogic*/)
{
  //QCTK_D(qSlicerAbstractModuleWidget);
  //Q_ASSERT(appLogic);
  //this->setAppLogic(appLogic);
  this->setup();
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractModuleWidget, qSlicerModuleLogic*, logic, Logic);

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setLogic(qSlicerModuleLogic* logic)
{
  // setLogic should be called only one time with valid object...
  Q_ASSERT(logic);
  QCTK_D(qSlicerAbstractModuleWidget);
  d->Logic = logic; 
}
