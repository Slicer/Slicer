/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMGeneral.h,v $
  Date:      $Date: 2006/12/29 05:16:05 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
// .NAME vtkImageEMGeneral
 
// The idea of vtkImageEMGeneral is to include all the tools we need for the 
// different EM Segmentation tools. The tool is put together in four parts
// - the special made header files
// - files needed so it is a vtk filter
// - Genral Math Functions 

// ------------------------------------
// Standard EM necessaties
// ------------------------------------
#ifndef __vtkImageEMGeneral_h
#define __vtkImageEMGeneral_h
#include "vtkEMSegment.h"
#include <math.h>
#include <cmath>
#include "vtkMath.h"
#include "vtkImageMultipleInputFilter.h"
#include "vtkImageThreshold.h"
#include "vtkImageMathematics.h"
#include "vtkImageAccumulate.h"
#include "vtkImageData.h" 
// Just made for vtkImageEMGeneral and its kids
#include "vtkDataDef.h"
#include "vtkFileOps.h" 
#include "vtkImageReader.h"



// ------------------------------------
// Definitions for gauss calculations
// ------------------------------------
// Abuse the type system.
//BTX
// this macro should not be used---the results are undefined
//#define COERCE(x, type) (*((type *)(&(x))))
// Some constants having to do with the way single
// floats are represented on alphas and sparcs
#define EMSEGMENT_MANTSIZE (23)
#define EMSEGMENT_SIGNBIT (1 << 31)
#define EMSEGMENT_EXPMASK (255 << EMSEGMENT_MANTSIZE)
#define EMSEGMENT_MENTMASK ((~EMSEGMENT_EXPMASK)&(~EMSEGMENT_SIGNBIT))
#define EMSEGMENT_PHANTOM_BIT (1 << EMSEGMENT_MANTSIZE)
#define EMSEGMENT_EXPBIAS 127
#define EMSEGMENT_SHIFTED_BIAS (EMSEGMENT_EXPBIAS << EMSEGMENT_MANTSIZE)
#define EMSEGMENT_SHIFTED_BIAS_COMP ((~ EMSEGMENT_SHIFTED_BIAS) + 1)

  // #define EMSEGMENT_ONE_OVER_2_PI 0.5/3.14159265358979
#define EMSEGMENT_ONE_OVER_2_PI 0.15915494309190
  // #define EMSEGMENT_ONE_OVER_ROOT_2_PI sqrt(EMSEGMENT_ONE_OVER_2_PI)
#define EMSEGMENT_ONE_OVER_ROOT_2_PI 0.39894228040144
#define EMSEGMENT_ROOT_2_PI 2.50662827463
#define EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 ((float) -.72134752)
#define EMSEGMENT_E  2.7182818284590452353
#define EMSEGMENT_SQRT_E 1.64872127070013

// Definitions for Markof Field Approximation
#define EMSEGMENT_NORTH     1 
#define EMSEGMENT_SOUTH     2
#define EMSEGMENT_EAST      4 
#define EMSEGMENT_WEST      8 
#define EMSEGMENT_FIRST    16
#define EMSEGMENT_LAST     32 
#define EMSEGMENT_DEFINED  64
#define EMSEGMENT_INCORRECT_MODEL  64
#define EMSEGMENT_CORRECT_MODEL  ~EMSEGMENT_INCORRECT_MODEL
#define EMSEGMENT_NOTROI  128


//Definitions For Gauss Lookuptable
#define EMSEGMENT_TABLE_SIZE          64000   // For one image version
#define EMSEGMENT_TABLE_SIZE_MULTI    10000   // For multiple images
#define EMSEGMENT_TABLE_EPSILON       1e-4    // Cut of for smallest eps value: x < Eps  => GausTable(x) = 0
#define EMSEGMENT_TABLE_EPSILON_MULTI 1e-3    // Cut of for smallest eps value: x < Eps  => GausTable(x) = 0

// How many quality measures are used 
#define EMSEGMENT_NUM_OF_QUALITY_MEASURE 1

// the same as 1 / (e^6 - 1) - needed for Mean Field Approximation
#define EMSEGMENT_INVERSE_NEIGHBORHOOD_ENERGY 0.00248491165684


//ETX
class VTK_EMSEGMENT_EXPORT vtkImageEMGeneral : public vtkImageMultipleInputFilter
{
  public:
  // -------------------------------
  // General vtk Functions
  // -------------------------------
  static vtkImageEMGeneral *New();
  vtkTypeMacro(vtkImageEMGeneral,vtkObject);
  void PrintSelf(ostream& os) { };
//Kilian
//BTX
  void SetInputIndex(int index, vtkImageData *image) {this->SetInput(index,image);}

  void PrintMatrix(double **mat, int yMax,int xMax); 
  void PrintMatrix3D(double ***mat, int zMax,int yMax,int xMax); 

  // Description:
  // Calculated the determinant for a dim dimensional matrix -> the value is returned 
  static double determinant(double **mat,int dim); 

  // Description:
  // Inverts the matrix -> Returns 0 if it could not do it 
  static int InvertMatrix(double **mat, double **inv_mat,int dim);

  // Description:
  // Just squares the matrix 
  static void SquareMatrix(double **Input,double **Output,int dim);

  // Description:
  // Multiplies the Matrix mat with the vector vec => res = mat * vec 
  static void MatrixVectorMulti(double **mat,double *vec,double *res,int dim);

  // Description :
  // Smoothes  3D-Matrix -> w(k) = sum(u(j)*v(k+1-j)) -> returns Matrix of size r_m
  // void smoothConv(double ***mat3D, int mat3DZlen, int mat3DYlen, int mat3DXlen, double v[],int vLen);

  // Description :
  // -------------------------------------------------------------------------------------------------------------------
  // CalculateLogMeanandLogCovariance - for multiple Image
  //
  // Input: Mu                  = Matrix with Mean values for every tissue class (row) and image (column) => [NumberOfClasses]x[NumberOfInputImages]
  //        CovMatrix           = Covariance matrix's diagonal for every tissue class (row) and image (column) => [NumberOfClasses]x[NumberOfInputImages]x[NumberOfInputImages]
  //        LogMu               = Log Mean Values calculated by the function
  //        LogCov              = Log Covariance Values calculated by the function
  //        NumberOfInputImages = Number of Input Images 
  //        NumberOfClasses     = Number of Classes 
  //        SequenceMax         = Maximum "grey value" for every image the mean and sigma value should be computed
  // 
  // Output: If return value is 0 => could not invert coveriance matrix  
  //     
  // Idea: Calculates the MuLog Vecotr and CovarianceLog Matrix given mu and CovDiag for every tissue class. To calculate MuLog 
  //       and LogCovariance we normally need samples from the image. In our case we do not have any samples, because we do not 
  //       know how the image is segmented to get around it we generate a Testsequence T where
  //       T[i] =  p(i)*log(i+1) with  
  //       p(i) = the probability that grey value i appears in the image given sigma and mu = Gauss(i, Sigma, Mu)
  //       and i ranges from [0,SequenceMax]   
  // -------------------------------------------------------------------------------------------------------------------
  static int CalculateLogMeanandLogCovariance(double **Mu, double ***CovMatrix, double **LogMu, double ***LogCov,int NumberOfInputImages, int  NumberOfClasses, int SequenceMax); 

  // Description:
  // -------------------------------------------------------------------------------------------------------------------
  // CalculateLogMeanandLogCovariance - for 1 Image
  //
  // Input: mu              = Vector with Mean values for every tissue class
  //        Sigma           = Vector with Sigma values for every tissue class
  //        logmu           = Log Mean Values calculated by the function
  //        logSigma        = Log Sigma Values calculated by the function - this case is standard deviation
  //                          or sqrt(variation)
  //        NumberOfClasses = Number of classes
  //        SequenceMax     = Maximum "grey value" the mean and sigma value should be computed
  //        
  // Idea: Calculates the MuLog and SigmaLog values given mu and sigma. To calculate MuLog and SigmaLog we 
  //       normally need samples from the image. In our case we do not have any samples, because we do not 
  //       know how the image is segmented to get around it we generate a Testsequence T where
  //       T[i] =  p(i)*log(i+1) with  
  //       p(i) = the probability that grey value i appears in the image given sigma and mu = Gauss(i, Sigma, Mu)
  //       and i ranges from [0,SequenceMax]   
  // -------------------------------------------------------------------------------------------------------------------
  static void CalculateLogMeanandLogCovariance(double *mu, double *Sigma, double *LogMu, double *LogSigma, int NumberOfClasses, int SequenceMax);

  // Description:
  // -------------------------------------------------------------------------------------------------------------------
  // CalculatingPJointDistribution
  //
  // Input: x                = set input values, x has to be of size numvar and the set variables have to be defined,
  //                           e.g. index n and m are set => y[n] and y[m] have to be defined
  //        Vleft            = all the vairables that are flexible => e.g Vleft = [1,...,n-1,n+1, .., m-1, m+1 ,..., numvar]
  //        mu               = Mean Values of the distribution of size numvar
  //        invcov           = The covariance Matrix's inverse of the distribution - has to be of size numvarxnumvar  
  //        inv_sqrt_det_cov = The covariance Matrix' determinant squared and inverted
  //        SequenceMax      = Test sequence for the different images - has to be of size numvar
  //        setvar           = How many veriable are allready fixed 
  //        numvar           = Size of Varaible Space V
  //        
  // Idea: Calculates the joint probability of the  P(i= V/Vleft: y[i] =x[i]) , e.g. P(y[n] = x[n], y[m] = x[m])  
  // Explanation: V = {y[1] ... y[numvar]}, V~ = Vleft = V / {y[n],y[m]}, and y = X <=> y element of X 
  //              =>  P(y[n] = x[n],y[m] = x[m]) = \sum{k = V~} (\sum{ l = [0..seq[y]]} P(y[n] = x[n],y[m] = x[m],y[k] = l,V~\{ y[k]}) 
  // -------------------------------------------------------------------------------------------------------------------
  static double CalculatingPJointDistribution(float* x,int *Vleft,double *mu, double **inv_cov, double inv_sqrt_det_cov,int SequenceMax, int setvar,int numvar);
  
  // -------------------------------
  // Gauss Functions
  // -------------------------------
  // Description :
  // Normal Gauss Function - pritty slow 
  static double GeneralGauss(double x,double m,double s);
  // Description :
  // Normal Gauss Function for 3D - pritty slow 
  // Input: 
  // x                = Value Vector of dimension n
  // mu               = Mean Value Vector of dimension n
  // inv_cov          = The inverse of the coveriance Matrix
  // inv_sqrt_det_cov = The covariance matrix's determinant sqrt and inverted = sqrt(inv_cov)
  static double GeneralGauss(float *x,double *mu,double **inv_cov, double inv_sqrt_det_cov,int n);

  // Description :
  // 3xfaster Gauss Function written by Sandy
  static double FastGauss(const double inverse_sigma, const double x);
  static double FastGaussTest(const double inverse_sigma, const double x);
  // Description:
  // Special feature necessary bc we use weighted input images thus a matix of realDim 2 can be virtualDim 1
  // e.g. 1 0 | 0 0
  static float FastGauss2(const double inverse_sqrt_det_covariance, const float *x ,const double *mu, double **inv_cov, const int virtualDim );

  // Description :
  // Same as FastGauss - just for multi dimensional input ->  x = (vec - mu) * InvCov *(vec - mu)
  static float FastGaussMulti(const double inverse_sqrt_det_covariance, const float x,const int dim);

  // Description :
  // Same as FastGauss - just for multi dimensional input 
  // Special feature necessary bc we use weighted input images thus a matix of realDim 2 can be virtualDim 1
  // e.g. 1 0 | 0 0
  static float FastGaussMulti(const double inverse_sqrt_det_covariance, const float* x,const double *mu, double **inv_cov, const int realDim, const int virtualDim);

  // Description :
  // Fastes Gauss Function (jep I wrote it) - just look in a predifend lookup table 
  static double LookupGauss(double* table, double lbound, double ubound, double resolution,double value);
  static double LookupGauss(double* table, double *lbound, double *ubound, double *resolution,double *value,int NumberOfInputImages);
  // Description:
  // Calculate the Gauss-Lookup-Table for one tissue class 
  static int CalculateGaussLookupTable(double *GaussLookupTable,double **ValueTable,double **InvCovMatrix, double InvSqrtDetCovMatrix, 
                                       double *ValueVec,int GaussTableIndex,int TableSize,int NumberOfInputImages, int index);
  // Description :
  // Calculates DICE and Jakobian Simularity Measure 
  // Value defines the vooxel with those label to be measured
  // Returns  Dice sim measure
  static float CalcSimularityMeasure (vtkImageData *Image1, vtkImageData *Image2,float val, int PrintRes, int *BoundaryMin, int *BoundaryMax);

//Kilian
//ETX
  static float CalcSimularityMeasure (vtkImageData *Image1, vtkImageData *Image2,float val, int PrintRes);

  // Description:
  // Opens text file with the a file name according to Kilian's EM nameing convention 
  static FILE* OpenTextFile(const char* FileDir, const char FileName[], int Label, int LabelFlag, 
                const char *LevelName, int LevelNameFlag, int iter, int IterFlag, 
                const char FileSucessMessage[],char OpenFileName[]);

  void* GetPointerToVtkImageData(vtkImageData *Image, int DataType, int Ext[6]); 
  void  GEImageReader(vtkImageReader *VOLUME, const char FileName[], int Zmin, int Zmax, int ScalarType);
  int   GEImageWriter(vtkImageData *Volume, char *FileName,int PrintFlag);


protected:
  vtkImageEMGeneral() {};
  vtkImageEMGeneral(const vtkImageEMGeneral&) {};
  ~vtkImageEMGeneral() {};

  void DeleteVariables();
  void operator=(const vtkImageEMGeneral&) {};
  void ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id){};
//Kilian
//BTX
  // -------------------------------
  // Matrix Functions
  // -------------------------------
  // Description:
  // Writes Vector to file in Matlab format if name is specified otherwise just 
  // writes the values in the file
  // void WriteVectorToFile (FILE *f,char *name, double *vec, int xMax) const;

  // Description:
  // Writes Matrix to file in Matlab format if name is specified otherwise just 
  // writes the values in the file
  // void WriteMatrixToFile (FILE *f,char *name,double **mat, int imgY, int imgX) const;

  // Description:
  // Convolution and polynomial multiplication . 
  // This is assuming u and 'this' have the same dimension
  static void convMatrix3D(double*** mat3D, double*** U,int mat3DZlen, int mat3DYlen, int mat3DXlen, double v[],int vLen);

  // Description:
  // Convolution and polynomial multiplication . 
  // This is assuming u and 'this' have the same dimension
  // Convolution and polynomial multiplication . 
  // This is assuming u and 'this' have the same dimension
  // static void convMatrix(double** mat, double** U, int matYlen, int matXlen, double v[], int vLen);

  // Description:
  // Same just v is a row vector instead of column one
  // We use the following equation :
  // conv(U,v) = conv(U',v')' => conv(U,v') = conv(U',v)';
  // static void convMatrixT(double** mat, double** U, int matYlen, int matXlen, double v[], int vLen);

  // Description:
  // Calculates Vector * Matrix * Vector
  static double CalculateVectorMatrixVectorOperation(double** mat, double *vec, int offY,int dimY, int offX, int dimX);

  // Description:                                                           
  // Calculates the inner product of <vec, mat[posY:dimY-1][posX]> = vec * mat[posY:dimY-1][posX]
  static double InnerproductWithMatrixY(double* vec, double **mat, int posY,int dimY, int posX);

  // Description:                                                             
  // Product of mat[posY][posX:dimY-1]*vec
  static double InnerproductWithMatrixX(double **mat, int posY,int posX, int dimX, double *vec);
 
  // Description:
  // -------------------------------------------------------------------------------------------------------------------
  // CalculateLogMeanandLogCovariance - for 1 Image
  //
  // Input: mu              = Vector with Mean values for every tissue class
  //        Sigma           = Vector with Sigma values for every tissue class
  //        LogMu           = Log Mean Values calculated by the function
  //        LogVariance     = Log Variance Values calculated by the function
  //        LogTestSequence = Sequence of log(i+1) with i ranges from [0,SequenceMax]
  //        SequenceMax     = Maximum "grey value" the mean and sigma value should be computed
  //        
  // Idea: Calculates the MuLog and SigmaLog values given mu and sigma. To calculate MuLog and SigmaLog we 
  //       normally need samples from the image. In our case we do not have any samples, because we do not 
  //       know how the image is segmented to get around it we generate a Testsequence T where
  //       T[i] =  p(i)*log(i+1) with  
  //       p(i) = the probability that grey value i appears in the image given sigma and mu = Gauss(i, Sigma, Mu)
  //       and i ranges from [0,SequenceMax]   
  // -------------------------------------------------------------------------------------------------------------------
  static void CalculateLogMeanandLogCovariance(double *mu, double *Sigma, double *LogMu, double *LogVariance,double *LogTestSequence, int NumberOfClasses, int SequenceMax); 
 
  void TestMatrixFunctions(int MatrixDim,int iter);

//Kilian
//ETX
};

//BTX 
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

inline float vtkImageEMGeneral_qnexp2(float const x)
{
    unsigned result_bits;

    // use a union to avoid violating aliasing rules
    union { float f; unsigned int ui; } fuiu;
    fuiu.f = x;
    unsigned bits = fuiu.ui;

    int exponent = ((EMSEGMENT_EXPMASK & bits) >> EMSEGMENT_MANTSIZE) - (EMSEGMENT_EXPBIAS);
    int neg_mant =   - (int)((EMSEGMENT_MENTMASK & bits) | EMSEGMENT_PHANTOM_BIT);

    unsigned r1 = (neg_mant << exponent);
    unsigned r2 = (neg_mant >> (- exponent));

    result_bits = (exponent < 0) ? r2 : r1;
    result_bits = (exponent > 5) ? EMSEGMENT_SHIFTED_BIAS_COMP  : result_bits;
    
    result_bits += EMSEGMENT_SHIFTED_BIAS;

    fuiu.ui = result_bits;
#ifdef DEBUG
    {
    float result;
    result = fuiu.f;
    fprintf(stderr, "x %g, b %x, e %d, m %x, R %g =?",
           x,     bits, exponent,  neg_mant, pow(2.0, x));
    fflush(stderr);
    fprintf(stderr, " %g\n", result);
    }
#endif
    return fuiu.f;
}

// An approximation to the Gaussian function.
// The error seems to be a six percent ripple.
inline double vtkImageEMGeneral::FastGauss(const double inverse_sigma, const double x)
{
    float tmp = float(inverse_sigma * x);
    return (double) EMSEGMENT_ONE_OVER_ROOT_2_PI * inverse_sigma * vtkImageEMGeneral_qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * tmp * tmp);
}

// An approximation to the Gaussian function.
// The error seems to be a six percent ripple.
inline double vtkImageEMGeneral::FastGaussTest(const double inverse_sigma, const double x)
{
    float tmp = float(inverse_sigma * x);
    return (double) EMSEGMENT_ONE_OVER_ROOT_2_PI * inverse_sigma 
    * vtkImageEMGeneral_qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * tmp * tmp);
}

 // Same as FastGauss - just for 2 Dimensional multi dimensional input 
 // Special feature necessary bc we use weighted input images thus a matix of realDim 2 can be virtualDim 1
 // e.g. 1 0 | 0 0
inline float vtkImageEMGeneral::FastGauss2(const double inverse_sqrt_det_covariance, const float *x ,const double *mu,  double **inv_cov, const int virtualDim) {
  float term1 = x[0] - float(mu[0]),
        term2 = x[1] - float(mu[1]);
  // Kilian: can be done faster: term1*(inv_cov[0][0]*term1 + 2.0*inv_cov[0][1]*term2) + term2*term2*inv_cov[1][1];
  term2 = term1*(float(inv_cov[0][0])*term1 + float(inv_cov[0][1])*term2) + term2*(float(inv_cov[1][0])*term1 + float(inv_cov[1][1])*term2);
  if (virtualDim > 1) return EMSEGMENT_ONE_OVER_2_PI * float(inverse_sqrt_det_covariance)  * vtkImageEMGeneral_qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * term2);
  return EMSEGMENT_ONE_OVER_ROOT_2_PI * float(inverse_sqrt_det_covariance)  * vtkImageEMGeneral_qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * term2);
}

// Same as FastGauss - just for multi dimensional input ->  x = (vec - mu) * InvCov *(vec - mu) 
inline float vtkImageEMGeneral::FastGaussMulti(const double inverse_sqrt_det_covariance, const float x,const int dim) {
  return pow(EMSEGMENT_ONE_OVER_ROOT_2_PI,dim) * inverse_sqrt_det_covariance * vtkImageEMGeneral_qnexp2(EMSEGMENT_MINUS_ONE_OVER_2_LOG_2 * x);
}

 // Special feature necessary bc we use weighted input images thus a matix of realDim 2 can be virtualDim 1
 // e.g. 1 0 | 0 0
inline float vtkImageEMGeneral::FastGaussMulti(const double inverse_sqrt_det_covariance, const float* x,const double *mu, double **inv_cov, const int realDim, const int virtualDim) {
  if (realDim <2) return (float) vtkImageEMGeneral::FastGauss(inverse_sqrt_det_covariance,x[0]- float(mu[0]));
  if (realDim <3) return vtkImageEMGeneral::FastGauss2(inverse_sqrt_det_covariance, x ,mu,inv_cov, virtualDim);
  float *x_m = new float[realDim];
  float term = 0;
  int i,j; 
  for (i=0; i < realDim; i++) x_m[i] = x[i] - float(mu[i]);
  for (i=0; i < realDim; i++) {
    for (j=0; j < realDim; j++) term += (float(inv_cov[i][j])*x_m[j]);
    term *= x_m[i];
  }
  delete []x_m;
  return vtkImageEMGeneral::FastGaussMulti(inverse_sqrt_det_covariance, term,virtualDim);        
}


//ETX

#endif







