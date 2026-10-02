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

#include "vtkMRMLLayerDMPipelineCallbackCreator.h"

// Layer DM includes
#include "vtkMRMLLayerDMPipeline.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

//-----------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLLayerDMPipelineCallbackCreator);

//-----------------------------------------------------------------------------
vtkSmartPointer<vtkMRMLLayerDMPipeline> vtkMRMLLayerDMPipelineCallbackCreator::CreatePipeline(vtkMRMLAbstractViewNode* viewNode, vtkMRMLNode* node) const
{
  return this->Callback(viewNode, node);
}

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineCallbackCreator::SetCallback(const std::function<vtkSmartPointer<vtkMRMLLayerDMPipeline>(vtkMRMLAbstractViewNode*, vtkMRMLNode*)>& callback)
{
  this->Callback = callback;
}

vtkMRMLLayerDMPipelineCallbackCreator::vtkMRMLLayerDMPipelineCallbackCreator()
  : Callback([](vtkMRMLAbstractViewNode*, vtkMRMLNode*) { return nullptr; }) {};

//-----------------------------------------------------------------------------
void vtkMRMLLayerDMPipelineCallbackCreator::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Callback: " << (this->Callback ? "set" : "(none)") << std::endl;
}
