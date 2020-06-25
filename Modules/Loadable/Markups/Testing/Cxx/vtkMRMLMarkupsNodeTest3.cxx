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

// MRML includes
#include "vtkMRMLCoreTestingMacros.h"
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsClosedCurveNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkIndent.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkRegularPolygonSource.h>
#include <vtkTestingOutputWindow.h>

// STL includes
#include <sstream>

// Test markups measurements


int CheckCurvePointDistances(vtkMRMLMarkupsCurveNode* curveNode, double expectedRegularDistance,
  double expectedDistanceSecondLast, double expectedDistanceLast, double tolerance, bool distanceOnCurve = false, bool verbose = false)
{
  bool rerunWithReporting = false;
  while (true)
    {
    if (verbose || rerunWithReporting)
      {
      std::cout << "CheckCurvePointDistances:" << std::endl;
      }
    int lastPointIndex = (curveNode->GetCurveClosed() ? curveNode->GetNumberOfControlPoints() : curveNode->GetNumberOfControlPoints() - 1);
    for (int pointIndex = 0; pointIndex < lastPointIndex; pointIndex++)
      {
      double actualDistance;
      if (distanceOnCurve)
        {
        actualDistance = curveNode->GetCurveLengthBetweenStartEndPointsWorld(
          curveNode->GetCurvePointIndexFromControlPointIndex(pointIndex % curveNode->GetNumberOfControlPoints()),
          curveNode->GetCurvePointIndexFromControlPointIndex((pointIndex + 1) % curveNode->GetNumberOfControlPoints()));
        }
      else
        {
        actualDistance = sqrt(vtkMath::Distance2BetweenPoints(
          curveNode->GetNthControlPointPosition(pointIndex % curveNode->GetNumberOfControlPoints()),
          curveNode->GetNthControlPointPosition((pointIndex + 1) % curveNode->GetNumberOfControlPoints())));
        }
      double expectedDistance = expectedRegularDistance;
      if (pointIndex == lastPointIndex - 1)
        {
        expectedDistance = expectedDistanceLast;
        }
      else if (pointIndex == lastPointIndex - 2)
        {
        expectedDistance = expectedDistanceSecondLast;
        }
      if (verbose || rerunWithReporting)
        {
        std::cout << "  " << pointIndex << ":  expected = " << expectedDistance << " ["
          << expectedDistance-tolerance << ", " << expectedDistance + tolerance << "]    actual = " << actualDistance
          << " (error=" << actualDistance- expectedDistance << ")" << std::endl;
        }
      if (fabs(actualDistance - expectedDistance) > tolerance)
        {
        // error found, rerun with reporting
        if (!rerunWithReporting)
          {
          rerunWithReporting = true;
          continue;
          }
        else
          {
          std::cout << "    -> ERROR - actual value is out of tolerance range" << std::endl;
          }
        }
      }
    return rerunWithReporting ? EXIT_FAILURE : EXIT_SUCCESS;
    }
}

int vtkMRMLMarkupsNodeTest3(int , char * [] )
{
  //
  // Test curve interpolation
  //

  bool verbose = false; // set it to true to help during debugging

  double tol = 0.001; // comparison tolerance
  bool distanceOnCurve = false;

  // Open curve
  // Test it on a straight line (to make sure we don't rely on distance measurement along curve)
  std::cout << "Open curve" << std::endl;

  vtkNew<vtkMRMLMarkupsCurveNode> curveNode;
  double curveLength = 100.0;
  curveNode->AddControlPoint(vtkVector3d(10, 25, -30));
  curveNode->AddControlPoint(vtkVector3d(10, 25+curveLength, -30));
  CHECK_DOUBLE_TOLERANCE(curveNode->GetCurveLengthWorld(), curveLength, tol);

  curveNode->ResampleCurveWorld(1.0);
  CHECK_INT(curveNode->GetNumberOfControlPoints(), 101);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(curveNode, 1.0, 1.0, 1.0, tol, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(curveNode->GetCurveLengthWorld(), curveLength, tol);

  curveNode->ResampleCurveWorld(10.0);
  CHECK_INT(curveNode->GetNumberOfControlPoints(), 11);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(curveNode, 10.0, 10.0, 10.0, tol, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(curveNode->GetCurveLengthWorld(), curveLength, tol);

  curveNode->ResampleCurveWorld(10.01);
  CHECK_INT(curveNode->GetNumberOfControlPoints(), 11);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(curveNode, 10.01, 9.96, 9.96, tol, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(curveNode->GetCurveLengthWorld(), curveLength, tol);

  curveNode->ResampleCurveWorld(9.99);
  CHECK_INT(curveNode->GetNumberOfControlPoints(), 11);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(curveNode, 9.99, 9.99, 10.09, tol, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(curveNode->GetCurveLengthWorld(), curveLength, tol);

  // Closed curve
  std::cout << "Closed curve" << std::endl;

  distanceOnCurve = true;

  vtkNew<vtkMRMLMarkupsClosedCurveNode> closedCurveNode;
  double radius = 15;
  int numberOfControlPoints = 20;
  double closedCurveLength = 2 * radius * vtkMath::Pi();
  double circumferenceTol = 0.1; // curve is not straight line, so length can slightly change as we resample

  vtkNew<vtkRegularPolygonSource> polygonSource;
  polygonSource->SetNumberOfSides(numberOfControlPoints);
  polygonSource->SetRadius(radius);
  polygonSource->SetCenter(3, 4, 9);
  polygonSource->Update();
  closedCurveNode->SetControlPointPositionsWorld(polygonSource->GetOutput()->GetPoints());
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol);

  double samplingDistance = 10.0;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 9);
  // tolerance is large because control points are quite far from each other, so curve segments are long and some variance is expected
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, samplingDistance, samplingDistance,
    samplingDistance / 2.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol);

  samplingDistance = 1.0;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 94);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, samplingDistance, samplingDistance,
    samplingDistance / 10.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol);

  samplingDistance = closedCurveNode->GetCurveLengthWorld() / 10.0;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 10);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, samplingDistance, samplingDistance,
    samplingDistance / 10.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol * 2);

  samplingDistance = closedCurveNode->GetCurveLengthWorld() / 10.1;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 10);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, samplingDistance, samplingDistance,
    samplingDistance / 10.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol * 2);

  samplingDistance = closedCurveNode->GetCurveLengthWorld() / 9.99;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 10);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, samplingDistance, samplingDistance,
    samplingDistance / 10.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol * 2);

  samplingDistance = closedCurveNode->GetCurveLengthWorld() / 10.4;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 10);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, 10.84, 10.82,
    samplingDistance / 100.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol * 3);

  samplingDistance = closedCurveNode->GetCurveLengthWorld() / 10.6;
  closedCurveNode->ResampleCurveWorld(samplingDistance);
  CHECK_INT(closedCurveNode->GetNumberOfControlPoints(), 11);
  CHECK_EXIT_SUCCESS(CheckCurvePointDistances(closedCurveNode, samplingDistance, 7.09, 7.12,
    samplingDistance / 100.0, distanceOnCurve, verbose));
  CHECK_DOUBLE_TOLERANCE(closedCurveNode->GetCurveLengthWorld(), closedCurveLength, circumferenceTol * 3);

  std::cout << "Success." << std::endl;

  return EXIT_SUCCESS;
}
