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

// LoadableModuleTemplate Logic includes
#include <vtkSlicerLoadableModuleTemplateLogic.h>

// LoadableModuleTemplate includes
#include "qSlicerLoadableModuleTemplateModule.h"
#include "qSlicerLoadableModuleTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLoadableModuleTemplateModulePrivate
{
public:
  qSlicerLoadableModuleTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModulePrivate::qSlicerLoadableModuleTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLoadableModuleTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModule::qSlicerLoadableModuleTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerLoadableModuleTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableModuleTemplateModule::~qSlicerLoadableModuleTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleTemplateModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableModuleTemplateModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableModuleTemplateModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerLoadableModuleTemplateModule::icon() const
{
  return QIcon(":/Icons/LoadableModuleTemplate.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableModuleTemplateModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableModuleTemplateModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerLoadableModuleTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerLoadableModuleTemplateModule
::createWidgetRepresentation()
{
  return new qSlicerLoadableModuleTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerLoadableModuleTemplateModule::createLogic()
{
  return vtkSlicerLoadableModuleTemplateLogic::New();
}
