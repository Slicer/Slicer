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

// Qt includes
#include <QtPlugin>

// Models includes
#include "qSlicerModelsModule.h"
#include "qSlicerModelsModuleWidget.h"
#include "qSlicerModelsIO.h"
#include "qSlicerScalarOverlayIO.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerIOManager.h"
#include "qSlicerModelsDialog.h"
#include "qSlicerModuleManager.h"

// Slicer logic includes
#include <vtkSlicerApplicationLogic.h>
#include <vtkSlicerModelsLogic.h>

// MRML includes
#include "vtkMRMLColorLogic.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerModelsModule, qSlicerModelsModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Models
class qSlicerModelsModulePrivate
{
public:
  qSlicerModelsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerModelsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerModelsModulePrivate::qSlicerModelsModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerModelsModule methods

//-----------------------------------------------------------------------------
qSlicerModelsModule::qSlicerModelsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModelsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerModelsModule::~qSlicerModelsModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerModelsModule::helpText()const
{
  QString help =
    "The Models Module loads and adjusts display parameters of models.<br>"
    "For more information see <a href=\"%1/Documentation/4.0/Modules/Models\">%1/Documentation/4.0/Modules/Models</a><br>"
    "Save models via the File menu, Save button.<br>"
    "The Add 3D model or a model directory button will allow you to load any "
    "model that Slicer can read, as well as all the VTK models in a directory. "
    "Add Scalar Overlay will load a scalar file and associate it with the "
    "currently active model.<br>You can adjust the display properties of the "
    "models in the Display pane. Select the model you wish to work on from the "
    "model selector drop down menu. Scalar overlays are loaded with a default "
    "colour look up table, but can be reassigned manually. Once a new scalar "
    "overlay is chosen, currently the old color map is still used, so that "
    "must be adjusted in conjunction with the overlay.<br>"
    "Clipping is turned on for a model in the Display pane, and the slice "
    "planes that will clip the model are selected in the Clipping pane.<br>"
    "The Model Hierarchy pane allows you to group models together and set the "
    "group's properties.";
  return help.arg(this->slicerWikiUrl());
}

//-----------------------------------------------------------------------------
QString qSlicerModelsModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Julien Finet (Kitware)");
  moduleContributors << QString("Alex Yarmakovich (Isomics)");
  moduleContributors << QString("Nicole Aucoin (SPL, BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerModelsModule::icon()const
{
  return QIcon(":/Icons/Large/SlicerModels.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsModule::categories() const
{
  return QStringList() << "";
}

//-----------------------------------------------------------------------------
QStringList qSlicerModelsModule::dependencies() const
{
  QStringList moduleDependencies;
  moduleDependencies << "Colors";
  return moduleDependencies;
}

//-----------------------------------------------------------------------------
void qSlicerModelsModule::setup()
{
  this->Superclass::setup();
  // Configure models logic
  vtkSlicerModelsLogic* modelsLogic =
    vtkSlicerModelsLogic::SafeDownCast(this->logic());
  qSlicerAbstractCoreModule* colorsModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Colors");
  if (colorsModule)
    {
    vtkMRMLColorLogic* colorLogic =
      vtkMRMLColorLogic::SafeDownCast(colorsModule->logic());
    modelsLogic->SetColorLogic(colorLogic);
    }
  // Register IOs
  qSlicerIOManager* ioManager = qSlicerApplication::application()->ioManager();
  ioManager->registerIO(new qSlicerModelsIO(modelsLogic, this));
  ioManager->registerIO(new qSlicerScalarOverlayIO(modelsLogic, this));
  ioManager->registerDialog(new qSlicerModelsDialog(this));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerModelsModule::createWidgetRepresentation()
{
  qSlicerModelsModuleWidget* widget = new qSlicerModelsModuleWidget;
  return widget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerModelsModule::createLogic()
{
  return vtkSlicerModelsLogic::New();
}
