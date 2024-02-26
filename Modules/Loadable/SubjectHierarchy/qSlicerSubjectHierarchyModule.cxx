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

#include <vtkSlicerConfigure.h> // For Slicer_USE_PYTHONQT

// QtGUI includes
#include <qSlicerApplication.h>
#ifdef Slicer_USE_PYTHONQT
# include <qSlicerPythonManager.h>
#endif

// SubjectHierarchy includes
#include "qSlicerSubjectHierarchyModule.h"
#include "qSlicerSubjectHierarchyModuleWidget.h"
#include "qSlicerSubjectHierarchySettingsPanel.h"
#include "vtkSlicerSubjectHierarchyModuleLogic.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginLogic.h"
#include "qSlicerSubjectHierarchyPluginHandler.h"

// MRML includes
#include <vtkMRMLScene.h>

//-----------------------------------------------------------------------------
class qSlicerSubjectHierarchyModulePrivate
{
public:
  qSlicerSubjectHierarchyModulePrivate();
  ~qSlicerSubjectHierarchyModulePrivate();

  qSlicerSubjectHierarchyPluginLogic* PluginLogic{ nullptr };
};

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModulePrivate::qSlicerSubjectHierarchyModulePrivate() = default;

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModulePrivate::~qSlicerSubjectHierarchyModulePrivate()
{
  if (this->PluginLogic)
  {
    delete this->PluginLogic;
    this->PluginLogic = nullptr;
  }
}

//-----------------------------------------------------------------------------
// qSlicerSubjectHierarchyModule methods

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModule::qSlicerSubjectHierarchyModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSubjectHierarchyModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSubjectHierarchyModule::~qSlicerSubjectHierarchyModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModule::helpText() const
{
  QString help = "The Subject hierarchy module provides a nice and intuitive tree view of the loaded data."
                 " It acts as a convenient central organizing point for many of the operations that 3D Slicer and its "
                 "extensions perform.<br>";
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerSubjectHierarchyModule::acknowledgementText() const
{
  return "This work is part of SparKit project, funded by Cancer Care Ontario (CCO)'s ACRU program and Ontario "
         "Consortium for Adaptive Interventions in Radiation Oncology (OCAIRO).";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyModule::categories() const
{
  return QStringList() << "" << qSlicerAbstractCoreModule::tr("Informatics");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSubjectHierarchyModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Csaba Pinter (Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSubjectHierarchyModule::icon() const
{
  return QIcon(":/Icons/SubjectHierarchy.png");
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::setup()
{
  this->Superclass::setup();

  qSlicerApplication* app = qSlicerApplication::application();
  if (app)
  {
    // Register settings panel
    qSlicerSubjectHierarchySettingsPanel* panel = new qSlicerSubjectHierarchySettingsPanel();
    app->settingsDialog()->addPanel("Subject hierarchy", panel);

    // Explicitly import associated python library to trigger registration of plugins
#ifdef Slicer_USE_PYTHONQT
    if (!qSlicerCoreApplication::testAttribute(qSlicerCoreApplication::AA_DisablePython))
    {
      app->pythonManager()->executeString(QString("import SubjectHierarchyLib"));
    }
#endif
  }
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSubjectHierarchyModule::createLogic()
{
  Q_D(qSlicerSubjectHierarchyModule);

  // Create logic
  vtkSlicerSubjectHierarchyModuleLogic* logic = vtkSlicerSubjectHierarchyModuleLogic::New();
  // Handle scene change event if occurs
  qvtkConnect(logic, vtkCommand::ModifiedEvent, this, SLOT(onLogicModified()));

  // Create plugin logic
  d->PluginLogic = new qSlicerSubjectHierarchyPluginLogic();
  // Set plugin logic to plugin handler
  qSlicerSubjectHierarchyPluginHandler::instance()->setPluginLogic(d->PluginLogic);

  return logic;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSubjectHierarchyModule::createWidgetRepresentation()
{
  Q_D(qSlicerSubjectHierarchyModule);

  qSlicerSubjectHierarchyModuleWidget* moduleWidget = new qSlicerSubjectHierarchyModuleWidget();
  if (!d->PluginLogic)
  {
    this->createLogic();
  }
  moduleWidget->setPluginLogic(d->PluginLogic);

  return moduleWidget;
}

//-----------------------------------------------------------------------------
void qSlicerSubjectHierarchyModule::onLogicModified()
{
  Q_D(qSlicerSubjectHierarchyModule);

  vtkMRMLScene* scene = this->mrmlScene();
  if (d->PluginLogic && scene != d->PluginLogic->mrmlScene())
  {
    // Set the new scene to the plugin logic
    d->PluginLogic->setMRMLScene(scene);
  }
}
