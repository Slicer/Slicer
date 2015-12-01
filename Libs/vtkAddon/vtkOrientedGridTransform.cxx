/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkOrientedGridTransform.h"

#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkNew.h"
#include "vtkObjectFactory.h"

vtkStandardNewMacro(vtkOrientedGridTransform);

vtkCxxSetObjectMacro(vtkOrientedGridTransform,GridDirectionMatrix,vtkMatrix4x4);

//----------------------------------------------------------------------------
vtkOrientedGridTransform::vtkOrientedGridTransform()
{
  this->GridDirectionMatrix = NULL;
  this->GridIndexToOutputTransformMatrixCached = vtkMatrix4x4::New();
  this->OutputToGridIndexTransformMatrixCached = vtkMatrix4x4::New();
}

//----------------------------------------------------------------------------
vtkOrientedGridTransform::~vtkOrientedGridTransform()
{
  this->SetGridDirectionMatrix(NULL);
  if (this->GridIndexToOutputTransformMatrixCached)
    {
    this->GridIndexToOutputTransformMatrixCached->Delete();
    this->GridIndexToOutputTransformMatrixCached = NULL;
    }
  if (this->OutputToGridIndexTransformMatrixCached)
    {
    this->OutputToGridIndexTransformMatrixCached->Delete();
    this->OutputToGridIndexTransformMatrixCached = NULL;
    }
}

//----------------------------------------------------------------------------
void vtkOrientedGridTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "GridDirectionMatrix: " << this->GridDirectionMatrix << "\n";
  if (this->GridDirectionMatrix)
    {
    this->GridDirectionMatrix->PrintSelf(os,indent.GetNextIndent());
    }
}

//------------------------------------------------------------------------
inline void vtkLinearTransformPoint(const double matrix[4][4],
                                    const double in[3], double out[3])
{
  double x =
    matrix[0][0]*in[0]+matrix[0][1]*in[1]+matrix[0][2]*in[2]+matrix[0][3];
  double y =
    matrix[1][0]*in[0]+matrix[1][1]*in[1]+matrix[1][2]*in[2]+matrix[1][3];
  double z =
    matrix[2][0]*in[0]+matrix[2][1]*in[1]+matrix[2][2]*in[2]+matrix[2][3];

  out[0] = x;
  out[1] = y;
  out[2] = z;
}

//------------------------------------------------------------------------
// Simple multiplication of 3x3 matrices: aMat * bMat = cMat
// The only difference compared to Matrix3x3 multiply method is that
// bMat is stored in a 4x4 matrix.
inline void vtkLinearTransformJacobian(double aMat[3][3], double bMat[4][4], double cMat[3][3])
{
  double Accum[3][3];
  for (int i = 0; i < 3; i++)
    {
    for (int k = 0; k < 3; k++)
      {
      Accum[i][k] = aMat[i][0] * bMat[0][k] +
                    aMat[i][1] * bMat[1][k] +
                    aMat[i][2] * bMat[2][k];
      }
    }
  // Copy to final dest
  for (int i = 0; i < 3; i++)
    {
    cMat[i][0] = Accum[i][0];
    cMat[i][1] = Accum[i][1];
    cMat[i][2] = Accum[i][2];
    }
}

//----------------------------------------------------------------------------
void vtkOrientedGridTransform::ForwardTransformPoint(const double inPoint[3],
                                             double outPoint[3])
{
  if (this->GridDirectionMatrix == NULL || this->GridPointer == NULL)
    {
    this->Superclass::ForwardTransformPoint(inPoint,outPoint);
    return;
    }

  void *gridPtr = this->GridPointer;
  int gridType = this->GridScalarType;

  int *extent = this->GridExtent;
  vtkIdType *increments = this->GridIncrements;

  double scale = this->DisplacementScale;
  double shift = this->DisplacementShift;

  double point[3];
  double displacement[3];

  // Convert the inPoint to i,j,k indices into the deformation grid
  // plus fractions
  vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inPoint, point);

  this->InterpolationFunction(point,displacement,NULL,
                              gridPtr,gridType,extent,increments);

  outPoint[0] = inPoint[0] + (displacement[0]*scale + shift);
  outPoint[1] = inPoint[1] + (displacement[1]*scale + shift);
  outPoint[2] = inPoint[2] + (displacement[2]*scale + shift);
}

//----------------------------------------------------------------------------
void vtkOrientedGridTransform::ForwardTransformDerivative(const double inPoint[3],
                                                  double outPoint[3],
                                                  double derivative[3][3])
{
  if (this->GridDirectionMatrix == NULL || this->GridPointer == NULL)
    {
    this->Superclass::ForwardTransformDerivative(inPoint,outPoint,derivative);
    return;
    }

  void *gridPtr = this->GridPointer;
  int gridType = this->GridScalarType;

  int *extent = this->GridExtent;
  vtkIdType *increments = this->GridIncrements;

  double scale = this->DisplacementScale;
  double shift = this->DisplacementShift;

  double point[3];
  double displacement[3];

  // convert the inPoint to i,j,k indices plus fractions
  vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inPoint, point);

  this->InterpolationFunction(point,displacement,derivative,
                              gridPtr,gridType,extent,increments);

  vtkLinearTransformJacobian(derivative, this->OutputToGridIndexTransformMatrixCached->Element, derivative);
  for (int i = 0; i < 3; i++)
    {
    derivative[i][0] = derivative[i][0]*scale;
    derivative[i][1] = derivative[i][1]*scale;
    derivative[i][2] = derivative[i][2]*scale;
    derivative[i][i] += 1.0;
    }

  outPoint[0] = inPoint[0] + (displacement[0]*scale + shift);
  outPoint[1] = inPoint[1] + (displacement[1]*scale + shift);
  outPoint[2] = inPoint[2] + (displacement[2]*scale + shift);
}

//----------------------------------------------------------------------------
void vtkOrientedGridTransform::InverseTransformDerivative(const double inPoint[3],
                                                  double outPoint[3],
                                                  double derivative[3][3])
{
  if (this->GridDirectionMatrix == NULL || this->GridPointer == NULL)
    {
    this->Superclass::InverseTransformDerivative(inPoint,outPoint,derivative);
    return;
    }

  void *gridPtr = this->GridPointer;
  int gridType = this->GridScalarType;

  int *extent = this->GridExtent;
  vtkIdType *increments = this->GridIncrements;

  double shift = this->DisplacementShift;
  double scale = this->DisplacementScale;

  double point[3], inverse[3], lastInverse[3], inverse_IJK[3];
  double deltaP[3], deltaI[3];

  double functionValue = 0;
  double functionDerivative = 0;
  double lastFunctionValue = VTK_DOUBLE_MAX;

  double errorSquared = 0.0;
  double toleranceSquared = this->InverseTolerance;
  toleranceSquared *= toleranceSquared;

  double f = 1.0;
  double a;

  // convert the inPoint to i,j,k indices plus fractions
  vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inPoint, point);

  // first guess at inverse point, just subtract displacement
  // (the inverse point is given in i,j,k indices plus fractions)
  this->InterpolationFunction(point, deltaP, NULL,
                              gridPtr, gridType, extent, increments);

  inverse[0] = inPoint[0] - (deltaP[0]*scale + shift);
  inverse[1] = inPoint[1] - (deltaP[1]*scale + shift);
  inverse[2] = inPoint[2] - (deltaP[2]*scale + shift);
  lastInverse[0] = inverse[0];
  lastInverse[1] = inverse[1];
  lastInverse[2] = inverse[2];

  // do a maximum 500 iterations, usually less than 10 are required
  int n = this->InverseIterations;
  int i, j;

  for (i = 0; i < n; i++)
    {
    vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inverse, inverse_IJK);
    this->InterpolationFunction(inverse_IJK, deltaP, derivative,
                                gridPtr, gridType, extent, increments);

    // convert displacement
    deltaP[0] = (inverse[0] + deltaP[0]*scale + shift) - inPoint[0];
    deltaP[1] = (inverse[1] + deltaP[1]*scale + shift) - inPoint[1];
    deltaP[2] = (inverse[2] + deltaP[2]*scale + shift) - inPoint[2];

    // convert derivative
    vtkLinearTransformJacobian(derivative, this->OutputToGridIndexTransformMatrixCached->Element, derivative);
    for (j = 0; j < 3; j++)
      {
      derivative[j][0] = derivative[j][0]*scale;
      derivative[j][1] = derivative[j][1]*scale;
      derivative[j][2] = derivative[j][2]*scale;
      derivative[j][j] += 1.0;
      }

    // get the current function value
    functionValue = (deltaP[0]*deltaP[0] +
                     deltaP[1]*deltaP[1] +
                     deltaP[2]*deltaP[2]);

    // if the function value is decreasing, do next Newton step
    // (the f < 1.0 is there because I found that convergence
    // is more stable if only a single reduction step is done)
    if (i == 0 || functionValue < lastFunctionValue || f < 1.0)
      {
      // here is the critical step in Newton's method
      vtkMath::LinearSolve3x3(derivative,deltaP,deltaI);

      // get the error value in the output coord space
      errorSquared = (deltaI[0]*deltaI[0] +
                      deltaI[1]*deltaI[1] +
                      deltaI[2]*deltaI[2]);

      // break if less than tolerance in both coordinate systems
      if (errorSquared < toleranceSquared &&
          functionValue < toleranceSquared)
        {
        break;
        }

      // save the last inverse point
      lastInverse[0] = inverse[0];
      lastInverse[1] = inverse[1];
      lastInverse[2] = inverse[2];

      // save error at last inverse point
      lastFunctionValue = functionValue;

      // derivative of functionValue at last inverse point
      functionDerivative = (deltaP[0]*derivative[0][0]*deltaI[0] +
                            deltaP[1]*derivative[1][1]*deltaI[1] +
                            deltaP[2]*derivative[2][2]*deltaI[2])*2;

      // calculate new inverse point
      inverse[0] -= deltaI[0];
      inverse[1] -= deltaI[1];
      inverse[2] -= deltaI[2];

      // reset f to 1.0
      f = 1.0;

      continue;
      }

    // the error is increasing, so take a partial step
    // (see Numerical Recipes 9.7 for rationale, this code
    //  is a simplification of the algorithm provided there)

    // quadratic approximation to find best fractional distance
    a = -functionDerivative/(2*(functionValue -
                                lastFunctionValue -
                                functionDerivative));

    // clamp to range [0.1,0.5]
    f *= (a < 0.1 ? 0.1 : (a > 0.5 ? 0.5 : a));

    // re-calculate inverse using fractional distance
    inverse[0] = lastInverse[0] - f*deltaI[0];
    inverse[1] = lastInverse[1] - f*deltaI[1];
    inverse[2] = lastInverse[2] - f*deltaI[2];
    }

  vtkDebugMacro("Inverse Iterations: " << (i+1));

  if (i >= n)
    {
    // didn't converge: back up to last good result
    inverse[0] = lastInverse[0];
    inverse[1] = lastInverse[1];
    inverse[2] = lastInverse[2];

    vtkWarningMacro("InverseTransformPoint: no convergence (" <<
                    inPoint[0] << ", " << inPoint[1] << ", " << inPoint[2] <<
                    ") error = " << sqrt(errorSquared) << " after " <<
                    i << " iterations.");
    }

  // convert point
  outPoint[0] = inverse[0];
  outPoint[1] = inverse[1];
  outPoint[2] = inverse[2];
}

//----------------------------------------------------------------------------
void vtkOrientedGridTransform::InternalDeepCopy(vtkAbstractTransform *transform)
{
  vtkOrientedGridTransform *gridTransform = (vtkOrientedGridTransform *)transform;

  this->SetGridDirectionMatrix(gridTransform->GetGridDirectionMatrix());

  // Cached matrices will be recomputed automatically in InternalUpdate()
  // therefore we do not need to copy them.

  this->Superclass::InternalDeepCopy(transform);
}

//----------------------------------------------------------------------------
void vtkOrientedGridTransform::InternalUpdate()
{
  this->Superclass::InternalUpdate();

  // Pre-compute GridIndexToOutputTransformMatrixCached transform and store it in a member variable
  // to avoid recomputing it each time a point is transformed.

  this->GridIndexToOutputTransformMatrixCached->Identity();
  for (unsigned int row=0; row < 3; row++ )
    {
    for (unsigned int col = 0; col < 3; col++)
      {
      if (this->GridDirectionMatrix!=NULL)
        {
        this->GridIndexToOutputTransformMatrixCached->SetElement(row, col,
          this->GridSpacing[col]*this->GridDirectionMatrix->GetElement(row,col) );
        }
      else
        {
        this->GridIndexToOutputTransformMatrixCached->SetElement(row, col,
          this->GridSpacing[col]*this->GridIndexToOutputTransformMatrixCached->GetElement(row, col) );
        }
      }
    this->GridIndexToOutputTransformMatrixCached->SetElement(row, 3, this->GridOrigin[row]);
    }

  // Compute Output to GridIndex transform
  vtkMatrix4x4::Invert(this->GridIndexToOutputTransformMatrixCached, this->OutputToGridIndexTransformMatrixCached);

}

//----------------------------------------------------------------------------
vtkAbstractTransform *vtkOrientedGridTransform::MakeTransform()
{
  return vtkOrientedGridTransform::New();
}
