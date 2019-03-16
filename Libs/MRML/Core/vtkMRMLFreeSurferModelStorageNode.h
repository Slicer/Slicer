/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFreeSurferModelStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLFreeSurferModelStorageNode_h
#define __vtkMRMLFreeSurferModelStorageNode_h

#include "vtkMRMLModelStorageNode.h"

/// \brief MRML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk
/// It uses the superclass vtkMRMLModelStorageNode for writing
/// (same file format supported).
class VTK_MRML_EXPORT vtkMRMLFreeSurferModelStorageNode : public vtkMRMLModelStorageNode
{
  public:
  static vtkMRMLFreeSurferModelStorageNode *New();
  vtkTypeMacro(vtkMRMLFreeSurferModelStorageNode,vtkMRMLModelStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Read node attributes from XML file
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Copy data from a  referenced node's filename to new location.
  /// NOTE: use this instead of Write Data in the Remote IO Pipeline
  /// until FreeSurferModel Writers are available.
  virtual int CopyData(vtkMRMLNode *refNode, const char *newFileName);

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override  {return "FreeSurferModelStorage";};

  ///
  /// Control use of the triangle stipper when reading the polydata
  vtkGetMacro(UseStripper, int);
  vtkSetMacro(UseStripper, int);

protected:
  vtkMRMLFreeSurferModelStorageNode();
  ~vtkMRMLFreeSurferModelStorageNode() override;
  vtkMRMLFreeSurferModelStorageNode(const vtkMRMLFreeSurferModelStorageNode&);
  void operator=(const vtkMRMLFreeSurferModelStorageNode&);

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  int UseStripper;
};

#endif



