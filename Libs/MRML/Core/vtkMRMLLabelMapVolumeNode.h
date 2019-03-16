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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkMRMLLabelMapVolumeNode_h
#define __vtkMRMLLabelMapVolumeNode_h

// MRML includes
#include "vtkMRMLScalarVolumeNode.h"

/// \brief MRML node for representing a label map volume.
///
/// A label map volume is typically the output of a segmentation procedure that
/// labels each voxel according to its segment (e.g., a certain type of tissue).
///
/// For a long time vtkMRMLLabelMapVolumeNode was not a separate node type
/// but it was a vtkMRMLScalarVolumeNode with the custom LabelMap attribute set to 1.
/// Now the LabelMap attribute of vtkMRMLScalarVolumeNode is ignored and the node
/// type is used for determining if the type of the volume is grayscale/color or label map.
class VTK_MRML_EXPORT vtkMRMLLabelMapVolumeNode : public vtkMRMLScalarVolumeNode
{
  public:
  static vtkMRMLLabelMapVolumeNode *New();
  vtkTypeMacro(vtkMRMLLabelMapVolumeNode,vtkMRMLScalarVolumeNode);

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "LabelMapVolume";}

  ///
  /// Make a 'None' volume node with blank image data
  static void CreateNoneNode(vtkMRMLScene *scene);

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

protected:
  vtkMRMLLabelMapVolumeNode();
  ~vtkMRMLLabelMapVolumeNode() override;
  vtkMRMLLabelMapVolumeNode(const vtkMRMLLabelMapVolumeNode&);
  void operator=(const vtkMRMLLabelMapVolumeNode&);
};

#endif
