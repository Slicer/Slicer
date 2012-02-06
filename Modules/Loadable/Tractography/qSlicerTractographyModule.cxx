/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>

// Tractography Logic includes
#include <vtkSlicerTractographyLogic.h>

// Tractography includes
#include "qSlicerTractographyModule.h"
#include "qSlicerTractographyModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTractographyModule, qSlicerTractographyModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_Tractography
class qSlicerTractographyModulePrivate
{
public:
  qSlicerTractographyModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTractographyModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTractographyModulePrivate::qSlicerTractographyModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTractographyModule methods

//-----------------------------------------------------------------------------
qSlicerTractographyModule::qSlicerTractographyModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTractographyModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerTractographyModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerTractographyModule::~qSlicerTractographyModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyModule::helpText()const
{
  QString help = 
    "This template module is meant to be used with the"
    "with the ModuleWizard.py script distributed with the"
    "Slicer source code (starting with version 4)."
    "Developers can generate their own source code using the"
    "wizard and then customize it to fit their needs.";
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community...";
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyModule::contributor()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Julien Finet, Kitware Inc.");
  return moduleContributors.join(";");
}

//-----------------------------------------------------------------------------
QIcon qSlicerTractographyModule::icon()const
{
  return QIcon(":/Icons/Tractography.png");
}

//-----------------------------------------------------------------------------
void qSlicerTractographyModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerTractographyModule::createWidgetRepresentation()
{
  return new qSlicerTractographyModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTractographyModule::createLogic()
{
  return vtkSlicerTractographyLogic::New();
}
