/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

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

#include "vtkMRML.h"
#include "vtkMRMLNode.h"
#include "vtkMRMLStorageNode.h"
#include "vtkMRMLVolumeDisplayNode.h"

#include "vtkMatrix4x4.h"
#include "vtkTransform.h"
#include "vtkImageData.h"

class vtkImageData;

class VTK_MRML_EXPORT vtkMRMLVolumeNode : public vtkMRMLNode
{
  public:
  static vtkMRMLVolumeNode *New();
  vtkTypeMacro(vtkMRMLVolumeNode,vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkMRMLNode* CreateNodeInstance();

  // Description:
  // Set node attributes
  virtual void ReadXMLAttributes( const char** atts);

  // Description:
  // Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent);

  // Description:
  // Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode *node);

  // Description:
  // Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() {return "Volume";};

  // Description:
  // Finds the storage node and read the data
  virtual void UpdateScene(vtkMRMLScene *scene);

  //--------------------------------------------------------------------------
  // Display Information
  //--------------------------------------------------------------------------
  
  // Description:
  // Indicates if this volume is a label map, which is the output of 
  // segmentation that labels each voxel according to its tissue type.  
  // The alternative is a gray-level or color image.
  vtkGetMacro(LabelMap, int);
  vtkSetMacro(LabelMap, int);
  vtkBooleanMacro(LabelMap, int);

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


  // Description:
  // Numerical ID of the storage MRML node
  vtkSetStringMacro(StorageNodeID);
  vtkGetStringMacro(StorageNodeID);

  // Description:
  // Numerical ID of the display MRML node
  vtkSetStringMacro(DisplayNodeID);
  vtkGetStringMacro(DisplayNodeID);

  vtkGetObjectMacro(StorageNode, vtkMRMLStorageNode);
  vtkSetObjectMacro(StorageNode, vtkMRMLStorageNode);

  vtkGetObjectMacro(DisplayNode, vtkMRMLVolumeDisplayNode);
  vtkSetObjectMacro(DisplayNode, vtkMRMLVolumeDisplayNode);

  vtkGetObjectMacro(ImageData, vtkImageData);
  vtkSetObjectMacro(ImageData, vtkImageData);

  
protected:
  vtkMRMLVolumeNode();
  ~vtkMRMLVolumeNode();
  vtkMRMLVolumeNode(const vtkMRMLVolumeNode&);
  void operator=(const vtkMRMLVolumeNode&);


  // Strings

  // Booleans
  int LabelMap;

  double IjkToRasDirections[9];
  double IToRasDirections[3];
  double JToRasDirections[3];
  double KToRasDirections[3];

  char *StorageNodeID;
  char *DisplayNodeID;

  vtkMRMLStorageNode *StorageNode;
  vtkMRMLVolumeDisplayNode *DisplayNode;
  vtkImageData       *ImageData;

};

#endif

