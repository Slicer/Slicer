/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkLevelSetFastMarching.cxx,v $
  Date:      $Date: 2006/05/26 19:54:50 $
  Version:   $Revision: 1.9 $

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

#include "vtkLevelSetFastMarching.h"
#include "vtkObjectFactory.h"
#include "vtkStructuredPointsWriter.h"
#include "vtkFloatArray.h"

#ifndef _WIN32
#include <strings.h>
#endif

//
//  Code by Karl Krissian oct-nov 2001
//
//  Brigham and Women's hospital
//  Surgical Planning Laboratory
//

#define macro_min(n1,n2) ((n1)<(n2)?(n1):(n2))
#define macro_max(n1,n2) ((n1)>(n2)?(n1):(n2))
#define macro_abs(n)     ((n)>0?(n):-(n))

#define EPSILON 1E-5


//--------------------------------------------------
ostream& operator << (ostream& os, const FM_TrialPoint& p)
//       -----------
{

  os << p.value;
  return os;

} // operator << ( , const FM_TrialPoint&)

//----------------------------------------------------------------------
//  Solve  x^2 - 2.b2.x + c = 0
//
inline float SolveQuad(double b2,double c)
{

  double delta;

  delta = b2*b2-c;

  if (delta<0) {
    fprintf(stderr,"SolveQuad() \t Delta <0 \n");
    return 0;
  }

  return b2+sqrt(delta);

}

//----------------------------------------------------------------------
//  Solve a.x^2 - 2.b2.x + c = 0
//
inline float SolveQuad(double a, double b2,double c)
{

  double delta;

  delta = b2*b2-a*c;

  if (delta<0) {
    fprintf(stderr,"SolveQuad() \t Delta <0 \n");
    return 0;
  }

  return (b2+sqrt(delta))/a;

}

//----------------------------------------------------------------------
void UpdateMinHeapPos(const FM_TrialPoint& tp, int pos, void* data)
{
  unsigned int* mhpos     = (unsigned int*) data;

  mhpos[tp.impos]=pos;
}

//-----------------------------------------------------------------------
vtkLevelSetFastMarching* vtkLevelSetFastMarching::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkLevelSetFastMarching");
  if(ret)
    {
    return (vtkLevelSetFastMarching*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkLevelSetFastMarching;

} // vtkLevelSetFastMarching::New()


//----------------------------------------------------------------------
// Construct object to extract all of the input data.
//
vtkLevelSetFastMarching::vtkLevelSetFastMarching() : mh(100000)
{

  mask   = NULL;
  
  force  = NULL;
  T      = NULL;
  status = NULL;
  mhPos  = NULL;
  
  
  dim = VTK_MODE_2D;
  tx = ty = tz = 0;
  maxTime = 100;

  force_allocated = 0;
  mask_allocated  = 0;


  initimage    = NULL;
  initmaxdist  = 1;

  initiso      = -1E10;

  isotropic_voxels = 0;

  EvolutionScheme = 0; // Sethian

  output_array    = NULL;

  //  this->ExecuteInformation();

  narrowband = NULL;
  bandsize   = 0;

  UseGaussianForce = 0;

} // vtkLevelSetFastMarching::vtkLevelSetFastMarching()


//----------------------------------------------------------------------
vtkLevelSetFastMarching::~vtkLevelSetFastMarching()
{
  if (mask_allocated)  mask ->Delete();
  if (force_allocated) force->Delete();

  if (status != NULL) {
    delete [] status;
    status = NULL;
  }

  if (mhPos != NULL) {
    delete [] mhPos;
    mhPos = NULL;
  }

} // ~vtkLevelSetFastMarching

//----------------------------------------------------------------------
void vtkLevelSetFastMarching::InitParam()
{

  int type;

  //  fprintf(stderr,"vtkLevelSetFastMarching::InitParam() begin\n");

  // Get force image from input
  force  = this->GetInput();
  if (force == NULL)
    {
      vtkErrorMacro("Missing input");
      return;
    }
  // check the image is in float format, or convert
  type = force->GetScalarType();
  if (type != VTK_FLOAT) {
    vtkDebugMacro(<<"making a copy of the input into float format");
    // Create a copy of the data
    force = vtkImageData::New();
    force->SetScalarType( VTK_FLOAT);
    force->SetNumberOfScalarComponents(1);
    force->SetDimensions( this->GetInput()->GetDimensions());
    force->SetOrigin(     this->GetInput()->GetOrigin());
    force->SetSpacing(    this->GetInput()->GetSpacing());

    force->CopyAndCastFrom(this->GetInput(),
               this->GetInput()->GetExtent());
    force_allocated = 1;
  }


  // The mask must be UNSIGNED_CHAR
  if (mask != NULL) {
    type = mask->GetScalarType();
    if (type != VTK_UNSIGNED_CHAR) {
      vtkDebugMacro(<<"convert mask to unsigned char");
      // Create a copy of the data
      vtkImageData* mask1;
      mask1 = vtkImageData::New();
      mask1->SetScalarType( VTK_UNSIGNED_CHAR);
      mask1->SetNumberOfScalarComponents(1);
      mask1->SetDimensions( this->mask->GetDimensions());
      mask1->SetOrigin(     this->mask->GetOrigin());
      mask1->SetSpacing(    this->mask->GetSpacing());
      
      mask1->CopyAndCastFrom(this->mask,
                 this->mask->GetExtent());
      mask           = mask1;
      mask_allocated = 1;
    }
  }


  // Image size
  tx = force->GetDimensions()[0];
  ty = force->GetDimensions()[1];
  tz = force->GetDimensions()[2];
  txy = tx*ty;
  imsize = txy*tz;

  force->GetSpacing(vs);
  ivs2[0] = 1.0/vs[0]/vs[0];
  ivs2[1] = 1.0/vs[1]/vs[1];
  ivs2[2] = 1.0/vs[2]/vs[2];

  //  fprintf(stderr," voxel size %f %f %f \n",vs[0],vs[1],vs[2]);

  // check for isotropic voxels
  isotropic_voxels = ((vs[0]==vs[1])&&(vs[0]==vs[2]));

  // Check the initimage
  if (initimage != NULL) {
    if (initimage->GetScalarType() != VTK_FLOAT) {
      fprintf(stderr,"The initialization image must be of type VTK_FLOAT \n");
      initimage = NULL;
    }
    if ((initimage->GetDimensions()[0] != tx) ||
    (initimage->GetDimensions()[1] != ty) ||
    (initimage->GetDimensions()[2] != tz)) {
      fprintf(stderr,"The initialization image must have the same dimensions as the Force \n");
      initimage = NULL;
    }
  }


  // Default dimension mode
  if (tz>4) dim = VTK_MODE_3D; else dim = VTK_MODE_2D;        


  // Get the time image (output of the algorithm)
  T      = this->GetOutput();
  
  T->SetDimensions(this->GetInput()->GetDimensions());
  T->SetSpacing(   this->GetInput()->GetSpacing());
  T->SetScalarType(VTK_FLOAT); 
  T->SetNumberOfScalarComponents(1);

  if (output_array != NULL) {
    vtkFloatArray* da = vtkFloatArray::New();
    da->SetArray(output_array,imsize,1);
    T->GetPointData()->SetScalars(da);
  } 
  else {
    T->AllocateScalars();
  }

  // initialization of the buffers:
  // always points to the first element of the images data
  T_buf     = (float*) (this->T    ->GetScalarPointer());
  force_buf = (float*) (this->force->GetScalarPointer());

    
  // Initialization of status image
  if (status==NULL)
    status = new unsigned char[imsize];

  // Initialization of mhPos image
  if (mhPos==NULL)
    mhPos = new unsigned int[imsize];

  // initializing mhPos to 0
  memset(mhPos,0,imsize*sizeof(unsigned int));
  //  for(x=0;x<imsize;x++) mhPos[x] = 0;
    
  // Set the callback function to update the position image
  // when a value is moved in the MinHeap structure
  mh.SetMoveFunction( UpdateMinHeapPos, this->mhPos);

  // Empty the min heap structure just in case
  mh.RemoveAll();

  //  fprintf(stderr,"vtkLevelSetFastMarching::InitParam() end \n");

} // vtkLevelSetFastMarching::InitParam()


//----------------------------------------------------------------------------
void vtkLevelSetFastMarching::SetNarrowBand( int* band, int size)
{
  narrowband = band;
  bandsize = size;
}


//----------------------------------------------------------------------
void vtkLevelSetFastMarching::ExecuteData(vtkDataObject *outData)
{
  
  FM_TrialPoint p;

  //  InrImage* ImEvol;
  float last_value = 0;
  int iterations;


  //  Si dim==VTK_MODE_2D AlorsFait
  //    ImEvol = new InrImage(tx,ty,50,WT_FLOAT,"evol_fm.ami.gz");

  //  fprintf(stderr,"vtkLevelSetFastMarching::Execute() begin ----------------------------- \n");
  //  fprintf(stderr,"vtkLevelSetFastMarching::Execute() initparam \n");

  InitParam();

  if (initimage == NULL) 
    Init(cx,cy,cz, (int) radius);
  else
    if (initiso == -1E10)  
      InitWithImage();
    else
      InitIsoSurf();

  iterations = 0;
  //  printf("iterations = %7d",iterations);
  //  fflush(stdout);

  do {

    p = this->mh.GetMin();

    if ( p.value > last_value+100 ){
      last_value = p.value;
      printf("\n current val = %f \t it %d \n",last_value,iterations);
    } // end if


    T_buf[p.impos] = p.value;

    AddAcceptedPoint(p.x,p.y,p.z,p.impos);

    iterations++;
    if (((iterations % 10000==0)&&(dim==VTK_MODE_3D))||
        ((iterations % 200  ==0)&&(dim==VTK_MODE_2D))) {
      
      //      printf("\b\b\b\b\b");
      // printf("%5d",iterations/100);
      //fflush(stdout);
      //      sprintf(Tname,"T%d.ami.gz",iterations/100);
      //      this->T->Sauve(Tname);
      
//    if( (dim==VTK_MODE_2D) ){
//    n = iterations/200;
//        if( n<ImEvol->_tz ){
//    for(x=0; x<=tx-1; x++){
//    for(y=0; y<=ty-1; y++) {
//    ImEvol->BufferPos(x,y,n);
//    ImEvol->FixeValeur((*this->T)(x,y,0));
//    }
//    }
//        } // end if

    } // end if
    
    //      cout << this->mh << endl;
  
  } while (!( (p.value>=this->maxTime) || (this->mh.Size() == 0) ));

  //  printf("\n");
  

//  if ( dim==VTK_MODE_2D ){
//    ImEvol->Sauve();
//    delete ImEvol;
//  } // end if  


  // Give back a distance function in the case of isosurface
  // For LevelSet application: negative distance for intensity
  // values higher than the threshold
  
  if (initiso != -1E10) {
    float* init_buf;
    int    pos;
    init_buf   = (float*)this->initimage->GetScalarPointer();
    
    for(pos=0;pos<this->initimage->GetNumberOfPoints();pos++) 
      if (init_buf[pos]<initiso) 
    T_buf     [pos] *= -1;
  }

  //  fprintf(stderr,"vtkLevelSetFastMarching::Execute() end ----------------------------- \n");

  // Check for the maximum

} // vtkLevelSetFastMarching::Execute()


//----------------------------------------------------------------------
//
void vtkLevelSetFastMarching::AddAcceptedPoint( short x, short y, short z, int pos)
//                    ----------------
{

  unsigned char* status_buf;
  unsigned char* mask_buf;
  register int neighbors[6];
  register short neighb_x [6];
  register short neighb_y [6];
  register short neighb_z [6];
  // direction: 0,1,2 for X,Y,Z
  unsigned char dir[6];
  int n,nb;
  register int nx,ny,nz;
  double t,F,cost,val1;
  FM_TrialPoint   trial;

  float      value; // time of the accepted point

  // setup the neighbors  
  for (nb=0; nb<=5; nb++) {
    neighbors[nb] = pos;
    neighb_x[nb]  = x;
    neighb_y[nb]  = y;
    neighb_z[nb]  = z;
  }

  nb = 0;
  if (x-1>=0) { neighb_x[nb] = x-1; neighbors[nb] -=1;   dir[nb]=0; nb++;}
  if (x+1<tx) { neighb_x[nb] = x+1; neighbors[nb] +=1;   dir[nb]=0; nb++;}
  if (y-1>=0) { neighb_y[nb] = y-1; neighbors[nb] -=tx;  dir[nb]=1; nb++;}
  if (y+1<ty) { neighb_y[nb] = y+1; neighbors[nb] +=tx;  dir[nb]=1; nb++;}
  if (z-1>=0) { neighb_z[nb] = z-1; neighbors[nb] -=txy; dir[nb]=2; nb++;}
  if (z+1<tz) { neighb_z[nb] = z+1; neighbors[nb] +=txy; dir[nb]=2; nb++;}


  status_buf  = this->status+pos;
  *status_buf = VTK_VAL_ACCEPTED;

  value = T_buf[pos];

  // Tag as trial all the neighbors of A that are Known
  // if the neighbor is far, remove and add to the set of Trial
  for (n=0; n<=nb-1; n++) {

    status_buf  =  this->status+neighbors[n];

    // Check for the mask
    if ( this->mask != NULL ) {
      mask_buf  = (unsigned char*) this->mask->GetScalarPointer();
      mask_buf += neighbors[n];
      if (*mask_buf <0.5) continue;
    }

    switch (*status_buf) {
      case VTK_VAL_FAR:
    nx = neighb_x[n];
    ny = neighb_y[n];
    nz = neighb_z[n];
    *status_buf = VTK_VAL_TRIAL;

    //        val = this->ComputeValue(nx,ny,nz);
    //    if (val>maxTime) val = maxTime;

    t    =  value;
    if (force==initimage) 
      cost = 1;
    else {
          F    =  force_buf[neighbors[n]];
      if (UseGaussianForce) 
        F = exp(-(F-IntensityMean)*(F-IntensityMean)/
            IntensityStandardDeviation/IntensityStandardDeviation);
      if (F < 1E-5)
        cost = 1E5;
      else
        cost = 1.0/F;
    }

    if (isotropic_voxels)  
      val1 = t+cost;
    else
      val1 = t+cost*vs[dir[n]];
      
    if (val1>maxTime) val1 = maxTime;

    //    if ((val-val1)*(val-val1)>1E-2) 
    //      fprintf(stderr," (%d %d %d) val %f val1 %f \n",x,y,z);
    trial.Init(nx,ny,nz,neighbors[n],val1);
    trial.valmin[dir[n]] = value;

    this->mh += trial;

      break;

      case VTK_VAL_TRIAL:

    trial=mh[mhPos[neighbors[n]]];

    // if the value has changed,
    // then update the Min Heap structure
        if (this->ComputeValue(trial,value,dir[n]))
      this->mh.ChangeValue(  mhPos[neighbors[n]],trial);

        break;
    } // end switch
  
  }
    


} // AddAcceptedPoint()


//----------------------------------------------------------------------
// Solution: ne pas s'approcher trop du bord ...
//
void vtkLevelSetFastMarching::AddTrialPointsOld( short x, short y, short z, int pos)
//                    -----------------
{

  unsigned char* status_buf;
  unsigned char* mask_buf;

  register int neighbors[6];
  register short neighb_x [6];
  register short neighb_y [6];
  register short neighb_z [6];
  int n,nb;

  // direction: 0,1,2 for X,Y,Z
  unsigned char dir[6];
  int nx,ny,nz;
  FM_TrialPoint trial;
  float valmin;

  // setup the neighbors  
  for (nb=0; nb<=5; nb++) {
    neighbors[nb] = pos;
    neighb_x[nb]  = x;
    neighb_y[nb]  = y;
    neighb_z[nb]  = z;
  }

  nb = 0;
  if (x-1>=0) { neighb_x[nb] = x-1; neighbors[nb] -=1;   dir[nb]=0; nb++;}
  if (x+1<tx) { neighb_x[nb] = x+1; neighbors[nb] +=1;   dir[nb]=0; nb++;}
  if (y-1>=0) { neighb_y[nb] = y-1; neighbors[nb] -=tx;  dir[nb]=1; nb++;}
  if (y+1<ty) { neighb_y[nb] = y+1; neighbors[nb] +=tx;  dir[nb]=1; nb++;}
  if (z-1>=0) { neighb_z[nb] = z-1; neighbors[nb] -=txy; dir[nb]=2; nb++;}
  if (z+1<tz) { neighb_z[nb] = z+1; neighbors[nb] +=txy; dir[nb]=2; nb++;}

  //  mask_buf0  = (unsigned char*) this->mask->GetScalarPointer();


  // Tag as trial all the neighbors of A that are Known
  // if the neighbor is far, remove and add to the set of Trial
  for (n=0; n<=nb-1; n++) {

    status_buf  =  this->status+neighbors[n];
    if ( this->mask != NULL ) {
      mask_buf  = (unsigned char*) this->mask->GetScalarPointer();
      mask_buf += neighbors[n];
    }
    switch (*status_buf) {
      case VTK_VAL_FAR:
        if( (this->mask==NULL) || 
        ((this->mask != NULL) && (*mask_buf > 0.5)) 
      )
      {
      nx  = neighb_x[n];
      ny  = neighb_y[n];
      nz  = neighb_z[n];
      pos = neighbors[n];

      *status_buf = VTK_VAL_TRIAL;


        trial.Init(nx,ny,nz,pos,maxTime);

      // initialize the minimums in each direction
      if (nx>0)    trial.valmin[0]=T_buf[pos-1];
      else         trial.valmin[0]=T_buf[pos];
      if (nx<tx-1) trial.valmin[0]=macro_min(trial.valmin[0],T_buf[pos+1]);

      if (ny>0)    trial.valmin[1]=T_buf[pos-tx];
      else         trial.valmin[1]=T_buf[pos];
      if (ny<ty-1) trial.valmin[1]=macro_min(trial.valmin[1],T_buf[pos+tx]);
      if (dim==VTK_MODE_3D) {
        if (nz>0)    trial.valmin[2]=T_buf[pos-txy];
        else         trial.valmin[2]=T_buf[pos];
        
        if (nz<tz-1) trial.valmin[2]=macro_min(trial.valmin[2],T_buf[pos+txy]);
      }

      // put the value in the current direction to the maximum
      // in order to process the point
      valmin = trial.valmin[dir[n]];
      trial.valmin[dir[n]]=maxTime;
          this->ComputeValue(trial,valmin,dir[n]); // set dir to 4 for initialization
      this->mh += trial;

    } // end if
      break;
    } // end switch
  
  }
    


} // AddTrialPointsOld()


//----------------------------------------------------------------------
// Solution: ne pas s'approcher trop du bord ...
// x in [2,tx-3]
// y in [2,ty-3]
// si 3D alors z in [2,tz-3]
//
void vtkLevelSetFastMarching::AddTrialPoints( short x, short y, short z, int pos)
//                    --------------
{

  
  unsigned char* mask_buf;
  int npos,ndir;  // direction: 0,1,2 for X,Y,Z
  register FM_TrialPoint trial;
  register float valmin;

  if (mask!=NULL)
    mask_buf  = (unsigned char*) this->mask->GetScalarPointer();

  // (x-1,y,z)
  npos = pos-1;
  ndir=0; 
  if ((status[npos]==VTK_VAL_FAR)&&
      ((this->mask==NULL)||((this->mask!=NULL) && (mask_buf[npos]>0.5)))
      )
  {
    status[npos] = VTK_VAL_TRIAL;
    trial.Init(x-1,y,z,npos,maxTime);
    // initialize the minimums in each direction
    trial.valmin[0]=macro_min(T_buf[npos-1], T_buf[npos+1]);
    trial.valmin[1]=macro_min(T_buf[npos-tx],T_buf[npos+tx]);
    if (dim==VTK_MODE_3D) {
      trial.valmin[2]=macro_min(T_buf[npos-txy],T_buf[npos+txy]);
    }
    // put the value in the current direction to the maximum
    // in order to process the point
    valmin = trial.valmin[ndir];
    trial.valmin[ndir]=maxTime;
    this->ComputeValue(trial,valmin,ndir); 
    this->mh += trial;
  } // end if

  // (x+1,y,z)
  npos = pos+1;
  ndir=0; 
  if ((status[npos]==VTK_VAL_FAR)&&
      ((this->mask==NULL)||((this->mask!=NULL) && (mask_buf[npos]>0.5)))
      )
  {
    status[npos] = VTK_VAL_TRIAL;
    trial.Init(x+1,y,z,npos,maxTime);
    // initialize the minimums in each direction
    trial.valmin[0]=macro_min(T_buf[npos-1], T_buf[npos+1]);
    trial.valmin[1]=macro_min(T_buf[npos-tx],T_buf[npos+tx]);
    if (dim==VTK_MODE_3D) {
      trial.valmin[2]=macro_min(T_buf[npos-txy],T_buf[npos+txy]);
    }
    // put the value in the current direction to the maximum
    // in order to process the point
    valmin = trial.valmin[ndir];
    trial.valmin[ndir]=maxTime;
    this->ComputeValue(trial,valmin,ndir); 
    this->mh += trial;
  } // end if

  // (x,y-1,z)
  npos = pos-tx;
  ndir=1; 
  if ((status[npos]==VTK_VAL_FAR)&&
      ((this->mask==NULL)||((this->mask!=NULL) && (mask_buf[npos]>0.5)))
      )
  {
    status[npos] = VTK_VAL_TRIAL;
    trial.Init(x,y-1,z,npos,maxTime);
    // initialize the minimums in each direction
    trial.valmin[0]=macro_min(T_buf[npos-1], T_buf[npos+1]);
    trial.valmin[1]=macro_min(T_buf[npos-tx],T_buf[npos+tx]);
    if (dim==VTK_MODE_3D) {
      trial.valmin[2]=macro_min(T_buf[npos-txy],T_buf[npos+txy]);
    }
    // put the value in the current direction to the maximum
    // in order to process the point
    valmin = trial.valmin[ndir];
    trial.valmin[ndir]=maxTime;
    this->ComputeValue(trial,valmin,ndir); 
    this->mh += trial;
  } // end if

  // (x,y+1,z)
  npos = pos+tx;
  ndir=1; 
  if ((status[npos]==VTK_VAL_FAR)&&
      ((this->mask==NULL)||((this->mask!=NULL) && (mask_buf[npos]>0.5)))
      )
  {
    status[npos] = VTK_VAL_TRIAL;
    trial.Init(x,y+1,z,npos,maxTime);
    // initialize the minimums in each direction
    trial.valmin[0]=macro_min(T_buf[npos-1], T_buf[npos+1]);
    trial.valmin[1]=macro_min(T_buf[npos-tx],T_buf[npos+tx]);
    if (dim==VTK_MODE_3D) {
      trial.valmin[2]=macro_min(T_buf[npos-txy],T_buf[npos+txy]);
    }
    // put the value in the current direction to the maximum
    // in order to process the point
    valmin = trial.valmin[ndir];
    trial.valmin[ndir]=maxTime;
    this->ComputeValue(trial,valmin,ndir); 
    this->mh += trial;
  } // end if

  if (dim==VTK_MODE_3D) {
    // (x,y,z-1)
    npos = pos-txy;
    ndir=2; 
    if ((status[npos]==VTK_VAL_FAR)&&
    ((this->mask==NULL)||((this->mask!=NULL) && (mask_buf[npos]>0.5)))
    )
      {
    status[npos] = VTK_VAL_TRIAL;
    trial.Init(x,y,z-1,npos,maxTime);
    // initialize the minimums in each direction
    trial.valmin[0]=macro_min(T_buf[npos-1], T_buf[npos+1]);
    trial.valmin[1]=macro_min(T_buf[npos-tx],T_buf[npos+tx]);
    if (dim==VTK_MODE_3D) {
      trial.valmin[2]=macro_min(T_buf[npos-txy],T_buf[npos+txy]);
    }
    // put the value in the current direction to the maximum
    // in order to process the point
    valmin = trial.valmin[ndir];
    trial.valmin[ndir]=maxTime;
    this->ComputeValue(trial,valmin,ndir); 
    this->mh += trial;
      } // end if
    
    // (x,y,z+1)
    npos = pos+txy;
    ndir=2; 
    if ((status[npos]==VTK_VAL_FAR)&&
    ((this->mask==NULL)||((this->mask!=NULL) && (mask_buf[npos]>0.5)))
    )
      {
    status[npos] = VTK_VAL_TRIAL;
    trial.Init(x,y,z+1,npos,maxTime);
    // initialize the minimums in each direction
    trial.valmin[0]=macro_min(T_buf[npos-1], T_buf[npos+1]);
    trial.valmin[1]=macro_min(T_buf[npos-tx],T_buf[npos+tx]);
    if (dim==VTK_MODE_3D) {
      trial.valmin[2]=macro_min(T_buf[npos-txy],T_buf[npos+txy]);
    }
    // put the value in the current direction to the maximum
    // in order to process the point
    valmin = trial.valmin[ndir];
    trial.valmin[ndir]=maxTime;
    this->ComputeValue(trial,valmin,ndir); 
    this->mh += trial;
      } // end if

  } // mode 3D


} // AddTrialPoints()


//----------------------------------------------------------------------
//
float vtkLevelSetFastMarching::ComputeValue( short x, short y, short z, int pos)
//                     ------------
{

  switch (EvolutionScheme) {
    //    case 0: return ComputeValueSethian( x,y,z,pos);
    case 1: 
      return ComputeValueDikjstra(x,y,z,pos);
    default:
      return 0.;
  }

} // ComputeValue()


//----------------------------------------------------------------------
//
inline float vtkLevelSetFastMarching::ComputeValueDikjstra( short x, short y, short z, int pos)
//                            --------------------
{

#define macro_swap(a,b) tmp=a; a=b; b=tmp;

  register double valmin;
  double F;
  register float*   T_buf1 = T_buf+pos;


  if ( x>0    ) {
    valmin = *(T_buf1-1);
  } else {
    valmin = *T_buf1;
  } // end if

  if ( x<tx-1 ) {
    valmin = macro_min(valmin,*(T_buf1+1));
  } // end if

  if ( y>0    ) {
    valmin = macro_min(valmin,*(T_buf1-tx));
  } else {
    valmin = macro_min(valmin,*T_buf1);
  } // end if

  if ( y<ty-1 ) {
    valmin = macro_min(valmin,*(T_buf1+tx));
  } // end if

  if ( dim==VTK_MODE_3D ) {
    if ( z>0    ) {
      valmin = macro_min(valmin,*(T_buf1-txy));
    } else {
      valmin = macro_min(valmin,*T_buf1);
    } // end if

    if ( z<tz-1 ) {
      valmin = macro_min(valmin,*(T_buf1+txy));
    } // end if

  } // end if

  if (force==initimage) 
    F = 1;
  else {
    F = *(force_buf+pos);
    if (UseGaussianForce) 
      F = exp(-(F-IntensityMean)*(F-IntensityMean)/
          IntensityStandardDeviation/IntensityStandardDeviation);
 }

  if (F<1E-5) 
    return maxTime;
  else
    return valmin + 1.0/F;

  return 1;

} // LevelSetFastMarching::ComputeValueDikjstra()
 

//----------------------------------------------------------------------
//
unsigned char vtkLevelSetFastMarching::ComputeValue( FM_TrialPoint& trial, float val, unsigned char dir)
//                                    ------------
{

  switch (EvolutionScheme) {
    case 0: 
      if (isotropic_voxels)
    return ComputeValueSethian( trial,val,dir);
      else
    return ComputeValueSethian2( trial,val,dir);
    case 1: 
      trial.SetValue(ComputeValueDikjstra(trial.x,trial.y,trial.z,trial.impos));
      return 1;
    default:
      return 0;
  }

} // ComputeValue()


//----------------------------------------------------------------------
//
// For isotropic voxels
// update the minimal value of the neighbors in the direction "dir"
// then solves the second order equation
// 
// Output
//    computes trial.value
unsigned char vtkLevelSetFastMarching::ComputeValueSethian( FM_TrialPoint& trial, float value, unsigned char dir)
//                                    -------------------
{

#define macro_swap(a,b) tmp=a; a=b; b=tmp;

  register double val0,val1,val2;
  double tmp;
  double F;
  double cost;
  double cost2;
  double d1,d2;
  double  res;

  if (value<trial.valmin[dir])
    trial.valmin[dir] = value;
  else 
    return 0;

  val0 = trial.valmin[0];
  val1 = trial.valmin[1];
  val2 = trial.valmin[2];

  if ( val1<val0 ) {
    macro_swap(val0,val1);
  } // end if

  if ( dim==VTK_MODE_3D ) {
    if ( val2<val0 ) {
      tmp = val2;
      val2 = val1;
      val1 = val0;
      val0 = tmp;
    } else
    if ( val2< val1 ) {
      macro_swap(val1,val2);
    } // end if      
  } // end if

  if (force==initimage) 
    cost = 1;
  else {
    F = *(force_buf+trial.impos);
    if (UseGaussianForce) 
      F = exp(-(F-IntensityMean)*(F-IntensityMean)/
          IntensityStandardDeviation/IntensityStandardDeviation);
    if (F==1) 
      cost=1;
    else
      if (F<1E-5)  cost = 1E5;
      else         cost = 1.0/F;
  }
    
  d1 = val1-val0;

  if ( d1 > cost ) 
    res = val0+cost;
  else {
    
    if (val1>=maxTime) 
      res = maxTime;
    else
      if ( dim==VTK_MODE_2D ) 
        res = SolveQuad(
            (val0+val1)/2.0,
            (val0*val0+val1*val1-cost*cost)/2.0
            );
      else {
    d1    = val2-val0;
    d2    = val2-val1;
    cost2 = cost*cost;
    if ( d1*d1+d2*d2 > cost2 ) 
      res = SolveQuad(
              (val0+val1)/2.0, 
              (val0*val0+val1*val1-cost2)/2.0
              );
    else 
      if (val2>=maxTime) 
        res = maxTime;
      else
        res = SolveQuad(
                (val0+val1+val2)/3.0,
                (val0*val0+val1*val1+val2*val2-cost2)/3.0
                );
      } // end if
  } // end if

  if (res>maxTime)
    trial.SetValue(maxTime);
  else
    trial.SetValue(res);

//  fprintf(stderr,"LevelSetFastMarching::ComputeValue() \t case not found \n");
  return 1;

} // LevelSetFastMarching::ComputeValueSethian()
 

//----------------------------------------------------------------------
//
// Take into account the voxel size
//
//
// Input
//    vs[i]   voxel size in the direction i
//    ivs2[i] inverse of the squared voxel size in the direction i
//
// Output
//    computes trial.value
//
inline unsigned char vtkLevelSetFastMarching::ComputeValueSethian2( FM_TrialPoint& trial, float value, unsigned char dir)
//                            -------------------
{

#define macro_swap(a,b) tmp=a; a=b; b=tmp;

  register double val[3];
  register double val1[3];
  double F;
  double cost;
  double cost2;
  double d1,d2;
  double  res;

  // indices
  unsigned char i0,i1,i2,tmp;


  if (value<trial.valmin[dir])
    trial.valmin[dir] = value;
  else 
    return 0;

  val[0] = trial.valmin[0];
  val[1] = trial.valmin[1];
  val[2] = trial.valmin[2];

  i0 = 0;
  i1 = 1;
  i2 = 2;

  if ( val[i1]<val[i0] ) {
    macro_swap(i1,i0);
  } // end if

  if ( dim==VTK_MODE_3D ) {
    if ( val[i2]<val[i0] ) {
      tmp = i2;
      i2 = i1;
      i1 = i0;
      i0 = tmp;
    } else
    if ( val[i2]< val[i1] ) {
      macro_swap(i1,i2);
    } // end if      
  } // end if

  if (force==initimage) 
    cost = 1;
  else {
    F = *(force_buf+trial.impos);
    if (UseGaussianForce) 
      F = exp(-(F-IntensityMean)*(F-IntensityMean)/
          IntensityStandardDeviation/IntensityStandardDeviation);
    if (F==1) 
      cost=1;
    else
      if (F<1E-5)  cost = 1E5;
      else         cost = 1.0/F;
  }
    
  d1 = val[i1]-val[i0];

  if ( d1 > cost*vs[i0] ) 
    res = val[i0]+cost*vs[i0];
  else {
    
    if (val[i1]>=maxTime) 
      res = maxTime;
    else {
    cost2    = cost*cost;
    val1[i0] = val[i0]*ivs2[i0];
    val1[i1] = val[i1]*ivs2[i1];
    if ( dim==VTK_MODE_2D )
      res = SolveQuad(ivs2[i0]+ivs2[i1],
              val1[i0]+val1[i1],
              val[i0]*val1[i0]+val[i1]*val1[i1]-cost2
              );
    else {
      d1    = val[i2]-val[i0];
      d2    = val[i2]-val[i1];
      if ( d1*d1*ivs2[i0]+d2*d2*ivs2[i1] > cost2 ) 
        res = SolveQuad(ivs2[i0]+ivs2[i1],
                val1[i0]+val1[i1],
                val[i0]*val1[i0]+val[i1]*val1[i1]-cost2
                );
      else 
        if (val[i2]>=maxTime) 
          res = maxTime;
        else {
          val1[i2] = val[i2]*ivs2[i2];
          res = SolveQuad(ivs2[i0]+ivs2[i1]+ivs2[i2],
                  val1[i0]+val1[i1]+val1[i2],
                  val[i0]*val1[i0]+val[i1]*val1[i1]+val[i2]*val1[i2]-cost2
                  );
        }
    } // end if
    } // end if
  } // end if

  if (res>maxTime)
    trial.SetValue(maxTime);
  else
    trial.SetValue(res);

//  fprintf(stderr,"LevelSetFastMarching::ComputeValue() \t case not found \n");
  return 1;

} // LevelSetFastMarching::ComputeValueSethian2()
 

//----------------------------------------------------------------------
void vtkLevelSetFastMarching::PrintSelf(ostream& os, vtkIndent indent)
{
  // Nothing for the moment ...
}

//----------------------------------------------------------------------
//
void vtkLevelSetFastMarching::Init(int cx, int cy, int cz, int radius)
//
{

  if (this->dim==VTK_MODE_3D)
    this->Init3D(cx,cy,cz,radius);
  else
    this->Init2D(cx,cy,radius);


} // vtkLevelSetFastMarching::Init()


//----------------------------------------------------------------------
//
void vtkLevelSetFastMarching::Init2D(int cx, int cy, int radius)
//                    -----
{
    int   x, y, myrad2;
    float t,dt,NG;
    int   n;
    int   margin;
    long pos;

    float*        surf;
    float         newval,val;
    float         NGx, NGy;

    float*         T_buf1;
    unsigned char* status_buf;
    float*         surf_buf;
    float*         force_buf1;

  margin=2;

  fprintf(stderr,"vtkLevelSetFastMarching::Init2D() 1 \n");

  myrad2 = radius+margin+margin;

  fprintf(stderr,"vtkLevelSetFastMarching::Init2D() 2 --- \n");
  // Initialisation to maxTime and VAL_FAR

  T_buf1      = T_buf;

  fprintf(stderr,"vtkLevelSetFastMarching::Init2D() 3 \n");
  status_buf = this->status;

  for(pos=0;pos<this->T->GetNumberOfPoints();pos++) {
    T_buf1[pos]      = maxTime;
    status_buf[pos] = VTK_VAL_FAR;
  }

  fprintf(stderr,"vtkLevelSetFastMarching::Init2D() 4 \n");

  //  if ( GB_debug AlorsFait fprintf(stderr,"LevelSetFastMarching::Init() \t 2 \n");
    if ( (this->T->FindPoint(cx-myrad2,cy-myrad2,0)<0) ||
         (this->T->FindPoint(cx+myrad2,cy+myrad2,0)<0) 
     ) {
      //      if ( GB_debug ) fprintf(stderr,"LevelSetFastMarching::Init() \t Error, out of image \n");
      return;
    } // end if
    
    //  if ( GB_debug ) fprintf(stderr,"LevelSetFastMarching::Init() \t 3 \n");

  // Initialization of mhPos image
  surf = new float[tx*ty*tz];

    
  // initializing mhPos to 0
  surf_buf = surf;
  for(pos=0;pos<tx*ty*tz;pos++) surf_buf[pos] = 0.0;

  // Distance function around the surface
  for (x=cx-myrad2; x<=cx+myrad2; x++) {
      for(y=cy-myrad2; y<=cy+myrad2; y++) {

          val =  sqrt((double)((x-cx)*(x-cx)+
                               (y-cy)*(y-cy))) - 
              (radius-margin);


          surf_buf  = surf+x+y*tx;
          *surf_buf = val;
          
          if ( val<0 ) {
              T_buf1  = (float*) this->T->GetScalarPointer(x,y,0);
              *T_buf1 = val;
          } // end if

      }
  }


  //----- Compute the time T around the surface
  //
  // Make evolving the surface forward and backward ??
  t   = 0;
  dt = 0.01;

  for (n=1; n<=1000; n++) {

      //      if ( GB_debug ) fprintf(stderr,"LevelSetFastMarching::Init() \t 7 \n");
      for (y=cy-myrad2; y<=cy+myrad2; y++) {
          for(x=cx-myrad2; x<=cx+myrad2; x++) {

              pos = x+y*tx;
              surf_buf  = surf + pos;
              val = *surf_buf;
              
              NGx = 0;
              NGx = macro_max(NGx, val- *(surf_buf+1));
              NGx = macro_max(NGx, val- *(surf_buf-1));
              NGy = 0;
              NGy = macro_max(NGy, val- *(surf_buf+tx));
              NGy = macro_max(NGy, val- *(surf_buf-tx));
              //        NG = macro_max(NGx,NGy);
              NG = sqrt(NGx*NGx+NGy*NGy);
              
              //    if (NG>0.01) printf("x %d y %d \n", x,y);
              
              force_buf1 = force_buf + pos;
              newval = val - dt*(*force_buf1)*NG;
              if ( (val>=0)&&(newval<0) ) {
                  
                  T_buf1 = (float*) this->T->GetScalarPointer(x,y,0);
                  if ( val-newval > EPSILON ) {
                      *T_buf1 = ((t-dt)*(-newval)+t*val)/(val-newval);
                  } else {
                      *T_buf1 = t-dt;
                  } // end if
              } // end if
              *surf_buf = newval;
              
          }
      }    
      t += dt;
  }


      //  if ( GB_debug ) fprintf(stderr,"LevelSetFastMarching::Init() \t 10 \n");

      //  surf->Sauve();
      //  this->T->Sauve();

  // 1. substract d to the time image
  T_buf1 =  T_buf;
  for(pos=0;pos<T->GetNumberOfPoints();pos++) 
    if (T_buf1[pos]<999) T_buf1[pos] = T_buf1[pos]-margin;

  // 2. Set the known, trial, and far away points

  for(y=cy-myrad2; y<=cy+myrad2; y++) {
      // fprintf(stderr,"y= %d \n",y);
      for(x=cx-myrad2; x<=cx+myrad2; x++) {

          T_buf1      = (float*)         this->T     ->GetScalarPointer(x,y,0);
          status_buf = this->status+x+y*tx;
          val = *T_buf1;
          
          if ( val<=0 ) {
              *status_buf = VTK_VAL_ACCEPTED;
          } else {
              if ( val<=margin ) {
                  *status_buf = VTK_VAL_TRIAL;
                  this->mh   += FM_TrialPoint(x,y,0,x+y*tx,val);
              } // end if
          } // end if
          
      }
  }    

    //this->T     ->Sauve("T-init.ami.gz");
    //this->status->Sauve("status-init.ami.gz");
  cout << mh << endl;

      //  delete surf;

    fprintf(stderr," vtkLevelSetFastMarching::Init2D() end \n");

} // vtkLevelSetFastMarching::Init2D()


//----------------------------------------------------------------------
//
void vtkLevelSetFastMarching::Init3D(int cx, int cy, int cz, int radius)
//                    -----
{

  //  Local
    int   d=2;
    int   x,y,z;
    int   myrad2;
    float t,dt,NG;
    int   n;
    long pos;

    float*        surf;
    float         newval,val;
    float         NGx, NGy, NGz;

    float*         T_buf1;
    unsigned char* status_buf;
    float*         surf_buf;
    float*         force_buf1;

    fprintf(stderr,"Init3D() begin \n");

  myrad2 = radius+d+d;

  // Initialisation to 1000 and VAL_FAR
  T_buf1      = T_buf;

  status_buf = this->status;

  for(pos=0;pos<this->T->GetNumberOfPoints();pos++) {
    T_buf1[pos]      = maxTime;
    status_buf[pos] = VTK_VAL_FAR;
  }


  //  if ( GB_debug AlorsFait fprintf(stderr,"LevelSetFastMarching::Init() \t 2 \n");
    if ( (this->T->FindPoint(cx-myrad2,cy-myrad2,cz-myrad2)<0) ||
         (this->T->FindPoint(cx+myrad2,cy+myrad2,cz+myrad2)<0) 
     ) {
      //      if ( GB_debug )
      fprintf(stderr,"LevelSetFastMarching::Init() \t Error, out of image \n");
      return;
    } // end if
    

  // Initialization of mhPos image
  surf = new float[tx*ty*tz];

    
  // initializing mhPos to 0
  surf_buf = surf;
  for(pos=0;pos<tx*ty*tz;pos++) surf_buf[pos] = 0.0;

  // Distance function around the surface
  for(x=cx-myrad2; x <=cx+myrad2; x++) {
      for(y=cy-myrad2; y<=cy+myrad2; y++) {
          for(z=cz-myrad2; z<=cz+myrad2; z++) {

              val =  sqrt((double)((x-cx)*(x-cx)+
                                   (y-cy)*(y-cy)+
                                   (z-cz)*(z-cz))) - 
                  (radius-d);
              
              pos = x+y*tx+z*txy;
              surf_buf  = surf+pos;
              *surf_buf = val;
              
              if ( val<0 ) {
                  T_buf1  = T_buf+pos;
                  *T_buf1 = val;
              } // end if

          }
      }
  }


  //----- Compute the time T around the surface
  //
  // Make evolving the surface forward and backward ??
  t   = 0;
  dt = 0.01;

  for(n=1; n<=1000; n++) {


      for(z=cz-myrad2; z<=cz+myrad2; z++) {
          for(y=cy-myrad2; y<=cy+myrad2; y++) {
              for(x=cx-myrad2; x<=cx+myrad2; x++) {

                  pos = x+y*tx+z*txy;
                  surf_buf  = surf + pos;
                  val = *surf_buf;
                  
                  NGx = 0;
                  NGx = macro_max(NGx, val- *(surf_buf+1));
                  NGx = macro_max(NGx, val- *(surf_buf-1));
                  NGy = 0;
                  NGy = macro_max(NGy, val- *(surf_buf+tx));
                  NGy = macro_max(NGy, val- *(surf_buf-tx));
                  NGz = 0;
                  NGz = macro_max(NGz, val- *(surf_buf+txy));
                  NGz = macro_max(NGz, val- *(surf_buf-txy));
                  NG = sqrt(NGx*NGx+NGy*NGy+NGz*NGz);
                  
                  
                  //    if (NG>0.01) printf("x %d y %d \n", x,y);
                  
                  force_buf1 = force_buf + pos;
                  newval = val - dt*(*force_buf1)*NG;
        if ( (val>=0)&&(newval<0) ) {
            
            T_buf1 = T_buf + pos;
            if ( val-newval > EPSILON ) {
                *T_buf1 = ((t-dt)*(-newval)+t*val)/(val-newval);
            } else {
                *T_buf1 = t-dt;
            } // end if
        } // end if
        *surf_buf = newval;
        
              }
          }
      }    
      t += dt;
      
    }


  // 1. substract d to the time image
  T_buf1 =  T_buf;
  for(pos=0;pos<T->GetNumberOfPoints();pos++) 
    if (T_buf1[pos]<999) T_buf1[pos] = T_buf1[pos]-d;

  // 2. Set the known, trial, and far away points

  for(z=cz-myrad2; z<=cz+myrad2; z++) {
      for(y=cy-myrad2; y<=cy+myrad2; y++) {
          for(x=cx-myrad2; x<=cx+myrad2; x++) {

              pos = x+y*tx+z*txy;
              T_buf1     = T_buf+pos;
              status_buf = this->status+pos;
              val        = *T_buf1;
              
              if ( val<=0 ) {
                  *status_buf = VTK_VAL_ACCEPTED;
              } else {
                  if ( val<=d ) {
                      *status_buf = VTK_VAL_TRIAL;
                      this->mh   += FM_TrialPoint(x,y,z,pos,val);
                  } // end if
              } // end if
              
          }
      }
  }    
  

  cout << mh << endl;

  fprintf(stderr,"Init3D() end \n");

} // vtkLevelSetFastMarching::Init3D()


//----------------------------------------------------------------------
//
// Initialize the Fast Marching with a given image
//  
// Inputs parameters:
//   initimage   : the initial image
//   initmaxdist : the maximal distance, accept all the intensity
//                 value lower than this distance
//                 and add all the accepted values to the trial points
//
// Modified:
//    T_buf
//    status
//    trial points
//
//  We suppose the initial image positive
//
void vtkLevelSetFastMarching::InitWithImage()
//                    -------------
{

    int            x,y,z;
    unsigned char* status_buf;
    float*         init_buf;
    int            pos,p,i;
    int*           tab_pos;
    int            npos;

    //   fprintf(stderr, "InitWithImage() .");fflush(stderr);

  status_buf = this->status;
  init_buf   = (float*)this->initimage->GetScalarPointer();

  //  fprintf(stderr, ".");fflush(stderr);

  tab_pos = new int[imsize];
  npos = 0;
 
  // initialize status to FAR
  memset(status,VTK_VAL_FAR,imsize);

  // copy the initial image into the time
  memcpy(T_buf,init_buf,imsize*sizeof(float));

  if (narrowband==NULL) {
    for(pos=0;pos<imsize;pos++) 
      if (init_buf[pos]<initmaxdist) {
    status_buf[pos] = VTK_VAL_ACCEPTED;
    tab_pos[npos++] = pos;
      }
  }
  else {
    for(i=0;i<bandsize;i++) {
      pos = narrowband[i];
      if (init_buf[pos]<initmaxdist) {
    status_buf[pos] = VTK_VAL_ACCEPTED;
    tab_pos[npos++] = pos;
      }
    }
  }

  //  Set the  trial points
  //  fprintf(stderr, ".");fflush(stderr);

  for(i=0;i<npos;i++) {
    p = pos = tab_pos[i];
    x = p%tx;
    p = (p-x)/tx;
    y = p%ty;
    z = (p-y)/ty;
    AddTrialPoints(x,y,z,pos);
  }

  delete [] tab_pos;
  // fprintf(stderr, ";\n");

} // InitWithImage()


//----------------------------------------------------------------------
//
//  Initialize the Euclidian distance for the voxels which are neighbors
//  to the isosurface
//
//  Inputs
//    initimage
//    vs[0-2]           : voxel size
//    isotropic_voxels  : tell if we should consider the voxels isotropic of size 1x1x1
//    initiso           : threshold of the isosurface
// 
//  Outputs
//    T_buf   : pointer to the data of T (resulting time for the eikonal equation)
//
//
void vtkLevelSetFastMarching::InitIsoSurf()
//                    -----------
{

  register float          val0_new,val1_new;
  register int            pos;
  register int            displace[3];  // displacement for the 5 neighbors
  register int            sign, neigh_sign;
  register float          val0,val1,diff;
  unsigned char           grad_computed;
  register float          norm;
  register float          Grad[3];
  register int            x,y,z;
  register float*         init_buf;
  register float*         T_buf1;
  register unsigned char* status_buf;
  register int            n;
  register float          val;
  int                     zmin,zmax,nmax;

  // Put the result in the time image
  init_buf   = (float*)this->initimage->GetScalarPointer();

  for(pos=0;pos<imsize;pos++) {
    if (init_buf[pos]>initiso) 
      T_buf     [pos] = maxTime;
    else 
      T_buf     [pos] = -maxTime;
  }

  displace[0] = 1;
  displace[1] = tx;
  displace[2] = txy;


  // Compute the distance approximation close to the surface

  switch (dim) {
  case VTK_MODE_2D: zmin = zmax = 0;       nmax = 1; break;
  case VTK_MODE_3D: zmin = 0; zmax = tz-2; nmax = 2; break;
  }

  for(z=zmin; z<=zmax; z++) {
      for(y=0; y<=ty-2; y++) {

          init_buf   = (float*)this->initimage->GetScalarPointer(0,y,z);
          T_buf1     = T_buf + y*tx + z*txy;
          
          for(x=0; x<=tx-2; x++) {

              val0 =  *init_buf-initiso;
              sign = (val0>0); 
              
              grad_computed = 0;
              
              for(n=0; n<=nmax; n++) {
                  
                  val1 = *(init_buf+displace[n])-initiso;
                  neigh_sign =  (val1>0);

                  if (sign != neigh_sign) {

                      if (!grad_computed) {
                          // gradient estimation
                          if (isotropic_voxels) {
                              Grad[0] = *(init_buf+1)    - *init_buf;
                              Grad[1] = *(init_buf+tx)   - *init_buf;
                              if (dim==VTK_MODE_3D)
                                  Grad[2] = *(init_buf+txy)  - *init_buf;
                          } else {
                              Grad[0] = (*(init_buf+1)    - *init_buf)/vs[0];
                              Grad[1] = (*(init_buf+tx)   - *init_buf)/vs[1];
                              if (dim==VTK_MODE_3D)
                                  Grad[2] = (*(init_buf+txy)  - *init_buf)/vs[2];
                          }
                          
                          // Normalization
                          norm = Grad[0]*Grad[0]+Grad[1]*Grad[1];
                          if (dim==VTK_MODE_3D)
                              norm += Grad[2]*Grad[2];
                          norm = sqrt(norm);
                          
                          grad_computed = 1;
                          if (norm<EPSILON) 
                              fprintf(stderr, " %d %d %d norm=%f \n",x,y,z,norm);
                      }
                      
                      if (sign)
                          diff = val0-val1;
                      else
                          diff = val1-val0;
                      
                      if (diff<EPSILON) {
                          fprintf(stderr," %d %d %d %d  diff = %f : val0 %f val1 %f vs[n] %f diff<1-2 \n",x,y,z,n,diff,
                                  val0,val1,vs[n]);
                          continue;
                      }
                      
                      val = fabs(Grad[n])/norm/diff;
                      if (!isotropic_voxels)  val *= vs[n];
                      
                      if (norm>EPSILON) {
                          val0_new = val0*val;
                          val1_new = val1*val;
                      }
                      
                      if (fabs(val0_new)<fabs(*T_buf1)) 
                          *T_buf1 = val0_new;
                      
                      if (fabs(val1_new)<fabs(*(T_buf1+displace[n])))
                          *(T_buf1+displace[n]) = val1_new;
                      
                  }
              }
              
              init_buf++;
              T_buf1++;
              
          }
      }
  }

 
  // Now initialize with positive values and set the status image

  T_buf1     = T_buf;
  status_buf = this->status;

  fprintf(stderr, "InitIsoSurf() .");fflush(stderr);

  for(pos=0;pos<imsize;pos++) {
    if (fabs(T_buf[pos])>=maxTime) {
      T_buf1    [pos] = maxTime;
      status_buf[pos] = VTK_VAL_FAR;
    }
    else { 
      T_buf1    [pos] = fabs(T_buf[pos]);
      status_buf[pos] = VTK_VAL_ACCEPTED;
    }
  }

  //  Set the  trial points
  //  fprintf(stderr, ";\n");

  switch (dim) {
  case VTK_MODE_2D: zmin = zmax = 0;        break;
  case VTK_MODE_3D: zmin = 2; zmax = tz-3;  break;
  }

  for(z=zmin; z<=zmax; z++) {
      for(y=2; y<=ty-3; y++) {
          pos = z*txy+y*tx+2;
          for(x=2; x<=tx-3; x++) {
              if (fabs(T_buf1[pos])<maxTime) AddTrialPoints(x,y,z,pos);
              pos++;
          }
      }
  }    

  

} // InitIsoSurf()
