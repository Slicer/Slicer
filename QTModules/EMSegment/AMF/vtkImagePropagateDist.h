/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImagePropagateDist.h,v $
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
// Compute the distance to an isocontour by propagation
//


#ifndef _vtkImagePropagateDist_h
#define _vtkImagePropagateDist_h

#define POINT_NOT_PARSED    0
#define POINT_TRIAL         1
#define POINT_TRIAL_INLIST  2
#define POINT_SET_FRONT     3
#define POINT_SET           4

#include "vtkEMSegment.h"
#include "vtkImageToImageFilter.h"

//BTX

class PD_element {
  public:
    PD_element() {  
      x=y=z=0;
      state=POINT_NOT_PARSED;
      // initialize track to -1, mean no track point yet
      track         = -1;
      prev_neighbor = -1;
      skeleton = 0;
    };

    void Init( const short& px,
      const short& py,
      const short& pz,
      const unsigned char& st,
      const int& t) 
   {
     x=px;
     y=py;
     z=pz;
     state=st;
     track=t;
   }
 
  const short& X() {return x;}
  const short& Y() {return y;}
  const short& Z() {return z;}

  const int&           GetTrack()        {return track;}
  const char&          GetPrevNeighbor() {return prev_neighbor;}
  const unsigned char& GetState()        {return state;}
  const unsigned char& GetSkeleton()     {return skeleton;}

  void SetPos( const short& px, 
           const short& py,
           const short& pz
      )
    {
      x = px;
      y = py;
      z = pz;
    }

  void SetPosTrack( const short& px, 
           const short& py,
           const short& pz,
           const int& t
      )
    {
      x = px;
      y = py;
      z = pz;
      track = t;
    }

  void SetState( const unsigned char& st)
  {
    state=st;
  }

  void SetTrack( const int& t)
  {
    track=t;
  }

  void SetSkeleton( const unsigned char& s)
  {
    skeleton=s;
  }

  void SetPrevNeighbor( const int& n)
  {
    prev_neighbor=n;
  }

  short x;
  short y;
  short z;
  unsigned char state;
  int track;
  char prev_neighbor;
  unsigned char skeleton;
};

//ETX

class VTK_EMSEGMENT_EXPORT vtkImagePropagateDist : public vtkImageToImageFilter
{
public:
  static vtkImagePropagateDist *New();
  vtkTypeMacro(vtkImagePropagateDist,vtkImageToImageFilter);
  void PrintSelf(ostream& os, vtkIndent indent);
  
  // Description

  // Maximal distance >0
  vtkSetMacro(maxdist,float);
  vtkGetMacro(maxdist,float);

  // Minimal distance <0
  vtkSetMacro(mindist,float);
  vtkGetMacro(mindist,float);


  void InitParam(       vtkImageData* in, vtkImageData* out);

  void UseInputOutputArray(float* array) { input_output_array = array; }

  void SetMinMaxX( int** minx, int** maxx);

  int* GetExtent() { return extent;}

protected:
  vtkImagePropagateDist();
  ~vtkImagePropagateDist();

  void ExecuteData(vtkDataObject *outData);

  void PreComputeDistanceArray();

  void FreeDistanceArray();

  void InitLists();

  void FreeLists();

  void PropagateDanielsson2D();

  void PropagateDanielsson3D();

  void SaveTrajectories2D( int num);
  void SaveTrajectories3D( int num);

  void SaveProjection( int num);

  void SaveState( int num);
  void SaveSkeleton( int num);
  void SaveDistance( int num);


//BTX
  float maxdist;
  float mindist;

  int tx,ty,tz,txy;
  long imsize;

  // We copy input data in float format
  vtkImageData *inputImage;
  int inputImage_allocated;

  vtkImageData *outputImage;

  // allows to use an array for the output image without allocation
  // and not to copy any input image to this output: faster ...
  float* input_output_array;

  // Limits for each line in X direction
  int** min_x;
  int** max_x;

  // Extent
  int extent[6];

  int noborder;

  // precomputed table for the square
  int*    sq;
  int     sq_size;
  double*  sqroot;
  int     sqroot_size;
  
  double***  distance;

  // List information
  int*      list0;  
  int*      list1;  
  int*      list_remaining_trial;  
  int       list0_size;
  int       list1_size;
  int       list_remaining_trial_size;  
  int       list_maxsize;
  PD_element* list_elts;

  unsigned char save_intermediate_images;
  unsigned char trial_loop;

//ETX
};


#endif // _vtkImagePropagateDist_h
