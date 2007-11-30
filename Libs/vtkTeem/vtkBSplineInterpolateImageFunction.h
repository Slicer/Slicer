/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkBSplineInterpolateImageFunction.h,v $
  Date:      $Date: 2006/05/26 20:02:42 $
  Version:   $Revision: 1.10 $

=========================================================================auto=*/
// .NAME vtkBSplineInterpolateImageFunction - BSpline interpolation of a image dataset of points
// .SECTION Description
// vtkBSplineInterpolateImageFunction
// .SECTION Bugs
// This class should be rewritten to properly take into account the ImageData. This is
// not a regular SetInput (not ref count). Also mtime of image is not taken into account...


#ifndef __vtkBSplineInterpolateImageFunction_h
#define __vtkBSplineInterpolateImageFunction_h

#include "vtkImplicitFunction.h"
#include "vtkTeemConfigure.h"

#include <vtkstd/vector>  // for the buffer

#define VTK_INTEGRATE_MAJOR_EIGENVECTOR  0
#define VTK_INTEGRATE_MEDIUM_EIGENVECTOR 1
#define VTK_INTEGRATE_MINOR_EIGENVECTOR  2

#define ImageDimension 3

class vtkImageData;
class  VTK_TEEM_EXPORT vtkBSplineInterpolateImageFunction : public vtkImplicitFunction
{
 public:
  static vtkBSplineInterpolateImageFunction *New();
  vtkTypeRevisionMacro(vtkBSplineInterpolateImageFunction, vtkImplicitFunction );
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  virtual vtkFloatingPointType EvaluateFunction (vtkFloatingPointType x[ImageDimension]);

  virtual void EvaluateGradient (vtkFloatingPointType x[ImageDimension], 
                                 vtkFloatingPointType g[ImageDimension]);
  void SetInput(vtkImageData* dataset);

  unsigned int GetSplineOrder() { return this->SplineOrder; }
  void SetSplineOrder(unsigned int order);

protected:
  vtkBSplineInterpolateImageFunction() {
    this->Initialized = 0;
    this->SplineOrder = 0; // Nasty bug, really calling a Set* function in constructor is bad
    this->SetSplineOrder(3);
  }
  ~vtkBSplineInterpolateImageFunction() {}
  int                                 DataLength[ImageDimension];  // Image size
  unsigned int                        SplineOrder;    // User specified spline order (3rd or cubic is the default)

  vtkImageData *                      Coefficients; // Spline coefficients  
  vtkFloatingPointType * Origin;
  vtkFloatingPointType * Spacing;
  int * Extent;
private:
  //BTX
  int Initialized;
  vtkBSplineInterpolateImageFunction(const vtkBSplineInterpolateImageFunction&);  // Not implemented.
  void operator=(const vtkBSplineInterpolateImageFunction&);  // Not implemented.

  /** Determines the weights for interpolation of the value x */
  void SetInterpolationWeights( vtkFloatingPointType *x, long *evaluateIndex[ImageDimension],
    double *weights[ImageDimension],unsigned int splineOrder ) const;

  /** Determines the weights for the derivative portion of the value x */
  void SetDerivativeWeights( vtkFloatingPointType *x, long *evaluateIndex[ImageDimension],
    double *weights[ImageDimension], unsigned int splineOrder ) const;

  /** Precomputation for converting the 1D index of the interpolation neighborhood 
    * to an N-dimensional index. */
  void GeneratePointsToIndex();

  /** Determines the indicies to use give the splines region of support */
  void DetermineRegionOfSupport( long *evaluateIndex[ImageDimension],
    vtkFloatingPointType x[], unsigned int splineOrder ) const;

  /** Set the indicies in evaluateIndex at the boundaries based on mirror 
    * boundary conditions. */
  void ApplyMirrorBoundaryConditions(long *evaluateIndex[ImageDimension], 
    unsigned int splineOrder) const;


  unsigned int             MaxNumberInterpolationPoints; // number of neighborhood points used for interpolation
  std::vector<int>    PointsToIndex[ImageDimension];  // Preallocation of interpolation neighborhood indicies
  //ETX
};


#endif
