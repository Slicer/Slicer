
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
  QString help =
    "Volume Rendering allows the rendering of volumes in 3D space and not only "
    "as 2D surfaces defined in 3D space. \n"
    "<a href=\"%1/Documentation/4.0/Modules/VolumeRendering\">"
    "%1/Documentation/4.0/Modules/VolumeRendering</a>\n"
    "Tutorials are available at <a href=\"%1/Documentation/4.0/Modules/VolumeRendering\">"
    "%1/Documentation/4.0/Modules/VolumeRendering</a>";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingModule::acknowledgementText()const
{
  QString acknowledgement =
    "<center><table border=\"0\"><tr>"
    "<td><img src=\":Logos/NAMIC.png\" alt\"NA-MIC\"></td>"
    "<td><img src=\":Logos/NAC.png\" alt\"NAC\"></td>"
    "</tr><tr>"
    "<td><img src=\":Logos/BIRN-NoText.png\" alt\"BIRN\"></td>"
    "<td><img src=\":Logos/NCIGT.png\" alt\"NCIGT\"></td>"
    "</tr></table></center>"
    "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. "
    "See <a href=\"http://www.slicer.org\">slicer.org</a> for details.<br/>"
    "Some of the transfer functions were contributed by Kitware Inc. (VolView)"
    ;
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeRenderingModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Julien Finet (Kitware Inc.)");
  moduleContributors << QString("Alex Yarmarkovich (Isomics Inc.)");
  moduleContributors << QString("Yanling Liu (NCI-Frederick/SAIC-Frederick)");
  moduleContributors << QString("Andreas Freudling (Student Intern at SPL)");
  moduleContributors << QString("Ron Kikinis (BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVolumeRenderingModule::icon()const
{
  return QIcon(":/Icons/VolumeRendering.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeRenderingModule::categories() const
{
  return QStringList() << "";
}

//-----------------------------------------------------------------------------
void qSlicerVolumeRenderingModule::setup()
{
  this->Superclass::setup();
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->
    RegisterDisplayableManager("vtkMRMLVolumeRenderingDisplayableManager");

  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  if (qSlicerApplication::application())
    {
    qSlicerVolumeRenderingSettingsPanel* panel =
      new qSlicerVolumeRenderingSettingsPanel;
    panel->setVolumeRenderingLogic(volumeRenderingLogic);
    qSlicerApplication::application()->settingsDialog()->addPanel(
      "Volume rendering", panel);
    }
  qSlicerCoreApplication::application()->coreIOManager()->registerIO(
    new qSlicerVolumeRenderingIO(volumeRenderingLogic, this));
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
