/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLModelHierarchyNode_h
#define __vtkMRMLModelHierarchyNode_h

#include "vtkMRMLDisplayableHierarchyNode.h"
class vtkMRMLModelDisplayNode;
class vtkMRMLModelNode;

/// \brief MRML node to represent a hierarchyu of models.
class VTK_MRML_EXPORT vtkMRMLModelHierarchyNode : public vtkMRMLDisplayableHierarchyNode
{
public:
  static vtkMRMLModelHierarchyNode *New();
  vtkTypeMacro(vtkMRMLModelHierarchyNode,vtkMRMLDisplayableHierarchyNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

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
  /// Get node XML tag name (like Volume, ModelHierarchy)
  virtual const char* GetNodeTagName() {return "ModelHierarchy";};

   ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences();

  ///
  /// Finds the model node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  ///
  /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  ///
  /// String ID of the model MRML node
  void SetModelNodeID(const char* id)
  {
    this->SetDisplayableNodeID(id);
  }

  char *GetModelNodeID()
  {
    return this->GetDisplayableNodeID();
  }

  /// Need this for tcl wrapping to call ReferenceStringMacro methods
  void SetModelNodeIDReference(const char* ref) {
    this->SetModelNodeID(ref);
  };


  ///
  /// Get associated model MRML node
  vtkMRMLModelNode* GetModelNode();

  ///
  /// Get associated display MRML node
  vtkMRMLModelDisplayNode* GetModelDisplayNode();


  ///
  /// Get the first parent node in hierarchy which is not expanded
  vtkMRMLModelHierarchyNode* GetCollapsedParentNode();

  ///
  /// Find all child model nodes in the hierarchy
  void GetChildrenModelNodes(vtkCollection *models);

  ///
  /// alternative method to propagate events generated in Display nodes
  virtual void ProcessMRMLEvents ( vtkObject * /*caller*/,
                                   unsigned long /*event*/,
                                   void * /*callData*/ );


protected:
  vtkMRMLModelHierarchyNode();
  ~vtkMRMLModelHierarchyNode();
  vtkMRMLModelHierarchyNode(const vtkMRMLModelHierarchyNode&);
  void operator=(const vtkMRMLModelHierarchyNode&);


  /// Data

  vtkMRMLModelDisplayNode *ModelDisplayNode;

};

#endif
