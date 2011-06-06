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
#include <vtkSlicerLoadableExtensionTemplateLogic.h>

// ExtensionTemplate includes
#include "qSlicerLoadableExtensionTemplateModule.h"
#include "qSlicerLoadableExtensionTemplateModuleWidget.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerLoadableExtensionTemplateModule, qSlicerLoadableExtensionTemplateModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLoadableExtensionTemplateModulePrivate
{
public:
  qSlicerLoadableExtensionTemplateModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLoadableExtensionTemplateModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateModulePrivate::qSlicerLoadableExtensionTemplateModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLoadableExtensionTemplateModule methods

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateModule::qSlicerLoadableExtensionTemplateModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerLoadableExtensionTemplateModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerLoadableExtensionTemplateModule::~qSlicerLoadableExtensionTemplateModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableExtensionTemplateModule::helpText()const
{
  return QString();
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableExtensionTemplateModule::acknowledgementText()const
{
  return "This work was supported by ...";
}

//-----------------------------------------------------------------------------
QIcon qSlicerLoadableExtensionTemplateModule::icon()const
{
  return QIcon(":/Icons/LoadableExtensionTemplate.png");
}

//-----------------------------------------------------------------------------
void qSlicerLoadableExtensionTemplateModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerLoadableExtensionTemplateModule::createWidgetRepresentation()
{
  return new qSlicerLoadableExtensionTemplateModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerLoadableExtensionTemplateModule::createLogic()
{
  return vtkSlicerLoadableExtensionTemplateLogic::New();
}
