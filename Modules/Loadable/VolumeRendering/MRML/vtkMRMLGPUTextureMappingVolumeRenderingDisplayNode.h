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

#ifndef __vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode_h
#define __vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode
/// \brief MRML node for storing information for GPU Texture Mapping Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode
  : public vtkMRMLVolumeRenderingDisplayNode
{
public:
  static vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode,vtkMRMLVolumeRenderingDisplayNode);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "GPUTextureMappingVolumeRendering";};

protected:
  vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode();
  ~vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode();
  vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode(const vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLGPUTextureMappingVolumeRenderingDisplayNode&);

};

#endif

