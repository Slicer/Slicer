/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// QT includes
#include <QtPlugin>

// SlicerQt includes
#include <qSlicerCoreApplication.h>
#include <qSlicerCoreIOManager.h>
#include <qSlicerNodeWriter.h>

// Tractography QTModule includes
#include "qSlicerTractographyDisplayModule.h"
#include "qSlicerTractographyDisplayModuleWidget.h"
//#include "qSlicerTractographyDisplayWidget.h"
#include "qSlicerFiberBundleReader.h"

// Tractography Logic includes
#include "vtkSlicerFiberBundleLogic.h"

#include <vtkMRMLThreeDViewDisplayableManagerFactory.h>
#include "TractographyDisplayInstantiator.h"

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerTractographyDisplayModule, qSlicerTractographyDisplayModule);

//-----------------------------------------------------------------------------
qSlicerTractographyDisplayModule::
qSlicerTractographyDisplayModule(QObject* _parent)
  : Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
void qSlicerTractographyDisplayModule::setup()
{
  this->Superclass::setup();

  vtkMRMLThreeDViewDisplayableManagerFactory::GetInstance()->
    RegisterDisplayableManager("vtkMRMLTractographyDisplayDisplayableManager");


  vtkSlicerFiberBundleLogic* fiberBundleLogic =
    vtkSlicerFiberBundleLogic::SafeDownCast(this->logic());
  qSlicerCoreIOManager* coreIOManager =
    qSlicerCoreApplication::application()->coreIOManager();
  coreIOManager->registerIO(
    new qSlicerFiberBundleReader(fiberBundleLogic, this));
  coreIOManager->registerIO(new qSlicerNodeWriter(
    "FiberBundles", QString("FiberBundleFile"),
    QStringList() << "vtkMRMLFiberBundleNode", true, this));
}

//-----------------------------------------------------------------------------
QStringList qSlicerTractographyDisplayModule::categories() const
{
  return QStringList() << "Diffusion";
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerTractographyDisplayModule::createWidgetRepresentation()
{
  return new qSlicerTractographyDisplayModuleWidget;
}

//-----------------------------------------------------------------------------
//
vtkMRMLAbstractLogic* qSlicerTractographyDisplayModule::createLogic()
{
  return vtkSlicerFiberBundleLogic::New();
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyDisplayModule::helpText()const
{
  QString help =
    "Load, save and adjust display parameters of fiber bundles. \n"
    "<a href= \"%1/Documentation/%2.%3/Modules/TractographyDisplay\">%1/Documentation/%2.%3/Modules/TractographyDisplay</a>";
  return help.arg(this->slicerWikiUrl()).arg(Slicer_VERSION_MAJOR).arg(Slicer_VERSION_MINOR);
}

//-----------------------------------------------------------------------------
QString qSlicerTractographyDisplayModule::acknowledgementText()const
{
  QString acknowledgement =
    "This work was supported by NA-MIC, NAC, BIRN, NCIGT, and the Slicer Community. "
    "See <a href=\"http://www.slicer.org\">http://www.slicer.org</a> for details.\n";
  return acknowledgement;
}

//-----------------------------------------------------------------------------
QStringList qSlicerTractographyDisplayModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Julien Finet (Kitware)");
  return moduleContributors;
}
