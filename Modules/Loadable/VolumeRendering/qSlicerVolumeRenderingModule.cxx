/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Alex Yarmakovich, Isomics Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QtGUI includes
#include <qSlicerApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// VolumeRendering Logic includes
#include <vtkSlicerVolumeRenderingLogic.h>
#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>

// VolumeRendering includes
#include "qSlicerVolumeRenderingModule.h"
#include "qSlicerVolumeRenderingModuleWidget.h"
#include "qSlicerVolumeRenderingReader.h"
#include "qSlicerShaderPropertyReader.h"
#include "qSlicerVolumeRenderingSettingsPanel.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyVolumeRenderingPlugin.h"

// DisplayableManager initialization
#include <vtkAutoInit.h>
VTK_MODULE_INIT(vtkSlicerVolumeRenderingModuleMRMLDisplayableManager)

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
qSlicerVolumeRenderingModulePrivate::qSlicerVolumeRenderingModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerVolumeRenderingModule methods

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModule::qSlicerVolumeRenderingModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerVolumeRenderingModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerVolumeRenderingModule::~qSlicerVolumeRenderingModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerVolumeRenderingModule::helpText()const
{
  QString help = QString(
    "Volume Rendering Module provides advanced tools for toggling interactive "
    "volume rendering of datasets.<br/>"
    "If supported, hardware accelerated volume rendering is made available."
    "The module permits selection of preset transfer functions to colorize and set opacity "
    "of data in a task-appropriate way, and tools to customize the transfer functions that specify "
    "these parameters.<br/>"
    "For more information and tutorials see the <a href=\"%1/Documentation/%2.%3/Modules/VolumeRendering\">"
    "online documentation</a><br/>");
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
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
    "This work is supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community."
    "Some of the transfer functions were contributed by Kitware Inc. (VolView)";
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeRenderingModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Julien Finet (Kitware)");
  moduleContributors << QString("Alex Yarmarkovich (Isomics)");
  moduleContributors << QString("Yanling Liu (SAIC-Frederick, NCI-Frederick)");
  moduleContributors << QString("Andreas Freudling (SPL, BWH)");
  moduleContributors << QString("Ron Kikinis (SPL, BWH)");
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
  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->RegisterDisplayableManager(
    "vtkMRMLVolumeRenderingDisplayableManager");

  vtkSlicerVolumeRenderingLogic* volumeRenderingLogic =
    vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  if (qSlicerApplication::application())
    {
    qSlicerVolumeRenderingSettingsPanel* panel = new qSlicerVolumeRenderingSettingsPanel;
    qSlicerApplication::application()->settingsDialog()->addPanel("Volume rendering", panel);
    panel->setVolumeRenderingLogic(volumeRenderingLogic);
    }

  // Register VolumeProperty reader/writer
  qSlicerCoreIOManager* coreIOManager = qSlicerCoreApplication::application()->coreIOManager();
  coreIOManager->registerIO(new qSlicerVolumeRenderingReader(volumeRenderingLogic, this));
  coreIOManager->registerIO(new qSlicerNodeWriter("Transfer Function", QString("TransferFunctionFile"),
    QStringList() << "vtkMRMLVolumePropertyNode", true, this));

  // Register ShaderProperty reader/writer
  coreIOManager->registerIO(new qSlicerShaderPropertyReader(volumeRenderingLogic,this));
  coreIOManager->registerIO(new qSlicerNodeWriter("Shader Property", QString("ShaderPropertyFile"),
    QStringList() << "vtkMRMLShaderPropertyNode", true, this ));

  // Register Subject Hierarchy core plugins
  vtkSlicerVolumeRenderingLogic* logic = vtkSlicerVolumeRenderingLogic::SafeDownCast(this->logic());
  qSlicerSubjectHierarchyVolumeRenderingPlugin* shPlugin = new qSlicerSubjectHierarchyVolumeRenderingPlugin();
  shPlugin->setVolumeRenderingLogic(logic);
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(shPlugin);
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

//-----------------------------------------------------------------------------
QStringList qSlicerVolumeRenderingModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkMRMLVolumePropertyNode"
    << "vtkMRMLShaderPropertyNode"
    << "vtkMRMLVolumeRenderingDisplayNode"
    << "vtkMRMLAnnotationROINode"; // volume rendering clipping box
}
