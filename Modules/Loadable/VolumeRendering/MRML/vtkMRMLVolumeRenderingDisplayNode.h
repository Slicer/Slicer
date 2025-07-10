/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLVolumeRenderingDisplayNode.h,v $
  Date:      $Date: 2006/03/19 17:12:29 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/

#ifndef __vtkMRMLVolumeRenderingDisplayNode_h
#define __vtkMRMLVolumeRenderingDisplayNode_h

// Volume Rendering includes
#include "vtkSlicerVolumeRenderingModuleMRMLExport.h"

// MRML includes
#include "vtkMRMLDisplayNode.h"
class vtkIntArray;
class vtkMRMLAnnotationROINode;
class vtkMRMLMarkupsROINode;
class vtkMRMLMessageCollection;
class vtkMRMLShaderPropertyNode;
class vtkMRMLViewNode;
class vtkMRMLVolumeNode;
class vtkMRMLVolumePropertyNode;

/// \name vtkMRMLVolumeRenderingDisplayNode
/// \brief Abstract MRML node for storing information for Volume Rendering
class VTK_SLICER_VOLUMERENDERING_MODULE_MRML_EXPORT vtkMRMLVolumeRenderingDisplayNode
  : public vtkMRMLDisplayNode
{
public:
  vtkTypeMacro(vtkMRMLVolumeRenderingDisplayNode, vtkMRMLDisplayNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Set node attributes
  void ReadXMLAttributes( const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  void Copy(vtkMRMLNode* node) override;

  const char* GetVolumeNodeID();
  vtkMRMLVolumeNode* GetVolumeNode();

  const char* GetVolumePropertyNodeID();
  void SetAndObserveVolumePropertyNodeID(const char* volumePropertyNodeID);
  vtkMRMLVolumePropertyNode* GetVolumePropertyNode();

  const char* GetShaderPropertyNodeID();
  void SetAndObserveShaderPropertyNodeID(const char* shaderPropertyNodeID);
  vtkMRMLShaderPropertyNode* GetShaderPropertyNode();
  vtkMRMLShaderPropertyNode* GetOrCreateShaderPropertyNode( vtkMRMLScene* mrmlScene );

  const char* GetROINodeID();
  void SetAndObserveROINodeID(const char* roiNodeID);
  vtkMRMLDisplayableNode* GetROINode();
  /// Deprecated. Use GetROINode() instead for retrieving the markups ROI node.
  vtkMRMLAnnotationROINode* GetAnnotationROINode() { return nullptr; };
  vtkMRMLMarkupsROINode* GetMarkupsROINode();

  vtkMRMLViewNode* GetFirstViewNode();

  double GetSampleDistance();

  vtkSetMacro(CroppingEnabled, int);
  vtkGetMacro(CroppingEnabled, int);
  vtkBooleanMacro(CroppingEnabled, int);

  vtkSetVector2Macro(Threshold, double);
  vtkGetVectorMacro(Threshold, double, 2);

  vtkGetMacro(FollowVolumeDisplayNode, int);
  vtkSetMacro(FollowVolumeDisplayNode, int);

  vtkGetMacro(IgnoreVolumeDisplayNodeThreshold, int);
  vtkSetMacro(IgnoreVolumeDisplayNodeThreshold, int);

  vtkGetMacro(UseSingleVolumeProperty, int);
  vtkSetMacro(UseSingleVolumeProperty, int);

  vtkSetVector2Macro(WindowLevel, double);
  vtkGetVectorMacro(WindowLevel, double, 2);

  //@{
  /// Get/Set the number of voxels to soften the edges when applying a clipping function.
  /// This is useful to avoid aliasing artifacts when the clipping function is applied.
  /// The default value is 0.0, which means that no softening is applied.
  vtkSetClampMacro(ClippingSoftEdgeVoxels, double, 0.0, VTK_DOUBLE_MAX);
  vtkGetMacro(ClippingSoftEdgeVoxels, double);
  //@}

  //@{
  /// Get/Set the blank voxel value for the volume rendering.
  /// This value is used for the fill value when clipping for volume rendering.
  /// It is only used when AutoClippingBlankVoxelValue is false.
  /// The default value is 0.0.
  vtkSetMacro(ClippingBlankVoxelValue, double);
  vtkGetMacro(ClippingBlankVoxelValue, double);
  //@}

  //@{
  /// Get/Set whether the blank voxel value is automatically set to the background value of the volume node.
  /// If disabled, then ClippingBlankVoxelValue is used.
  /// The default value is true.
  vtkSetMacro(AutoClippingBlankVoxelValue, bool);
  vtkGetMacro(AutoClippingBlankVoxelValue, bool);
  vtkBooleanMacro(AutoClippingBlankVoxelValue, bool);
  //@}

  //@{
  /// Check if a fast clipping method can be used with the display node.
  /// Returns true if fast clipping can be utilized, or returns false otherwise.
  /// If userMessages is specified, messages will be added to provide reasons for why fast clipping
  /// is not available.
  bool IsFastClippingAvailable(vtkMRMLMessageCollection* userMessages = nullptr);
  //@}

protected:
  vtkMRMLVolumeRenderingDisplayNode();
  ~vtkMRMLVolumeRenderingDisplayNode() override;
  vtkMRMLVolumeRenderingDisplayNode(const vtkMRMLVolumeRenderingDisplayNode&);
  void operator=(const vtkMRMLVolumeRenderingDisplayNode&);

  void ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData) override;

  /// Check if a fast clipping method can be used with the specified clip node.
  /// Returns true if fast clipping can be utilized, or returns false otherwise.
  /// If userMessages is specified, messages will be added to provide reasons for why fast clipping
  /// is not available.
  static bool IsFastClippingAvailable(vtkMRMLClipNode* clipNode, vtkMRMLMessageCollection* userMessages = nullptr);

  static const char* VolumePropertyNodeReferenceRole;
  static const char* VolumePropertyNodeReferenceMRMLAttributeName;
  static const char* ROINodeReferenceRole;
  static const char* ROINodeReferenceMRMLAttributeName;
  static const char* ShaderPropertyNodeReferenceRole;
  static const char* ShaderPropertyNodeReferenceMRMLAttributeName;

protected:
  /// Flag indicating whether cropping is enabled
  int CroppingEnabled;

  double Threshold[2];

  /// Follow window/level and thresholding setting in volume display node
  int FollowVolumeDisplayNode;
  int IgnoreVolumeDisplayNodeThreshold;

  int UseSingleVolumeProperty;

  /// Volume window & level
  double WindowLevel[2];

  double ClippingSoftEdgeVoxels{ 0.0 };

  double ClippingBlankVoxelValue{ 0.0 };
  bool AutoClippingBlankVoxelValue{ true };
};

#endif
