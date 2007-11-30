/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTensorImplicitFunctionToFunctionSet.h,v $
  Date:      $Date: 2006/03/06 21:07:34 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/

// .NAME vtkTensorImplicitFunctionToFunctionSet - Wrapper class around itk::BSplineInterpolateImageFuntion
// .SECTION Description
// vtkTensorImplicitFunctionToFunctionSet


#ifndef __vtkTensorImplicitFunctionToFunctionSet_h
#define __vtkTensorImplicitFunctionToFunctionSet_h

#define VTK_INTEGRATE_MAJOR_EIGENVECTOR  0
#define VTK_INTEGRATE_MEDIUM_EIGENVECTOR 1
#define VTK_INTEGRATE_MINOR_EIGENVECTOR  2

#include "vtkTeemConfigure.h"

#include "vtkImplicitFunction.h"
#include "vtkFunctionSet.h"
#include "vtkImageData.h"

class VTK_TEEM_EXPORT vtkTensorImplicitFunctionToFunctionSet : public vtkFunctionSet
{
 public:
  static vtkTensorImplicitFunctionToFunctionSet *New();
  vtkTypeRevisionMacro(vtkTensorImplicitFunctionToFunctionSet, vtkFunctionSet );
  virtual void PrintSelf(ostream& os, vtkIndent indent);
  virtual int FunctionValues(vtkFloatingPointType* x, vtkFloatingPointType* f);
  int GetTensor(vtkFloatingPointType *x, vtkFloatingPointType * f);
  virtual void AddDataSet(vtkDataSet* ) {}
  void AddImplicitFunction(vtkImplicitFunction * func, int numcomp ) {
    if ( numcomp < 0 || numcomp > 5 )
      return;
    else {
      TensorComponent[numcomp] = func;
    }
  }
  void SetDataBounds(vtkImageData* im) {
    im->GetExtent(DataExtent);
    im->GetOrigin(DataOrigin);
    im->GetSpacing(DataSpacing);
  }
  // Set the Integration Direction
  void SetIntegrationForward(void);
  void SetIntegrationBackward(void);
  void SetChangeIntegrationDirection(void);
  void SetIntegrationDirection(vtkFloatingPointType dir[3]);

  // Description:
  // Set/get lower bound bias. If fractional anisotropy falls below this value, no regularization is done
  // ( should be set to the value of the terminal fractional anisotropy )
  vtkSetClampMacro(LowerBoundBias,vtkFloatingPointType,0.0,UpperBoundBias);
  vtkGetMacro(LowerBoundBias,vtkFloatingPointType);

  // Description:
  // Set/get upper bound bias. If fractional anisotropy falls below this value, regularization is done on the tensor
  vtkSetClampMacro(UpperBoundBias,vtkFloatingPointType,LowerBoundBias,1.0);
  vtkGetMacro(UpperBoundBias,vtkFloatingPointType);

  vtkGetMacro(LastFractionalAnisotropy,vtkFloatingPointType);

  // Description:
  // Set/get the magnitude of the correction bias
  vtkSetClampMacro(CorrectionBias,vtkFloatingPointType,0.0,UpperBoundBias);
  vtkGetMacro(CorrectionBias,vtkFloatingPointType);
  int IsInsideImage(vtkFloatingPointType x[3]);
  void GetLastEigenvalues(vtkFloatingPointType v[3]) {
    memcpy(v,eigVal,3*sizeof(vtkFloatingPointType));
  }
  void GetLastEigenvectors(vtkFloatingPointType *v[3]) {
    for ( int i = 0 ; i < 3 ; i++ )
      memcpy(v[i],eigVec[i],3*sizeof(vtkFloatingPointType));
  }
 protected:
  
  vtkFloatingPointType Direction[3];
  int DirectionValid;
  int IntegrationDirection;
  vtkFloatingPointType LastFractionalAnisotropy;
  vtkFloatingPointType LowerBoundBias;
  vtkFloatingPointType UpperBoundBias;
  vtkFloatingPointType CorrectionBias;
  int DataExtent[6];
  vtkFloatingPointType DataOrigin[3];
  vtkFloatingPointType DataSpacing[3];

  vtkFloatingPointType vec[9];
  vtkFloatingPointType *eigVec[3];
  vtkFloatingPointType eigVal[3];
  //BTX

  vtkImplicitFunction* TensorComponent[6];

  vtkTensorImplicitFunctionToFunctionSet()
    {
    int i;
    for (i = 0 ; i < 6 ; i++ ) {
    TensorComponent[i]=0;
    DataExtent[i]=0;
      }
    for (i = 0 ; i < 3 ; i++ ) {
    DataOrigin[i]=0.0;
    DataSpacing[i]=1.0;
    eigVec[i] = &(vec[3*i]);
      }
      DirectionValid=0;
      NumFuncs=3;
      NumIndepVars=4;
      LowerBoundBias=0.0;
      UpperBoundBias=0.0;
      CorrectionBias=0.0;
      IntegrationDirection = VTK_INTEGRATE_MAJOR_EIGENVECTOR;
      SetIntegrationForward();
    }
  ~vtkTensorImplicitFunctionToFunctionSet() {}

  //ETX
  
private:
  vtkTensorImplicitFunctionToFunctionSet(const vtkTensorImplicitFunctionToFunctionSet&);  // Not implemented.
  void operator=(const vtkTensorImplicitFunctionToFunctionSet&);  // Not implemented.
};

#endif
