/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLModelStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLModelStorageNode_h
#define __vtkMRMLModelStorageNode_h

#include "vtkMRMLStorageNode.h"

class vtkMRMLModelNode;
class vtkPointSet;

/// \brief MRML node for model storage on disk.
///
/// Storage nodes has methods to read/write vtkPolyData to/from disk.
///
/// Coordinate system conversion (RAS/LPS): Slicer stores mesh point coordinates internally in the
/// RAS coordinate system, while most model files use LPS. A model file may specify its coordinate
/// system by a "SPACE=RAS" or "SPACE=LPS" specification, stored in the file header comment
/// (STL, OBJ, PLY, legacy VTK) or in a "SPACE" field data array (VTP, VTU). When a file is read,
/// this specification (if found) overrides the CoordinateSystem property of the node, and if the
/// file is in LPS then the mesh is converted to RAS by flipping the sign of the first two
/// coordinate axes. If no coordinate system specification is found in the file then the current
/// value of the CoordinateSystem property is used (LPS by default; RAS when importing legacy scenes,
/// which were saved before the coordinate system was written into model files). When a file is
/// written, the mesh is converted from RAS to the coordinate system chosen in the CoordinateSystem
/// property and the "SPACE=..." specification is written into the file header.
///
/// Automatic unit scaling: some applications (for example, Materialise Mimics and 3-matic) write
/// the length unit of the vertex coordinates into the OBJ file header as a comment, such as
/// "vertex coordinates are measured in units, where 1 unit = 1000.000000 mm". When an OBJ file
/// containing this specification is read, the point coordinates are automatically multiplied by the
/// specified scale to convert them to millimeters (the length unit used internally in Slicer).
/// Only "mm" unit is supported; if any other unit or a non-positive scale is specified then a
/// warning is logged and the coordinates are loaded without unit conversion.
class VTK_MRML_EXPORT vtkMRMLModelStorageNode : public vtkMRMLStorageNode
{
public:
  static vtkMRMLModelStorageNode* New();
  vtkTypeMacro(vtkMRMLModelStorageNode, vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  ///
  /// Get node XML tag name (like Storage, Model)
  const char* GetNodeTagName() override { return "ModelStorage"; }

  /// Read node attributes from XML file
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Return true if the reference node can be read in
  bool CanReadInReferenceNode(vtkMRMLNode* refNode) override;

  /// Get/Set flag that controls if points are to be written in various coordinate systems
  vtkSetClampMacro(CoordinateSystem, int, 0, vtkMRMLStorageNode::CoordinateSystemType_Last - 1);
  vtkGetMacro(CoordinateSystem, int);
  static const char* GetCoordinateSystemAsString(int id);
  static int GetCoordinateSystemFromString(const char* name);

  /// Helper function that can convert a mesh (polydata, unstructured grid, or even just a point cloud)
  /// between RAS and LPS coordinate system.
  static void ConvertBetweenRASAndLPS(vtkPointSet* inputMesh, vtkPointSet* outputMesh);

  /// Helper function that scales point coordinates of a mesh (polydata, unstructured grid, or even just a point cloud)
  /// by the given scale factors along the x, y, z axes.
  static void ScalePointSet(vtkPointSet* inputMesh, vtkPointSet* outputMesh, double scaleX, double scaleY, double scaleZ);

protected:
  vtkMRMLModelStorageNode();
  ~vtkMRMLModelStorageNode() override;
  vtkMRMLModelStorageNode(const vtkMRMLModelStorageNode&);
  void operator=(const vtkMRMLModelStorageNode&);

  /// Initialize all the supported read file types
  void InitializeSupportedReadFileTypes() override;

  /// Initialize all the supported write file types
  void InitializeSupportedWriteFileTypes() override;

  /// Get data node that is associated with this storage node
  vtkMRMLModelNode* GetAssociatedDataNode();

  /// Read data and set it in the referenced node
  int ReadDataInternal(vtkMRMLNode* refNode) override;

  /// Write data from a  referenced node
  int WriteDataInternal(vtkMRMLNode* refNode) override;

  static int GetCoordinateSystemFromFileHeader(const char* header);

  static int GetCoordinateSystemFromFieldData(vtkPointSet* mesh);

  static bool GetCoordinateUnitScaleFromFileHeader(const char* header, double& scale, std::string& unit);

  int CoordinateSystem;
};

#endif
