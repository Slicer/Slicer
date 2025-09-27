/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Children's Hospital of Philadelphia, USA. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, Ebatinca, funded
  by the grant GRT-00000485 of Children's Hospital of Philadelphia, USA.

==============================================================================*/

///  vtkMRMLTransformSequenceStorageNode - MRML node that can read/write
///  a Sequence node containing grid transforms in a single NRRD file
///

#ifndef __vtkMRMLTransformSequenceStorageNode_h
#define __vtkMRMLTransformSequenceStorageNode_h

#include "vtkMRML.h"

#include "vtkMRMLStorageNode.h"

#include <string>

class vtkMRMLSequenceNode;
class vtkOrientedGridTransform;

class VTK_MRML_EXPORT vtkMRMLTransformSequenceStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLTransformSequenceStorageNode* New();
  vtkTypeMacro(vtkMRMLTransformSequenceStorageNode, vtkMRMLStorageNode);

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "TransformSequenceStorage"; };

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Return true if the node can be written by using the writer.
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;

  /// Write the data. Returns 1 on success, 0 otherwise.
  ///
  /// The nrrd file will be formatted such as:
  /// "kinds: [component] domain domain domain list"
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  ///
  /// Return a default file extension for writing
  const char* GetDefaultWriteFileExtension() override;

  /// Check to see if this storage node can handle the file type. Returns
  /// nonzero if supported, 0 otherwise. The higher the value, the higher
  /// the confidence that this reader is the most suitable for reading the file.
  /// This is an override of the generic method, because it looks into the
  /// actual file content. It reads the header and checks if the intent code
  /// is set to the value that indicates that it is a grid transform sequence
  /// stored as a volume.
  virtual int SupportedFileType(const char* fileName);

protected:
  vtkMRMLTransformSequenceStorageNode();
  ~vtkMRMLTransformSequenceStorageNode() override;
  vtkMRMLTransformSequenceStorageNode(const vtkMRMLTransformSequenceStorageNode&);
  void operator=(const vtkMRMLTransformSequenceStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  ///
  /// It is assumed that the nrrd file is formatted such as:
  /// "kinds: [component] domain domain domain list"
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Get an oriented grid transform that is used as a common reference geometry.
  /// Returns nullptr if not all transforms are grid transforms or identity, or if transforms
  /// have different directions (mix of TransformFromParent and TransformToParent).
  /// isTransformFromParent will be set to true if all transforms use TransformFromParent,
  /// false if all use TransformToParent.
  vtkOrientedGridTransform* GetReferenceGridTransform(vtkMRMLSequenceNode* seqNode, bool& isTransformFromParent);
};

#endif
