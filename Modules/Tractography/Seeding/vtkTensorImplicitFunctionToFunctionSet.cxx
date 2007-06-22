/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTensorImplicitFunctionToFunctionSet.cxx,v $
  Date:      $Date: 2006/03/06 21:07:34 $
  Version:   $Revision: 1.8 $

=========================================================================auto=*/
#ifndef _vtkTensorImplicitFunctionToFunctionSet_txx
#define _vtkTensorImplicitFunctionToFunctionSet_txx

#include "vtkMath.h"
#include "vtkDiffusionTensorMathematics.h"
#include "vtkObjectFactory.h"
#include "vtkTensorImplicitFunctionToFunctionSet.h"


vtkCxxRevisionMacro(vtkTensorImplicitFunctionToFunctionSet, "$Revision: 1.8 $");
vtkStandardNewMacro(vtkTensorImplicitFunctionToFunctionSet);

void vtkTensorImplicitFunctionToFunctionSet::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  for ( int i = 0 ; i < 6 ; i++ ) {
    if ( TensorComponent[i] ) {
      os << indent << "Tensor Component: " << i <<endl;
      TensorComponent[i]->PrintSelf(os,indent);
    }
  }
}

int vtkTensorImplicitFunctionToFunctionSet::GetTensor(vtkFloatingPointType *x, vtkFloatingPointType * res) {
    
  if ( TensorComponent[0] )
    res[0] = TensorComponent[0]->EvaluateFunction(x);
  else {
    res[0] = 0.0;
  }

  if ( TensorComponent[1] )
    res[1] = TensorComponent[1]->EvaluateFunction(x);
  else {
    res[1] = 0.0;
  }

  if ( TensorComponent[2] )
    res[2] = TensorComponent[2]->EvaluateFunction(x);
  else {
    res[2] = 0.0;
  }

  if ( TensorComponent[3] )
    res[4] = TensorComponent[3]->EvaluateFunction(x);
  else {
    res[4] = 0.0;
  }

  if ( TensorComponent[4] )
    res[5] = TensorComponent[4]->EvaluateFunction(x);
  else {
    res[5] = 0.0;
  }

  if ( TensorComponent[5] )
    res[8] = TensorComponent[5]->EvaluateFunction(x);
  else {
    res[8] = 0.0;
  }

  res[3] = res[1];
  res[6] = res[2];
  res[7] = res[5];

  return 1;
}

int vtkTensorImplicitFunctionToFunctionSet::FunctionValues(vtkFloatingPointType* x, vtkFloatingPointType* res){
  vtkFloatingPointType f[9];
  vtkFloatingPointType * val[3];
  vtkFloatingPointType trace;
  vtkFloatingPointType correction;
  int i,j;

  for ( i=0 ; i < 3 ; i++ )
    {
      val[i] = &(f[3*i]);
    }
  if ( GetTensor(x,f) ) {
    LastFractionalAnisotropy = 2*(f[0]*f[0] + f[4]*f[4] + f[8]*f[8]) + 4*(f[1]*f[1]+f[2]*f[2]+f[5]*f[5]);
    trace = (f[0] + f[4] + f[8])/3;
    LastFractionalAnisotropy = sqrt(3*((f[0]-trace)*(f[0]-trace)+(f[4]-trace)*(f[4]-trace)+(f[8]-trace)*(f[8]-trace)+2*(f[1]*f[1]+f[2]*f[2]+f[5]*f[5]))/LastFractionalAnisotropy);
    if ( LastFractionalAnisotropy > LowerBoundBias && LastFractionalAnisotropy < UpperBoundBias && this->DirectionValid) {
      correction = ( UpperBoundBias - LastFractionalAnisotropy ) / ( UpperBoundBias - LowerBoundBias );
      for (i = 0 ; i < 3 ; i++ ){
    for (j = 0 ; j < 3 ; j++){
      val[i][j] += eigVal[0]*correction*CorrectionBias*Direction[i]*Direction[j];
    }
      }
      //vtkMath::Jacobi(val,eigVal,eigVec);
     vtkDiffusionTensorMathematics::TeemEigenSolver(val,eigVal,eigVec);
    }
    //vtkMath::Jacobi(val,eigVal,eigVec);
    vtkDiffusionTensorMathematics::TeemEigenSolver(val,eigVal,eigVec);
    for ( i=0; i < 3 ; i++ ) 
      {
    res[i] = eigVec[i][this->IntegrationDirection];
      }
    if (  vtkMath::Dot(Direction,res) < 0 )
      {
    for ( i=0; i < 3 ; i++ ) 
      {
        res[i] = -res[i];
      }
      }
    return 1;
    
  }  
  else {
    for ( i = 0 ; i < 3 ; i++ )
      res[i] = 0;
    return 0;
  }
}


void vtkTensorImplicitFunctionToFunctionSet::SetIntegrationForward(void) {
  this->Direction[0] = 0.0;
  this->Direction[1] = 0.0;
  this->Direction[2] = 1.0;
  this->DirectionValid = 0;
}

void vtkTensorImplicitFunctionToFunctionSet::SetIntegrationBackward(void) {
  this->Direction[0] = 0.0;
  this->Direction[1] = 0.0;
  this->Direction[2] = -1.0;
  this->DirectionValid = 0;
}

void vtkTensorImplicitFunctionToFunctionSet::SetChangeIntegrationDirection(void) {
  this->Direction[0] = -this->Direction[0];
  this->Direction[1] = -this->Direction[1];
  this->Direction[2] = -this->Direction[2];
}

void vtkTensorImplicitFunctionToFunctionSet::SetIntegrationDirection(vtkFloatingPointType dir[3]) {
  this->Direction[0] = dir[0];
  this->Direction[1] = dir[1];
  this->Direction[2] = dir[2];
  this->DirectionValid = 1;
}

int vtkTensorImplicitFunctionToFunctionSet::IsInsideImage(vtkFloatingPointType x[3]) {
  vtkFloatingPointType temp;
  int i;
  for ( i = 0 ; i < 3 ; i++ ) {
    temp = (x[i] - DataOrigin[i])/DataSpacing[i];
    if ( temp < ((vtkFloatingPointType)this->DataExtent[2*i]) || temp > ((vtkFloatingPointType)this->DataExtent[2*i+1]) )
      return 0;
  }
  return 1;
}

#endif
