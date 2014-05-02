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

/// \ingroup Slicer_QtModules_CropVolume
class VTK_SLICER_CROPVOLUME_MODULE_LOGIC_EXPORT vtkSlicerCropVolumeLogic
  : public vtkSlicerModuleLogic
{
public:

  static vtkSlicerCropVolumeLogic *New();
  vtkTypeMacro(vtkSlicerCropVolumeLogic,vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  void SetVolumesLogic(vtkSlicerVolumesLogic* logic);
  vtkSlicerVolumesLogic* GetVolumesLogic();

  void SetResampleLogic(vtkSlicerCLIModuleLogic* logic);
  vtkSlicerCLIModuleLogic* GetResampleLogic();

  int Apply(vtkMRMLCropVolumeParametersNode*);

  void CropVoxelBased(vtkMRMLAnnotationROINode* roi, vtkMRMLVolumeNode* inputVolume, vtkMRMLVolumeNode* outputNode);

  virtual void RegisterNodes();

  static bool IsVolumeTiltedInRAS(vtkMRMLVolumeNode* inputVolume, vtkMatrix4x4* rotation);
  static bool ComputeIJKToRASRotationOnlyMatrix(vtkMRMLVolumeNode* inputVolume, vtkMatrix4x4* outputMatrix);

  void SnapROIToVoxelGrid(vtkMRMLAnnotationROINode* inputROI, vtkMRMLVolumeNode* inputVolume);


protected:
  vtkSlicerCropVolumeLogic();
  virtual ~vtkSlicerCropVolumeLogic();

  static bool ComputeOrientationMatrixFromScanOrder(const char *order, vtkMatrix4x4 *outputMatrix);

private:
  vtkSlicerCropVolumeLogic(const vtkSlicerCropVolumeLogic&); // Not implemented
  void operator=(const vtkSlicerCropVolumeLogic&);           // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
};

#endif

