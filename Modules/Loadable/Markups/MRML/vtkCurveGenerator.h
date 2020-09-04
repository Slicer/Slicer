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

#ifndef __vtkCurveGenerator_h
#define __vtkCurveGenerator_h

// vtk includes
#include <vtkParametricFunction.h>
#include <vtkPointLocator.h>
#include <vtkPolyData.h>
#include <vtkPolyDataAlgorithm.h>
#include <vtkSetGet.h>
#include <vtkSmartPointer.h>

class vtkSlicerDijkstraGraphGeodesicPath;
class vtkDoubleArray;
class vtkPoints;
class vtkSpline;

// export
#include "vtkSlicerMarkupsModuleMRMLExport.h"

/// Filter that generates curves between points of an input polydata
class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkCurveGenerator : public vtkPolyDataAlgorithm
{
public:
  vtkTypeMacro(vtkCurveGenerator, vtkPolyDataAlgorithm);
  static vtkCurveGenerator* New();

  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// This indicates whether the curve should loop back in on itself,
  /// connecting the last point back to the first point (disabled by default).
  vtkSetMacro(CurveIsLoop, bool);
  vtkGetMacro(CurveIsLoop, bool);
  vtkBooleanMacro(CurveIsLoop, bool);

  /// Type of curve to generate
  enum
    {
    CURVE_TYPE_LINEAR_SPLINE = 0, // Curve interpolates between input points with straight lines
    CURVE_TYPE_CARDINAL_SPLINE, // Curve interpolates between input points smoothly
    CURVE_TYPE_KOCHANEK_SPLINE, // Curve interpolates between input points smoothly, generalized
    CURVE_TYPE_POLYNOMIAL, // Curve approximates the input points with a polynomial fit
    CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE, // Curve finds the closest path along a the edges of a surface mesh
    CURVE_TYPE_LAST // Valid types go above this line
    };
  vtkGetMacro(CurveType, int);
  vtkSetMacro(CurveType, int);
  static const char* GetCurveTypeAsString(int id);
  static int GetCurveTypeFromString(const char* name);
  void SetCurveTypeToLinearSpline() { this->SetCurveType(CURVE_TYPE_LINEAR_SPLINE); }
  void SetCurveTypeToCardinalSpline() { this->SetCurveType(CURVE_TYPE_CARDINAL_SPLINE); }
  void SetCurveTypeToKochanekSpline() { this->SetCurveType(CURVE_TYPE_KOCHANEK_SPLINE); }
  void SetCurveTypeToPolynomial() { this->SetCurveType(CURVE_TYPE_POLYNOMIAL); }
  void SetCurveTypeToShortestDistanceOnSurface() { this->SetCurveType(CURVE_TYPE_SHORTEST_DISTANCE_ON_SURFACE); }

  virtual bool IsInterpolatingCurve();

  /// Sample an *interpolating* curve this many times per segment (pair of points in sequence). Range 1 and up. Default 5.
  vtkSetMacro(NumberOfPointsPerInterpolatingSegment, int);
  vtkGetMacro(NumberOfPointsPerInterpolatingSegment, int);

  /// Bias of derivative toward previous point (negative value) or next point. Range -1 to 1. Default 0.
  vtkGetMacro(KochanekBias, double);
  vtkSetMacro(KochanekBias, double);

  /// Make the curve sharper( negative value) or smoother (positive value). Range -1 to 1. Default 0.
  vtkGetMacro(KochanekContinuity, double);
  vtkSetMacro(KochanekContinuity, double);

  /// How quickly the curve turns, higher values like tightening an elastic. Range -1 to 1. Default 0.
  vtkGetMacro(KochanekTension, double);
  vtkSetMacro(KochanekTension, double);

  /// Make the ends of the curve 'straighter' by copying derivative of the nearest point. Default false.
  vtkGetMacro(KochanekEndsCopyNearestDerivatives, bool);
  vtkSetMacro(KochanekEndsCopyNearestDerivatives, bool);

  /// Set the order of the polynomials for fitting. Range 1 to 9 (equation becomes unstable from 9 upward). Default 1.
  vtkGetMacro(PolynomialOrder, int);
  vtkSetMacro(PolynomialOrder, int);

  // Wednesday May 9, 2018 TODO
  // InputParameters is currently computed by this class depending on the
  // value of PolynomialPointSortingMethod, and is only supported for polynomials.
  // In the future this could be expanded to support splines, and to allow
  // the user to specify their own parameters (make a SetInputParameters function)
  // e.g. through functions below
  // Set the parameter values (e.g. point distances) that the curve should be based on
  //virtual void SetInputParameters( vtkDoubleArray* );
  //virtual vtkDoubleArray* GetInputParameters();

  /// Set the sorting method for points in a polynomial.
  enum
    {
    SORTING_METHOD_INDEX = 0,
    SORTING_METHOD_MINIMUM_SPANNING_TREE_POSITION,
    SORTING_METHOD_LAST // valid types should be written above this line
    };
  vtkGetMacro(PolynomialPointSortingMethod, int);
  vtkSetMacro(PolynomialPointSortingMethod, int);
  static const char* GetPolynomialPointSortingMethodAsString(int id);
  static int GetPolynomialPointSortingMethodFromString(const char* name);
  void SetPolynomialPointSortingMethodToIndex() { this->SetPolynomialPointSortingMethod(vtkCurveGenerator::SORTING_METHOD_INDEX); }
  void SetPolynomialPointSortingMethodToMinimumSpanningTreePosition()
    {
    this->SetPolynomialPointSortingMethod(vtkCurveGenerator::SORTING_METHOD_MINIMUM_SPANNING_TREE_POSITION);
    }

  /// Set the type of fit for polynomials
  /// see corresponding entries in vtkParametricPolynomialApproximation.h for more information
  enum
    {
    POLYNOMIAL_FIT_METHOD_GLOBAL_LEAST_SQUARES = 0,
    POLYNOMIAL_FIT_METHOD_MOVING_LEAST_SQUARES,
    POLYNOMIAL_FIT_METHOD_LAST // Valid types go above this line
    };
  vtkGetMacro(PolynomialFitMethod, double);
  vtkSetMacro(PolynomialFitMethod, double);
  static const char* GetPolynomialFitMethodAsString(int id);
  static int GetPolynomialFitMethodFromString(const char* name);
  void SetPolynomialFitMethodToGlobalLeastSquares() { this->SetPolynomialFitMethod(vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_GLOBAL_LEAST_SQUARES); }
  void SetPolynomialFitMethodToMovingLeastSquares() { this->SetPolynomialFitMethod(vtkCurveGenerator::POLYNOMIAL_FIT_METHOD_MOVING_LEAST_SQUARES); }

  /// Set the sampling distance (in parameter space) for moving least squares sampling
  vtkGetMacro(PolynomialSampleWidth, double);
  vtkSetMacro(PolynomialSampleWidth, double);

  /// Set the weight function for moving least squares polynomial fits
  /// see corresponding entries in vtkParametricPolynomialApproximation.h for more information
  enum
    {
    POLYNOMIAL_WEIGHT_FUNCTION_RECTANGULAR = 0,
    POLYNOMIAL_WEIGHT_FUNCTION_TRIANGULAR,
    POLYNOMIAL_WEIGHT_FUNCTION_COSINE,
    POLYNOMIAL_WEIGHT_FUNCTION_GAUSSIAN,
    POLYNOMIAL_WEIGHT_FUNCTION_LAST // Valid types go above this line
    };
  vtkGetMacro(PolynomialWeightFunction, double);
  vtkSetMacro(PolynomialWeightFunction, double);
  static const char* GetPolynomialWeightFunctionAsString(int id);
  static int GetPolynomialWeightFunctionFromString(const char* name);
  void SetPolynomialWeightFunctionToRectangular() { this->SetPolynomialWeightFunction(vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_RECTANGULAR); }
  void SetPolynomialWeightFunctionToTriangular() { this->SetPolynomialWeightFunction(vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_TRIANGULAR); }
  void SetPolynomialWeightFunctionToCosine() { this->SetPolynomialWeightFunction(vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_COSINE); }
  void SetPolynomialWeightFunctionToGaussian() { this->SetPolynomialWeightFunction(vtkCurveGenerator::POLYNOMIAL_WEIGHT_FUNCTION_GAUSSIAN); }

  /// If the surface scalars should be used to weight the distances in the pathfinding algorithm
  int GetSurfaceCostFunctionType();
  void SetSurfaceCostFunctionType(int surfaceCostFunctionType);

  /// Get the control point id from the interpolated point id
  /// Currently only works for shortest surface distance
  vtkIdType GetControlPointIdFromInterpolatedPointId(vtkIdType interpolatedPointId);

  /// Get the list of curve point ids on the surface mesh
  vtkIdList* GetSurfacePointIds();

  /// Get the length of the curve
  double GetOutputCurveLength();

  /// The internal instance of the current parametric function use of the curve for other computations.
  vtkParametricFunction* GetParametricFunction() { return this->ParametricFunction.GetPointer(); };

  /// Set the input control points
  void SetInputPoints(vtkPoints* points);

  /// Get the output sampled points
  vtkPoints* GetOutputPoints();

  /// Calculates point parameters for use in vtkParametricPolynomialApproximation
  /// The parameter values are based on the point index and range from 0.0 at the start to 1.0 at the end of the line.
  /// \sa SortByMinimumSpanningTreePosition
  /// \param inputPoints Input list of points. The points form a continuous line from the first to last point.
  /// \param outputParameters Parameters used by vtkParametricPolynomialApproximation to approximate a polynomial from the input points.
  static void SortByIndex(vtkPoints* inputPoints, vtkDoubleArray* outputParameters);

  /// Calculates point parameters for use in vtkParametricPolynomialApproximation
  /// The parameter values are calculated using the following algorithm:
  /// 1. Construct an undirected graph as a 2D array
  /// 2. Find the two vertices that are the farthest apart
  /// 3. Run prim's algorithm on the graph
  /// 4. Extract the "trunk" path from the last vertex to the first
  /// 5. Based on the distance along that path, assign each vertex a polynomial parameter value
  /// \sa SortByIndex
  /// \param inputPoints Input point cloud that should be sorted to form a continuous line.
  /// \param outputParameters Parameters used by vtkParametricPolynomialApproximation to approximate a polynomial from the input points.
  static void SortByMinimumSpanningTreePosition(vtkPoints* inputPoints, vtkDoubleArray* outputParameters);

protected:
  // input parameters
  int NumberOfPointsPerInterpolatingSegment;
  int CurveType;
  bool CurveIsLoop;
  double KochanekBias;
  double KochanekContinuity;
  double KochanekTension;
  bool KochanekEndsCopyNearestDerivatives;
  int PolynomialOrder;
  int PolynomialPointSortingMethod;
  int PolynomialFitMethod;
  double PolynomialSampleWidth;
  int PolynomialWeightFunction;
  std::vector<vtkIdType> InterpolatedPointIdsForControlPoints;

  // internal storage
  vtkSmartPointer<vtkPointLocator> SurfacePointLocator;
  vtkSmartPointer<vtkSlicerDijkstraGraphGeodesicPath> SurfacePathFilter;
  vtkSmartPointer<vtkDoubleArray> InputParameters;
  vtkSmartPointer<vtkParametricFunction> ParametricFunction;

  // output
  double OutputCurveLength;

  // logic
  void SetParametricFunctionToSpline(vtkPoints* inputPoints, vtkSpline* xSpline, vtkSpline* ySpline, vtkSpline* zSpline);
  void SetParametricFunctionToLinearSpline(vtkPoints* inputPoints);
  void SetParametricFunctionToCardinalSpline(vtkPoints* inputPoints);
  void SetParametricFunctionToKochanekSpline(vtkPoints* inputPoints);
  void SetParametricFunctionToPolynomial(vtkPoints* inputPoints);
  int GeneratePoints(vtkPoints* inputPoints, vtkPolyData* inputSurface, vtkPolyData* outputPolyData);
  int GeneratePointsFromFunction(vtkPoints* inputPoints, vtkPoints* outputPoints);
  int GeneratePointsFromSurface(vtkPoints* inputPoints, vtkPolyData* inputSurface, vtkPoints* outputPoints);
  int GenerateLines(vtkPolyData* polyData);

  int FillInputPortInformation(int port, vtkInformation* info) override;
  int RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector) override;

protected:
  vtkCurveGenerator();
  ~vtkCurveGenerator() override;
  vtkCurveGenerator(const vtkCurveGenerator&) = delete;
  void operator=(const vtkCurveGenerator&) = delete;
};

#endif
