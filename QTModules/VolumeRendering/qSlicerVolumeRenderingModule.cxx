
// Qt includes
#include <QtPlugin>

// VolumeRendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

// VolumeRendering includes
#include "qSlicerVolumeRenderingModule.h"
#include "qSlicerVolumeRenderingModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumeRenderingModule, qSlicerVolumeRenderingModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_VolumeRendering
class qSlicerVolumeRenderingModulePrivate
{
public:
  qSlicerVolumeRenderingModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModulePrivate::qSlicerVolumeRenderingModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModule methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModule::qSlicerVolumeRenderingModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumeRenderingModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModule::~qSlicerVolumeRenderingModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerVolumeRenderingModule::icon()const
{
  return QIcon(":/Icons/VolumeRendering.png");
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModule::setup()
{
  this->Superclass::setup();
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->
    RegisterDisplayableManager("vtkMRMLVolumeRenderingDisplayableManager");
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerVolumeRenderingModule::createWidgetRepresentation()
{
  return new qSlicerVolumeRenderingModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVolumeRenderingModule::createLogic()
{
  return vtkSlicerVolumeRenderingLogic::New();
}
