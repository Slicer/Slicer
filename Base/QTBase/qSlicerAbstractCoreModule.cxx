/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerAbstractCoreModule.h"

#include <QDebug>

//-----------------------------------------------------------------------------
struct qSlicerAbstractCoreModulePrivate: public qCTKPrivate<qSlicerAbstractCoreModule>
{
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractCoreModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractCoreModule::setup()
{
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractCoreModule::name()const
{
  return this->metaObject()->className();
}
