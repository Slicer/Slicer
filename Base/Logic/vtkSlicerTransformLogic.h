/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  This file was partly developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

=========================================================================auto=*/

///  vtkSlicerTransformLogic - slicer logic class for volumes manipulation
///
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the volumes

#ifndef __vtkSlicerTransformLogic_h
#define __vtkSlicerTransformLogic_h

// SlicerLogic includes
#include "vtkSlicerBaseLogic.h"

// MRMLLogic includes
#include <vtkMRMLAbstractLogic.h>

// MRML includes
class vtkMRMLScene;
class vtkMRMLSliceNode;
class vtkMRMLTransformableNode;
class vtkMRMLTransformDisplayNode;
class vtkMRMLTransformNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLVolumeNode;

// VTK includes
class vtkImageData;
class vtkMatrix4x4;
class vtkPointSet;
class vtkPolyData;

class VTK_SLICER_BASE_LOGIC_EXPORT vtkSlicerTransformLogic : public vtkMRMLAbstractLogic
{
  public:

  /// The Usual vtk class functions
  static vtkSlicerTransformLogic *New();
  vtkTypeMacro(vtkSlicerTransformLogic,vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) { Superclass::PrintSelf(os, indent); };

  /// Apply the associated transform to the transformable node. Return true
  /// on success, false otherwise.
  static bool hardenTransform(vtkMRMLTransformableNode* node);

  ///
  /// Read transform from file
  vtkMRMLTransformNode* AddTransform (const char* filename, vtkMRMLScene *scene);

  ///
  /// Write transform's data to a specified file
  int SaveTransform (const char* filename, vtkMRMLTransformNode *transformNode);

  /// Generate polydata for 2D transform visualization
  /// Return true on success.
  static bool GetVisualization2d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMRMLSliceNode* sliceNode);

  /// Generate polydata for 2D transform visualization
  /// Return true on success.
  static bool GetVisualization2d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize);

  /// Generate polydata for 3D transform visualization
  /// roiToRAS defines the ROI origin and direction.
  /// roiSize defines the ROI size (in the ROI coordinate system spacing)  .
  /// Return true on success.
  static bool GetVisualization3d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize);

  /// Generate polydata for 3D transform visualization
  /// Region node can be slice (vtkMRMLSliceNode), volume (vtkMRMLVolumeNode), region of interest (vtkMRMLAnnotationROINode), or model (vtkMRMLModelNode).
  /// Return true on success.
  static bool GetVisualization3d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMRMLNode* regionNode);

  /// Name of the scalar array that stores the displacement magnitude values
  /// in polydata returned by GetVisualization2d and GetVisualization3d.
  static const char* GetVisualizationDisplacementMagnitudeScalarName();

  /// Create a volume node that contains the transform displacement in each voxel.
  /// If magnitude is true then a scalar volume is created, each voxel containing the magnitude of the displacement.
  /// If magnitude is false then a 3-component scalar volume is created, each voxel containing the displacement vector.
  /// referenceVolumeNode specifies the volume origin, spacing, extent, and orientation
  vtkMRMLScalarVolumeNode* CreateDisplacementVolumeFromTransform(vtkMRMLTransformNode* inputTransformNode, vtkMRMLVolumeNode* referenceVolumeNode, bool magnitude=true);

  /// Take samples from the displacement field and store the magnitude in an image volume
  /// The extents of the output image must be set before calling this method.
  /// The origin and spacing attributes of the output image are ignored (origin, spacing, and axis directions
  /// are all specified by ijkToRAS).
  static void GetTransformedPointSamplesAsMagnitudeImage(vtkImageData* outputMagnitudeImage, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* ijkToRAS);

  /// Take samples from the displacement field and store the vector components in an image volume
  /// The extents of the output image must be set before calling this method.
  /// The origin and spacing attributes of the output image are ignored (origin, spacing, and axis directions
  /// are all specified by ijkToRAS).
  static void GetTransformedPointSamplesAsVectorImage(vtkImageData* outputVectorImage, vtkMRMLTransformNode* inputTransformNode, vtkMatrix4x4* ijkToRAS);

  enum TransformKind
  {
    TRANSFORM_OTHER,
    TRANSFORM_LINEAR,
    TRANSFORM_BSPLINE,
    TRANSFORM_GRID
  };
  /// Returns TRANSFORM_LINEAR if the node contains a simple linear transform.
  /// Returns TRANSFORM_BSPLINE if the node contains a bspline transform with an
  ///   optional additive or composite bulk component.
  /// Returns TRANSFORM_GRID if the node contains a simple grid transform.
  /// Returns TRANSFORM_OTHER in all other cases.
  static TransformKind GetTransformKind(vtkMRMLTransformNode *transformNode);

protected:
  vtkSlicerTransformLogic();
  virtual ~vtkSlicerTransformLogic();
  vtkSlicerTransformLogic(const vtkSlicerTransformLogic&);
  void operator=(const vtkSlicerTransformLogic&);

  /// Generate glyph for 2D transform visualization
  /// \sa GetVisualization2d
  static void GetGlyphVisualization2d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize);
  /// Generate glyph for 3D transform visualization
  /// \sa GetVisualization3d
  static void GetGlyphVisualization3d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize);

  /// Generate grid for 2D transform visualization
  /// \sa GetVisualization2d
  static void GetGridVisualization2d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize);
  /// Generate grid for 3D transform visualization
  /// \sa GetVisualization3d
  static void GetGridVisualization3d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize);

  /// Generate contours for 2D transform visualization
  /// \sa GetVisualization2d
  static void GetContourVisualization2d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize);
  /// Generate contours for 3D transform visualization
  /// \sa GetVisualization3d
  static void GetContourVisualization3d(vtkPolyData* output_RAS, vtkMRMLTransformDisplayNode* displayNode, vtkMatrix4x4* roiToRAS, int* roiSize);

  /// Return the number of samples in each grid
  static int GetGridSubdivision(vtkMRMLTransformDisplayNode* displayNode);

  /// Add lines to the gridPolyData to make it a grid. If warpedGrid is specified then a warped grid is generated, too.
  static void CreateGrid(vtkPolyData* outputGrid_RAS, vtkMRMLTransformDisplayNode* displayNode, int numGridPoints[3], vtkPolyData* outputWarpedGrid_RAS=NULL);

  /// Takes samples from the displacement field specified by the transformation on a uniform grid
  /// and stores it in an unstructured grid.
  /// gridToRAS specifies the grid origin, direction, and spacing
  /// gridSize is a 3-component int array specifying the dimension of the grid
  static void GetTransformedPointSamples(vtkPointSet* outputPointSet_RAS, vtkMRMLTransformNode* inputTransformNode,
    vtkMatrix4x4* gridToRAS, int* gridSize);

  /// Takes samples from the displacement field specified by the transformation on a slice
  /// and stores it in an unstructured grid.
  /// pointGroupSize: the number of points will be N*pointGroupSize (the actual number will be returned in numGridPoints[3])
  static void GetTransformedPointSamplesOnSlice(vtkPointSet* outputPointSet_RAS, vtkMRMLTransformNode* inputTransformNode,
    vtkMatrix4x4* sliceToRAS, double* fieldOfViewOrigin, double* fieldOfViewSize, double pointSpacing, int pointGroupSize=1, int* numGridPoints=0);

  /// Takes samples from the displacement field specified by the transformation on a 3D ROI
  /// and stores it in an unstructured grid.
  /// pointGroupSize: the number of points will be N*pointGroupSize (the actual number will be returned in numGridPoints[3])
  static void GetTransformedPointSamplesOnRoi(vtkPointSet* outputPointSet_RAS, vtkMRMLTransformNode* inputTransformNode,
    vtkMatrix4x4* roiToRAS, int* roiSize, double pointSpacingMm, int pointGroupSize=1, int* numGridPoints=0);

};

#endif
