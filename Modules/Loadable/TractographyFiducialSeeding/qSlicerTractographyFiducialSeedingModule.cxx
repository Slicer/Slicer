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
QStringList qSlicerTractographyFiducialSeedingModule::categories()const
{
  return QStringList() << "Diffusion.Diffusion Tensor Images";
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyFiducialSeedingModule::helpText()const
{
  QString help =
    "The Tractography Fiducial Seeding Module creates fiber tracts at specified seeding location. \n"
    "<a href=%1/Documentation/%2.%3/Modules/TractographyFiducialSeeding>%1/Documentation/%2.%3/Modules/TractographyFiducialSeeding</a>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
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

//-----------------------------------------------------------------------------
QStringList qSlicerTractographyFiducialSeedingModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Alex Yarmakovich (Isomics)");
  return moduleContributors;
}
