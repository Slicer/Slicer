/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
///  vtkMRMLVolumeSequenceStorageNode - MRML node that can read/write
///  a Sequence node containing volumes in a single NRRD file
///

#ifndef __vtkMRMLVolumeSequenceStorageNode_h
#define __vtkMRMLVolumeSequenceStorageNode_h

#include "vtkMRML.h"

#include "vtkMRMLNRRDStorageNode.h"
#include <string>

/// \ingroup Slicer_QtModules_Sequences
class VTK_MRML_EXPORT vtkMRMLVolumeSequenceStorageNode : public vtkMRMLNRRDStorageNode
{
  public:

  static vtkMRMLVolumeSequenceStorageNode *New();
  vtkTypeMacro(vtkMRMLVolumeSequenceStorageNode,vtkMRMLNRRDStorageNode);

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override {return "VolumeSequenceStorage";};

  /// Return true if the node can be read in.
  bool CanReadInReferenceNode(vtkMRMLNode *refNode) override;

  /// Return true if the node can be written by using the writer.
  bool CanWriteFromReferenceNode(vtkMRMLNode* refNode) override;

  /// Write the data. Returns 1 on success, 0 otherwise.
  ///
#ifdef NRRD_CHUNK_IO_AVAILABLE
  /// The nrrd file will be formatted such as:
  /// "kinds: domain domain domain list"
  /// This order is the optimal, best perfomance, choice for streaming
  /// 3D+T data to a file using the Teem library.
#else
  /// The nrrd file will be formatted such as:
  /// "kinds: list domain domain domain"
#endif
  int WriteDataInternal(vtkMRMLNode *refNode) override;

  ///
  /// Return a default file extension for writting
  const char* GetDefaultWriteFileExtension() override;

protected:
  vtkMRMLVolumeSequenceStorageNode();
  ~vtkMRMLVolumeSequenceStorageNode() override;
  vtkMRMLVolumeSequenceStorageNode(const vtkMRMLVolumeSequenceStorageNode&);
  void operator=(const vtkMRMLVolumeSequenceStorageNode&);

  /// Does the actual reading. Returns 1 on success, 0 otherwise.
  /// Returns 0 by default (read not supported).
  /// This implementation delegates most everything to the superclass
  /// but it has an early exit if the file to be read is incompatible.
  ///
  /// It is assumed that the nrrd file is formatted such as:
  /// "kinds: list domain domain domain"
#ifdef NRRD_CHUNK_IO_AVAILABLE
  /// or
  /// "kinds: domain domain domain list"
#endif

  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Initialize all the supported write file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;
};

#endif
