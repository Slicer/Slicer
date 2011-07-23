/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLHierarchyNode - abstract class representing a hierarchy member

#ifndef __vtkMRMLHierarchyNode_h
#define __vtkMRMLHierarchyNode_h

// MRML includes
#include "vtkMRMLNode.h"

// VTK includes
class vtkCollection;

// STD includes
#include <vector>

class VTK_MRML_EXPORT vtkMRMLHierarchyNode : public vtkMRMLNode
{
public:
  static vtkMRMLHierarchyNode *New();
  vtkTypeMacro(vtkMRMLHierarchyNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Hierarchy";};

  /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  /// 
  /// Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  /// 
  /// Associated prent MRML node
  vtkMRMLHierarchyNode* GetParentNode();

  /// 
  /// Get the top parent node in the hierarchy
  vtkMRMLHierarchyNode* GetTopParentNode();

  /// 
  /// String ID of the parent hierarchy MRML node
  virtual char* GetParentNodeID()
  {
    return GetParentNodeIDReference();
  }

  virtual void SetParentNodeID(const char* ref);

//BTX
  /// 
  /// Given this hierarchy node returns all it's children recursively. 
  void GetAllChildrenNodes(std::vector< vtkMRMLHierarchyNode *> &childrenNodes);

  /// 
  /// Given this hierarchy node returns all it's 1st level children (not recursive). 
  /// Note: Most compilers don't make a copy of the list if you call the function like that:
  /// std::vector< vtkMRMLModelHierarchyNode > children = this->GetHierarchyChildrenNodes(parent);
  std::vector< vtkMRMLHierarchyNode *> GetChildrenNodes();
//ETX

  /// Returns the number of immediate children in the hierarchy
  int GetNumberOfChildrenNodes()
  {
    return static_cast<int>(this->GetChildrenNodes().size());
  }

  /// Get n-th child node sorted in the order of their SortingValue
  vtkMRMLHierarchyNode *GetNthChildNode(int index);

  /// Get index of this node in it's parent based on the value of their SortingValue
  int GetIndexInParent();

  /// Set index of this node in it's parent based on the value of their SortingValue
  void SetIndexInParent(int index);

  /// Removes immediate children nodes, their children are reparented to this parent node.
  void RemoveHierarchyChildrenNodes();

  /// Removes all children hierarchy nodes including children of childern, etc.
  void RemoveAllHierarchyChildrenNodes();

  /// ChildNodeAddedEvent is send when a child node added to this parent
  enum
    {
      ChildNodeAddedEvent = 15550,
      ChildNodeRemovedEvent = 15551
    };
 
  //// Assocailted node methods ////////////////

  /// 
  /// String ID of the corresponding displayable MRML node
  virtual char* GetAssociatedNodeID()
  {
    return GetAssociatedNodeIDReference();
  }

  virtual void SetAssociatedNodeID(const char* ref) 
  {
    if ((this->AssociatedNodeIDReference && ref && strcmp(ref, this->AssociatedNodeIDReference)) ||
        (this->AssociatedNodeIDReference != ref))
      {
      this->SetAssociatedNodeIDReference(ref);
      this->AssociatedHierarchyIsModified(this->GetScene());
      }
  };


  /// Get node associated with this hierarchy node
  vtkMRMLNode* GetAssociatedNode();


  /// Find all associated children nodes of a specified class in the hierarchy
  /// if childClass is NULL returns all associated children nodes.
  void GetAssociateChildrendNodes(vtkCollection *children, const char* childClass=NULL);

  /// 
  /// Get Hierarchy node for a given associated node
  static vtkMRMLHierarchyNode* GetAssociatedHierarchyNode(vtkMRMLScene *scene,
                                                          const char *associatedNodeID);
  /// 
  /// Node's Sorting Value
  //vtkSetMacro(SortingValue, double);
  /// Use a method for Set because it needs to call modified on any associated
  //nodes (since the order of that associated node could have changed as well)
  void SetSortingValue(double value);
  vtkGetMacro(SortingValue, double);


  /// turn off if only want to have one child associated with this hierarchy
  /// node, as with the leaf type nodes that are pointing to a single mrml
  /// node. Used first in checking drag and drop targets. Default to true.
  vtkGetMacro(AllowMultipleChildren, int);
  vtkSetMacro(AllowMultipleChildren, int);
  vtkBooleanMacro(AllowMultipleChildren, int);

protected:
  vtkMRMLHierarchyNode();
  ~vtkMRMLHierarchyNode();
  vtkMRMLHierarchyNode(const vtkMRMLHierarchyNode&);
  void operator=(const vtkMRMLHierarchyNode&);


  /// 
  /// String ID of the parent hierarchy MRML node
  void SetParentNodeIDReference(const char* id);
  vtkGetStringMacro(ParentNodeIDReference);

  char *ParentNodeIDReference;

  /// Mark hierarchy as modified when you
  static void HierarchyIsModified(vtkMRMLScene *scene);


  ///////////////////////

  /// Mark hierarchy as modified
  static void AssociatedHierarchyIsModified(vtkMRMLScene *scene);
  /// 
  /// String ID of the associated MRML node
  char *AssociatedNodeIDReference;

  void SetAssociatedNodeIDReference(const char*);
  vtkGetStringMacro(AssociatedNodeIDReference);

  //BTX
  typedef std::map<std::string, std::vector< vtkMRMLHierarchyNode *> > HierarchyChildrenNodesType;

  static std::map< vtkMRMLScene*, HierarchyChildrenNodesType> SceneHierarchyChildrenNodes;
  static std::map< vtkMRMLScene*, unsigned long> SceneHierarchyChildrenNodesMTime;
  //ETX
  
  ////////////////////////////
  /// 
  /// Create Associated to hierarchy map, 
  /// return number of Associated hierarchy nodes
  static int UpdateAssociatedToHierarchyMap(vtkMRMLScene *scene);
  
  //BTX
  typedef std::map<std::string, vtkMRMLHierarchyNode *> AssociatedHierarchyNodesType;

  static std::map< vtkMRMLScene*, AssociatedHierarchyNodesType> SceneAssociatedHierarchyNodes;

  static std::map< vtkMRMLScene*, unsigned long> SceneAssociatedHierarchyNodesMTime;
  //ETX

  double SortingValue;

  static double MaximumSortingValue;

  void UpdateChildrenMap();

  /// is this a node that's only supposed to have one child?
  int AllowMultipleChildren;
};

#endif


 

