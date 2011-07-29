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

// ExtensionTemplate Logic includes
#include <vtkSlicerSuperBuildLoadableExtensionTemplateLogic.h>

// ExtensionTemplate includes
#include "qSlicerSuperBuildLoadableExtensionTemplateModule.h"
#include "qSlicerSuperBuildLoadableExtensionTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerSuperBuildLoadableExtensionTemplateModule, qSlicerSuperBuildLoadableExtensionTemplateModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerSuperBuildLoadableExtensionTemplateModulePrivate
{
public:
  qSlicerSuperBuildLoadableExtensionTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerSuperBuildLoadableExtensionTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerSuperBuildLoadableExtensionTemplateModulePrivate::qSlicerSuperBuildLoadableExtensionTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerSuperBuildLoadableExtensionTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerSuperBuildLoadableExtensionTemplateModule::qSlicerSuperBuildLoadableExtensionTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerSuperBuildLoadableExtensionTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerSuperBuildLoadableExtensionTemplateModule::~qSlicerSuperBuildLoadableExtensionTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerSuperBuildLoadableExtensionTemplateModule::helpText()const
{
  return "This LoadableExtensionTemplate module illustrates how a loadable module should "
      "be implemented.";
}

//-----------------------------------------------------------------------------
QString qSlicerSuperBuildLoadableExtensionTemplateModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerSuperBuildLoadableExtensionTemplateModule::icon()const
{
  return QIcon(":/Icons/SuperBuildLoadableExtensionTemplate.png");
}

//-----------------------------------------------------------------------------
void qSlicerSuperBuildLoadableExtensionTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerSuperBuildLoadableExtensionTemplateModule::createWidgetRepresentation()
{
  return new qSlicerSuperBuildLoadableExtensionTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerSuperBuildLoadableExtensionTemplateModule::createLogic()
{
  return vtkSlicerSuperBuildLoadableExtensionTemplateLogic::New();
}
