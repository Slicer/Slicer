/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFreeSurferModelOverlayStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLFreeSurferModelOverlayStorageNode_h
#define __vtkMRMLFreeSurferModelOverlayStorageNode_h

#include "vtkMRMLModelStorageNode.h"

/// \brief MRML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
class VTK_MRML_EXPORT vtkMRMLFreeSurferModelOverlayStorageNode
  : public vtkMRMLModelStorageNode
{
public:
  static vtkMRMLFreeSurferModelOverlayStorageNode *New();
  vtkTypeMacro(vtkMRMLFreeSurferModelOverlayStorageNode,vtkMRMLModelStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Copy data from a  referenced node's filename to new location.
  /// NOTE: use this instead of Write Data in the Remote IO Pipeline
  /// until FreeSurferModel Writers are available.
  virtual int CopyData(vtkMRMLNode *refNode, const char *newFileName);

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FreeSurferModelOverlayStorage";};

  /// Return true if reference node can be written from
  virtual bool CanWriteFromReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLFreeSurferModelOverlayStorageNode();
  ~vtkMRMLFreeSurferModelOverlayStorageNode();
  vtkMRMLFreeSurferModelOverlayStorageNode(const vtkMRMLFreeSurferModelOverlayStorageNode&);
  void operator=(const vtkMRMLFreeSurferModelOverlayStorageNode&);

  /// Initialize all the supported read file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  ///
  /// Read data and set it in the referenced node
  /// NOTE: Subclasses should implement this method
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  ///
  /// Write data from a  referenced node
  /// NOTE: Subclasses should implement this method
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  std::string GetColorNodeIDFromExtension(const std::string& extension);
  std::string GetColorNodeIDFromType(int type);
};

#endif



