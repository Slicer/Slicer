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
  return QString();
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
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerModelsModule::createWidgetRepresentation()
{
  return new qSlicerModelsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkSlicerLogic* qSlicerModelsModule::createLogic()
{
  return 0;
}
