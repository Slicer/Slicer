/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care
  and CANARIE.

==============================================================================*/

#ifndef __vtkMRMLMultiVolumeRenderingDisplayNode_h
#define __vtkMRMLMultiVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkMRMLVolumeRenderingDisplayNode.h"

/// \ingroup Slicer_QtModules_VolumeRendering
/// \name vtkMRMLGPURayCastGPURayCastVolumeRenderingDisplayNode
/// \brief MRML node for storing information for GPU Raycast Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLMultiVolumeRenderingDisplayNode
  : public vtkMRMLVolumeRenderingDisplayNode
{
public:
  static vtkMRMLMultiVolumeRenderingDisplayNode *New();
  vtkTypeMacro(vtkMRMLMultiVolumeRenderingDisplayNode,vtkMRMLVolumeRenderingDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLMultiVolumeRenderingDisplayNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "MultiVolumeRendering";}

protected:
  vtkMRMLMultiVolumeRenderingDisplayNode();
  ~vtkMRMLMultiVolumeRenderingDisplayNode() override;
  vtkMRMLMultiVolumeRenderingDisplayNode(const vtkMRMLMultiVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLMultiVolumeRenderingDisplayNode&);
};

#endif
