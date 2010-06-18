/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>

// SlicerQt includes
#include "qSlicerAbstractLoadableModule.h"

//-----------------------------------------------------------------------------
class qSlicerAbstractLoadableModulePrivate: public ctkPrivate<qSlicerAbstractLoadableModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerAbstractLoadableModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerAbstractLoadableModule::setup()
{
  Q_ASSERT(ctk_d() != 0);
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::helpText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::helpText() is not implemented";
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractLoadableModule::acknowledgementText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::acknowledgementText - Not implemented";
  return QString();
}
