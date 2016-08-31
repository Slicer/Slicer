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
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "DoubleArrayStorage";};

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLDoubleArrayStorageNode();
  ~vtkMRMLDoubleArrayStorageNode();
  vtkMRMLDoubleArrayStorageNode(const vtkMRMLDoubleArrayStorageNode&);
  void operator=(const vtkMRMLDoubleArrayStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

};

#endif



