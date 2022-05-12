
// Qt includes
#include <QDebug>

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// CropVolume Logic includes
#include <vtkSlicerCLIModuleLogic.h>
#include <vtkSlicerCropVolumeLogic.h>
#include <vtkSlicerVolumesLogic.h>

// CropVolume includes
#include "qSlicerCropVolumeModule.h"
#include "qSlicerCropVolumeModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_CropVolume
class qSlicerCropVolumeModulePrivate
{
public:
  qSlicerCropVolumeModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModulePrivate::qSlicerCropVolumeModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModule methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModule::qSlicerCropVolumeModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCropVolumeModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModule::~qSlicerCropVolumeModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerCropVolumeModule::helpText()const
{
  return "CropVolume module extracts the subvolume of the image described "
         "by the Region of Interest widget and can also be used to resample the volume.";
}

//-----------------------------------------------------------------------------
QString qSlicerCropVolumeModule::acknowledgementText()const
{
  return "This module was developed by Andrey Fedorov and Ron Kikinis. "
         "This work was supported by NIH grants CA111288 and CA151261, "
         "NA-MIC, NAC and Slicer community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCropVolumeModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andrey Fedorov (BWH, SPL)");
  moduleContributors << QString("Ron Kikinis (BWH, SPL)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerCropVolumeModule::icon()const
{
  return QIcon(":/Icons/CropVolume.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCropVolumeModule::categories()const
{
  return QStringList() << "Converters";
}

//-----------------------------------------------------------------------------
QStringList qSlicerCropVolumeModule::dependencies()const
{
  return QStringList() << "Volumes" << "ResampleScalarVectorDWIVolume";
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerCropVolumeModule::createWidgetRepresentation()
{
  return new qSlicerCropVolumeModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerCropVolumeModule::createLogic()
{
  return vtkSlicerCropVolumeLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerCropVolumeModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLCropVolumeParametersNode";
}
