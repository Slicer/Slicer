/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

// VectorFieldRendering includes
#include "qSlicerVectorFieldRenderingModule.h"
#include "qSlicerVectorFieldRenderingModuleWidget.h"
#include "vtkSlicerVectorFieldRenderingLogic.h"

// SubjectHierarchy Plugins includes
#include "qSlicerSubjectHierarchyPluginHandler.h"
#include "qSlicerSubjectHierarchyVectorFieldRenderingPlugin.h"

// VectorFieldRendering MRMLDM includes
#include "vtkMRMLVectorFieldViewPipeline.h"

//-----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModule::qSlicerVectorFieldRenderingModule(QObject* _parent)
  : Superclass(_parent)
{
}

//-----------------------------------------------------------------------------
qSlicerVectorFieldRenderingModule::~qSlicerVectorFieldRenderingModule() = default;

//-----------------------------------------------------------------------------
QStringList qSlicerVectorFieldRenderingModule::categories() const
{
  return QStringList() << qSlicerAbstractCoreModule::tr("Informatics");
}

//-----------------------------------------------------------------------------
QIcon qSlicerVectorFieldRenderingModule::icon() const
{
  return QIcon(":/Icons/VectorFieldRendering.png");
}

//-----------------------------------------------------------------------------
QString qSlicerVectorFieldRenderingModule::helpText() const
{
  QString help = tr("Shows the vector field of a model (a 3-component point or cell data array), a vector volume, or a transform (its displacement) as glyphs, a "
                    "deformed grid, contours of the magnitude, or streamlines. A node can have any number of vector field displays.");
  help += this->defaultDocumentationLink();
  return help;
}

//-----------------------------------------------------------------------------
QString qSlicerVectorFieldRenderingModule::acknowledgementText() const
{
  return tr("This module was developed with funding from the Children's Hospital of Philadelphia Frontier Program.");
}

//-----------------------------------------------------------------------------
QStringList qSlicerVectorFieldRenderingModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("Andras Lasso (PerkLab, Queen's University)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QStringList qSlicerVectorFieldRenderingModule::dependencies() const
{
  return QStringList() << "Colors" << "LayerDM" << "SubjectHierarchy";
}

//-----------------------------------------------------------------------------
QStringList qSlicerVectorFieldRenderingModule::associatedNodeTypes() const
{
  return QStringList() << "vtkMRMLVectorFieldDisplayNode";
}

//-----------------------------------------------------------------------------
void qSlicerVectorFieldRenderingModule::setup()
{
  this->Superclass::setup();
  // The displayable manager of the LayerDM module draws the vector fields in 3D and slice views
  vtkMRMLVectorFieldViewPipeline::RegisterPipelineCreator();
  qSlicerSubjectHierarchyPluginHandler::instance()->registerPlugin(new qSlicerSubjectHierarchyVectorFieldRenderingPlugin());
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerVectorFieldRenderingModule::createWidgetRepresentation()
{
  return new qSlicerVectorFieldRenderingModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerVectorFieldRenderingModule::createLogic()
{
  return vtkSlicerVectorFieldRenderingLogic::New();
}
