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

// Slices QTModule includes
#include "qSlicerViewControllersModule.h"
#include "qSlicerViewControllersModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerViewControllersModule, qSlicerViewControllersModule);

//-----------------------------------------------------------------------------
class qSlicerViewControllersModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerViewControllersModule::qSlicerViewControllersModule(QObject* _parent)
  :Superclass(_parent)
  , d_ptr(new qSlicerViewControllersModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerViewControllersModule::~qSlicerViewControllersModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerViewControllersModule::acknowledgementText()const
{
  return "This module was developed by Jean-Christophe Fillion-Robin, Kitware Inc. "
         "This work was supported by NIH grant 3P41RR013218-12S1, "
         "NA-MIC, NAC and Slicer community.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerViewControllersModule::categories() const
{
  return QStringList() << "";
}

//-----------------------------------------------------------------------------
QIcon qSlicerViewControllersModule::icon() const
{
  return QIcon(":Icons/ViewControllers.png");
}

//-----------------------------------------------------------------------------
void qSlicerViewControllersModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerViewControllersModule::createWidgetRepresentation()
{
  return new qSlicerViewControllersModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerViewControllersModule::createLogic()
{
  return 0;
}

//-----------------------------------------------------------------------------
QStringList qSlicerViewControllersModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Wendy Plesniak (SPL, BWH)");
  moduleContributors << QString("Jim Miller (GE)");
  moduleContributors << QString("Steve Pieper (Isomics)");
  moduleContributors << QString("Ron Kikinis (SPL, BWH)");
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}
