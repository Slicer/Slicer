/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkDiffusionTensorMathematicsSimple.h,v $

  Copyright (c) Ken Martin, Will Schroeder, Bill Lorensen
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkDiffusionTensorMathematicsSimple - Simple example of an image-image filter.
// .SECTION Description
// This is an example of a simple image-image filter. It copies it's input
// to it's output (point by point). It shows how templates can be used
// to support various data types.
// .SECTION See also
// vtkDiffusionTensorMathematicsSimple

#ifndef __vtkDiffusionTensorMathematicsSimple_h
#define __vtkDiffusionTensorMathematicsSimple_h

#include "vtkSimpleImageToImageFilter.h"
#include "vtkTeemConfigure.h"
#include "vtkThreadedImageAlgorithm.h"

class vtkMatrix4x4;
class vtkImageData;

class VTK_TEEM_EXPORT vtkDiffusionTensorMathematicsSimple : public vtkSimpleImageToImageFilter
{
public:
  static vtkDiffusionTensorMathematicsSimple *New();
  vtkTypeRevisionMacro(vtkDiffusionTensorMathematicsSimple,vtkSimpleImageToImageFilter);
  
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description:
  // Get the Operation to perform.
  vtkGetMacro(Operation,int);
  vtkSetClampMacro(Operation,int, VTK_TENS_TRACE, VTK_TENS_PERPENDICULAR_DIFFUSIVITY);


  // Operation options.
  //BTX
  enum
  {
    VTK_TENS_TRACE,
    VTK_TENS_DETERMINANT,
    VTK_TENS_RELATIVE_ANISOTROPY,
    VTK_TENS_FRACTIONAL_ANISOTROPY,
    VTK_TENS_MAX_EIGENVALUE,
    VTK_TENS_MID_EIGENVALUE,
    VTK_TENS_MIN_EIGENVALUE,
    VTK_TENS_LINEAR_MEASURE,
    VTK_TENS_PLANAR_MEASURE,
    VTK_TENS_SPHERICAL_MEASURE,
    VTK_TENS_COLOR_ORIENTATION,
    VTK_TENS_D11,
    VTK_TENS_D22,
    VTK_TENS_D33,
    VTK_TENS_MODE,
    VTK_TENS_COLOR_MODE,
    VTK_TENS_MAX_EIGENVALUE_PROJX,
    VTK_TENS_MAX_EIGENVALUE_PROJY,
    VTK_TENS_MAX_EIGENVALUE_PROJZ,
    VTK_TENS_RAI_MAX_EIGENVEC_PROJX,
    VTK_TENS_RAI_MAX_EIGENVEC_PROJY,
    VTK_TENS_RAI_MAX_EIGENVEC_PROJZ,
    VTK_TENS_PARALLEL_DIFFUSIVITY,
    VTK_TENS_PERPENDICULAR_DIFFUSIVITY,
  };
  //ETX


  // Description:
  // Output the trace (sum of eigenvalues = sum along diagonal)
  void SetOperationToTrace() 
    {this->SetOperation(VTK_TENS_TRACE);};

  // Description:
  // Output the determinant
  void SetOperationToDeterminant() 
    {this->SetOperation(VTK_TENS_DETERMINANT);};

  // Description:
  // Output various anisotropy and shape measures
  void SetOperationToRelativeAnisotropy() 
    {this->SetOperation(VTK_TENS_RELATIVE_ANISOTROPY);};
  void SetOperationToFractionalAnisotropy() 
    {this->SetOperation(VTK_TENS_FRACTIONAL_ANISOTROPY);};
  void SetOperationToLinearMeasure() 
    {this->SetOperation(VTK_TENS_LINEAR_MEASURE);};
  void SetOperationToPlanarMeasure() 
    {this->SetOperation(VTK_TENS_PLANAR_MEASURE);};
  void SetOperationToSphericalMeasure() 
    {this->SetOperation(VTK_TENS_SPHERICAL_MEASURE);};
  // This is the skewness of the eigenvalues 
  // (thanks to Gordon Lothar (of the Hill People) Kindlmann)
  void SetOperationToMode() 
    {this->SetOperation(VTK_TENS_MODE);};
  void SetOperationToParallelDiffusivity()
    {this->SetOperation(VTK_TENS_PARALLEL_DIFFUSIVITY);};
  void SetOperationToPerpendicularDiffusivity()
    {this->SetOperation(VTK_TENS_PERPENDICULAR_DIFFUSIVITY);};

  // Description:
  // Output a selected eigenvalue
  void SetOperationToMaxEigenvalue() 
    {this->SetOperation(VTK_TENS_MAX_EIGENVALUE);};
  void SetOperationToMiddleEigenvalue() 
    {this->SetOperation(VTK_TENS_MID_EIGENVALUE);};
  void SetOperationToMinEigenvalue() 
    {this->SetOperation(VTK_TENS_MIN_EIGENVALUE);};

  // Description:
  // Output Maxeigenvalue*Maxeigenvec_projection also known as L1Z
  void SetOperationToMaxEigenvalueProjectionX()
  {this->SetOperation(VTK_TENS_MAX_EIGENVALUE_PROJX);};
  void SetOperationToMaxEigenvalueProjectionY()
  {this->SetOperation(VTK_TENS_MAX_EIGENVALUE_PROJY);};
  void SetOperationToMaxEigenvalueProjectionZ()
  {this->SetOperation(VTK_TENS_MAX_EIGENVALUE_PROJZ);};
  
  // Description:
  // Output Relative_anisotropy*Maxeigenvec_projection also known as L1z
  void SetOperationToRAIMaxEigenvecX()
  {this->SetOperation(VTK_TENS_RAI_MAX_EIGENVEC_PROJX);}
  void SetOperationToRAIMaxEigenvecY()
  {this->SetOperation(VTK_TENS_RAI_MAX_EIGENVEC_PROJY);}
  void SetOperationToRAIMaxEigenvecZ()
  {this->SetOperation(VTK_TENS_RAI_MAX_EIGENVEC_PROJZ);}
  
  // Description: 
  // Output a matrix (tensor) component
  void SetOperationToD11() 
    {this->SetOperation(VTK_TENS_D11);};
  void SetOperationToD22() 
    {this->SetOperation(VTK_TENS_D22);};
  void SetOperationToD33() 
    {this->SetOperation(VTK_TENS_D33);};
  
  // Description:
  // Output RGB color according to XYZ of eigenvectors.
  // Output A (alpha, or transparency) according to 
  // anisotropy (1-spherical measure).
  void SetOperationToColorByOrientation() 
    {this->SetOperation(VTK_TENS_COLOR_ORIENTATION);};

  // Description:
  // Output RGB color according to colormapping of mode, with 
  // final RGB being a linear combination of gray and 
  // this color.  Amount of gray is determined by FA.
  // Thanks to Gordon Lothar Kindlmann for this method.
  void SetOperationToColorByMode() 
    {this->SetOperation(VTK_TENS_COLOR_MODE);};

  // Description:
  // Specify scale factor to scale output (float) scalars by.
  // This is not used when the output is RGBA (char color data).
  vtkSetMacro(ScaleFactor,double);
  vtkGetMacro(ScaleFactor,double);

  // Description:
  // Turn on/off extraction of eigenvalues from tensor.
  vtkSetMacro(ExtractEigenvalues,int);
  vtkBooleanMacro(ExtractEigenvalues,int);
  vtkGetMacro(ExtractEigenvalues,int);

  // Description
  // This matrix is only used for ColorByOrientation.
  // We transform the tensor orientation by this matrix
  // before setting the output RGB values.
  //
  // This is useful to put the output colors into a standard
  // coordinate system (i.e. RAS) regardless of the data scan order.
  //
  // Example usage is as follows:
  // 1) If tensors are to be displayed in a coordinate system
  //    that is not IJK (array-based), and the whole volume is
  //    being rotated, each tensor needs also to be rotated.
  //    First find the matrix that positions your volume.
  //    (This is how the entire volume is positioned, not 
  //    the matrix that positions an arbitrary reformatted slice.)
  // 2) Remove scaling and translation from this matrix; we
  //    just need to rotate each tensor.
  // 3) Set TensorRotationMatrix to this rotation matrix.
  //
  virtual void SetTensorRotationMatrix(vtkMatrix4x4*);
  vtkGetObjectMacro(TensorRotationMatrix, vtkMatrix4x4);

  // Description
  // Input scalars are a binary mask: 0 prevents display
  // of tensor quantity at that point
  vtkBooleanMacro(MaskWithScalars, int);
  vtkSetMacro(MaskWithScalars, int);
  vtkGetMacro(MaskWithScalars, int);

  vtkBooleanMacro(FixNegativeEigenvalues, int);
  vtkSetMacro(FixNegativeEigenvalues, int);
  vtkGetMacro(FixNegativeEigenvalues, int);

  // Description:
  // Scalar mask
  virtual void SetScalarMask(vtkImageData*);
  vtkGetObjectMacro(ScalarMask, vtkImageData);
  
  // Description:
  // Label value defining ROI for mask  
  vtkSetMacro(MaskLabelValue, int);
  vtkGetMacro(MaskLabelValue, int);

  // Public for access from threads
  static void ModeToRGB(double Mode, double FA,
                 double &R, double &G, double &B);


  // Description:
  // Helper functions to perform operations pixel-wise
  static int FixNegativeEigenvaluesMethod(double w[3]);
  static double Determinant(double D[3][3]);
  static double Trace(double D[3][3]);
  static double Trace(double w[3]);
  static double RelativeAnisotropy(double w[3]);
  static double FractionalAnisotropy(double w[3]);
  static double LinearMeasure(double w[3]);
  static double PlanarMeasure(double w[3]);
  static double SphericalMeasure(double w[3]);
  static double MaxEigenvalue(double w[3]);
  static double MiddleEigenvalue(double w[3]);
  static double ParallelDiffusivity(double w[3]);
  static double PerpendicularDiffusivity(double w[3]);
  static double MinEigenvalue(double w[3]);
  static double RAIMaxEigenvecX(double **v, double w[3]);
  static double RAIMaxEigenvecY(double **v, double w[3]);
  static double RAIMaxEigenvecZ(double **v, double w[3]);
  static double MaxEigenvalueProjectionX(double **v, double w[3]);
  static double MaxEigenvalueProjectionY(double **v, double w[3]);
  static double MaxEigenvalueProjectionZ(double **v, double w[3]);
  static double Mode(double w[3]);
  static void ColorByMode(double w[3], double &R,double &G, double &B);

  //Description
  //Wrap function to teem eigen solver
  static int TeemEigenSolver(double **m, double *w, double **v);
  void ComputeTensorIncrements(vtkImageData *imageData, vtkIdType incr[3]);

protected:

  vtkDiffusionTensorMathematicsSimple();
  ~vtkDiffusionTensorMathematicsSimple();

  virtual void SimpleExecute(vtkImageData* input, vtkImageData* output);

  virtual int RequestInformation (
                                  vtkInformation * vtkNotUsed(request),
                                  vtkInformationVector **inputVector,
                                  vtkInformationVector *outputVector);

  int Operation; // math operation to perform
  double ScaleFactor; // Scale factor for output scalars
  int ExtractEigenvalues; // Boolean controls eigenfunction extraction

  int MaskWithScalars;
  vtkImageData *ScalarMask;
  int MaskLabelValue;

  vtkMatrix4x4 *TensorRotationMatrix;
  int FixNegativeEigenvalues;

  int FillInputPortInformation(int port, vtkInformation* info);


private:
  vtkDiffusionTensorMathematicsSimple(const vtkDiffusionTensorMathematicsSimple&);  // Not implemented.
  void operator=(const vtkDiffusionTensorMathematicsSimple&);  // Not implemented.
};

#endif







