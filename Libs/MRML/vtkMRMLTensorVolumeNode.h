/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLTensorVolumeNode - MRML node for representing diffusion weighted MRI volume 
// .SECTION Description
// Diffusion Weigthed Volume nodes describe data sets that encode diffusion weigthed
// images. These images are the basis for computing the diffusion tensor.
// The node is a container for the neccesary information to interpert DW images:
// 1. Gradient information.
// 2. B value for each gradient.
// 3. Measurement frame that relates the coordinate system where the gradients are given 
//  to RAS.

#ifndef __vtkMRMLTensorVolumeNode_h
#define __vtkMRMLTensorVolumeNode_h


#include "vtkMRMLScalarVolumeNode.h"

class vtkImageData;
class vtkDoubleArray;
class vtkMatrix4x4;
class vtkDiffusionTensorMathematics;
class vtkAssignAttribute;

class VTK_MRML_EXPORT vtkMRMLTensorVolumeNode : public vtkMRMLScalarVolumeNode
{
  public:
  static vtkMRMLTensorVolumeNode *New();
  vtkTypeMacro(vtkMRMLTensorVolumeNode,vtkMRMLVolumeNode);
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
  virtual const char* GetNodeTagName() {return "TensorVolume";};

  // Description:
  // Updates this node if it depends on other nodes 
  // when the node is deleted in the scene
  virtual void UpdateReferences()
    { Superclass::UpdateReferences(); };

 // Description:
 // Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID) 
    { Superclass::UpdateReferenceID(oldID, newID); };

  // Description:
  vtkGetMacro(Order,int);
  vtkSetMacro(Order,int);
  
  // Description:
  // Set the Measurement frame matrix from 3x3 array
  void SetMeasurementFrameMatrix(const double mf[3][3]);
  // Description
  // Set the measurement frame matrix from doubles
  void SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs);

  void GetMeasurementFrameMatrix(double mf[3][3]);
  
  // Description:
  // Set/Get the measurement frame matrix from a vtk 4x4 matrix
  void SetMeasurementFrameMatrix(vtkMatrix4x4 *mat);
  void GetMeasurementFrameMatrix(vtkMatrix4x4 *mat);

  // Description:
  // response to update event, call calcualte auto levels
  virtual void UpdateFromMRML();
  
  // Description:
  // Extract the component defined by the diffusion tensor dispaly properties
  // node, and pass it on to CalculateScalarAutoLevels.
  virtual void CalculateAutoLevels(vtkMRMLVolumeDisplayNode *refNode = NULL, vtkImageData *refData = NULL);
  
protected:
  vtkMRMLTensorVolumeNode();
  ~vtkMRMLTensorVolumeNode();
  vtkMRMLTensorVolumeNode(const vtkMRMLTensorVolumeNode&);
  void operator=(const vtkMRMLTensorVolumeNode&);
  
  double MeasurementFrameMatrix[3][3];
  int Order;

  vtkDiffusionTensorMathematics *DTIMathematics;  
  vtkAssignAttribute *AssignAttributeTensorsFromScalars ;
  
};

#endif


 

