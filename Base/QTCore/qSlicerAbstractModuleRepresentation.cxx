/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerAbstractCoreModule.h"
#include "qSlicerAbstractModuleRepresentation.h"

// MRMLLogic includes
#include "vtkMRMLAbstractLogic.h"

// VTK includes
#include <vtkWeakPointer.h>

//-----------------------------------------------------------------------------
class qSlicerAbstractModuleRepresentationPrivate
{
public:
  qSlicerAbstractModuleRepresentationPrivate();
  vtkWeakPointer<vtkMRMLAbstractLogic> Logic;
  qSlicerAbstractCoreModule*           Module;
};

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentationPrivate
::qSlicerAbstractModuleRepresentationPrivate()
{
  this->Module = nullptr;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation::qSlicerAbstractModuleRepresentation()
  : d_ptr(new qSlicerAbstractModuleRepresentationPrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation::~qSlicerAbstractModuleRepresentation()
{
  Q_D(qSlicerAbstractModuleRepresentation);
  if (d->Module)
    {
    d->Module->representationDeleted(this);
    }
}

//-----------------------------------------------------------------------------
QString qSlicerAbstractModuleRepresentation::moduleName()const
{
  Q_D(const qSlicerAbstractModuleRepresentation);
  return d->Module->name();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerAbstractModuleRepresentation::logic()const
{
  Q_D(const qSlicerAbstractModuleRepresentation);
  return d->Logic.GetPointer();
}

//-----------------------------------------------------------------------------
qSlicerAbstractCoreModule* qSlicerAbstractModuleRepresentation::module()const
{
  Q_D(const qSlicerAbstractModuleRepresentation);
  return d->Module;
}

//-----------------------------------------------------------------------------
void qSlicerAbstractModuleRepresentation::setModule(qSlicerAbstractCoreModule* module)
{
  Q_D(qSlicerAbstractModuleRepresentation);
  d->Module = module;
  d->Logic = module ? module->logic() : nullptr;
  this->setup();
}

//-----------------------------------------------------------
bool qSlicerAbstractModuleRepresentation::setEditedNode(vtkMRMLNode* node,
                                                        QString role /* = QString()*/,
                                                        QString context /* = QString() */)
{
  Q_UNUSED(node);
  Q_UNUSED(role);
  Q_UNUSED(context);
  qWarning() << Q_FUNC_INFO << " failed: method is not implemented in " << this->moduleName();
  return false;
}

//-----------------------------------------------------------
double qSlicerAbstractModuleRepresentation::nodeEditable(vtkMRMLNode* node)
{
  Q_UNUSED(node);
  // It is assumed that only associated nodes will be tried to be edited,
  // so most of the time using the recommended neutral confidence value is
  // reasonable. If a module is more or less confident than default
  // then that module has to override this method.
  return 0.5;
}
