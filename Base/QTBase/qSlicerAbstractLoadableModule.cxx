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
struct qSlicerAbstractLoadableModulePrivate: public qCTKPrivate<qSlicerAbstractLoadableModule>
{
  qSlicerAbstractLoadableModulePrivate()
    {
    this->Name = "NA";
    }
  QString Name;
};

//-----------------------------------------------------------------------------
QCTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractLoadableModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::setup()
{
  Q_ASSERT(qctk_d() != 0);
}

//-----------------------------------------------------------------------------
QCTK_GET_CXX(qSlicerAbstractLoadableModule, QString, name, Name);
QCTK_SET_CXX(qSlicerAbstractLoadableModule, const QString&, setName, Name);

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::helpText()
{
  qDebug() << "qSlicerAbstractLoadableModule::helpText - Not implemented";
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::acknowledgementText()
{
  qDebug() << "qSlicerAbstractLoadableModule::acknowledgementText - Not implemented";
  return QString();
}
