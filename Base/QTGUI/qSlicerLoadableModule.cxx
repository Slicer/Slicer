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
#include "qSlicerLoadableModule.h"

//-----------------------------------------------------------------------------
class qSlicerLoadableModulePrivate: public ctkPrivate<qSlicerLoadableModule>
{
public:
};

//-----------------------------------------------------------------------------
CTK_CONSTRUCTOR_1_ARG_CXX(qSlicerLoadableModule, QObject*);

//-----------------------------------------------------------------------------
void qSlicerLoadableModule::setup()
{
  Q_ASSERT(ctk_d() != 0);
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModule::helpText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::helpText() is not implemented";
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModule::acknowledgementText()const
{
  qDebug() << "WARNING: " << this->metaObject()->className()
           << "::acknowledgementText - Not implemented";
  return QString();
}
