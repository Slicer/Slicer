/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLHierarchyNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLHierarchyNode - abstract class representing a hierarchy member

#ifndef __vtkMRMLHierarchyNode_h
#define __vtkMRMLHierarchyNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLHierarchyNode : public vtkMRMLNode
{
  public:
  static vtkMRMLHierarchyNode *New(){return NULL;};
  vtkTypeMacro(vtkMRMLHierarchyNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance() = 0;

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() = 0;

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences();

  // Description:
  // Observe the reference transform node
  virtual void UpdateScene(vtkMRMLScene *scene);

  // Description:
  // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID);

  // Description:
  // String ID of the parent hierarchy MRML node
  vtkSetReferenceStringMacro(ParentNodeID);
  vtkGetStringMacro(ParentNodeID);

  // Description:
  // Associated prent MRML node
  vtkMRMLHierarchyNode* GetParentNode();

  // Need this for tcl wrapping to call ReferenceStringMacro methods
  void SetParentNodeIDReference(const char* ref) {
    this->SetParentNodeID(ref);
  };


protected:
  vtkMRMLHierarchyNode();
  ~vtkMRMLHierarchyNode();
  vtkMRMLHierarchyNode(const vtkMRMLHierarchyNode&);
  void operator=(const vtkMRMLHierarchyNode&);


  char *ParentNodeID;

};

#endif


 

