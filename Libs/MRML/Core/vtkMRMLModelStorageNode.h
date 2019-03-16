/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLModelStorageNode_h
#define __vtkMRMLModelStorageNode_h

#include "vtkMRMLStorageNode.h"

class vtkMRMLModelNode;

/// \brief MRML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
class VTK_MRML_EXPORT vtkMRMLModelStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLModelStorageNode *New();
  vtkTypeMacro(vtkMRMLModelStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override  {return "ModelStorage";}

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Return true if the reference node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

protected:
  vtkMRMLModelStorageNode();
  ~vtkMRMLModelStorageNode() override;
  vtkMRMLModelStorageNode(const vtkMRMLModelStorageNode&);
  void operator=(const vtkMRMLModelStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Get data node that is associated with this storage node
  vtkMRMLModelNode* GetAssociatedDataNode();

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;

};

#endif
