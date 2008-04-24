/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkSimonParameterReaderWriter.cxx,v $
  Date:      $Date: 2006/12/08 23:28:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// -------------------------------------------------------------------
/* Copyright (c) 1997 Simon Warfield simonw@bwh.harvard.edu */
#include "vtkSimonParameterReaderWriter.h"
#include "vtkObjectFactory.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <ctype.h>
vtkCxxRevisionMacro(vtkSimonParameterReaderWriter, "$Revision: 1.1 $");
vtkStandardNewMacro(vtkSimonParameterReaderWriter);


// from UtilsInC.c
#ifndef UCHAR
#define UCHAR(c) ((unsigned char)(c))
#endif

#ifndef anint
#define anint(x) ((x) >=0 ? (int)((x)+0.5) : (int)((x)-0.5))
#endif

#ifndef nint
#define nint(x) anint((x))
#endif


int countFloatsInString(const char *fltString)
/* char *flts - character array of floats 
 Return -1 on a malformed string
 Return the count of the number of floating point numbers
*/
{
  char *end;
  const char *start = fltString;
  double d;
  int count = 0;
  while ((UCHAR(*start) != '\0') && isspace(UCHAR(*start))) { start++; }
  if (UCHAR(*start) == '\0') return -1; /* Don't ask to convert empty strings */
  do {
    d = strtod(start, (char **)&end);
    if (end == start) { 
      /* I want to parse strings of numbers with comments at the end */
      /* This return is executed when the next thing along can't be parsed */
      return count; 
    } 
    count++;   /* Count the number of floats */
    start = end;  /* Keep converting from the returned position. */
    while ((UCHAR(*start) != '\0') && isspace(UCHAR(*start))) { start++; }
  } while (UCHAR(*start) != '\0');
  return count; /* Success */
}

int getDoubleString(int numDoubles, const char *dbls, double *tgts)
/* char *dbls - character array of doubles */
/* double *tgts - array to save doubles */
{
  char *end;
  const char *start = dbls;
  double d;
  int count = 0;
  if (countFloatsInString(dbls) != numDoubles) return 1;
  while ((UCHAR(*start) != '\0') && isspace(UCHAR(*start))) { start++; }
  if (UCHAR(*start) == '\0') return -1; /* Don't ask to convert empty strings */
  do {
    d = strtod(start, (char **)&end);
    if (end == start) { 
      /* Can't do any more conversions on this line */
      return (count == numDoubles) ? 0 : 1;
    }
    tgts[count++] = d;   /* place double in array */
    start = end;  /* Keep converting from the returned position. */
    while ((UCHAR(*start) != '\0') && isspace(UCHAR(*start))) { start++; }
    if (count == numDoubles) return 0; /* I don't care if there are more on
            the line as long as I got what I wanted */
  } while (UCHAR(*start) != '\0');
  return 0; /* Success */
}


// -------------------------------------------------------------------
// from transform.c 
int readRegTransformFile(char *fname, double *tran)
/* We assume that enough memory has already been allocated for tran */
{
  FILE *fp = fopen(fname,"r");
  if (fp == (FILE *)NULL) {
    return 1;
  }
  if (fscanf(fp,"%lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf %lf",
    tran, tran+1, tran+2, tran+3, tran+4, tran+5, tran+6,
    tran+7,tran+8,tran+9,tran+10,tran+11) != 12) {
    fclose(fp);
    return 1; /* Didn't read enough elements */
  }
  fclose(fp);
  return 0; /* Success */
}


// *out and inA or inB can be the same pointer
// out = A * B
template <class TinA,class TinB,class Tout>  inline void matmult_3x3Template(TinA *inA, TinB *inB, Tout *out)
{
  Tout tmp[9];
  tmp[0] = Tout(double(inA[0])*double(inB[0]) + double(inA[1])*double(inB[3]) + double(inA[2])*double(inB[6]));
  tmp[1] = Tout(double(inA[0])*double(inB[1]) + double(inA[1])*double(inB[4]) + double(inA[2])*double(inB[7]));
  tmp[2] = Tout(double(inA[0])*double(inB[2]) + double(inA[1])*double(inB[5]) + double(inA[2])*double(inB[8]));
  tmp[3] = Tout(double(inA[3])*double(inB[0]) + double(inA[4])*double(inB[3]) + double(inA[5])*double(inB[6]));
  tmp[4] = Tout(double(inA[3])*double(inB[1]) + double(inA[4])*double(inB[4]) + double(inA[5])*double(inB[7]));
  tmp[5] = Tout(double(inA[3])*double(inB[2]) + double(inA[4])*double(inB[5]) + double(inA[5])*double(inB[8]));
  tmp[6] = Tout(double(inA[6])*double(inB[0]) + double(inA[7])*double(inB[3]) + double(inA[8])*double(inB[6]));
  tmp[7] = Tout(double(inA[6])*double(inB[1]) + double(inA[7])*double(inB[4]) + double(inA[8])*double(inB[7]));
  tmp[8] = Tout(double(inA[6])*double(inB[2]) + double(inA[7])*double(inB[5]) + double(inA[8])*double(inB[8]));

/*
printf("in[6] %g inout[3] %g in[7] %g inout[4] %g in[8] %g inout[5] %g == %g\n",
    in[6], inout[3], in[7], inout[4], in[8], inout[5], tmp[7]); 
*/

  // Do not erase - otherwise concept above does not work
  out[0] = tmp[0];
  out[1] = tmp[1];
  out[2] = tmp[2];
  out[3] = tmp[3];
  out[4] = tmp[4];
  out[5] = tmp[5];
  out[6] = tmp[6];
  out[7] = tmp[7];
  out[8] = tmp[8];

}

// *out and inA or inB can be the same pointer
inline void vtkSimonParameterReaderWriter::matmult_3x3(float *inA, float *inB, float *out) {
  matmult_3x3Template(inA,inB,out);
}

// Needed to calculate not inverted transfromation matrix 
// This is a matrix vector calculation
template <class T>  inline void matvect_multTemplate(T *matrix, T *inVect, T *outVect) { 
  T tmp[3];

  tmp[0] = T(double(matrix[0])*double(inVect[0]) + double(matrix[1])*double(inVect[1]) + double(matrix[2])*double(inVect[2]));
  tmp[1] = T(double(matrix[3])*double(inVect[0]) + double(matrix[4])*double(inVect[1]) + double(matrix[5])*double(inVect[2]));
  tmp[2] = T(double(matrix[6])*double(inVect[0]) + double(matrix[7])*double(inVect[1]) + double(matrix[8])*double(inVect[2]));

  outVect[0] = tmp[0];
  outVect[1] = tmp[1];
  outVect[2] = tmp[2];
}



// Needed for vtkImageEMLocalSegment
template <class T>  inline void matmult_3x4Template(T *inARotation, T *inATranslation, T *inBRotation,  T *inBTranslation, T *outRotation,  T *outTranslation) {
  T tmp[3];

  tmp[0] = T(double(inARotation[0])*double(inBTranslation[0]) + double(inARotation[1])*double(inBTranslation[1]) + double(inARotation[2])*double(inBTranslation[2]) +  double(inATranslation[0]));
  tmp[1] = T(double(inARotation[3])*double(inBTranslation[0]) + double(inARotation[4])*double(inBTranslation[1]) + double(inARotation[5])*double(inBTranslation[2]) +  double(inATranslation[1]));
  tmp[2] = T(double(inARotation[6])*double(inBTranslation[0]) + double(inARotation[7])*double(inBTranslation[1]) + double(inARotation[8])*double(inBTranslation[2]) +  double(inATranslation[2]));

  outTranslation[0] = tmp[0];
  outTranslation[1] = tmp[1];
  outTranslation[2] = tmp[2];

  matmult_3x3Template(inARotation,inBRotation,outRotation);
}

// No inline in functions which are called by other functions and who refere to function within this file 
void vtkSimonParameterReaderWriter::matmult_3x4(float *inARotation, float *inATranslation, float *inBRotation,  float *inBTranslation, float *outRotation,  float *outTranslation) {
  matmult_3x4Template(inARotation, inATranslation, inBRotation, inBTranslation, outRotation, outTranslation);
}

template <class T> void buildTransformMatrix(double *parms, T *transform, int numparms,int paraType)
{
  int i;
  double tmp[12] = {0,0,0,0,0,0,0,0,0,0,0,0};
  double conv = 3.1415926535897932384626433832795028841971693993751058209/180.0;
  double rad = 0.0;
  double radcos = 0.0;
  double radsin = 0.0;

  /* Copy the translate parameters into place */
  if (paraType == 1) {
    transform[9] =  (T) parms[1];
    transform[10] = (T) parms[0];
    transform[11] = (T) parms[2];
  } else {
    transform[9] = (T)parms[0];
    transform[10] = (T) parms[1];
    transform[11] = (T) parms[2];
  }

  /* Initialize transform with the scale parameters */
  for (i = 0; i < 9; i++) {
    transform[i] = (T) 0.0;
  }

  if (numparms <= 6) {
      /* Only translate or rotate to be estimated */
      transform[0] = (T)1.0;
      transform[4] = (T)1.0;
      transform[8] = (T)1.0;
  } else if (numparms == 7) {
    /* One scale parameter */
    transform[0] = (T)parms[6];
    transform[4] = (T)parms[6];
    transform[8] = (T)parms[6];
  } else if (numparms == 8) {
    fprintf(stderr,"Don't know what to do with 8 parameters\n");
    assert(numparms != 8);
    return;
  } else if (numparms == 9) {
    if (paraType == 1) {
    /* Three scale parameters */
    transform[0] = (T)parms[7];
    transform[4] = (T)parms[6];
    transform[8] = (T)parms[8];
    } else {
      /* Three scale parameters */
      transform[0] = (T)parms[6];
      transform[4] = (T)parms[7];
      transform[8] = (T)parms[8];
    } 
  }
  /* If no rotation to estimate */
  /* we can leave the matrix initialized with 1's down the diagonal */
  if (numparms >= 6) {
/*
printf("Transform is: %f %f %f %f %f %f %f %f %f %f %f %f\n",
        transform[0],transform[1], transform[2],  transform[3], transform[4], 
        transform[5],  transform[6], transform[7], transform[8], transform[9], 
        transform[10],  transform[11]); 
*/
    /* Now construct the rotation matrices */
    /* rotate about y axis */

    if (paraType == 1) {
      // This is OK this way because we changed the x and y axis in the alignment matrix from the original one   
      rad = -parms[3] * conv;
      radcos = cos(rad);
      radsin = sin(rad);
      tmp[0] = radcos;
      tmp[1] = 0.;
      tmp[2] = radsin;
      tmp[3] = 0.;
      tmp[4] = 1.;
      tmp[5] = 0.;
      tmp[6] = -1.0 * radsin;
      tmp[7] = 0.;
      tmp[8] = radcos; 

      matmult_3x3Template(tmp,transform,transform);

      /*
    printf("Transform is: %f %f %f %f %f %f %f %f %f %f %f %f\n",
    transform[0],transform[1], transform[2],  transform[3], transform[4], 
    transform[5],  transform[6], transform[7], transform[8], transform[9], 
    transform[10],  transform[11]); 
      */

    /* rotate about x axis */
      rad = -parms[4] * conv;
      radcos = cos(rad);
      radsin = sin(rad);

      tmp[0] = 1.; tmp[1] = 0.; tmp[2] = 0.; tmp[3] = 0.; tmp[6] = 0.0;
      tmp[4] = radcos;
      tmp[5] = -1.0 * radsin;
      tmp[7] = radsin;
      tmp[8] = radcos;
  

      /*
    printf("Tmp is: %f %f %f %f %f %f %f %f %f %f %f %f\n",
    tmp[0],tmp[1], tmp[2],  tmp[3], tmp[4], 
    tmp[5],  tmp[6], tmp[7], tmp[8], tmp[9], 
    tmp[10],  tmp[11]); 
      */
      matmult_3x3Template(tmp,transform,transform);


      /* rotate about z axis */
      rad = -parms[5] * conv;
      radcos = cos(rad);
      radsin = sin(rad);
      tmp[0] = radcos;
      tmp[1] = -1.0 * radsin;
      tmp[2] = 0.;
      tmp[3] = radsin;
      tmp[4] = radcos;
      tmp[5] = 0.;
      tmp[6] = 0.;
      tmp[7] = 0.;
      tmp[8] = 1.;
  
      matmult_3x3Template(tmp,transform,transform);

    } else {

      rad = parms[3] * conv;
      radcos = cos(rad);
      radsin = sin(rad);
      tmp[0] = 1.; tmp[1] = 0.; tmp[2] = 0.; tmp[3] = 0.; tmp[6] = 0.0;
      tmp[4] = radcos;
      tmp[5] = -1.0 * radsin;
      tmp[7] = radsin;
      tmp[8] = radcos;
  
      matmult_3x3Template(tmp,transform,transform);

      /*
    printf("Transform is: %f %f %f %f %f %f %f %f %f %f %f %f\n",
    transform[0],transform[1], transform[2],  transform[3], transform[4], 
    transform[5],  transform[6], transform[7], transform[8], transform[9], 
    transform[10],  transform[11]); 
      */

    /* rotate about y axis */
      rad = parms[4] * conv;
      radcos = cos(rad);
      radsin = sin(rad);
      tmp[0] = radcos;
      tmp[1] = 0.;
      tmp[2] = radsin;
      tmp[3] = 0.;
      tmp[4] = 1.;
      tmp[5] = 0.;
      tmp[6] = -1.0 * radsin;
      tmp[7] = 0.;
      tmp[8] = radcos;
      /*
    printf("Tmp is: %f %f %f %f %f %f %f %f %f %f %f %f\n",
    tmp[0],tmp[1], tmp[2],  tmp[3], tmp[4], 
    tmp[5],  tmp[6], tmp[7], tmp[8], tmp[9], 
    tmp[10],  tmp[11]); 
      */
  
      matmult_3x3Template(tmp,transform,transform);

      /* rotate about z axis */
      rad = parms[5] * conv;
      radcos = cos(rad);
      radsin = sin(rad);
      tmp[0] = radcos;
      tmp[1] = -1.0 * radsin;
      tmp[2] = 0.;
      tmp[3] = radsin;
      tmp[4] = radcos;
      tmp[5] = 0.;
      tmp[6] = 0.;
      tmp[7] = 0.;
      tmp[8] = 1.;
      
      matmult_3x3Template(tmp,transform,transform);
    }
  } else if (numparms > 3) {
    printf("Must specify 3 rotation parameters for 3D optimization\n");
    return;
  }
}


/* Convert from parameter estimates to transform matrix */
/* Convert from the estimated parameters to the transform matrix
   theta[0] = translate x dim
   theta[1] = translate y dim
   theta[2] = translate z dim
   theta[3] = rotate around x axis in degrees
   theta[4] = rotate around y axis in degrees
   theta[5] = rotate around z axis in degrees
   theta[6] = scale x axis
   theta[7] = scale y axis
   theta[8] = scale z axis
*/


/* Take a 3x3 matrix in row major order, and write its inverse
 * into out.  If it is singular return 1 else return 0
 */
template  <class T>  int fast_invert_3x3_matrixTemplate(T* in, T *out)
{
  T det = in[0]*in[4]*in[8] -
                in[0]*in[5]*in[7] +
              in[1]*in[5]*in[6] -
                in[1]*in[3]*in[8] +
              in[2]*in[3]*in[7] -
                in[2]*in[4]*in[6];

  assert(in != out);

  if (fabs(det) <= 0.00000001) {
    fprintf(stderr,"matrix is (close to) singular\n");
    return 1;
  }
  out[0] = (in[4]*in[8] - in[5]*in[7]) / det;
  out[1] =-(in[1]*in[8] - in[2]*in[7]) / det;
  out[2] = (in[1]*in[5] - in[2]*in[4]) / det;
  out[3] =-(in[3]*in[8] - in[5]*in[6]) / det;
  out[4] = (in[0]*in[8] - in[2]*in[6]) / det;
  out[5] =-(in[0]*in[5] - in[2]*in[3]) / det;
  out[6] = (in[3]*in[7] - in[4]*in[6]) / det;
  out[7] =-(in[0]*in[7] - in[1]*in[6]) / det;
  out[8] = (in[0]*in[4] - in[1]*in[3]) / det;

  return 0; /* Success */
}


int vtkSimonParameterReaderWriter::fast_invert_3x3_matrix(float *in, float *out) {return fast_invert_3x3_matrixTemplate(in,out);}


template  <class T> void convertParmsToTransformTemplate(double *theta, T *transform, int numparms, int compute2Dregistration, int paraType)
{
  /* Use this rather than theta since for 2D case we manipulate theta
   * so we can compute a nice 12 parameter transform without
   * complications
   */
  double parms[9];
  int i;

  if (!compute2Dregistration) {
    buildTransformMatrix(theta,transform,numparms,paraType);
    return;
  }

  /* Convert theta into a 12 parameter transform */
  if (compute2Dregistration && (paraType == 2)) {
    std::cerr << "ERROR: convertParmsToTransformTemplate: Did not update parametersetting for 2D resampling!" << endl; 
  }

  /* IF we are only doing a 2D registration - assume we have 6 
     * parameters and convert into a 12 variable matrix */
  if (compute2Dregistration) {
    for (i = 0; i < numparms; i++) {
      parms[i] = theta[i];
    }
    /* In 2D only rotate around z axis makes sense */
    if (numparms == 5) {
      parms[8] = 1;
      parms[7] = theta[4];
      parms[6] = theta[3];
      parms[5] = theta[2];
      parms[4] = 0.0;
      parms[3] = 0.0;
      parms[2] = 0.0;
    } else if (numparms == 3) { /* No scaling is being estimated */
      parms[8] = 1.;
      parms[7] = 1.;
      parms[6] = 1.;
      parms[5] = theta[2];
      parms[4] = 0.0;
      parms[3] = 0.0;
      parms[2] = 0.0;
    } else {
      /* In 2D, only 5 or 3 parameters are allowed */
      printf("In 2D, only 5 or 3 parameters are allowed\n");
            return;
    } 
    buildTransformMatrix(parms,transform,9,paraType);
  }
}

// paraType = 1 old Simon parameterisation 
// paraType = 2 Kilian parameterisation 
template  <class T> 
int TurnParameteresIntoInverseRotationTranslationTemplate( double Xtranslate, double Ytranslate , double Ztranslate,
                                                           double Xrotate,    double Yrotate,     double Zrotate, 
                                   double Xscale,     double Yscale,      double Zscale, 
                               T *invRotation, T *invTranslation, int paraType) {
  T transform[12];
  double theta[9] = {Xtranslate, Ytranslate , Ztranslate, Xrotate, Yrotate, Zrotate, Xscale, Yscale, Zscale};
  convertParmsToTransformTemplate(theta, transform,9,0,paraType);

  T invRotationTry[9];
  if (fast_invert_3x3_matrixTemplate(transform, invRotationTry) != 0) {
    return 1;
  }

  memcpy(invRotation, invRotationTry, sizeof(T)* 9);
  invTranslation[0] = (T)-transform[9];
  invTranslation[1] = (T)-transform[10];
  invTranslation[2] = (T)-transform[11];

  return 0;
}

// Produces inverse of last function 
// Last Function invRot =  (R3*R2*R1*SC) ^ -1  and invTran = - Trans so that  y = invRot x + invTran
// Thus the inverse is RotMat =  R3*R2*R1*SC TransVec = RotMat*Trans as x = RotMat y + TransVec

template  <class T> 
void TurnParameteresIntoRotationTranslationTemplate( double Xtranslate, double Ytranslate , double Ztranslate,
                                                           double Xrotate,    double Yrotate,     double Zrotate, 
                                   double Xscale,     double Yscale,      double Zscale, 
                               T *RotationMatrix, T *DeformationVector, int paraType) {
  T transform[12];
  double theta[9] = {Xtranslate, Ytranslate , Ztranslate, Xrotate, Yrotate, Zrotate, Xscale, Yscale, Zscale};
  convertParmsToTransformTemplate(theta, transform,9,0,paraType);
  memcpy(RotationMatrix, transform, sizeof(T)* 9);
  
  matvect_multTemplate(RotationMatrix,&transform[9],DeformationVector);
  std::cerr << "DeformationVector: " << DeformationVector[0] << " " << DeformationVector[1] << " " << DeformationVector[2] << endl;
}

void vtkSimonParameterReaderWriter::convertParmsToTransform(double *theta, float *transform, int numparms, int compute2Dregistration) {
  convertParmsToTransformTemplate(theta, transform, numparms, compute2Dregistration,0);
}




// Description: 
// paraType = 1 old Simon parameterisation 
// paraType = 2 Kilian parameterisation 
int vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation( double Xtranslate, double Ytranslate , double Ztranslate,
                                                   double Xrotate,    double Yrotate,     double Zrotate, 
                           double Xscale,     double Yscale,      double Zscale, 
                           float *invRotation, float *invTranslation, int paraType) {
  return  TurnParameteresIntoInverseRotationTranslationTemplate(Xtranslate,Ytranslate, Ztranslate, 
                                Xrotate, Yrotate, Zrotate, Xscale, Yscale, Zscale, 
                                invRotation, invTranslation,paraType);
}

int vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(const float *parameters, float *invRotation, float *invTranslation, int paraType, int TwoDFlag, int RigidFlag) {
  if (RigidFlag) {
    if (TwoDFlag) {
      return  TurnParameteresIntoInverseRotationTranslationTemplate((double) parameters[0], (double) parameters[1], 0.0, 0.0, 0.0, (double) parameters[2], 1.0,  
                                    1.0 , 1.0, invRotation, invTranslation,paraType);
    } else {
      return  TurnParameteresIntoInverseRotationTranslationTemplate((double) parameters[0], (double) parameters[1], (double) parameters[2], (double) parameters[3], (double) parameters[4],  
                                    (double) parameters[5], 1.0, 1.0, 1.0, invRotation, invTranslation,paraType);

    }
  } else {
    if (TwoDFlag) {
      return  TurnParameteresIntoInverseRotationTranslationTemplate((double) parameters[0], (double) parameters[1], 0.0, 0.0, 0.0, (double) parameters[2], (double) parameters[3],  
                                    (double) parameters[4], 1.0, invRotation, invTranslation,paraType);
    } else {
      return  TurnParameteresIntoInverseRotationTranslationTemplate((double) parameters[0], (double) parameters[1], (double) parameters[2], (double) parameters[3], (double) parameters[4],  
                                    (double) parameters[5], (double) parameters[6], (double) parameters[7], (double) parameters[8], invRotation, invTranslation,paraType);
    }
  }
}


int vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(const double *parameters, float *invRotation, float *invTranslation, int paraType, 
                                         int TwoDFlag, int RigidFlag) {
  if (RigidFlag) {
    if (TwoDFlag) {
      return  TurnParameteresIntoInverseRotationTranslationTemplate(parameters[0], parameters[1], 0.0, 0.0, 0.0,parameters[2], 1.0,  1.0 , 1.0, invRotation, invTranslation, 
                                    paraType);
    } else {
      return  TurnParameteresIntoInverseRotationTranslationTemplate(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], 1.0, 1.0, 1.0, 
                                    invRotation, invTranslation,paraType);
    }
  } else {
    if (TwoDFlag) {
      return  TurnParameteresIntoInverseRotationTranslationTemplate(parameters[0], parameters[1], 0.0, 0.0, 0.0, parameters[2], parameters[3], parameters[4], 1.0, invRotation,
                                    invTranslation,paraType);
    } else {
      return  TurnParameteresIntoInverseRotationTranslationTemplate(parameters[0], parameters[1], parameters[2], parameters[3], parameters[4], parameters[5], parameters[6], 
                                    parameters[7], parameters[8], invRotation, invTranslation,paraType);
    }
  }
}


int vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(double *Translation, double *Rotation, double *Scale, float *invRotation, float *invTranslation, int paraType, int TwoDFlag, int RigidFlag) {
 
  if (RigidFlag) {
    if (TwoDFlag) {
      return  TurnParameteresIntoInverseRotationTranslationTemplate(Translation[0], Translation[1], 0.0, 0.0, 0.0, Rotation[2], 1.0, 1.0, 1.0, invRotation, invTranslation,paraType);
    } else {
      return  TurnParameteresIntoInverseRotationTranslationTemplate( Translation[0], Translation[1], Translation[2], Rotation[0], Rotation[1], Rotation[2], 1.0, 1.0, 1.0, invRotation, invTranslation,paraType);
    }
    
  } else {
    if (TwoDFlag) {
      return  TurnParameteresIntoInverseRotationTranslationTemplate(Translation[0], Translation[1], 0.0, 0.0, 0.0, Rotation[2], Scale[0], Scale[1], 1.0, invRotation, invTranslation,paraType);
    } else {
      return  TurnParameteresIntoInverseRotationTranslationTemplate( Translation[0], Translation[1], Translation[2], Rotation[0], Rotation[1], Rotation[2], 
                                     Scale[0], Scale[1], Scale[2], invRotation, invTranslation,paraType);
    }
  }
}
// This produces the inverse of the last function 
void vtkSimonParameterReaderWriter::TurnParameteresIntoRotationTranslation(double Xtranslate, double Ytranslate , double Ztranslate,
                           double Xrotate, double Yrotate, double Zrotate, double Xscale, double Yscale, double Zscale, float *RotationMatrix, 
                                       float *DisplacementVector, int paraType) {

  TurnParameteresIntoRotationTranslationTemplate(Xtranslate,Ytranslate, Ztranslate, 
                         Xrotate, Yrotate, Zrotate, Xscale, Yscale, Zscale, 
                         RotationMatrix, DisplacementVector,paraType);
}

int readParametersFromFile(char *fname, double *parameters)
{
  char buffer[2048];
  int numparms = 0;

  FILE *fp = fopen(fname,"r");
  if (fp == NULL) {
    return -1; /* Failure */
  }

  memset(buffer,0,sizeof(buffer));
  fgets(buffer,sizeof(buffer),fp);
  if (fclose(fp) != 0) {
    fprintf(stderr,"Failed closing file %s\n",fname);
    return -1; /* Failure */
  }

  numparms = countFloatsInString(buffer);
  if (getDoubleString(numparms, buffer, parameters) != 0) {
    fprintf(stderr,"Failed to convert string to parameters\n");
    return -1; /* Failure */
  }

  return numparms;
}

int writeParametersToFile(char *fname, double *parameters, int numparms)
{
  std::cerr << "Write results to " << fname << endl; 
  int i = 0;
  FILE *fp = fopen(fname,"w");
  if (fp == NULL) {
    return -1; /* Failure */
  }

  for (i = 0; i < numparms; i++) {
    fprintf(fp,"%g ",parameters[i]);
  }
  fprintf(fp,"\n");
  
  if (fclose(fp) != 0) {
    fprintf(stderr,"Failed closing file %s\n",fname);
    return -1; /* Failure */
  }

  return 0; /* success */
}
// End of Simon's File
// -------------------------------------------------------------------

int writeParametersToGuimondFile(char *fname, double transform[12])
{
  std::cerr << "Write results to " << fname << endl; 
  FILE *fp = fopen(fname,"w");
  if (fp == NULL) {
    return -1; /* Failure */
  }
  
  // Transform matrix is defined as 
  // Rotation Matrix   Translation Matrix 
  // t[0] t[1] t[2]      t[9]    
  // t[3] t[4] t[5]      t[10]
  // t[6] t[7] t[8]      t[11]
  fprintf(fp,"VTK Transform File\n");
  fprintf(fp,"Homogeneous Transform\n");  
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) fprintf(fp,"%g ",transform[i*3 + j]);
    fprintf(fp,"%g \n",transform[9 + i]);
  }
  fprintf(fp,"0.0000 0.0000 0.0000 1.0000 \n\n");
  
  if (fclose(fp) != 0) {
    fprintf(stderr,"Failed closing file %s\n",fname);
    return -1; /* Failure */
  }

  return 0; /* success */
}
// -------------------------------------------------------------------
// Transferes file from Simon To Alex 
// the nine parameters are defined as following
// 
// Parameter 0-2 : translation 
// Parameter 3-5 : Rotation 
// Parameter 6-8 : Scale Parameter  
// where p[i] is abreviation of parameter i
// 
// Transform matrix is defined as 
// Rotation Matrix   Translation Matrix 
// t[0] t[1] t[2]      t[9]    
// t[3] t[4] t[5]      t[10]
// t[6] t[7] t[8]      t[11]
// 
// where R=  t[0],..., t[8] and T = t[9] ,.. , t[11]
// Before applying rotation 
// p[6]  0    0        p[0]
// 0    p[7]  0        p[1]
// 0     0   p[8]      p[2]
// 
// Afterwards we apply Yaw Pitch Roll to the new R where 
// we first rotate around the x axis with paramter p[3]
// then rotate around the y axis with paramter p[4]
// and finally rotate around the z axis with paramter p[5]


int ReadParameterFile(char *WarfieldFileName, double* parameter) {
  int NumParams = readParametersFromFile(WarfieldFileName, parameter);
  if (NumParams < 0) {
    cerr << "Could not read file " << WarfieldFileName << endl;
    return -1;
  }
  if (NumParams != 9) {
    cerr << "Warfield file needs 9 parameters but has " << NumParams <<  endl;
    return -1;
  }
  return NumParams;
}

int vtkSimonParameterReaderWriter::transfereWarfieldToGuimondParameterFile(char *WarfieldFileName, char *GuimondFileName) {
  double parameters[10];
  int NumParams =  ReadParameterFile(WarfieldFileName,parameters);
  if (NumParams < 0) return 1;
  double transform[12];

  // Write result back 
  buildTransformMatrix(parameters,transform,NumParams,1);
  if (writeParametersToGuimondFile(GuimondFileName,transform)) {
    cerr << "Could not create file " << GuimondFileName <<  endl;
    return 1;
  }
  return 0;
} 

int vtkSimonParameterReaderWriter::ReadFileParameter(char *WarfieldFileName) {
  if (ReadParameterFile(WarfieldFileName,this->FileParameter) < 0) return 1;
  return 0;
}
