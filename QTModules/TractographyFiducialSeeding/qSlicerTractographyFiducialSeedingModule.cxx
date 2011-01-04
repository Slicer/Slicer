// QT includes
#include <QtPlugin>

// Tractography Logic includes
#include "vtkSlicerTractographyFiducialSeedingLogic.h"

// Tractography QTModule includes
#include "qSlicerTractographyFiducialSeedingModule.h"
#include "qSlicerTractographyFiducialSeedingModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTractographyFiducialSeedingModule, qSlicerTractographyFiducialSeedingModule);

//-----------------------------------------------------------------------------
qSlicerTractographyFiducialSeedingModule::
qSlicerTractographyFiducialSeedingModule(QObject* _parent):Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTractographyFiducialSeedingModule::createWidgetRepresentation()
{
  return new qSlicerTractographyFiducialSeedingModuleWidget;
}
//-----------------------------------------------------------------------------
//
vtkMRMLAbstractLogic* qSlicerTractographyFiducialSeedingModule::createLogic()
{
  return vtkSlicerTractographyFiducialSeedingLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyFiducialSeedingModule::helpText()const
{
  QString help =
    "The Tractography Fiducial Seeding Module creates fiber tracts at specified seeding location. \n"
    "<a href=%1/Modules:FiducialSeeding-Documentation-3.4>%1/Modules:FiducialSeeding-Documentation-3.4</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyFiducialSeedingModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. "
    "See <a href=\"http://www.slicer.org\">http://www.slicer.org</a> for details.\n"
    "The FiducialSeeding module was contributed by Alex Yarmarkovich, Isomics Inc. with "
    "help from others at SPL, BWH (Ron Kikinis)";
  return acknowledgement;
}
