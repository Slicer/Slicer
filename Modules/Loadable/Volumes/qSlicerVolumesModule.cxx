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

// Slicer includes
#include <qSlicerCoreApplication.h>
#include <qSlicerIOManager.h>
#include <qSlicerModuleManager.h>
#include <qSlicerNodeWriter.h>

// Volumes Logic includes
#include <vtkSlicerVolumesLogic.h>

// Volumes QTModule includes
#include "qSlicerVolumesReader.h"
#include "qSlicerVolumesModule.h"
#include "qSlicerVolumesModuleWidget.h"

// MRML Logic includes
#include <vtkMRMLColorLogic.h>

// MRML includes
#include <vtkMRMLScene.h>

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyVolumesPlugin.h"
#include "qSlicerSubjectHierarchyLabelMapsPlugin.h"
#include "qSlicerSubjectHierarchyDiffusionTensorVolumesPlugin.h"

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
qSlicerVolumesModule::~qSlicerVolumesModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerVolumesModule::helpText()const
{
  QString help = QString(
    "The Volumes Module is the interface for adjusting Window, Level, Threshold, "
    "Color LUT and other parameters that control the display of volume image data "
    "in the scene.<br>"
    "For more information see the <a href=\"%1/Documentation/%2.%3/Modules/Volumes\">"
    "online documentation</a>.<br>");
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
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.");
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
  moduleContributors << QString("Ron Kikinis (SPL, BWH)");
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
  moduleDependencies << "Colors" << "Units";
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
  ioManager->registerIO(new qSlicerVolumesReader(volumesLogic,this));
  ioManager->registerIO(new qSlicerNodeWriter(
    "Volumes", QString("VolumeFile"),
    QStringList() << "vtkMRMLVolumeNode", true, this));

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyVolumesPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyLabelMapsPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyDiffusionTensorVolumesPlugin());
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

//-----------------------------------------------------------------------------
QStringList qSlicerVolumesModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkMRMLVolumeNode"
    << "vtkMRMLVolumeDisplayNode";
}
