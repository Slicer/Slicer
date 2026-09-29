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

==============================================================================*/

#include "vtkSlicerShaderPropertyReader.h"

// Logic includes
#include "vtkSlicerVolumeRenderingLogic.h"
#include <vtkMRMLIOProperties.h>

// MRML includes
#include <vtkMRMLI18N.h>
#include <vtkMRMLShaderPropertyNode.h>

// VTK includes
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkSlicerShaderPropertyReader);

//----------------------------------------------------------------------------
vtkSlicerShaderPropertyReader::vtkSlicerShaderPropertyReader()
{
  this->SetFileType("ShaderPropertyFile");
  this->SetDescription(vtkMRMLTr("qSlicerShaderPropertyReader", "GPU Shader Property"));
  this->SetNameFilters(std::vector<std::string>{ vtkMRMLTr("qSlicerShaderPropertyReader", "Shader Property") + " (*.sp)" });
}

//----------------------------------------------------------------------------
vtkSlicerShaderPropertyReader::~vtkSlicerShaderPropertyReader() = default;

//----------------------------------------------------------------------------
void vtkSlicerShaderPropertyReader::SetVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* logic)
{
  this->VolumeRenderingLogic = logic;
}

//----------------------------------------------------------------------------
vtkSlicerVolumeRenderingLogic* vtkSlicerShaderPropertyReader::GetVolumeRenderingLogic()
{
  return this->VolumeRenderingLogic;
}

//----------------------------------------------------------------------------
bool vtkSlicerShaderPropertyReader::Load(vtkMRMLIOProperties* properties)
{
  this->ClearLoadedNodeIDs();
  if (!properties || !this->VolumeRenderingLogic)
  {
    return false;
  }
  // Name is ignored
  std::string fileName = properties->GetStringProperty("fileName");
  vtkMRMLShaderPropertyNode* node = this->VolumeRenderingLogic->AddShaderPropertyFromFile(fileName.c_str());
  if (!node)
  {
    return false;
  }
  this->AddLoadedNodeID(node->GetID());
  return true;
}
