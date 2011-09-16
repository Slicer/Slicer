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
// Propogates the labels to areas with values 0 ! 
// Same as Voronoi diagram - also see http://en.wikipedia.org/wiki/Voronoi_diagram

/*=========================================================================

  Program:   Visualization Toolkit
  Module:    $RCSfile: vtkImageLabelPropagation.h,v $
  Language:  C++
  Date:      $Date: 2007/08/24 11:43:33 $
  Version:   $Revision: 1.2 $

  Copyright (c) 1993-2002 Ken Martin, Will Schroeder, Bill Lorensen 
  All rights reserved.
  See Copyright.txt or http://www.kitware.com/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkImageLabelPropagation - computes 3D Euclidean DT 
// .SECTION Description
// vtkImageLabelPropagation implements the Euclidean DT using
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
 

#ifndef __vtkImageLabelPropagation_h
#define __vtkImageLabelPropagation_h

#include "vtkEMSegment.h"
#include "vtkImageDecomposeFilter.h"

#define VTK_EMKILIAN_EDT_SAITO_CACHED 0
#define VTK_EMKILIAN_EDT_SAITO 1 

#define VTK_EMKILIAN_EDT_EUCLIDEAN  0
#define VTK_EMKILIAN_EDT_SQUARE_ROOT 1
                                  
class  VTK_EMSEGMENT_EXPORT vtkImageLabelPropagation : public vtkImageDecomposeFilter
{
public:
  static vtkImageLabelPropagation *New();
  vtkTypeRevisionMacro(vtkImageLabelPropagation,vtkImageDecomposeFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  vtkGetMacro(Initialize, int);
  vtkGetMacro(MaximumDistance, float);
  vtkGetMacro(ConsiderAnisotropy, int);
  vtkImageData* GetDistanceMap() {return this->GetOutput();}

  vtkImageData* GetPropagatedMap() { return this->PropagatedMap; } 

  virtual int IterativeRequestData(vtkInformation*,
                                   vtkInformationVector**,
                                   vtkInformationVector*);
protected:
  vtkImageLabelPropagation();
  ~vtkImageLabelPropagation(); 

  float MaximumDistance;
  int Initialize;
  int ConsiderAnisotropy;
  vtkImageData *PropagatedMap;

  // Replaces "EnlargeOutputUpdateExtent"
  virtual void AllocateOutputScalars(vtkImageData *outData);
  
// void ExecuteInformation(vtkImageData *input, vtkImageData *output);
  // Kilian: old vtk style 
  // void ExecuteInformation() {this->vtkImageIterateFilter::ExecuteInformation();}
  //void ComputeInputUpdateExtent(int inExt[6], int outExt[6]);

  virtual int IterativeRequestInformation(vtkInformation* in,
                                          vtkInformation* out);
  virtual int IterativeRequestUpdateExtent(vtkInformation* in,
                                           vtkInformation* out);

private:

  vtkImageLabelPropagation(const vtkImageLabelPropagation&);  // Not implemented.
  void operator=(const vtkImageLabelPropagation&);  // Not implemented.
};

#endif










