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

#include "qSlicerLayerDMModule.h"

#include "vtkMRMLLayerDisplayableManager.h"

#include <vtkAutoInit.h>

VTK_MODULE_INIT(vtkSlicerLayerDMModuleMRMLDisplayableManager);

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
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerLayerDMModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLayerDMModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerLayerDMModule::icon() const
{
  return QIcon(":/Icons/LayerDM.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerLayerDMModule::categories() const
{
  return QStringList() << "Examples";
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
qSlicerAbstractModuleRepresentation* qSlicerLayerDMModule ::createWidgetRepresentation()
{
  return nullptr;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerLayerDMModule::createLogic()
{
  return nullptr;
}
