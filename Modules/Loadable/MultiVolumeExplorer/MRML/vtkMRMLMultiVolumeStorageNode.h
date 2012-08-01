/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLNRRDStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
///  vtkMRMLMultiVolumeStorageNode - MRML node for representing a
///  multi value volume storage
/// 

#ifndef __vtkMRMLMultiVolumeStorageNode_h
#define __vtkMRMLMultiVolumeStorageNode_h

// MultiVolumeExplorer includes
#include <vtkSlicerMultiVolumeExplorerModuleMRMLExport.h>

#include "vtkMRMLNRRDStorageNode.h"

/// \ingroup Slicer_QtModules_MultiVolumeNode
class VTK_SLICER_MULTIVOLUMEEXPLORER_MODULE_MRML_EXPORT vtkMRMLMultiVolumeStorageNode : public vtkMRMLNRRDStorageNode
{
  public:

  static vtkMRMLMultiVolumeStorageNode *New();
  vtkTypeMacro(vtkMRMLMultiVolumeStorageNode,vtkMRMLNRRDStorageNode);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// 
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "MultiVolumeStorage";};

  /// Return true if the node can be read in.
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLMultiVolumeStorageNode();
  ~vtkMRMLMultiVolumeStorageNode();
  vtkMRMLMultiVolumeStorageNode(const vtkMRMLMultiVolumeStorageNode&);
  void operator=(const vtkMRMLMultiVolumeStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is not a
  /// MultiVolume, e.g. the file is a NRRD but not a MultiVolume NRRD.
  virtual int ReadDataInternal(vtkMRMLNode* refNode);
};

#endif
