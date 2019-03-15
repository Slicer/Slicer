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

/**
 * @class   vtkParametricPolynomialApproximation
 * @brief   parametric function for 1D polynomials
 *
 * vtkParametricPolynomialApproximation is a parametric function for 1D approximating
 * polynomials. vtkParametricPolynomialApproximation maps the single parameter u to a
 * 3D point (x,y,z). Internally a polynomial is fit to a set of input
 * points using the least squares basis.
*/

#ifndef vtkParametricPolynomialApproximation_h
#define vtkParametricPolynomialApproximation_h

class vtkPoints;
class vtkDoubleArray;
class vtkTimeStamp;

#include "vtkSlicerMarkupsModuleMRMLExport.h" // For export macro

#include <vtkParametricFunction.h>
#include <vtkSmartPointer.h>

class VTK_SLICER_MARKUPS_MODULE_MRML_EXPORT vtkParametricPolynomialApproximation : public vtkParametricFunction
{
public:
  vtkTypeMacro(vtkParametricPolynomialApproximation, vtkParametricFunction);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  static vtkParametricPolynomialApproximation *New();

  /**
   * Return the parametric dimension of the class.
   */
  int GetDimension() override { return 1; }

  /**
   * Evaluate the parametric function at parametric coordinate u[0] returning
   * the point coordinate Pt[3].
   */
  void Evaluate(double u[3], double Pt[3], double Du[9]) override;

  /**
   * Evaluate a scalar value at parametric coordinate u[0] and Pt[3].
   * Simply returns the parameter u[0].
   */
  double EvaluateScalar(double u[3], double Pt[3], double Du[9]) override;

  //@{
  /**
   * Specify the order of polynomial (maximum exponent) that should be fit.
   */
  vtkGetMacro(PolynomialOrder, int);
  vtkSetMacro(PolynomialOrder, int);
  //@}

  //@{
  /**
   * Specify the list of points that the polynomial should approximate.
   * Set the point parameters that should be used during fitting with SetParameters.
   */
  void SetPoints(vtkPoints*);
  //@}

  //@{
  /**
   * Specify the parameters for the points. Length of list should be the same,
   * and the points should be in the same order as the parameters.
   */
  void SetParameters(vtkDoubleArray*);
  //@}

  //@{
  /**
   * Fitting options, see FitMethod
   */
  enum {
    FIT_METHOD_GLOBAL_LEAST_SQUARES = 0, // global fit
    FIT_METHOD_MOVING_LEAST_SQUARES, // local fit
    FIT_METHOD_LAST // valid types go above this line
  };
  //@}

  //@{
  /**
   * What type of fit should be used (e.g. global/local)
   */
  vtkGetMacro(FitMethod, int);
  vtkSetMacro(FitMethod, int);
  void SetFitMethodToGlobalLeastSquares() { this->SetFitMethod(vtkParametricPolynomialApproximation::FIT_METHOD_GLOBAL_LEAST_SQUARES); }
  void SetFitMethodToMovingLeastSquares() { this->SetFitMethod(vtkParametricPolynomialApproximation::FIT_METHOD_MOVING_LEAST_SQUARES); }
  //@}

  //@{
  /**
   * Options for weighing points in moving least squares, see WeightFunction
   */
  enum {
    WEIGHT_FUNCTION_RECTANGULAR = 0, // All points within the sampling width (see above) are treated with equal importance
    WEIGHT_FUNCTION_TRIANGULAR, // Points closer are treated as more important, falloff is linear
    WEIGHT_FUNCTION_COSINE, // Points closer are treated as more important, falloff curve follows a smooth cosine
    WEIGHT_FUNCTION_GAUSSIAN, // Points closer are treated as more important, falloff curve follows a smooth gaussian with 5% cutoff
    WEIGHT_FUNCTION_LAST // valid types go above this line
  };
  //@}

  //@{
  /**
   * Specify the order of polynomial (maximum exponent) that should be fit.
   */
  vtkGetMacro(WeightFunction, int);
  vtkSetMacro(WeightFunction, int);
  void SetWeightFunctionToRectangular() { this->SetWeightFunction(vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_RECTANGULAR); }
  void SetWeightFunctionToTriangular() { this->SetWeightFunction(vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_TRIANGULAR); }
  void SetWeightFunctionToCosine() { this->SetWeightFunction(vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_COSINE); }
  void SetWeightFunctionToGaussian() { this->SetWeightFunction(vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_GAUSSIAN); }
  //@}

  //@{
  /**
   * The width of sampling for moving least squares (in parameter space)
   * This is total width, so the space that is sampled is
   * SamplePosition - ( SampleWidth / 2 ) through to
   * SamplePosition + ( SampleWidth / 2 ).
   * Valid range is from 0.0 to 1.0. Default is 0.5.
   */
  vtkGetMacro(SampleWidth, double);
  vtkSetMacro(SampleWidth, double);
  //@}

protected:
  vtkParametricPolynomialApproximation();
  ~vtkParametricPolynomialApproximation() override;

private:
  // What kind of fit (global/local)
  int FitMethod;

  // These apply to all types of polynomials
  int PolynomialOrder;
  vtkSmartPointer< vtkDoubleArray > Parameters;
  vtkSmartPointer< vtkPoints > Points;
  double SamplePosition; // Internally store the sample position queried by the user

  // These apply only to moving least squares (for now)
  vtkSmartPointer< vtkDoubleArray > Weights; // Internally compute and store weights
  vtkSmartPointer< vtkDoubleArray > SortedParameters; // Internally computed, same values as Parameters but sorted
  int WeightFunction;
  double SampleWidth;
  double SafeSampleWidth; // internally computed, ensures that moving least squares polynomials are always computed in parameters 0..1
  vtkTimeStamp SafeHalfSampleWidthComputedTime;

  // This is directly used to evaluate points in 3D (in a sense this is the "output")
  vtkSmartPointer< vtkDoubleArray > Coefficients;

  // Logic functions
  void ComputeCoefficients();
  bool ComputeCoefficientsNeeded();
  void ComputeWeights();
  void ComputeWeightsGlobalLeastSquares();
  void ComputeWeightsMovingLeastSquares();
  bool ComputeWeightsNeeded();
  void ComputeSortedParameters();
  bool ComputeSortedParametersNeeded();
  void ComputeSafeSampleWidth();
  bool ComputeSafeSampleWidthNeeded();
  static void FitLeastSquaresPolynomials(vtkDoubleArray* parameters, vtkPoints* points,
    vtkDoubleArray* weights, int polynomialOrder, vtkDoubleArray* coefficients);

  vtkParametricPolynomialApproximation(const vtkParametricPolynomialApproximation&) = delete;
  void operator=(const vtkParametricPolynomialApproximation&) = delete;
};

#endif
