/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSceneSnapshotNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLSceneSnapshotNode - abstract class representing a hierarchy member

#ifndef __vtkMRMLSceneSnapshotNode_h
#define __vtkMRMLSceneSnapshotNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLDisplayableNode.h"

class VTK_MRML_EXPORT vtkMRMLSceneSnapshotNode : public vtkMRMLDisplayableNode
{
  public:
  static vtkMRMLSceneSnapshotNode *New();
  vtkTypeMacro(vtkMRMLSceneSnapshotNode,vtkMRMLDisplayableNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  /// 
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  /// 
  /// Write this node's body to a MRML file in XML format.
  virtual void WriteNodeBodyXML(ostream& of, int indent);

  /// 
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  /// 
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "SceneSnapshot";};

  /// 
  /// Updates scene nodes 
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Updates internal nodes 
  virtual void UpdateSnapshotScene(vtkMRMLScene *);

  /// 
  /// Set dependencies between this node and a child node
  /// when parsing XML file
  virtual void ProcessChildNode(vtkMRMLNode *node);

  /// 
  /// Store content of the scene
  void StoreScene();

  /// 
  /// Restore content of the scene from the node
  void RestoreScene();

  vtkGetObjectMacro ( Nodes, vtkMRMLScene );

  void SetAbsentStorageFileNames();

  bool CanApplyNonLinearTransforms() {return false;};
  void ApplyTransform(vtkMatrix4x4* vtkNotUsed(transformMatrix)) {};
  void ApplyTransform(vtkAbstractTransform* vtkNotUsed(vtkUtransform)) {};

protected:
  vtkMRMLSceneSnapshotNode();
  ~vtkMRMLSceneSnapshotNode();
  vtkMRMLSceneSnapshotNode(const vtkMRMLSceneSnapshotNode&);
  void operator=(const vtkMRMLSceneSnapshotNode&);


  vtkMRMLScene* Nodes;

};

#endif


 

