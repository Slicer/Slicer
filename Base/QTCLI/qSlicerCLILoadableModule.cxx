/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) 
 All Rights Reserved.

 See Doc/copyright/copyright.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

#include "qSlicerCLILoadableModule.h"

// QT includes
#include <QTextStream>

//-----------------------------------------------------------------------------
class qSlicerCLILoadableModulePrivate: public qCTKPrivate<qSlicerCLILoadableModule>
{
public:
  QCTK_DECLARE_PUBLIC(qSlicerCLILoadableModule);
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
  QCTK_INIT_PRIVATE(qSlicerCLILoadableModule);
}

//-----------------------------------------------------------------------------
void qSlicerCLILoadableModule::setup()
{
  Superclass::setup();
}

//-----------------------------------------------------------------------------
QString qSlicerCLILoadableModule::entryPoint()
{
  QCTK_D(qSlicerCLILoadableModule);
  QString str;
  QTextStream(&str) << "slicer:" << (void*)(d->EntryPoint);
  return str; 
}

//-----------------------------------------------------------------------------
QCTK_SET_CXX(qSlicerCLILoadableModule, ModuleEntryPointType, setEntryPoint, EntryPoint);
