
// Qt includes
#include <QtPlugin>

// QtGUI includes
#include <qSlicerApplication.h>
#include <qSlicerCoreIOManager.h>

// VolumeRendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

// VolumeRendering includes
#include "qSlicerVolumeRenderingIO.h"
#include "qSlicerVolumeRenderingModule.h"
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "qSlicerVolumeRenderingSettingsPanel.h"

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
  if (qSlicerApplication::application())
    {
    qSlicerApplication::application()->settingsDialog()->addPanel(
      "Volume rendering", new qSlicerVolumeRenderingSettingsPanel);
    }
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->
    RegisterDisplayableManager("vtkMRMLVolumeRenderingDisplayableManager");

  qSlicerCoreApplication::application()->coreIOManager()->registerIO(
    new qSlicerVolumeRenderingIO(this));
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
