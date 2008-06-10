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
// I had to alter vtkImageEuclideanDistance.h to make it work for my purposes 

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageKilianDistanceTransform.h,v $
  Language:  C++
  Date:      $Date: 2006/05/11 22:08:21 $
  Version:   $Revision: 1.3 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageKilianDistanceTransform - computes 3D Euclidean DT 
// .SECTION Description
// vtkImageKilianDistanceTransform implements the Euclidean DT using
// Saito's algorithm. The distance map produced contains the square of the
// Euclidean distance values. 
//
// The algorithm has a o(n^(D+1)) complexity over nxnx...xn images in D 
// dimensions. It is very efficient on relatively small images. Cuisenaire's
// algorithms should be used instead if n >> 500. These are not implemented
// yet.
//
// For the special case of images where the slice-size is a multiple of 
// 2^N with a large N (typically for 256x256 slices), Saito's algorithm 
// encounters a lot of cache conflicts during the 3rd iteration which can 
// slow it very significantly. In that case, one should use 
// ::SetAlgorithmToSaitoCached() instead for better performance. 
//
// References:
//
// T. Saito and J.I. Toriwaki. New algorithms for Euclidean distance 
// transformations of an n-dimensional digitised picture with applications.
// Pattern Recognition, 27(11). pp. 1551--1565, 1994. 
// 
// O. Cuisenaire. Distance Transformation: fast algorithms and applications
// to medical image processing. PhD Thesis, Universite catholique de Louvain,
// October 1999. http://ltswww.epfl.ch/~cuisenai/papers/oc_thesis.pdf 
 

#ifndef __vtkImageKilianDistanceTransform_h
#define __vtkImageKilianDistanceTransform_h

#include "vtkImageDecomposeFilter.h"
#include "vtkTumorGrowth.h"

#define VTK_EMKILIAN_EDT_SAITO_CACHED 0
#define VTK_EMKILIAN_EDT_SAITO 1 

#define VTK_EMKILIAN_EDT_EUCLIDEAN  0
#define VTK_EMKILIAN_EDT_SQUARE_ROOT 1
                                  
class  VTK_TUMORGROWTH_EXPORT vtkImageKilianDistanceTransform : public vtkImageDecomposeFilter
{
public:
  static vtkImageKilianDistanceTransform *New();
  vtkTypeRevisionMacro(vtkImageKilianDistanceTransform,vtkImageDecomposeFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Used internally for streaming and threads.  
  // Splits output update extent into num pieces.
  // This method needs to be called num times.  Results must not overlap for
  // consistent starting extent.  Subclass can override this method.
  // This method returns the number of peices resulting from a
  // successful split.  This can be from 1 to "total".  
  // If 1 is returned, the extent cannot be split.
  int SplitExtent(int splitExt[6], int startExt[6], 
                  int num, int total);

  // Description:
  // Used to set all non-zero voxels to MaximumDistance before starting
  // the distance transformation. Setting Initialize off keeps the current 
  // value in the input image as starting point. This allows to superimpose 
  // several distance maps. 
  // Kilian: For my stuff should always be set to 1 otherwise use the vtk filter - so I uncommented it 
  //vtkSetMacro(Initialize, int);
  vtkGetMacro(Initialize, int);
  //vtkBooleanMacro(Initialize, int);
  
  // Description:
  // Used to define whether Spacing should be used in the computation of the
  // distances 
  vtkSetMacro(ConsiderAnisotropy, int);
  vtkGetMacro(ConsiderAnisotropy, int);
  vtkBooleanMacro(ConsiderAnisotropy, int);
  
  // Description:
  // Any distance bigger than this->MaximumDistance will not ne computed but
  // set to this->MaximumDistance instead.
  // This value is always in Euclidean space, e.g. if you set this->DistanceFormat = VTK_EMKILIAN_EDT_SQUARE_ROOT
  // and you want to have a maximum distance of -10 to 10 then set   MaximumDistance = 100
  vtkSetMacro(MaximumDistance, float);
  vtkGetMacro(MaximumDistance, float);

  // Description:
  // Selects a Euclidean DT algorithm. 
  // 1. Saito
  // 2. Saito-cached 
  // More algorithms will be added later on. 
  vtkSetMacro(Algorithm, int);
  vtkGetMacro(Algorithm, int);
  void SetAlgorithmToSaito () 
    { this->SetAlgorithm(VTK_EMKILIAN_EDT_SAITO); } 
  void SetAlgorithmToSaitoCached () 
    { this->SetAlgorithm(VTK_EMKILIAN_EDT_SAITO_CACHED); }   


  // Description: 
  // Defines if the value of the boundary inside the object should have the value zero or positive
  // By default it is set to positive so that the outside boundary is set to zero 
  // This is necessary to be able to produce signed distance maps 
    
  vtkSetMacro(InsideBoundaryZero, int);
  vtkGetMacro(InsideBoundaryZero, int);
  void SetZeroBoundaryInside() {this->InsideBoundaryZero = 1;}
  void SetZeroBoundaryOutside() {this->InsideBoundaryZero = 0;}
 
  //  Description:
  //  What value has the object for which the distance map should be produced
  vtkSetMacro(ObjectValue,int);
  vtkGetMacro(ObjectValue,int);

  // Descitption:
  // Should the distance be calculated in Euclidean or SquareRooted value of the Euclidean distance
  // Simon used SquareRooted - so my MICCAI 04 stuff does too. 
  vtkSetMacro(DistanceFormat, int);
  vtkGetMacro(DistanceFormat, int);
  void DistanceTransform()  {this->DistanceFormat = VTK_EMKILIAN_EDT_EUCLIDEAN; }
  void SquareRootDistance() {this->DistanceFormat = VTK_EMKILIAN_EDT_SQUARE_ROOT; }

  // Descitption:
  // Do we want to have a signed distance map or just positive values inside and zero outside 
  vtkSetMacro(SignedDistanceMap, int);
  vtkGetMacro(SignedDistanceMap, int);
  vtkBooleanMacro(SignedDistanceMap, int); 

  void IterativeExecuteData(vtkImageData *in, vtkImageData *out);
  
protected:
  vtkImageKilianDistanceTransform();
  ~vtkImageKilianDistanceTransform() {}

  float MaximumDistance;
  int Initialize;
  int ConsiderAnisotropy;
  int Algorithm;

  int InsideBoundaryZero;
  int ObjectValue;

  int SignedDistanceMap;
int DistanceFormat; 
  // Replaces "EnlargeOutputUpdateExtent"
  virtual void AllocateOutputScalars(vtkImageData *outData);
  
  void ExecuteInformation(vtkImageData *input, vtkImageData *output);
// void ExecuteInformation() {vtkImageDecomposeFilter::ExecuteInformation();}
  void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);
private:
  vtkImageKilianDistanceTransform(const vtkImageKilianDistanceTransform&);  // Not implemented.
  void operator=(const vtkImageKilianDistanceTransform&);  // Not implemented.
// Not Implemented
  void ThreadedExecute( vtkImageData *vtkNotUsed(inData), vtkImageData *vtkNotUsed(outData), int extent[6], int vtkNotUsed(threadId)) {
    printf("vtkImageKilianDistanceTransform::ThreadedExecute: if this comes up then the filter does not work - please fix !\n");  
  } 
};

#endif










