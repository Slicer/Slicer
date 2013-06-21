/*=auto=========================================================================

 Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH)
 All Rights Reserved.

 See COPYRIGHT.txt
 or http://www.slicer.org/copyright/copyright.txt for details.

 Program:   3D Slicer

=========================================================================auto=*/

// SlicerQt includes
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
  this->Module = 0;
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
  d->Logic = module ? module->logic() : 0;
  this->setup();
}
