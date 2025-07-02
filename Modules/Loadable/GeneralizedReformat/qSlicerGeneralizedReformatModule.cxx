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

// GeneralizedReformat Logic includes
#include <vtkSlicerGeneralizedReformatLogic.h>

// GeneralizedReformat includes
#include "qSlicerGeneralizedReformatModule.h"

//-----------------------------------------------------------------------------
class qSlicerGeneralizedReformatModulePrivate
{
public:
  qSlicerGeneralizedReformatModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerGeneralizedReformatModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerGeneralizedReformatModulePrivate::qSlicerGeneralizedReformatModulePrivate() {}

//-----------------------------------------------------------------------------
// qSlicerGeneralizedReformatModule methods

//-----------------------------------------------------------------------------
qSlicerGeneralizedReformatModule::qSlicerGeneralizedReformatModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerGeneralizedReformatModulePrivate)
{
  this->setWidgetRepresentationCreationEnabled(false);
}

//-----------------------------------------------------------------------------
qSlicerGeneralizedReformatModule::~qSlicerGeneralizedReformatModule() {}

//-----------------------------------------------------------------------------
QString qSlicerGeneralizedReformatModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerGeneralizedReformatModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerGeneralizedReformatModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Lee Newberg (Kitware)");
  moduleContributors << QString("Jean-Christophe (Kitware)");
  moduleContributors << QString("Sam Horvath (Kitware)");
  moduleContributors << QString("Andras Lasso (PerkLab)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerGeneralizedReformatModule::icon() const
{
  return QIcon(":/Icons/GeneralizedReformat.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerGeneralizedReformatModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerGeneralizedReformatModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerGeneralizedReformatModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerGeneralizedReformatModule ::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerGeneralizedReformatModule::createLogic()
{
  return vtkSlicerGeneralizedReformatLogic::New();
}
