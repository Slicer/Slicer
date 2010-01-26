#include "qSlicerTractographyFiducialSeedingModule.h"

// SlicerQT includes
#include "qSlicerTractographyFiducialSeedingModuleWidget.h"

// QT includes
#include <QtPlugin>
//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTractographyFiducialSeedingModule, qSlicerTractographyFiducialSeedingModule);
//-----------------------------------------------------------------------------
qSlicerTractographyFiducialSeedingModule::qSlicerTractographyFiducialSeedingModule(QObject* parent) 
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleWidget* qSlicerTractographyFiducialSeedingModule::createWidgetRepresentation()
{
  return new qSlicerTractographyFiducialSeedingModuleWidget;
}
//-----------------------------------------------------------------------------
//
qSlicerModuleLogic* qSlicerTractographyFiducialSeedingModule::createLogic()
{
  return 0;
}
