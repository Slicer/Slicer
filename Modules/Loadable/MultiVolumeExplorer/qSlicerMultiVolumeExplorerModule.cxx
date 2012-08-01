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
#include <QFileInfo>
#include <QScopedPointer>
#include <QtPlugin>

// Slicer includes
#include <vtkSlicerVolumesLogic.h>

// MultiVolumeExplorer Logic includes
#include <vtkSlicerMultiVolumeExplorerLogic.h>

// MultiVolumeExplorer includes
#include "qSlicerMultiVolumeExplorerModule.h"

// SlicerQT includes
#include <qSlicerUtils.h>
#include <qSlicerModuleManager.h>
#include <qSlicerScriptedLoadableModuleWidget.h>
#include <qSlicerApplication.h>
#include <vtkSlicerConfigure.h>

//-----------------------------------------------------------------------------
Q_EXPORT_PLUGIN2(qSlicerMultiVolumeExplorerModule, qSlicerMultiVolumeExplorerModule);

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerMultiVolumeExplorerModulePrivate
{
public:
  qSlicerMultiVolumeExplorerModulePrivate();
};

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeExplorerModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModulePrivate::qSlicerMultiVolumeExplorerModulePrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerMultiVolumeExplorerModule methods

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModule::qSlicerMultiVolumeExplorerModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerMultiVolumeExplorerModulePrivate)
{
}

//-----------------------------------------------------------------------------
qSlicerMultiVolumeExplorerModule::~qSlicerMultiVolumeExplorerModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerMultiVolumeExplorerModule::helpText()const
{
  return "MultiVolumeExplorer module supports various operations related to examining multivolume nodes.";
}

//-----------------------------------------------------------------------------
QString qSlicerMultiVolumeExplorerModule::acknowledgementText()const
{
  return "Development of this module was supported in part by "
      "the following grants: P41EB015898, P41RR019703, R01CA111288 "
      "and U01CA151261.";
}

//-----------------------------------------------------------------------------
QIcon qSlicerMultiVolumeExplorerModule::icon()const
{
  return QIcon(":/Icons/MultiVolumeExplorer.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeExplorerModule::categories() const
{
  return QStringList() << "Work in Progress.MultiVolume Support";
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeExplorerModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
QStringList qSlicerMultiVolumeExplorerModule::contributors()const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andrey Fedorov (SPL, BWH)");
  moduleContributors << QString("Jean-Christophe Fillion-Robin (Kitware)");
  moduleContributors << QString("Julien Finet (Kitware)");
  moduleContributors << QString("Steve Pieper (SPL, BWH)");
  moduleContributors << QString("Ron Kikinis (SPL, BWH)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
void qSlicerMultiVolumeExplorerModule::setup()
{
  this->Superclass::setup();

  // Register the IO module for loading MultiVolumes as a variant of nrrd files
  qSlicerAbstractCoreModule* volumes = qSlicerApplication::application()->moduleManager()->module("Volumes");
  if (volumes)
    {
    vtkSlicerVolumesLogic* volumesLogic 
      = dynamic_cast<vtkSlicerVolumesLogic*>(volumes->logic());
    vtkSlicerMultiVolumeExplorerLogic* logic
      = dynamic_cast<vtkSlicerMultiVolumeExplorerLogic*>(this->logic());
    if (volumesLogic && logic)
      {
      logic->RegisterArchetypeVolumeNodeSetFactory( volumesLogic );
      }
    }
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation * qSlicerMultiVolumeExplorerModule::createWidgetRepresentation()
{
  QString pythonPath = qSlicerUtils::pathWithoutIntDir(
              QFileInfo(this->path()).path(), Slicer_QTLOADABLEMODULES_LIB_DIR);

  QScopedPointer<qSlicerScriptedLoadableModuleWidget> widget(new qSlicerScriptedLoadableModuleWidget);
  QString classNameToLoad = "qSlicerMultiVolumeExplorerModuleWidget";
  bool ret = widget->setPythonSource(
        pythonPath + "/Python/" + classNameToLoad + ".py", classNameToLoad);
  if (!ret)
    {
    return 0;
    }
  return widget.take();
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerMultiVolumeExplorerModule::createLogic()
{
  return vtkSlicerMultiVolumeExplorerLogic::New();
}
