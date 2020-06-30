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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerModuleManager.h"

// Data includes
#include "qSlicerDataDialog.h"
#include "qSlicerDataModule.h"
#include "qSlicerDataModuleWidget.h"
#include "qSlicerSaveDataDialog.h"
#include "qSlicerSceneBundleReader.h"
#include "qSlicerSceneReader.h"
#include "qSlicerSceneWriter.h"
#include "qSlicerSlicer2SceneReader.h"

// SlicerLogic includes
#include <vtkSlicerApplicationLogic.h>

// Data Logic includes
#include "vtkSlicerDataModuleLogic.h"

// Logic includes
#include <vtkSlicerCamerasModuleLogic.h>

// VTK includes
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
class qSlicerDataModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerDataModule::qSlicerDataModule(QObject* parentObject)
  :Superclass(parentObject)
  , d_ptr(new qSlicerDataModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDataModule::~qSlicerDataModule() = default;

//-----------------------------------------------------------------------------
QIcon qSlicerDataModule::icon()const
{
  return QIcon(":/Icons/SubjectHierarchy.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerDataModule::categories() const
{
  return QStringList() << "" << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDataModule::dependencies() const
{
  QStringList moduleDependencies;
  // Cameras: Required in qSlicerSceneReader
  moduleDependencies << "Cameras";
  return moduleDependencies;
}

//-----------------------------------------------------------------------------
void qSlicerDataModule::setup()
{
  this->Superclass::setup();

  qSlicerAbstractCoreModule* camerasModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Cameras");
  vtkSlicerCamerasModuleLogic* camerasLogic =
    vtkSlicerCamerasModuleLogic::SafeDownCast(camerasModule ? camerasModule->logic() : nullptr);

  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();

  // Readers
  ioManager->registerIO(new qSlicerSceneReader(camerasLogic, this));
  ioManager->registerIO(new qSlicerSceneBundleReader(this));
  ioManager->registerIO(new qSlicerSlicer2SceneReader(this->appLogic(), this));

  // Writers
  ioManager->registerIO(new qSlicerSceneWriter(this));

  // Dialogs
  ioManager->registerDialog(new qSlicerDataDialog(this));
  ioManager->registerDialog(new qSlicerSaveDataDialog(this));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerDataModule::createWidgetRepresentation()
{
  qSlicerDataModuleWidget *widget = new qSlicerDataModuleWidget;
  return widget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerDataModule::createLogic()
{
  return vtkSlicerDataModuleLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerDataModule::helpText()const
{
  QString help = QString(
    "The Data module is the central data-organizing point where all loaded data is "
    "presented for access and manipulation is the Data module. It allows organizing "
    "the data in folders or patient/study trees (automatically done for DICOM), "
    "visualizing any displayable data, transformation of whole branches, and a "
    "multitude of data type specific features.");
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerDataModule::acknowledgementText()const
{
  QString about =
    "<center><table border=\"0\"><tr>"
    "<td><img src=\":Logos/NAMIC.png\" alt\"NA-MIC\"></td>"
    "<td><img src=\":Logos/NAC.png\" alt\"NAC\"></td>"
    "</tr><tr>"
    "<td><img src=\":Logos/BIRN-NoText.png\" alt\"BIRN\"></td>"
    "<td><img src=\":Logos/NCIGT.png\" alt\"NCIGT\"></td>"
    "</tr></table></center>"
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, CTSC, and the Slicer "
    "Community.";
  return about;
}

//-----------------------------------------------------------------------------
QStringList qSlicerDataModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Csaba Pinter (Queen's)");
  moduleContributors << QString("Julien Finet (Kitware)");
  moduleContributors << QString("Alex Yarmarkovich (Isomics)");
  moduleContributors << QString("Nicole Aucoin (SPL, BWH)");
  return moduleContributors;
}
