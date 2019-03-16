/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLDoubleArrayStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLDoubleArrayStorageNode_h
#define __vtkMRMLDoubleArrayStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for representing a volume storage
///
/// vtkMRMLDoubleArrayStorageNode nodes describe the fiducial storage
/// node that allows to read/write point data from/to file.
class VTK_MRML_EXPORT vtkMRMLDoubleArrayStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLDoubleArrayStorageNode *New();
  vtkTypeMacro(vtkMRMLDoubleArrayStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "DoubleArrayStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

protected:
  vtkMRMLDoubleArrayStorageNode();
  ~vtkMRMLDoubleArrayStorageNode() override;
  vtkMRMLDoubleArrayStorageNode(const vtkMRMLDoubleArrayStorageNode&);
  void operator=(const vtkMRMLDoubleArrayStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;

};

#endif



