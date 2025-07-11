/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// GeneralizedReformat Logic includes
#include "vtkSlicerGeneralizedReformatLogic.h"
#include "vtkMRMLScalarVectorDWIVolumeResampler.h"

// Slice includes
#include <vtkSlicerConfigure.h> // For Slicer_BUILD_CLI_SUPPORT

// MRML includes
#include <vtkMRMLMarkupsCurveNode.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLScalarVolumeNode.h>
#include <vtkMRMLTransformNode.h>
#include <vtkMRMLVolumeNode.h>

// vtkAddon includes
#include <vtkAddonMathUtilities.h>
#include <vtkOrientedGridTransform.h>

// VTK includes
#include <vtkAppendPolyData.h>
#include <vtkDoubleArray.h>
#include <vtkImageData.h>
#include <vtkIntArray.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkOrientedGridTransform.h>
#include <vtkParallelTransportFrame.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkPlane.h>
#include <vtkPlaneSource.h>
#include <vtkTransform.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerGeneralizedReformatLogic);

//----------------------------------------------------------------------------
vtkSlicerGeneralizedReformatLogic::vtkSlicerGeneralizedReformatLogic()
{
#ifdef Slicer_BUILD_CLI_SUPPORT
  this->RegisterVolumeResampler("ResampleScalarVectorDWIVolume", vtkNew<vtkMRMLScalarVectorDWIVolumeResampler>().GetPointer());
#endif
}

//----------------------------------------------------------------------------
vtkSlicerGeneralizedReformatLogic::~vtkSlicerGeneralizedReformatLogic() {}

//----------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
//----------------------------------------------------------------------------
bool vtkSlicerGeneralizedReformatLogic::GetPointsProjectedToPlane(vtkPoints* pointsArrayIn, vtkMatrix4x4* transformWorldToPlane, vtkPoints* pointsArrayOut)
{
  if (pointsArrayIn == nullptr)
  {
    vtkErrorMacro("GetPointsProjectedToPlane: pointsArrayIn is nullptr");
    return false;
  }
  if (transformWorldToPlane == nullptr)
  {
    vtkErrorMacro("GetPointsProjectedToPlane: transformWorldToPlane is nullptr");
    return false;
  }
  if (pointsArrayOut == nullptr)
  {
    vtkErrorMacro("GetPointsProjectedToPlane: pointsArrayOut is nullptr");
    return false;
  }

  // Returns points projected to the plane coordinate system (plane normal = plane Z axis).

  // Compute the inverse transformation
  vtkNew<vtkMatrix4x4> transformPlaneToWorld;
  vtkMatrix4x4::Invert(transformWorldToPlane, transformPlaneToWorld);

  const vtkIdType numPoints = pointsArrayIn->GetNumberOfPoints();
  pointsArrayOut->SetNumberOfPoints(numPoints);
  double pIn[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pMiddle[4] = { 0.0, 0.0, 0.0, 1.0 };
  double pOut[4] = { 0.0, 0.0, 0.0, 1.0 };

  for (vtkIdType pointIndex = 0; pointIndex < numPoints; ++pointIndex)
  {
    // Note: uses only the first three elements of pIn
    pointsArrayIn->GetPoint(pointIndex, static_cast<double*>(pIn));
    // Point positions in the plane coordinate system:
    transformWorldToPlane->MultiplyPoint(pIn, pMiddle);
    // Projected point positions in the plane coordinate system:
    pMiddle[2] = 0.0;
    // Projected point positions in the world coordinate system:
    transformPlaneToWorld->MultiplyPoint(pMiddle, pOut);
    pointsArrayOut->SetPoint(pointIndex, pOut[0], pOut[1], pOut[2]);
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerGeneralizedReformatLogic::ComputeStraighteningTransform(vtkMRMLTransformNode* transformToStraightenedNode,
                                                                      vtkMRMLMarkupsCurveNode* curveNode,
                                                                      const double sliceSizeMm[2],
                                                                      double outputSpacingMm,
                                                                      bool stretching,
                                                                      double rotationDeg,
                                                                      vtkMRMLModelNode* reslicingPlanesModelNode)
{
  struct vtkMRMLNodeCleanup
  {
  public:
    vtkMRMLNodeCleanup(vtkMRMLScene* scene, vtkMRMLNode* node)
      : Scene(scene)
      , Node(node)
    {
    }
    ~vtkMRMLNodeCleanup()
    {
      if (this->Scene != nullptr)
      {
        this->Scene->RemoveNode(this->Node);
      }
    }

  private:
    vtkMRMLScene* Scene{ nullptr };
    vtkMRMLNode* Node{ nullptr };
  };

  if (transformToStraightenedNode == nullptr)
  {
    vtkErrorMacro("ComputeStraighteningTransform: transformToStraightenedNode is nullptr");
    return false;
  }
  if (curveNode == nullptr)
  {
    vtkErrorMacro("ComputeStraighteningTransform: curveNode is nullptr");
    return false;
  }

  // Create a temporary resampled curve
  const double resamplingCurveSpacing = outputSpacingMm * this->TransformSpacingFactor;
  vtkPoints* originalCurvePoints = curveNode->GetCurvePointsWorld();
  vtkNew<vtkPoints> sampledPoints;
  if (!vtkMRMLMarkupsCurveNode::ResamplePoints(originalCurvePoints, sampledPoints, resamplingCurveSpacing, false))
  {
    vtkErrorMacro("ComputeStraighteningTransform: Resampling curve failed");
    return false;
  }
  vtkMRMLMarkupsCurveNode* resampledCurveNode =
    vtkMRMLMarkupsCurveNode::SafeDownCast(this->GetMRMLScene()->AddNewNodeByClass("vtkMRMLMarkupsCurveNode", "CurvedPlanarReformat_resampled_curve_temp"));
  vtkMRMLNodeCleanup nodeCleanup(this->GetMRMLScene(), resampledCurveNode);

  resampledCurveNode->SetNumberOfPointsPerInterpolatingSegment(1);
  resampledCurveNode->SetCurveTypeToLinear();
  resampledCurveNode->SetControlPointPositionsWorld(sampledPoints);

  vtkPoints* resampledCurvePointsWorld = resampledCurveNode->GetCurvePointsWorld();
  if (resampledCurvePointsWorld == nullptr || resampledCurvePointsWorld->GetNumberOfPoints() < 3)
  {
    vtkErrorMacro("ComputeStraighteningTransform: Not enough resampled curve points");
    return false;
  }
  vtkNew<vtkPlane> curveNodePlane;
  vtkAddonMathUtilities::FitPlaneToPoints(resampledCurvePointsWorld, curveNodePlane);

  // Z axis (from first curve point to last, this will be the straightened curve long axis)
  double curveStartPoint[3] = { 0.0 };
  double curveEndPoint[3] = { 0.0 };
  resampledCurveNode->GetNthControlPointPositionWorld(0, curveStartPoint);
  resampledCurveNode->GetNthControlPointPositionWorld(resampledCurveNode->GetNumberOfControlPoints() - 1, curveEndPoint);
  double transformGridAxisZ[3] = { 0.0 };
  vtkMath::Subtract(curveEndPoint, curveStartPoint, transformGridAxisZ);
  vtkMath::Normalize(transformGridAxisZ);

  double transformGridAxisX[3] = { 0.0 };
  double transformGridAxisY[3] = { 0.0 };
  if (stretching)
  {
    // Y axis = best fit plane normal
    curveNodePlane->GetNormal(transformGridAxisY);

    // X axis normalize
    vtkMath::Cross(transformGridAxisZ, transformGridAxisY, transformGridAxisX);
    vtkMath::Normalize(transformGridAxisX);

    // Make sure that Z axis is orthogonal to X and Y
    double orthogonalizedTransformGridAxisZ[3] = { 0.0 };
    vtkMath::Cross(transformGridAxisX, transformGridAxisY, orthogonalizedTransformGridAxisZ);
    vtkMath::Normalize(orthogonalizedTransformGridAxisZ);
    if (vtkMath::Dot(transformGridAxisZ, orthogonalizedTransformGridAxisZ) > 0)
    {
      for (int axis = 0; axis < 3; ++axis)
      {
        transformGridAxisZ[axis] = orthogonalizedTransformGridAxisZ[axis];
      }
    }
    else
    {
      for (int axis = 0; axis < 3; ++axis)
      {
        transformGridAxisZ[axis] = -orthogonalizedTransformGridAxisZ[axis];
        transformGridAxisX[axis] = -transformGridAxisX[axis];
      }
    }
  }
  else
  {
    // X axis = average X axis of curve, to minimize torsion (and so have a
    // simple displacement field, which can be robustly inverted)
    double sumCurveAxisX_RAS[3] = { 0.0 };
    for (int gridK = 0; gridK < resampledCurveNode->GetNumberOfControlPoints(); ++gridK)
    {
      vtkNew<vtkMatrix4x4> curvePointToWorld;
      resampledCurveNode->GetCurvePointToWorldTransformAtPointIndex(resampledCurveNode->GetCurvePointIndexFromControlPointIndex(gridK), curvePointToWorld);
      const double curveAxisX_RAS[3] = { curvePointToWorld->GetElement(0, 0), curvePointToWorld->GetElement(1, 0), curvePointToWorld->GetElement(2, 0) };
      vtkMath::Add(sumCurveAxisX_RAS, curveAxisX_RAS, sumCurveAxisX_RAS);
    }
    vtkMath::Normalize(sumCurveAxisX_RAS);
    for (int axis = 0; axis < 3; ++axis)
    {
      transformGridAxisX[axis] = sumCurveAxisX_RAS[axis];
    }

    // Y axis normalize
    vtkMath::Cross(transformGridAxisZ, transformGridAxisX, transformGridAxisY);
    vtkMath::Normalize(transformGridAxisY);

    // Make sure that X axis is orthogonal to Y and Z
    vtkMath::Cross(transformGridAxisY, transformGridAxisZ, transformGridAxisX);
    vtkMath::Normalize(transformGridAxisX);
  }

  // Rotate by rotationDeg around the Z axis
  vtkNew<vtkMatrix4x4> gridDirectionMatrix;
  gridDirectionMatrix->Identity();
  for (int axis = 0; axis < 3; ++axis)
  {
    gridDirectionMatrix->SetElement(axis, 0, transformGridAxisX[axis]);
    gridDirectionMatrix->SetElement(axis, 1, transformGridAxisY[axis]);
    gridDirectionMatrix->SetElement(axis, 2, transformGridAxisZ[axis]);
  }

  vtkNew<vtkTransform> gridDirectionTransform;
  gridDirectionTransform->Concatenate(gridDirectionMatrix);
  gridDirectionTransform->RotateZ(rotationDeg);

  vtkNew<vtkMatrix4x4> rotatedGridMatrix;
  gridDirectionTransform->GetMatrix(rotatedGridMatrix);
  for (int axis = 0; axis < 3; ++axis)
  {
    transformGridAxisX[axis] = rotatedGridMatrix->GetElement(axis, 0);
    transformGridAxisY[axis] = rotatedGridMatrix->GetElement(axis, 1);
    transformGridAxisZ[axis] = rotatedGridMatrix->GetElement(axis, 2);
  }

  if (stretching)
  {
    // Project curve points to grid YZ plane
    vtkNew<vtkMatrix4x4> transformFromGridYZPlane;
    transformFromGridYZPlane->Identity();
    const double* origin = curveNodePlane->GetOrigin();
    for (int axis = 0; axis < 3; ++axis)
    {
      transformFromGridYZPlane->SetElement(axis, 0, transformGridAxisY[axis]);
      transformFromGridYZPlane->SetElement(axis, 1, transformGridAxisZ[axis]);
      transformFromGridYZPlane->SetElement(axis, 2, transformGridAxisX[axis]);
      transformFromGridYZPlane->SetElement(axis, 3, origin[axis]);
    }
    vtkNew<vtkMatrix4x4> transformToGridYZPlane;
    vtkMatrix4x4::Invert(transformFromGridYZPlane, transformToGridYZPlane);

    vtkPoints* originalCurvePointsArray = curveNode->GetCurvePoints();
    vtkNew<vtkPoints> curvePointsProjected_RAS;
    this->GetPointsProjectedToPlane(originalCurvePointsArray, transformToGridYZPlane, curvePointsProjected_RAS);
    for (int pointIndex = resampledCurveNode->GetNumberOfControlPoints() - 1; pointIndex >= 0; --pointIndex)
    {
      resampledCurveNode->RemoveNthControlPoint(pointIndex);
    }
    for (int pointIndex = 0; pointIndex < curvePointsProjected_RAS->GetNumberOfPoints(); ++pointIndex)
    {
      resampledCurveNode->AddControlPoint(curvePointsProjected_RAS->GetPoint(pointIndex));
    }

    // After projection, resampling is needed to get uniform distances
    originalCurvePoints = resampledCurveNode->GetCurvePointsWorld();
    vtkNew<vtkPoints> uniformlySampledPoints;
    if (!vtkMRMLMarkupsCurveNode::ResamplePoints(originalCurvePoints, uniformlySampledPoints, resamplingCurveSpacing, false))
    {
      vtkErrorMacro("ComputeStraighteningTransform: second call to resampling curve failed");
      return false;
    }
    for (int controlPointIndex = resampledCurveNode->GetNumberOfControlPoints() - 1; controlPointIndex >= 0; --controlPointIndex)
    {
      resampledCurveNode->RemoveNthControlPoint(controlPointIndex);
    }
    for (int pointIndex = 0; pointIndex < curvePointsProjected_RAS->GetNumberOfPoints(); ++pointIndex)
    {
      resampledCurveNode->AddControlPoint(uniformlySampledPoints->GetPoint(pointIndex));
    }
  }

  // Origin (makes the grid centered at the curve)
  const double curveLength = resampledCurveNode->GetCurveLengthWorld();
  double transformGridOrigin[3] = { 0.0 };
  curveNodePlane->GetOrigin(transformGridOrigin);
  for (int axis = 0; axis < 3; ++axis)
  {
    transformGridOrigin[axis] -= transformGridAxisX[axis] * sliceSizeMm[0] / 2.0;
    transformGridOrigin[axis] -= transformGridAxisY[axis] * sliceSizeMm[1] / 2.0;
    transformGridOrigin[axis] -= transformGridAxisZ[axis] * curveLength / 2.0;
  }

  // Create grid transform
  // Each corner of each slice is mapped from the original volume's reformatted slice
  // to the straightened volume slice.
  // The grid transform contains one vector at the corner of each slice.
  // The transform is in the same space and orientation as the straightened volume.
  const int numberOfSlices = resampledCurveNode->GetNumberOfControlPoints();
  const int gridDimensions[3] = { 2, 2, numberOfSlices };
  const double gridSpacing[3] = { sliceSizeMm[0], sliceSizeMm[1], resamplingCurveSpacing };
  vtkNew<vtkMatrix4x4> newGridDirectionMatrix;
  newGridDirectionMatrix->Identity();
  for (int axis = 0; axis < 3; ++axis)
  {
    newGridDirectionMatrix->SetElement(axis, 0, transformGridAxisX[axis]);
    newGridDirectionMatrix->SetElement(axis, 1, transformGridAxisY[axis]);
    newGridDirectionMatrix->SetElement(axis, 2, transformGridAxisZ[axis]);
  }

  vtkNew<vtkImageData> gridImage;
  gridImage->SetOrigin(transformGridOrigin);
  gridImage->SetDimensions(gridDimensions);
  gridImage->SetSpacing(gridSpacing);
  gridImage->AllocateScalars(VTK_DOUBLE, 3);
  vtkNew<vtkOrientedGridTransform> transform;
  transform->SetDisplacementGridData(gridImage);
  transform->SetGridDirectionMatrix(newGridDirectionMatrix);
  transformToStraightenedNode->SetAndObserveTransformFromParent(transform);
  vtkGridTransform* transformGrid = vtkGridTransform::SafeDownCast(transformToStraightenedNode->GetTransformFromParent());
  if (transformGrid == nullptr)
  {
    vtkErrorMacro("ComputeStraighteningTransform: transformToStraightenedNode must have a transform from parent");
    return false;
  }

  vtkSmartPointer<vtkAppendPolyData> appender;
  if (reslicingPlanesModelNode != nullptr)
  {
    appender = vtkSmartPointer<vtkAppendPolyData>::New();
  }

  // Currently there is no API to set PreferredInitialNormalVector in the curve
  // coordinate system, therefore a new coordinate system generator must be set up:
  vtkNew<vtkParallelTransportFrame> curveCoordinateSystemGeneratorWorld;
  curveCoordinateSystemGeneratorWorld->SetInputData(resampledCurveNode->GetCurveWorld());
  curveCoordinateSystemGeneratorWorld->SetPreferredInitialNormalVector(transformGridAxisX);
  curveCoordinateSystemGeneratorWorld->Update();
  vtkPolyData* curvePoly = curveCoordinateSystemGeneratorWorld->GetOutput();
  vtkPointData* pointData = curvePoly->GetPointData();
  vtkDoubleArray* normals = vtkDoubleArray::SafeDownCast(pointData->GetAbstractArray(curveCoordinateSystemGeneratorWorld->GetNormalsArrayName()));
  vtkDoubleArray* binormals = vtkDoubleArray::SafeDownCast(pointData->GetAbstractArray(curveCoordinateSystemGeneratorWorld->GetBinormalsArrayName()));

  // Compute displacements
  vtkImageData* displacementGrid = transformGrid->GetDisplacementGrid();
  vtkDataArray* transformDisplacements_RAS = displacementGrid->GetPointData()->GetScalars();

  for (int gridK = 0; gridK < gridDimensions[2]; ++gridK)
  {
    // The curve's built-in coordinate system generator could be used like this
    // (if it had PreferredInitialNormalVector exposed):
    //
    // vtkNew<vtkMatrix4x4> curvePointToWorld;
    // resampledCurveNode->GetCurvePointToWorldTransformAtPointIndex(
    //   resampledCurveNode->GetCurvePointIndexFromControlPointIndex(gridK), curvePointToWorld);
    // double curveAxisX_RAS[3] = { curvePointToWorld->GetElement(0, 0),
    //                              curvePointToWorld->GetElement(1, 0),
    //                              curvePointToWorld->GetElement(2, 0) };
    // double curveAxisY_RAS[3] = { curvePointToWorld->GetElement(0, 1),
    //                              curvePointToWorld->GetElement(1, 1),
    //                              curvePointToWorld->GetElement(2, 1) };
    // double curvePoint_RAS[3] = { curvePointToWorld->GetElement(0, 3),
    //                              curvePointToWorld->GetElement(1, 3),
    //                              curvePointToWorld->GetElement(2, 3) };
    // But now we get the values from our own coordinate system generator:
    const int curvePointIndex = resampledCurveNode->GetCurvePointIndexFromControlPointIndex(gridK);
    const double* curveAxisX_RAS = normals->GetTuple3(curvePointIndex);
    const double* curveAxisY_RAS = binormals->GetTuple3(curvePointIndex);
    const double* curvePoint_RAS = curvePoly->GetPoint(curvePointIndex);

    vtkSmartPointer<vtkPlaneSource> plane;
    for (int gridJ = 0; gridJ < gridDimensions[1]; ++gridJ)
    {
      for (int gridI = 0; gridI < gridDimensions[0]; ++gridI)
      {
        double straightenedVolume_RAS[3] = { 0.0 };
        double inputVolume_RAS[3] = { 0.0 };
        for (int axis = 0; axis < 3; ++axis)
        {
          straightenedVolume_RAS[axis] = transformGridOrigin[axis] + gridI * gridSpacing[0] * transformGridAxisX[axis] + gridJ * gridSpacing[1] * transformGridAxisY[axis]
                                         + gridK * gridSpacing[2] * transformGridAxisZ[axis];
          inputVolume_RAS[axis] = curvePoint_RAS[axis] + (gridI - 0.5) * sliceSizeMm[0] * curveAxisX_RAS[axis] + (gridJ - 0.5) * sliceSizeMm[1] * curveAxisY_RAS[axis];
        }
        if (reslicingPlanesModelNode)
        {
          if (gridI == 0 && gridJ == 0)
          {
            plane = vtkSmartPointer<vtkPlaneSource>::New();
            plane->SetOrigin(inputVolume_RAS);
          }
          else if (gridI == 1 && gridJ == 0)
          {
            plane->SetPoint1(inputVolume_RAS);
          }
          else if (gridI == 0 && gridJ == 1)
          {
            plane->SetPoint2(inputVolume_RAS);
          }
        }
        const int index = (gridK * gridDimensions[1] + gridJ) * gridDimensions[0] + gridI;
        const double difference_RAS[3] = { inputVolume_RAS[0] - straightenedVolume_RAS[0],
                                           inputVolume_RAS[1] - straightenedVolume_RAS[1],
                                           inputVolume_RAS[2] - straightenedVolume_RAS[2] };
        transformDisplacements_RAS->SetTuple(index, difference_RAS);
      }
    }
    if (reslicingPlanesModelNode)
    {
      plane->Update();
      appender->AddInputData(plane->GetOutput());
    }
  }

  displacementGrid->GetPointData()->GetScalars()->Modified();
  displacementGrid->Modified();

  if (reslicingPlanesModelNode)
  {
    appender->Update();
    if (!reslicingPlanesModelNode->GetPolyData())
    {
      reslicingPlanesModelNode->CreateDefaultDisplayNodes();
      reslicingPlanesModelNode->GetDisplayNode()->SetVisibility2D(true);
    }
    reslicingPlanesModelNode->SetAndObservePolyData(appender->GetOutput());
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerGeneralizedReformatLogic::StraightenVolume(vtkMRMLScalarVolumeNode* outputStraightenedVolume,
                                                         vtkMRMLScalarVolumeNode* inputVolume,
                                                         const double outputStraightenedVolumeSpacing[3],
                                                         vtkMRMLTransformNode* straighteningTransformNode)
{
  if (outputStraightenedVolume == nullptr)
  {
    vtkErrorMacro("StraightenVolume: outputStraightenedVolume is nullptr");
    return false;
  }
  if (inputVolume == nullptr)
  {
    vtkErrorMacro("StraightenVolume: inputVolume is nullptr");
    return false;
  }
  if (straighteningTransformNode == nullptr)
  {
    vtkErrorMacro("StraightenVolume: straighteningTransformNode is nullptr");
    return false;
  }

  vtkOrientedGridTransform* gridTransform = vtkOrientedGridTransform::SafeDownCast(straighteningTransformNode->GetTransformFromParentAs("vtkOrientedGridTransform"));
  if (!gridTransform)
  {
    vtkErrorMacro("StraightenVolume: straightening transform must contain a vtkOrientedGridTransform from parent");
    return false;
  }

  // Get transformation grid geometry
  vtkMatrix4x4* gridIjkToRasDirectionMatrix = gridTransform->GetGridDirectionMatrix();
  vtkImageData* gridTransformImage = gridTransform->GetDisplacementGrid();
  double gridOrigin[3] = { 0.0 };
  gridTransformImage->GetOrigin(gridOrigin);
  double gridSpacing[3] = { 0.0 };
  gridTransformImage->GetSpacing(gridSpacing);
  int gridDimensions[3] = { 0 };
  gridTransformImage->GetDimensions(gridDimensions);
  const double gridExtentMm[3] = { gridSpacing[0] * (gridDimensions[0] - 1), gridSpacing[1] * (gridDimensions[1] - 1), gridSpacing[2] * (gridDimensions[2] - 1) };

  // Compute IJK to RAS matrix of output volume
  // Get grid axis directions
  vtkNew<vtkMatrix4x4> straightenedVolumeIJKToRASMatrix;
  straightenedVolumeIJKToRASMatrix->DeepCopy(gridIjkToRasDirectionMatrix);
  // Apply scaling
  for (int row = 0; row < 4; ++row)
  {
    for (int col = 0; col < 3; ++col)
    {
      straightenedVolumeIJKToRASMatrix->SetElement(row, col, straightenedVolumeIJKToRASMatrix->GetElement(row, col) * outputStraightenedVolumeSpacing[col]);
    }
  }
  // Set origin
  for (int dim = 0; dim < 3; ++dim)
  {
    straightenedVolumeIJKToRASMatrix->SetElement(dim, 3, gridOrigin[dim]);
  }

  vtkNew<vtkImageData> outputStraightenedImageData;
  outputStraightenedImageData->SetExtent(0,
                                         static_cast<int>(gridExtentMm[0] / outputStraightenedVolumeSpacing[0]) - 1,
                                         0,
                                         static_cast<int>(gridExtentMm[1] / outputStraightenedVolumeSpacing[1]) - 1,
                                         0,
                                         static_cast<int>(gridExtentMm[2] / outputStraightenedVolumeSpacing[2]) - 1);
  outputStraightenedImageData->AllocateScalars(inputVolume->GetImageData()->GetScalarType(), inputVolume->GetImageData()->GetNumberOfScalarComponents());
  outputStraightenedVolume->SetAndObserveImageData(outputStraightenedImageData);
  outputStraightenedVolume->SetIJKToRASMatrix(straightenedVolumeIJKToRASMatrix);

  // Resample input volume to straightened volume
  std::string volumeResamplerName = "ResampleScalarVectorDWIVolume";
  if (!this->IsVolumeResamplerRegistered(volumeResamplerName))
  {
    vtkErrorMacro("StraightenVolume: failed to get CLI logic for module: " << volumeResamplerName);
    return false;
  }

  vtkMRMLVolumeNode* outputVolume = outputStraightenedVolume;
  vtkMRMLTransformNode* resamplingTransform = straighteningTransformNode;
  vtkMRMLVolumeNode* referenceVolume = outputStraightenedVolume;
  int interpolationType =
    (inputVolume->IsA("vtkMRMLLabelMapVolumeNode") ? vtkMRMLAbstractVolumeResampler::InterpolationTypeNearestNeighbor : vtkMRMLAbstractVolumeResampler::InterpolationTypeBSpline);
  const vtkMRMLAbstractVolumeResampler::ResamplingParameters resamplingParameters;

  bool success = this->ResampleVolume(volumeResamplerName, inputVolume, outputVolume, resamplingTransform, referenceVolume, interpolationType, resamplingParameters);
  if (!success)
  {
    vtkErrorMacro("StraightenVolume: Failed to resample volume using " << volumeResamplerName);
    return false;
  }

  outputStraightenedVolume->CreateDefaultDisplayNodes();
  vtkMRMLDisplayNode* volumeDisplayNode = inputVolume->GetDisplayNode();
  if (volumeDisplayNode)
  {
    outputStraightenedVolume->GetDisplayNode()->CopyContent(volumeDisplayNode);
  }
  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerGeneralizedReformatLogic::ProjectVolume(vtkMRMLScalarVolumeNode* outputProjectedVolume, vtkMRMLScalarVolumeNode* inputStraightenedVolume, int projectionAxisIndex)
{
  if (outputProjectedVolume == nullptr)
  {
    vtkErrorMacro("ProjectVolume: outputProjectedVolume is nullptr");
    return false;
  }
  if (inputStraightenedVolume == nullptr)
  {
    vtkErrorMacro("ProjectVolume: inputStraightenedVolume is nullptr");
    return false;
  }
  if ((projectionAxisIndex < 0) || (projectionAxisIndex >= 3))
  {
    vtkErrorMacro("ProjectVolume: projectionAxisIndex is out of range");
    return false;
  }

  // Create a new vtkImageData for the projected volume
  vtkNew<vtkImageData> projectedImageData;
  outputProjectedVolume->SetAndObserveImageData(projectedImageData);

  // Get the image data from the input straightened volume
  vtkImageData* straightenedImageData = inputStraightenedVolume->GetImageData();
  if (!straightenedImageData)
  {
    vtkErrorMacro("ProjectVolume: input straightened volume must have image data");
    return false;
  }

  // Get the dimensions of the straightened volume
  int outputImageDimensions[3] = { 0 };
  straightenedImageData->GetDimensions(outputImageDimensions);
  outputImageDimensions[projectionAxisIndex] = 1; // Set the projection axis to size 1
  projectedImageData->SetDimensions(outputImageDimensions);

  // Allocate scalars for the projected image
  projectedImageData->AllocateScalars(straightenedImageData->GetScalarType(), straightenedImageData->GetNumberOfScalarComponents());

  // Get arrays of the input and output volumes
  vtkDataArray* outputProjectedVolumeArray = projectedImageData->GetPointData()->GetScalars();
  vtkDataArray* inputStraightenedVolumeArray = straightenedImageData->GetPointData()->GetScalars();

  // Perform the projection (mean intensity projection along the specified axis)
  int dims[3] = { 0 };
  projectedImageData->GetDimensions(dims);

  if (projectionAxisIndex == 0)
  {
    for (int sliceY = 0; sliceY < dims[1]; ++sliceY)
    {
      for (int sliceZ = 0; sliceZ < dims[2]; ++sliceZ)
      {
        double sum = 0.0;
        int count = 0;
        for (int sliceX = 0; sliceX < dims[0]; ++sliceX)
        {
          const int index = sliceX + dims[0] * (sliceY + dims[1] * sliceZ);
          sum += inputStraightenedVolumeArray->GetComponent(index, 0); // Assuming single component
          count++;
        }
        const int outputIndex = sliceY + dims[1] * sliceZ;
        outputProjectedVolumeArray->SetComponent(outputIndex, 0, sum / count);
      }
    }
  }
  else if (projectionAxisIndex == 1)
  {
    for (int sliceX = 0; sliceX < dims[0]; ++sliceX)
    {
      for (int sliceZ = 0; sliceZ < dims[2]; ++sliceZ)
      {
        double sum = 0.0;
        int count = 0;
        for (int sliceY = 0; sliceY < dims[1]; ++sliceY)
        {
          const int index = sliceX + dims[0] * (sliceY + dims[1] * sliceZ);
          sum += inputStraightenedVolumeArray->GetComponent(index, 0); // Assuming single component
          count++;
        }
        const int outputIndex = sliceX + dims[0] * sliceZ;
        outputProjectedVolumeArray->SetComponent(outputIndex, 0, sum / count);
      }
    }
  }
  else
  {
    for (int sliceX = 0; sliceX < dims[0]; ++sliceX)
    {
      for (int sliceY = 0; sliceY < dims[1]; ++sliceY)
      {
        double sum = 0.0;
        int count = 0;
        for (int sliceZ = 0; sliceZ < dims[2]; ++sliceZ)
        {
          int index = sliceX + dims[0] * (sliceY + dims[1] * sliceZ);
          sum += inputStraightenedVolumeArray->GetComponent(index, 0); // Assuming single component
          count++;
        }
        int outputIndex = sliceX + dims[0] * sliceY;
        outputProjectedVolumeArray->SetComponent(outputIndex, 0, sum / count);
      }
    }
  }

  // Mark the volume as modified
  outputProjectedVolume->GetImageData()->Modified();

  // Shift projection image into the center of the input image
  vtkNew<vtkMatrix4x4> ijkToRas;
  inputStraightenedVolume->GetIJKToRASMatrix(ijkToRas);

  double curvePointToWorldArray[4][4] = { { 0.0 } };
  for (int row = 0; row < 4; ++row)
  {
    for (int col = 0; col < 4; ++col)
    {
      curvePointToWorldArray[row][col] = ijkToRas->GetElement(row, col);
    }
  }

  double origin[3] = { 0.0 };
  for (int dim = 0; dim < 3; ++dim)
  {
    origin[dim] = curvePointToWorldArray[3][dim];
  }

  double offsetToCenterDirectionVector[3] = { 0.0 };
  for (int dim = 0; dim < 3; ++dim)
  {
    offsetToCenterDirectionVector[dim] = curvePointToWorldArray[projectionAxisIndex][dim];
  }

  double offsetToCenterDirectionLength = inputStraightenedVolume->GetImageData()->GetDimensions()[projectionAxisIndex] * inputStraightenedVolume->GetSpacing()[projectionAxisIndex];

  double newOrigin[3] = { 0.0 };
  for (int dim = 0; dim < 3; ++dim)
  {
    newOrigin[dim] = origin[dim] + offsetToCenterDirectionVector[dim] * offsetToCenterDirectionLength;
  }

  ijkToRas->SetElement(0, 3, newOrigin[0]);
  ijkToRas->SetElement(1, 3, newOrigin[1]);
  ijkToRas->SetElement(2, 3, newOrigin[2]);

  outputProjectedVolume->SetIJKToRASMatrix(ijkToRas);

  // Create default display nodes
  outputProjectedVolume->CreateDefaultDisplayNodes();

  return true;
}

//----------------------------------------------------------------------------
bool vtkSlicerGeneralizedReformatLogic::ResampleVolume(std::string& resamplerName,
                                                       vtkMRMLVolumeNode* inputVolume,
                                                       vtkMRMLVolumeNode* outputVolume,
                                                       vtkMRMLTransformNode* resamplingTransform,
                                                       vtkMRMLVolumeNode* referenceVolume,
                                                       int interpolationType,
                                                       const vtkMRMLAbstractVolumeResampler::ResamplingParameters& resamplingParameters)
{
  vtkMRMLAbstractVolumeResampler* resampler = this->GetVolumeResampler(resamplerName);
  if (!resampler)
  {
    vtkErrorMacro("ResampleVolume: resampler not registered " << resamplerName);
    return false;
  }
  return resampler->Resample(inputVolume, outputVolume, resamplingTransform, referenceVolume, interpolationType, resamplingParameters);
}

//----------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::RegisterVolumeResampler(const std::string& resamplerName, vtkMRMLAbstractVolumeResampler* resampler)
{
  if (resamplerName.empty())
  {
    vtkErrorMacro("RegisterVolumeResampler: invalid sampler name.");
    return;
  }
  if (this->IsVolumeResamplerRegistered(resamplerName))
  {
    return;
  }
  this->Resamplers[resamplerName] = resampler;
}

//----------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::UnregisterVolumeResampler(const std::string& resamplerName)
{
  if (!this->IsVolumeResamplerRegistered(resamplerName))
  {
    return;
  }
  this->Resamplers.erase(resamplerName);
}

//----------------------------------------------------------------------------
bool vtkSlicerGeneralizedReformatLogic::IsVolumeResamplerRegistered(const std::string& resamplerName)
{
  return this->Resamplers.find(resamplerName) != this->Resamplers.end();
}

//----------------------------------------------------------------------------
vtkMRMLAbstractVolumeResampler* vtkSlicerGeneralizedReformatLogic::GetVolumeResampler(const std::string& resamplerName)
{
  if (!this->IsVolumeResamplerRegistered(resamplerName))
  {
    return nullptr;
  }
  return this->Resamplers[resamplerName];
}

//---------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::OnMRMLSceneNodeAdded(vtkMRMLNode* vtkNotUsed(node)) {}

//---------------------------------------------------------------------------
void vtkSlicerGeneralizedReformatLogic::OnMRMLSceneNodeRemoved(vtkMRMLNode* vtkNotUsed(node)) {}
