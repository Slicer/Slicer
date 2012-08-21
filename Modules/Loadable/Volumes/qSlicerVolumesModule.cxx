/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Qt includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerIOManager.h>
#include <qSlicerModuleManager.h>
#include <qSlicerNodeWriter.h>

// Volumes Logic includes
#include <vtkSlicerVolumesLogic.h>

// Volumes QTModule includes
#include "qSlicerVolumesIO.h"
#include "qSlicerVolumesModule.h"
#include "qSlicerVolumesModuleWidget.h"

// MRML Logic includes
#include <vtkMRMLColorLogic.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerVolumesModule, qSlicerVolumesModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Volumes
class qSlicerVolumesModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerVolumesModule::qSlicerVolumesModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumesModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumesModule::~qSlicerVolumesModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesModule::helpText()const
{
  QString help = QString(
    "The Volumes Module loads and adjusts display parameters of volume data.<br>"
    "<a href=\"%1/Documentation/%2.%3/Modules/Volumes\">"
    "%1/Documentation/%2.%3/Modules/Volumes</a><br>"
    "The Diffusion Editor allows modifying "
    "parameters (gradients, bValues, measurement frame) of DWI data and "
    "provides a quick way to interpret them. "
    "For that it estimates a tensor and shows glyphs and tracts "
    "for visual exploration.<br><br>"
    "Help for Diffusion Editor: "
    "<a href=\"%1/Modules:Volumes:Diffusion_Editor-Documentation\">"
    "%1/Modules:Volumes:Diffusion_Editor-Documentation</a>");
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerVolumesModule::acknowledgementText()const
{
  QString acknowledgement = QString(
    "<center><table border=\"0\"><tr>"
    "<td><img src=\":Logos/NAMIC.png\" alt\"NA-MIC\"></td>"
    "<td><img src=\":Logos/NAC.png\" alt\"NAC\"></td>"
    "</tr><tr>"
    "<td><img src=\":Logos/BIRN-NoText.png\" alt\"BIRN\"></td>"
    "<td><img src=\":Logos/NCIGT.png\" alt\"NCIGT\"></td>"
    "</tr></table></center>"
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer "
    "Community. See <a href=\"http://www.slicer.org\">http://www.slicer.org"
    "</a> for details.<br>"
    "The Volumes module was contributed by Alex Yarmarkovich, Isomics Inc. "
    "(Steve Pieper) and Julien Finet, Kitware Inc. with help from others at "
    "SPL, BWH (Ron Kikinis).<br><br>"
    "The Diffusion Editor was developed by Kerstin Kessel.");
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumesModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Steve Pieper (Isomics)");
  moduleContributors << QString("Julien Finet (Kitware)");
  moduleContributors << QString("Alex Yarmarkovich (Isomics)");
  moduleContributors << QString("Nicole Aucoin (SPL, BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerVolumesModule::icon()const
{
  return QIcon(":/Icons/Medium/SlicerVolumes.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumesModule::categories() const
{
  return QStringList() << "";
}


//-----------------------------------------------------------------------------
QStringList qSlicerVolumesModule::dependencies() const
{
  QStringList moduleDependencies;
  moduleDependencies << "Colors";
  return moduleDependencies;
}

//-----------------------------------------------------------------------------
void qSlicerVolumesModule::setup()
{
  this->Superclass::setup();
  vtkSlicerVolumesLogic* volumesLogic =
    vtkSlicerVolumesLogic::SafeDownCast(this->logic());
  qSlicerAbstractCoreModule* colorsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Colors");
  if (colorsModule)
    {
    vtkMRMLColorLogic* colorLogic =
      vtkMRMLColorLogic::SafeDownCast(colorsModule->logic());
    volumesLogic->SetColorLogic(colorLogic);
    }

  qSlicerCoreIOManager* ioManager =
    qSlicerCoreApplication::application()->coreIOManager();
  ioManager->registerIO(new qSlicerVolumesIO(volumesLogic,this));
  ioManager->registerIO(new qSlicerNodeWriter(
    "Volumes", QString("VolumeFile"),
    QStringList() << "vtkMRMLVolumeNode", this));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVolumesModule::createWidgetRepresentation()
{
  return new qSlicerVolumesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVolumesModule::createLogic()
{
  return vtkSlicerVolumesLogic::New();
}
