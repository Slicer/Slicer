/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelHierarchyLogic.h,v $
  Date:      $Date: 2010-02-15 16:35:35 -0500 (Mon, 15 Feb 2010) $
  Version:   $Revision: 12142 $

=========================================================================auto=*/

#ifndef __vtkMRMLModelHierarchyLogic_h
#define __vtkMRMLModelHierarchyLogic_h

// MRML includes
#include <vtkMRMLAbstractLogic.h>
//#include <vtkMRML.h>
//#include <vtkMRMLModelNode.h>
class vtkMRMLModelHierarchyNode;
class vtkMRMLDisplayableHierarchyNode;
//#include <vtkMRMLModelHierarchyNode.h>

// STD includes
#include <cstdlib>
#include <vector>

typedef std::vector< vtkMRMLModelHierarchyNode *> vtkMRMLModelHierarchyNodeList;

/// \brief Slicer logic class for volumes manipulation.
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes.
class VTK_MRML_LOGIC_EXPORT vtkMRMLModelHierarchyLogic : public vtkMRMLAbstractLogic
{
public:
  /// The Usual vtk class functions
  static vtkMRMLModelHierarchyLogic *New();
  vtkTypeMacro(vtkMRMLModelHierarchyLogic,vtkMRMLAbstractLogic);

  ///
  /// Given model id return it's hierarchy
  /// or NULL if it does not have one
  vtkMRMLModelHierarchyNode* GetModelHierarchyNode(const char *modelNodeID);

  ///
  /// Return number of model that are in a hierarchy
  int GetNumberOfModelsInHierarchy()
    {
    this->UpdateModelToHierarchyMap();
    return (int)this->ModelHierarchyNodes.size();
    };

  ///
  /// Given model hierarchy node returns all it's children recursively.
  void GetHierarchyChildrenNodes(vtkMRMLModelHierarchyNode *parentNode,
                                 vtkMRMLModelHierarchyNodeList &childrenNodes);
  ///
  /// Given model hierarchy node returns all it's 1st level children (not recursive).
  /// Note: Most compilers don't make a copy of the list if you call the function like that:
  /// std::vector< vtkMRMLModelHierarchyNode > children = logic->GetHierarchyChildrenNodes(parent);
  vtkMRMLModelHierarchyNodeList GetHierarchyChildrenNodes(vtkMRMLModelHierarchyNode *parentNode);

  ///
  /// Call this to update the cache when hierarchy is modified.
  void HierarchyIsModified()
    {
    ModelHierarchyNodesMTime = 0;
    HierarchyChildrenNodesMTime = 0;
    }

  ///
  /// Set visibility on all children of the node recursively
  /// If displayNodeClass is not 0 apply only to the disaply nodes of that class,
  /// otherwise to all display nodes
  static void SetChildrenVisibility(vtkMRMLDisplayableHierarchyNode *displayableHierarchyNode,
                                   const char *displayableNodeClass, const char *displayNodeClass,
                                   int visibility);

  ///
  /// Children visibility setting will be performed in batch processing mode
  /// if the number of children nodes are more than this number
  /// (the same value is used by all the logic instances)
  vtkSetMacro(ChildrenVisibilitySetBatchUpdateThreshold, int);
  vtkGetMacro(ChildrenVisibilitySetBatchUpdateThreshold, int);

protected:
  vtkMRMLModelHierarchyLogic();
  ~vtkMRMLModelHierarchyLogic();
  vtkMRMLModelHierarchyLogic(const vtkMRMLModelHierarchyLogic&);
  void operator=(const vtkMRMLModelHierarchyLogic&);

  /// Reimplemented to observe the scene
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Delete the hierarchy node when a model is removed from the scene
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* removedNode);

  ///
  /// Create model to hierarchy map,
  /// return number of model hierarchy nodes
  int UpdateModelToHierarchyMap();

  void UpdateHierarchyChildrenMap();

  std::map<std::string, vtkMRMLModelHierarchyNode *> ModelHierarchyNodes;
  typedef std::map<std::string, std::vector< vtkMRMLModelHierarchyNode *> > HierarchyChildrenNodesType;
  HierarchyChildrenNodesType HierarchyChildrenNodes;

  vtkMTimeType ModelHierarchyNodesMTime;
  vtkMTimeType HierarchyChildrenNodesMTime;

  static int ChildrenVisibilitySetBatchUpdateThreshold;
};

#endif
