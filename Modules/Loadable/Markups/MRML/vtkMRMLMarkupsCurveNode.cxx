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
#include <vtkLine.h>
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

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(curveType, CurveType);
  vtkMRMLWriteXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(curveType, CurveType);
  vtkMRMLReadXMLIntMacro(numberOfPointsPerInterpolatingSegment, NumberOfPointsPerInterpolatingSegment);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(CurveType);
  vtkMRMLCopyIntMacro(NumberOfPointsPerInterpolatingSegment);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(CurveType);
  vtkMRMLPrintIntMacro(NumberOfPointsPerInterpolatingSegment);
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
bool vtkMRMLMarkupsCurveNode::GetPositionAndClosestPointIndexAlongCurve(double foundCurvePosition[3], vtkIdType foundClosestPointIndex,
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
        curvePoints->GetPoint(startCurvePointId, foundCurvePosition);
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
  this->CurveGenerator->SetCurveTypeToLinearSpline();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToCardinalSpline()
{
  this->CurveGenerator->SetCurveTypeToCardinalSpline();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToKochanekSpline()
{
  this->CurveGenerator->SetCurveTypeToKochanekSpline();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsCurveNode::SetCurveTypeToPolynomial()
{
  this->CurveGenerator->SetCurveTypeToPolynomial();
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
  return true;
}
