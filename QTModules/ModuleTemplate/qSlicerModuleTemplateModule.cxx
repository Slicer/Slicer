/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QtPlugin>

// ModuleTemplate Logic includes
#include <vtkSlicerModuleTemplateLogic.h>

// ModuleTemplate includes
#include "qSlicerModuleTemplateModule.h"
#include "qSlicerModuleTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerModuleTemplateModule, qSlicerModuleTemplateModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ModuleTemplate
class qSlicerModuleTemplateModulePrivate
{
public:
  qSlicerModuleTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModulePrivate::qSlicerModuleTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerModuleTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModule::qSlicerModuleTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerModuleTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerModuleTemplateModule::~qSlicerModuleTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerModuleTemplateModule::helpText()const
{
  return "Write some help here...";
}

//-----------------------------------------------------------------------------
QString qSlicerModuleTemplateModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerModuleTemplateModule::icon()const
{
  return QIcon(":/Icons/ModuleTemplate.png");
}

//-----------------------------------------------------------------------------
void qSlicerModuleTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerModuleTemplateModule::createWidgetRepresentation()
{
  return new qSlicerModuleTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerModuleTemplateModule::createLogic()
{
  return vtkSlicerModuleTemplateLogic::New();
}
