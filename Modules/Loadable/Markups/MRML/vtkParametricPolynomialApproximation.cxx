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

#include "vtkParametricPolynomialApproximation.h"

#include <vtkDoubleArray.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkSortDataArray.h>
#include <vtkTimeStamp.h>

#include <vector>
#include <set>

vtkStandardNewMacro(vtkParametricPolynomialApproximation);

double EPSILON = 0.001;

//----------------------------------------------------------------------------
vtkParametricPolynomialApproximation::vtkParametricPolynomialApproximation()
{
  this->MinimumU = 0;
  this->MaximumU = 1.0;
  this->JoinU = 0;

  this->FitMethod = vtkParametricPolynomialApproximation::FIT_METHOD_GLOBAL_LEAST_SQUARES;

  this->Points = nullptr;
  this->Parameters = nullptr;
  this->PolynomialOrder = 1;
  this->SamplePosition = 0.0;

  this->Weights = nullptr;
  this->SortedParameters = nullptr;
  this->WeightFunction = vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_GAUSSIAN;
  this->SampleWidth = 0.5;
  this->SafeSampleWidth = 0.0; // recomputed as needed by this class
  this->SafeHalfSampleWidthComputedTime.Modified();

  this->Coefficients = nullptr;
}

//----------------------------------------------------------------------------
vtkParametricPolynomialApproximation::~vtkParametricPolynomialApproximation()  = default;

//----------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::SetPoints(vtkPoints* points)
{
  this->Points = points;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::SetParameters(vtkDoubleArray* array)
{
  this->Parameters = array;
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::Evaluate(double u[3], double outputPoint[3], double*)
{
  // Set default value
  outputPoint[0] = outputPoint[1] = outputPoint[2] = 0;

  this->SamplePosition = vtkMath::ClampValue< double >(u[0], 0.0, 1.0);

  // make sure everything has been set up
  if (this->ComputeCoefficientsNeeded())
    {
    this->ComputeCoefficients();
    }

  // error cases, just return
  if (this->Coefficients == nullptr || this->Coefficients->GetNumberOfTuples() == 0)
    {
    vtkErrorMacro("Polynomial coefficients were not computed. Returning without evaluating.");
    return;
    }

  const int numberOfDimensions = 3;
  int numberOfCoefficients = this->Coefficients->GetNumberOfComponents();
  for (int dimensionIndex = 0; dimensionIndex < numberOfDimensions; dimensionIndex++)
    {
    for (int coefficientIndex = 0; coefficientIndex < numberOfCoefficients; coefficientIndex++)
      {
      double coefficient = this->Coefficients->GetComponent(dimensionIndex, coefficientIndex);
      double samplePositionToExponent = std::pow(this->SamplePosition, coefficientIndex);
      outputPoint[dimensionIndex] += coefficient * samplePositionToExponent;
      }
    }
}

//----------------------------------------------------------------------------
double vtkParametricPolynomialApproximation::EvaluateScalar(double u[3], double*, double*)
{
  return u[0];
}

//----------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::ComputeCoefficients()
{
  this->Coefficients = nullptr; // this indicates that the coefficients have not been computed (yet)

  if (this->Points == nullptr || this->Points->GetNumberOfPoints() == 0)
    {
    vtkErrorMacro("Points are missing. Cannot compute coefficients.");
    return;
    }

  if (this->Parameters == nullptr || this->Parameters->GetNumberOfTuples() == 0)
    {
    vtkErrorMacro("Parameters are missing. Cannot compute coefficients.");
    return;
    }

  int numberOfPoints = this->Points->GetNumberOfPoints();
  int numberOfParameters = this->Parameters->GetNumberOfTuples();
  if (numberOfPoints != numberOfParameters)
    {
    vtkErrorMacro("Need equal number of parameters and points. Got "
      << numberOfParameters << " and " << numberOfPoints << ", respectively. Cannot compute coefficients.");
    return;
    }

  if (this->ComputeWeightsNeeded())
    {
    this->ComputeWeights();
    }

  this->Coefficients = vtkSmartPointer< vtkDoubleArray >::New();
  this->FitLeastSquaresPolynomials(this->Parameters, this->Points, this->Weights, this->PolynomialOrder, this->Coefficients);
}

//------------------------------------------------------------------------------
bool vtkParametricPolynomialApproximation::ComputeCoefficientsNeeded()
{
  // Assume that if anything is null, then the user intends for everything to be computed.
  // In normal use, none of these should be null
  if (this->Coefficients == nullptr || this->Points == nullptr || this->Parameters == nullptr)
    {
    return true;
    }

  // In moving least squares the weights (and hence coefficients) change
  // depending on the sample position, so this should always be recomputed
  if (this->FitMethod == vtkParametricPolynomialApproximation::FIT_METHOD_MOVING_LEAST_SQUARES)
    {
    return true;
    }

  vtkMTimeType coefficientsModifiedTime = this->Coefficients->GetMTime();
  vtkMTimeType approximatorModifiedTime = this->GetMTime();
  if (approximatorModifiedTime > coefficientsModifiedTime)
    {
    return true;
    }

  vtkMTimeType pointsModifiedTime = this->Points->GetMTime();
  if (pointsModifiedTime > coefficientsModifiedTime)
    {
    return true;
    }

  vtkMTimeType parametersModifiedTime = this->Parameters->GetMTime();
  if (parametersModifiedTime > coefficientsModifiedTime)
    {
    return true;
    }

  // If weights are made modifiable by the user, they will need to be checked here too

  return false;
}

//------------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::ComputeWeights()
{
  if (this->Parameters == nullptr)
    {
    vtkErrorMacro("Parameters are null. cannot compute weights.");
    return;
    }

  if (this->Weights == nullptr)
    {
    this->Weights = vtkSmartPointer< vtkDoubleArray >::New();
    }
  else
    {
    this->Weights->Reset();
    }

  if (this->FitMethod == vtkParametricPolynomialApproximation::FIT_METHOD_GLOBAL_LEAST_SQUARES)
    {
    this->ComputeWeightsGlobalLeastSquares();
    }
  else if (this->FitMethod == vtkParametricPolynomialApproximation::FIT_METHOD_MOVING_LEAST_SQUARES)
    {
    this->ComputeWeightsMovingLeastSquares();
    }
  else
    {
    vtkErrorMacro("Unexpected FitMethod case in ComputeWeights: " << this->FitMethod << ". Cannot compute weights.");
    }
}

//------------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::ComputeWeightsGlobalLeastSquares()
{
  int numberOfParameters = this->Parameters->GetNumberOfTuples();
  for (int parameterIndex = 0; parameterIndex < numberOfParameters; parameterIndex++)
    {
    this->Weights->InsertNextTuple1(1.0);
    }
}

//------------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::ComputeWeightsMovingLeastSquares()
{
  if (this->ComputeSafeSampleWidthNeeded())
    {
    this->ComputeSafeSampleWidth();
    }

  double samplePosition = this->SamplePosition;
  double halfSampleWidth = this->SafeSampleWidth / 2.0;
  double maximumDistanceFromSamplePosition = halfSampleWidth + EPSILON;

  int numberOfParameters = this->Parameters->GetNumberOfTuples();
  for (int parameterIndex = 0; parameterIndex < numberOfParameters; parameterIndex++)
    {
    double weight = 0.0; // assume this value until we know parameterValue (next line) is in range
    double parameterValue = this->Parameters->GetValue(parameterIndex);
    double parameterDistanceFromSamplePosition = abs(parameterValue - samplePosition);
    if (parameterDistanceFromSamplePosition <= maximumDistanceFromSamplePosition)
      {
      switch (this->WeightFunction)
        {
        case vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_RECTANGULAR:
          {
          weight = 1.0;
          break;
          }
        case vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_TRIANGULAR:
          {
          weight = 1.0 - (parameterDistanceFromSamplePosition / halfSampleWidth);
          break;
          }
        case vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_COSINE:
          {
          // map between -PI and PI
          double distanceNormalizedRadians = (parameterDistanceFromSamplePosition / halfSampleWidth) * vtkMath::Pi();
          double cosine = std::cos(distanceNormalizedRadians);
          // remap from -1..1 to 0..1
          weight = cosine / 2.0 + 0.5;
          break;
          }
        case vtkParametricPolynomialApproximation::WEIGHT_FUNCTION_GAUSSIAN:
          {
          // halfSampleWidth represents 3 standard deviations, so 99.7% of the gaussian will be captured
          double stdev = halfSampleWidth / 3.0;
          double variance = stdev * stdev;
          weight = vtkMath::GaussianAmplitude(variance, parameterDistanceFromSamplePosition);
          break;
          }
        default:
          {
          vtkErrorMacro("Unexpected weight function: " << this->WeightFunction << ". Cannot compute weights.");
          return;
          }
        }
      }
    this->Weights->InsertNextTuple1(weight);
    }
}

//------------------------------------------------------------------------------
bool vtkParametricPolynomialApproximation::ComputeWeightsNeeded()
{
  // Assume that if anything is null, then the user intends for everything to be computed.
  // In normal use, none of these should be null
  if (this->Weights == nullptr || this->Parameters == nullptr)
    {
    return true;
    }

  if (this->FitMethod == vtkParametricPolynomialApproximation::FIT_METHOD_MOVING_LEAST_SQUARES)
    {
    return true;
    }

  vtkMTimeType approximatorModifiedTime = this->GetMTime();
  vtkMTimeType weightsModifiedTime = this->Weights->GetMTime();
  if (approximatorModifiedTime > weightsModifiedTime)
    {
    return true;
    }

  vtkMTimeType parametersModifiedTime = this->Parameters->GetMTime();
  if (parametersModifiedTime > weightsModifiedTime)
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::ComputeSortedParameters()
{
  if (this->Parameters == nullptr)
    {
    vtkErrorMacro("Cannot compute sorted parameters. Parameters are null.");
    return;
    }

  if (this->SortedParameters == nullptr)
    {
    this->SortedParameters = vtkSmartPointer< vtkDoubleArray >::New();
    }
  else
    {
    this->SortedParameters->Reset();
    }

  this->SortedParameters->DeepCopy(this->Parameters);
  vtkSortDataArray::Sort(this->SortedParameters);
}

//------------------------------------------------------------------------------
bool vtkParametricPolynomialApproximation::ComputeSortedParametersNeeded()
{
  // Assume that if anything is null, then the user intends for everything to be computed.
  // In normal use, none of these should be null
  if (this->SortedParameters == nullptr || this->Parameters == nullptr)
    {
    return true;
    }

  vtkMTimeType sortedModifiedTime = this->SortedParameters->GetMTime();
  vtkMTimeType originalModifiedTime = this->Parameters->GetMTime();
  if (originalModifiedTime > sortedModifiedTime)
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
// Check to make sure that there are no gaps between parameters larger than sampleWidth,
// otherwise there is a risk of fitting a polynomial with no underlying data (which is bad)
void vtkParametricPolynomialApproximation::ComputeSafeSampleWidth()
{
  if (this->ComputeSortedParametersNeeded())
    {
    this->ComputeSortedParameters();
    }

  if (this->SortedParameters == nullptr || this->SortedParameters->GetNumberOfTuples() == 0)
    {
    vtkErrorMacro("Sorted parameters are null. " <<
      "Cannot compute \"safe\" sample width, will use user-requested sample width " << this->SampleWidth << ".");
    this->SafeSampleWidth = this->SampleWidth;
    return;
    }

  int numberOfParameters = this->Parameters->GetNumberOfTuples();
  int numberOfSortedParameters = this->SortedParameters->GetNumberOfTuples();
  if (numberOfSortedParameters != numberOfParameters)
    {
    vtkErrorMacro("Number of parameters before sorting " << numberOfParameters <<
      " is not the same as after " << numberOfSortedParameters << ". " <<
      "Cannot compute \"safe\" sample width, will use user-requested sample width " << this->SampleWidth << ".");
    this->SafeSampleWidth = this->SampleWidth;
    return;
    }

  double safeSampleWidth = this->SampleWidth;
  for (int parameterIndex = 0; parameterIndex < numberOfParameters - 1; parameterIndex++)
    {
    double firstParameter = this->SortedParameters->GetValue(parameterIndex);
    double secondParameter = this->SortedParameters->GetValue(parameterIndex + 1);
    double widthBetweenParameters = std::abs(secondParameter - firstParameter);
    if (safeSampleWidth < widthBetweenParameters)
      {
      safeSampleWidth = widthBetweenParameters + EPSILON;
      }
    }

  this->SafeSampleWidth = safeSampleWidth;
  this->SafeHalfSampleWidthComputedTime.Modified();
  if (this->SampleWidth != this->SafeSampleWidth)
    {
    vtkWarningMacro("Reqested sample width " << this->SampleWidth << " is too small and will result in singularities. " <<
      "Computations will use the smallest \"safe\" value " << this->SafeSampleWidth << " instead.");
    }
}

//------------------------------------------------------------------------------
bool vtkParametricPolynomialApproximation::ComputeSafeSampleWidthNeeded()
{
  // Assume that if parameters are null, then the user intends for everything to be computed.
  // In normal use, none of these should be null
  if (this->Parameters == nullptr)
    {
    return true;
    }

  vtkMTimeType safeSampleModifiedTime = this->SafeHalfSampleWidthComputedTime.GetMTime();
  vtkMTimeType parametersModifiedTime = this->Parameters->GetMTime();
  if (safeSampleModifiedTime < parametersModifiedTime)
    {
    return true;
    }

  vtkMTimeType approximatorModifiedTime = this->GetMTime(); // last change by user
  if (safeSampleModifiedTime < approximatorModifiedTime)
    {
    return true;
    }

  return false;
}

//------------------------------------------------------------------------------
// This function formats the data so it works with vtkMath::SolveLeastSquares.
void vtkParametricPolynomialApproximation::FitLeastSquaresPolynomials(
  vtkDoubleArray* parameters, vtkPoints* points, vtkDoubleArray* weights, int polynomialOrder, vtkDoubleArray* coefficients)
{
  // error checking
  if (parameters == nullptr)
    {
    vtkGenericWarningMacro("Parameters are null. Aborting least squares fit.");
    return;
    }

  if (points == nullptr)
    {
    vtkGenericWarningMacro("Points are null. Aborting least squares fit.");
    return;
    }

  if (weights == nullptr)
    {
    vtkGenericWarningMacro("Weights are null. Aborting least squares fit.");
    return;
    }

  int numberOfPoints = points->GetNumberOfPoints();
  int numberOfParameters = parameters->GetNumberOfTuples();
  if (numberOfPoints != numberOfParameters)
    {
    vtkGenericWarningMacro("The number of points " << numberOfPoints << " does not match number of parameters "
      << numberOfParameters << ". Aborting least squares fit.");
    return;
    }

  int numberOfWeights = weights->GetNumberOfTuples();
  if (numberOfPoints != numberOfWeights)
    {
    vtkGenericWarningMacro("The number of points " << numberOfPoints << " does not match number of weights "
      << numberOfWeights << ". Aborting least squares fit.");
    return;
    }

  // The system of equations using high-order polynomials is not well-conditioned.
  // The vtkMath implementation will usually abort with polynomial orders higher than 9.
  // Since there is also numerical instability, we decide to limit the polynomial order to 6.
  // If order higher than 6 is needed on a global fit, then another algorithm should be considered anyway.
  // If at some point we want to add support for higher order polynomials, then here are two options (from Andras):
  // 1. VNL. While the VNL code is more sophisticated, and I guess also more stable, you would probably need to
  //    limit the number of samples and normalize data that you pass to the LSQR solver to be able to compute
  //    higher-order fits (see for example this page for related discussion:
  //    http://digital.ni.com/public.nsf/allkb/45C2016C23B3B0298525645F0073B828).
  //    See an example how VNL is used in Plus:
  //    https://github.com/PlusToolkit/PlusLib/blob/master/src/PlusCommon/PlusMath.cxx#L111
  // 2. Mathematica uses different basis functions for polynomial fitting (shifted Chebyshev polynomials) instead
  //    of basis functions that are simple powers of a variable to make the fitting more robust (the source code
  //    is available here: http://library.wolfram.com/infocenter/MathSource/6780/).
  const int maximumPolynomialOrder = 6;
  if (polynomialOrder > maximumPolynomialOrder)
    {
    vtkGenericWarningMacro("Desired polynomial order " << polynomialOrder << " is not supported. "
      << "Maximum supported order is " << maximumPolynomialOrder << ". "
      << "Will attempt to create polynomial order " << maximumPolynomialOrder << " instead.");
    polynomialOrder = maximumPolynomialOrder;
    }
  const int minimumPolynomialOrder = 0; // It's a pretty weird input, but it does work. Just creates an average.
  if (polynomialOrder < minimumPolynomialOrder)
    {
    vtkGenericWarningMacro("Desired polynomial order " << polynomialOrder << " is not supported. "
      << "Minimum supported order is " << minimumPolynomialOrder << ". "
      << "Will attempt to create constant average instead.");
    polynomialOrder = minimumPolynomialOrder;
    }

  // determine number of coefficients for this polynomial
  std::set< double > uniqueParameters;
  for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
    double weight = weights->GetValue(pointIndex);
    if (weight > EPSILON)
      {
      double parameterValue = parameters->GetValue(pointIndex);
      uniqueParameters.insert(parameterValue); // set cannot contain duplicates
      }
    }
  int numberOfUniqueParameters = uniqueParameters.size();
  int numberOfCoefficients = polynomialOrder + 1;
  if (numberOfUniqueParameters < numberOfCoefficients)
    {
    // must reduce the order of polynomial according to the amount of information is available
    numberOfCoefficients = numberOfUniqueParameters;
    }

  // independent values (parameter along the curve)
  int numIndependentValues = numberOfPoints * numberOfCoefficients;
  std::vector< double > independentValues(numIndependentValues, 0.0);
  for (int coefficientIndex = 0; coefficientIndex < numberOfCoefficients; coefficientIndex++)
    {
    for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
      {
      double parameterValue = parameters->GetValue(pointIndex);
      double weight = weights->GetValue(pointIndex);
      double independentValue = std::pow(parameterValue, coefficientIndex) * weight;
      independentValues[pointIndex * numberOfCoefficients + coefficientIndex] = independentValue;
      }
    }
  std::vector< double* > independentMatrix(numberOfPoints, nullptr);
  for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
    independentMatrix[pointIndex] = &(independentValues[pointIndex * numberOfCoefficients]);
    }
  double** independentMatrixPtr = &(independentMatrix[0]);

  // dependent values
  const int numberOfDimensions = 3;
  int numDependentValues = numberOfPoints * numberOfDimensions;
  std::vector< double > dependentValues(numDependentValues, 0.0);
  for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
    double* currentPoint = points->GetPoint(pointIndex);
    double weight = weights->GetValue(pointIndex);
    for (int dimensionIndex = 0; dimensionIndex < numberOfDimensions; dimensionIndex++)
      {
      double value = currentPoint[dimensionIndex] * weight;
      dependentValues[pointIndex * numberOfDimensions + dimensionIndex] = value;
      }
    }
  std::vector< double* > dependentMatrix(numberOfPoints, nullptr);
  for (int pointIndex = 0; pointIndex < numberOfPoints; pointIndex++)
    {
    dependentMatrix[pointIndex] = &(dependentValues[pointIndex * numberOfDimensions]);
    }
  double** dependentMatrixPtr = &(dependentMatrix[0]);

  // solution to least squares
  std::vector< double > coefficientValues(numberOfDimensions * numberOfCoefficients, 0.0);
  std::vector< double* > coefficientMatrix(numberOfCoefficients, nullptr);
  for (int coefficientIndex = 0; coefficientIndex < numberOfCoefficients; coefficientIndex++)
    {
    coefficientMatrix[coefficientIndex] = &(coefficientValues[coefficientIndex * numberOfDimensions]);
    }
  double** coefficientMatrixPtr = &(coefficientMatrix[0]); // the solution

  // Input the forumulation into vtkMath::SolveLeastSquares
  vtkMath::SolveLeastSquares(numberOfPoints, independentMatrixPtr, numberOfCoefficients, dependentMatrixPtr, numberOfDimensions, coefficientMatrixPtr);

  // Store result in the appropriate variable
  coefficients->Reset();
  coefficients->SetNumberOfComponents(numberOfCoefficients); // must be set before number of tuples
  coefficients->SetNumberOfTuples(numberOfDimensions);
  for (int dimensionIndex = 0; dimensionIndex < numberOfDimensions; dimensionIndex++)
    {
    for (int coefficientIndex = 0; coefficientIndex < numberOfCoefficients; coefficientIndex++)
      {
      double coefficient = coefficientValues[coefficientIndex * numberOfDimensions + dimensionIndex];
      coefficients->SetComponent(dimensionIndex, coefficientIndex, coefficient);
      }
    }
}

//----------------------------------------------------------------------------
void vtkParametricPolynomialApproximation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  os << indent << "Parameters: ";
  if (this->Parameters != nullptr)
    {
    os << this->Parameters << "\n";
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << "Points: ";
  if (this->Points != nullptr)
    {
    os << this->Points << "\n";
    }
  else
    {
    os << "(none)\n";
    }

  os << indent << this->PolynomialOrder << "\n";

  os << indent << "Coefficients: ";
  if (this->Coefficients != nullptr)
    {
    os << this->Coefficients << "\n";
    }
  else
    {
    os << "(none)\n";
    }
}
