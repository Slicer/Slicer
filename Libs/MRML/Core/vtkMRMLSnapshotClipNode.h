/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSnapshotClipNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLSnapshotClipNode_h
#define __vtkMRMLSnapshotClipNode_h

// MRML includes
#include "vtkMRMLNode.h"
class vtkMRMLSceneViewNode;

// VTK includes
class vtkCollection;

// STD includes
#include <vector>

/// \brief Abstract class representing a hierarchy member.
class VTK_MRML_EXPORT vtkMRMLSnapshotClipNode : public vtkMRMLNode
{
  public:
  static vtkMRMLSnapshotClipNode *New();
  vtkTypeMacro(vtkMRMLSnapshotClipNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "SnapshotClip";}

   ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  void UpdateScene(vtkMRMLScene *scene) override;

  ///
  /// Add SceneSnapshot node
  void AddSceneSnapshotNode(vtkMRMLSceneViewNode * node);

  ///
  /// Get Numbre of SceneSnapshot nodes
  int GetNumberOfSceneSnapshotNodes();
  ///
  /// Get SceneSnapshot node
  vtkMRMLSceneViewNode* GetSceneSnapshotNode(int index);

protected:
  vtkMRMLSnapshotClipNode();
  ~vtkMRMLSnapshotClipNode() override;
  vtkMRMLSnapshotClipNode(const vtkMRMLSnapshotClipNode&);
  void operator=(const vtkMRMLSnapshotClipNode&);

  std::vector< std::string > SceneSnapshotNodeIDs;
  vtkCollection* SceneSnapshotNodes;

};

#endif
