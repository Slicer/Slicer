#include "qSlicerTractographyFiducialSeedingModule.h"

// SlicerQT includes
#include "qSlicerTractographyFiducialSeedingModuleWidget.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

#include "vtkSlicerTractographyFiducialSeedingLogic.h"
#include "vtkMRMLTractographyFiducialSeedingNode.h"

// QT includes
#include <QtPlugin>
//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTractographyFiducialSeedingModule, qSlicerTractographyFiducialSeedingModule);
//-----------------------------------------------------------------------------
qSlicerTractographyFiducialSeedingModule::
qSlicerTractographyFiducialSeedingModule(QObject* _parent):Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerTractographyFiducialSeedingModule::createWidgetRepresentation()
{
  return new qSlicerTractographyFiducialSeedingModuleWidget;
}
//-----------------------------------------------------------------------------
//
vtkSlicerLogic* qSlicerTractographyFiducialSeedingModule::createLogic()
{
  return 0;
}
