/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLColorTableStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

#ifndef __vtkMRMLColorTableStorageNode_h
#define __vtkMRMLColorTableStorageNode_h

#include "vtkMRMLStorageNode.h"

/// \brief MRML node for representing a volume storage.
///
/// vtkMRMLColorTableStorageNode nodes describe the archetybe based volume storage
/// node that allows to read/write volume data from/to file using generic ITK mechanism.
class VTK_MRML_EXPORT vtkMRMLColorTableStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLColorTableStorageNode *New();
  vtkTypeMacro(vtkMRMLColorTableStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "ColorTableStorage";};

  /// Return true if the node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode* refNode);

protected:
  vtkMRMLColorTableStorageNode();
  ~vtkMRMLColorTableStorageNode();
  vtkMRMLColorTableStorageNode(const vtkMRMLColorTableStorageNode&);
  void operator=(const vtkMRMLColorTableStorageNode&);

  /// Initialize all the supported read file types
  virtual void InitializeSupportedReadFileTypes();

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  /// maximum valid number of colours to read in
  int MaximumColorID;

};

#endif



