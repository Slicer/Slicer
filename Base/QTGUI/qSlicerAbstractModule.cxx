/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>
#include <QAction>

// SlicerQt includes
#include "qSlicerAbstractModule.h"

//-----------------------------------------------------------------------------
qSlicerAbstractModule::qSlicerAbstractModule(QObject* parentObject)
  :Superclass(parentObject)
{

}

//-----------------------------------------------------------------------------
QIcon qSlicerAbstractModule::icon()const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QAction* qSlicerAbstractModule::createAction()
{
  QAction* action = new QAction(this->icon(), this->title(), this);
  action->setData(this->name());
  action->setIconVisibleInMenu(true);
  return action;
}
