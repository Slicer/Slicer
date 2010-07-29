/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QAction>
#include <QIcon>

// SlicerQt includes
#include "qSlicerAbstractModule.h"
#include "qSlicerAbstractModuleWidget.h"

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget::qSlicerAbstractModuleWidget(QWidget* parentWidget)
  :qSlicerWidget(parentWidget)
{
}

//-----------------------------------------------------------------------------
QAction* qSlicerAbstractModuleWidget::createAction()
{
  QAction* action = new QAction(this->windowIcon(), this->module()->title(), this);
  action->setData(this->module()->name());
  return action;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleWidget::setup()
{
  this->setWindowTitle(module()->title());
}
