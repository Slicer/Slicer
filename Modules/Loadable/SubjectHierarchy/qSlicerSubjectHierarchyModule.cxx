/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Qt includes
#include <QtPlugin>

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModule.h"
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyCloneNodePlugin.h"
#include "qSlicerSubjectHierarchyChartsPlugin.h"
#include "qSlicerSubjectHierarchyParseLocalDataPlugin.h"
#include "qSlicerSubjectHierarchyRegisterPlugin.h"
#include "qSlicerSubjectHierarchySegmentPlugin.h"
#include "qSlicerSubjectHierarchyFolderPlugin.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSubjectHierarchyModule, qSlicerSubjectHierarchyModule);

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModule::qSlicerSubjectHierarchyModule(QObject* _parent)
  : Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModule::~qSlicerSubjectHierarchyModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModule::helpText()const
{
  QString help =
    "The Subject hierarchy module provides a nice and intuitive tree view of the loaded data. It acts as a convenient central organizing point for many of the operations that 3D Slicer and its extensions perform."
    "For more information see <a href=\"%1/Documentation/%2.%3/Modules/SubjectHierarchy\">%1/Documentation/%2.%3/Modules/SubjectHierarchy</a><br>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModule::acknowledgementText()const
{
  return "This work is part of SparKit project, funded by Cancer Care Ontario (CCO)'s ACRU program and Ontario Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyModule::categories() const
{
  return QStringList() << "" << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Csaba Pinter (Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModule::icon()const
{
  return QIcon(":/Icons/SubjectHierarchy.png");
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::setup()
{
  this->Superclass::setup();

  // Register Subject Hierarchy core plugins
  this->registerCorePlugins();

  // Handle scene change event if occurs
  qSlicerSubjectHierarchyModuleWidget* moduleWidget =
    dynamic_cast<qSlicerSubjectHierarchyModuleWidget*>(this->widgetRepresentation());
  qvtkConnect( this->logic(), vtkCommand::ModifiedEvent, moduleWidget, SLOT( onLogicModified() ) );
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::registerCorePlugins()
{
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyFolderPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyParseLocalDataPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyCloneNodePlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyChartsPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchyRegisterPlugin());
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(
    new qSlicerSubjectHierarchySegmentPlugin());
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSubjectHierarchyModule::createLogic()
{
  return vtkSlicerSubjectHierarchyModuleLogic::New();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSubjectHierarchyModule::createWidgetRepresentation()
{
  return new qSlicerSubjectHierarchyModuleWidget;
}
