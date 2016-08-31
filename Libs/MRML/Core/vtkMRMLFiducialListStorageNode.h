/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLFiducialListStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLFiducialListStorageNode_h
#define __vtkMRMLFiducialListStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for handling fiducial list storage
///
/// \deprecated Used for reading in Slicer3 fiducial lists, please use the Annotation Module MRML storage nodes
/// \sa vtkMRMLAnnotationStorageNode, vtkMRMLAnnotationFiducialsStorageNode
///
/// vtkMRMLFiducialListStorageNode nodes describe the fiducial storage
/// node that allows to read/write point data from/to file.
class VTK_MRML_EXPORT vtkMRMLFiducialListStorageNode
  : public vtkMRMLStorageNode
{
public:
  /// \deprecated Used for reading in Slicer3 fiducial lists, please use the Annotation Module MRML storage nodes
  /// \sa vtkMRMLAnnotationStorageNode, vtkMRMLAnnotationFiducialsStorageNode
  ///
  static vtkMRMLFiducialListStorageNode *New();
  vtkTypeMacro(vtkMRMLFiducialListStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "FiducialListStorage";};

  /// Get/Set the storage node version
  vtkGetMacro(Version, int);
  vtkSetMacro(Version, int);

  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);

protected:
  vtkMRMLFiducialListStorageNode();
  ~vtkMRMLFiducialListStorageNode();
  vtkMRMLFiducialListStorageNode(const vtkMRMLFiducialListStorageNode&);
  void operator=(const vtkMRMLFiducialListStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  /// the storage node version
  // version 1 has the old glyph numbering (pre svn 12553), starting at 0
  // version 2 has the new glyph numbering, starting at 1
  int Version;
};

#endif



