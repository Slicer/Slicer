/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLProceduralColorStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLProceduralColorStorageNode_h
#define __vtkMRMLProceduralColorStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for procedural color node storage
///
/// vtkMRMLProceduralColorStorageNode nodes describe the color storage
/// node that allows to read/write volume data from/to file using XML to describe
/// the points defined in the color transfer function
class VTK_MRML_EXPORT vtkMRMLProceduralColorStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLProceduralColorStorageNode *New();
  vtkTypeMacro(vtkMRMLProceduralColorStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "ProceduralColorStorage";};

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  vtkMRMLProceduralColorStorageNode();
  ~vtkMRMLProceduralColorStorageNode();
  vtkMRMLProceduralColorStorageNode(const vtkMRMLProceduralColorStorageNode&);
  void operator=(const vtkMRMLProceduralColorStorageNode&);

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
