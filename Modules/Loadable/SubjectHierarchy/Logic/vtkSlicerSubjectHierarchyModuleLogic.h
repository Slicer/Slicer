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

// .NAME vtkSlicerSubjectHierarchyModuleLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerSubjectHierarchyModuleLogic_h
#define __vtkSlicerSubjectHierarchyModuleLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerSubjectHierarchyModuleLogicExport.h"

class vtkMRMLSubjectHierarchyNode;
class vtkMRMLTransformNode;

/// \ingroup Slicer_QtModules_SubjectHierarchy
class VTK_SLICER_SUBJECTHIERARCHY_LOGIC_EXPORT vtkSlicerSubjectHierarchyModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  /// Postfix added to cloned node name by default
  static const char* CLONED_NODE_NAME_POSTFIX;

public:
  static vtkSlicerSubjectHierarchyModuleLogic *New();
  vtkTypeMacro(vtkSlicerSubjectHierarchyModuleLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

public:
  /// Place series in subject hierarchy. Create subject and study node if needed
  /// \return Series subject hierarchy node of the inserted series
  static vtkIdType InsertDicomSeriesInHierarchy(
    vtkMRMLSubjectHierarchyNode* shNode, const char* subjectId, const char* studyInstanceUID, const char* seriesInstanceUID );

  /// Determine if two subject hierarchy items are in the same branch (share the same parent)
  /// \param shNode Subject hierarchy to search in
  /// \param item1 First item to check
  /// \param item2 Second item to check
  /// \param lowestCommonLevel Lowest level on which they have to share an ancestor
  /// \return The common parent if the two items share a parent  on the specified level, INVALID_ITEM_ID otherwise
  static vtkIdType AreItemsInSameBranch(
    vtkMRMLSubjectHierarchyNode* shNode, vtkIdType item1, vtkIdType item2, std::string lowestCommonLevel );
  /// Determine if two data nodes are in the same branch in subject hierarchy (share the same parent)
  /// \param node1 First node to check. Can be subject hierarchy node or a node associated with one
  /// \param node2 Second node to check
  /// \param lowestCommonLevel Lowest level on which they have to share an ancestor
  /// \return The common parent if the two nodes share a parent on the specified level, INVALID_ITEM_ID otherwise
  static vtkIdType AreNodesInSameBranch(
    vtkMRMLNode* node1, vtkMRMLNode* node2, std::string lowestCommonLevel );

  /// Determine if a tag name is a patient tag (not attribute, but tag - without prefix!)
  static bool IsPatientTag(std::string tagName);

  /// Determine if a tag name is a study tag (not attribute, but tag - without prefix!)
  static bool IsStudyTag(std::string tagName);

  /// Apply transform node as parent transform on subject hierarchy node and on all children, recursively
  /// \param shNode Subject hierarchy where item can be found
  /// \param itemID Subject hierarchy item defining branch to apply transform on
  /// \param transformNode Transform node to apply. If nullptr, then any existing transform is removed
  /// \param hardenExistingTransforms Mode of handling already transformed nodes. If true (default), then the possible parent transforms
  ///   of target nodes are hardened before applying the specified transform. If false, then the already applied parent transforms are
  ///   transformed with the specified transform (Note: this latter approach may result in unwanted transformations of other nodes)
  static void TransformBranch(
    vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemID, vtkMRMLTransformNode* transformNode, bool hardenExistingTransforms=true);

  /// Harden transform on subject hierarchy item and on all children, recursively
  /// \param shNode Subject hierarchy where item can be found
  /// \param itemID Subject hierarchy item defining branch to harden transform on
  static void HardenTransformOnBranch(vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemID);

  /// Clone subject hierarchy node, the associated data node, and its display and storage nodes
  /// \param itemID Subject hierarchy item to clone
  /// \param name Custom name. If omitted, then default postfix is added from \sa node
  /// \return Clone subject hierarchy node
  static vtkIdType CloneSubjectHierarchyItem(
    vtkMRMLSubjectHierarchyNode* shNode, vtkIdType itemID, const char* name=nullptr );

  /// Convenience function to get subject hierarchy node from the logic
  vtkMRMLSubjectHierarchyNode* GetSubjectHierarchyNode();

protected:
  /// Called each time a new scene is set
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Called every time the scene has been significantly changed.
  void UpdateFromMRMLScene() override;

protected:
  vtkSlicerSubjectHierarchyModuleLogic();
  ~vtkSlicerSubjectHierarchyModuleLogic() override;

private:
  vtkSlicerSubjectHierarchyModuleLogic(const vtkSlicerSubjectHierarchyModuleLogic&) = delete;
  void operator=(const vtkSlicerSubjectHierarchyModuleLogic&) = delete;
};

#endif
