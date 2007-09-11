/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
// .NAME vtkMRMLVolumeNode - MRML node for representing diffusion weighted MRI volume 
// .SECTION Description
// Diffusion Weigthed Volume nodes describe data sets that encode diffusion weigthed
// images. These images are the basis for computing the diffusion tensor.
// The node is a container for the neccesary information to interpert DW images:
// 1. Gradient information.
// 2. B value for each gradient.
// 3. Measurement frame that relates the coordinate system where the gradients are given 
//  to RAS.

#ifndef __vtkMRMLDiffusionWeightedVolumeNode_h
#define __vtkMRMLDiffusionWeightedVolumeNode_h


#include "vtkMRMLVolumeNode.h"
#include "vtkMRMLDiffusionWeightedVolumeDisplayNode.h"

class vtkImageData;
class vtkDoubleArray;

class VTK_MRML_EXPORT vtkMRMLDiffusionWeightedVolumeNode : public vtkMRMLVolumeNode
{
  public:
  static vtkMRMLDiffusionWeightedVolumeNode *New();
  vtkTypeMacro(vtkMRMLDiffusionWeightedVolumeNode,vtkMRMLVolumeNode);
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
  virtual const char* GetNodeTagName() {return "DiffusionWeightedVolume";};

  // Description:
  void SetNumberOfGradients(int val);
  vtkGetMacro(NumberOfGradients,int);

  // Description:
  void SetDiffusionGradient(int val, const double g[3]);
  void SetDiffusionGradients(vtkDoubleArray *grad);
  double *GetDiffusionGradient(int val);
  void GetDiffusionGradient(int val, double g[3]);  
  vtkGetObjectMacro(DiffusionGradients,vtkDoubleArray);

  // Description:
  void SetBValue (int val, const double b);
  void SetBValues (vtkDoubleArray *bValue);
  //double GetBValue(int val);
  vtkGetObjectMacro(BValues,vtkDoubleArray);

  // Description: Set/Get measurement frame that relates the coordinate system where the
  // tensor measurements are given with the RAS coordinate system
  void SetMeasurementFrameMatrix(const double mf[3][3]);
  void GetMeasurementFrameMatrix(double mf[3][3]);
  void SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs);

  void SetMeasurementFrameMatrix(vtkMatrix4x4 *mat);
  void GetMeasurementFrameMatrix(vtkMatrix4x4 *mat);
 
  // Description:
  // Associated display MRML node
  virtual vtkMRMLDiffusionWeightedVolumeDisplayNode* GetDiffusionWeightedVolumeDisplayNode()
  {
    return vtkMRMLDiffusionWeightedVolumeDisplayNode::SafeDownCast(this->GetDisplayNode());
  }


protected:
  vtkMRMLDiffusionWeightedVolumeNode();
  ~vtkMRMLDiffusionWeightedVolumeNode();
  vtkMRMLDiffusionWeightedVolumeNode(const vtkMRMLDiffusionWeightedVolumeNode&);
  void operator=(const vtkMRMLDiffusionWeightedVolumeNode&);
  
  double MeasurementFrameMatrix[3][3];
  
  vtkDoubleArray *DiffusionGradients;
  vtkDoubleArray *BValues;
  
  int NumberOfGradients;

};

#endif


 

