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
