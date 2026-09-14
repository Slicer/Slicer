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

///  vtkMRMLVolumeSequenceStorageNode - MRML node that can read/write
///  a Sequence node containing volumes in a single NRRD file.

#ifndef __vtkMRMLVolumeSequenceStorageNode_h
#define __vtkMRMLVolumeSequenceStorageNode_h

#include "vtkMRML.h"

#include "vtkMRMLStorageNode.h"
#include <string>

/// \brief Store a sequence of volumes in a NRRD file.
///
/// The sequence axis is always the last image axis ("list" kind)..
/// It can store vector (spatial or RGB/RGBA color) voxel in the first image axis (vector or RGB/RGBA kind).
/// Altogether it can store 4D (xyzt) or 5D (cxyzt) data.
///
/// If the image stores spatial vectors then those vectors are written to the file in LPS coordinate system.
/// A file stores spatial vectors if the component axis is "vector" kind and measurement frame is specified;
/// or it is "covariant-vector" kind.
///
/// Custom fields store information on the sequence:
/// - DataNodeClassName: exact node type that the file should be read into (e.g., vtkMRMLScalarVolumeNode)
/// - axis 3 index type: numeric or text
/// - axis 3 index values: space-separated list of index values (URL-encoded, to deal with special characters)
///
/// By default, if the image is stored in the file using a left-handed IJK coordinate system then the
/// volumes are converted to right-handed IJK coordinate system when reading (same as for scalar volumes).
/// See ForceRightHandedIJKCoordinateSystem.
///

class VTK_MRML_EXPORT vtkMRMLVolumeSequenceStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLVolumeSequenceStorageNode* New();
  vtkTypeMacro(vtkMRMLVolumeSequenceStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  //@{
  /// Force right-handed IJK coordinate system when reading an image sequence from file.
  /// If enabled and the file stored on disk uses left-handed IJK coordinate system,
  /// then the reader will flip the K axis direction and update the image origin
  /// to make the IJK coordinate system of the loaded volumes right-handed.
  /// Enabled by default, as certain processing algorithms assume this right-handed IJK.
  /// \sa vtkMRMLVolumeArchetypeStorageNode::SetForceRightHandedIJKCoordinateSystem
  vtkSetMacro(ForceRightHandedIJKCoordinateSystem, bool);
  vtkGetMacro(ForceRightHandedIJKCoordinateSystem, bool);
  vtkBooleanMacro(ForceRightHandedIJKCoordinateSystem, bool);
  //@}

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "VolumeSequenceStorage"; };

  /// Get node type to be displayed to the user.
  std::string GetTypeDisplayName() override { return vtkMRMLTr("vtkMRMLVolumeSequenceStorageNode", "Volume Sequence Storage"); };

  /// Return true if this class can read the node.
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Return true if this class can write the node.
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;

  /// Write the data. Returns 1 on success, 0 otherwise.
  ///
  /// The nrrd file will be formatted such as:
  /// "kinds: [component] domain domain domain list"
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  ///
  /// Return a default file extension for writing
  const char* GetDefaultWriteFileExtension() override;

protected:
  vtkMRMLVolumeSequenceStorageNode();
  ~vtkMRMLVolumeSequenceStorageNode() override;
  vtkMRMLVolumeSequenceStorageNode(const vtkMRMLVolumeSequenceStorageNode&);
  void operator=(const vtkMRMLVolumeSequenceStorageNode&);

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

  bool ForceRightHandedIJKCoordinateSystem{ true };
};

#endif
