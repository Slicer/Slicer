/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSlicerCropVolumeLogic.h,v $
  Date:      $Date: 2006/01/08 04:48:05 $
  Version:   $Revision: 1.45 $

=========================================================================auto=*/

// .NAME vtkSlicerCropVolumeLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerCropVolumeLogic_h
#define __vtkSlicerCropVolumeLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"
class vtkSlicerCLIModuleLogic;
class vtkSlicerVolumesLogic;
class vtkMRMLVolumeNode;
class vtkMRMLAnnotationROINode;
// vtk includes
class vtkMatrix4x4;
// CropVolumes includes
#include "vtkSlicerCropVolumeModuleLogicExport.h"
class vtkMRMLCropVolumeParametersNode;


/// \class vtkSlicerCropVolumeLogic
/// \brief Crop a volume to the specified region of interest.
///
/// This class implements cropping and resampling of a volume.
/// Two main use cases:
///
/// 1. Reduce size (both extent and resolution) of a large volume.
/// Size reduction is useful, as it reduces memory need and makes
/// visualization and processing faster.
///
/// 2. Increase resolution of a specific region.
/// Increasing resolution (decreasing voxel size) is useful for
/// segmentation and visualization of fine details.
///
/// If interpolation is disabled then only the extent of the volume
/// is decreased. Cropping without resampling is very fast and needs
/// almost no extra memory.
///
/// If interpolation is enabled, then both the size and resolution
/// of the volume can be changed.
///
/// Limitations:
/// * Region of interes (ROI) node cannot be under non-linear transform
/// * Cropped output volume node cannot be under non-linear transform
/// * If interpolation is disabled: input volume node cannot be under non-linear transform
///   and ROI node must be aligned with the input volume
///
/// \ingroup Slicer_QtModules_CropVolume
class VTK_SLICER_CROPVOLUME_MODULE_LOGIC_EXPORT vtkSlicerCropVolumeLogic
  : public vtkSlicerModuleLogic
{
public:

  static vtkSlicerCropVolumeLogic *New();
  vtkTypeMacro(vtkSlicerCropVolumeLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  void SetVolumesLogic(vtkSlicerVolumesLogic* logic);
  vtkSlicerVolumesLogic* GetVolumesLogic();

  void SetResampleLogic(vtkSlicerCLIModuleLogic* logic);
  vtkSlicerCLIModuleLogic* GetResampleLogic();

  /// Crop input volume using the specified ROI node.
  int Apply(vtkMRMLCropVolumeParametersNode*);

  /// Perform non-interpolated (voxel-based) cropping.
  /// If limitToInputExtent is set to true (default) then the extent can only be smaller than the input volume.
  static int CropVoxelBased(vtkMRMLAnnotationROINode* roi, vtkMRMLVolumeNode* inputVolume,
    vtkMRMLVolumeNode* outputNode, bool limitToInputExtent=true, double fillValue=0.0);

  /// Compute non-interpolated (voxel-based) cropping output volume geometry (without actually cropping the image).
  /// If limitToInputExtent is set to true (default) then the extent can only be smaller than the input volume.
  static bool GetVoxelBasedCropOutputExtent(vtkMRMLAnnotationROINode* roi, vtkMRMLVolumeNode* inputVolume,
    int outputExtent[6], bool limitToInputExtent=false);

  /// Perform interpolated cropping.
  int CropInterpolated(vtkMRMLAnnotationROINode* roi, vtkMRMLVolumeNode* inputVolume, vtkMRMLVolumeNode* outputNode,
    bool isotropicResampling, double spacingScale, int interpolationMode, double fillValue);

  /// Computes output volume geometry for interpolated cropping (without actually cropping the image).
  static bool GetInterpolatedCropOutputGeometry(vtkMRMLAnnotationROINode* roi, vtkMRMLVolumeNode* inputVolume,
    bool isotropicResampling, double spacingScale, int outputExtent[6], double outputSpacing[3]);

  /// Sets ROI to fit to input volume.
  /// If ROI is under a non-linear transform then the ROI transform will be reset to RAS.
  static bool FitROIToInputVolume(vtkMRMLCropVolumeParametersNode* parametersNode);

  static void SnapROIToVoxelGrid(vtkMRMLCropVolumeParametersNode* parametersNode);

  static bool IsROIAlignedWithInputVolume(vtkMRMLCropVolumeParametersNode* parametersNode);

  void RegisterNodes() override;

protected:
  vtkSlicerCropVolumeLogic();
  ~vtkSlicerCropVolumeLogic() override;

private:
  vtkSlicerCropVolumeLogic(const vtkSlicerCropVolumeLogic&) = delete;
  void operator=(const vtkSlicerCropVolumeLogic&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif

