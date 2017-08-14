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

// MultiVolumeRendering Logic includes
#include <vtkSlicerMultiVolumeRenderingLogic.h>

// MultiVolumeRendering includes
#include "qSlicerMultiVolumeRenderingModule.h"
#include "qSlicerMultiVolumeRenderingModuleWidget.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerMultiVolumeRenderingModule, qSlicerMultiVolumeRenderingModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_MultiVolumeRendering
class qSlicerMultiVolumeRenderingModulePrivate
{
public:
  qSlicerMultiVolumeRenderingModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModulePrivate::qSlicerMultiVolumeRenderingModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeRenderingModule methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModule::qSlicerMultiVolumeRenderingModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMultiVolumeRenderingModulePrivate)
{
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeRenderingModule::categories()const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeRenderingModule::~qSlicerMultiVolumeRenderingModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerMultiVolumeRenderingModule::helpText()const
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
QString qSlicerMultiVolumeRenderingModule::acknowledgementText()const
{
  return "This work was supported by NAMIC, NAC, and the Slicer Community...";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeRenderingModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (Organization)");
  // moduleContributors << QString("Richard Roe (Organization2)");
  // ...
  return moduleContributors;
}

QStringList qSlicerMultiVolumeRenderingModule::dependencies()const
{
  return QStringList() << "VolumeRendering";
}

//-----------------------------------------------------------------------------
QIcon qSlicerMultiVolumeRenderingModule::icon()const
{
  return QIcon(":/Icons/MultiVolumeRendering.png");
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeRenderingModule::setup()
{
  this->Superclass::setup();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerMultiVolumeRenderingModule::createWidgetRepresentation()
{
  return new qSlicerMultiVolumeRenderingModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMultiVolumeRenderingModule::createLogic()
{
  return vtkSlicerMultiVolumeRenderingLogic::New();
}
