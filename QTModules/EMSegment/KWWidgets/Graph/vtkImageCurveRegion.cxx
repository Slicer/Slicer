/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageCurveRegion.cxx,v $
  Date:      $Date: 2006/04/13 19:29:02 $
  Version:   $Revision: 1.13 $

=========================================================================auto=*/
#include "vtkImageCurveRegion.h"

#include "vtkImageData.h"
#include "vtkImageProgressIterator.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkStreamingDemandDrivenPipeline.h"

vtkCxxRevisionMacro(vtkImageCurveRegion, "1.32");
vtkStandardNewMacro(vtkImageCurveRegion);


// ---------------------------------------------------------------------------------------------
//  -*- Mode: C++;  -*-
//  File: qgauss.hh
//  Author: Sandy Wells (sw@ai.mit.edu)
//  Copyright (C) MIT Artificial Intelligence Laboratory, 1995
// *------------------------------------------------------------------
// * FUNCTION:
// *
// * Implements an approximation to the Gaussian function.
// * It is based on a piecewise-linear approximation
// * to the 2**x function for negative arguments using integer arithmetic and
// * bit fiddling.  
// * origins: May 14, 1995, sw.  
// *
// * On an alpha qgauss is about 3 times faster than vanilla gaussian.
// * The error seems to be a six percent ripple.
// *
// * HISTORY:
// * Last edited: Nov  3 15:26 1995 (sw)
// * Created: Wed Jun  7 02:03:35 1995 (sw)
// *------------------------------------------------------------------

// A piecewise linear approximation to 2**x for negative arugments
// Provides exact results when the argument is a power of two,
// and some other times as well.
// The strategy is rougly as follows:
//    coerce the single float argument to unsigned int
//    extract the exponent as a signed integer
//    construct the mantissa, including the phantom high bit, and negate it
//    construct the result bit pattern by leftshifting the signed mantissa
//      this is done for both cases of the exponent sign
//      and check for potenital underflow

// Does no conditional branching on alpha or sparc :Jun  7, 1995

inline float qnexp2(float x)
{
    unsigned result_bits;
    unsigned bits = COERCE(x, unsigned int);
    int exponent = ((EMSEGMENT_EXPMASK & bits) >> EMSEGMENT_MANTSIZE) - (EMSEGMENT_EXPBIAS);
    int neg_mant =  ((EMSEGMENT_MENTMASK & bits) | EMSEGMENT_PHANTOM_BIT);
    neg_mant = -neg_mant;

    unsigned r1 = (neg_mant << exponent);
    unsigned r2 = (neg_mant >> (- exponent));

    result_bits = (exponent < 0) ? r2 : r1;
    result_bits = (exponent > 5) ? EMSEGMENT_SHIFTED_BIAS_COMP  : result_bits;
    
    result_bits += EMSEGMENT_SHIFTED_BIAS;

#ifdef DEBUG
    {
    float result;
    result = COERCE(result_bits, float);
    fprintf(stderr, "x %g, b %x, e %d, m %x, R %g =?",
           x,     bits, exponent,  neg_mant, pow(2.0, x));
    fflush(stderr);
    fprintf(stderr, " %g\n", result);
    }
#endif
    return(COERCE(result_bits, float));
}

// Kilian : Copied from vtkImageEMGeneral.h - later when versions are merged should be integrated
// An approximation to the Gaussian function.
// The error seems to be a six percent ripple.
inline float FastGauss(float inverse_sigma, float x) {
    float tmp = float(inverse_sigma * x);
    return EMSEGMENT_ONE_OVER_ROOT_2_PI * inverse_sigma 
    * qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * tmp * tmp);
}
// Same as FastGauss - just for 2 Dimensional multi dimensional input 
inline float FastGauss2(float inverse_sqrt_det_covariance, float *x ,float *mu, float **inv_cov) {
  float term1 = x[0] - mu[0],
         term2 = x[1] - mu[1];
  // Kilian: can be done faster: term1*(inv_cov[0][0]*term1 + 2.0*inv_cov[0][1]*term2) + term2*term2*inv_cov[1][1];
  term2 = term1*(inv_cov[0][0]*term1 + inv_cov[0][1]*term2) + term2*(inv_cov[1][0]*term1 + inv_cov[1][1]*term2);
  return EMSEGMENT_ONE_OVER_2_PI * inverse_sqrt_det_covariance  * qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * float(term2));
}

// copied from vtkImageEMGeneral
// Calculated the determinant for a dim dimensional matrix -> the value is returned 
// Faster with LU decomposition - look in Numerical Recipecs
inline float determinant(float **mat,int dim) {
  if (dim < 2) return mat[0][0];
  if (dim < 3) return mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
  if (dim < 4) return mat[0][0]*mat[1][1]*mat[2][2] + mat[1][0]*mat[2][1]*mat[0][2] + 
                      mat[2][0]*mat[0][1]*mat[1][2] - mat[0][0]*mat[2][1]*mat[1][2] - 
                      mat[1][0]*mat[0][1]*mat[2][2] - mat[2][0]*mat[1][1]*mat[0][2];
  int j,k,i;
  float result = 0;
  float **submat = new float*[dim-1];
  for (i=0; i< dim-1; i++) submat[i] = new float[dim-1];

  for (j = 0 ; j < dim ; j ++) {
    if (j < 1) {
      for (k=1 ; k < dim; k++) {
    for (i=1; i < dim; i++)
      submat[k-1][i-1] = mat[k][i];  
      }
    } else {
      for (i=1; i < dim; i++) submat[j-1][i-1] = mat[j-1][i];
    }

    result += (j%2 ? -1:1) * mat[0][j]*determinant(submat,dim-1);
  }

  for (i=0; i< dim-1; i++) delete[] submat[i];
  delete[] submat;

  return result;
}


// ---------------------------------
// Normal Gauss Function
// --------------------------------
inline float GeneralGauss(float term,float InvSigma) {
  term *= InvSigma;
  return (EMSEGMENT_ONE_OVER_ROOT_2_PI* InvSigma * exp(-0.5 *term*term));
}

// ----------------------------------------
// Normal Gauss Function for Multiple Input 
// ----------------------------------------
inline float GeneralGauss(float *x,float *mu,float **inv_cov, float inv_sqrt_det_cov,int n) {
  float *x_m = new float[n];
  float term1, 
        term  = 0.0;

  int i,j; 

  for (i=0; i < n; i++) x_m[i] = x[i] - mu[i];
  for (i=0; i < n; i++) {
    term1 = 0.0;
    for (j=0; j < n; j++) term1 += (inv_cov[i][j]*x_m[j]);
    term += term1*x_m[i];
  }
  delete[] x_m;
  return (pow(EMSEGMENT_ONE_OVER_ROOT_2_PI,n)*inv_sqrt_det_cov * exp(-0.5 *term));
}

inline int InvertMatrix(float **mat, float **inv_mat,int dim) {
  float det;
  if (dim < 2) {
    if (mat[0][0] == 0) return 0;
    inv_mat[0][0] = 1.0 / mat[0][0];
    return 1;
  } 

  det = determinant(mat,2);
  if (fabs(det) <  1e-15 ) return 0;
  det = 1.0 / det;
  inv_mat[0][0] = det * mat[1][1];
  inv_mat[1][1] = det * mat[0][0];
  inv_mat[0][1] = -det * mat[0][1];
  inv_mat[1][0] = -det * mat[1][0];
  return 1;
}


//----------------------------------------------------------------------------
vtkImageCurveRegion::vtkImageCurveRegion()
{
  this->Xmin = this->Xmax = this->Ymin = this->Ymax = this->FctMax = this->FctMin = 0.0;
  this->Xunit = this->Yunit = - 1.0;
  this->Xlength = this->Ylength = -1;

  this->Dimension = this->Function = 0;
  this->Mean = NULL;
  this->Covariance = NULL;
  this->Probability = 1.0;
  this->FileName = NULL;
  this->SetNumberOfInputPorts(0);
}

//----------------------------------------------------------------------------
void vtkImageCurveRegion:: DeleteVariables() {
  if (this->Dimension > 0 ) {
    for (int y =0 ; y < this->Dimension; y++) delete [] this->Covariance[y];
    delete [] this->Covariance; 
    delete [] this->Mean;
  }
  this->Dimension = 0;
}

//----------------------------------------------------------------------------
void vtkImageCurveRegion::SetFunction(int val) {
  if ((3<val) || (val < 1)) {
    vtkErrorMacro(<< "Currently Function has to bedefined as 1 (for log gaussian) or 2 (normal gaussian) !");
    return;
  }
  if (val == this->Function) return;
  this->Function = val;
  this->Modified(); 
}

//----------------------------------------------------------------------------
void vtkImageCurveRegion::SetDimension (int value){
  if ((2<value) || (value < 1)) {
    vtkErrorMacro(<< "Currently dimension has to be 1 or 2 !");
    return;
  }
  if (value == this->Dimension) return;
  this->DeleteVariables();
  this->Dimension = value;
  this->Mean = new float[this->Dimension];
  memset(this->Mean,0,this->Dimension*sizeof(float));
  this->Covariance = new float*[this->Dimension];
  for (int y = 0; y < this->Dimension; y++) {
    this->Covariance[y] = new float[this->Dimension];
    memset(this->Covariance[y],0,this->Dimension*sizeof(float));
  }
  this->Modified();
}

void vtkImageCurveRegion::ExecuteDataGauss(vtkDataObject *output) {

  vtkImageData *data = this->AllocateOutputData(output);
  if (data->GetScalarType() != VTK_FLOAT) {
    vtkErrorMacro("Execute: This source only outputs floats");
    return;
  }

  float **InvCov = new float*[this->Dimension];
  for (int i = 0 ; i < this->Dimension; i++)
    {
       InvCov[i] = new float[this->Dimension];
    } 
  
  // This is copied from vtkImageEMLocalSegment.cxx EMClass::CalcInvCovParamters()
  if (InvertMatrix(this->Covariance,InvCov,this->Dimension) == 0) {
    vtkErrorMacro(<< "Could not caluclate the Inverse of the Covariance of tissue class. Covariance Matrix is probably almost singular!");
    for (int i = 0 ; i < this->Dimension; i++)
    {
      delete[] InvCov[i];
      InvCov[i] = NULL;
    } 
    delete [] InvCov;

    return;
  }

  float DetCov;
  DetCov = determinant(this->Covariance,this->Dimension);
  if (DetCov <= 0.0) {
    vtkErrorMacro(<< "Coveriance Matrix (Det= " << DetCov <<") for tissue class is probably almost singular or not positiv! Could not calculate the inverse determinant of it ");
    return;
  }

  float *outPtr;
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  vtkIdType outIncX, outIncY, outIncZ;
  int rowLength;
  int *outExt;
  float *value   = new float[this->Dimension];
  for (idxR = 0 ; idxR < this->Dimension; idxR++) InvCov[idxR] = new float[this->Dimension];
  float InvSqrtDetCov;
  bool FlagInit = false;

  InvSqrtDetCov = sqrt(1/DetCov);

  float *LogValue = new float[this->Dimension];

  outExt = data->GetExtent();
  
  // find the region to loop over
  rowLength = (outExt[1] - outExt[0]+1);
  maxY = outExt[3] - outExt[2] + 1; 
  maxZ = outExt[5] - outExt[4] + 1;
  
  // Get increments to march through data 
  data->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  outPtr = (float *) data->GetScalarPointer(outExt[0],outExt[2],outExt[4]);

  // Loop through ouput pixels
  for (idxZ = 0; idxZ < maxZ; idxZ++) {
    if (this->Dimension > 1) value[1] = this->Ymin;
    for (idxY = 0; idxY < maxY; idxY++) {
      value[0] = this->Xmin;
      for (idxR = 0; idxR < rowLength; idxR++) {
        // Pixel operation
        if (this->Dimension > 1) {
          switch (this->Function) {
            case 1:  *outPtr = Probability*FastGauss2(InvSqrtDetCov,value,this->Mean, InvCov);
                 break;

                case 2:  LogValue[0] = log (value[0] +1.0); 
                    *outPtr = Probability*FastGauss2(InvSqrtDetCov,LogValue,this->Mean, InvCov);
              // *outPtr = Probability * GeneralGauss(LogValue,this->Mean,InvCov, InvSqrtDetCov,2);  
                 break;
          }
        } else {
              switch (this->Function) {
                  case 1:  *outPtr = Probability*FastGauss(InvSqrtDetCov,value[0] - this->Mean[0]);
                  // *outPtr = Probability* GeneralGauss(value[0] - this->Mean[0],InvSqrtDetCov);
                  break;
                  case 2:  *outPtr = Probability*FastGauss(InvSqrtDetCov,log(value[0] +1.0) - this->Mean[0]);
                  // *outPtr = Probability* GeneralGauss(log(value[0] + 1.0) - this->Mean[0],InvSqrtDetCov);
                  break;
          }
        }
        // Define Maximum and Minimum value of the function 
        if (!FlagInit) {
          this->FctMin = this->FctMax = *outPtr;
          FlagInit = true;
        } else {
          if (*outPtr > this->FctMax) this->FctMax = *outPtr;
          else if (*outPtr < this->FctMin) this->FctMin = *outPtr;
        }

        outPtr++;
        value[0] += this->Xunit;
      }
      if (this->Dimension >1) 
      { value[1] += this->Yunit;
        if (this->Function == 2) LogValue[1] = log(value[1] + 1.0);
      }
      outPtr += outIncY;
    }
    outPtr += outIncZ;
  }
  delete[] LogValue;
  for (idxR = 0 ; idxR < this->Dimension; idxR++) delete[] InvCov[idxR];
  delete[] InvCov;
  delete[] value;  
}

void vtkImageCurveRegion::ExecuteDataReadFile(vtkDataObject *output) {
  vtkImageData *data = this->AllocateOutputData(output);
  int *outExt = data->GetExtent();
  int Numbers = data->GetNumberOfScalarComponents();  
  if (Numbers == 0) {return;}
  int index = 0;
  int ReadNumber =0;
  int Lines =  outExt[1] - outExt[0]+1;

  // Set read format 
  float* outPtr = (float *) data->GetScalarPointer(outExt[0],outExt[2],outExt[4]);
  FILE *f = fopen(this->FileName,"r");
  if ( f == NULL ) {
    cerr << "vtkImageCurveRegion::ExecuteDataFileRead: Could not open file " << this->FileName << "\n";
    fclose(f);
    return;
  }

  index = 0;
  while ((index < Lines) && (!feof(f))) {
    ReadNumber = 0;   
    while  (ReadNumber < Numbers) {
      fscanf(f, "%f", outPtr++);
      ReadNumber ++;
    }
    index ++;
  }
  fclose(f);
}

//----------------------------------------------------------------------------
void vtkImageCurveRegion::PrintSelf(ostream& os, vtkIndent indent)
{
  int x ,y;
  this->Superclass::PrintSelf(os,indent);
  os << indent << "Dimension:  " << this->Dimension << endl;
  os << indent << "Function:   " << this->Function  << endl;
  os << indent << "Xmin:       " << this->Xmin      << endl;
  os << indent << "Xmax:       " << this->Xmax      << endl;
  os << indent << "Xunit:      " << this->Xunit     << endl;
  os << indent << "Ymin:       " << this->Ymin      << endl;
  os << indent << "Ymax:       " << this->Ymax      << endl;
  os << indent << "Yunit:      " << this->Yunit     << endl;
  os << indent << "Mean:       "; 
  for (x = 0 ; x < this->Dimension; x++ ) os << this->Mean[x] << " ";
  os << endl; 
  os << indent << "Covariance: ";
  for (y = 0 ; y < this->Dimension; y++ ) {
    for (x = 0 ; x <this->Dimension; x++ ) os << this->Covariance[y][x] << " ";
    if (y < (this->Dimension -1 )) os << "| ";
  }
  os << endl;
}

//----------------------------------------------------------------------------
int vtkImageCurveRegion::RequestInformation (
  vtkInformation * vtkNotUsed(request),
  vtkInformationVector** vtkNotUsed( inputVector ),
  vtkInformationVector *outputVector)
{

   int Extent[6];
  Extent[0] = Extent[2] = Extent[4] = Extent[5] = 0;
   int ScalarComp = 0;

  if (this->Function == 3 ) {
    FILE *f = fopen(this->FileName,"r");
    if ( f == NULL ) {
      cerr << "vtkImageCurveRegion::ExecuteInformation: Could not open file " << this->FileName << "\n";
      fclose(f);
      return 0 ;
    }
    int MaxLength = 100;
    int index = 0; 
    bool flag = false;
    char *line = new char[MaxLength];
    if (feof(f) || fgets(line, MaxLength, f ) == NULL) {
      cout << "vtkImageCurveRegion::ExecuteInformation: Error : File was empty !" << endl;
      fclose(f);
      delete[] line;
      return 0 ;
    }
    Extent[1] = Extent[3] = 0;

    line[MaxLength-1] = '\n';
    if (line[index] != ' ') ScalarComp = 1;
    // Check how many numbers are in a row 
    cout << line << endl;
    while(line[index] != '\n') {
      if (line[index] == ' ' ) flag = true;
      else if (flag) { 
    ScalarComp++;
    flag = false;
      } 
      index ++;
    }
    // Check How many rows there are
    fgets(line, MaxLength, f); 
    while(!feof(f)) {
      Extent[1] ++;
      fgets(line, MaxLength, f);
    }
    fclose(f);
    delete[] line;
  } else {
    float dist;
    ScalarComp = 1;
    if ((this->Xunit <= 0.0) && (this->Xlength <= 0)) {
       vtkErrorMacro(<< "Value for Xunit or Xlength has to be greater 0 !");
      return 0;
    }
    if (this->Xlength > 0) {
      dist = this->Xmax - this->Xmin;
      if (dist) {
    this->Xunit = dist / float(this->Xlength);
    while (dist/this->Xunit <  float(this->Xlength))  this->Xunit *= 0.999;
      } else this->Xunit = 1.0;
    } else {
      this->Xlength = int((this->Xmax - this->Xmin)/this->Xunit);
    }
    Extent[1] = this->Xlength - 1;
    
    if (this->Dimension == 2) {
      if ((this->Yunit <= 0.0) && (this->Ylength <= 0)) {
       vtkErrorMacro(<< "Value for Yunit or Ylength has to be greater 0 !");
       return 0 ;
      }
      if (this->Ylength > 0) {
    dist = this->Ymax - this->Ymin;
    if (dist) {
      this->Yunit = dist / float(this->Ylength);
      while (dist/this->Yunit <  float(this->Ylength))  this->Yunit *= 0.999;
    } else this->Yunit = 1.0;
      } else {
    this->Ylength = int((this->Ymax - this->Ymin)/this->Yunit);
      }
      Extent[3] = this->Ylength - 1;
    } else {Extent[3] = 0;}
   
  }
  vtkInformation* outInfo = outputVector->GetInformationObject(0);  
  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), Extent,6);
  vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_FLOAT, ScalarComp);
  return 1;
}

void vtkImageCurveRegion::ExecuteData(vtkDataObject *output)
{
   vtkImageData *data = this->AllocateOutputData(output);
  
   if (data->GetScalarType() != VTK_FLOAT)
    {
    vtkErrorMacro("Execute: This source only outputs floats");
    }
  
   switch (this->Function) {
    case 1: 
    case 2: this->ExecuteDataGauss(output); break;
    case 3: this->ExecuteDataReadFile(output); break;
    default:   cerr << "vtkImageCurveRegion::ExecuteData:Error: Function " << this->Function << "not available !" << endl;   
   }
}

