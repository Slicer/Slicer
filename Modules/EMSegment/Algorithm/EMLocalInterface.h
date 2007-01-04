/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology 

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/
// .NAME EMLocalInterface

#ifndef _EMLOCALINTERFACE_H_INCLUDED
#define _EMLOCALINTERFACE_H_INCLUDED 1

#include "vtkEMSegment.h"
#include "vtkMultiThreader.h"

// Defines the maximum number of threads used throughout this program.
// This is very helpful when validating results from machines with different cpu number
// EMLOCALSEGMENTER_MAX_MULTI_THREAD = -1 => no restriction
 
#define EMLOCALSEGMENTER_MAX_MULTI_THREAD -1

//--------------------------------------------------------------------
// Registration Parameters 
//--------------------------------------------------------------------
// Parameters needed to define cost function 
#define EMSEGMENT_REGISTRATION_DISABLED     0
#define EMSEGMENT_REGISTRATION_APPLY        1 
#define EMSEGMENT_REGISTRATION_GLOBAL_ONLY  2
#define EMSEGMENT_REGISTRATION_CLASS_ONLY   3
#define EMSEGMENT_REGISTRATION_SIMULTANEOUS 4
#define EMSEGMENT_REGISTRATION_SEQUENTIAL   5
 
#define EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR 1
#define EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR 2 

#define EMSEGMENT_REGISTRATION_SIMPLEX 1 
#define EMSEGMENT_REGISTRATION_POWELL 2 

#define EMSEGMENT_STOP_FIXED    0 
#define EMSEGMENT_STOP_LABELMAP 1 
#define EMSEGMENT_STOP_WEIGHTS  2 

#define EMSEGMENT_PCASHAPE_DEPENDENT 0
#define EMSEGMENT_PCASHAPE_INDEPENDENT 1
#define EMSEGMENT_PCASHAPE_APPLY 2


//--------------------------------------------------------------------
// Hierachy / SuperClass specific parameters 
//--------------------------------------------------------------------
//BTX  
class VTK_EMSEGMENT_EXPORT EMLocal_Hierarchical_Class_Parameters {
 public: 
  int           NumClasses;
  int           NumTotalTypeCLASS;
  int*          NumChildClasses;
  // Do not put it here even though it makees sense bc in MF changes for each job
  // void          ** ProbDataPtr;
  int           *ProbDataIncY; 
  int           *ProbDataIncZ; 
  float         *ProbDataWeight;
  float         *ProbDataMinusWeight;
  int           ProbDataType;
  double        **LogMu;
  double        ***InvLogCov;
  double        *InvSqrtDetLogCov;
  double        *TissueProbability;
  int           *VirtualNumInputImages;
  double        ***MrfParams;
  EMLocal_Hierarchical_Class_Parameters(); 
  ~EMLocal_Hierarchical_Class_Parameters() {} 
  void Copy(EMLocal_Hierarchical_Class_Parameters init) ; 
}; 

//  Utility function used by shape.cxx and EMPrivatRegistration - should be put in .h file
inline int EMLocalInterface_InterpolationNearestNeighbourVoxelIndex(float col, float row, float slice, int dataIncY,int dataIncZ, int* Image_Length) {
  /* AVOID floor() overhead */
  /* Is floor call necessary ?
     i = (int)floor(row + .5);
     j = (int)floor(col + .5);
     k = (int)floor(slice + .5);
  */
  int i = (int)((row < 0) ? (row - 0.5) : (row + 0.5));
  int j = (int)((col < 0) ? (col - 0.5) : (col + 0.5));
  int k = (int)((slice < 0) ? (slice - 0.5) : (slice + 0.5));

  int MaxIndexCol   = Image_Length[0] - 1;
  int MaxIndexRow   = Image_Length[1] - 1;
  int MaxIndexSlice = Image_Length[2] - 1;

  if (i < 0) {i = 0;}
  else if (i > MaxIndexRow) i = MaxIndexRow;  
  if (j < 0) j = 0;
  else if (j > MaxIndexCol) j =  MaxIndexCol;
  if (k < 0) k = 0;
  else if (k > MaxIndexSlice) k = MaxIndexSlice;

  int colsize   = Image_Length[0] +dataIncY; 
  int slicesize = Image_Length[1]*colsize + dataIncZ;

  return i*colsize + j + k*slicesize;
}    

template <class T>
inline double  EMLocalInterface_Interpolation(
    float col, float row,   float slice,
    int ncol,  int nrow,  int nslice,
    T* data, int dataIncY, int dataIncZ, int InterpolationType, int* Image_Length ) {


  int j = (int)floor(col);
  int i = (int)floor(row);
  int k = (int)floor(slice);

  /* Decide if we can do trilinear interpolation */
  // ------------------------------------------------
  // from NearestNeighbourInterpolation 
  // ------------------------------------------------

  if ((InterpolationType == EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR) || (i < 0) || (j < 0) || (k < 0) || (i >= (nrow - 1)) || (j >= (ncol - 1)) || ((k >= (nslice - 1) ) && (nslice != 1))) {
    int VoxelJump = EMLocalInterface_InterpolationNearestNeighbourVoxelIndex(col, row, slice, dataIncY,dataIncZ, Image_Length);
    return double(data[VoxelJump]);
  }

  // ------------------------------------------------
  // From LinearInterpolation
  // ------------------------------------------------
  int index;

  int colsize   = ncol+dataIncY; 
  int slicesize = nrow*colsize + dataIncZ;

  double t;
  double tt;
  double u;
  double uu;
  double v;
  double vv;

  t = double(row - i);
  tt = double(1.0 - t);
  u = double(col - j);
  uu = double(1.0 - u);
  v = double(slice - k);
  vv = double(1.0 - v);

  if (k >= (nslice - 1) && (nslice == 1)) {
    /* we have 2D data - so do a restricted interpolation */

    /* So off slice will make no difference */
    v = 0.0;
    vv = 1.0; 
    /* So we don't dereference an illegal location */
    slicesize = 0;
  }

/*
fprintf(stdout,"row %g col %g slice %g ", row, col, slice);
fprintf(stdout,"(i,j,k) (%d, %d, %d)\n", i, j, k);
fprintf(stdout, "t %g tt %g u %g uu %g v %g vv %g\n", t, tt, u, uu, v, vv);
*/

  index = i*colsize + j + k*slicesize;
/*
fprintf(stdout, "centre index: %d lastindex: %d\n", index, lastindex);
fprintf(stdout, "data indexed: %d %d %d %d %d %d %d\n", index + ncol,
index + 1, index + 1 + ncol, index + slicesize, index + ncol + slicesize, 
index + 1 + slicesize, index + 1 + ncol + slicesize);
*/


 return double(tt*uu*vv*double(data[index])     + t*uu*vv*double(data[index + colsize]) + 
     tt*u*vv*double(data[index + 1])            + t*u*vv*double(data[index + 1 + colsize]) + 
     tt*uu*v*double(data[index + slicesize])    + t*uu*v*double(data[index + colsize + slicesize]) +
     tt*u*v*double(data[index + 1 + slicesize]) + t*u*v*double(data[index + 1 + colsize + slicesize]));

}


template <class T>
inline void  EMLocalInterface_Interpolation(float col, float row, float slice, int ncol,  int nrow, int nslice, T* data, int dataIncY, int dataIncZ, 
                                              int InterpolationType, int* Image_Length, double &result) {
     result = EMLocalInterface_Interpolation(col, row, slice, ncol, nrow, nslice, data, dataIncY, dataIncZ, InterpolationType, Image_Length);
}

// This interpolation descirbes the following:
// ccol, crow, cslice are in image space 
// The function calculates the coordinates in target space (in our case the atlas space)
inline void EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(
    float *invRot, float *invTran,
    int ccol, int crow,   int cslice,
    float &ocol,  float &orow,  float &oslice,
    float Image_MidX, float Image_MidY, float Image_MidZ) {

  float mmcoords[3];

  // Kilian: Scale has to be applied this way so that the center stays the same
  mmcoords[0] = float(ccol)   - Image_MidX;
  mmcoords[1] = float(crow)   - Image_MidY;
  mmcoords[2] = float(cslice) - Image_MidZ;
  ocol    = invRot[0]*mmcoords[0] +
                     invRot[1]*mmcoords[1]+                     
                     invRot[2]*mmcoords[2] + invTran[0] + Image_MidX;
  orow    = invRot[3]*mmcoords[0] +
                     invRot[4]*mmcoords[1] +
                     invRot[5]*mmcoords[2] + invTran[1] + Image_MidY;
  oslice  = invRot[6]*mmcoords[0] +
                     invRot[7]*mmcoords[1] +
                     invRot[8]*mmcoords[2] + invTran[2] + Image_MidZ;

}


// ==============================================
// Shape Based Functions 
// This is currnelty without scaling of the PCA parameters 
// Voxel jump is needed so we can do registration easily

inline float  EMLocalInterface_CalcDistanceMap(const double *ShapeParameter, float **PCAEigenVectorsPtr, float *MeanShape, int DimEigenVector, int VoxelJump) {
  float term = *(MeanShape + VoxelJump); 
  for (int l = 0 ; l < DimEigenVector; l++) term += (*(PCAEigenVectorsPtr[l] + VoxelJump)) * ShapeParameter[l];
  return  term;
}

inline float  EMLocalInterface_CalcDistanceMap(const float *ShapeParameter, float **PCAEigenVectorsPtr, float *MeanShape, int DimEigenVector, int VoxelJump) {
  float term = *(MeanShape + VoxelJump); 
  for (int l = 0 ; l < DimEigenVector; l++) term += (*(PCAEigenVectorsPtr[l] + VoxelJump)) * ShapeParameter[l];
  return  term;
}

inline int EMLocalInterface_DefineMultiThreadJump(int *Start, int BoundaryMaxX, int BoundaryMaxY, int DataIncY,  int DataIncZ) {
  int LengthOfXDim = BoundaryMaxX + DataIncY;
  int LengthOfYDim = LengthOfXDim*(BoundaryMaxY) + DataIncZ;  
  return Start[0] + Start[1] * LengthOfXDim + LengthOfYDim *Start[2];
}

inline int EMLocalInterface_GetDefaultNumberOfThreads(int DisableFlag) {
  if (DisableFlag) return 1;
  int result = vtkMultiThreader::GetGlobalDefaultNumberOfThreads();
  if ((EMLOCALSEGMENTER_MAX_MULTI_THREAD > 0 ) && (result >  EMLOCALSEGMENTER_MAX_MULTI_THREAD)) return EMLOCALSEGMENTER_MAX_MULTI_THREAD;
  return result;
}
#endif
