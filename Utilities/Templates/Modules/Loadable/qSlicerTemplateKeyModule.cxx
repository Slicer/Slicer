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

// TemplateKey Logic includes
#include <vtkSlicerTemplateKeyLogic.h>

// TemplateKey includes
#include "qSlicerTemplateKeyModule.h"
#include "qSlicerTemplateKeyModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerTemplateKeyModulePrivate
{
public:
  qSlicerTemplateKeyModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModulePrivate::qSlicerTemplateKeyModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerTemplateKeyModule methods

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModule::qSlicerTemplateKeyModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerTemplateKeyModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerTemplateKeyModule::~qSlicerTemplateKeyModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerTemplateKeyModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerTemplateKeyModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerTemplateKeyModule::icon() const
{
  return QIcon(":/Icons/TemplateKey.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerTemplateKeyModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerTemplateKeyModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTemplateKeyModule
::createWidgetRepresentation()
{
  return new qSlicerTemplateKeyModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerTemplateKeyModule::createLogic()
{
  return vtkSlicerTemplateKeyLogic::New();
}
