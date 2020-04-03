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
  and was supported through CANARIE.

==============================================================================*/

#ifndef __vtkMRMLFolderDisplayNode_h
#define __vtkMRMLFolderDisplayNode_h

// MRML includes
#include "vtkMRMLDisplayNode.h"

class vtkMRMLDisplayableNode;

/// \brief MRML node to represent a display property for child nodes of a
///        subject hierarchy folder.
///
/// vtkMRMLFolderDisplayNode nodes stores display property of a generic
/// displayable node, so that it can override display properties of its
/// children when the user turns on that option.
///
/// The class has no additional features compared to its base class, but
/// since the base display node class is abstract, we need one that we can
/// instantiate.
///
/// Displayable managers must take into account parent folder display nodes as follows:
///
/// If the \sa ApplyDisplayPropertiesOnBranch flag is off, then the displayable node's own
/// Visibility and Opacity properties must be combined with the corresponding values stored
/// in the folder displayable (opacity = folderOpacity * ownOpacity;
/// visibility = folderVisibility && ownVisibility).
///
/// If the \sa ApplyDisplayPropertiesOnBranch flag is on, then displayable node's own display node
/// properties must be combined or replaced by the folder display nodes properties.
/// Properties to combine: Visibility, Visibility3D, Visibility2D, Opacity.
/// Properties to replace: all other properties stored in folder display node
/// (Color, EdgeColor, SelectedColor, Ambient, Specular, etc.).
///
class VTK_MRML_EXPORT vtkMRMLFolderDisplayNode : public vtkMRMLDisplayNode
{
public:
  static vtkMRMLFolderDisplayNode *New();
  vtkTypeMacro(vtkMRMLFolderDisplayNode,vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "FolderDisplay";}

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLFolderDisplayNode);

  /// Make sure display node and transform node are present and valid
  void SetScene(vtkMRMLScene* scene) override;

  /// Process events from the MRML scene
  void ProcessMRMLEvents( vtkObject *caller, unsigned long event, void *callData) override;

  /// Overridden to be able to call modified on branch if necessary
  void Modified() override;

  ///@{
  /// Set/Get whether to apply display properties on the whole branch
  /// \sa ApplyDisplayPropertiesOnBranch
  void SetApplyDisplayPropertiesOnBranch(bool on);
  vtkGetMacro(ApplyDisplayPropertiesOnBranch, bool);
  ///@}

  /// Call modified event on display nodes in branch that allow overriding
  /// \sa FolderDisplayOverrideAllowed
  void ChildDisplayNodesModified();

  /// Get display node from hierarchy that overrides the display properties of a given
  /// displayable node.
  /// Note: Subject hierarchy folders have display nodes associated to be able to
  ///   override display properties of a branch on the request of the user
  static vtkMRMLDisplayNode* GetOverridingHierarchyDisplayNode(vtkMRMLDisplayableNode* node);

  /// Get visibility determined by the hierarchy.
  /// Visibility is influenced by the hierarchy regardless the fact whether there is override
  /// or not. Visibility defined by hierarchy is off if any of the ancestors is explicitly hidden.
  static bool GetHierarchyVisibility(vtkMRMLDisplayableNode* node);

  /// Get opacity determined by the hierarchy.
  /// Opacity is influenced by the hierarchy regardless the fact whether there is override
  // or not. Opacity defined by hierarchy is the product of the ancestors' opacities.
  static double GetHierarchyOpacity(vtkMRMLDisplayableNode* node);

protected:
  vtkMRMLFolderDisplayNode();
  ~vtkMRMLFolderDisplayNode() override;
  vtkMRMLFolderDisplayNode(const vtkMRMLFolderDisplayNode&);
  void operator=(const vtkMRMLFolderDisplayNode&);

private:
  /// Flag determining whether the display node is to be applied on the
  /// displayable nodes in the subject hierarchy branch under the item that
  /// has the display node associated.
  bool ApplyDisplayPropertiesOnBranch{false};
};

#endif
