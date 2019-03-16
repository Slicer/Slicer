/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumePropertyStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLVolumePropertyStorageNode - MRML node for transform storage on disk
///
/// Storage nodes has methods to read/write transforms to/from disk

#ifndef __vtkMRMLVolumePropertyStorageNode_h
#define __vtkMRMLVolumePropertyStorageNode_h

// VolumeRendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLStorageNode.h"

class vtkImageData;

class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumePropertyStorageNode
  : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLVolumePropertyStorageNode *New();
  vtkTypeMacro(vtkMRMLVolumePropertyStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Transform)
  const char* GetNodeTagName() override {return "VolumePropertyStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

protected:
  vtkMRMLVolumePropertyStorageNode();
  ~vtkMRMLVolumePropertyStorageNode() override;
  vtkMRMLVolumePropertyStorageNode(const vtkMRMLVolumePropertyStorageNode&);
  void operator=(const vtkMRMLVolumePropertyStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode *refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode *refNode) override;

};

#endif
