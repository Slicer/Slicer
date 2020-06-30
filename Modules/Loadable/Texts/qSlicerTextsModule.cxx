/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerCoreIOManager.h"
#include <qSlicerNodeWriter.h>

// Texts includes
#include "qSlicerTextsModule.h"
#include "qSlicerTextsModuleWidget.h"
#include "qSlicerTextsReader.h"
#include "vtkSlicerTextsLogic.h"

// VTK includes
#include "vtkSmartPointer.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyTextsPlugin.h"

//-----------------------------------------------------------------------------
class qSlicerTextsModulePrivate
{
public:
};

//-----------------------------------------------------------------------------
qSlicerTextsModule::qSlicerTextsModule(QObject* _parentObject)
  : Superclass(_parentObject)
  , d_ptr(new qSlicerTextsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTextsModule::~qSlicerTextsModule() = default;

//-----------------------------------------------------------------------------
QIcon qSlicerTextsModule::icon()const
{
  return QIcon(":/Icons/SlicerTexts.png");
}


//-----------------------------------------------------------------------------
QStringList qSlicerTextsModule::categories() const
{
  return QStringList() << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTextsModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTextsModule::createWidgetRepresentation()
{
  return new qSlicerTextsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTextsModule::createLogic()
{
  return vtkSlicerTextsLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerTextsModule::helpText()const
{
  QString help =
    "A module to create, edit and manage text data in the scene.<br>"
    "For more information see the <a href=%1/Documentation/%2.%3/Modules/Texts>online documentation</a>.<br>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerTextsModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported through CANARIE's Research Software Program, and Cancer Care Ontario.<br>"
    "The Texts module was contributed by Kyle Sunderland and Andras Lasso (Perk Lab, Queen's University)";
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTextsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Kyle Sunderland (PerkLab, Queen's)");
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
void qSlicerTextsModule::setup()
{
  qSlicerApplication * app = qSlicerApplication::application();
  if (!app)
    {
    return;
    }

  qSlicerTextsReader* textFileReader = new qSlicerTextsReader(this);
  app->coreIOManager()->registerIO(textFileReader);
  app->coreIOManager()->registerIO(new qSlicerNodeWriter("TextFileImporter", textFileReader->fileType(), QStringList() << "vtkMRMLTextNode", false, this));

  // Register Subject Hierarchy core plugins
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyTextsPlugin());
}

//-----------------------------------------------------------------------------
QStringList qSlicerTextsModule::associatedNodeTypes() const
{
  return QStringList()
    << "vtkMRMLTextNode";
}
