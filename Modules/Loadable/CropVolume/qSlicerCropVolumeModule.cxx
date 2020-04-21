
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
  return "CropVolume module extracts subvolume of the image described "
         "by Region of Interest widget.";
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

  vtkSlicerCropVolumeLogic* cropVolumeLogic =
    vtkSlicerCropVolumeLogic::SafeDownCast(this->logic());

  qSlicerAbstractCoreModule* volumesModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Volumes");
  if (volumesModule)
    {
    vtkSlicerVolumesLogic* volumesLogic =
      vtkSlicerVolumesLogic::SafeDownCast(volumesModule->logic());
    cropVolumeLogic->SetVolumesLogic(volumesLogic);
    }
  else
    {
    qWarning() << "Volumes module is not found";
    }

  qSlicerAbstractCoreModule* resampleModule =
    qSlicerCoreApplication::application()->moduleManager()->module("ResampleScalarVectorDWIVolume");
  if (resampleModule)
    {
    vtkSlicerCLIModuleLogic* resampleLogic =
      vtkSlicerCLIModuleLogic::SafeDownCast(resampleModule->logic());
    cropVolumeLogic->SetResampleLogic(resampleLogic);
    }
  else
    {
    qWarning() << "ResampleScalarVectorDWIVolume module is not found";
    }
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
