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

// Slice includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// DoubleArrays Logic includes
#include <vtkSlicerDoubleArraysLogic.h>

// DoubleArrays includes
#include "qSlicerDoubleArraysModule.h"
#include "qSlicerDoubleArraysReader.h"

//-----------------------------------------------------------------------------
#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
#include <QtPlugin>
Q_EXPORT_PLUGIN2(qSlicerDoubleArraysModule, qSlicerDoubleArraysModule);
#endif

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerDoubleArraysModulePrivate
{
public:
  qSlicerDoubleArraysModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerDoubleArraysModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerDoubleArraysModulePrivate::qSlicerDoubleArraysModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerDoubleArraysModule methods

//-----------------------------------------------------------------------------
qSlicerDoubleArraysModule::qSlicerDoubleArraysModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerDoubleArraysModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerDoubleArraysModule::~qSlicerDoubleArraysModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerDoubleArraysModule::helpText()const
{
  return "This module provides support for double array nodes";
}

//-----------------------------------------------------------------------------
QString qSlicerDoubleArraysModule::acknowledgementText()const
{
  return "This work was was partially funded by NIH grant 3P41RR013218-12S1";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDoubleArraysModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Julien Finet (Kitware)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerDoubleArraysModule::categories() const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
QStringList qSlicerDoubleArraysModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
void qSlicerDoubleArraysModule::setup()
{
  this->Superclass::setup();
  vtkSlicerDoubleArraysLogic* doubleArraysLogic =
    vtkSlicerDoubleArraysLogic::SafeDownCast(this->logic());

  qSlicerCoreIOManager* ioManager =
    qSlicerCoreApplication::application()->coreIOManager();
  ioManager->registerIO(new qSlicerDoubleArraysReader(doubleArraysLogic,this));
  ioManager->registerIO(new qSlicerNodeWriter(
    "Double Arrays", QString("DoubleArrayFile"),
    QStringList() << "vtkMRMLDoubleArrayNode", true, this));
}

//-----------------------------------------------------------------------------
bool qSlicerDoubleArraysModule::isHidden() const
{
  return true;
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerDoubleArraysModule::createWidgetRepresentation()
{
  return 0;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerDoubleArraysModule::createLogic()
{
  return vtkSlicerDoubleArraysLogic::New();
}
