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
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkCutter.h>
#include <vtkDoubleArray.h>
#include <vtkFrenetSerretFrame.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>

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
  this->CurveGenerator->SetCurveTypeToCardinalSpline();
  this->CurveGenerator->SetNumberOfPointsPerInterpolatingSegment(10);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsCurveNode::~vtkMRMLMarkupsCurveNode()
= default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
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
double vtkMRMLMarkupsCurveNode::GetCurveLengthWorld(vtkIdType startCurvePointIndex /*=0*/, vtkIdType numberOfCurvePoints /*=-1*/)
{
  vtkPoints* points = this->GetCurvePointsWorld();
  if (!points || points->GetNumberOfPoints() < 2)
    {
    return 0.0;
    }
  if (startCurvePointIndex < 0)
    {
    vtkWarningMacro("Invalid startCurvePointIndex=" << startCurvePointIndex << ", using 0 instead");
    startCurvePointIndex = 0;
    }
  vtkIdType lastCurvePointIndex = points->GetNumberOfPoints()-1;
  if (numberOfCurvePoints >= 0 && startCurvePointIndex + numberOfCurvePoints - 1 < lastCurvePointIndex)
    {
    lastCurvePointIndex = startCurvePointIndex + numberOfCurvePoints - 1;
    }

  double length = 0.0;
  double previousPoint[3] = { 0.0 };
  points->GetPoint(startCurvePointIndex, previousPoint);
  for (vtkIdType curvePointIndex = startCurvePointIndex + 1; curvePointIndex <= lastCurvePointIndex; curvePointIndex++)
    {
    double nextPoint[3];
    points->GetPoint(curvePointIndex, nextPoint);
    length += sqrt(vtkMath::Distance2BetweenPoints(previousPoint, nextPoint));
    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }
  return length;
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
  originalPoints->DeepCopy(points);
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
  double previousCurvePoint[3] = { 0.0 };
  originalPoints->GetPoint(0, previousCurvePoint);
  sampledPoints->Reset();
  sampledPoints->InsertNextPoint(previousCurvePoint);
  vtkIdType numberOfOriginalPoints = originalPoints->GetNumberOfPoints();
  for (vtkIdType originalPointIndex = 0; originalPointIndex < numberOfOriginalPoints; originalPointIndex++)
    {
    double* currentCurvePoint = originalPoints->GetPoint(originalPointIndex);
    double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(currentCurvePoint, previousCurvePoint));
    if (segmentLength <= 0.0)
      {
      continue;
      }
    double remainingSegmentLength = distanceFromLastSampledPoint + segmentLength;
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

  // The last segment may be much shorter than all the others, which may introduce artifact in spline fitting.
  // To fix that, move the last point to have two equal segments at the end.
  if (closedCurve && sampledPoints->GetNumberOfPoints() > 3)
    {
    double firstPoint[3] = { 0.0 };
    double secondLastPoint[3] = { 0.0 };
    double lastPoint[3] = { 0.0 };
    sampledPoints->GetPoint(0, firstPoint);
    sampledPoints->GetPoint(sampledPoints->GetNumberOfPoints()-2, secondLastPoint);
    sampledPoints->GetPoint(sampledPoints->GetNumberOfPoints() - 1, lastPoint);
    double lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(secondLastPoint, lastPoint));
    double lastTwoSegmentLengthAverage = (lastSegmentLength + sqrt(vtkMath::Distance2BetweenPoints(lastPoint, firstPoint)))/2.0;
    double lastTwoSegmentRatio = lastTwoSegmentLengthAverage / lastSegmentLength;
    double adjustedLastPoint[3] =
      {
      secondLastPoint[0] + (lastPoint[0] - secondLastPoint[0]) * lastTwoSegmentRatio,
      secondLastPoint[1] + (lastPoint[1] - secondLastPoint[1]) * lastTwoSegmentRatio,
      secondLastPoint[2] + (lastPoint[2] - secondLastPoint[2]) * lastTwoSegmentRatio
      };
    sampledPoints->SetPoint(sampledPoints->GetNumberOfPoints() - 1, adjustedLastPoint);
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
vtkIdType vtkMRMLMarkupsCurveNode::GetClosestCurvePointIndexToPositionWorld(double posWorld[3])
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
    return controlPointIndex * this->CurveGenerator->GetNumberOfPointsPerInterpolatingSegment() + 1;
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
vtkIdType vtkMRMLMarkupsCurveNode::GetFarthestCurvePointIndexToPositionWorld(double posWorld[3])
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
  if (!points)
    {
    return -1;
    }
  vtkIdType n = points->GetNumberOfPoints();
  if (startCurvePointId < 0 || startCurvePointId >= n)
    {
    return -1;
    }

  vtkIdType idIncrement = (distanceFromStartPoint >= 0 ? 1 : -1);
  double remainingDistanceFromStartPoint = abs(distanceFromStartPoint);
  double previousPoint[3] = { 0.0 };
  points->GetPoint(startCurvePointId, previousPoint);
  vtkIdType pointId = startCurvePointId;
  bool curveConfirmedToBeNonZeroLength = false;
  double lastSegmentLength = 0;
  while (remainingDistanceFromStartPoint>0)
    {
    pointId += idIncrement;

    // if reach the end then wrap around for closed curve, terminate search for open curve
    if (pointId < 0 || pointId >= n)
      {
      if (this->CurveClosed)
        {
        if (!curveConfirmedToBeNonZeroLength)
          {
          if (this->GetCurveLengthWorld() == 0.0)
            {
            return -1;
            }
          curveConfirmedToBeNonZeroLength = true;
          }
        pointId = (pointId < 0 ? n : -1);
        continue;
        }
      else
        {
        // reached end of curve before getting at the requested distance
        // return closest
        return (pointId < 0 ? 0 : n - 1);
        }
      }

    // determine how much closer we are now
    double* nextPoint = points->GetPoint(pointId);
    lastSegmentLength = sqrt(vtkMath::Distance2BetweenPoints(nextPoint, previousPoint));
    remainingDistanceFromStartPoint -= lastSegmentLength;
    previousPoint[0] = nextPoint[0];
    previousPoint[1] = nextPoint[1];
    previousPoint[2] = nextPoint[2];
    }

  if (fabs(remainingDistanceFromStartPoint) <= fabs(remainingDistanceFromStartPoint + lastSegmentLength))
    {
    return pointId;
    }
  else
    {
    return pointId-1;
    }
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
    vtkErrorMacro("vtkMRMLMarkupsCurveNode::GetCurvePointToWorldTransformAtPointIndex failed: Invalid curvePointIndex");
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
