/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

// Slicer includes
#include "vtkSlicerLogic.h"

// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAbstractModuleWidgetPrivate: public ctkPrivate<qSlicerAbstractModuleWidget>
{
public:
  QString Name;
  vtkWeakPointer<vtkSlicerLogic>  Logic;
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractModuleWidget, QWidget*);

//-----------------------------------------------------------------------------
CTK_SET_CXX(qSlicerAbstractModuleWidget, const QString&, setName, Name);
CTK_GET_CXX(qSlicerAbstractModuleWidget, QString, name, Name);

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerAbstractModuleWidget::logic()const
{
  CTK_D(const qSlicerAbstractModuleWidget);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setLogic(vtkSlicerLogic* _logic)
{
  // setLogic should be called only one time with valid object...
  Q_ASSERT(_logic);
  CTK_D(qSlicerAbstractModuleWidget);
  d->Logic = _logic; 
}
