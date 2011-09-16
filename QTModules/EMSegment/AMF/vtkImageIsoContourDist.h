/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageIsoContourDist.h,v $
  Date:      $Date: 2006/05/26 19:54:49 $
  Version:   $Revision: 1.3 $

=========================================================================auto=*/
/*  ==================================================
    Module : vtkLevelSets
    Authors: Karl Krissian
    Email  : karl@bwh.harvard.edu

    This module implements a Active Contour evolution
    for segmentation of 2D and 3D images.
    It implements a 'codimension 2' levelsets as an
    option for the smoothing term.
    It comes with a Tcl/Tk interface for the '3D Slicer'.
    ==================================================
    Copyright (C) 2003  LMI, Laboratory of Mathematics in Imaging, 
    Brigham and Women's Hospital, Boston MA USA

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    ================================================== 
   The full GNU Lesser General Public License file is in vtkLevelSets/LesserGPL_license.txt
*/

// .NAME vtkImageIsoContourDist - short description

// .SECTION Description
// vtkImageIsoContourDist
//
// Compute the distance to an isocontour
//

#ifndef _vtkImageIsoContourDist_h
#define _vtkImageIsoContourDist_h

#include "vtkEMSegment.h"
#include "vtkImageToImageFilter.h"
#include "vtkFloatArray.h"

class VTK_EMSEGMENT_EXPORT vtkImageIsoContourDist : public vtkImageToImageFilter
{
public:
  static vtkImageIsoContourDist *New();
  vtkTypeMacro(vtkImageIsoContourDist,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description

  // Intensity of the IsoContour
  vtkSetMacro(threshold,float);
  vtkGetMacro(threshold,float);

  // value for voxel not neighbors to the isocontour
  vtkSetMacro(farvalue,float);
  vtkGetMacro(farvalue,float);

  // Modify the input image without creating the output
  //  vtkSetMacro(     modify_input,int);
  //  vtkGetMacro(     modify_input,int);
  //  vtkBooleanMacro( modify_input,int);

  void InitParam();

  void UseOutputArray(float* array) { output_array = array; }

  void SetNarrowBand( int* band, int size);
  void SetMinMaxX( int** minx, int** maxx);

  // for multi-thread purpose
  int  SplitBand(int& first, int& last, int num, int total);
  void IsoSurfDist3D_band(int first_band,int last_band);

protected:
  vtkImageIsoContourDist();
  ~vtkImageIsoContourDist();

  void ExecuteData(vtkDataObject *outData);

  void IsoSurfDistInit();

  void IsoSurfDist2D();
  void IsoSurfDist3D();

  void IsoSurfDist3D_band();

//BTX
  float threshold;

  // value +/- farvalue for voxels which are not
  // neighbors to the isocontour.
  float farvalue;

  int tx,ty,tz,txy;
  unsigned long imsize;

  // Narrow Band Information
  int*           narrowband;
  int            bandsize;  

  // Limits for each line in X direction
  int** min_x;
  int** max_x;

  // We copy input data in float format
  vtkImageData *inputImage;
  int inputImage_allocated;

  //  int modify_input;

  vtkImageData *outputImage;

  // allows to use an array for the output image without allocation
  float* output_array;

  vtkFloatArray* float_array;
  unsigned char  float_array_allocated;

//ETX
};


#endif // _vtkImageIsoContourDist_h
