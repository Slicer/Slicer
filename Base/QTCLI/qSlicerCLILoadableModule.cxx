/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QTextStream>

// SlicerQt includes
#include "qSlicerCLILoadableModule.h"

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModulePrivate: public ctkPrivate<qSlicerCLILoadableModule>
{
public:
  CTK_DECLARE_PUBLIC(qSlicerCLILoadableModule);
  typedef qSlicerCLILoadableModulePrivate Self;
  qSlicerCLILoadableModulePrivate();

  qSlicerCLILoadableModule::ModuleEntryPointType EntryPoint; 
};

//-----------------------------------------------------------------------------
// qSlicerCLILoadableModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCLILoadableModulePrivate::qSlicerCLILoadableModulePrivate()
{
  this->EntryPoint = 0; 
}

//-----------------------------------------------------------------------------
// qSlicerCLILoadableModule methods

//-----------------------------------------------------------------------------
qSlicerCLILoadableModule::qSlicerCLILoadableModule(QWidget* _parent):Superclass(_parent)
{
  CTK_INIT_PRIVATE(qSlicerCLILoadableModule);
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModule::setup()
{
  Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::entryPoint()const
{
  CTK_D(const qSlicerCLILoadableModule);
  QString str;
  QTextStream(&str) << "slicer:" << (void*)(d->EntryPoint);
  return str; 
}

//-----------------------------------------------------------------------------
CTK_SET_CXX(qSlicerCLILoadableModule, ModuleEntryPointType, setEntryPoint, EntryPoint);

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::moduleType()const
{
  return QLatin1String("SharedObjectModule");
}
