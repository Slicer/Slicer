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

#ifndef __vtkMRMLGPURayCastVolumeRenderingDisplayNode_h
#define __vtkMRMLGPURayCastVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLGPURayCastGPURayCastVolumeRenderingDisplayNode
/// \brief MRML node for storing information for GPU Raycast Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLGPURayCastVolumeRenderingDisplayNode
  : public vtkMRMLVolumeRenderingDisplayNode
{
public:
  static vtkMRMLGPURayCastVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLGPURayCastVolumeRenderingDisplayNode,vtkMRMLVolumeRenderingDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  // Description:
  // Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLGPURayCastVolumeRenderingDisplayNode);

  // Description:
  // Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "GPURayCastVolumeRendering";}

protected:
  vtkMRMLGPURayCastVolumeRenderingDisplayNode();
  ~vtkMRMLGPURayCastVolumeRenderingDisplayNode() override;
  vtkMRMLGPURayCastVolumeRenderingDisplayNode(const vtkMRMLGPURayCastVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLGPURayCastVolumeRenderingDisplayNode&);
};

#endif
