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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// .NAME vtkMRMLSubjectHierarchyLegacyNode
// .SECTION
// MRML node to represent legacy subject hierarchy node
//

#ifndef __vtkMRMLSubjectHierarchyLegacyNode_h
#define __vtkMRMLSubjectHierarchyLegacyNode_h

// MRML includes
#include <vtkMRMLHierarchyNode.h>

// STD includes
#include <map>

/// \ingroup Slicer_MRML_Core
/// \brief Legacy subject hierarchy node to allow loading older MRML scenes
///
///   Older MRML scenes contain per-item subject hierarchy nodes, which are incompatible
///   with the new single-node subject hierarchy design. Those nodes (identified by the
///   lack of vtkMRMLSubjectHierarchyNode::SUBJECTHIERARCHY_VERSION_ATTRIBUTE_NAME attribute)
///   are converted to this node type.
///
///   Notes:
///   - Only those methods are kept in this node from the old one that may be necessary to
///     extract or use information loaded into this not and are not propagated into the new
///     hierarchy (such as UIDs).
///   - Nested association support is also removed (because it never became completely stable),
///     so if a scene is loaded where a data node is associated to the subject hierarchy node
///     using nested association, then that node will not be properly represented in the hierarchy.
///
class VTK_MRML_EXPORT vtkMRMLSubjectHierarchyLegacyNode : public vtkMRMLHierarchyNode
{
public:
  // Separator characters for (de)serializing the UID map
  static const std::string SUBJECTHIERARCHY_UID_ITEM_SEPARATOR;
  static const std::string SUBJECTHIERARCHY_UID_NAME_VALUE_SEPARATOR;

public:
  /// IMPORTANT! New method should not be used to create subject hierarchy nodes.
  ///   The nodes are created automatically for supported data types when adding
  ///   the data nodes. To set up the node (name, level, parent, associated data)
  ///   call \sa CreateSubjectHierarchyNode method.
  static vtkMRMLSubjectHierarchyLegacyNode *New();
  vtkTypeMacro(vtkMRMLSubjectHierarchyLegacyNode,vtkMRMLHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  /// Get node XML tag name (like Volume, Contour)
  const char* GetNodeTagName() override;

public:
  /// Find subject hierarchy node according to a UID (by exact match)
  /// \param scene MRML scene
  /// \param uidName UID string to lookup
  /// \param uidValue UID string that needs to _exactly match_ the UID string of the subject hierarchy node
  /// \sa GetUID()
  static vtkMRMLSubjectHierarchyLegacyNode* GetSubjectHierarchyLegacyNodeByUID(vtkMRMLScene* scene, const char* uidName, const char* uidValue);

  /// Find subject hierarchy node according to a UID (by containing). For example find UID in instance UID list
  /// \param scene MRML scene
  /// \param uidName UID string to lookup
  /// \param uidValue UID string that needs to be _contained_ in the UID string of the subject hierarchy node
  /// \return First match
  /// \sa GetUID()
  static vtkMRMLSubjectHierarchyLegacyNode* GetSubjectHierarchyLegacyNodeByUIDList(vtkMRMLScene* scene, const char* uidName, const char* uidValue);

public:
  /// Deserialize a UID list string (stored in the UID map) into a vector of UID strings
  static void DeserializeUIDList(std::string uidListString, std::vector<std::string>& deserializedUIDList);

  /// Get subject hierarchy nodes that are referenced from this node by DICOM.
  /// Finds the series nodes that contain the SOP instance UIDs that are listed in
  /// the MRML attribute of this node containing the referenced SOP instance UIDs
  /// \sa vtkMRMLSubjectHierarchyConstants::GetDICOMReferencedInstanceUIDsAttributeName()
  std::vector<vtkMRMLSubjectHierarchyLegacyNode*> GetSubjectHierarchyNodesReferencedByDICOM();

public:
  /// Set level
  vtkSetStringMacro(Level);
  /// Get level
  vtkGetStringMacro(Level);

  /// Set owner plugin name
  vtkSetStringMacro(OwnerPluginName);
  /// Get owner plugin name
  vtkGetStringMacro(OwnerPluginName);

  /// Get a UID with a known name
  /// \return The UID value if exists, empty string if does not
  std::string GetUID(std::string uidName);

  /// Get UID map for this subject hierarchy node
  std::map<std::string, std::string> GetUIDs()const;

protected:
  /// Level identifier (default levels are Subject and Study)
  char* Level{nullptr};

  /// Name of the owner plugin that claimed this node
  char* OwnerPluginName{nullptr};

  /// List of UIDs of this subject hierarchy node
  /// UIDs can be DICOM UIDs, MIDAS urls, etc.
  std::map<std::string, std::string> UIDs;

protected:
  vtkMRMLSubjectHierarchyLegacyNode();
  ~vtkMRMLSubjectHierarchyLegacyNode() override;
  vtkMRMLSubjectHierarchyLegacyNode(const vtkMRMLSubjectHierarchyLegacyNode&);
  void operator=(const vtkMRMLSubjectHierarchyLegacyNode&);
};

#endif
