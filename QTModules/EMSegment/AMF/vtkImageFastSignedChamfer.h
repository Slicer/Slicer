/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageFastSignedChamfer.h,v $
  Date:      $Date: 2006/05/26 19:54:49 $
  Version:   $Revision: 1.4 $

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

#ifndef _vtkImageFastSignedChamfer_h
#define _vtkImageFastSignedChamfer_h

#include "vtkEMSegment.h"
#include "vtkImageToImageFilter.h"
#include "vtkFloatArray.h"

class VTK_EMSEGMENT_EXPORT vtkImageFastSignedChamfer : public vtkImageToImageFilter
{
public:
  static vtkImageFastSignedChamfer *New();
  vtkTypeMacro(vtkImageFastSignedChamfer,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description

  // Maximal distance
  vtkSetMacro(maxdist,float);
  vtkGetMacro(maxdist,float);

  // Coefficient a
  vtkSetMacro(coeff_a,float);
  vtkGetMacro(coeff_a,float);

  // Coefficient b
  vtkSetMacro(coeff_b,float);
  vtkGetMacro(coeff_b,float);

  // Coefficient c
  vtkSetMacro(coeff_c,float);
  vtkGetMacro(coeff_c,float);

  // Don't treat the border of the volume
  vtkSetMacro(noborder,int);
  vtkGetMacro(noborder,int);

  void InitParam(       vtkImageData* in, vtkImageData* out);

  void UseInputOutputArray(float* array) { input_output_array = array; }

  void SetMinMaxX( int** minx, int** maxx);

  int* GetExtent() { return extent;}

protected:
  vtkImageFastSignedChamfer();
  ~vtkImageFastSignedChamfer();

  void ExecuteData(vtkDataObject *outData);

  void FastSignedChamfer2D();
  void FastSignedChamfer3DOld();
  void FastSignedChamfer3D();
  void FastSignedChamfer3DBorders();

//BTX
  float maxdist;

  float coeff_a;
  float coeff_b;
  float coeff_c;

  int tx,ty,tz,txy;
  unsigned long imsize;

  // We copy input data in float format
  vtkImageData *inputImage;
  int inputImage_allocated;

  vtkImageData *outputImage;

  // allows to use an array for the output image without allocation
  // and not to copy any input image to this output: faster ...
  float*           input_output_array;
  vtkFloatArray*   local_floatarray;

  // Limits for each line in X direction
  int** min_x;
  int** max_x;

  // Extent
  int extent[6];

  int noborder;

//ETX
};


#endif // _vtkImageFastSignedChamfer_h
