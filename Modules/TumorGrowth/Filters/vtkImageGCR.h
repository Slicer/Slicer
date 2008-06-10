/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageGCR.h,v $
  Date:      $Date: 2006/01/06 17:57:10 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
// .NAME vtkImageGCR - 
// 
// .SECTION Description
// 
// .SECTION See Also

#ifndef __vtkImageGCR_h
#define __vtkImageGCR_h

#include "vtkTumorGrowth.h"

#include <vtkTransform.h>
#include <vtkGeneralTransform.h>
#include <vtkImageData.h>

#define VTK_IMAGE_GCR_L1 1
#define VTK_IMAGE_GCR_L2 2
#define VTK_IMAGE_GCR_CO 3
#define VTK_IMAGE_GCR_MI 4

class VTK_TUMORGROWTH_EXPORT vtkImageGCR : public vtkTransform {
public:
  static vtkImageGCR* New();
  vtkTypeMacro(vtkImageGCR,vtkTransform);
  void PrintSelf(ostream& os,vtkIndent indent);

  vtkBooleanMacro(Verbose, int);
  vtkSetMacro(Verbose,int);
  vtkGetMacro(Verbose,int);

  vtkSetObjectMacro(Target,vtkImageData);
  vtkGetObjectMacro(Target,vtkImageData);
  vtkSetObjectMacro(Source,vtkImageData);
  vtkGetObjectMacro(Source,vtkImageData);
  vtkSetObjectMacro(Mask,vtkImageData);
  vtkGetObjectMacro(Mask,vtkImageData);
  vtkGetObjectMacro(GeneralTransform,vtkGeneralTransform);

  vtkSetMacro(TransformDomain, int);
  vtkGetMacro(TransformDomain, int);
  
  void SetInterpolation(int i);
  vtkGetMacro(Interpolation, int);

  vtkSetMacro(Criterion, int);
  vtkGetMacro(Criterion, int);

  vtkBooleanMacro(TwoD, int);
  vtkSetMacro(TwoD, int);
  vtkGetMacro(TwoD, int);

  void Inverse();
  
protected:
  vtkImageGCR();
  ~vtkImageGCR();
  vtkImageGCR(const vtkImageGCR&);
  void operator=(const vtkImageGCR&);
  
  void InternalUpdate();

  float Compute(); 
  float ComputeL1(float jh[256][256]);
  float ComputeL2(float jh[256][256]);
  float ComputeCO(float jh[256][256]);
  float ComputeMI(float jh[256][256]);

  bool CheckImages();
  void NormalizeImages();
  
  float MinimizeWithTranslationTransform(float* p);
  float MinimizeWithRigidTransform(float* p);
  float MinimizeWithSimilarityTransform(float* p);
  float MinimizeWithAffineTransform(float* p);
  
  bool TrilinearInterpolation(float *point,unsigned char *inPtr,
                  int inExt[6], int inInc[3],unsigned char& q);
  bool TrilinearWeights(float *point,unsigned char *inPtr,int inExt[6],
            int inInc[3], unsigned char p[2][2][2],
            float w[2][2][2]);
      
  vtkImageData* Target;
  vtkImageData* Source;
  vtkImageData* Mask;
  vtkImageData* WorkTarget;
  vtkImageData* WorkSource;
  vtkImageData* WorkMask;
  vtkTransform* WorkTransform;
  vtkGeneralTransform* GeneralTransform;
  int TransformDomain;
  int Interpolation;
  int Criterion;
  int TwoD;
  int Verbose;

private:
  void ComputeWithTrilinearInterpolation(float* point,unsigned char* tptr,
                     unsigned char* sptr,int* ext,
                     int* inc,float jh[256][256]);
  void ComputeWithPVInterpolation(float* point,unsigned char* tptr,
                  unsigned char* sptr,int* ext,int* inc,
                  float jh[256][256]);
  
  //BTX
  void (vtkImageGCR::* InterpolationFunction)
    (float* point,unsigned char* tptr,
     unsigned char* sptr,int* ext,int* inc,
     float jh[256][256]);
  //ETX

  // from NR
  //BTX
  float brent(float ax,float bx,float cx,
          float (vtkImageGCR::*f)(float),
          float tol,float* xmin);
  float f1dim(float x);
  void mnbrak(float* ax,float* bx,float* cx,float* fa,float* fb,float* fc,
          float (vtkImageGCR::*func)(float));
  void linmin(float* p,float* xi,int n,float* fret,
          float (vtkImageGCR::*func)(float*));
  void powell(float* p,float** xi,int n,
          float ftol,int* iter,
          float* fret,float (vtkImageGCR::*func)(float*));

  float* vector(int nl,int nh);
  float** matrix(int nrl,int nrh,int ncl,int nch);
  void free_vector(float* v,int nl,int nh);
  void free_matrix(float** m,int nrl,int nrh,int ncl,int nch);
      
  int ncom;
  float* pcom;
  float* xicom;
  float (vtkImageGCR::*nrfunc)(float*);
  //ETX      
};
#endif
