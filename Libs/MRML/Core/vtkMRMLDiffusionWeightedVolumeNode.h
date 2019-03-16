/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/

#ifndef __vtkMRMLDiffusionWeightedVolumeNode_h
#define __vtkMRMLDiffusionWeightedVolumeNode_h

#include "vtkMRMLScalarVolumeNode.h"
class vtkMRMLDiffusionWeightedVolumeDisplayNode;

class vtkImageData;
class vtkDoubleArray;
class vtkImageExtractComponents;

/// \brief MRML node for representing diffusion weighted MRI volume
///
/// Diffusion Weigthed Volume nodes describe data sets that encode diffusion weigthed
/// images. These images are the basis for computing the diffusion tensor.
/// The node is a container for the necessary information to interpert DW images:
/// 1. Gradient information.
/// 2. B value for each gradient.
/// 3. Measurement frame that relates the coordinate system where the gradients are given
///  to RAS.
class VTK_MRML_EXPORT vtkMRMLDiffusionWeightedVolumeNode : public vtkMRMLScalarVolumeNode
{
  public:
  static vtkMRMLDiffusionWeightedVolumeNode *New();
  vtkTypeMacro(vtkMRMLDiffusionWeightedVolumeNode,vtkMRMLScalarVolumeNode);
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
  const char* GetNodeTagName() override {return "DiffusionWeightedVolume";}

  ///
  void SetNumberOfGradients(int val);
  int GetNumberOfGradients();

  ///
  void SetDiffusionGradient(int val, const double g[3]);
  void SetDiffusionGradients(vtkDoubleArray *grad);
  double *GetDiffusionGradient(int val);
  void GetDiffusionGradient(int val, double g[3]);
  vtkGetObjectMacro(DiffusionGradients,vtkDoubleArray);

  ///
  void SetBValue (int val, const double b);
  void SetBValues (vtkDoubleArray *bValue);
/// Work around issue that GetBValue is defined as a macro in windows.h
#ifdef GetBValue
#undef GetBValue
#endif
  double GetBValue(int val);
  vtkGetObjectMacro(BValues,vtkDoubleArray);

  ///  Set/Get measurement frame that relates the coordinate system where the
  /// tensor measurements are given with the RAS coordinate system
  void SetMeasurementFrameMatrix(const double mf[3][3]);
  void GetMeasurementFrameMatrix(double mf[3][3]);
  void SetMeasurementFrameMatrix(const double xr, const double xa, const double xs,
                           const double yr, const double ya, const double ys,
                           const double zr, const double za, const double zs);

  void SetMeasurementFrameMatrix(vtkMatrix4x4 *mat);
  void GetMeasurementFrameMatrix(vtkMatrix4x4 *mat);

  ///
  /// Associated display MRML node
  virtual vtkMRMLDiffusionWeightedVolumeDisplayNode* GetDiffusionWeightedVolumeDisplayNode();

  ///
  /// Create default storage node or nullptr if does not have one
  vtkMRMLStorageNode* CreateDefaultStorageNode() override;

  ///
  /// Create and observe default display node
  void CreateDefaultDisplayNodes() override;

protected:
  vtkMRMLDiffusionWeightedVolumeNode();
  ~vtkMRMLDiffusionWeightedVolumeNode() override;
  vtkMRMLDiffusionWeightedVolumeNode(const vtkMRMLDiffusionWeightedVolumeNode&);
  void operator=(const vtkMRMLDiffusionWeightedVolumeNode&);

  void SetNumberOfGradientsInternal(int val);

  double MeasurementFrameMatrix[3][3];

  vtkDoubleArray *DiffusionGradients;
  vtkDoubleArray *BValues;

};

#endif




