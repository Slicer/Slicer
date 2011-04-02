
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
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerCropVolumeModule::acknowledgementText()const
{
  return "This work was supported by ...";
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
