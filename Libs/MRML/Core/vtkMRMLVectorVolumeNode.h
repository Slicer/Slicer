/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVectorVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLVectorVolumeNode_h
#define __vtkMRMLVectorVolumeNode_h

#include "vtkMRMLTensorVolumeNode.h"
class vtkMRMLVolumeArchetypeStorageNode;
class vtkMRMLVectorVolumeDisplayNode;

/// \brief MRML node for representing a vector volume (image stack).
///
/// Volume with vector pixel type.
class VTK_MRML_EXPORT vtkMRMLVectorVolumeNode : public vtkMRMLTensorVolumeNode
{
  public:
  static vtkMRMLVectorVolumeNode *New();
  vtkTypeMacro(vtkMRMLVectorVolumeNode,vtkMRMLTensorVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "VectorVolume";};

  ///
  /// Associated display MRML node
  virtual vtkMRMLVectorVolumeDisplayNode* GetVectorVolumeDisplayNode();

  ///
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

  ///
  /// Create and observe default display node
  virtual void CreateDefaultDisplayNodes();

protected:
  vtkMRMLVectorVolumeNode();
  ~vtkMRMLVectorVolumeNode();
  vtkMRMLVectorVolumeNode(const vtkMRMLVectorVolumeNode&);
  void operator=(const vtkMRMLVectorVolumeNode&);

};

#endif
