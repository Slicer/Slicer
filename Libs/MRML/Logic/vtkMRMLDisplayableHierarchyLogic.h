/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDisplayableHierarchyLogic.h,v $
  Date:      $Date: 2010-02-15 16:35:35 -0500 (Mon, 15 Feb 2010) $
  Version:   $Revision: 12142 $

=========================================================================auto=*/

#ifndef __vtkMRMLDisplayableHierarchyLogic_h
#define __vtkMRMLDisplayableHierarchyLogic_h

// MRML includes
#include <vtkMRMLAbstractLogic.h>
class vtkMRMLDisplayableNode;
class vtkMRMLDisplayableHierarchyNode;

// STD includes
#include <cstdlib>
#include <vector>

typedef std::vector< vtkMRMLDisplayableHierarchyNode *> vtkMRMLDisplayableHierarchyNodeList;

/// \brief Slicer logic class for hierarchy manipulation.
///
/// This class manages the logic associated with displayable hierarchy nodes.
class VTK_MRML_LOGIC_EXPORT vtkMRMLDisplayableHierarchyLogic : public vtkMRMLAbstractLogic
{
  public:

  /// The Usual vtk class functions
  static vtkMRMLDisplayableHierarchyLogic *New();
  vtkTypeMacro(vtkMRMLDisplayableHierarchyLogic,vtkMRMLAbstractLogic);

  /// Create a 1:1 displayable hierarchy node for this node, add it to the
  /// scene and return the id, null on failure
  char *AddDisplayableHierarchyNodeForNode(vtkMRMLDisplayableNode *node);

  /// Create displayable hierarchy nodes as needed to make the child node a
  /// child of the parent node (may need to add 1:1 hierarchy nodes for both
  /// parent and child). Return true on success, false on failure.
  bool AddChildToParent(vtkMRMLDisplayableNode *child, vtkMRMLDisplayableNode *parent);

  /// Delete the passed hierarchy node and all children hierarchy nodes and
  /// the associated nodes to which they point. Return true on success, false
  /// on failure. Gets the mrml scene from the node.
  bool DeleteHierarchyNodeAndChildren(vtkMRMLDisplayableHierarchyNode *hnode);

protected:
  vtkMRMLDisplayableHierarchyLogic();
  ~vtkMRMLDisplayableHierarchyLogic() override;
  vtkMRMLDisplayableHierarchyLogic(const vtkMRMLDisplayableHierarchyLogic&);
  void operator=(const vtkMRMLDisplayableHierarchyLogic&);

  /// Reimplemented to observe the scene
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Delete the hierarchy node when a node is removed from the scene
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* removedNode) override;

};

#endif
