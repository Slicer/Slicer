/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLSceneViewStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLSceneViewStorageNode_h
#define __vtkMRMLSceneViewStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
class VTK_MRML_EXPORT vtkMRMLSceneViewStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLSceneViewStorageNode *New();
  vtkTypeMacro(vtkMRMLSceneViewStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "SceneViewStorage";}

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

protected:
  vtkMRMLSceneViewStorageNode();
  ~vtkMRMLSceneViewStorageNode() override;
  vtkMRMLSceneViewStorageNode(const vtkMRMLSceneViewStorageNode&);
  void operator=(const vtkMRMLSceneViewStorageNode&);

  ///
  /// Read data and set it in the referenced node
  /// NOTE: Subclasses should implement this method
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  ///
  /// Write data from a  referenced node
  /// NOTE: Subclasses should implement this method
  int WriteDataInternal(vtkMRMLNode *refNode) override;

};

#endif
