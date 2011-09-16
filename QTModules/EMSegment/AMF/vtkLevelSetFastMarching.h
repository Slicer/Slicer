/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkLevelSetFastMarching.h,v $
  Date:      $Date: 2006/01/06 17:57:55 $
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

// .NAME vtkLevelSetFastMarching - short description

// .SECTION Description
// vtkLevelSetFastMarching 
//
//

#ifndef __vtkLevelSetFastMarching_h
#define __vtkLevelSetFastMarching_h

#include "vtkEMSegment.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkImageToImageFilter.h"


//BTX
#include "vtkMinHeap.h"
//ETX

#define VTK_VAL_ACCEPTED 0
#define VTK_VAL_TRIAL    1
#define VTK_VAL_FAR      2

#define VTK_MODE_2D 2
#define VTK_MODE_3D 3


//BTX
class FM_TrialPoint;

void UpdateMinHeapPos(unsigned int image_pos, int pos, void* data);

//----------------------------------------------------------------------
//
class VTK_EMSEGMENT_EXPORT FM_TrialPoint {

public:
  short   x,y,z;
  int     impos;
  // minimal value for the neighbor in each direction:
  // 0,1,2 for X,Y,Z
  float   valmin[3];
  float   value;

  FM_TrialPoint()
  {
    x = y = z = impos = 0;
    value = 
    valmin[0] = 
    valmin[1] =
    valmin[2] = 1E5;
  }

  FM_TrialPoint(int px, int py, int pz, int im_pos, float val)
  {
    x     = px;
    y     = py;
    z     = pz;
    impos = im_pos;
    value = val;
    valmin[0] = 
    valmin[1] =
    valmin[2] = 1E5;
  }


  FM_TrialPoint& operator=(const FM_TrialPoint& p)
  {
    x     = p.x;
    y     = p.y;
    z     = p.z;
    impos = p.impos;
    value = p.value;
    valmin[0] = p.valmin[0];
    valmin[1] = p.valmin[1];
    valmin[2] = p.valmin[2];
    return *this;
  }


  void Init(int px, int py, int pz, int im_pos, float val)
  {
    x     = px;
    y     = py;
    z     = pz;
    impos = im_pos;
    value = val;
    valmin[0] = 
    valmin[1] =
    valmin[2] = 1E5;
  }


  void SetValue(float val) { value=val; }


  friend inline int operator < ( const FM_TrialPoint& p1, const FM_TrialPoint& p2)
  {
    return (p1.value < p2.value);
  }

  friend inline int operator > ( const FM_TrialPoint& p1, const FM_TrialPoint& p2)
  {
    return (p1.value > p2.value);
  }

  friend ostream& operator << (ostream&, const FM_TrialPoint& p);

}; // FM_TrialPoint


//ETX

//----------------------------------------------------------------------
class VTK_EXPORT vtkLevelSetFastMarching : public vtkImageToImageFilter
{
public:
  vtkTypeMacro(vtkLevelSetFastMarching,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Construct object to extract all of the input data.
  static vtkLevelSetFastMarching *New();

  //
  vtkSetMacro(dim,int);
  vtkGetMacro(dim,int);

  //
  vtkGetMacro(tx,int);

  //
  vtkGetMacro(ty,int);

  //
  vtkGetMacro(tz,int);

  //
  vtkSetMacro(cx,int);
  vtkGetMacro(cx,int);

  //
  vtkSetMacro(cy,int);
  vtkGetMacro(cy,int);

  //
  vtkSetMacro(cz,int);
  vtkGetMacro(cz,int);

  //
  vtkSetMacro(radius,float);
  vtkGetMacro(radius,float);

  //
  vtkSetMacro(maxTime,float);
  vtkGetMacro(maxTime,float);

  //
  vtkSetObjectMacro(mask,vtkImageData);
  vtkGetObjectMacro(mask,vtkImageData);

  //
  vtkSetObjectMacro(initimage,vtkImageData);
  vtkGetObjectMacro(initimage,vtkImageData);

  //
  vtkSetMacro(initiso,float);
  vtkGetMacro(initiso,float);

  //
  vtkSetMacro(initmaxdist,float);
  vtkGetMacro(initmaxdist,float);

  //
  vtkSetMacro(EvolutionScheme,int);
  vtkGetMacro(EvolutionScheme,int);

  // Initialization of the time image
  void Init( int cx, int cy, int cz, int radius);

  void UseOutputArray(float* array) { output_array = array; }

  void SetNarrowBand( int* band, int size);

  //
  void SetGaussianForce( float mean, float sd)
    {
      UseGaussianForce=1;
      IntensityMean=mean;
      IntensityStandardDeviation=sd;
    }

protected:
  vtkLevelSetFastMarching();
  ~vtkLevelSetFastMarching();

  vtkLevelSetFastMarching(const vtkLevelSetFastMarching&);
  void operator=(const vtkLevelSetFastMarching&);

  void ExecuteData(vtkDataObject* output);
  void InitParam();

//BTX
  // 2D or 3D processing
  int dim;

  // image size
  int tx,ty,tz,txy,imsize;

  // voxel size
  vtkFloatingPointType vs[3];

  // inverse of the squared voxel size
  vtkFloatingPointType ivs2[3];

  // center of the initial sphere
  int cx,cy,cz;

  // radius of the initial sphere
  float radius;

  // MinHeap binary tree object
  vtkMinHeap<FM_TrialPoint> mh;

  // Maximal treated evolution time
  float maxTime;

  // Output: time at which the surface goes though each point
  vtkImageData* T;

  // Input: the force that drives the evolution
  vtkImageData* force;
  unsigned char force_allocated;

  // status of the voxels (or pixels)
  unsigned char* status;

  // Position of each point in the MinHeap array
  unsigned int* mhPos;

  // Mask where to evolve the surface
  vtkImageData* mask;
  unsigned char mask_allocated;

  // Initialize with an isosurface : threshold
  float                initiso;

  // Initialize with an image
  vtkImageData* initimage;
  float                initmaxdist;

  // pointers to the first element of the images data
  float* T_buf;  
  float* force_buf;

  // 0: Sethian
  // 1: Dikjstra
  int  EvolutionScheme;

  unsigned char isotropic_voxels;

  float* output_array;

  // Narrow Band Information
  int*           narrowband;
  int            bandsize;  

  // Gaussian Intensity Based Force
  int         UseGaussianForce;
  float       IntensityMean;
  float       IntensityStandardDeviation;


//ETX

  // Initialization of the time image
  //  void Init( int cx, int cy, int cz, int radius);
  void Init2D( int cx, int cy, int radius);
  void Init3D( int cx, int cy, int cz, int radius);

  //
  void InitWithImage();

  // 
  void InitIsoSurf();

  // 
  void AddAcceptedPoint( short x, short y, short z, int pos);

  // Only used when initializing with a distance map
  void AddTrialPointsOld( short x, short y, short z, int pos);
  void AddTrialPoints( short x, short y, short z, int pos);

  // Computation of the new value at (x,y,z)
  float ComputeValue(short x, short y, short z, int pos);

  // Computation of the new value at (x,y,z)
  float ComputeValueDikjstra(short x, short y, short z, int pos);

  // Computation of the new value at (x,y,z)
  unsigned char ComputeValue(FM_TrialPoint& trial, float val, unsigned char dir);

  // Computation of the new value at (x,y,z)
  unsigned char ComputeValueSethian(FM_TrialPoint& trial, float val, unsigned char dir);

  // Take into account the voxel size 
  unsigned char ComputeValueSethian2(FM_TrialPoint& trial, float val, unsigned char dir);

};

#endif


