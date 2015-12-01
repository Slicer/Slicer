/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLTensorVolumeNode_h
#define __vtkMRMLTensorVolumeNode_h

#include "vtkMRMLScalarVolumeNode.h"
class vtkMRMLStorageNode;

class vtkDoubleArray;
class vtkImageData;
class vtkMatrix4x4;

/// \brief MRML node for representing diffusion weighted MRI volume.
///
/// Diffusion Weigthed Volume nodes describe data sets that encode diffusion weigthed
/// images. These images are the basis for computing the diffusion tensor.
/// The node is a container for the neccesary information to interpert DW images:
/// 1. Gradient information.
/// 2. B value for each gradient.
/// 3. Measurement frame that relates the coordinate system where the gradients are given
///  to RAS.
class VTK_MRML_EXPORT vtkMRMLTensorVolumeNode : public vtkMRMLScalarVolumeNode
{
  public:
  static vtkMRMLTensorVolumeNode *New();
  vtkTypeMacro(vtkMRMLTensorVolumeNode,vtkMRMLScalarVolumeNode);
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
  virtual const char* GetNodeTagName() {return "TensorVolume";};

  ///
  /// Updates this node if it depends on other nodes
  /// when the node is deleted in the scene
  virtual void UpdateReferences()
    { Superclass::UpdateReferences(); };

 /// Description:
 /// Update the stored reference to another node in the scene
  virtual void UpdateReferenceID(const char *oldID, const char *newID)
    { Superclass::UpdateReferenceID(oldID, newID); };

  ///
  vtkGetMacro(Order,int);
  vtkSetMacro(Order,int);

  ///
  /// Set the Measurement frame matrix from 3x3 array
  void SetMeasurementFrameMatrix(const double mf[3][3]);
  /// Description
  /// Set the measurement frame matrix from doubles
  void SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs);

  void GetMeasurementFrameMatrix(double mf[3][3]);

  ///
  /// Set/Get the measurement frame matrix from a vtk 4x4 matrix
  void SetMeasurementFrameMatrix(vtkMatrix4x4 *mat);
  void GetMeasurementFrameMatrix(vtkMatrix4x4 *mat);

  ///
  /// Create default storage node or NULL if does not have one
  virtual vtkMRMLStorageNode* CreateDefaultStorageNode();

protected:
  vtkMRMLTensorVolumeNode();
  ~vtkMRMLTensorVolumeNode();
  vtkMRMLTensorVolumeNode(const vtkMRMLTensorVolumeNode&);
  void operator=(const vtkMRMLTensorVolumeNode&);

  double MeasurementFrameMatrix[3][3];
  int Order;
};

#endif
