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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

#ifndef __vtkMRMLTextStorageNode_h
#define __vtkMRMLTextStorageNode_h

// MRML includes
#include <vtkMRMLStorageNode.h>

/// \ingroup Slicer_QtModules_Sequences
class VTK_MRML_EXPORT vtkMRMLTextStorageNode : public vtkMRMLStorageNode
{
public:

  static vtkMRMLTextStorageNode* New();
  vtkTypeMacro(vtkMRMLTextStorageNode, vtkMRMLStorageNode);

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "TextStorage"; };

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Return true if the node can be written by using thie writer.
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  /// Return a default file extension for writting
  const char* GetDefaultWriteFileExtension() override;

protected:
  vtkMRMLTextStorageNode();
  ~vtkMRMLTextStorageNode() override;
  vtkMRMLTextStorageNode(const vtkMRMLTextStorageNode&);
  void operator=(const vtkMRMLTextStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;
};

#endif
