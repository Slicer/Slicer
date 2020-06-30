/*==============================================================================

  Copyright (c) Kapteyn Astronomical Institute
  University of Groningen, Groningen, Netherlands. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Davide Punzo, Kapteyn Astronomical Institute,
  and was supported through the European Research Council grant nr. 291531.

==============================================================================*/

// Slice includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// Plots Logic includes
#include <vtkSlicerPlotsLogic.h>

// Plots includes
#include "qSlicerPlotsModule.h"
#include "qSlicerPlotsModuleWidget.h"
// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyPlotsPlugin.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerPlotsModulePrivate
{
public:
  qSlicerPlotsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerPlotsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerPlotsModulePrivate::qSlicerPlotsModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerPlotsModule methods

//-----------------------------------------------------------------------------
qSlicerPlotsModule::qSlicerPlotsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerPlotsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerPlotsModule::~qSlicerPlotsModule() = default;

//-----------------------------------------------------------------------------
QIcon qSlicerPlotsModule::icon()const
{
  return QIcon(":/Icons/Plots.png");
}

//-----------------------------------------------------------------------------
QString qSlicerPlotsModule::helpText()const
{
  QString help =
    "The Plots module allows editing properties of plots.<br>"
    "For more information see the <a href=\"%1/Documentation/%2.%3/Developers/Plots\">oneline documentation</a>.<br>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerPlotsModule::acknowledgementText()const
{
  return "This module was originally developed by Davide Punzo, Kapteyn Astronomical Institute,"
    "and was supported through the European Research Council grant nr. 291531.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlotsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << "Davide Punzo (Kapteyn Astronomical Institute)";
  moduleContributors << "Andras Lasso (PerkLab)";
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlotsModule::categories() const
{
  return QStringList() << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlotsModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerPlotsModule::setup()
{
  this->Superclass::setup();

  // Register Subject Hierarchy core plugins

  vtkSlicerPlotsLogic* plotsLogic = vtkSlicerPlotsLogic::SafeDownCast(this->logic());
  qSlicerSubjectHierarchyPlotsPlugin* shPlugin = new qSlicerSubjectHierarchyPlotsPlugin();
  shPlugin->setPlotsLogic(plotsLogic);
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(shPlugin);
}


//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerPlotsModule::createWidgetRepresentation()
{
  return new qSlicerPlotsModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerPlotsModule::createLogic()
{
  return vtkSlicerPlotsLogic::New();
}

//-----------------------------------------------------------------------------
QStringList qSlicerPlotsModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLPlotChartNode" << "vtkMRMLPlotSeriesNode";
}
