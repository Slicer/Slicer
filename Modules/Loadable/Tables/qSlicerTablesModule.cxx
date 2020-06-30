/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// Slice includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// Tables Logic includes
#include <vtkSlicerTablesLogic.h>

// Tables includes
#include "qSlicerTablesModule.h"
#include "qSlicerTablesReader.h"
#include "qSlicerTablesModuleWidget.h"
// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTablesPlugin.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTablesModulePrivate
{
public:
  qSlicerTablesModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTablesModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTablesModulePrivate::qSlicerTablesModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerTablesModule methods

//-----------------------------------------------------------------------------
qSlicerTablesModule::qSlicerTablesModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTablesModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTablesModule::~qSlicerTablesModule() = default;

//-----------------------------------------------------------------------------
QIcon qSlicerTablesModule::icon()const
{
  return QIcon(":/Icons/Tables.png");
}

//-----------------------------------------------------------------------------
QString qSlicerTablesModule::helpText()const
{
  QString help =
    "The Tables module allows displaying and editing of spreadsheets.<br>"
    "For more information see <a href=\"%1/Documentation/%2.%3/Modules/Tables\">%1/Documentation/%2.%3/Modules/Tables</a><br>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR); 
}

//-----------------------------------------------------------------------------
QString qSlicerTablesModule::acknowledgementText()const
{
  return "This work was was partially funded by OCAIRO, the Applied Cancer Research Unit program of Cancer Care Ontario, and Department of Anesthesia and Critical Care Medicine, Children's Hospital of Philadelphia.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andras Lasso (PerkLab), Kevin Wang (PMH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::categories() const
{
  return QStringList() << "Informatics";
}


//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerTablesModule::setup()
{
  this->Superclass::setup();
  vtkSlicerTablesLogic* TablesLogic =
    vtkSlicerTablesLogic::SafeDownCast(this->logic());

  qSlicerCoreIOManager* ioManager =
    qSlicerCoreApplication::application()->coreIOManager();
  ioManager->registerIO(new qSlicerTablesReader(TablesLogic,this));
  ioManager->registerIO(new qSlicerNodeWriter(
    "Table", QString("TableFile"),
    QStringList() << "vtkMRMLTableNode", false, this));
  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyTablesPlugin());
}


//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTablesModule::createWidgetRepresentation()
{
  return new qSlicerTablesModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTablesModule::createLogic()
{
  return vtkSlicerTablesLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerTablesModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLTableNode";
}
