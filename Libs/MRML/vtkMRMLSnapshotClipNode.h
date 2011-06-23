/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSnapshotClipNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
///  vtkMRMLSnapshotClipNode - abstract class representing a hierarchy member

#ifndef __vtkMRMLSnapshotClipNode_h
#define __vtkMRMLSnapshotClipNode_h


#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLSceneViewNode.h"

class VTK_MRML_EXPORT vtkMRMLSnapshotClipNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSnapshotClipNode *New();
  vtkTypeMacro(vtkMRMLSnapshotClipNode,vtkMRMLNode);
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
  virtual const char* GetNodeTagName() {return "SnapshotClip";};

   /// 
  /// Updates this node if it depends on other nodes 
  /// when the node is deleted in the scene
  virtual void UpdateScene(vtkMRMLScene *scene);

  /// 
  /// Add SceneSnapshot node
  void AddSceneSanpshotNode(vtkMRMLSceneViewNode * node)
  {
    this->SceneSnapshotNodes->AddItem(node);
  };

  /// 
  /// Get Numbre of SceneSnapshot nodes
  int GetNumberOfSceneSanpshotNodes()
  {
    return this->SceneSnapshotNodes->GetNumberOfItems();
  };
  /// 
  /// Get SceneSnapshot node
  vtkMRMLSceneViewNode* GetSceneSanpshotNode(int index)
  {
    return vtkMRMLSceneViewNode::SafeDownCast(this->SceneSnapshotNodes->GetItemAsObject(index));
  };

protected:
  vtkMRMLSnapshotClipNode();
  ~vtkMRMLSnapshotClipNode();
  vtkMRMLSnapshotClipNode(const vtkMRMLSnapshotClipNode&);
  void operator=(const vtkMRMLSnapshotClipNode&);

  //BTX
  std::vector< std::string > SceneSnapshotNodeIDs;
  //ETX
  vtkCollection* SceneSnapshotNodes;

};

#endif


 

