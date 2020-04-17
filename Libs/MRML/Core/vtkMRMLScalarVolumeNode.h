/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLScalarVolumeNode_h
#define __vtkMRMLScalarVolumeNode_h

// MRML includes
#include "vtkMRMLVolumeNode.h"
class vtkMRMLScalarVolumeDisplayNode;
class vtkCodedEntry;

/// \brief MRML node for representing a volume (image stack).
///
/// Volume nodes describe data sets that can be thought of as stacks of 2D
/// images that form a 3D volume. Volume nodes contain only the image data,
/// where it is store on disk and how to read the files is controlled by
/// the volume storage node, how to render the data (window and level) is
/// controlled by the volume display nodes. Image information is extracted
/// from the image headers (if they exist) at the time the MRML file is
/// generated.
/// Consequently, MRML files isolate MRML browsers from understanding how
/// to read the myriad of file formats for medical data.
class VTK_MRML_EXPORT vtkMRMLScalarVolumeNode : public vtkMRMLVolumeNode
{
  public:
  static vtkMRMLScalarVolumeNode *New();
  vtkTypeMacro(vtkMRMLScalarVolumeNode,vtkMRMLVolumeNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  ///
  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentDefaultMacro(vtkMRMLScalarVolumeNode);

  ///
  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override {return "Volume";}

  ///
  /// Make a 'None' volume node with blank image data
  static void CreateNoneNode(vtkMRMLScene *scene);

  ///
  /// Associated display MRML node
  virtual vtkMRMLScalarVolumeDisplayNode* GetScalarVolumeDisplayNode();

  ///
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

  /// Creates the most appropriate storage node class for storing a sequence of these nodes.
  vtkMRMLStorageNode* CreateDefaultSequenceStorageNode() override;

  /// Measured quantity of voxel values, specified as a standard coded entry.
  /// For example: (DCM, 112031, "Attenuation Coefficient")
  void SetVoxelValueQuantity(vtkCodedEntry*);
  vtkGetObjectMacro(VoxelValueQuantity, vtkCodedEntry);

  /// Measurement unit of voxel value quantity, specified as a standard coded entry.
  /// For example: (UCUM, [hnsf'U], "Hounsfield unit")
  /// It stores a single unit. Plural (units) name is chosen to be consistent
  /// with nomenclature in the DICOM standard.
  void SetVoxelValueUnits(vtkCodedEntry*);
  vtkGetObjectMacro(VoxelValueUnits, vtkCodedEntry);

protected:
  vtkMRMLScalarVolumeNode();
  ~vtkMRMLScalarVolumeNode() override;
  vtkMRMLScalarVolumeNode(const vtkMRMLScalarVolumeNode&);
  void operator=(const vtkMRMLScalarVolumeNode&);

  vtkCodedEntry* VoxelValueQuantity{nullptr};
  vtkCodedEntry* VoxelValueUnits{nullptr};
};

#endif
