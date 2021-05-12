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

#include "vtkMRMLMarkupsClosedCurveNode.h"

// MRML includes
#include "vtkCurveGenerator.h"
#include "vtkCurveMeasurementsCalculator.h"
#include "vtkMRMLMeasurementArea.h"
#include "vtkMRMLMessageCollection.h"

// vtkAddon includes
#include "vtkAddonMathUtilities.h"

// VTK includes
#include <vtkCleanPolyData.h>
#include <vtkDelaunay2D.h>
#include <vtkDiskSource.h>
#include <vtkMassProperties.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPlane.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkStringArray.h>
#include <vtkThinPlateSplineTransform.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsClosedCurveNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode::vtkMRMLMarkupsClosedCurveNode()
{
  this->CurveClosed = true;
  this->CurveGenerator->SetCurveIsClosed(true);
  this->CurveMeasurementsCalculator->SetCurveIsClosed(true);

  // Setup measurements calculated for this markup type
  vtkNew<vtkMRMLMeasurementArea> areaMeasurement;
  areaMeasurement->SetEnabled(false);
  areaMeasurement->SetName("area");
  areaMeasurement->SetInputMRMLNode(this);
  this->Measurements->AddItem(areaMeasurement);
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsClosedCurveNode::~vtkMRMLMarkupsClosedCurveNode() = default;

//---------------------------------------------------------------------------
double vtkMRMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(vtkMRMLMarkupsClosedCurveNode* curveNode,
  vtkPolyData* inputSurface /*=nullptr*/, bool projectWarp /*=true*/)
{
  vtkSmartPointer<vtkPolyData> surface;
  if (inputSurface)
    {
    inputSurface->Reset();
    surface = inputSurface;
    }
  else
    {
    surface = vtkSmartPointer<vtkPolyData>::New();
    }
  vtkPoints* curvePointsWorld = curveNode->GetCurvePointsWorld();
  if (curvePointsWorld == nullptr || curvePointsWorld->GetNumberOfPoints() == 0)
    {
    return 0.0;
    }
  bool success = false;
  if (projectWarp)
    {
    success = vtkMRMLMarkupsClosedCurveNode::FitSurfaceProjectWarp(curvePointsWorld, surface);
    }
  else
    {
    success = vtkMRMLMarkupsClosedCurveNode::FitSurfaceDiskWarp(curvePointsWorld, surface);
    }
  if (!success)
    {
    return 0.0;
    }

  vtkNew<vtkMassProperties> metrics;
  metrics->SetInputData(surface);
  double surfaceArea = metrics->GetSurfaceArea();

  return surfaceArea;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsClosedCurveNode::FitSurfaceProjectWarp(vtkPoints* curvePoints,
  vtkPolyData* surface, double vtkNotUsed(radiusScalingFactor)/*=1.0*/, vtkIdType numberOfInternalGridPoints/*=225*/)
{
  if (!surface)
    {
    vtkGenericWarningMacro("FitSurfaceProjectWarp failed: invalid surface");
    return false;
    }

  if (!curvePoints)
    {
    vtkGenericWarningMacro("FitSurfaceProjectWarp failed: invalid curvePoints");
    surface->Initialize();
    return false;
    }

  // The triangulator needs a polygon, where the first and last points are different.
  // However, in the curve points, the first and last points are the same, therefore we remove the last point
  // by setting number of points to n-1.
  vtkIdType numberOfCurvePoints = curvePoints->GetNumberOfPoints()-1;
  if (numberOfCurvePoints < 3)
    {
    // less than 3 points means that the surface is empty
    surface->Initialize();
    return true;
    }

  // Create a polydata containing a single polygon of the curve points
  vtkNew<vtkPolyData> inputSurface;
  inputSurface->SetPoints(curvePoints);
  vtkNew<vtkCellArray> polys;
  polys->InsertNextCell(numberOfCurvePoints);
  for (int i = 0; i < numberOfCurvePoints; i++)
    {
    polys->InsertCellPoint(i);
    }
  polys->Modified();
  inputSurface->SetPolys(polys);

  // Remove duplicate points (it would confuse the triangulator)
  vtkNew<vtkCleanPolyData> cleaner;
  cleaner->SetInputData(inputSurface);
  cleaner->Update();
  inputSurface->DeepCopy(cleaner->GetOutput());
  vtkNew<vtkPoints> cleanedCurvePoints;
  cleanedCurvePoints->DeepCopy(inputSurface->GetPoints());
  numberOfCurvePoints = cleanedCurvePoints->GetNumberOfPoints();

  // The triangulator requires all points to be on the XY plane
  vtkNew<vtkMatrix4x4> transformToBestFitPlaneMatrix;
  if (!vtkAddonMathUtilities::FitPlaneToPoints(inputSurface->GetPoints(), transformToBestFitPlaneMatrix))
    {
    surface->Initialize();
    return false;
    }
  vtkNew<vtkTransform> transformToXYPlane;
  transformToXYPlane->SetMatrix(transformToBestFitPlaneMatrix); // set XY plane -> best-fit plane
  transformToXYPlane->Inverse(); // // change the transform to: set best-fit plane -> XY plane
  vtkNew<vtkPoints> pointsOnPlane;
  transformToXYPlane->TransformPoints(cleanedCurvePoints, pointsOnPlane);
  inputSurface->SetPoints(pointsOnPlane);
  for (vtkIdType i = 0; i < numberOfCurvePoints; i++)
    {
    double* pt = pointsOnPlane->GetPoint(i);
    pointsOnPlane->SetPoint(i, pt[0], pt[1], 0.0);
    }

  // Ensure points are in counter-clockwise direction
  // (that indicates to Delaunay2D that it is a polygon to be
  // filled in and not a hole).
  vtkNew<vtkIdList> cleanedCurvePointIds;
  polys->GetCell(0, cleanedCurvePointIds);
  if (vtkMRMLMarkupsClosedCurveNode::IsPolygonClockwise(pointsOnPlane, cleanedCurvePointIds))
    {
    vtkIdType numberOfCleanedCurvePointIds = cleanedCurvePointIds->GetNumberOfIds();
    vtkNew<vtkCellArray> reversePolys;
    reversePolys->InsertNextCell(numberOfCleanedCurvePointIds);
    for (vtkIdType i = 0; i < numberOfCleanedCurvePointIds; i++)
      {
      reversePolys->InsertCellPoint(cleanedCurvePointIds->GetId(numberOfCleanedCurvePointIds-1-i));
      }
    reversePolys->Modified();
    inputSurface->SetPolys(reversePolys);
    }

  // Add set of internal points to improve triangulation quality.
  // We already have many points on the boundary but no points inside the polygon.
  // If we passed these points to the triangulator then opposite points on the curve
  // would be connected by triangles, so we would end up with many very skinny triangles,
  // and not smooth surface after warping.
  // By adding random points, the triangulator can create evenly sized triangles.
  double bounds[6] = { 0.0 };
  pointsOnPlane->GetBounds(bounds);
  double width = bounds[1] - bounds[0];
  double height = bounds[3] - bounds[2];
  // Compute the number of grid points along the two axes from these:
  // 1.  rows * cols = numberOfInternalGridPoints
  // 2.  rows/cols = height/width
  vtkIdType rows = 1;
  if (height > width / numberOfInternalGridPoints)
    {
    rows = static_cast<vtkIdType>(sqrt(numberOfInternalGridPoints * height / width));
    if (rows>numberOfInternalGridPoints)
      {
      rows = numberOfInternalGridPoints;
      }
    }
  vtkIdType cols = numberOfInternalGridPoints / rows;
  if (cols < 1)
    {
    cols = 1;
    }
  double colSpacing = width / cols;
  double rowSpacing = height / rows;
  double colStart = bounds[0] + 0.5 * colSpacing;
  double rowStart = bounds[2] + 0.5 * rowSpacing;
  for (vtkIdType row = 0; row < rows; row++)
    {
    for (vtkIdType col = 0; col < cols; col++)
      {
      pointsOnPlane->InsertNextPoint(colStart + colSpacing * col, rowStart + rowSpacing * row, 0.0);
      }
    }

  vtkNew<vtkDelaunay2D> triangulator;
  triangulator->SetInputData(inputSurface);
  triangulator->SetSourceData(inputSurface);

  vtkNew<vtkMRMLMessageCollection> messages;
  messages->SetObservedObject(triangulator);
  triangulator->Update();

  bool errorFound = false;
  bool warningFound = false;
  std::string messageStr = messages->GetAllMessagesAsString(&errorFound, &warningFound);
  if (errorFound || warningFound)
    {
    vtkGenericWarningMacro("FitSurfaceProjectWarp failed: error triangulating the surface area of the closed curve. Details: " << messageStr);
    surface->Initialize();
    return false;
    }

  vtkPolyData* triangulatedSurface = triangulator->GetOutput();
  vtkPoints* triangulatedSurfacePoints = triangulatedSurface->GetPoints();

  vtkNew<vtkPoints> sourceLandmarkPoints; // points on the triangulated surface
  vtkNew<vtkPoints> targetLandmarkPoints; // points on the curve
  // Use only the transformed curve points (first numberOfCurvePoints points)
  // We use only every 3rd boundary point as warping transform control point for simpler and faster warping.
  int step = 3;
  vtkIdType numberOfRegistrationLandmarkPoints = numberOfCurvePoints / step;
  sourceLandmarkPoints->SetNumberOfPoints(numberOfRegistrationLandmarkPoints);
  targetLandmarkPoints->SetNumberOfPoints(numberOfRegistrationLandmarkPoints);
  for (vtkIdType landmarkPointIndex = 0; landmarkPointIndex < numberOfRegistrationLandmarkPoints; landmarkPointIndex++)
    {
    sourceLandmarkPoints->SetPoint(landmarkPointIndex, triangulatedSurfacePoints->GetPoint(landmarkPointIndex*step));
    targetLandmarkPoints->SetPoint(landmarkPointIndex, cleanedCurvePoints->GetPoint(landmarkPointIndex*step));
    }

  vtkNew<vtkThinPlateSplineTransform> landmarkTransform;
  // Disable regularization to make sure transformation is correct even if source or target points are coplanar
  landmarkTransform->SetRegularizeBulkTransform(false);
  landmarkTransform->SetBasisToR();
  landmarkTransform->SetSourceLandmarks(sourceLandmarkPoints);
  landmarkTransform->SetTargetLandmarks(targetLandmarkPoints);

  vtkNew<vtkTransformPolyDataFilter> polyTransformToCurve;
  polyTransformToCurve->SetTransform(landmarkTransform);
  polyTransformToCurve->SetInputData(triangulatedSurface);

  vtkNew<vtkPolyDataNormals> polyDataNormals;
  polyDataNormals->SetInputConnection(polyTransformToCurve->GetOutputPort());
  polyDataNormals->SplittingOff();
  polyDataNormals->Update();

  surface->DeepCopy(polyDataNormals->GetOutput());
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsClosedCurveNode::IsPolygonClockwise(vtkPoints* points, vtkIdList* pointIds/*=nullptr*/)
{
  if (!points)
    {
    return false;
    }
  vtkIdType numberOfPoints = (pointIds!=nullptr ? pointIds->GetNumberOfIds() : points->GetNumberOfPoints());
  if (numberOfPoints < 3)
    {
    return false;
    }

  // Find the bottom-left point (it is on the convex hull) of the polygon,
  // and check sign of cross-product of the edges before and after that point.
  // (https://en.wikipedia.org/wiki/Curve_orientation#Orientation_of_a_simple_polygon)

  double* point0 = points->GetPoint(0);
  double minX = point0[0];
  double minY = point0[1];
  vtkIdType cornerPointIndex = 0;
  for (vtkIdType i = 1; i < numberOfPoints; i++)
    {
    vtkIdType pointId = (pointIds != nullptr ? pointIds->GetId(i) : i);
    double* p = points->GetPoint(pointId);
    if ((p[1] < minY) || ((p[1] == minY) && (p[0] < minX)))
      {
      cornerPointIndex = i;
      minX = p[0];
      minY = p[1];
      }
    }

  double p1[3];
  double p2[3];
  double p3[3];
  if (pointIds != nullptr)
    {
    points->GetPoint(pointIds->GetId((cornerPointIndex - 1 + numberOfPoints) % numberOfPoints), p1);
    points->GetPoint(pointIds->GetId(cornerPointIndex), p2);
    points->GetPoint(pointIds->GetId((cornerPointIndex + 1) % numberOfPoints), p3);
    }
  else
    {
    points->GetPoint((cornerPointIndex - 1 + numberOfPoints) % numberOfPoints, p1);
    points->GetPoint(cornerPointIndex, p2);
    points->GetPoint((cornerPointIndex + 1) % numberOfPoints, p3);
    }
  double det = p2[0] * p3[1] - p2[1] * p3[0] - p1[0] * p3[1] + p1[0] * p2[1] + p1[1] * p3[0] - p1[1] * p2[0];
  return (det < 0);
}


//---------------------------------------------------------------------------
bool vtkMRMLMarkupsClosedCurveNode::FitSurfaceDiskWarp(vtkPoints* curvePoints, vtkPolyData* surface, double radiusScalingFactor/*=1.0*/)
{
  if (!curvePoints || !surface)
    {
    return false;
    }

  // Transform a unit disk to the curve circumference using thin-plate spline interpolation.
  // It does not guarantee minimum area surface but at least it is a smooth surface that tightly
  // fits at the provided contour at the boundary.
  // A further refinement step could be added during that the surface points are adjusted so that
  // the surface area is minimized.

  // We have a landmark point at every 4.5 degrees (360/80) around the perimeter of the curve.
  // This is accurate enough for reproducing even very complex curves and can be still computed
  // quite quickly.
  const vtkIdType numberOfLandmarkPoints = 80;
  const vtkIdType numberOfCurvePoints = curvePoints->GetNumberOfPoints();

  vtkNew<vtkPoints> sourceLandmarkPoints; // points on the unit disk
  sourceLandmarkPoints->SetNumberOfPoints(numberOfLandmarkPoints); // points on the curve
  vtkNew<vtkPoints> targetLandmarkPoints; // curve points
  targetLandmarkPoints->SetNumberOfPoints(numberOfLandmarkPoints);
  for (vtkIdType landmarkPointIndex = 0; landmarkPointIndex < numberOfLandmarkPoints; ++landmarkPointIndex)
    {
    double angle = double(landmarkPointIndex) / double(numberOfLandmarkPoints) * 2.0 * vtkMath::Pi();
    vtkIdType curvePointIndex = vtkMath::Round(round(double(landmarkPointIndex) / double(numberOfLandmarkPoints) * numberOfCurvePoints));
    sourceLandmarkPoints->SetPoint(landmarkPointIndex, cos(angle), sin(angle), 0);
    targetLandmarkPoints->SetPoint(landmarkPointIndex, curvePoints->GetPoint(curvePointIndex));
    }

  vtkNew<vtkThinPlateSplineTransform> landmarkTransform;
  // Disable regularization to make sure transformation is correct even if source or target points are coplanar
  landmarkTransform->SetRegularizeBulkTransform(false);
  landmarkTransform->SetBasisToR();
  landmarkTransform->SetSourceLandmarks(sourceLandmarkPoints);
  landmarkTransform->SetTargetLandmarks(targetLandmarkPoints);

  vtkNew<vtkDiskSource> unitDisk;
  unitDisk->SetOuterRadius(radiusScalingFactor);
  unitDisk->SetInnerRadius(0.0);
  unitDisk->SetCircumferentialResolution(80);
  unitDisk->SetRadialResolution(15);

  vtkNew<vtkDelaunay2D> triangulator;
  triangulator->SetTolerance(0.01); // get rid of the small triangles near the center of the unit disk
  triangulator->SetInputConnection(unitDisk->GetOutputPort());

  vtkNew<vtkTransformPolyDataFilter> polyTransformToCurve;
  polyTransformToCurve->SetTransform(landmarkTransform);
  polyTransformToCurve->SetInputConnection(triangulator->GetOutputPort());

  vtkNew<vtkPolyDataNormals> polyDataNormals;
  polyDataNormals->SetInputConnection(polyTransformToCurve->GetOutputPort());
  // There are a few triangles in the triangulated unit disk with inconsistent
  // orientation. Enabling consistency check fixes them.
  polyDataNormals->ConsistencyOn();
  polyDataNormals->Update();

  surface->DeepCopy(polyDataNormals->GetOutput());
  return true;
}
