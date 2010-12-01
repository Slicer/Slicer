/*==============================================================================

  Program: 3D Slicer

  Copyright (c) 2010 Kitware Inc.

  See Doc/copyright/copyright.txt
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

// Slicer includes
#include "qSlicerCoreApplication.h"
#include "qSlicerCoreIOManager.h"

// Slicer logic includes
#include <vtkSlicerModelsLogic.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerModelsModule, qSlicerModelsModule);

//-----------------------------------------------------------------------------
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
    "The Models Module loads and adjusts display parameters of models.\n"
    "<a>%1/Modules:Models-Documentation-3.6</a>\n"
    "Save models via the File menu, Save button.\n"
    "The Add 3D model or a model directory button will allow you to load any "
    "model that Slicer can read, as well as all the VTK models in a directory. "
    "Add Scalar Overlay will load a scalar file and associate it with the "
    "currently active model.\nYou can adjust the display properties of the "
    "models in the Display pane. Select the model you wish to work on from the "
    "model selector drop down menu. Scalar overlays are loaded with a default "
    "colour look up table, but can be reassigned manually. Once a new scalar "
    "overlay is chosen, currently the old color map is still used, so that "
    "must be adjusted in conjunction with the overlay.\n"
    "Clipping is turned on for a model in the Display pane, and the slice "
    "planes that will clip the model are selected in the Clipping pane.\n"
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
QIcon qSlicerModelsModule::icon()const
{
  return QIcon(":/Icons/Large/SlicerModels.png");
}

//-----------------------------------------------------------------------------
void qSlicerModelsModule::setup()
{
  this->Superclass::setup();
  qSlicerCoreApplication::application()->coreIOManager()->registerIO(
    new qSlicerModelsIO(this));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerModelsModule::createWidgetRepresentation()
{
  return new qSlicerModelsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerModelsModule::createLogic()
{
  return vtkSlicerModelsLogic::New();
}
