/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/


#include "qSlicerAbstractLoadableModule.h"

#include <QDebug>

//-----------------------------------------------------------------------------
class qSlicerAbstractLoadableModulePrivate: public qCTKPrivate<qSlicerAbstractLoadableModule>
{
public:
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractLoadableModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::setup()
{
  Q_ASSERT(qctk_d() != 0);
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::helpText()const
{
  qDebug() << "qSlicerAbstractLoadableModule::helpText - Not implemented";
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::acknowledgementText()const
{
  qDebug() << "qSlicerAbstractLoadableModule::acknowledgementText - Not implemented";
  return QString();
}
