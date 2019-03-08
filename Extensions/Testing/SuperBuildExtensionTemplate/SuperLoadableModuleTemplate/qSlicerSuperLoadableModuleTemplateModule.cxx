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

// SuperLoadableModuleTemplate Logic includes
#include <vtkSlicerSuperLoadableModuleTemplateLogic.h>

// SuperLoadableModuleTemplate includes
#include "qSlicerSuperLoadableModuleTemplateModule.h"
#include "qSlicerSuperLoadableModuleTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSuperLoadableModuleTemplateModulePrivate
{
public:
  qSlicerSuperLoadableModuleTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSuperLoadableModuleTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSuperLoadableModuleTemplateModulePrivate::qSlicerSuperLoadableModuleTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSuperLoadableModuleTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerSuperLoadableModuleTemplateModule::qSlicerSuperLoadableModuleTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSuperLoadableModuleTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSuperLoadableModuleTemplateModule::~qSlicerSuperLoadableModuleTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSuperLoadableModuleTemplateModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerSuperLoadableModuleTemplateModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSuperLoadableModuleTemplateModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerSuperLoadableModuleTemplateModule::icon() const
{
  return QIcon(":/Icons/SuperLoadableModuleTemplate.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerSuperLoadableModuleTemplateModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerSuperLoadableModuleTemplateModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerSuperLoadableModuleTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerSuperLoadableModuleTemplateModule
::createWidgetRepresentation()
{
  return new qSlicerSuperLoadableModuleTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSuperLoadableModuleTemplateModule::createLogic()
{
  return vtkSlicerSuperLoadableModuleTemplateLogic::New();
}
