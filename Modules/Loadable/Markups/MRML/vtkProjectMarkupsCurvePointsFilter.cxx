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

#include "vtkProjectMarkupsCurvePointsFilter.h"

#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLTransformNode.h"

#include <vtkBoundingBox.h>
#include <vtkDoubleArray.h>
#include <vtkGenericCell.h>
#include <vtkGeneralTransform.h>
#include <vtkInformationVector.h>
#include <vtkMathUtilities.h>
#include <vtkOBBTree.h>
#include <vtkPolyDataNormals.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkTransformPolyDataFilter.h>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkProjectMarkupsCurvePointsFilter);

//------------------------------------------------------------------------------
vtkProjectMarkupsCurvePointsFilter::vtkProjectMarkupsCurvePointsFilter()
  : InputCurveNode()
  , MaximumSearchRadiusTolerance(0.25)
  , PointProjection()
{}

//------------------------------------------------------------------------------
void vtkProjectMarkupsCurvePointsFilter::SetInputCurveNode(vtkMRMLMarkupsCurveNode* inputCurveNode)
{
  if (this->InputCurveNode == inputCurveNode)
    {
    return;
    }
  this->InputCurveNode = inputCurveNode;
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkProjectMarkupsCurvePointsFilter::SetMaximumSearchRadiusTolerance(double maximumSearchRadiusTolerance)
{
  if (vtkMathUtilities::FuzzyCompare<double>(this->MaximumSearchRadiusTolerance, maximumSearchRadiusTolerance))
    {
    return;
    }
  this->MaximumSearchRadiusTolerance = maximumSearchRadiusTolerance;
  this->Modified();
}
//------------------------------------------------------------------------------
double vtkProjectMarkupsCurvePointsFilter::GetMaximumSearchRadiusTolerance() const
{
  return this->MaximumSearchRadiusTolerance;
}

//---------------------------------------------------------------------------
int vtkProjectMarkupsCurvePointsFilter::FillInputPortInformation(int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    }
  else
    {
    vtkErrorMacro("Cannot set input info for port " << port);
    return 0;
    }
  return 1;
}

//---------------------------------------------------------------------------
int vtkProjectMarkupsCurvePointsFilter::RequestData(
  vtkInformation* vtkNotUsed(request),
  vtkInformationVector** inputVector,
  vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkPolyData* inputPolyData = vtkPolyData::SafeDownCast(
    inInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!inputPolyData)
    {
    return 1;
    }

  vtkPoints* inputPoints = inputPolyData->GetPoints();
  if (!inputPoints)
    {
    return 1;
    }

  if (!this->InputCurveNode)
    {
    return 1;
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* outputPolyData = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));
  if (!outputPolyData)
    {
    return 1;
    }
  outputPolyData->DeepCopy(inputPolyData);
  vtkNew<vtkPoints> outputPoints;

  // If we have a surface node, project, otherwise the copy was enough
  if (this->InputCurveNode->GetSurfaceConstraintNode())
    {
    if(!this->ProjectPointsToSurface(
         this->InputCurveNode->GetSurfaceConstraintNode(), this->GetMaximumSearchRadiusTolerance(), inputPoints,
         outputPoints))
      {
      return 0;
      }
    outputPolyData->SetPoints(outputPoints);
    }

  outputPolyData->Squeeze();
  return 1;
}

//---------------------------------------------------------------------------
bool vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurfaceImpl(vtkOBBTree* surfaceObbTree, vtkPointLocator* pointLocator,
  vtkPoints* originalPoints, vtkDoubleArray* normalVectors, vtkPolyData* surfacePolydata,
  vtkPoints* surfacePoints, double maximumSearchRadiusTolerance)
{
  if (originalPoints->GetNumberOfPoints()!= normalVectors->GetNumberOfTuples())
    {
    vtkGenericWarningMacro("vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurface failed: invalid inputs");
    return false;
    }
  if (maximumSearchRadiusTolerance <= 0.0 || maximumSearchRadiusTolerance > 1.0)
    {
    vtkGenericWarningMacro("vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurface failed: Invalid search radius");
    return false;
    }

  double tolerance = surfaceObbTree->GetTolerance();

  double originalPoint[3] = { 0.0, 0.0, 0.0 };
  double rayDirection[3] = { 0.0, 0.0, 0.0 };
  double exteriorPoint[3] = { 0.0, 0.0, 0.0 };

  // Curves are expected to be close to surface. The maximumSearchRadiusTolerance
  // sets the allowable projection distance as a percentage of the model's
  // bounding box diagonal in world coordinate system.
  double polydataBounds[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
  surfacePolydata->GetBounds(polydataBounds);
  vtkBoundingBox modelBoundingBox;
  modelBoundingBox.AddBounds(polydataBounds);
  double polydataDiagonalLength = modelBoundingBox.GetDiagonalLength();
  double rayLength = maximumSearchRadiusTolerance*sqrt(polydataDiagonalLength);

  size_t noIntersectionCount = 0;
  for (vtkIdType controlPointIndex = 0; controlPointIndex < originalPoints->GetNumberOfPoints(); controlPointIndex++)
    {
    originalPoints->GetPoint(controlPointIndex, originalPoint);
    normalVectors->GetTuple(controlPointIndex, rayDirection);
    // Cast ray and find model intersection point
    double rayEndPoint[3] = { 0.0, 0.0, 0.0 };
    rayEndPoint[0] = originalPoint[0] + rayDirection[0] * rayLength;
    rayEndPoint[1] = originalPoint[1] + rayDirection[1] * rayLength;
    rayEndPoint[2] = originalPoint[2] + rayDirection[2] * rayLength;

    double t = 0.0;
    double pcoords[3] = { 0.0, 0.0, 0.0 };
    int subId = 0;
    vtkIdType cellId = 0;
    vtkNew <vtkGenericCell> cell;
    int foundIntersection = surfaceObbTree->IntersectWithLine(rayEndPoint, originalPoint, tolerance, t, exteriorPoint, pcoords, subId, cellId, cell);
    if(foundIntersection == 0)
      {
      // If no intersection, reverse direction of normal vector ray
      rayEndPoint[0] = originalPoint[0] + rayDirection[0] * -rayLength;
      rayEndPoint[1] = originalPoint[1] + rayDirection[1] * -rayLength;
      rayEndPoint[2] = originalPoint[2] + rayDirection[2] * -rayLength;
      int foundIntersection = surfaceObbTree->IntersectWithLine(originalPoint, rayEndPoint, tolerance, t, exteriorPoint, pcoords, subId, cellId, cell);
      if(foundIntersection == 0)
        {
        // If no intersection in either direction, use closest mesh point
        vtkIdType closestPointId = pointLocator->FindClosestPoint(originalPoint);
        surfacePolydata->GetPoint(closestPointId, exteriorPoint);
        ++noIntersectionCount;
        }
      }
    surfacePoints->InsertNextPoint(exteriorPoint);
    }
  if (noIntersectionCount > 0)
    {
    vtkGenericWarningMacro("No intersections found for " << noIntersectionCount << " points for curve ");
    }
  return true;
}

//---------------------------------------------------------------------------
bool vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurface(vtkPoints* originalPoints, vtkDoubleArray* normalVectors, vtkPolyData* surfacePolydata,
  vtkPoints* surfacePoints, double maximumSearchRadiusTolerance)
{
  vtkNew<vtkOBBTree> surfaceObbTree;
  surfaceObbTree->SetDataSet(surfacePolydata);
  surfaceObbTree->BuildLocator();

  vtkNew<vtkPointLocator> pointLocator;
  pointLocator->SetDataSet(surfacePolydata);
  pointLocator->BuildLocator();

  return vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurfaceImpl(surfaceObbTree, pointLocator,
    originalPoints, normalVectors, surfacePolydata,
    surfacePoints, maximumSearchRadiusTolerance);
}

//---------------------------------------------------------------------------
bool vtkProjectMarkupsCurvePointsFilter::ProjectPointsToSurface(
  vtkMRMLModelNode* modelNode,
  double maximumSearchRadiusTolerance,
  vtkPoints* pointsToProject,
  vtkPoints* outputPoints)
{
  this->PointProjection.SetModel(modelNode);
  vtkSmartPointer<vtkPolyData> surfacePolydata = this->PointProjection.GetSurfacePolyData();
  if(!surfacePolydata)
    {
    vtkErrorMacro("vtkProjectMarkupsCurvePointsFilter::ProjectPointsToSurface failed: Constraint surface polydata is not valid");
    return false;
    }

  vtkNew<vtkPoints> controlPoints;
  this->InputCurveNode->GetControlPointPositionsWorld(controlPoints);

  auto pointNormalArray = this->PointProjection.GetPointNormals(pointsToProject, controlPoints);
  if (!pointNormalArray)
    {
    return false;
    }

  return vtkProjectMarkupsCurvePointsFilter::ConstrainPointsToSurfaceImpl(this->PointProjection.GetObbTree(), this->PointProjection.GetPointLocator(),
    pointsToProject, pointNormalArray, surfacePolydata,
    outputPoints, maximumSearchRadiusTolerance);
}

//---------------------------------------------------------------------------
vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::PointProjectionHelper()
  : Model(nullptr)
  , LastModelModifiedTime(0)
  , LastTransformModifiedTime(0)
  , ModelNormalVectorArray()
  , ModelPointLocator()
  , ModelObbTree()
  , SurfacePolyData()
{}

//---------------------------------------------------------------------------
void vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::SetModel(vtkMRMLModelNode* model)
{
  this->Model = model;
}

//---------------------------------------------------------------------------
vtkPointLocator* vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::GetPointLocator()
{
  this->UpdateAll();
  return this->ModelPointLocator;
}

//---------------------------------------------------------------------------
vtkPolyData* vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::GetSurfacePolyData()
{
  this->UpdateAll();
  return this->SurfacePolyData;
}

//---------------------------------------------------------------------------
vtkOBBTree* vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::GetObbTree()
{
  this->UpdateAll();
  return this->ModelObbTree;
}

//---------------------------------------------------------------------------
bool vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::UpdateAll()
{
  if (!this->Model)
    {
    this->ModelPointLocator = vtkSmartPointer<vtkPointLocator>();
    this->ModelNormalVectorArray = vtkSmartPointer<vtkDataArray>();
    this->ModelObbTree = vtkSmartPointer<vtkOBBTree>();
    this->SurfacePolyData = vtkSmartPointer<vtkPolyData>();
    return false;
    }

  // by using != instead of say, <, this will catch both if the model is updated
  // and if a different model was set
  vtkMRMLTransformNode* parentTransformNode = this->Model->GetParentTransformNode();
  if (this->Model->GetMTime() != this->LastModelModifiedTime
    || (parentTransformNode && parentTransformNode->GetTransformToWorldMTime() != this->LastTransformModifiedTime)
    || !this->ModelNormalVectorArray)
    {
    this->LastModelModifiedTime = this->Model->GetMTime();
    this->SurfacePolyData = this->Model->GetPolyData();
    if (parentTransformNode)
      {
      this->LastTransformModifiedTime = parentTransformNode->GetTransformToWorldMTime();
      vtkNew<vtkGeneralTransform> modelToWorldTransform;
      parentTransformNode->GetTransformToWorld(modelToWorldTransform);
      vtkNew<vtkTransformPolyDataFilter> transformPolydataFilter;
      transformPolydataFilter->SetInputData(this->SurfacePolyData);
      transformPolydataFilter->SetTransform(modelToWorldTransform);
      transformPolydataFilter->Update();
      this->SurfacePolyData = transformPolydataFilter->GetOutput();
      }

    this->ModelPointLocator = vtkSmartPointer<vtkPointLocator>::New();
    this->ModelPointLocator->SetDataSet(this->SurfacePolyData);
    this->ModelPointLocator->BuildLocator();

    this->ModelObbTree = vtkSmartPointer<vtkOBBTree>::New();
    this->ModelObbTree->SetDataSet(this->SurfacePolyData);
    this->ModelObbTree->BuildLocator();

    vtkNew<vtkPolyDataNormals> normalFilter;
    normalFilter->SetInputData(this->SurfacePolyData);
    normalFilter->ComputePointNormalsOn();
    normalFilter->Update();
    vtkPolyData* normalPolydata = normalFilter->GetOutput();
    this->ModelNormalVectorArray = vtkArrayDownCast<vtkDataArray>(normalPolydata->GetPointData()->GetNormals());
    if (!this->ModelNormalVectorArray)
      {
      vtkGenericWarningMacro("vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::GetPointNormals failed: Unable to calculate normals");
      this->ModelPointLocator = vtkSmartPointer<vtkPointLocator>();
      this->ModelNormalVectorArray = vtkSmartPointer<vtkDataArray>();
      this->ModelObbTree = vtkSmartPointer<vtkOBBTree>();
      this->SurfacePolyData = vtkSmartPointer<vtkPolyData>();
      return false;
      }
    }
  return true;
}

//---------------------------------------------------------------------------
vtkIdType vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::GetClosestControlPointIndex(const double point[3], vtkPoints* controlPoints)
{
  const auto numberOfControlPoints = controlPoints->GetNumberOfPoints();
  if (numberOfControlPoints == 0)
    {
    return -1;
    }
  vtkIdType closestIndex = 0;
  double closestDistanceSquare = vtkMath::Distance2BetweenPoints(point, controlPoints->GetPoint(0));
  for (vtkIdType i = 1; i < numberOfControlPoints; ++i)
    {
    const double distSquare = vtkMath::Distance2BetweenPoints(point, controlPoints->GetPoint(i));
    if (distSquare < closestDistanceSquare)
      {
      closestDistanceSquare = distSquare;
      closestIndex = i;
      }
    }
  return closestIndex;
}

//---------------------------------------------------------------------------
vtkSmartPointer<vtkDoubleArray> vtkProjectMarkupsCurvePointsFilter::PointProjectionHelper::GetPointNormals(
  vtkPoints* points,
  vtkPoints* controlPoints)
{
  if (!this->UpdateAll())
    {
    return vtkSmartPointer<vtkDoubleArray>();
    }

  auto normals = vtkSmartPointer<vtkDoubleArray>::New();
  normals->SetNumberOfComponents(3);
  const auto numberOfPoints = points->GetNumberOfPoints();
  for (vtkIdType i = 0; i < numberOfPoints; ++i)
    {
    const double* point = points->GetPoint(i);
    const auto segmentStartIndex = GetClosestControlPointIndex(point, controlPoints);
    double segmentStartPoint[3] = { 0.0, 0.0, 0.0 };
    controlPoints->GetPoint(segmentStartIndex, segmentStartPoint);
    const auto segmentEndIndex = [&]() -> vtkIdType {
      if (segmentStartIndex == 0)
        {
        return 1;
        }
      else if (segmentStartIndex == controlPoints->GetNumberOfPoints() - 1)
        {
        return segmentStartIndex - 1;
        }
      else
        {
        double segmentEndPoint1[3] = { 0.0, 0.0, 0.0 };
        controlPoints->GetPoint(segmentStartIndex - 1, segmentEndPoint1);
        double dist1 = vtkMath::Distance2BetweenPoints(segmentEndPoint1, point);
        double segmentEndPoint2[3];
        controlPoints->GetPoint(segmentStartIndex + 1, segmentEndPoint2);
        double dist2 = vtkMath::Distance2BetweenPoints(segmentEndPoint2, point);

        if ((dist1 < dist2) && dist1 < vtkMath::Distance2BetweenPoints(segmentEndPoint1, segmentStartPoint))
          {
          return segmentStartIndex - 1;
          }
        else
          {
          return segmentStartIndex + 1;
          }
        }
      }();
    double segmentEndPoint[3] = { 0.0, 0.0, 0.0 };
    controlPoints->GetPoint(segmentEndIndex, segmentEndPoint);

    const auto distance2ToStart = vtkMath::Distance2BetweenPoints(point, segmentStartPoint);
    const auto distance2ToEnd = vtkMath::Distance2BetweenPoints(point, segmentEndPoint);

    vtkIdType pointIdStart = this->ModelPointLocator->FindClosestPoint(segmentStartPoint);
    double startNormal[3] = { 0.0, 0.0, 0.0 };
    this->ModelNormalVectorArray->GetTuple(pointIdStart, startNormal);
    vtkIdType pointIdEnd = this->ModelPointLocator->FindClosestPoint(segmentEndPoint);
    double endNormal[3] = { 0.0, 0.0, 0.0 };
    this->ModelNormalVectorArray->GetTuple(pointIdEnd, endNormal);

    const double startWeight = distance2ToEnd / (distance2ToStart + distance2ToEnd);
    const double endWeight = distance2ToStart / (distance2ToStart + distance2ToEnd);
    double rayDirection[3] =
      {
      (startWeight * startNormal[0]) + (endWeight * endNormal[0]),
      (startWeight * startNormal[1]) + (endWeight * endNormal[1]),
      (startWeight * startNormal[2]) + (endWeight * endNormal[2])
      };
    vtkMath::Normalize(rayDirection);
    normals->InsertNextTuple(rayDirection);
    }

    return normals;
}
