/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLHierarchyStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLHierarchyStorageNode_h
#define __vtkMRMLHierarchyStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for representing a no-op hierarchy storage.
///
/// vtkMRMLHierarchyStorageNode nodes describe the storage that is a place
/// holder for hierarchy nodes that don't need to write anything to file,
/// subclasses should reimplement ReadData and WriteData
class VTK_MRML_EXPORT vtkMRMLHierarchyStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLHierarchyStorageNode *New();
  vtkTypeMacro(vtkMRMLHierarchyStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  // Description:
  // Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "HierarchyStorage";}

  /// Return true if reference node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;
protected:
  vtkMRMLHierarchyStorageNode();
  ~vtkMRMLHierarchyStorageNode() override;
  vtkMRMLHierarchyStorageNode(const vtkMRMLHierarchyStorageNode&);
  void operator=(const vtkMRMLHierarchyStorageNode&);

  // Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  // Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  // Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  // Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;
};

#endif
