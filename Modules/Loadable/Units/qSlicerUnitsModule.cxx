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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// Slicer includes
#include <qSlicerApplication.h>
#include <vtkSlicerUnitsLogic.h>

// Units includes
#include "qSlicerUnitsModule.h"
#include "qSlicerUnitsSettingsPanel.h"

//-----------------------------------------------------------------------------
class qSlicerUnitsModulePrivate
{
public:
  qSlicerUnitsModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerUnitsModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerUnitsModulePrivate
::qSlicerUnitsModulePrivate() = default;

//-----------------------------------------------------------------------------
// qSlicerUnitsModule methods

//-----------------------------------------------------------------------------
qSlicerUnitsModule
::qSlicerUnitsModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerUnitsModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerUnitsModule::~qSlicerUnitsModule() = default;

//-----------------------------------------------------------------------------
QString qSlicerUnitsModule::helpText()const
{
  QString help =
    "This module controls the Units of the scene."
    " It gives the option to create, edit and remove units.<br>"
    "<a href=%1/Documentation/%2.%3/Modules/Units>"
    "%1/Documentation/%2.%3/Modules/Units</a>";
  return help.arg(this->slicerWikiUrl())
    .arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerUnitsModule::acknowledgementText()const
{
  QString acknowledgement = QString(
    "<center><table border=\"0\"><tr>"
    "<td><img src=\":Logos/NAMIC.png\" alt\"NA-MIC\"></td>"
    "<td><img src=\":Logos/NAC.png\" alt\"NAC\"></td>"
    "</tr><tr>"
    "<td><img src=\":Logos/BIRN-NoText.png\" alt\"BIRN\"></td>"
    "<td><img src=\":Logos/NCIGT.png\" alt\"NCIGT\"></td>"
    "</tr></table></center>"
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community.");
  return acknowledgement;
}
//-----------------------------------------------------------------------------
QStringList qSlicerUnitsModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Johan Andruejol (Kitware)")
    << QString("Julien Finet (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerUnitsModule::icon()const
{
  return QIcon("");
}

//-----------------------------------------------------------------------------
QStringList qSlicerUnitsModule::categories() const
{
  return QStringList() << "Informatics";
}

//-----------------------------------------------------------------------------
QStringList qSlicerUnitsModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
bool qSlicerUnitsModule::isHidden() const
{
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerUnitsModule::setup()
{
  this->Superclass::setup();

  vtkSlicerUnitsLogic* logic =
    vtkSlicerUnitsLogic::SafeDownCast(this->logic());
  if (logic && qSlicerApplication::application())
    {
    qSlicerUnitsSettingsPanel* panel = new qSlicerUnitsSettingsPanel;
    qSlicerApplication::application()->settingsDialog()->addPanel(
      "Units", panel);
    panel->setUnitsLogic(logic);
    }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerUnitsModule
::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerUnitsModule::createLogic()
{
  return vtkSlicerUnitsLogic::New();
}
