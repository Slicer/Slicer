/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/
// .NAME vtkMRMLVolumeRenderingParametersNode - MRML node for storing a slice through RAS space
// .SECTION Description
// This node stores the information about the currently selected volume
//
//

#ifndef __vtkMRMLCropVolumeParametersNode_h
#define __vtkMRMLCropVolumeParametersNode_h

#include "vtkMRML.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLNode.h"
#include "vtkSlicerCropVolumeModuleMRMLExport.h"

class vtkMRMLDisplayableNode;
class vtkMRMLTransformNode;
class vtkMRMLVolumeNode;

class VTK_SLICER_CROPVOLUME_MODULE_MRML_EXPORT vtkMRMLCropVolumeParametersNode : public vtkMRMLNode
{
public:
  enum
  {
    InterpolationNearestNeighbor = 1,
    InterpolationLinear = 2,
    InterpolationWindowedSinc = 3,
    InterpolationBSpline = 4
  };

  enum
  {
    FitROIAlignToVolume = 0,   ///< Before resizing the ROI, ROI orientation is adjusted to align with the axes of the input volume
    FitROIAlignToWorld = 1,    ///< Before resizing the ROI, ROI orientation is adjusted to align with the world coordinate system axes
    FitROIKeepOrientation = 2, ///< The ROI orientation is not modified during fitting
    FitROI_Last                ///< PositionStatus_Last: indicates the end of the enum (int first = 0, int last = FitROI_Last)
  };

  static vtkMRMLCropVolumeParametersNode* New();
  vtkTypeMacro(vtkMRMLCropVolumeParametersNode, vtkMRMLNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLNode* CreateNodeInstance() override;

  /// Set node attributes from XML attributes
  void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  void WriteXML(ostream& of, int indent) override;

  /// Copy node content (excludes basic data, such as name and node references).
  /// \sa vtkMRMLNode::CopyContent
  vtkMRMLCopyContentMacro(vtkMRMLCropVolumeParametersNode);

  /// Get node XML tag name (like Volume, Model)
  const char* GetNodeTagName() override { return "CropVolumeParameters"; }

  /// Set volume node to be cropped
  void SetInputVolumeNodeID(const char* nodeID);
  /// Get volume node to be cropped
  const char* GetInputVolumeNodeID();
  vtkMRMLVolumeNode* GetInputVolumeNode();

  /// Set resulting cropped volume node
  void SetOutputVolumeNodeID(const char* nodeID);
  /// Get resulting cropped volume node
  const char* GetOutputVolumeNodeID();
  vtkMRMLVolumeNode* GetOutputVolumeNode();

  /// Set cropping region of interest.
  /// It must be vtkMRMLMarkupsROINode.
  void SetROINodeID(const char* nodeID);
  /// Get cropping region of interest
  const char* GetROINodeID();
  vtkMRMLDisplayableNode* GetROINode();

  /// Set transform node that may be used for aligning
  /// the ROI with the input volume.
  void SetROIAlignmentTransformNodeID(const char* nodeID);
  const char* GetROIAlignmentTransformNodeID();
  vtkMRMLTransformNode* GetROIAlignmentTransformNode();
  void DeleteROIAlignmentTransformNode();

  vtkSetMacro(IsotropicResampling, bool);
  vtkGetMacro(IsotropicResampling, bool);
  vtkBooleanMacro(IsotropicResampling, bool);

  vtkSetMacro(VoxelBased, bool);
  vtkGetMacro(VoxelBased, bool);
  vtkBooleanMacro(VoxelBased, bool);

  vtkSetMacro(InterpolationMode, int);
  vtkGetMacro(InterpolationMode, int);

  vtkSetMacro(SpacingScalingConst, double);
  vtkGetMacro(SpacingScalingConst, double);

  vtkSetMacro(FillValue, double);
  vtkGetMacro(FillValue, double);

  /// Set method for fitting the ROI to the input volume.
  /// Before resizing the ROI, orientation of the ROI is adjusted according to chosen option:
  /// - FitROIAlignToVolume to align with the axes of the input volume (default)
  /// - FitROIAlignToWorld to align with world coordinate system axes
  /// - FitROIKeepOrientation to keep the current orientation
  vtkSetMacro(FitROIMode, int);
  vtkGetMacro(FitROIMode, int);
  static const char* GetFitROIModeAsString(int slabReconstructionType);
  static int GetFitROIModeFromString(const char* name);

protected:
  vtkMRMLCropVolumeParametersNode();
  ~vtkMRMLCropVolumeParametersNode() override;

  vtkMRMLCropVolumeParametersNode(const vtkMRMLCropVolumeParametersNode&);
  void operator=(const vtkMRMLCropVolumeParametersNode&);

  bool VoxelBased;
  int InterpolationMode;
  bool IsotropicResampling;
  double SpacingScalingConst;
  double FillValue;
  int FitROIMode;
};

#endif
