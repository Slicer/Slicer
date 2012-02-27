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
  return "This LoadableExtensionTemplate module illustrates how a loadable module should "
      "be implemented.";
}

//-----------------------------------------------------------------------------
QString qSlicerLoadableExtensionTemplateModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableExtensionTemplateModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerLoadableExtensionTemplateModule::icon()const
{
  return QIcon(":/Icons/LoadableExtensionTemplate.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableExtensionTemplateModule::categories() const
{
  return QStringList() << "Examples";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLoadableExtensionTemplateModule::dependencies() const
{
  return QStringList();
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
