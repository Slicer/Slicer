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

#include "vtkLinearSpline.h"

#include <vtkObjectFactory.h>
#include <vtkPiecewiseFunction.h>
#include <cassert>
#include <vector>

vtkStandardNewMacro(vtkLinearSpline);

//----------------------------------------------------------------------------
// Construct a Linear Spline.
vtkLinearSpline::vtkLinearSpline()
{
  // Improce performance of point insertion (we never insert points
  // at the same parametric position anyway).
  this->PiecewiseFunction->AllowDuplicateScalarsOn();
}

//----------------------------------------------------------------------------
// Evaluate a 1D Spline
// Note that this function is essentially a clone of implementation in
// vtkCardinalSpline
double vtkLinearSpline::Evaluate(double t)
{
  // check to see if we need to recompute the spline
  if (this->ComputeTime < this->GetMTime())
   {
    this->Compute();
    }

  // make sure we have at least 2 points
  int size = this->PiecewiseFunction->GetSize();
  if (size == 0)
    {
    return 0.0;
    }
  else if (size == 1)
    {
    double node[4];
    this->PiecewiseFunction->GetNodeValue(0, node);
    return node[1]; // dependent value
    }

  if (this->Closed)
    {
    size = size + 1;
    }

  // clamp the function at both ends
  if (t < this->Intervals[0])
    {
    t = this->Intervals[0];
    }
  if (t > this->Intervals[size - 1])
    {
    t = this->Intervals[size - 1];
    }

  // find pointer to cubic spline coefficient using bisection method
  int index = this->FindIndex(size, t);

  // calculate offset within interval
  t = (t - this->Intervals[index]);

  // evaluate function value
  double t1Coefficient = this->Coefficients[index * 2];
  double t0Coefficient = this->Coefficients[index * 2 + 1];
  return (t * t1Coefficient + t0Coefficient);
}

//----------------------------------------------------------------------------
// Compute linear splines for each dependent variable
// Note that in linear splines the derivatives at each sample (t,x) is ignored
// LeftConstraint, RightConstraint, LeftValue, and RightValue have no effect
void vtkLinearSpline::Compute()
{
  // how many input points?
  int numberOfInputPoints = this->PiecewiseFunction->GetSize();

  if (numberOfInputPoints < 2)
    {
    vtkErrorMacro("Cannot compute a spline with less than 2 points. # of points is: " << numberOfInputPoints);
    return;
    }

  // this->Closed determines how many points to interpolate between
  if (this->Closed)
    {
    delete[] this->Intervals;
    this->Intervals = new double[numberOfInputPoints + 1];
    std::vector< double > values = std::vector< double >(numberOfInputPoints + 1);
    for (int pointIndex = 0; pointIndex < numberOfInputPoints; pointIndex++)
      {
      double node[4];
      this->PiecewiseFunction->GetNodeValue(pointIndex, node);
      this->Intervals[pointIndex] = node[0]; // independent value
      values[pointIndex] = node[1]; // dependant value
      }

    // last point interval
    if (this->ParametricRange[0] != this->ParametricRange[1]) // has user specified last range?
      {
      this->Intervals[numberOfInputPoints] = this->ParametricRange[1];
      }
    else // use default behaviour for vtkSpline by adding 1.0 to last value
      {
      this->Intervals[numberOfInputPoints] = this->Intervals[numberOfInputPoints - 1] + 1.0;
      }
    // last point value
    double nextValue = values[0];
    values[numberOfInputPoints] = nextValue;

    // compute coefficients
    delete[] this->Coefficients;
    int numberOfSegments = numberOfInputPoints;
    this->Coefficients = new double[2 * numberOfSegments];
    for (int segmentIndex = 0; segmentIndex < numberOfSegments; segmentIndex++)
      {
      double intervalWidth = this->Intervals[segmentIndex + 1] - this->Intervals[segmentIndex];
      double changeInValue = values[segmentIndex + 1] - values[segmentIndex];
      this->Coefficients[segmentIndex * 2] = changeInValue / intervalWidth;
      this->Coefficients[segmentIndex * 2 + 1] = values[segmentIndex];
      }
    }
  else
    {
    delete[] this->Intervals;
    this->Intervals = new double[numberOfInputPoints];
    std::vector< double > values = std::vector< double >(numberOfInputPoints);
    for (int pointIndex = 0; pointIndex < numberOfInputPoints; pointIndex++)
      {
      double node[4];
      this->PiecewiseFunction->GetNodeValue(pointIndex, node);
      this->Intervals[pointIndex] = node[0]; // independent value
      values[pointIndex] = node[1]; // dependant value
      }

    // compute coefficients
    delete[] this->Coefficients;
    int numberOfSegments = numberOfInputPoints - 1;
    this->Coefficients = new double[2 * numberOfSegments];
    for (int segmentIndex = 0; segmentIndex < numberOfSegments; segmentIndex++)
      {
      double intervalWidth = this->Intervals[segmentIndex + 1] - this->Intervals[segmentIndex];
      double changeInValue = values[segmentIndex + 1] - values[segmentIndex];
      this->Coefficients[segmentIndex * 2] = changeInValue / intervalWidth;
      this->Coefficients[segmentIndex * 2 + 1] = values[segmentIndex];
      }
    }

  // update compute time
  this->ComputeTime = this->GetMTime();
}

//----------------------------------------------------------------------------
void vtkLinearSpline::DeepCopy(vtkSpline *s)
{
  vtkLinearSpline *spline = vtkLinearSpline::SafeDownCast(s);
  if (spline == nullptr)
    {
    vtkWarningMacro("Cannot deep copy contents into spline - not of matching type.");
    return;
    }

  this->vtkSpline::DeepCopy(s);
}

//----------------------------------------------------------------------------
void vtkLinearSpline::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
