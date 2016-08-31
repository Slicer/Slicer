/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLUnstructuredGridStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLUnstructuredGridStorageNode_h
#define __vtkMRMLUnstructuredGridStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for fiberBundle storage on disk.
///
/// The storage node has methods to read/write vtkPolyData to/from disk.
class VTK_MRML_EXPORT vtkMRMLUnstructuredGridStorageNode
  : public vtkMRMLStorageNode
{
public:
  static vtkMRMLUnstructuredGridStorageNode *New();
  vtkTypeMacro(vtkMRMLUnstructuredGridStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "UnstructuredGridStorage";};

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  vtkMRMLUnstructuredGridStorageNode();
  ~vtkMRMLUnstructuredGridStorageNode(){};
  vtkMRMLUnstructuredGridStorageNode(const vtkMRMLUnstructuredGridStorageNode&);
  void operator=(const vtkMRMLUnstructuredGridStorageNode&);

  /// Initialize all the supported read file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

};

#endif
