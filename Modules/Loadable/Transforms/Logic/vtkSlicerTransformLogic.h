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
#include "vtkSlicerTransformsModuleLogicExport.h"
#include <vtkSlicerModuleLogic.h>

// STD includes
#include <vector>

// MRML includes
class vtkMRMLDisplayableNode;
class vtkMRMLMessageCollection;
class vtkMRMLModelNode;
class vtkMRMLScalarVolumeNode;
class vtkMRMLScene;
class vtkMRMLTransformableNode;
class vtkMRMLTransformNode;
class vtkMRMLVectorVolumeNode;
class vtkMRMLVolumeNode;

// VTK includes
class vtkDataArray;
class vtkImageData;
class vtkMatrix4x4;
class vtkPointSet;
class vtkScatteredBSplineTransform;

class VTK_SLICER_TRANSFORMS_MODULE_LOGIC_EXPORT vtkSlicerTransformLogic : public vtkSlicerModuleLogic
{
public:
  /// The Usual vtk class functions
  static vtkSlicerTransformLogic* New();
  vtkTypeMacro(vtkSlicerTransformLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override { Superclass::PrintSelf(os, indent); }

  /// Apply the associated transform to the transformable node. Return true
  /// on success, false otherwise.
  /// This method is kept for backward compatibility only, it is recommended to use
  /// vtkMRMLTransformableNode::HardenTransform() method instead.
  static bool hardenTransform(vtkMRMLTransformableNode* node);

  ///
  /// Read transform from file
  vtkMRMLTransformNode* AddTransform(const char* filename, vtkMRMLScene* scene, vtkMRMLMessageCollection* userMessages = nullptr);

  /// Create a volume node that contains the transform displacement in each voxel.
  /// If magnitude is true then a scalar volume is created, each voxel containing the magnitude of the displacement.
  /// If magnitude is false then a 3-component scalar volume is created, each voxel containing the displacement vector.
  /// referenceVolumeNode specifies the volume origin, spacing, extent, and orientation.
  /// If existingOutputVolumeNode is specified then instead of creating a new volume node, that existing node will be updated.
  vtkMRMLVolumeNode* CreateDisplacementVolumeFromTransform(vtkMRMLTransformNode* inputTransformNode,
                                                           vtkMRMLVolumeNode* referenceVolumeNode = nullptr,
                                                           bool magnitude = true,
                                                           vtkMRMLVolumeNode* existingOutputVolumeNode = nullptr);

  /// Convert the input transform to a grid transform.
  /// If referenceVolumeNode is specified then it will determine the origin, spacing, extent, and orientation of the displacement field.
  /// If existingOutputTransformNode is specified then instead of creating a new transform node, that existing node will be updated.
  vtkMRMLTransformNode* ConvertToGridTransform(vtkMRMLTransformNode* inputTransformNode,
                                               vtkMRMLVolumeNode* referenceVolumeNode = nullptr,
                                               vtkMRMLTransformNode* existingOutputTransformNode = nullptr);

  /// Set the transform to a grid transform (to parent, so CreateDisplacementVolumeFromTransform round-trips) from the RAS
  /// voxel displacements (copied) of a 3-component vector volume. Returns false for other volumes or under a non-linear transform.
  bool ImportTransformFromDisplacementVolume(vtkMRMLTransformNode* transformNode, vtkMRMLVectorVolumeNode* displacementVolumeNode);

  /// Fit a vtkScatteredBSplineTransform (SetScatteredBSplineTransformDefaults()) to the displacements at the points of a model
  /// (named point data array, or active vectors), in mesh coordinates. Returns false if the mesh has no such array.
  /// If keepLinked, it becomes the transform to parent with the model as displacement model: it follows the mesh and
  /// stays editable. Its file holds its parameters, so it can only be used with its model. Otherwise the node gets a
  /// fixed B-spline copy, which other applications can read too.
  bool ImportTransformFromModel(vtkMRMLTransformNode* transformNode, vtkMRMLModelNode* modelNode, const char* displacementArrayName = nullptr, bool keepLinked = true);

  /// The named point data array of a mesh, or its active vectors if no name is given.
  /// Returns nullptr if the mesh has no points or the array does not have 3 components.
  static vtkDataArray* GetDisplacementArray(vtkPointSet* mesh, const char* displacementArrayName);

  /// Set the defaults of a scattered B-spline transform for the displacements of a mesh (not set as input): GridBoundsMargin a tenth of the
  /// points diagonal, then linear trend; automatic border falloff (zero at twice the largest displacement from the points) and zero
  /// at the border of the fitted region (BorderMode ZeroAtBorder); control points down to 1/100 of the fitted region apart; tolerance
  /// 1/1000 of the max.
  static void SetScatteredBSplineTransformDefaults(vtkScatteredBSplineTransform* transform, vtkPointSet* mesh, const char* displacementArrayName);

  /// Take samples from the displacement field and store the magnitude in an image volume
  /// The extents of the output image must be set before calling this method.
  /// The origin and spacing attributes of the output image are ignored (origin, spacing, and axis directions
  /// are all specified by ijkToRAS).
  /// If transformToWorld is true then transform to world is returned, otherwise transform from world is returned.
  /// Returns true on success.
  static bool GetTransformedPointSamplesAsMagnitudeImage(vtkImageData* outputMagnitudeImage,
                                                         vtkMRMLTransformNode* inputTransformNode,
                                                         vtkMatrix4x4* ijkToRAS,
                                                         bool transformToWorld = true);

  /// Take samples from the displacement field and store the vector components in an image volume
  /// The extents of the output image must be set before calling this method.
  /// The origin and spacing attributes of the output image are ignored (origin, spacing, and axis directions
  /// are all specified by ijkToRAS).
  /// If transformToWorld is true then transform to world is returned, otherwise transform from world is returned.
  /// Returns true on success.
  static bool GetTransformedPointSamplesAsVectorImage(vtkImageData* outputVectorImage,
                                                      vtkMRMLTransformNode* inputTransformNode,
                                                      vtkMatrix4x4* ijkToRAS,
                                                      bool transformToWorld = true);

  /// Return the list of nodes that are transformed by the given node.
  /// If recursive is True, this be recursively called on any transform node
  /// that might be transformed by the given node. Otherwise, only the
  /// nodes immediately transformed by the given transform are returned.
  static void GetTransformedNodes(vtkMRMLScene* scene, vtkMRMLTransformNode* transformNode, std::vector<vtkMRMLDisplayableNode*>& transformedNodes, bool recursive = true);

  /// Return the RAS bounding box around the list of given nodes
  /// using GetRASBounds. Only the nodes with a valid bounding box are taken
  /// into account.
  /// \sa GetNodesBounds()
  static void GetNodesRASBounds(const std::vector<vtkMRMLDisplayableNode*>& nodes, double bounds[6]);

  /// Return the bounding box around the list of given nodes
  /// using GetBounds. Only the nodes with a valid bounding box
  /// are taken into account.
  /// \sa GetNodesRASBounds()
  static void GetNodesBounds(const std::vector<vtkMRMLDisplayableNode*>& nodes, double bounds[6]);

  enum TransformKind
  {
    TRANSFORM_OTHER,
    TRANSFORM_LINEAR,
    TRANSFORM_BSPLINE,
    TRANSFORM_GRID,
    TRANSFORM_THINPLATESPLINE
  };
  /// Returns TRANSFORM_LINEAR if the node contains a simple linear transform.
  /// Returns TRANSFORM_BSPLINE if the node contains a bspline transform with an
  ///   optional additive or composite bulk component.
  /// Returns TRANSFORM_GRID if the node contains a simple grid transform.
  /// Returns TRANSFORM_THINPLATESPLINE if the node contains a simple thin plate spline transform.
  /// Returns TRANSFORM_OTHER in all other cases.
  static TransformKind GetTransformKind(vtkMRMLTransformNode* transformNode);

  /// Create file readers and writers of the module.
  std::vector<vtkSmartPointer<vtkMRMLFileIOHandler>> CreateFileIOHandlers() override;

protected:
  vtkSlicerTransformLogic();
  ~vtkSlicerTransformLogic() override;
  vtkSlicerTransformLogic(const vtkSlicerTransformLogic&);
  void operator=(const vtkSlicerTransformLogic&);
};

#endif
