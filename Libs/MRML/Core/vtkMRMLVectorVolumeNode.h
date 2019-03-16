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
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  ///
  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode *node) override;

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "VectorVolume";}

  ///
  /// Associated display MRML node
  virtual vtkMRMLVectorVolumeDisplayNode* GetVectorVolumeDisplayNode();

  ///
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

protected:
  vtkMRMLVectorVolumeNode();
  ~vtkMRMLVectorVolumeNode() override;
  vtkMRMLVectorVolumeNode(const vtkMRMLVectorVolumeNode&);
  void operator=(const vtkMRMLVectorVolumeNode&);

};

#endif
