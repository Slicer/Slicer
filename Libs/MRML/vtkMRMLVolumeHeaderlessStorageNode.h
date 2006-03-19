/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeHeaderlessStorageNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeHeaderlessStorageNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Storage nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Storage nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLVolumeHeaderlessStorageNode_h
#define __vtkMRMLVolumeHeaderlessStorageNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLStorageNode.h"


class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVolumeHeaderlessStorageNode : public vtkMRMLStorageNode
{
  public:
  static vtkMRMLVolumeHeaderlessStorageNode *New();
  vtkTypeMacro(vtkMRMLVolumeHeaderlessStorageNode,vtkMRMLStorageNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

   // Description:
  // Read data and set it in the referenced node
  // NOTE: Subclasses should implement this method
  virtual void ReadData(vtkMRMLNode *refNode);

  // Description:
  // Write data from a  referenced node
  // NOTE: Subclasses should implement this method
  virtual void WriteData(vtkMRMLNode *refNode);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

 // Description:
  // Set dependencies between this node and the parent node
  // when parsing XML file
  virtual void ProcessParentNode(vtkMRMLNode *parentNode);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Storage, Model)
  virtual const char* GetNodeTagName()  {return "VolumeHeaderlessStorage";};

  // Description:
  // A file name or one name in a series
  vtkGetStringMacro(Filename);
  vtkSetStringMacro(Filename);

  // Description:
  // Scan order in the file
  vtkGetStringMacro(FileScanOrder);
  vtkSetStringMacro(FileScanOrder);

  // Description:
  // Two numbers: the number of columns and rows of pixels in each image
  vtkGetVector3Macro(FileDimensions, int);
  vtkSetVector3Macro(FileDimensions, int);

  // Description:
  // Three numbers for the dimensions of each voxel, in millimeters
  vtkGetVector3Macro(FileSpacing, vtkFloatingPointType);
  vtkSetVector3Macro(FileSpacing, vtkFloatingPointType);
  
  // Description:
  // The type of data in the file. One of: Char, UnsignedChar, Short, 
  // UnsignedShort, Int, UnsignedInt, Long, UnsignedLong, Float, Double
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
  
  // Description:
  // The number of scalar components for each voxel. 
  // Gray-level data has 1. Color data has 3
  vtkGetMacro(FileNumberOfScalarComponents, int);
  vtkSetMacro(FileNumberOfScalarComponents, int);

  // Description:
  // Describes the order of bytes for each voxel.  Little endian 
  // positions the least-significant byte on the rightmost end, 
  // and is true of data generated on a PC or SGI.
  vtkGetMacro(FileLittleEndian, int);
  vtkSetMacro(FileLittleEndian, int);
  vtkBooleanMacro(FileLittleEndian, int);



protected:


  vtkMRMLVolumeHeaderlessStorageNode();
  ~vtkMRMLVolumeHeaderlessStorageNode();
  vtkMRMLVolumeHeaderlessStorageNode(const vtkMRMLVolumeHeaderlessStorageNode&);
  void operator=(const vtkMRMLVolumeHeaderlessStorageNode&);

  char *Filename;

  char *FileScanOrder;
  int FileScalarType;
  int FileNumberOfScalarComponents;
  int FileLittleEndian;
  vtkFloatingPointType FileSpacing[3];
  int FileDimensions[3];


};

#endif

