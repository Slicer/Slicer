/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/02/11 17:20:11 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeNode - MRML node for representing a volume (image stack).
// .SECTION Description
// Volume nodes describe data sets that can be thought of as stacks of 2D 
// images that form a 3D volume.  Volume nodes describe where the images 
// are stored on disk, how to render the data (window and level), and how 
// to read the files.  This information is extracted from the image 
// headers (if they exist) at the time the MRML file is generated.  
// Consequently, MRML files isolate MRML browsers from understanding how 
// to read the myriad of file formats for medical data. 

#ifndef __vtkMRMLVolumeNode_h
#define __vtkMRMLVolumeNode_h

#include "vtkMRMLNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"
#include "vtkITKArchetypeImageSeriesReader.h"

class vtkImageData;

class VTK_EXPORT vtkMRMLVolumeNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeNode *New();
  vtkTypeMacro(vtkMRMLVolumeNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Read data for the node
  virtual void ReadData();

  // Write data for the node
  virtual void WriteData();

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual char* GetNodeTagName() {return "Volume";};

  // Description:
  // A file name or one name in a series
  vtkSetStringMacro(FileArcheType);
  vtkGetStringMacro(FileArcheType);

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

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------
  
  // Description:
  // Numerical ID of the color lookup table to use for rendering the volume
  vtkSetStringMacro(LUTName);
  vtkGetStringMacro(LUTName);

  // Description:
  // Indicates if this volume is a label map, which is the output of 
  // segmentation that labels each voxel according to its tissue type.  
  // The alternative is a gray-level or color image.
  vtkGetMacro(LabelMap, int);
  vtkSetMacro(LabelMap, int);
  vtkBooleanMacro(LabelMap, int);

  // Description:
  // Specifies whether windowing and leveling are to be performed automatically
  vtkBooleanMacro(AutoWindowLevel, int);
  vtkGetMacro(AutoWindowLevel, int);
  vtkSetMacro(AutoWindowLevel, int);

  // Description:
  // The window value to use when autoWindowLevel is 'no'
  vtkGetMacro(Window, vtkFloatingPointType);
  vtkSetMacro(Window, vtkFloatingPointType);

  // Description:
  // The level value to use when autoWindowLevel is 'no'
  vtkGetMacro(Level, vtkFloatingPointType);
  vtkSetMacro(Level, vtkFloatingPointType);

  // Description:
  // Specifies whether to apply the threshold
  vtkBooleanMacro(ApplyThreshold, int);
  vtkGetMacro(ApplyThreshold, int);
  vtkSetMacro(ApplyThreshold, int);

  // Description:
  // Specifies whether the threshold should be set automatically
  vtkBooleanMacro(AutoThreshold, int);
  vtkGetMacro(AutoThreshold, int);
  vtkSetMacro(AutoThreshold, int);

  // Description:
  // The upper threshold value to use when autoThreshold is 'no'
  // Warning:
  // XXX-MH Should be floating point....
  vtkGetMacro(UpperThreshold, vtkFloatingPointType);
  vtkSetMacro(UpperThreshold, vtkFloatingPointType);

  // Description:
  // The lower threshold value to use when autoThreshold is 'no'
  // Warning:
  // XXX-MH Should be floating point....
  vtkGetMacro(LowerThreshold, vtkFloatingPointType);
  vtkSetMacro(LowerThreshold, vtkFloatingPointType);

  // Description:
  // Set/Get interpolate reformated slices
  vtkGetMacro(Interpolate, int);
  vtkSetMacro(Interpolate, int);
  vtkBooleanMacro(Interpolate, int);

  //--------------------------------------------------------------------------
  // RAS->IJK Matrix Calculation
  //--------------------------------------------------------------------------

  // Description:
  // The order of slices in the volume. One of: LR (left-to-right), 
  // RL, AP, PA, IS, SI. This information is encoded in the rasToIjkMatrix.
  // This matrix can be computed either from corner points, or just he
  // scanOrder.
  static void ComputeIjkToRasFromScanOrder(char *order, vtkMatrix4x4 *IjkToRas);

  static const char* ComputeScanOrderFromIjkToRas(vtkMatrix4x4 *IjkToRas);

  vtkGetStringMacro(ScanOrder);
  vtkSetStringMacro(ScanOrder);

  void SetIjkToRasDirections(double dirs[9]);
  void SetIjkToRasDirections(double ir, double ia, double is,
                             double jr, double ja, double js,
                             double kr, double ka, double ks);
  void SetIToRasDirection(double ir, double ia, double is);
  void SetJToRasDirection(double jr, double ja, double ijs);
  void SetKToRasDirection(double kr, double ka, double ks);

  void GetIjkToRasDirections(double dirs[9]);
  void GetIToRasDirection(double dirs[3]);
  void GetJToRasDirection(double dirs[3]);
  void GetKToRasDirection(double dirs[3]);
  double* GetIjkToRasDirections();

  void GetIjkToRasMatrix(vtkMatrix4x4* mat);
  void SetIjkToRasMatrix(vtkMatrix4x4* mat);

  vtkGetObjectMacro(ImageData, vtkImageData);
  vtkSetObjectMacro(ImageData, vtkImageData);

  
protected:
  vtkMRMLVolumeNode();
  ~vtkMRMLVolumeNode();
  vtkMRMLVolumeNode(const vtkMRMLVolumeNode&);
  void operator=(const vtkMRMLVolumeNode&);


  // Strings
  char *FileArcheType;
  char *LUTName;
  char *ScanOrder;

  int FileScalarType;
  int FileNumberOfScalarComponents;
  int FileLittleEndian;
  vtkFloatingPointType FileSpacing[3];
  int FileDimensions[3];

  vtkFloatingPointType Window;
  vtkFloatingPointType Level;
  vtkFloatingPointType UpperThreshold;
  vtkFloatingPointType LowerThreshold;


  // Booleans
  int LabelMap;
  int Interpolate;
  int AutoWindowLevel;
  int ApplyThreshold;
  int AutoThreshold;

  double IjkToRasDirections[9];
  double IToRasDirections[3];
  double JToRasDirections[3];
  double KToRasDirections[3];

  vtkImageData *ImageData;

  vtkITKArchetypeImageSeriesReader* ImageReader;
};

#endif

