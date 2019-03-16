/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLAnnotationSnapshotStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
///  vtkMRMLAnnotationSnapshotStorageNode - MRML node for model storage on disk
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk

#ifndef __vtkMRMLAnnotationSnapshotStorageNode_h
#define __vtkMRMLAnnotationSnapshotStorageNode_h

#include "vtkSlicerAnnotationsModuleMRMLExport.h"
#include "vtkMRMLStorageNode.h"

class vtkImageData;
/// \ingroup Slicer_QtModules_Annotation
class VTK_SLICER_ANNOTATIONS_MODULE_MRML_EXPORT vtkMRMLAnnotationSnapshotStorageNode
  : public vtkMRMLStorageNode
{
public:
  static vtkMRMLAnnotationSnapshotStorageNode *New();
  vtkTypeMacro(vtkMRMLAnnotationSnapshotStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "AnnotationSnapshotStorage";}

  /// Return true if the node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;
protected:
  vtkMRMLAnnotationSnapshotStorageNode();
  ~vtkMRMLAnnotationSnapshotStorageNode() override;
  vtkMRMLAnnotationSnapshotStorageNode(const vtkMRMLAnnotationSnapshotStorageNode&);
  void operator=(const vtkMRMLAnnotationSnapshotStorageNode&);

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



