
// Qt includes
#include <QtPlugin>

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>

// CropVolume Logic includes
#include <vtkSlicerCropVolumeLogic.h>
#include <vtkSlicerVolumesLogic.h>

// CropVolume includes
#include "qSlicerCropVolumeModule.h"
#include "qSlicerCropVolumeModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerCropVolumeModule, qSlicerCropVolumeModule);

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
qSlicerCropVolumeModulePrivate::qSlicerCropVolumeModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerCropVolumeModule methods

//-----------------------------------------------------------------------------
qSlicerCropVolumeModule::qSlicerCropVolumeModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerCropVolumeModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerCropVolumeModule::~qSlicerCropVolumeModule()
{
}

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
QIcon qSlicerCropVolumeModule::icon()const
{
  return QIcon(":/Icons/CropVolume.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerCropVolumeModule::dependencies()const
{
  return QStringList(QString("Volumes"));
}

//-----------------------------------------------------------------------------
void qSlicerCropVolumeModule::setup()
{
  this->Superclass::setup();

  qSlicerAbstractCoreModule* volumesModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Volumes");
  vtkSlicerVolumesLogic* volumesLogic = 
    vtkSlicerVolumesLogic::SafeDownCast(volumesModule->logic());
  vtkSlicerCropVolumeLogic* cropVolumeLogic =
    vtkSlicerCropVolumeLogic::SafeDownCast(this->logic());
  cropVolumeLogic->SetVolumesLogic(volumesLogic);
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
