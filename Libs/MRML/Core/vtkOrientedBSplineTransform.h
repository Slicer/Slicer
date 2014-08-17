/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

=========================================================================auto=*/

/// \brief vtkOrientedBSplineTransform - arbitrarily oriented cubic b-spline
/// deformation transformation.
///
/// This transforms extends vtkBSplineTransform to arbitrary grid orientation.
/// Optional affine bulk transform component can be added to the transform so that
/// it can fully represent a itk::BSplineDeformableTransform.
///
/// Unfortunately, the bulk transform cannot be replaced by a multiplication with
/// a linear transform because in itk::BSplineDeformableTransform not the
/// BSpline-transformed point is transformed
/// (outputPoint=affineTransform(bsplineTransform(inputPoint)) but the affine
/// transform is added to the output:
/// outputPoint = affineTransform(inputPoint)+bsplineTransform(inputPoint).
/// This choice does not seem reasonable and this bulk transform has been
/// already removed from the more recent itk::BSplineTransform transform
/// but we need to support this for backward compatibility.

#ifndef __vtkOrientedBSplineTransform_h
#define __vtkOrientedBSplineTransform_h

#include "vtkMRML.h"

#include "vtkBSplineTransform.h"

class VTK_MRML_EXPORT vtkOrientedBSplineTransform : public vtkBSplineTransform
{
public:
  static vtkOrientedBSplineTransform *New();
  vtkTypeMacro(vtkOrientedBSplineTransform,vtkBSplineTransform);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Make another transform of the same type.
  vtkAbstractTransform *MakeTransform();

  // Description:
  // Set/Get the b-spline grid axis directions.
  // This transform class will never modify the data.
  // Must be an orthogonal, normalized matrix.
  // The 4th column and 4th row are ignored.
  virtual void SetGridDirectionMatrix(vtkMatrix4x4*);
  vtkGetObjectMacro(GridDirectionMatrix,vtkMatrix4x4);

  // Description:
  // Set/Get bulk transform that will be added to the b-spline.
  // This transform class will never modify the data.
  virtual void SetBulkTransformMatrix(vtkMatrix4x4*);
  vtkGetObjectMacro(BulkTransformMatrix,vtkMatrix4x4);

  // Description:
  // Get a scale to apply to the transformation.
  // This method is just added because it is missing from the
  // superclass by mistake, so this method can be removed if
  // the GetDisplacementScale method is added to the superclass.
  vtkGetMacro(DisplacementScale,double);

protected:
  vtkOrientedBSplineTransform();
  ~vtkOrientedBSplineTransform();

  // Description:
  // Update the displacement grid.
  void InternalUpdate();

  // Description:
  // Copy this transform from another of the same type.
  void InternalDeepCopy(vtkAbstractTransform *transform);

  // Description:
  // Internal functions for calculating the transformation.
  void ForwardTransformPoint(const double in[3], double out[3]);
  using Superclass::ForwardTransformPoint; // Inherit the float version from parent

  void ForwardTransformDerivative(const double in[3], double out[3],
                                  double derivative[3][3]);
  using Superclass::ForwardTransformDerivative; // Inherit the float version from parent

  void InverseTransformDerivative(const double in[3], double out[3],
                                  double derivative[3][3]);
  using Superclass::InverseTransformDerivative; // Inherit the float version from parent

  // Description:
  // Grid axis direction vectors (i, j, k) in the output space
  vtkMatrix4x4* GridDirectionMatrix;

  // Description:
  // Bulk linear transform that is added to the b-spline transform
  vtkMatrix4x4* BulkTransformMatrix;

  vtkMatrix4x4* GridIndexToOutputTransformMatrixCached;
  vtkMatrix4x4* OutputToGridIndexTransformMatrixCached;
  vtkMatrix4x4* InverseBulkTransformMatrixCached;

private:
  vtkOrientedBSplineTransform(const vtkOrientedBSplineTransform&);  // Not implemented.
  void operator=(const vtkOrientedBSplineTransform&);  // Not implemented.
};

#endif
