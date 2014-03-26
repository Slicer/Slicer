/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeHeaderlessStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLVolumeHeaderlessStorageNode_h
#define __vtkMRMLVolumeHeaderlessStorageNode_h

#include "vtkMRMLStorageNode.h"

class vtkImageData;

/// \brief MRML node for representing a volume storage.
///
/// vtkMRMLVolumeHeaderlessStorageNode nodes describes how volume data sets is stored on disk.
class VTK_MRML_EXPORT vtkMRMLVolumeHeaderlessStorageNode
  : public vtkMRMLStorageNode
{
public:
  static vtkMRMLVolumeHeaderlessStorageNode *New();
  vtkTypeMacro(vtkMRMLVolumeHeaderlessStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  ///
  /// Read node attributes from XML file
  virtual void ReadXMLAttributes( const char** atts);

  ///
  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  ///
  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  ///
  /// Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "VolumeHeaderlessStorage";};

  ///
  /// Scan order in the file
  vtkGetStringMacro(FileScanOrder);
  vtkSetStringMacro(FileScanOrder);

  ///
  /// Two numbers: the number of columns and rows of pixels in each image
  vtkGetVector3Macro(FileDimensions, int);
  vtkSetVector3Macro(FileDimensions, int);

  ///
  /// Three numbers for the dimensions of each voxel, in millimeters
  vtkGetVector3Macro(FileSpacing, double);
  vtkSetVector3Macro(FileSpacing, double);

  ///
  /// The type of data in the file. One of: Char, UnsignedChar, Short,
  /// UnsignedShort, Int, UnsignedInt, Long, UnsignedLong, Float, Double
  vtkSetMacro(FileScalarType, int);
  vtkGetMacro(FileScalarType, int);

  void SetFileScalarTypeToUnsignedChar()
    {this->SetFileScalarType(VTK_UNSIGNED_CHAR);};
  void SetFileScalarTypeToChar()
    {this->SetFileScalarType(VTK_CHAR);};
  void SetFileScalarTypeToShort() {
    this->SetFileScalarType(VTK_SHORT);};
  void SetFileScalarTypeToUnsignedShort()
    {this->SetFileScalarType(VTK_UNSIGNED_SHORT);};
  void SetFileScalarTypeToInt() {
    this->SetFileScalarType(VTK_INT);};
  void SetFileScalarTypeToUnsignedInt() {
    this->SetFileScalarType(VTK_UNSIGNED_INT);};
  void SetFileScalarTypeToLong() {
    this->SetFileScalarType(VTK_LONG);};
  void SetFileScalarTypeToUnsignedLong() {
    this->SetFileScalarType(VTK_UNSIGNED_LONG);};
  void SetFileScalarTypeToFloat() {
    this->SetFileScalarType(VTK_FLOAT);};
  void SetFileScalarTypeToDouble() {
    this->SetFileScalarType(VTK_DOUBLE);};

  const char* GetFileScalarTypeAsString();

  void SetFileScalarTypeAsString(const char* );

  ///
  /// The number of scalar components for each voxel.
  /// Gray-level data has 1. Color data has 3
  vtkGetMacro(FileNumberOfScalarComponents, int);
  vtkSetMacro(FileNumberOfScalarComponents, int);

  ///
  /// Describes the order of bytes for each voxel.  Little endian
  /// positions the least-significant byte on the rightmost end,
  /// and is true of data generated on a PC or SGI.
  vtkGetMacro(FileLittleEndian, int);
  vtkSetMacro(FileLittleEndian, int);
  vtkBooleanMacro(FileLittleEndian, int);

  ///
  /// Center image on read
  vtkGetMacro(CenterImage, int);
  vtkSetMacro(CenterImage, int);

  /// Return true if node can be read in
  virtual bool CanReadInReferenceNode(vtkMRMLNode *refNode);
  virtual bool CanWriteFromReferenceNode(vtkMRMLNode *refNode);

protected:

  vtkMRMLVolumeHeaderlessStorageNode();
  ~vtkMRMLVolumeHeaderlessStorageNode();
  vtkMRMLVolumeHeaderlessStorageNode(const vtkMRMLVolumeHeaderlessStorageNode&);
  void operator=(const vtkMRMLVolumeHeaderlessStorageNode&);

  /// Initialize all the supported write file types
  virtual void InitializeSupportedWriteFileTypes();

  /// Read data and set it in the referenced node
  virtual int ReadDataInternal(vtkMRMLNode *refNode);

  /// Write data from a  referenced node
  virtual int WriteDataInternal(vtkMRMLNode *refNode);

  char *FileScanOrder;
  int FileScalarType;
  int FileNumberOfScalarComponents;
  int FileLittleEndian;
  double FileSpacing[3];
  int FileDimensions[3];

  int CenterImage;

  char* WriteFileFormat;
};

#endif
