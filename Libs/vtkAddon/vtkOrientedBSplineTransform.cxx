/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

#include "vtkOrientedBSplineTransform.h"

#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkMatrix4x4.h"
#include "vtkObjectFactory.h"

#include <math.h>

vtkStandardNewMacro(vtkOrientedBSplineTransform);

vtkCxxSetObjectMacro(vtkOrientedBSplineTransform,GridDirectionMatrix,vtkMatrix4x4);
vtkCxxSetObjectMacro(vtkOrientedBSplineTransform,BulkTransformMatrix,vtkMatrix4x4);

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

//------------------------------------------------------------------------
inline void vtkLinearTransformDerivative(const double matrix[4][4],
                                         const double in[3], double out[3],
                                         double derivative[3][3])
{
  vtkLinearTransformPoint(matrix,in,out);

  for (int i = 0; i < 3; i++)
    {
    derivative[0][i] = matrix[0][i];
    derivative[1][i] = matrix[1][i];
    derivative[2][i] = matrix[2][i];
    }
}

//----------------------------------------------------------------------------
vtkOrientedBSplineTransform::vtkOrientedBSplineTransform()
{
  this->GridDirectionMatrix = NULL;
  this->BulkTransformMatrix = NULL;
  this->GridIndexToOutputTransformMatrixCached = vtkMatrix4x4::New();
  this->OutputToGridIndexTransformMatrixCached = vtkMatrix4x4::New();
  this->InverseBulkTransformMatrixCached = vtkMatrix4x4::New();
}

//----------------------------------------------------------------------------
vtkOrientedBSplineTransform::~vtkOrientedBSplineTransform()
{
  this->SetGridDirectionMatrix(NULL);
  this->SetBulkTransformMatrix(NULL);
  if (this->GridIndexToOutputTransformMatrixCached!=NULL)
    {
    this->GridIndexToOutputTransformMatrixCached->Delete();
    this->GridIndexToOutputTransformMatrixCached=NULL;
    }
  if (this->OutputToGridIndexTransformMatrixCached!=NULL)
    {
    this->OutputToGridIndexTransformMatrixCached->Delete();
    this->OutputToGridIndexTransformMatrixCached=NULL;
    }
  if (this->InverseBulkTransformMatrixCached!=NULL)
    {
    this->InverseBulkTransformMatrixCached->Delete();
    this->InverseBulkTransformMatrixCached=NULL;
    }
}

//----------------------------------------------------------------------------
void vtkOrientedBSplineTransform::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "GridDirectionMatrix: " << this->GridDirectionMatrix << "\n";
  if (this->GridDirectionMatrix)
    {
    this->GridDirectionMatrix->PrintSelf(os,indent.GetNextIndent());
    }
  os << indent << "BulkTransform: " << this->GetBulkTransformMatrix() << "\n";
  if (this->GetBulkTransformMatrix())
    {
    this->GetBulkTransformMatrix()->PrintSelf(os,indent.GetNextIndent());
    }
}

//----------------------------------------------------------------------------
void vtkOrientedBSplineTransform::ForwardTransformPoint(const double inPointTemp[3],
                                                double outPoint[3])
{
  // inPointTemp and outPoint may be the same vector, so make a copy of the
  // input before modifying the output
  double inPoint[3]={inPointTemp[0],inPointTemp[1],inPointTemp[2]};

  if (this->BulkTransformMatrix)
    {
    vtkLinearTransformPoint(this->BulkTransformMatrix->Element, inPoint, outPoint);
    }
  else
    {
    outPoint[0] = inPoint[0];
    outPoint[1] = inPoint[1];
    outPoint[2] = inPoint[2];
    }

  
  if (!this->GridPointer || !this->CalculateSpline)
    {
    return;
    }

  void *gridPtr = this->GridPointer;
  int *extent = this->GridExtent;
  vtkIdType *increments = this->GridIncrements;
  double scale = this->DisplacementScale;

  double point[3];
  // Convert the inPoint to i,j,k indices into the deformation grid
  // plus fractions
  vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inPoint, point);

  double displacement[3]={0.0, 0.0, 0.0};

  this->CalculateSpline(point, displacement, 0,
                        gridPtr, extent, increments, this->BorderMode);

  outPoint[0] += displacement[0]*scale;
  outPoint[1] += displacement[1]*scale;
  outPoint[2] += displacement[2]*scale;
}

//----------------------------------------------------------------------------
// calculate the derivative of the transform
void vtkOrientedBSplineTransform::ForwardTransformDerivative(const double inPointTemp[3],
                                                     double outPoint[3],
                                                     double derivative[3][3])
{
  // inPointTemp and outPoint may be the same vector, so make a copy of the
  // input before modifying the output
  double inPoint[3] = {inPointTemp[0],inPointTemp[1],inPointTemp[2]};

  if (this->BulkTransformMatrix)
    {
    vtkLinearTransformDerivative(this->BulkTransformMatrix->Element,inPoint,outPoint,derivative);
    }
  else
    {
    outPoint[0] = inPoint[0];
    outPoint[1] = inPoint[1];
    outPoint[2] = inPoint[2];
    vtkMath::Identity3x3(derivative);
    }

  if (!this->GridPointer || !this->CalculateSpline)
    {
    return;
    }

  void *gridPtr = this->GridPointer;
  int *extent = this->GridExtent;
  vtkIdType *increments = this->GridIncrements;
  double scale = this->DisplacementScale;

  double point[3];
  // Convert the inPoint to i,j,k indices into the deformation grid
  // plus fractions
  vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inPoint, point);

  double displacement[3]={0.0, 0.0, 0.0};
  double splineDerivative[3][3];

  this->CalculateSpline(point,displacement,splineDerivative,
                        gridPtr,extent,increments, this->BorderMode);

  // derivative = BulkTransformDerivativeMatrix + SplineDerivativeMatrix * spacing * scale
  // Take into account grid spacing and direction cosines
  vtkLinearTransformJacobian(splineDerivative, this->OutputToGridIndexTransformMatrixCached->Element, splineDerivative);
  for (int i = 0; i < 3; i++)
    {
    derivative[i][0] += splineDerivative[i][0]*scale;
    derivative[i][1] += splineDerivative[i][1]*scale;
    derivative[i][2] += splineDerivative[i][2]*scale;
    }

  // outPoint = BulkTransformMatrix * inPointVector + displacementVector * scale
  outPoint[0] += displacement[0]*scale;
  outPoint[1] += displacement[1]*scale;
  outPoint[2] += displacement[2]*scale;
}

//----------------------------------------------------------------------------
// We use Newton's method to iteratively invert the transformation.
// This is actally quite robust as long as the Jacobian matrix is never
// singular.
// Note that this is similar to vtkWarpTransform::InverseTransformPoint()
// but has been optimized specifically for uniform grid transforms.
void vtkOrientedBSplineTransform::InverseTransformDerivative(const double inPointTemp[3],
                                                     double outPoint[3],
                                                     double derivative[3][3])
{
  // inPointTemp and outPoint may be the same vector, so make a copy of the
  // input before modifying the output
  double inPoint[3] = {inPointTemp[0],inPointTemp[1],inPointTemp[2]};

  if (this->BulkTransformMatrix)
    {
    // Note that the derivative of the inverse transform is simply the
    // inverse of the derivative of the forward transform.
    vtkLinearTransformDerivative(this->InverseBulkTransformMatrixCached->Element,inPoint,outPoint,derivative);
    }
  else
    {
    outPoint[0] = inPoint[0];
    outPoint[1] = inPoint[1];
    outPoint[2] = inPoint[2];
    vtkMath::Identity3x3(derivative);
    }

  if (!this->GridPointer || !this->CalculateSpline)
    {
    return;
    }

  void *gridPtr = this->GridPointer;
  int *extent = this->GridExtent;
  vtkIdType *increments = this->GridIncrements;

  double scale = this->DisplacementScale;

  double inverse_IJK[3], lastInverse[3], inverse[3];
  double deltaP[3], errorVector[3], deltaI[3];

  double functionValue = 0;
  double functionDerivative = 0;
  double lastFunctionValue = VTK_DOUBLE_MAX;

  double errorSquared = 0.0;
  double toleranceSquared = this->InverseTolerance * this->InverseTolerance;

  double f = 1.0;
  double a;

  double inPoint_IJK[3];
  // Convert the inPoint to i,j,k indices into the deformation grid
  // plus fractions
  vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inPoint, inPoint_IJK);

  // first guess at inverse_IJK point, just subtract displacement
  // (the inverse point is given in i,j,k indices plus fractions)
  this->CalculateSpline(inPoint_IJK, deltaP, 0,
                        gridPtr, extent, increments, this->BorderMode);

  double inverseBulkTransformedInPoint[3];
  vtkLinearTransformPoint(this->InverseBulkTransformMatrixCached->Element,inPoint,inverseBulkTransformedInPoint);

  inverse[0] = inverseBulkTransformedInPoint[0] - deltaP[0]*scale;
  inverse[1] = inverseBulkTransformedInPoint[1] - deltaP[1]*scale;
  inverse[2] = inverseBulkTransformedInPoint[2] - deltaP[2]*scale;
  lastInverse[0] = inverse[0];
  lastInverse[1] = inverse[1];
  lastInverse[2] = inverse[2];

  // do a maximum 500 iterations, usually less than 10 are required
  int maxNumberOfIterations = this->InverseIterations;

  double splineDerivative[3][3];
  double inverseBulkTransformed[3];

  int iteration=0;
  for (; iteration < maxNumberOfIterations; iteration++)
    {
    // Get displacement and derivative from b-spline
    vtkLinearTransformPoint(this->OutputToGridIndexTransformMatrixCached->Element, inverse, inverse_IJK);
    this->CalculateSpline(inverse_IJK, deltaP, splineDerivative,
                          gridPtr, extent, increments, this->BorderMode);

    // Get displacement and derivative from bulk
    if (this->BulkTransformMatrix!=NULL)
      {
      vtkLinearTransformDerivative(this->BulkTransformMatrix->Element,inverse,inverseBulkTransformed,derivative);

      // Compute total derivative
      vtkLinearTransformJacobian(splineDerivative, this->OutputToGridIndexTransformMatrixCached->Element, splineDerivative);
      for (int i = 0; i < 3; i++)
        {
        derivative[i][0] += splineDerivative[i][0]*scale;
        derivative[i][1] += splineDerivative[i][1]*scale;
        derivative[i][2] += splineDerivative[i][2]*scale;
        }

      // Compute total displacement error
      errorVector[0] = (inverseBulkTransformed[0] + deltaP[0]*scale) - inPoint[0];
      errorVector[1] = (inverseBulkTransformed[1] + deltaP[1]*scale) - inPoint[1];
      errorVector[2] = (inverseBulkTransformed[2] + deltaP[2]*scale) - inPoint[2];
      }
    else
      {
      // Compute derivative
      vtkLinearTransformJacobian(splineDerivative, this->OutputToGridIndexTransformMatrixCached->Element, splineDerivative);
      for (int i = 0; i < 3; i++)
        {
        derivative[i][0] = splineDerivative[i][0]*scale;
        derivative[i][1] = splineDerivative[i][1]*scale;
        derivative[i][2] = splineDerivative[i][2]*scale;
        derivative[i][i] += 1.0;
        }

      // Compute displacement error
      errorVector[0] = (inverse[0] + deltaP[0]*scale) - inPoint[0];
      errorVector[1] = (inverse[1] + deltaP[1]*scale) - inPoint[1];
      errorVector[2] = (inverse[2] + deltaP[2]*scale) - inPoint[2];
      }

    // get the current function value
    functionValue = (errorVector[0]*errorVector[0] +
                     errorVector[1]*errorVector[1] +
                     errorVector[2]*errorVector[2]);

    // if the function value is decreasing, do next Newton step
    if (iteration == 0 || functionValue < lastFunctionValue)
      {
      // here is the critical step in Newton's method
      vtkMath::LinearSolve3x3(derivative,errorVector,deltaI);

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
      functionDerivative = (errorVector[0]*derivative[0][0]*deltaI[0] +
                            errorVector[1]*derivative[1][1]*deltaI[1] +
                            errorVector[2]*derivative[2][2]*deltaI[2])*2;

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

  if (iteration >= maxNumberOfIterations)
    {
    // didn't converge: back up to last good result
    inverse[0] = lastInverse[0];
    inverse[1] = lastInverse[1];
    inverse[2] = lastInverse[2];

    vtkWarningMacro("InverseTransformPoint: no convergence (" <<
                    inPoint[0] << ", " << inPoint[1] << ", " << inPoint[2] <<
                    ") error = " << sqrt(errorSquared) << " after " <<
                    iteration << " iterations.");
    }

  // Convert the inPoint to i,j,k indices into the deformation grid
  // plus fractions
  outPoint[0] = inverse[0];
  outPoint[1] = inverse[1];
  outPoint[2] = inverse[2];
}

//----------------------------------------------------------------------------
void vtkOrientedBSplineTransform::InternalDeepCopy(vtkAbstractTransform *transform)
{
  vtkOrientedBSplineTransform *orientedBSplineTransform = (vtkOrientedBSplineTransform *)transform;
  this->SetGridDirectionMatrix(orientedBSplineTransform->GetGridDirectionMatrix());
  this->SetBulkTransformMatrix(orientedBSplineTransform ->GetBulkTransformMatrix());

  // Cached matrices will be recomputed automatically in InternalUpdate()
  // therefore we do not need to copy them.

  this->Superclass::InternalDeepCopy(transform);
}

//----------------------------------------------------------------------------
void vtkOrientedBSplineTransform::InternalUpdate()
{
  this->Superclass::InternalUpdate();

  // Pre-compute GridIndex to Output transform and store it in a member variable
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

  if (this->BulkTransformMatrix)
    {
    vtkMatrix4x4::Invert(this->BulkTransformMatrix, this->InverseBulkTransformMatrixCached);
    }
}

//----------------------------------------------------------------------------
vtkAbstractTransform *vtkOrientedBSplineTransform::MakeTransform()
{
  return vtkOrientedBSplineTransform::New();
}
