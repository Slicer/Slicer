
// Qt includes
#include <QtPlugin>

// CropVolume Logic includes
#include <vtkSlicerCropVolumeLogic.h>

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
