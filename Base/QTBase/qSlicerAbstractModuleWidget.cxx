/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerAbstractModuleWidget.h"

// SlicerQT includes
#include "vtkSlicerLogic.h"
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAbstractModuleWidgetPrivate: public qCTKPrivate<qSlicerAbstractModuleWidget>
{
public:
  QString Name;
  vtkWeakPointer<vtkSlicerLogic>  Logic;
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerAbstractModuleWidget, const QString&, setName, Name);
QCTK_GET_CXX(qSlicerAbstractModuleWidget, QString, name, Name);

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAbstractModuleWidget::logic()const
{
  QCTK_D(const qSlicerAbstractModuleWidget);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setLogic(vtkSlicerLogic* _logic)
{
  // setLogic should be called only one time with valid object...
  Q_ASSERT(_logic);
  QCTK_D(qSlicerAbstractModuleWidget);
  d->Logic = _logic; 
}
