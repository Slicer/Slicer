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

#include "vtkCurveGenerator.h"
#include "vtkLinearSpline.h"

#include <vtkCardinalSpline.h>
#include <vtkDoubleArray.h>
#include <vtkKochanekSpline.h>
#include <vtkParametricSpline.h>
#include <vtkParametricFunction.h>
#include "vtkParametricPolynomialApproximation.h"
#include <vtkPoints.h>

#include <list>

//------------------------------------------------------------------------------
vtkStandardNewMacro(vtkCurveGenerator);

//------------------------------------------------------------------------------
vtkCurveGenerator::vtkCurveGenerator()
{
  this->InputPoints = nullptr;
  this->InputParameters = nullptr;
  this->SetCurveTypeToLinearSpline();
  this->CurveIsLoop = false;
  this->NumberOfPointsPerInterpolatingSegment = 5;
  this->KochanekBias = 0.0;
  this->KochanekContinuity = 0.0;
  this->KochanekTension = 0.0;
  this->KochanekEndsCopyNearestDerivatives = false;
  this->PolynomialOrder = 1; // linear
  this->PolynomialPointSortingMethod = vtkCurveGenerator::SORTING_METHOD_INDEX;
  this->PolynomialFitMethod = vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_GLOBAL_LEAST_SQUARES;
  this->PolynomialWeightFunction = vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_GAUSSIAN;
  this->PolynomialSampleWidth = 0.5;
  this->OutputPoints = nullptr;
  this->OutputCurveLength = 0.0;

  // timestamps for input and output are the same, initially
  this->Modified();

  // local storage variables
  this->ParametricFunction = nullptr;
}

//------------------------------------------------------------------------------
vtkCurveGenerator::~vtkCurveGenerator()
= default;

//------------------------------------------------------------------------------
void vtkCurveGenerator::PrintSelf(std::ostream &os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "InputPoints size: " << (this->InputPoints != nullptr ? this->InputPoints->GetNumberOfPoints() : 0) << std::endl;
  os << indent << "InputParameters size: " << (this->InputParameters != nullptr ? this->InputParameters->GetNumberOfTuples() : 0) << std::endl;
  os << indent << "CurveType: " << this->GetCurveTypeAsString(this->CurveType) << std::endl;
  os << indent << "CurveIsLoop: " << this->CurveIsLoop << std::endl;
  os << indent << "KochanekBias: " << this->KochanekBias << std::endl;
  os << indent << "KochanekContinuity: " << this->KochanekContinuity << std::endl;
  os << indent << "KochanekTension: " << this->KochanekTension << std::endl;
  os << indent << "KochanekEndsCopyNearestDerivatives: " << this->KochanekEndsCopyNearestDerivatives << std::endl;
  os << indent << "PolynomialOrder: " << this->PolynomialOrder << std::endl;
  os << indent << "OutputPoints size: " << (this->OutputPoints != nullptr ? this->OutputPoints->GetNumberOfPoints() : 0) << std::endl;
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
vtkPoints* vtkCurveGenerator::GetInputPoints()
{
  return this->InputPoints;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetInputPoints(vtkPoints* points)
{
  this->InputPoints = points;
  this->Modified();
}

//------------------------------------------------------------------------------
vtkPoints* vtkCurveGenerator::GetOutputPoints()
{
  if (this->UpdateNeeded())
    {
    this->Update();
    }
  return this->OutputPoints;
}

//------------------------------------------------------------------------------
double vtkCurveGenerator::GetOutputCurveLength()
{
  if (this->UpdateNeeded())
    {
    this->Update();
    }
  return this->OutputCurveLength;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetOutputPoints(vtkPoints* points)
{
  this->OutputPoints = points;
  this->Modified();
}

//------------------------------------------------------------------------------
// LOGIC
//------------------------------------------------------------------------------
void vtkCurveGenerator::Update()
{
  if (this->InputPoints == nullptr)
    {
    vtkWarningMacro("No input points. No curve generation possible.");
    return;
    }

  if (!this->UpdateNeeded())
    {
    return;
    }

  switch (this->CurveType)
    {
    case vtkCurveGenerator::CURVE_TYPE_LINEAR_SPLINE:
      {
      this->SetParametricFunctionToLinearSpline();
      break;
      }
    case vtkCurveGenerator::CURVE_TYPE_CARDINAL_SPLINE:
      {
      this->SetParametricFunctionToCardinalSpline();
      break;
      }
    case vtkCurveGenerator::CURVE_TYPE_KOCHANEK_SPLINE:
      {
      this->SetParametricFunctionToKochanekSpline();
      break;
      }
    case vtkCurveGenerator::CURVE_TYPE_POLYNOMIAL:
      {
      this->SetParametricFunctionToPolynomial();
      break;
      }
    default:
      {
      vtkErrorMacro("Error: Unrecognized curve type: " << this->CurveType << ".");
      break;
      }
    }
  this->GeneratePoints();
}

//------------------------------------------------------------------------------
bool vtkCurveGenerator::UpdateNeeded()
{
  // assume that if any of these is null, then the user intends for everything to be computed
  // in normal use, none of these should be null
  if (this->OutputPoints == nullptr || this->InputPoints == nullptr)
    {
    return true;
    }

  // If this->InputParameters ever become modifiable by the user,
  // then that modified time will need to be checked here too.

  vtkMTimeType outputModifiedTime = this->OutputPoints->GetMTime();
  vtkMTimeType curveGeneratorModifiedTime = this->GetMTime();
  if (curveGeneratorModifiedTime > outputModifiedTime)
    {
    return true;
    }

  vtkMTimeType inputPointsModifiedTime = this->InputPoints->GetMTime();
  if (inputPointsModifiedTime > outputModifiedTime)
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToSpline(vtkSpline* xSpline, vtkSpline* ySpline, vtkSpline* zSpline)
{
  vtkSmartPointer<vtkParametricSpline> parametricSpline = vtkSmartPointer<vtkParametricSpline>::New();
  parametricSpline->SetXSpline(xSpline);
  parametricSpline->SetYSpline(ySpline);
  parametricSpline->SetZSpline(zSpline);
  parametricSpline->SetPoints(this->InputPoints);
  parametricSpline->SetClosed(this->CurveIsLoop);
  parametricSpline->SetParameterizeByLength(false);
  this->ParametricFunction = parametricSpline;
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToLinearSpline()
{
  vtkSmartPointer<vtkLinearSpline> xSpline = vtkSmartPointer<vtkLinearSpline>::New();
  vtkSmartPointer<vtkLinearSpline> ySpline = vtkSmartPointer<vtkLinearSpline>::New();
  vtkSmartPointer<vtkLinearSpline> zSpline = vtkSmartPointer<vtkLinearSpline>::New();
  this->SetParametricFunctionToSpline(xSpline, ySpline, zSpline);
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToCardinalSpline()
{
  vtkSmartPointer<vtkCardinalSpline> xSpline = vtkSmartPointer<vtkCardinalSpline>::New();
  vtkSmartPointer<vtkCardinalSpline> ySpline = vtkSmartPointer<vtkCardinalSpline>::New();
  vtkSmartPointer<vtkCardinalSpline> zSpline = vtkSmartPointer<vtkCardinalSpline>::New();
  this->SetParametricFunctionToSpline(xSpline, ySpline, zSpline);
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToKochanekSpline()
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
    this->InputPoints->GetPoint(0, point0);
    double point1[3];
    this->InputPoints->GetPoint(1, point1);
    xSpline->SetLeftValue(point1[0] - point0[0]);
    ySpline->SetLeftValue(point1[1] - point0[1]);
    zSpline->SetLeftValue(point1[2] - point0[2]);
    // right derivative
    xSpline->SetRightConstraint(1);
    ySpline->SetRightConstraint(1);
    zSpline->SetRightConstraint(1);
    int numberOfInputPoints = this->InputPoints->GetNumberOfPoints();
    double pointNMinus2[3];
    this->InputPoints->GetPoint(numberOfInputPoints - 2, pointNMinus2);
    double pointNMinus1[3];
    this->InputPoints->GetPoint(numberOfInputPoints - 1, pointNMinus1);
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

  this->SetParametricFunctionToSpline(xSpline, ySpline, zSpline);
}

//------------------------------------------------------------------------------
void vtkCurveGenerator::SetParametricFunctionToPolynomial()
{
  vtkSmartPointer< vtkParametricPolynomialApproximation > polynomial = vtkSmartPointer< vtkParametricPolynomialApproximation >::New();
  polynomial->SetPoints(this->InputPoints);
  polynomial->SetPolynomialOrder(this->PolynomialOrder);

  if (this->InputParameters == nullptr)
    {
    this->InputParameters = vtkSmartPointer< vtkDoubleArray >::New();
    }

  if (this->PolynomialPointSortingMethod == vtkCurveGenerator::SORTING_METHOD_INDEX)
    {
    vtkCurveGenerator::SortByIndex(this->InputPoints, this->InputParameters);
    }
  else if (this->PolynomialPointSortingMethod == vtkCurveGenerator::SORTING_METHOD_MINIMUM_SPANNING_TREE_POSITION)
    {
    vtkCurveGenerator::SortByMinimumSpanningTreePosition(this->InputPoints, this->InputParameters);
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
void vtkCurveGenerator::GeneratePoints()
{
  if (this->InputPoints == nullptr)
    {
    vtkErrorMacro("Input points are null, so curve points cannot be generated.");
    return;
    }
  if (this->ParametricFunction == nullptr)
    {
    vtkErrorMacro("Parametric function is null, so curve points cannot be generated.");
    return;
    }

  if (this->OutputPoints == nullptr)
    {
    this->OutputPoints = vtkSmartPointer< vtkPoints >::New();
    }
  else
    {
    this->OutputPoints->Reset();
    }
  this->OutputCurveLength = 0.0;

  int numberOfInputPoints = this->InputPoints->GetNumberOfPoints();
  if (numberOfInputPoints <= 1)
    {
    return;
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
    this->OutputPoints->InsertNextPoint(curvePoint);
    if (pointIndex > 0)
      {
      double segmentLength = sqrt(vtkMath::Distance2BetweenPoints(previousPoint, curvePoint));
      this->OutputCurveLength += segmentLength;
      }
    previousPoint[0] = curvePoint[0];
    previousPoint[1] = curvePoint[1];
    previousPoint[2] = curvePoint[2];
    }
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
