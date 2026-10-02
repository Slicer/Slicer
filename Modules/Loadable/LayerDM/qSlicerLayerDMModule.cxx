/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware SAS

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thibault Pelletier, Kitware SAS,
  and was partially funded by ANR grants ANR-22-CE45-0034 and ANR-18-RHUS-005.

==============================================================================*/

#include "qSlicerLayerDMModule.h"

#include "vtkMRMLLayerDisplayableManager.h"

#include <vtkSlicerLayerDMLogic.h>

//-----------------------------------------------------------------------------
qSlicerLayerDMModule::qSlicerLayerDMModule(QObject* _parent)
  : Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
qSlicerLayerDMModule::~qSlicerLayerDMModule() {}

//-----------------------------------------------------------------------------
QString qSlicerLayerDMModule::helpText() const
{
  return "A 3D Slicer module introducing a new displayable manager architecture for layered rendering and interaction handling.";
}

//-----------------------------------------------------------------------------
QString qSlicerLayerDMModule::acknowledgementText() const
{
  return "This module was initially developed by Kitware SAS.";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLayerDMModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Thibault Pelletier (Kitware SAS)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerLayerDMModule::icon() const
{
  return QIcon();
}

//-----------------------------------------------------------------------------
QStringList qSlicerLayerDMModule::categories() const
{
  return QStringList() << "Developer Tools";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLayerDMModule::dependencies() const
{
  return QStringList();
}

//-----------------------------------------------------------------------------
bool qSlicerLayerDMModule::isHidden() const
{
  return true;
}

//-----------------------------------------------------------------------------
void qSlicerLayerDMModule::setup()
{
  this->Superclass::setup();
  vtkMRMLLayerDisplayableManager::RegisterInDefaultViews();
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerLayerDMModule::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerLayerDMModule::createLogic()
{
  return vtkSlicerLayerDMLogic::New();
}
