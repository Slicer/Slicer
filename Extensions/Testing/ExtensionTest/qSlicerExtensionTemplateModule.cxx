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

// ExtensionTemplate Logic includes
#include <vtkSlicerExtensionTemplateLogic.h>

// ExtensionTemplate includes
#include "qSlicerExtensionTemplateModule.h"
#include "qSlicerExtensionTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerExtensionTemplateModule, qSlicerExtensionTemplateModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerExtensionTemplateModulePrivate
{
public:
  qSlicerExtensionTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerExtensionTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerExtensionTemplateModulePrivate::qSlicerExtensionTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerExtensionTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerExtensionTemplateModule::qSlicerExtensionTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerExtensionTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerExtensionTemplateModule::~qSlicerExtensionTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionTemplateModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerExtensionTemplateModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerExtensionTemplateModule::icon()const
{
  return QIcon(":/Icons/ExtensionTemplate.png");
}

//-----------------------------------------------------------------------------
void qSlicerExtensionTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerExtensionTemplateModule::createWidgetRepresentation()
{
  return new qSlicerExtensionTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerExtensionTemplateModule::createLogic()
{
  return vtkSlicerExtensionTemplateLogic::New();
}
