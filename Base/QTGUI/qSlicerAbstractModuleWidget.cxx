/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::qSlicerAbstractModuleWidget(QWidget* parentWidget)
  :qSlicerWidget(parentWidget)
{
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setName(const QString& name)
{
  this->qSlicerAbstractModuleRepresentation::setName(name);
  // better than nothing but still not that...
  this->setWindowTitle(name);
}

//-----------------------------------------------------------------------------
QAction* qSlicerAbstractModuleWidget::showModuleAction()
{
  return 0;
}
