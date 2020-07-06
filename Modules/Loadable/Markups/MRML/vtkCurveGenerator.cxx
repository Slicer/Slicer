/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Thomas Vaughan, PerkLab, Queen's University.

==============================================================================*/

// Markups MRML includes
#include "vtkCurveGenerator.h"
#include "vtkLinearSpline.h"

// VTK includes
#include <vtkCardinalSpline.h>
#include <vtkDoubleArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkKochanekSpline.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunction.h>
#include "vtkParametricPolynomialApproximation.h"
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkSlicerDijkstraGraphGeodesicPath.h>

#include <vtkLine.h>

// std includes
#include <algorithm>
#include <list>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkCurveGenerator);

//------------------------------------------------------------------------------
vtkCurveGenerator::vtkCurveGenerator()
{
  this->SetNumberOfInputPorts(2);

  this->SetCurveTypeToLinearSpline();
  this->CurveIsLoop = false;
  this->NumberOfPointsPerInterpolatingSegment = 5;
  this->KochanekBias = 0.0;
  this->KochanekContinuity = 0.0;
  this->KochanekTension = 0.0;
  this->KochanekEndsCopyNearestDerivatives = false;
  this->PolynomialOrder = 3; // cubic
  this->PolynomialPointSortingMethod = vtkCurveGenerator::SORTING_METHOD_INDEX;
  this->PolynomialFitMethod = vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_GLOBAL_LEAST_SQUARES;
  this->PolynomialWeightFunction = vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_GAUSSIAN;
  this->PolynomialSampleWidth = 0.5;
  this->OutputCurveLength = 0.0;

  // timestamps for input and output are the same, initially
  this->Modified();

  // local storage variables
  this->SurfacePointLocator = vtkSmartPointer<vtkPointLocator>::New();
  this->SurfacePathFilter = vtkSmartPointer<vtkSlicerDijkstraGraphGeodesicPath>::New();
  this->SurfacePathFilter->StopWhenEndReachedOn();
  this->InputParameters = nullptr;
  this->ParametricFunction = nullptr;
}

//------------------------------------------------------------------------------
vtkCurveGenerator::~vtkCurveGenerator() = default;

//------------------------------------------------------------------------------
void vtkCurveGenerator::PrintSelf(std::ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "InputParameters size: " << (this->InputParameters != nullptr ? this->InputParameters->GetNumberOfTuples() : 0) << std::endl;
  os << indent << "CurveType: " << this->GetCurveTypeAsString(this->CurveType) << std::endl;
  os << indent << "CurveIsLoop: " << this->CurveIsLoop << std::endl;
  os << indent << "KochanekBias: " << this->KochanekBias << std::endl;
  os << indent << "KochanekContinuity: " << this->KochanekContinuity << std::endl;
  os << indent << "KochanekTension: " << this->KochanekTension << std::endl;
  os << indent << "KochanekEndsCopyNearestDerivatives: " << this->KochanekEndsCopyNearestDerivatives << std::endl;
  os << indent << "PolynomialOrder: " << this->PolynomialOrder << std::endl;
  os << indent << "SurfaceCostFunctionType: " <<
    vtkSlicerDijkstraGraphGeodesicPath::GetCostFunctionTypeAsString(this->GetSurfaceCostFunctionType()) << std::endl;
}

//----------------------------------------------------------------------------
int vtkCurveGenerator::FillInputPortInformation(
  int port, vtkInformation* info)
{
  if (port == 0)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    }
  else if (port == 1)
    {
    info->Set(vtkAlgorithm::INPUT_REQUIRED_DATA_TYPE(), "vtkPolyData");
    info->Set(vtkAlgorithm::INPUT_IS_OPTIONAL(), 1);
    }
  else
    {
    vtkErrorMacro("Cannot set input info for port " << port);
    return 0;
    }

  return 1;
}

//------------------------------------------------------------------------------
// ACCESSORS/MUTATORS
//------------------------------------------------------------------------------
const char* vtkCurveGenerator::GetCurveTypeAsString(int curveType)
{
  switch (curveType)
    {
    case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
      {
      return "linear";
      }
    case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
      {
      return "spline";
      }
    case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
      {
      return "kochanekSpline";
      }
    case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
      {
      return "polynomial";
      }
    case vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE:
      {
      return "shortestSurfaceDistance";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown curve type: " << curveType);
      return "";
      }
    }
}

//-----------------------------------------------------------
int vtkCurveGenerator::GetCurveTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkCurveGenerator::CURVE_TYPE_LAST; i++)
    {
    if (strcmp(name, vtkCurveGenerator::GetCurveTypeAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown curve type: " << name);
  return -1;
}

//------------------------------------------------------------------------------
const char* vtkCurveGenerator::GetPolynomialPointSortingMethodAsString(int polynomialPointSortingMethod)
{
  switch (polynomialPointSortingMethod)
    {
    case vtkCurveGenerator::SORTING_METHOD_INDEX:
      {
      return "indices";
      }
    case vtkCurveGenerator::SORTING_METHOD_MINIMUM_SPANNING_TREE_POSITION:
      {
      return "minimumSpanningTreePosition";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown sorting method: " << polynomialPointSortingMethod);
      return "";
      }
    }
}

//-----------------------------------------------------------
int vtkCurveGenerator::GetPolynomialPointSortingMethodFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    vtkGenericWarningMacro("Invalid sorting method name");
    return -1;
    }
  for (int i = 0; i < vtkCurveGenerator::SORTING_METHOD_LAST; i++)
    {
    if (strcmp(name, vtkCurveGenerator::GetPolynomialPointSortingMethodAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown sorting method name: " << name);
  return -1;
}

//------------------------------------------------------------------------------
const char* vtkCurveGenerator::GetPolynomialFitMethodAsString(int polynomialFitMethod)
{
  switch (polynomialFitMethod)
    {
    case vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_GLOBAL_LEAST_SQUARES:
      {
      return "globalLeastSquares";
      }
    case vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_MOVING_LEAST_SQUARES:
      {
      return "movingLeastSquares";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown fit method method: " << polynomialFitMethod);
      return "";
      }
    }
}

//-----------------------------------------------------------
int vtkCurveGenerator::GetPolynomialFitMethodFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    vtkGenericWarningMacro("Invalid polynomial fit method name");
    return -1;
    }
  for (int i = 0; i < vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_LAST; i++)
    {
    if (strcmp(name, vtkCurveGenerator::GetPolynomialFitMethodAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown polynomial fit method name: " << name);
  return -1;
}


//------------------------------------------------------------------------------
const char* vtkCurveGenerator::GetPolynomialWeightFunctionAsString(int polynomialWeightFunction)
{
  switch (polynomialWeightFunction)
    {
    case vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_RECTANGULAR:
      {
      return "rectangular";
      }
    case vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_TRIANGULAR:
      {
      return "triangular";
      }
    case vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_COSINE:
      {
      return "cosine";
      }
    case vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_GAUSSIAN:
      {
      return "gaussian";
      }
    default:
      {
      vtkGenericWarningMacro("Unknown weight function: " << polynomialWeightFunction);
      return "";
      }
    }
}

//-----------------------------------------------------------
int vtkCurveGenerator::GetPolynomialWeightFunctionFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    vtkGenericWarningMacro("Invalid polynomial weight function name");
    return -1;
    }
  for (int i = 0; i < vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_LAST; i++)
    {
    if (strcmp(name, vtkCurveGenerator::GetPolynomialWeightFunctionAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // name not found
  vtkGenericWarningMacro("Unknown polynomial weight function name: " << name);
  return -1;
}

//------------------------------------------------------------------------------
double vtkCurveGenerator::GetOutputCurveLength()
{
  return this->OutputCurveLength;
}

//------------------------------------------------------------------------------
int vtkCurveGenerator::RequestData(
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

  vtkPolyData* inputSurfaceMesh = nullptr;
  vtkInformation* inInfo2 = inputVector[1]->GetInformationObject(0);
  if (inInfo2)
    {
    inputSurfaceMesh = vtkPolyData::SafeDownCast(
      inInfo2->Get(vtkDataObject::DATA_OBJECT()));
    }

  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkPolyData* outputPolyData = vtkPolyData::SafeDownCast(
    outInfo->Get(vtkDataObject::DATA_OBJECT()));

  if (!this->GeneratePoints(inputPoints, inputSurfaceMesh, outputPolyData))
    {
    return 0;
    }

  if (!this->GenerateLines(outputPolyData))
    {
    return 0;
    }

  outputPolyData->Squeeze();
  return 1;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToSpline(vtkPoints* inputPoints, vtkSpline* xSpline, vtkSpline* ySpline, vtkSpline* zSpline)
{
  vtkSmartPointer<vtkParametricSpline> parametricSpline = vtkSmartPointer<vtkParametricSpline>::New();
  parametricSpline->SetXSpline(xSpline);
  parametricSpline->SetYSpline(ySpline);
  parametricSpline->SetZSpline(zSpline);
  parametricSpline->SetPoints(inputPoints);
  parametricSpline->SetClosed(this->CurveIsLoop);
  parametricSpline->SetParameterizeByLength(false);
  this->ParametricFunction = parametricSpline;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToLinearSpline(vtkPoints* inputPoints)
{
  vtkSmartPointer<vtkLinearSpline> xSpline = vtkSmartPointer<vtkLinearSpline>::New();
  vtkSmartPointer<vtkLinearSpline> ySpline = vtkSmartPointer<vtkLinearSpline>::New();
  vtkSmartPointer<vtkLinearSpline> zSpline = vtkSmartPointer<vtkLinearSpline>::New();
  this->SetParametricFunctionToSpline(inputPoints, xSpline, ySpline, zSpline);
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToCardinalSpline(vtkPoints* inputPoints)
{
  vtkSmartPointer<vtkCardinalSpline> xSpline = vtkSmartPointer<vtkCardinalSpline>::New();
  vtkSmartPointer<vtkCardinalSpline> ySpline = vtkSmartPointer<vtkCardinalSpline>::New();
  vtkSmartPointer<vtkCardinalSpline> zSpline = vtkSmartPointer<vtkCardinalSpline>::New();
  this->SetParametricFunctionToSpline(inputPoints, xSpline, ySpline, zSpline);
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToKochanekSpline(vtkPoints* inputPoints)
{
  vtkSmartPointer<vtkKochanekSpline> xSpline = vtkSmartPointer<vtkKochanekSpline>::New();
  xSpline->SetDefaultBias(this->KochanekBias);
  xSpline->SetDefaultTension(this->KochanekTension);
  xSpline->SetDefaultContinuity(this->KochanekContinuity);

  vtkSmartPointer< vtkKochanekSpline > ySpline = vtkSmartPointer< vtkKochanekSpline >::New();
  ySpline->SetDefaultBias(this->KochanekBias);
  ySpline->SetDefaultTension(this->KochanekTension);
  ySpline->SetDefaultContinuity(this->KochanekContinuity);

  vtkSmartPointer< vtkKochanekSpline > zSpline = vtkSmartPointer< vtkKochanekSpline >::New();
  zSpline->SetDefaultBias(this->KochanekBias);
  zSpline->SetDefaultTension(this->KochanekTension);
  zSpline->SetDefaultContinuity(this->KochanekContinuity);

  if (this->KochanekEndsCopyNearestDerivatives)
    {
    // manually set the derivative to the nearest value
    // (difference between the two nearest points). The
    // constraint mode is set to 1, this tells the spline
    // class to use our manual definition.
    // left derivative
    xSpline->SetLeftConstraint(1);
    ySpline->SetLeftConstraint(1);
    zSpline->SetLeftConstraint(1);
    // we assume there are at least 2 points, this is already checked in the Update() functions
    double point0[3];
    inputPoints->GetPoint(0, point0);
    double point1[3];
    inputPoints->GetPoint(1, point1);
    xSpline->SetLeftValue(point1[0] - point0[0]);
    ySpline->SetLeftValue(point1[1] - point0[1]);
    zSpline->SetLeftValue(point1[2] - point0[2]);
    // right derivative
    xSpline->SetRightConstraint(1);
    ySpline->SetRightConstraint(1);
    zSpline->SetRightConstraint(1);
    int numberOfInputPoints = inputPoints->GetNumberOfPoints();
    double pointNMinus2[3];
    inputPoints->GetPoint(numberOfInputPoints - 2, pointNMinus2);
    double pointNMinus1[3];
    inputPoints->GetPoint(numberOfInputPoints - 1, pointNMinus1);
    xSpline->SetRightValue(pointNMinus1[0] - pointNMinus2[0]);
    ySpline->SetRightValue(pointNMinus1[1] - pointNMinus2[1]);
    zSpline->SetRightValue(pointNMinus1[2] - pointNMinus2[2]);
    }
  else
    {
    // This ("0") is the most simple mode for end derivative computation,
    // described by documentation as using the "first/last two points".
    // Use this as the default because others would require setting the
    // derivatives manually
    xSpline->SetLeftConstraint(0);
    ySpline->SetLeftConstraint(0);
    zSpline->SetLeftConstraint(0);
    xSpline->SetRightConstraint(0);
    ySpline->SetRightConstraint(0);
    zSpline->SetRightConstraint(0);
    }

  this->SetParametricFunctionToSpline(inputPoints, xSpline, ySpline, zSpline);
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToPolynomial(vtkPoints* inputPoints)
{
  vtkSmartPointer< vtkParametricPolynomialApproximation > polynomial = vtkSmartPointer< vtkParametricPolynomialApproximation >::New();
  polynomial->SetPoints(inputPoints);
  polynomial->SetPolynomialOrder(this->PolynomialOrder);

  if (this->InputParameters == nullptr)
    {
    this->InputParameters = vtkSmartPointer< vtkDoubleArray >::New();
    }

  if (this->PolynomialPointSortingMethod == vtkCurveGenerator::SORTING_METHOD_INDEX)
    {
    vtkCurveGenerator::SortByIndex(inputPoints, this->InputParameters);
    }
  else if (this->PolynomialPointSortingMethod == vtkCurveGenerator::SORTING_METHOD_MINIMUM_SPANNING_TREE_POSITION)
    {
    vtkCurveGenerator::SortByMinimumSpanningTreePosition(inputPoints, this->InputParameters);
    }
  else
    {
    vtkWarningMacro("Did not recognize point sorting method: " << this->PolynomialPointSortingMethod << ". Empty parameters will be used.");
    }
  polynomial->SetParameters(this->InputParameters);

  polynomial->SetFitMethod(this->PolynomialFitMethod);

  if (this->PolynomialFitMethod == vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_GLOBAL_LEAST_SQUARES)
    {
    polynomial->SetFitMethodToGlobalLeastSquares();
    }
  else if (this->PolynomialFitMethod == vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_MOVING_LEAST_SQUARES)
    {
    polynomial->SetFitMethodToMovingLeastSquares();
    }
  else
    {
    vtkWarningMacro("Did not recognize fit method: " << this->PolynomialFitMethod << ". Parametric polynomial will use its default behaviour.");
    }

  polynomial->SetSampleWidth(this->PolynomialSampleWidth);

  if (this->PolynomialWeightFunction == vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_RECTANGULAR)
    {
    polynomial->SetWeightFunctionToRectangular();
    }
  else if (this->PolynomialWeightFunction == vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_TRIANGULAR)
    {
    polynomial->SetWeightFunctionToTriangular();
    }
  else if (this->PolynomialWeightFunction == vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_COSINE)
    {
    polynomial->SetWeightFunctionToCosine();
    }
  else if (this->PolynomialWeightFunction == vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_GAUSSIAN)
    {
    polynomial->SetWeightFunctionToGaussian();
    }
  else
    {
    vtkWarningMacro("Did not recognize weight function: " << this->PolynomialWeightFunction << ". Parametric polynomial will use its default behaviour.");
    }

  this->ParametricFunction = polynomial;
}

//------------------------------------------------------------------------------
int vtkCurveGenerator::GeneratePoints(vtkPoints* inputPoints, vtkPolyData* inputSurface, vtkPolyData* outputPolyData)
{
  vtkNew<vtkPoints> outputPoints;
  this->OutputCurveLength = 0.0;
  this->InterpolatedPointIdsForControlPoints.clear();

  switch (this->CurveType)
  {
  case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
  case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
  case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
  case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
    {
    if (!this->GeneratePointsFromFunction(inputPoints, outputPoints))
      {
      return 0;
      }
    break;
    }
  case vtkCurveGenerator::CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE:
    if (!this->GeneratePointsFromSurface(inputPoints, inputSurface, outputPoints))
      {
      return 0;
      }
    break;
  default:
    {
    vtkErrorMacro("Error: Unrecognized curve type: " << this->CurveType << ".");
    return 0;
    }
  }

  outputPolyData->SetPoints(outputPoints);
  return 1;
}

//------------------------------------------------------------------------------
int vtkCurveGenerator::GeneratePointsFromFunction(vtkPoints* inputPoints, vtkPoints* outputPoints)
{
  int numberOfInputPoints = inputPoints->GetNumberOfPoints();
  if (numberOfInputPoints <= 1)
    {
    return 0;
    }

  switch (this->CurveType)
    {
    case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
      {
      this->SetParametricFunctionToLinearSpline(inputPoints);
      break;
      }
    case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
      {
      this->SetParametricFunctionToCardinalSpline(inputPoints);
      break;
      }
    case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
      {
      this->SetParametricFunctionToKochanekSpline(inputPoints);
      break;
      }
    case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
      {
      this->SetParametricFunctionToPolynomial(inputPoints);
      break;
      }
    default:
      {
      vtkErrorMacro("Error: Unrecognized curve type: " << this->CurveType << ".");
      break;
      }
    }

  int numberOfSegments = 0; // temporary value
  if (this->CurveIsLoop && this->CurveType != vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL)
    {
    numberOfSegments = numberOfInputPoints;
    }
  else
    {
    numberOfSegments = (numberOfInputPoints - 1);
    }

  int totalNumberOfPoints = this->NumberOfPointsPerInterpolatingSegment * numberOfSegments + 1;
  double previousPoint[3];
  for (int pointIndex = 0; pointIndex < totalNumberOfPoints; pointIndex++)
    {
    double sampleParameter = pointIndex / (double)(totalNumberOfPoints - 1);
    double curvePoint[3];
    this->ParametricFunction->Evaluate(&sampleParameter, curvePoint, nullptr);
    outputPoints->InsertNextPoint(curvePoint);
    if (pointIndex > 0)
      {
      double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(previousPoint, curvePoint));
      this->OutputCurveLength += segmentLength;
      }
    previousPoint[0] = curvePoint[0];
    previousPoint[1] = curvePoint[1];
    previousPoint[2] = curvePoint[2];
  }
  return 1;
}

//------------------------------------------------------------------------------
int vtkCurveGenerator::GeneratePointsFromSurface(vtkPoints* inputPoints, vtkPolyData* inputSurface, vtkPoints* outputPoints)
{
  // If there is no surface, there are no points. Don't report as an error.
  if (!inputSurface)
    {
    return 1;
    }

  // If there are no input points, there are output points. Don't report as an error.
  vtkIdType numberOfInputPoints = inputPoints->GetNumberOfPoints();
  if (numberOfInputPoints <= 1)
    {
    return 1;
    }

  vtkIdType numberOfSegments = 0;
  if (this->CurveIsLoop)
    {
    numberOfSegments = numberOfInputPoints;
    }
  else
    {
    numberOfSegments = (numberOfInputPoints - 1);
    }

  this->SurfacePathFilter->SetInputData(inputSurface);
  this->SurfacePointLocator->SetDataSet(inputSurface);
  this->SurfacePointLocator->BuildLocator();

  for (vtkIdType controlPointIndex = 0; controlPointIndex < numberOfSegments; ++controlPointIndex)
    {
    double controlPoint1[3] = { 0 };
    inputPoints->GetPoint(controlPointIndex, controlPoint1);
    vtkIdType id1 = this->SurfacePointLocator->FindClosestPoint(controlPoint1);

    double controlPoint2[3] = { 0 };
    inputPoints->GetPoint((controlPointIndex + 1) % numberOfInputPoints, controlPoint2);
    vtkIdType id2 = this->SurfacePointLocator->FindClosestPoint(controlPoint2);

    // Path is traced backward, so start vertex should be point2, and end should be point1.
    this->SurfacePathFilter->SetStartVertex(id2);
    this->SurfacePathFilter->SetEndVertex(id1);
    this->SurfacePathFilter->Update();

    vtkPolyData* outputPath = this->SurfacePathFilter->GetOutput();
    double previousPoint[3] = { 0 };
    for (vtkIdType pointIndex = 0; pointIndex < outputPath->GetNumberOfPoints(); ++pointIndex)
      {
      double curvePoint[3] = { 0 };
      outputPath->GetPoint(pointIndex, curvePoint);

      if (controlPointIndex == 0 || pointIndex > 0)
        {
        vtkIdType outputPointId = outputPoints->InsertNextPoint(curvePoint);
        if (static_cast<vtkIdType>(this->InterpolatedPointIdsForControlPoints.size()) <= controlPointIndex)
          {
          this->InterpolatedPointIdsForControlPoints.push_back(outputPointId);
          }

        if (pointIndex > 0)
          {
          double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(previousPoint, curvePoint));
          this->OutputCurveLength += segmentLength;
          }
        }
      previousPoint[0] = curvePoint[0];
      previousPoint[1] = curvePoint[1];
      previousPoint[2] = curvePoint[2];
      }
    }
  this->InterpolatedPointIdsForControlPoints.push_back(outputPoints->GetNumberOfPoints() - 1);

  return 1;
}

//------------------------------------------------------------------------------
int vtkCurveGenerator::GenerateLines(vtkPolyData* polyData)
{
  // Update lines: a single cell containing a line with point
  // indices: 0, 1, ..., last point (and an extra 0 if closed curve).
  vtkIdType numberOfPoints = polyData->GetNumberOfPoints();
  vtkNew<vtkCellArray> lines;
  if (numberOfPoints > 1)
    {
    bool loop = (numberOfPoints > 2 && this->CurveIsLoop);
    vtkIdType numberOfCellPoints = (loop ? numberOfPoints + 1 : numberOfPoints);

    // Only regenerate indices if necessary
    bool needToUpdateLines = true;
    if (lines->GetNumberOfCells() == 1)
      {
      vtkIdType currentNumberOfCellPoints = 0;
      vtkIdType* currentCellPoints = nullptr;
      lines->GetCell(0, currentNumberOfCellPoints, currentCellPoints);

      if (currentNumberOfCellPoints == numberOfCellPoints)
        {
        needToUpdateLines = false;
        }
      }

    if (needToUpdateLines)
      {
      lines->Reset();
      lines->InsertNextCell(numberOfCellPoints);
      for (int i = 0; i < numberOfPoints; i++)
        {
        lines->InsertCellPoint(i);
        }
      if (loop)
        {
        lines->InsertCellPoint(0);
        }
      lines->Modified();
      }
    }
  polyData->SetLines(lines);
  return 1;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SortByIndex(vtkPoints* points, vtkDoubleArray* parameters)
{
  if (points == nullptr)
    {
    vtkGenericWarningMacro("Input control points are null. Returning.");
    return;
    }

  if (parameters == nullptr)
    {
    vtkGenericWarningMacro("Output control point parameters are null. Returning.");
    return;
    }

  int numberOfPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numberOfPoints < 2)
    {
    vtkGenericWarningMacro("Not enough points to compute polynomial parameters. Need at least 2 points but " << numberOfPoints << " are provided.");
    return;
    }

  parameters->Reset();
  for (int v = 0; v < numberOfPoints; v++)
    {
    parameters->InsertNextTuple1(v / double(numberOfPoints - 1));
    // division to clamp all values to range 0.0 - 1.0
    }
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SortByMinimumSpanningTreePosition(vtkPoints* points, vtkDoubleArray* parameters)
{
  if (points == nullptr)
    {
    vtkGenericWarningMacro("Input points are null. Returning");
    return;
    }

  if (parameters == nullptr)
    {
    vtkGenericWarningMacro("Output point parameters are null. Returning");
    return;
    }

  int numberOfPoints = points->GetNumberOfPoints();
  // redundant error checking, to be safe
  if (numberOfPoints < 2)
    {
    vtkGenericWarningMacro("Not enough points to compute polynomial parameters. Need at least 2 points but " << numberOfPoints << " are provided.");
    return;
    }

  // vtk boost algorithms cannot be used because they are not built with 3D Slicer
  // so this is a custom implementation of:
  // 1. constructing an undirected graph as a 2D array
  // 2. Finding the two vertices that are the farthest apart
  // 3. running prim's algorithm on the graph
  // 4. extract the "trunk" path from the last vertex to the first
  // 5. based on the distance along that path, assign each vertex a polynomial parameter value

  // in the following code, two tasks are done:
  // 1. construct an undirected graph
  std::vector< double > distances(numberOfPoints * numberOfPoints);
  distances.assign(numberOfPoints * numberOfPoints, 0.0);
  // 2. find the two farthest-seperated vertices in the distances array
  int treeStartIndex = 0;
  int treeEndIndex = 0;
  double maximumDistance = 0;
  // iterate through all points
  for (int v = 0; v < numberOfPoints; v++)
    {
    double pointV[3];
    points->GetPoint(v, pointV);
    for (int u = 0; u < numberOfPoints; u++)
      {
      double pointU[3];
      points->GetPoint(u, pointU);
      double distanceSquared = vtkMath::Distance2BetweenPoints(pointU, pointV);
      double distance = sqrt(distanceSquared);
      distances[v * numberOfPoints + u] = distance;
      if (distance > maximumDistance)
        {
        maximumDistance = distance;
        treeStartIndex = v;
        treeEndIndex = u;
        }
      }
    }
  // use the 1D vector as a 2D vector
  std::vector< double* > graph(numberOfPoints);
  for (int v = 0; v < numberOfPoints; v++)
    {
    graph[v] = &(distances[v * numberOfPoints]);
    }

  // implementation of Prim's algorithm heavily based on:
  // http://www.geeksforgeeks.org/greedy-algorithms-set-5-prims-minimum-spanning-tree-mst-2/
  std::vector< int > parent(numberOfPoints); // Array to store constructed MST
  std::vector< double > key(numberOfPoints);   // Key values used to pick minimum weight edge in cut
  std::vector< bool > mstSet(numberOfPoints);  // To represent set of vertices not yet included in MST

  // Initialize all keys as INFINITE (or at least as close as we can get)
  for (int i = 0; i < numberOfPoints; i++)
    {
    key[i] = VTK_DOUBLE_MAX;
    mstSet[i] = false;
    }

  // Always include first 1st vertex in MST.
  key[treeStartIndex] = 0.0; // Make key 0 so that this vertex is picked as first vertex
  parent[treeStartIndex] = -1; // First node is always root of MST

  // The MST will have numberOfPoints vertices
  for (int count = 0; count < numberOfPoints - 1; count++)
    {
    // Pick the minimum key vertex from the set of vertices
    // not yet included in MST
    int nextPointIndex = -1;
    double minDistance = VTK_DOUBLE_MAX;
    for (int v = 0; v < numberOfPoints; v++)
      {
      if (mstSet[v] == false && key[v] < minDistance)
        {
        minDistance = key[v];
        nextPointIndex = v;
        }
      }

    // Add the picked vertex to the MST Set
    mstSet[nextPointIndex] = true;

    // Update key value and parent index of the adjacent vertices of
    // the picked vertex. Consider only those vertices which are not yet
    // included in MST
    for (int v = 0; v < numberOfPoints; v++)
      {
      // graph[u][v] is non zero only for adjacent vertices of m
      // mstSet[v] is false for vertices not yet included in MST
      // Update the key only if graph[u][v] is smaller than key[v]
      if (graph[nextPointIndex][v] >= 0 &&
        mstSet[v] == false &&
        graph[nextPointIndex][v] < key[v])
        {
        parent[v] = nextPointIndex;
        key[v] = graph[nextPointIndex][v];
        }
      }
    }

  // determine the "trunk" path of the tree, from first index to last index
  std::vector< int > pathIndices;
  int currentPathIndex = treeEndIndex;
  while (currentPathIndex != -1)
    {
    pathIndices.push_back(currentPathIndex);
    currentPathIndex = parent[currentPathIndex]; // go up the tree one layer
    }

  // find the sum of distances along the trunk path of the tree
  double sumOfDistances = 0.0;
  for (unsigned int i = 0; i < pathIndices.size() - 1; i++)
    {
    int pathVertexIndexI = pathIndices[i];
    int pathVertexIndexIPlus1 = pathIndices[i + 1];
    sumOfDistances += graph[pathVertexIndexI][pathVertexIndexIPlus1];
    }

  // check this to prevent a division by zero (in case all points are duplicates)
  if (sumOfDistances == 0)
    {
    vtkGenericWarningMacro("Minimum spanning tree path has distance zero. No parameters will be assigned. Check inputs (are there duplicate points?).");
    return;
    }

  // find the parameters along the trunk path of the tree
  std::vector< double > pathParameters;
  double currentDistance = 0.0;
  for (unsigned int i = 0; i < pathIndices.size() - 1; i++)
    {
    pathParameters.push_back(currentDistance / sumOfDistances);
    int pathVertexIndexI = pathIndices[i];
    int pathVertexIndexIPlus1 = pathIndices[i + 1];
    currentDistance += graph[pathVertexIndexI][pathVertexIndexIPlus1];
    }
  pathParameters.push_back(currentDistance / sumOfDistances); // this should be 1.0

  // finally assign polynomial parameters to each point, and store in the output array
  parameters->Reset();
  for (int i = 0; i < numberOfPoints; i++)
    {
    int currentIndex = i;
    bool alongPath = false;
    int indexAlongPath = -1;
    for (unsigned int j = 0; j < pathIndices.size(); j++)
      {
      if (pathIndices[j] == currentIndex)
        {
        alongPath = true;
        indexAlongPath = j;
        break;
        }
      }
    while (!alongPath)
      {
      currentIndex = parent[currentIndex];
      for (unsigned int j = 0; j < pathIndices.size(); j++)
        {
        if (pathIndices[j] == currentIndex)
          {
          alongPath = true;
          indexAlongPath = j;
          break;
          }
        }
      }
    parameters->InsertNextTuple1(pathParameters[indexAlongPath]);
    }
}

//------------------------------------------------------------------------------
bool vtkCurveGenerator::IsInterpolatingCurve()
{
  return (this->CurveType == CURVE_TYPE_LINEAR_SPLINE
    || this->CurveType == CURVE_TYPE_CARDINAL_SPLINE
    || this->CurveType == CURVE_TYPE_KOCHANEK_SPLINE);
}

//------------------------------------------------------------------------------
vtkIdList* vtkCurveGenerator::GetSurfacePointIds()
{
  return this->SurfacePathFilter->GetIdList();
}

//------------------------------------------------------------------------------
vtkIdType vtkCurveGenerator::GetControlPointIdFromInterpolatedPointId(vtkIdType interpolatedPointId)
{
  int controlId = -1;
  if (this->CurveType == CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE)
    {
    std::vector<vtkIdType>::iterator it = std::lower_bound(this->InterpolatedPointIdsForControlPoints.begin(),
      this->InterpolatedPointIdsForControlPoints.end(), interpolatedPointId);
    if (it != this->InterpolatedPointIdsForControlPoints.end())
      {
      controlId = it - this->InterpolatedPointIdsForControlPoints.begin() - 1;
      }
    }
  else if (this->IsInterpolatingCurve())
    {
    controlId = int(floor(interpolatedPointId / this->GetNumberOfPointsPerInterpolatingSegment()));
    }
  return controlId;
}

//------------------------------------------------------------------------------
int vtkCurveGenerator::GetSurfaceCostFunctionType()
{
  return this->SurfacePathFilter->GetCostFunctionType();
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetSurfaceCostFunctionType(int surfaceCostFunctionType)
{
  if (this->SurfacePathFilter->GetCostFunctionType() == surfaceCostFunctionType)
    {
    return;
    }
  this->SurfacePathFilter->SetCostFunctionType(surfaceCostFunctionType);
  this->Modified();
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetInputPoints(vtkPoints* points)
{
  vtkNew<vtkPolyData> polyData;
  polyData->SetPoints(points);
  this->SetInputData(polyData);
}

//------------------------------------------------------------------------------
vtkPoints* vtkCurveGenerator::GetOutputPoints()
{
  vtkPolyData* polyData = this->GetOutput();
  if (!polyData)
    {
    return nullptr;
    }
  return polyData->GetPoints();
}
