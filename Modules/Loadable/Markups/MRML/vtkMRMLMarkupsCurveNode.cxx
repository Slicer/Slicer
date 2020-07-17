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

#include "vtkMRMLMarkupsCurveNode.h"

// MRML includes
#include "vtkCurveGenerator.h"
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLTransformNode.h"
#include "vtkMRMLUnitNode.h"
#include "vtkSlicerDijkstraGraphGeodesicPath.h"

// VTK includes
#include <vtkArrayCalculator.h>
#include <vtkBoundingBox.h>
#include <vtkCallbackCommand.h>
#include <vtkCellLocator.h>
#include <vtkCleanPolyData.h>
#include <vtkCommand.h>
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkFrenetSerretFrame.h>
#include <vtkGeneralTransform.h>
#include <vtkGenericCell.h>
#include <vtkLine.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkOBBTree.h>
#include <vtkObjectFactory.h>
#include <vtkPassThroughFilter.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkStringArray.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkTriangleFilter.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsCurveNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsCurveNode::vtkMRMLMarkupsCurveNode()
{
  // Set RequiredNumberOfControlPoints to a very high number to remain
  // in place mode after placing a curve point.
  this->RequiredNumberOfControlPoints = 1e6;

  this->CleanFilter = vtkSmartPointer<vtkCleanPolyData>::New();

  this->TriangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
  this->TriangleFilter->SetInputConnection(this->CleanFilter->GetOutputPort());

  this->SurfaceToLocalTransformer = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  this->SurfaceToLocalTransformer->SetTransform(vtkNew<vtkGeneralTransform>());
  this->SurfaceToLocalTransformer->SetInputConnection(this->TriangleFilter->GetOutputPort());

  this->SurfaceScalarCalculator = vtkSmartPointer<vtkArrayCalculator>::New();
  this->SurfaceScalarCalculator->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());
  this->SurfaceScalarCalculator->AddObserver(vtkCommand::ModifiedEvent, this->MRMLCallbackCommand);
  this->SurfaceScalarCalculator->SetAttributeTypeToPointData();
  this->SurfaceScalarCalculator->SetResultArrayName("weights");
  this->SurfaceScalarCalculator->SetResultArrayType(VTK_FLOAT);
  this->SetSurfaceDistanceWeightingFunction("activeScalar");

  this->PassThroughFilter = vtkSmartPointer<vtkPassThroughFilter>::New();
  this->PassThroughFilter->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());

  this->CurveGenerator->SetCurveTypeToCardinalSpline();
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(10);
  this->CurveGenerator->SetSurfaceCostFunctionType(vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE);

  vtkNew<vtkIntArray> events;
  events->InsertNextTuple1(vtkCommand::ModifiedEvent);
  events->InsertNextTuple1(vtkMRMLModelNode::MeshModifiedEvent);
  events->InsertNextTuple1(vtkMRMLTransformableNode::TransformModifiedEvent);
  this->AddNodeReferenceRole(this->GetShortestDistanceSurfaceNodeReferenceRole(), this->GetShortestDistanceSurfaceNodeReferenceMRMLAttributeName(), events);

  this->ActiveScalar = "";
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsCurveNode::~vtkMRMLMarkupsCurveNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(curveType, CurveType);
  vtkMRMLWriteXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkMRMLWriteXMLEnumMacro(surfaceCostFunctionType, SurfaceCostFunctionType);
  vtkMRMLWriteXMLStringMacro(surfaceDistanceWeightingFunction, SurfaceDistanceWeightingFunction);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(curveType, CurveType);
  vtkMRMLReadXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkMRMLReadXMLEnumMacro(surfaceCostFunctionType, SurfaceCostFunctionType);
  vtkMRMLReadXMLStringMacro(surfaceDistanceWeightingFunction, SurfaceDistanceWeightingFunction);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(CurveType);
  vtkMRMLCopyIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkMRMLCopyEnumMacro(SurfaceCostFunctionType);
  vtkMRMLCopyStringMacro(SurfaceDistanceWeightingFunction);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(CurveType);
  vtkMRMLPrintIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkMRMLPrintEnumMacro(SurfaceCostFunctionType);
  vtkMRMLPrintStringMacro(SurfaceDistanceWeightingFunction);
  vtkMRMLPrintEndMacro();
}

//---------------------------------------------------------------------------
vtkPoints* vtkMRMLMarkupsCurveNode::GetCurvePointsWorld()
{
  this->CurveGenerator->Update();
  vtkPolyData* curvePoly = this->GetCurveWorld();
  if (!curvePoly)
    {
    return nullptr;
    }
  return curvePoly->GetPoints();
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetCurveLength(vtkPoints* curvePoints, bool closedCurve,
  vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  if (!curvePoints || curvePoints->GetNumberOfPoints() < 2)
    {
    return 0.0;
    }
  if (startCurvePointIndex < 0)
    {
    vtkGenericWarningMacro("Invalid startCurvePointIndex=" << startCurvePointIndex << ", using 0 instead");
    startCurvePointIndex = 0;
    }
  vtkIdType lastCurvePointIndex = curvePoints->GetNumberOfPoints()-1;
  if (numberOfCurvePoints >= 0 && startCurvePointIndex + numberOfCurvePoints - 1 < lastCurvePointIndex)
    {
    lastCurvePointIndex = startCurvePointIndex + numberOfCurvePoints - 1;
    }

  double length = 0.0;
  double previousPoint[3] = { 0.0 };
  double nextPoint[3] = { 0.0 };
  curvePoints->GetPoint(startCurvePointIndex, previousPoint);
  for (vtkIdType curvePointIndex = startCurvePointIndex + 1; curvePointIndex <= lastCurvePointIndex; curvePointIndex++)
    {
    curvePoints->GetPoint(curvePointIndex, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  // Add length of closing segment
  if (closedCurve && (numberOfCurvePoints < 0 || numberOfCurvePoints >= curvePoints->GetNumberOfPoints()))
    {
    curvePoints->GetPoint(0, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    }
  return length;
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetCurveLengthWorld(
  vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  return vtkMRMLMarkupsCurveNode::GetCurveLength(points, this->CurveClosed,
    startCurvePointIndex, numberOfCurvePoints);
}

//---------------------------------------------------------------------------
double vtkMRMLMarkupsCurveNode::GetCurveLengthBetweenStartEndPointsWorld(vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex)
{
  if (startCurvePointIndex <= endCurvePointIndex)
  {
    return this->GetCurveLengthWorld(startCurvePointIndex, endCurvePointIndex - startCurvePointIndex + 1);
  }
  else
  {
    // wrap around
    return this->GetCurveLengthWorld(0, endCurvePointIndex + 1) + this->GetCurveLengthWorld(startCurvePointIndex, -1);
  }
}
//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::SetControlPointLabels(vtkStringArray* labels, vtkPoints* points)
{
  return this->SetControlPointLabelsWorld(labels, points);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::ResampleCurveSurface(double controlPointDistance, vtkMRMLModelNode* modelNode, double maximumSearchRadiusTolerance)
{
  if(!modelNode)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::ResampleCurveSurface failed: Constraint surface is not valid");
    return false;
    }
  if (maximumSearchRadiusTolerance <= 0 || maximumSearchRadiusTolerance > 1)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::ResampleCurveSurface failed: Invalid search radius");
    return false;
    }
  vtkPoints* originalPoints = this->GetCurvePointsWorld();
  // If there is less than two points there is no segment to resample. Control points are already assumed to be on the surface.
  if (!originalPoints  || originalPoints->GetNumberOfPoints() < 2)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::ResampleCurveSurface failed: Invalid number of control points");
    return false;
    }
  vtkNew<vtkPoints> originalControlPoints;
  this->GetControlPointPositionsWorld(originalControlPoints);
  vtkNew<vtkStringArray> originalLabels;
  this->GetControlPointLabels(originalLabels);

  vtkNew<vtkPoints> interpolatedPoints;
  vtkMRMLMarkupsCurveNode::ResamplePoints(originalPoints, interpolatedPoints, controlPointDistance, this->CurveClosed);

  vtkSmartPointer<vtkPolyData> surfacePolydata = modelNode->GetPolyData();
  if(!surfacePolydata)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::ResampleCurveSurface failed: Constraint surface polydata is not valid");
    return false;
    }

  vtkMRMLTransformNode* parentTransformNode = modelNode->GetParentTransformNode();
  if (parentTransformNode)
    {
    vtkNew<vtkGeneralTransform> modelToWorldTransform;
    parentTransformNode->GetTransformToWorld(modelToWorldTransform);
    vtkNew<vtkTransformPolyDataFilter> transformPolydataFilter;
    transformPolydataFilter->SetInputData(surfacePolydata);
    transformPolydataFilter->SetTransform(modelToWorldTransform);
    transformPolydataFilter->Update();
    surfacePolydata = transformPolydataFilter->GetOutput();
    }

  // vtkStaticCellLocator may imporove speed, but FindClosestPoint function is not yet supported
  vtkNew<vtkPointLocator> pointLocator;
  pointLocator->SetDataSet(surfacePolydata);
  pointLocator->BuildLocator();

  vtkSmartPointer<vtkDataArray> normalVectorArray = vtkArrayDownCast<vtkDataArray>(surfacePolydata->GetPointData()->GetArray("Normals"));
  if(!normalVectorArray)
    {
    vtkNew<vtkPolyDataNormals> normalFilter;
    normalFilter->SetInputData(surfacePolydata);
    normalFilter->ComputePointNormalsOn();
    normalFilter->Update();
    vtkPolyData* normalPolydata = normalFilter->GetOutput();
    normalVectorArray = vtkArrayDownCast<vtkDataArray>(normalPolydata->GetPointData()->GetNormals());
    if (!normalVectorArray)
      {
      vtkErrorMacro("vtkMRMLMarkupsCurveNode::ResamplePoints failed: Unable to find or calculate normals");
      return false;
      }
    }

  double interpolatedPoint[3] = { 0.0 };
  double projectedPoint[3] = { 0.0 };
  double segmentStartPoint[3] = { 0.0 };
  double segmentEndPoint[3] = { 0.0 };
  double distanceToStart = 0.0;
  double distanceToEnd = 0.0;
  vtkNew<vtkPoints> pointNormalArray;
  // Project each mesh point onto the most external model surface point along a normal vector ray. The normal vector
  // for each resampled point is estimated from normal vectors at the closest control points on either side.
  // If there is no intersection between the model and normal vector ray, use the closest model point.
  for (vtkIdType controlPointIndex = 0; controlPointIndex < interpolatedPoints->GetNumberOfPoints(); controlPointIndex++)
    {
    interpolatedPoints->GetPoint(controlPointIndex, interpolatedPoint);
    vtkIdType segmentStartIndex = vtkMRMLMarkupsNode::GetClosestControlPointIndexToPositionWorld(interpolatedPoint);
    // get nearest control point
    originalControlPoints->GetPoint(segmentStartIndex, segmentStartPoint);
    // get projection of point to curve
    this->GetClosestPointPositionAlongCurveWorld(interpolatedPoint, projectedPoint);
    distanceToStart = vtkMath::Distance2BetweenPoints(segmentStartPoint, projectedPoint);
    if (segmentStartIndex == 0)
      {
      originalControlPoints->GetPoint(segmentStartIndex + 1, segmentEndPoint);
      distanceToEnd = vtkMath::Distance2BetweenPoints(segmentEndPoint, projectedPoint);
      }
    else if (segmentStartIndex == (originalControlPoints->GetNumberOfPoints() - 1))
      {
      originalControlPoints->GetPoint(segmentStartIndex - 1, segmentEndPoint);
      distanceToEnd = vtkMath::Distance2BetweenPoints(segmentEndPoint, projectedPoint);
      }
    else
      {
      double* segmentEndPoint1 = originalControlPoints->GetPoint(segmentStartIndex - 1);
      double dist1 = vtkMath::Distance2BetweenPoints(segmentEndPoint1, projectedPoint);
      double* segmentEndPoint2 = originalControlPoints->GetPoint(segmentStartIndex + 1);
      double dist2 = vtkMath::Distance2BetweenPoints(segmentEndPoint2, projectedPoint);

      if ((dist1 < dist2) && dist1 < vtkMath::Distance2BetweenPoints(segmentEndPoint1, segmentStartPoint))
        {
        segmentEndPoint[0] = segmentEndPoint1[0];
        segmentEndPoint[1] = segmentEndPoint1[1];
        segmentEndPoint[2] = segmentEndPoint1[2];
        distanceToEnd = dist1;
        }
      else
        {
        segmentEndPoint[0] = segmentEndPoint2[0];
        segmentEndPoint[1] = segmentEndPoint2[1];
        segmentEndPoint[2] = segmentEndPoint2[2];
        distanceToEnd = dist2;
        }
      }

    // estimate normal vector from control points on either side of projected point
    vtkIdType pointIdStart = pointLocator->FindClosestPoint(segmentStartPoint);
    double startNormal[3] = { 0.0 };
    normalVectorArray->GetTuple(pointIdStart, startNormal);
    vtkIdType pointIdEnd = pointLocator->FindClosestPoint(segmentEndPoint);
    double endNormal[3] = { 0.0 };
    normalVectorArray->GetTuple(pointIdEnd, endNormal);

    double startWeight = distanceToEnd / (distanceToStart + distanceToEnd);
    double endWeight = distanceToStart / (distanceToStart + distanceToEnd);
    double rayDirection[3] = { 0.0 };
    rayDirection[0] = (startWeight*startNormal[0]) + (endWeight*endNormal[0]);
    rayDirection[1] = (startWeight*startNormal[1]) + (endWeight*endNormal[1]);
    rayDirection[2] = (startWeight*startNormal[2]) + (endWeight*endNormal[2]);
    vtkMath::Normalize(rayDirection);
    pointNormalArray->InsertNextPoint(rayDirection);
    }

  vtkNew<vtkPoints> snappedToSurfaceControlPoints;
  vtkMRMLMarkupsCurveNode::ConstrainPointsToSurface(interpolatedPoints, pointNormalArray, surfacePolydata,
    snappedToSurfaceControlPoints, maximumSearchRadiusTolerance);

  this->SetControlPointPositionsWorld(snappedToSurfaceControlPoints);
  this->SetControlPointLabelsWorld(originalLabels, originalControlPoints);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::ConstrainPointsToSurface(vtkPoints* originalPoints, vtkPoints* normalVectors, vtkPolyData* surfacePolydata,
  vtkPoints* surfacePoints, double maximumSearchRadiusTolerance)
{
  if (originalPoints->GetNumberOfPoints()!= normalVectors->GetNumberOfPoints())
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::ConstrainPointsToSurface failed: invalid inputs");
    return false;
    }
  if (maximumSearchRadiusTolerance <= 0.0 || maximumSearchRadiusTolerance > 1.0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::ConstrainPointsToSurface failed: Invalid search radius");
    return false;
    }
  vtkNew<vtkOBBTree> surfaceObbTree;
  surfaceObbTree->SetDataSet(surfacePolydata);
  surfaceObbTree->BuildLocator();
  double tolerance = surfaceObbTree->GetTolerance();

  vtkNew<vtkPointLocator> pointLocator;
  pointLocator->SetDataSet(surfacePolydata);
  pointLocator->BuildLocator();

  double originalPoint[3] = { 0.0 };
  double rayDirection[3] = { 0.0 };
  double exteriorPoint[3] = { 0.0 };

  // Curves are expected to be close to surface. The maximumSearchRadiusTolerance
  // sets the allowable projection distance as a percentage of the model's
  // bounding box diagonal in world coordinate system.
  double polydataBounds[6] = { 0.0 };
  surfacePolydata->GetBounds(polydataBounds);
  vtkBoundingBox modelBoundingBox;
  modelBoundingBox.AddBounds(polydataBounds);
  double polydataDiagonalLength = modelBoundingBox.GetDiagonalLength();
  double rayLength = maximumSearchRadiusTolerance*sqrt(polydataDiagonalLength);

  for (vtkIdType controlPointIndex = 0; controlPointIndex < originalPoints->GetNumberOfPoints(); controlPointIndex++)
    {
    originalPoints->GetPoint(controlPointIndex, originalPoint);
    normalVectors->GetPoint(controlPointIndex, rayDirection);
    //cast ray and find model intersection point
    double rayEndPoint[3] = { 0.0 };
    rayEndPoint[0] = originalPoint[0] + rayDirection[0] * rayLength;
    rayEndPoint[1] = originalPoint[1] + rayDirection[1] * rayLength;
    rayEndPoint[2] = originalPoint[2] + rayDirection[2] * rayLength;

    double t = 0.0;
    double pcoords[3] = { 0.0 };
    int subId = 0;
    vtkIdType cellId = 0;
    vtkNew <vtkGenericCell> cell;
    int foundIntersection = surfaceObbTree->IntersectWithLine(rayEndPoint, originalPoint, tolerance, t, exteriorPoint, pcoords, subId, cellId, cell);
    if(foundIntersection == 0)
      {
      //if no intersection, reverse direction of normal vector ray
      rayEndPoint[0] = originalPoint[0] + rayDirection[0] * -rayLength;
      rayEndPoint[1] = originalPoint[1] + rayDirection[1] * -rayLength;
      rayEndPoint[2] = originalPoint[2] + rayDirection[2] * -rayLength;
      int foundIntersection = surfaceObbTree->IntersectWithLine(originalPoint, rayEndPoint, tolerance, t, exteriorPoint, pcoords, subId, cellId, cell);
      if(foundIntersection == 0)
        {
        //if no intersection in either direction, use closest mesh point
        vtkIdType closestPointId = pointLocator->FindClosestPoint(originalPoint);
        surfacePolydata->GetPoint(closestPointId, exteriorPoint);
        vtkGenericWarningMacro("No intersections found");
        }
      }
    surfacePoints->InsertNextPoint(exteriorPoint);
    }
  return true;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ResampleCurveWorld(double controlPointDistance)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints() < 2)
    {
    return;
    }

  vtkNew<vtkPoints> interpolatedPoints;
  vtkMRMLMarkupsCurveNode::ResamplePoints(points, interpolatedPoints, controlPointDistance, this->CurveClosed);

  vtkNew<vtkPoints> originalPoints;
  this->GetControlPointPositionsWorld(originalPoints);
  vtkNew<vtkStringArray> originalLabels;
  this->GetControlPointLabels(originalLabels);

  this->SetControlPointPositionsWorld(interpolatedPoints);
  this->SetControlPointLabelsWorld(originalLabels, originalPoints);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::ResamplePoints(vtkPoints* originalPoints, vtkPoints* sampledPoints,
  double samplingDistance, bool closedCurve)
{
  if (!originalPoints || !sampledPoints || samplingDistance <= 0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::ResamplePoints failed: invalid inputs");
    return false;
    }

  if (originalPoints->GetNumberOfPoints() < 2)
    {
    sampledPoints->DeepCopy(originalPoints);
    return true;
    }

  double distanceFromLastSampledPoint = 0;
  double remainingSegmentLength = 0;
  double previousCurvePoint[3] = { 0.0 };
  originalPoints->GetPoint(0, previousCurvePoint);
  sampledPoints->Reset();
  sampledPoints->InsertNextPoint(previousCurvePoint);
  vtkIdType numberOfOriginalPoints = originalPoints->GetNumberOfPoints();
  bool addClosingSegment = closedCurve; // for closed curves, add a closing segment that connects last and first points
  double* currentCurvePoint = nullptr;
  for (vtkIdType originalPointIndex = 0; originalPointIndex < numberOfOriginalPoints || addClosingSegment; originalPointIndex++)
    {
    if (originalPointIndex >= numberOfOriginalPoints)
      {
      // this is the closing segment
      addClosingSegment = false;
      currentCurvePoint = originalPoints->GetPoint(0);
      }
    else
      {
      currentCurvePoint = originalPoints->GetPoint(originalPointIndex);
      }

    double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(currentCurvePoint, previousCurvePoint));
    if (segmentLength <= 0.0)
      {
      continue;
      }
    remainingSegmentLength = distanceFromLastSampledPoint + segmentLength;
    if (remainingSegmentLength >= samplingDistance)
      {
      double segmentDirectionVector[3] =
        {
        (currentCurvePoint[0] - previousCurvePoint[0]) / segmentLength,
        (currentCurvePoint[1] - previousCurvePoint[1]) / segmentLength,
        (currentCurvePoint[2] - previousCurvePoint[2]) / segmentLength
        };
      // distance of new sampled point from previous curve point
      double distanceFromLastInterpolatedPoint = samplingDistance - distanceFromLastSampledPoint;
      while (remainingSegmentLength >= samplingDistance)
        {
        double newSampledPoint[3] =
          {
          previousCurvePoint[0] + segmentDirectionVector[0] * distanceFromLastInterpolatedPoint,
          previousCurvePoint[1] + segmentDirectionVector[1] * distanceFromLastInterpolatedPoint,
          previousCurvePoint[2] + segmentDirectionVector[2] * distanceFromLastInterpolatedPoint
          };
        sampledPoints->InsertNextPoint(newSampledPoint);
        distanceFromLastSampledPoint = 0;
        distanceFromLastInterpolatedPoint += samplingDistance;
        remainingSegmentLength -= samplingDistance;
        }
      distanceFromLastSampledPoint = remainingSegmentLength;
      }
    else
      {
      distanceFromLastSampledPoint += segmentLength;
      }
    previousCurvePoint[0] = currentCurvePoint[0];
    previousCurvePoint[1] = currentCurvePoint[1];
    previousCurvePoint[2] = currentCurvePoint[2];
    }

  // Make sure the resampled curve has the same size as the original
  // but avoid having very long or very short line segments at the end.
  if (closedCurve)
  {
    // Closed curve
    // Ideally, remainingSegmentLength would be equal to samplingDistance.
    if (remainingSegmentLength < samplingDistance * 0.5)
      {
      // last segment would be too short, so remove the last point and adjust position of second last point
      double lastPointPosition[3] = { 0.0 };
      vtkIdType foundClosestPointIndex = -1; // not used
      if (vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, foundClosestPointIndex,
        0, -(2.0*samplingDistance+remainingSegmentLength)/2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
        }
      else
        {
        // something went wrong, we could not add a point, therefore just remove the last point
        sampledPoints->SetNumberOfPoints(sampledPoints->GetNumberOfPoints() - 1);
        }
      }
    else
      {
      // last segment is only slightly shorter than the sampling distance
      // so just adjust the position of the last point
      double lastPointPosition[3] = { 0.0 };
      vtkIdType foundClosestPointIndex = -1; // not used
      if (vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(lastPointPosition, foundClosestPointIndex,
        0, -(samplingDistance+remainingSegmentLength)/2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPointPosition);
        }
      }
  }
  else
    {
    // Open curve
    // Ideally, remainingSegmentLength would be equal to 0.
    if (remainingSegmentLength > samplingDistance * 0.5)
      {
      // last segment would be much longer than the sampling distance, so add an extra point
      double secondLastPointPosition[3] = { 0.0 };
      vtkIdType foundClosestPointIndex = -1; // not used
      if (vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(secondLastPointPosition, foundClosestPointIndex,
        originalPoints->GetNumberOfPoints() - 1, -(samplingDistance+remainingSegmentLength) / 2.0, originalPoints, closedCurve))
        {
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, secondLastPointPosition);
        sampledPoints->InsertNextPoint(originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        }
      else
        {
        // something went wrong, we could not add a point, therefore just adjust the last point position
        sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
          originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
        }
      }
    else
      {
      // last segment is only slightly longer than the sampling distance
      // so we just adjust the position of last point
      sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1,
        originalPoints->GetPoint(originalPoints->GetNumberOfPoints() - 1));
      }
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType& foundClosestPointIndex,
  vtkIdType startCurvePointId, double distanceFromStartPoint, vtkPoints* curvePoints, bool closedCurve)
{
  vtkIdType numberOfCurvePoints = (curvePoints != nullptr ? curvePoints->GetNumberOfPoints() : 0);
  if (numberOfCurvePoints == 0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetPositionAlongCurve failed: invalid input points");
    foundClosestPointIndex = -1;
    return false;
    }
  if (startCurvePointId < 0 || startCurvePointId >= numberOfCurvePoints)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetPositionAlongCurve failed: startCurvePointId is out of range");
    foundClosestPointIndex = -1;
    return false;
    }
  if (numberOfCurvePoints == 1 || distanceFromStartPoint == 0)
    {
    curvePoints->GetPoint(startCurvePointId, foundCurvePosition);
    foundClosestPointIndex = startCurvePointId;
    if (distanceFromStartPoint > 0.0)
      {
      vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetPositionAlongCurve failed: non-zero distance"
        " is requested but only 1 point is available");
      return false;
      }
    else
      {
      return true;
      }
    }
  vtkIdType idIncrement = (distanceFromStartPoint > 0 ? 1 : -1);
  double remainingDistanceFromStartPoint = abs(distanceFromStartPoint);
  double previousPoint[3] = { 0.0 };
  curvePoints->GetPoint(startCurvePointId, previousPoint);
  vtkIdType pointId = startCurvePointId;
  bool curveConfirmedToBeNonZeroLength = false;
  double lastSegmentLength = 0;
  while (true)
    {
    pointId += idIncrement;

    // if reach the end then wrap around for closed curve, terminate search for open curve
    if (pointId < 0 || pointId >= numberOfCurvePoints)
      {
      if (closedCurve)
        {
        if (!curveConfirmedToBeNonZeroLength)
          {
          if (vtkMRMLMarkupsCurveNode::GetCurveLength(curvePoints, closedCurve) == 0.0)
            {
            foundClosestPointIndex = -1;
            return false;
            }
          curveConfirmedToBeNonZeroLength = true;
          }
        pointId = (pointId < 0 ? numberOfCurvePoints : -1);
        continue;
        }
      else
        {
        // reached end of curve before getting at the requested distance
        // return closest
        foundClosestPointIndex = (pointId < 0 ? 0 : numberOfCurvePoints - 1);
        curvePoints->GetPoint(foundClosestPointIndex, foundCurvePosition);
        return false;
        }
      }

    // determine how much closer we are now
    double* nextPoint = curvePoints->GetPoint(pointId);
    lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(nextPoint, previousPoint));
    remainingDistanceFromStartPoint -= lastSegmentLength;

    if (remainingDistanceFromStartPoint <= 0)
      {
      // reached the requested distance (and probably a bit more)
      for (int i=0; i<3; i++)
        {
        foundCurvePosition[i] = nextPoint[i] +
          remainingDistanceFromStartPoint * (nextPoint[i] - previousPoint[i]) / lastSegmentLength;
        }
      if (fabs(remainingDistanceFromStartPoint) <= fabs(remainingDistanceFromStartPoint + lastSegmentLength))
        {
        foundClosestPointIndex = pointId;
        }
      else
        {
        foundClosestPointIndex = pointId-1;
        }
      break;
      }

    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPointsWorld(vtkPoints* sampledPoints,
  double samplingDistance, vtkIdType startCurvePointIndex, vtkIdType endCurvePointIndex)
{
  if (!sampledPoints || samplingDistance <= 0)
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPoints failed: invalid inputs");
    return false;
    }
  vtkPoints* allPoints = this->GetCurvePointsWorld();
  if (!allPoints)
    {
    return false;
    }
  if (startCurvePointIndex < 0 || endCurvePointIndex >= allPoints->GetNumberOfPoints())
    {
    vtkGenericWarningMacro("vtkMRMLMarkupsCurveNode::GetSampledCurvePointsBetweenStartEndPoints failed: invalid inputs ("
    << "requested " << startCurvePointIndex << ".." << endCurvePointIndex << " range, but there are "
    << allPoints->GetNumberOfPoints() << " curve points)");
    return false;
    }
  vtkNew<vtkPoints> points;
  if (startCurvePointIndex <= endCurvePointIndex)
    {
    points->InsertPoints(0, endCurvePointIndex - startCurvePointIndex + 1, startCurvePointIndex, allPoints);
    }
  else
    {
    // wrap around
    vtkNew<vtkPoints> points;
    points->InsertPoints(0, allPoints->GetNumberOfPoints() - startCurvePointIndex, startCurvePointIndex, allPoints);
    points->InsertPoints(points->GetNumberOfPoints(), endCurvePointIndex + 1, 0, allPoints);
    }
  return vtkMRMLMarkupsCurveNode::ResamplePoints(points, sampledPoints, samplingDistance, this->CurveClosed);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetClosestCurvePointIndexToPositionWorld(const double posWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points)
    {
    return -1;
    }
  this->TransformedCurvePolyLocator->Update(); // or ->BuildLocator()?
  return this->TransformedCurvePolyLocator->FindClosestPoint(posWorld);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetCurvePointIndexFromControlPointIndex(int controlPointIndex)
{
  if (this->CurveGenerator->IsInterpolatingCurve())
    {
    return controlPointIndex * this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment();
    }
  else
    {
    double controlPointPositionWorld[3] = { 0.0 };
    this->GetNthControlPointPositionWorld(controlPointIndex, controlPointPositionWorld);
    return GetClosestCurvePointIndexToPositionWorld(controlPointPositionWorld);
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetCurveDirectionAtPointIndexWorld(vtkIdType curvePointIndex, double directionVectorWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points)
    {
    return false;
    }
  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  if (numberOfPoints<2 || curvePointIndex < 0 || curvePointIndex >= numberOfPoints)
    {
    return false;
    }

  if (curvePointIndex == 0 || curvePointIndex == numberOfPoints - 1 || numberOfPoints < 3)
    {
    // Point is at the start or end of the line
    double pointPos[3] = { 0.0 };
    double pointPosAfter[3] = { 0.0 };
    points->GetPoint(curvePointIndex == 0 ? 0 : numberOfPoints - 2, pointPos);
    points->GetPoint(curvePointIndex == 0 ? 1 : numberOfPoints - 1, pointPosAfter);
    directionVectorWorld[0] = pointPosAfter[0] - pointPos[0];
    directionVectorWorld[1] = pointPosAfter[1] - pointPos[1];
    directionVectorWorld[2] = pointPosAfter[2] - pointPos[2];
    }
  else
    {
    // point is along the line, compute direction as the average of
    // direction before and after the line
    double pointPosBefore[3] = { 0.0 };
    double pointPos[3] = { 0.0 };
    double pointPosAfter[3] = { 0.0 };
    points->GetPoint(curvePointIndex - 1, pointPosBefore);
    points->GetPoint(curvePointIndex, pointPos);
    points->GetPoint(curvePointIndex + 1, pointPosAfter);
    double directionVectorBefore[3] = { pointPos[0] - pointPosBefore[0], pointPos[1] - pointPosBefore[1], pointPos[2] - pointPosBefore[2] };
    double directionVectorAfter[3] = { pointPosAfter[0] - pointPos[0], pointPosAfter[1] - pointPos[1], pointPosAfter[2] - pointPos[2] };
    vtkMath::Normalize(directionVectorBefore);
    vtkMath::Normalize(directionVectorAfter);
    directionVectorWorld[0] = directionVectorBefore[0] - directionVectorAfter[0];
    directionVectorWorld[1] = directionVectorBefore[1] - directionVectorAfter[1];
    directionVectorWorld[2] = directionVectorBefore[2] - directionVectorAfter[2];
    }
  vtkMath::Normalize(directionVectorWorld);
  return true;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetFarthestCurvePointIndexToPositionWorld(const double posWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints()<1)
    {
    return false;
    }

  double farthestPoint[3] = { 0.0 };
  points->GetPoint(0, farthestPoint);
  double farthestPointDistance2 = vtkMath::Distance2BetweenPoints(posWorld, farthestPoint);
  vtkIdType farthestPointId = 0;

  vtkIdType numberOfPoints = points->GetNumberOfPoints();
  for (vtkIdType pointIndex = 1; pointIndex < numberOfPoints; pointIndex++)
    {
    double* nextPoint = points->GetPoint(pointIndex);
    double nextPointDistance2 = vtkMath::Distance2BetweenPoints(posWorld, nextPoint);
    if (nextPointDistance2 > farthestPointDistance2)
      {
      farthestPoint[0] = nextPoint[0];
      farthestPoint[1] = nextPoint[1];
      farthestPoint[2] = nextPoint[2];
      farthestPointDistance2 = nextPointDistance2;
      farthestPointId = pointIndex;
      }
    }

  return farthestPointId;
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetCurvePointIndexAlongCurveWorld(vtkIdType startCurvePointId, double distanceFromStartPoint)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  double foundCurvePosition[3] = { 0.0 };
  vtkIdType foundClosestPointIndex = -1;
  vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(foundCurvePosition, foundClosestPointIndex,
    startCurvePointId, distanceFromStartPoint, points, this->CurveClosed);
  return foundClosestPointIndex;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetPositionAlongCurveWorld(double foundCurvePosition[3], vtkIdType startCurvePointId, double distanceFromStartPoint)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  vtkIdType foundClosestPointIndex = -1;
  return vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(foundCurvePosition, foundClosestPointIndex,
    startCurvePointId, distanceFromStartPoint, points, this->CurveClosed);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetPointsOnPlaneWorld(vtkPlane* plane, vtkPoints* intersectionPoints)
{
  if (!intersectionPoints)
    {
    return false;
    }
  intersectionPoints->Reset();
  if (!plane)
    {
    return false;
    }
  this->CurveGenerator->Update();
  vtkPolyData* curvePoly = this->GetCurveWorld();
  if (!curvePoly)
    {
    return true;
    }

  vtkNew<vtkCutter> cutEdges;
  cutEdges->SetInputData(curvePoly);
  cutEdges->SetCutFunction(plane);
  cutEdges->GenerateCutScalarsOff();
  cutEdges->SetValue(0, 0);
  cutEdges->Update();
  if (!cutEdges->GetOutput())
    {
    return true;
    }
  vtkPoints* points = cutEdges->GetOutput()->GetPoints();
  if (!points)
    {
    return true;
    }
  intersectionPoints->DeepCopy(points);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex(vtkIdType curvePointIndex, vtkMatrix4x4* curvePointToWorld)
{
  if (!curvePointToWorld)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointToWorld");
    return false;
    }
  this->CurveGenerator->Update();
  this->CurveCoordinateSystemGeneratorWorld->Update();
  vtkPolyData* curvePoly = this->CurveCoordinateSystemGeneratorWorld->GetOutput();
  if (!curvePoly)
    {
    return false;
    }
  vtkIdType n = curvePoly->GetNumberOfPoints();
  if (curvePointIndex < 0 || curvePointIndex >= n)
    {
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointIndex "
      << curvePointIndex << " (number of curve points: " << n << ")");
    return false;
    }
  curvePointToWorld->Identity();
  vtkPointData* pointData = curvePoly->GetPointData();
  if (!pointData)
    {
    return false;
    }
  vtkDoubleArray* normals = vtkDoubleArray::SafeDownCast(pointData->GetAbstractArray("FSNormals"));
  vtkDoubleArray* binormals = vtkDoubleArray::SafeDownCast(pointData->GetArray("FSBinormals"));
  vtkDoubleArray* tangents = vtkDoubleArray::SafeDownCast(pointData->GetArray("FSTangents"));
  if (!tangents || !normals || !binormals)
    {
    return false;
    }
  double* normal = normals->GetTuple3(curvePointIndex);
  double* binormal = binormals->GetTuple3(curvePointIndex);
  double* tangent = tangents->GetTuple3(curvePointIndex);
  double* position = curvePoly->GetPoint(curvePointIndex);
  for (int row=0; row<3; row++)
    {
    curvePointToWorld->SetElement(row, 0, normal[row]);
    curvePointToWorld->SetElement(row, 1, binormal[row]);
    curvePointToWorld->SetElement(row, 2, tangent[row]);
    curvePointToWorld->SetElement(row, 3, position[row]);
    }
  return true;
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetCurveType()
{
  return this->CurveGenerator->GetCurveType();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveType(int type)
{
  this->CurveGenerator->SetCurveType(type);
}

//-----------------------------------------------------------
const char* vtkMRMLMarkupsCurveNode::GetCurveTypeAsString(int id)
{
  return this->CurveGenerator->GetCurveTypeAsString(id);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetCurveTypeFromString(const char* name)
{
  return this->CurveGenerator->GetCurveTypeFromString(name);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToLinear()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToCardinalSpline()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToKochanekSpline()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToPolynomial()
{
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToShortestDistanceOnSurface(vtkMRMLModelNode* modelNode)
{
  MRMLNodeModifyBlocker blocker(this);
  this->SetCurveType(vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE);
  if (modelNode)
    {
    this->SetAndObserveShortestDistanceSurfaceNode(modelNode);
    }
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetNumberOfPointsPerInterpolatingSegment()
{
  return this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetNumberOfPointsPerInterpolatingSegment(int pointsPerSegment)
{
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(pointsPerSegment);
}

//---------------------------------------------------------------------------
vtkIdType vtkMRMLMarkupsCurveNode::GetClosestPointPositionAlongCurveWorld(const double posWorld[3], double closestPosWorld[3])
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints() < 1)
    {
    return -1;
    }
  if (points->GetNumberOfPoints() == 1)
    {
    points->GetPoint(0, closestPosWorld);
    return -1;
    }

  // Find closest curve point
  vtkIdType closestCurvePointIndex = this->GetClosestCurvePointIndexToPositionWorld(posWorld);
  if (closestCurvePointIndex < 0)
    {
    return -1;
    }
  double closestCurvePoint[3] = { 0.0 };
  points->GetPoint(closestCurvePointIndex, closestCurvePoint);
  double closestDistance2 = vtkMath::Distance2BetweenPoints(posWorld, closestPosWorld);
  closestPosWorld[0] = closestCurvePoint[0];
  closestPosWorld[1] = closestCurvePoint[1];
  closestPosWorld[2] = closestCurvePoint[2];
  vtkIdType lineIndex = closestCurvePointIndex;

  // See if we can find any points closer along the curve
  double relativePositionAlongLine = -1.0; // between 0.0-1.0 if between the endpoints of the line segment
  double otherPoint[3] = { 0.0 };
  double closestPointOnLine[3] = { 0.0 };
  if (closestCurvePointIndex - 1 >= 0)
    {
    points->GetPoint(closestCurvePointIndex - 1, otherPoint);
    double distance2 = vtkLine::DistanceToLine(posWorld, closestCurvePoint, otherPoint, relativePositionAlongLine, closestPointOnLine);
    if (distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      closestPosWorld[0] = closestPointOnLine[0];
      closestPosWorld[1] = closestPointOnLine[1];
      closestPosWorld[2] = closestPointOnLine[2];
      lineIndex = closestCurvePointIndex - 1;
      }
    }
  if (closestCurvePointIndex + 1 < points->GetNumberOfPoints())
    {
    points->GetPoint(closestCurvePointIndex + 1, otherPoint);
    double distance2 = vtkLine::DistanceToLine(posWorld, closestCurvePoint, otherPoint, relativePositionAlongLine, closestPointOnLine);
    if (distance2 < closestDistance2 && relativePositionAlongLine >= 0 && relativePositionAlongLine <= 1)
      {
      closestDistance2 = distance2;
      closestPosWorld[0] = closestPointOnLine[0];
      closestPosWorld[1] = closestPointOnLine[1];
      closestPosWorld[2] = closestPointOnLine[2];
      lineIndex = closestCurvePointIndex;
      }
    }
  return lineIndex;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::UpdateMeasurementsInternal()
{
  this->RemoveAllMeasurements();
  if (this->GetNumberOfDefinedControlPoints() > 1)
    {
    double length = this->GetCurveLengthWorld();
    std::string printFormat;
    std::string unit = "mm";
    vtkMRMLUnitNode* unitNode = GetUnitNode("length");
    if (unitNode)
      {
      if (unitNode->GetSuffix())
        {
        unit = unitNode->GetSuffix();
        }
      length = unitNode->GetDisplayValueFromValue(length);
      printFormat = unitNode->GetDisplayStringFormat();
      }
    this->SetNthMeasurement(0, "length", length, unit, printFormat);
    }
  this->WriteMeasurementsToDescription();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ProcessMRMLEvents(vtkObject* caller,
                                             unsigned long event,
                                             void* callData)
{
  if (event == vtkMRMLTransformableNode::TransformModifiedEvent)
    {
    this->OnSurfaceModelTransformChanged();
    }
  else if (caller == this->SurfaceScalarCalculator.GetPointer())
    {
    this->UpdateMeasurements();
    int n = -1;
    this->InvokeCustomModifiedEvent(vtkMRMLMarkupsNode::PointModifiedEvent, static_cast<void*>(&n));
    this->StorableModifiedTime.Modified();
    }
  else if (caller == this->CurveGenerator.GetPointer())
    {
    int surfaceCostFunctionType = this->CurveGenerator->GetSurfaceCostFunctionType();
    // Change the pass through filter input depending on if we need the scalar values.
    // Trying to run SurfaceScalarCalculator without an active scalar will result in an error message.
    if (surfaceCostFunctionType == vtkSlicerDijkstraGraphGeodesicPath::COST_FUNCTION_TYPE_DISTANCE)
      {
      this->PassThroughFilter->SetInputConnection(this->SurfaceToLocalTransformer->GetOutputPort());
      }
    else
      {
      this->PassThroughFilter->SetInputConnection(this->SurfaceScalarCalculator->GetOutputPort());
      }
    }

  if (caller == this->GetNodeReference(this->GetShortestDistanceSurfaceNodeReferenceRole()))
    {
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnNodeReferenceAdded(vtkMRMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetShortestDistanceSurfaceNodeReferenceRole()) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::OnNodeReferenceAdded(reference);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnNodeReferenceModified(vtkMRMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetShortestDistanceSurfaceNodeReferenceRole()) == 0)
    {
    this->OnSurfaceModelTransformChanged();
    this->OnSurfaceModelNodeChanged();
    }

  Superclass::OnNodeReferenceModified(reference);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnNodeReferenceRemoved(vtkMRMLNodeReference* reference)
{
  if (strcmp(reference->GetReferenceRole(), this->GetShortestDistanceSurfaceNodeReferenceRole()) == 0)
    {
    this->OnSurfaceModelNodeChanged();
    }
  Superclass::OnNodeReferenceRemoved(reference);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetAndObserveShortestDistanceSurfaceNode(vtkMRMLModelNode* modelNode)
{
  this->SetAndObserveNodeReferenceID(this->GetShortestDistanceSurfaceNodeReferenceRole(), modelNode ? modelNode->GetID() : nullptr);
}

//---------------------------------------------------------------------------
vtkMRMLModelNode* vtkMRMLMarkupsCurveNode::GetShortestDistanceSurfaceNode()
{
  return vtkMRMLModelNode::SafeDownCast(this->GetNodeReference(this->GetShortestDistanceSurfaceNodeReferenceRole()));
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetSurfaceCostFunctionType()
{
  return this->CurveGenerator->GetSurfaceCostFunctionType();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetSurfaceCostFunctionType(int surfaceCostFunctionType)
{
  this->CurveGenerator->SetSurfaceCostFunctionType(surfaceCostFunctionType);
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsCurveNode::GetSurfaceCostFunctionTypeAsString(int surfaceCostFunctionType)
{
  return vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeAsString(surfaceCostFunctionType);
}

//---------------------------------------------------------------------------
int vtkMRMLMarkupsCurveNode::GetSurfaceCostFunctionTypeFromString(const char* surfaceCostFunctionTypeName)
{
  return vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeFromString(surfaceCostFunctionTypeName);
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsCurveNode::GetSurfaceDistanceWeightingFunction()
{
  return this->SurfaceScalarCalculator->GetFunction();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetSurfaceDistanceWeightingFunction(const char* function)
{
  const char* currentFunction = this->SurfaceScalarCalculator->GetFunction();
  if ((currentFunction && function && strcmp(this->SurfaceScalarCalculator->GetFunction(), function) == 0) ||
    currentFunction == nullptr && function == nullptr)
    {
    return;
    }
  this->SurfaceScalarCalculator->SetFunction(function);
  this->UpdateSurfaceScalarVariables();
  this->UpdateMeasurements();
  this->Modified();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnSurfaceModelNodeChanged()
{
  this->UpdateSurfaceScalarVariables();

  vtkMRMLModelNode* modelNode = this->GetShortestDistanceSurfaceNode();
  if (modelNode)
    {
    this->CleanFilter->SetInputConnection(modelNode->GetPolyDataConnection());
    this->CurveGenerator->SetInputConnection(1, this->PassThroughFilter->GetOutputPort());
    }
  else
    {
    this->CleanFilter->RemoveAllInputs();
    this->CurveGenerator->RemoveInputConnection(1, this->PassThroughFilter->GetOutputPort());
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::OnSurfaceModelTransformChanged()
{
  vtkMRMLModelNode* modelNode = this->GetShortestDistanceSurfaceNode();
  if (!modelNode)
    {
    return;
    }

  vtkSmartPointer<vtkGeneralTransform> surfaceToLocalTransform = vtkGeneralTransform::SafeDownCast(
    this->SurfaceToLocalTransformer->GetTransform());
  if (!surfaceToLocalTransform)
    {
    surfaceToLocalTransform = vtkSmartPointer<vtkGeneralTransform>::New();
    this->SurfaceToLocalTransformer->SetTransform(surfaceToLocalTransform);
    }

  vtkMRMLTransformNode::GetTransformBetweenNodes(modelNode->GetParentTransformNode(), this->GetParentTransformNode(), surfaceToLocalTransform);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::UpdateSurfaceScalarVariables()
{
  vtkMRMLModelNode* modelNode = this->GetShortestDistanceSurfaceNode();
  if (!modelNode)
    {
    return;
    }

  vtkPolyData* polyData = modelNode->GetPolyData();
  if (!polyData)
    {
    return;
    }

  vtkPointData* pointData = polyData->GetPointData();
  if (!pointData)
    {
    return;
    }

  const char* activeScalarName = modelNode->GetActivePointScalarName(vtkDataSetAttributes::SCALARS);
  bool activeScalarChanged = false;
  if (!activeScalarName && this->ActiveScalar)
    {
    activeScalarChanged = true;
    }
  else if (activeScalarName && !this->ActiveScalar)
    {
    activeScalarChanged = true;
    }
  else if (activeScalarName && this->ActiveScalar && strcmp(activeScalarName, this->ActiveScalar) != 0)
    {
    activeScalarChanged = true;
    }
  this->ActiveScalar = activeScalarName;

  int numberOfArraysInMesh = pointData->GetNumberOfArrays();
  int numberOfArraysInCalculator = this->SurfaceScalarCalculator->GetNumberOfScalarArrays();
  if (!activeScalarChanged && numberOfArraysInMesh + 1 == numberOfArraysInCalculator)
    {
    return;
    }

  this->SurfaceScalarCalculator->RemoveAllVariables();
  for (int i = -1; i < numberOfArraysInMesh; ++i)
    {
    const char* variableName = "activeScalar";
    vtkDataArray* array = nullptr;
    if (i >= 0)
      {
      array = pointData->GetArray(i);
      variableName = array->GetName();
      }
    else
      {
      if (!activeScalarName)
        {
        continue;
        }
      array = pointData->GetArray(activeScalarName);
      }

    if (!array)
      {
      vtkWarningMacro("UpdateSurfaceScalarVariables: Could not get array " << i);
      continue;
      }

    this->SurfaceScalarCalculator->AddScalarVariable(variableName, array->GetName());
    }

  // Changing the variables doesn't invoke modified, so we need to invoke it here.
  this->SurfaceScalarCalculator->Modified();
  this->Modified();
}
