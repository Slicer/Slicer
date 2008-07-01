/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSceneSnapshotNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLSceneSnapshotNode - abstract class representing a hierarchy member

#ifndef __vtkMRMLSceneSnapshotNode_h
#define __vtkMRMLSceneSnapshotNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"

class VTK_MRML_EXPORT vtkMRMLSceneSnapshotNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSceneSnapshotNode *New();
  vtkTypeMacro(vtkMRMLSceneSnapshotNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Write this node's body to a MRML file in XML format.
  virtual void WriteNodeBodyXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "SceneSnapshot";};

  // Description:
  // Updates scene nodes 
  virtual void UpdateScene(vtkMRMLScene *) {};

  // Description:
  // Updates internal nodes 
  virtual void UpdateSnapshotScene(vtkMRMLScene *);

  // Description:
  // Set dependencies between this node and a child node
  // when parsing XML file
  virtual void ProcessChildNode(vtkMRMLNode *node);

  // Description:
  // Store content of the scene
  void StoreScene();

  // Description:
  // Restore content of the scene from the node
  void RestoreScene();

protected:
  vtkMRMLSceneSnapshotNode();
  ~vtkMRMLSceneSnapshotNode();
  vtkMRMLSceneSnapshotNode(const vtkMRMLSceneSnapshotNode&);
  void operator=(const vtkMRMLSceneSnapshotNode&);


  vtkMRMLScene* Nodes;

};

#endif


 

