/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImagePropagateDist.cxx,v $
  Date:      $Date: 2006/01/13 16:04:04 $
  Version:   $Revision: 1.7 $

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


#include "vtkImagePropagateDist.h"
#include "vtkObjectFactory.h"
//#include "vtkScalars.h"
#include "vtkFloatArray.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkStructuredPointsWriter.h"

#define TRUE    1
#define FALSE   0
#define EPSILON 1E-5

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))


#define dist2(a,b,c)  (sq[abs(a)]+sq[abs(b)]+sq[abs(c)])
#define dist(a,b,c)  (distance[abs(a)][abs(b)][abs(c)])



//
//---------------------------------------------------------------------------
vtkImagePropagateDist* vtkImagePropagateDist::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImagePropagateDist");
  if(ret)
    {
    return (vtkImagePropagateDist*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImagePropagateDist;
}


//----------------------------------------------------------------------------
// Constructor sets default values
vtkImagePropagateDist::vtkImagePropagateDist()
{

  maxdist   = 10.0;
  mindist   = -10.0;

  // by default set the coefficients given in Borgefors96

  inputImage           = NULL;
  inputImage_allocated = 0;

  input_output_array    = NULL;

  min_x = NULL;
  max_x = NULL;

  noborder = 0;

  save_intermediate_images = 0;
  trial_loop = 0;

} // vtkImagePropagateDist()


//---------------------------------------------------------------------------
// Destructor
vtkImagePropagateDist::~vtkImagePropagateDist()
//              ---------------
{
  if (inputImage_allocated) {
    inputImage->Delete();
    inputImage=NULL;
  }

} // ~vtkImagePropagateDist()


//---------------------------------------------------------------------------
// Init params and copy data to float
void vtkImagePropagateDist::InitParam( vtkImageData* input, vtkImageData* output)
//                   ---------
{
  int type;


  inputImage = input;

  if (inputImage == NULL) {
    vtkErrorMacro("Missing input");
    return;
  }
  else {

    // check the image is in float format, or convert
    type = input->GetScalarType();
    if ((type != VTK_FLOAT)&&(input_output_array==NULL)) {
      vtkDebugMacro(<<"making a copy of the input into float format");
      // Create a copy of the data
      inputImage = vtkImageData::New();

      inputImage->SetScalarType( VTK_FLOAT);
      inputImage->SetNumberOfScalarComponents(1);
      inputImage->SetDimensions( input->GetDimensions());
      inputImage->SetOrigin(     input->GetOrigin());
      inputImage->SetSpacing(    input->GetSpacing());
      
      inputImage->CopyAndCastFrom(input,input->GetExtent());
      inputImage_allocated = 1;
    }
    else
      inputImage = input;

    tx = this->inputImage->GetDimensions()[0];
    ty = this->inputImage->GetDimensions()[1];
    tz = this->inputImage->GetDimensions()[2];
    txy = tx*ty;
    imsize = txy*tz;
    
    extent[0] = 0;
    extent[1] = tx-1;
    extent[2] = 0;
    extent[3] = ty-1;
    extent[4] = 0;
    extent[5] = tz-1;
                                                      
    //--- outputImage
    outputImage      = output;
    
    outputImage->SetDimensions(inputImage->GetDimensions() );
    outputImage->SetSpacing(   inputImage->GetSpacing() );
    outputImage->SetScalarType(VTK_FLOAT); 
    outputImage->SetNumberOfScalarComponents(1);

    if (input_output_array != NULL) {
      vtkFloatArray* da = vtkFloatArray::New();
      da->SetArray(input_output_array,imsize,1);
      outputImage->GetPointData()->SetScalars(da);
    } 
    else {
      outputImage->AllocateScalars();
    }

    //    outputImage->CopyAndCastFrom(this->inputImage,
    //                                 this->inputImage->GetExtent());
  }
    
} //  InitParam()


//----------------------------------------------------------------------------
void vtkImagePropagateDist::SetMinMaxX( int** minx, int** maxx)
{
  min_x = minx;
  max_x = maxx;
}


//----------------------------------------------------------------------------
// This method is passed  input and output data, and executes the filter
// algorithm to fill the output from the input.
void vtkImagePropagateDist::ExecuteData(vtkDataObject *outData)
//                   -------
{

  InitParam( this->GetInput(), this->GetOutput());

  if (tz == 1) {
    PropagateDanielsson2D();
  }
  else {
    PropagateDanielsson3D();
  }


} // ExecuteData()


//----------------------------------------------------------------------
void vtkImagePropagateDist::PropagateDanielsson2D( )
{

    // 0: know values in the front
    // 1: value to compute in the front  
    register short     dx,dy;
    register int       n[8];
    register int       nx[8];
    register int       ny[8];
    register int       l;
    register int       tp;
    register int       p,k,pn;

    int                x0,y0,x1,y1;
    int                xpn0,ypn0,tpn;
    int                dxp,dyp;
    int                dxpn,dypn;
    int                i,j;
    register float known_dist_pos;
    register float known_dist_neg;
    register float next_dist_pos;
    register float next_dist_neg;
    register float step_dist;

    int       iteration;
    float     val_min_pos,val_max_pos;
    float     val_min_neg,val_max_neg;
    register float     val;
    register float     val0;

    int posupdated;
    int negupdated;

    PD_element pt0;
    float* buf;
    float* bufn;
    int distmap_count = 0;


  PreComputeDistanceArray();
  InitLists();

  memcpy(outputImage->GetScalarPointer(),
     inputImage->GetScalarPointer(),
     imsize*sizeof(float));

  buf    = (float*) outputImage ->GetScalarPointer();

  //------- initialize neighbors
  l = 0;
  for(i=-1;i<=1;i+=1)
    for(j=-tx;j<=tx;j+=tx)
      {
    if ((i==0)&&(j==0)) continue;
        ((i<0)?nx[l]=-1:((i>0)?nx[l]=1:nx[l]=0));
        ((j<0)?ny[l]=-1:((j>0)?ny[l]=1:ny[l]=0));
    n[l]=i+j;
    l++;
      }


  if (mindist>0) mindist = 0;
  if (maxdist<0) maxdist = 0;

  step_dist  = 0.7;
  known_dist_pos = 0;
  known_dist_neg = 0;

  //-------- Create the first list: values in ]-1,1[
  for(p=0;p<imsize;p++)  {
    if (fabs(buf[p])<1) {
      list0[list0_size++] = p;
      list_elts[p].Init(0,0,0,POINT_SET_FRONT,p);
    }
    else 
      if (buf[p]>0)
    buf[p] = maxdist;
      else
    buf[p] = mindist;
  }

  iteration = 0;

  // Already done ...
  //  for(k=0;k<list0_size;k++) list_elts[list0[k]].SetState(POINT_SET_FRONT);
  known_dist_pos = 0;
  known_dist_neg = 0;

  printf("list0_size = %5d \n",list0_size);

  SaveDistance(      distmap_count);
  SaveState(         distmap_count);
  SaveTrajectories2D(distmap_count++);

  list_remaining_trial_size = 0;

  while ((known_dist_pos<maxdist)||(known_dist_neg>mindist)) {

    val_min_pos = maxdist; 
    val_min_neg = 1;
    val_max_pos = -1;
    val_max_neg = mindist;

    iteration++;
    printf("known dist [%3.3f,%3.3f] \n",known_dist_neg,known_dist_pos);
    if (known_dist_pos<maxdist) next_dist_pos = known_dist_pos+step_dist;
    if (known_dist_neg>mindist) next_dist_neg = known_dist_neg-step_dist;

    //------ Create the trial points
    list1_size = 0;

    // Put the remaining trial points in the list
    for(k=0;k<list_remaining_trial_size;k++) {
      p = list_remaining_trial[k];
      list1[list1_size++]=p;
      list_elts[p].SetState(POINT_TRIAL_INLIST);
    }
    list_remaining_trial_size = 0;

    for(k=0;k<list0_size;k++) {
      p    = list0[k];
      pt0  = list_elts[p];
      tp   = pt0.GetTrack();
      val0 = buf[tp];

      if (buf[p]>0) {
    val_min_pos = min(val_min_pos,buf[p]);
    val_max_pos = max(val_max_pos,buf[p]);
      } else {
    val_min_neg = min(val_min_neg,buf[p]);
    val_max_neg = max(val_max_neg,buf[p]);
      }


      x0 = p%tx;
      y0 = ((p-x0)/tx)%ty;

      dxp = pt0.X();
      dyp = pt0.Y();

      // Check the neighbors for trial
      for(l=0;l<8;l++) {

    // checking the scalar product
    // I should check the equivalence ...
        if ((dxp*nx[l]+dyp*ny[l])<0) continue;

    pn = p+n[l];
        x1 = x0+nx[l];
        y1 = y0+ny[l];

        // Check the limits
        if ((x1>=0)&&(x1<tx)&&(y1>=0)&&(y1<ty))
      {

      PD_element& neighbor = list_elts[pn];

          switch (neighbor.GetState()) {
      case POINT_NOT_PARSED:
            neighbor.SetState(POINT_TRIAL);
      case POINT_TRIAL:
        list1[list1_size++]=pn;
            neighbor.SetState(POINT_TRIAL_INLIST);
      case POINT_TRIAL_INLIST:
        // Update here the values of the trial points
        dx = pt0.X()+nx[l];
        dy = pt0.Y()+ny[l];
        bufn = buf+pn;
        // Positive side
        if (*bufn>0) {
          val = val0 + dist(dx,dy,0);
          if (val<*bufn) {
        if (val<known_dist_pos) {
          printf("val<known_dist_pos; val0=%f val=%f (dx,dy)=(%d,%d)\n",val0, val,dx,dy);
          printf("point (%3d,%3d) \n",x1,y1);
        }
        *bufn = val;
        neighbor.SetPosTrack( dx,dy,0, tp);
                neighbor.SetPrevNeighbor(l);
          }
        } else
        // Negative side
        if (*bufn<0) {
          val = val0 - dist(dx,dy,0);
          if (val>*bufn) {
        if (val>known_dist_neg) {
          printf("val>known_dist_neg; val0=%f val=%f (dx,dy)=(%d,%d)\n",val0,val,dx,dy);
          printf("point (%3d,%3d) \n",x1,y1);
        }
        *bufn = val;
        neighbor.SetPosTrack( dx,dy,0, tp);
                neighbor.SetPrevNeighbor(l);
          }
        } // end if
        else
          fprintf(stderr," *bufn == 0 ! \n");
        break;
        
      case POINT_SET_FRONT:
      case POINT_SET:
        // check for skeleton ...
        if ((buf[p]<0)&&
        (buf[pn]<0)&&
        (!pt0.GetSkeleton())&&
        (!neighbor.GetSkeleton())) 
          {
        tpn = neighbor.GetTrack();
        xpn0 = tpn%tx;
        ypn0 = ((tpn-xpn0)/tx)%ty;
        dxpn = x1-xpn0;
        dypn = y1-ypn0;
        if ((dxp*dxpn+dyp*dypn)<0) {
          if (buf[p]<buf[pn])
            list_elts[p].SetSkeleton(1);
          else
            list_elts[pn].SetSkeleton(1);
        }
          } 

      } // end switch

    } // if pn in [0,size-1]
      } // for l in [0,7]
    } // for k in [0,list0_size-1]


    SaveDistance(      distmap_count);
    SaveState(         distmap_count);
    SaveTrajectories2D(distmap_count++);

    printf("pos [%2.2f, %2.2f]  neg [%2.2f, %2.2f] \n",
      val_min_pos,val_max_pos,
      val_min_neg,val_max_neg
      );

    for(k=0;k<list0_size;k++)  list_elts[list0[k]].SetState( POINT_SET);

    // First iteration, we complete list0 ...
    if (iteration>1) list0_size = 0;

    //----- iterate inside trial points
    if (trial_loop) {
      posupdated = 0;
      negupdated = 0;
      for(k=0;k<list1_size;k++) {
    p = list1[k];
    // update the value using the known points
    pt0  = list_elts[p];
    tp   = pt0.GetTrack();
    if (tp<0) {
      //    fprintf(stderr," negative track ? \n");
      continue;
    }
    val0 = buf[tp];
    
    x0 = p%tx;
    y0 = ((p-x0)/tx)%ty;
    
    // Check the neighbors for trial
    for(l=0;l<8;l++) {
      pn = p+n[l];
      x1 = x0+nx[l];
      y1 = y0+ny[l];
      
      // Check the limits
      if ((x1>=0)&&(x1<tx)&&(y1>=0)&&(y1<ty))
        {
        
          PD_element& neighbor = list_elts[pn];
          
          // Update here the values of the trial points
          if ((neighbor.GetState()==POINT_TRIAL_INLIST)) {
        dx = pt0.X()+nx[l];
        dy = pt0.Y()+ny[l];
        bufn = buf+pn;
        // Positive side
        if (*bufn>0) {
          val = val0 + dist(dx,dy,0);
          if ((val<*bufn)&&(val<next_dist_pos)) {
            printf(" point (%3d,%3d), (nx,ny)= (%3d,%3d); diff = %1.7f; val = %2.4f \n",
               x1,y1,
           nx[l]+nx[int(pt0.GetPrevNeighbor())],
           ny[l]+ny[int(pt0.GetPrevNeighbor())],
               *bufn-val,
               val);
            posupdated++;
            *bufn = val;
            neighbor.SetPosTrack( dx,dy,0, tp);
            neighbor.SetPrevNeighbor(l);
          }
        } else {
          // Negative side
          val = val0 - dist(dx,dy,0);
          if ((val>*bufn)&&(val>next_dist_neg)) {
            printf(" point (%3d,%3d), (nx,ny)= (%3d,%3d); diff = %1.7f; val = %2.4f \n",
               x1,y1,
           nx[l]+nx[int(pt0.GetPrevNeighbor())],
           ny[l]+ny[int(pt0.GetPrevNeighbor())],
               *bufn-val,
               val);
            negupdated++;
            *bufn = val;
            neighbor.SetPosTrack( dx,dy,0, tp);
            neighbor.SetPrevNeighbor(l);
          }
        } // end if
          } // POINT_TRIAL
        } // pn in [0,size-1]
    } // for l in [0,8]

      } // end for k in [0,list1_size-1]

      printf("Number of updated points: pos = %d; neg = %d \n",
         posupdated,negupdated);
    } // if trial_loop


    known_dist_pos = next_dist_pos;
    known_dist_neg = next_dist_neg;

    //    val_min = maxdist;
    list_remaining_trial_size = 0;
    for(k=0;k<list1_size;k++) {
      p = list1[k];
      // update the list of known points in the front
      if (((buf[p]>0)&&(buf[p]<=known_dist_pos)&&(known_dist_pos<maxdist))||
          ((buf[p]<0)&&(buf[p]>=known_dist_neg)&&(known_dist_neg>mindist))) {
    list0[list0_size++] = p;
    list_elts[p].SetState( POINT_SET_FRONT);
    //    if (fabs(buf[p])<val_min)
    //      val_min = fabs(buf[p]);
      }
      else 
    if (((buf[p]>0)&&(known_dist_pos<maxdist))||
        ((buf[p]<0)&&(known_dist_neg>mindist)))
      {
        list_remaining_trial[list_remaining_trial_size++] = p;
        list_elts[p].SetState( POINT_TRIAL);
      }
    }

    SaveDistance(      distmap_count);
    SaveState(         distmap_count);
    SaveTrajectories2D(distmap_count++);

    //    printf("list0_size = %5d  min = %f, borne max = %f \n",
    //       list0_size, val_min, known_dist_pos+step_dist);

    

  } // end while
  /*
  save_intermediate_images = 1;
  SaveProjection(0);
  SaveSkeleton(0);
  */

  FreeLists();
  FreeDistanceArray();

} // PropagateDanielsson2D()


//----------------------------------------------------------------------
void vtkImagePropagateDist::PreComputeDistanceArray()
{
  int i;
  int x,y,z;
  float maxd;

  maxd = (maxdist>-mindist?maxdist:-mindist);

  //------ Precomputes distances
  sq_size = (int) (maxd+2); 
  printf("sq size = %d \n",sq_size);
  sq = new int[sq_size];
  for(i=0;i<sq_size;i++) sq[i]=i*i;

  sqroot_size = 3*sq_size*sq_size;
  sqroot = new double[sqroot_size];
  printf("sqroot size = %d \n",sqroot_size);
  for(i=0;i<sqroot_size;i++)  sqroot[i] = sqrt((double)i);

  distance = new double**[sq_size];
  for(x=0;x<sq_size;x++) distance[x] = new double*[sq_size];
  for(x=0;x<sq_size;x++)
    for(y=0;y<sq_size;y++) distance[x][y] = new double[sq_size];

  for(x=0;x<sq_size;x++)
    for(y=0;y<sq_size;y++) 
      for(z=0;z<sq_size;z++) 
    distance[x][y][z] = sqroot[sq[x]+sq[y]+sq[z]];

}

//----------------------------------------------------------------------
void vtkImagePropagateDist::FreeDistanceArray()
{
  int x,y;

  delete [] sq;
  delete [] sqroot;

  for(x=0;x<sq_size;x++)
    for(y=0;y<sq_size;y++) delete [] distance[x][y];
  for(x=0;x<sq_size;x++) delete [] distance[x];
  delete [] distance;

} // FreeDistanceArray()

//----------------------------------------------------------------------
void vtkImagePropagateDist::InitLists()
{

  //------ Allocate the information

  list_maxsize = 500000;

  list0 = new int[list_maxsize]; // list of points in the front
  list1 = new int[list_maxsize]; // list of trial points
  list_remaining_trial = new int[list_maxsize]; // list of trial points

  list_elts = new PD_element[imsize];

  list0_size = 0;
  list1_size = 0;
  list_remaining_trial_size = 0;

} // InitLists()


//----------------------------------------------------------------------
void vtkImagePropagateDist::FreeLists()
{

  //------ Allocate the information

  delete [] list0;
  delete [] list1;

  delete [] list_elts;

} // FreeLists()


//----------------------------------------------------------------------
void vtkImagePropagateDist::PropagateDanielsson3D( )
{

    // 0: know values in the front
    // 1: value to compute in the front  
    register short     dx,dy,dz;
    register int       n[26];
    register int       nx[26];
    register int       ny[26];
    register int       nz[26];
    register int       l;
    register int       tp;
    register int       p,k,pn;

    int                x0,y0,z0,x1,y1,z1,p0;
    int                dxp,dyp,dzp;
    int                dxpn,dypn,dzpn;
    int                i,j;
    register float known_dist_pos;
    register float known_dist_neg;
    register float next_dist_pos;
    register float next_dist_neg;
    register float step_dist;

    int       iteration;
    float     val_min_pos,val_max_pos;
    float     val_min_neg,val_max_neg;
    register float     val;
    register float     val0;

    int posupdated;
    int negupdated;

    PD_element pt0;
    float* buf;
    float* bufn;
    int distmap_count = 0;


  PreComputeDistanceArray();
  InitLists();

  memcpy(outputImage->GetScalarPointer(),
     inputImage->GetScalarPointer(),
     imsize*sizeof(float));

  buf    = (float*) outputImage ->GetScalarPointer();

  //------- initialize neighbors
  l = 0;
  for(i=-1;i<=1;i+=1)
    for(j=-tx;j<=tx;j+=tx)
      for(k=-txy;k<=txy;k+=txy) {
    if ((i==0)&&(j==0)&&(k==0)) continue;
        ((i<0)?nx[l]=-1:((i>0)?nx[l]=1:nx[l]=0));
        ((j<0)?ny[l]=-1:((j>0)?ny[l]=1:ny[l]=0));
        ((k<0)?nz[l]=-1:((k>0)?nz[l]=1:nz[l]=0));
    n[l]=i+j+k;
    l++;
      }

  if (mindist>0) mindist = 0;
  if (maxdist<0) maxdist = 0;

  step_dist  = 0.6;
  known_dist_pos = 0;
  known_dist_neg = 0;

  //-------- Create the first list: values in ]-1,1[
  for(p=0;p<imsize;p++)  {
    if (fabs(buf[p])<1) {
      list0[list0_size++] = p;
      list_elts[p].Init(0,0,0,POINT_SET_FRONT,p);
    }
    else 
      if (buf[p]>0)
    buf[p] = maxdist;
      else
    buf[p] = mindist;
  }

  iteration = 0;

  // Already done ...
  //  for(k=0;k<list0_size;k++) list_elts[list0[k]].SetState(POINT_SET_FRONT);
  known_dist_pos = 0;
  known_dist_neg = 0;

  SaveDistance(      distmap_count);
  SaveState(         distmap_count);
  SaveTrajectories3D(distmap_count++);

  list_remaining_trial_size = 0;

  while ((known_dist_pos<maxdist)||(known_dist_neg>mindist)) {

    val_min_pos = maxdist; 
    val_min_neg = 1;
    val_max_pos = -1;
    val_max_neg = mindist;

    iteration++;
    printf("known dist [%3.3f,%3.3f] \n",known_dist_neg,known_dist_pos);
    if (known_dist_pos<maxdist) next_dist_pos = known_dist_pos+step_dist;
    if (known_dist_neg>mindist) next_dist_neg = known_dist_neg-step_dist;

    //------ Create the trial points
    list1_size = 0;

    // Put the remaining trial points in the list
    for(k=0;k<list_remaining_trial_size;k++) {
      p = list_remaining_trial[k];
      list1[list1_size++]=p;
      list_elts[p].SetState(POINT_TRIAL_INLIST);
    }
    list_remaining_trial_size = 0;

    for(k=0;k<list0_size;k++) {
      p    = list0[k];
      pt0  = list_elts[p];
      tp   = pt0.GetTrack();
      val0 = buf[tp];

      if (buf[p]>0) {
    val_min_pos = min(val_min_pos,buf[p]);
    val_max_pos = max(val_max_pos,buf[p]);
      } else {
    val_min_neg = min(val_min_neg,buf[p]);
    val_max_neg = max(val_max_neg,buf[p]);
      }

      x0 = p%tx;
      p0 = (p-x0)/tx;
      y0 = p0%ty;
      z0 = (p0-y0)/ty;

      dxp = pt0.X();
      dyp = pt0.Y();
      dzp = pt0.Z();

      // Check the neighbors for trial
      for(l=0;l<26;l++) {

        if ((dxp*nx[l]+dyp*ny[l]+dzp*nz[l])<0) continue;

    pn = p+n[l];
    x1 = x0+nx[l];
    y1 = y0+ny[l];
    z1 = z0+nz[l];

        // Check the limits
        if ((x1>=0)&&(x1<tx)&&
            (y1>=0)&&(y1<ty)&&
            (z1>=0)&&(z1<tz))
      {

        PD_element& neighbor = list_elts[pn];

        if (neighbor.GetTrack()==tp) continue;


          switch (neighbor.GetState()) {
      case POINT_NOT_PARSED:
        neighbor.SetState(POINT_TRIAL);
      case POINT_TRIAL:
        list1[list1_size++]=pn;
            neighbor.SetState(POINT_TRIAL_INLIST);
      case POINT_TRIAL_INLIST:
        dx = pt0.X()+nx[l];
        dy = pt0.Y()+ny[l];
        dz = pt0.Z()+nz[l];
        bufn = buf+pn;
        // Positive side
        if (*bufn>0) {
          val = val0 + dist(dx,dy,dz);
          if (val<*bufn) {
        if (val<known_dist_pos) {
          printf("val<known_dist_pos; val0=%f val=%f (dx,dy,dz)=(%d,%d,%d)\n",val0, val,dx,dy,dz);
          printf("point (%3d,%3d,%3d) \n",x1,y1,z1);
        }
        *bufn = val;
        neighbor.SetPosTrack( dx,dy,dz, tp);
                neighbor.SetPrevNeighbor(l);
          }
        } else
        // Negative side
        if (*bufn<0) {
          val = val0 - dist(dx,dy,dz);
          if (val>*bufn) {
        if (val>known_dist_neg) {
          printf("val>known_dist_neg; val0=%f val=%f (dx,dy,dz)=(%d,%d,%d)\n",val0,val,dx,dy,dz);
          printf("point (%3d,%3d,%3d) \n",x1,y1,z1);
        }
        *bufn = val;
        neighbor.SetPosTrack( dx,dy,dz, tp);
                neighbor.SetPrevNeighbor(l);
          }
        } // end if
        else
          fprintf(stderr," *bufn == 0 ! \n");
        break;
        
      case POINT_SET_FRONT:
      case POINT_SET:
        // check for skeleton ...
        if ((buf[p]<0)&&
        (buf[pn]<0)&&
        (!pt0.GetSkeleton())&&
        (!neighbor.GetSkeleton())) 
          {

        dxpn = neighbor.X();
        dypn = neighbor.Y();
        dzpn = neighbor.Z();
        if ((dxp*dxpn+dyp*dypn+dzp*dzpn)<0) {
          if (buf[p]<buf[pn])
            list_elts[p].SetSkeleton(1);
          else
            list_elts[pn].SetSkeleton(1);
        }
          } 

      } // end switch

    } // if pn in [0,size-1]
      } // for l in [0,25]
    } // for k in [0,list0_size-1]


    SaveDistance(      distmap_count);
    SaveState(         distmap_count);
    SaveTrajectories3D(distmap_count++);

    printf("pos [%2.2f, %2.2f]  neg [%2.2f, %2.2f] \n",
      val_min_pos,val_max_pos,
      val_min_neg,val_max_neg
      );

    for(k=0;k<list0_size;k++)  list_elts[list0[k]].SetState( POINT_SET);

    // First iteration, we complete list0 ...
    if (iteration>1) list0_size = 0;


    //----- iterate inside trial points
    if (trial_loop) {
      posupdated = 0;
      negupdated = 0;
      for(k=0;k<list1_size;k++) {
    p = list1[k];
    // update the value using the known points
    pt0  = list_elts[p];
    tp   = pt0.GetTrack();
    if (tp<0) {
      //    fprintf(stderr," negative track ? \n");
      continue;
    }
    val0 = buf[tp];
    
    x0 = p%tx;
    p0 = (p-x0)/tx;
    y0 = p0%ty;
    z0 = (p0-y0)/ty;

    // Check the neighbors for trial
    for(l=0;l<26;l++) {
      pn = p+n[l];
      x1 = x0+nx[l];
      y1 = y0+ny[l];
      z1 = z0+nz[l];

      // Check the limits
      if ((x1>=0)&&(x1<tx)&&
          (y1>=0)&&(y1<ty)&&
          (z1>=0)&&(z1<tz))
      {

        PD_element& neighbor = list_elts[pn];
        
        // Update here the values of the trial points
        if ((neighbor.GetState()==POINT_TRIAL)) {
          dx = pt0.X()+nx[l];
          dy = pt0.Y()+ny[l];
          dz = pt0.Z()+nz[l];
          bufn = buf+pn;
          // Positive side
          if (*bufn>0) {
        val = val0 + dist(dx,dy,dz);
        if ((val<*bufn)&&(val<next_dist_pos)) {
          printf(" point (%3d,%3d,%3d), (nx,ny,nz)= (%3d,%3d,%3d); diff = %1.7f; val = %2.4f \n",
             x1,y1,z1,
         nx[l]+nx[int(pt0.GetPrevNeighbor())],
         ny[l]+ny[int(pt0.GetPrevNeighbor())],
         nz[l]+nz[int(pt0.GetPrevNeighbor())],
             *bufn-val,
             val);
          posupdated++;
          *bufn = val;
          neighbor.SetPosTrack( dx,dy,dz, tp);
          neighbor.SetPrevNeighbor(l);
        }
          } else {
        // Negative side
        val = val0 - dist(dx,dy,dz);
        if ((val>*bufn)&&(val>next_dist_neg)) {
          printf(" point (%3d,%3d,%3d), (nx,ny,nz)= (%3d,%3d,%3d); diff = %1.7f; val = %2.4f \n",
             x1,y1,z1,
         nx[l]+nx[int(pt0.GetPrevNeighbor())],
         ny[l]+ny[int(pt0.GetPrevNeighbor())],
         nz[l]+nz[int(pt0.GetPrevNeighbor())],
             *bufn-val,
             val);
          negupdated++;
          *bufn = val;
          neighbor.SetPosTrack( dx,dy,dz, tp);
          neighbor.SetPrevNeighbor(l);
        }
          } // end if
        } // POINT_TRIAL
      } // pn in [0,size-1]
    } // for l in [0,25]
    
      } // end for k in [0,list1_size-1]

      printf("Number of updated points: pos = %d; neg = %d \n",
         posupdated,negupdated);

    } // if trial_loop

    known_dist_pos = next_dist_pos;
    known_dist_neg = next_dist_neg;

    //    val_min = maxdist;
    list_remaining_trial_size = 0;
    for(k=0;k<list1_size;k++) {
      p = list1[k];
      // update the list of known points in the front
      if (((buf[p]>0)&&(buf[p]<=known_dist_pos)&&(known_dist_pos<maxdist))||
          ((buf[p]<0)&&(buf[p]>=known_dist_neg)&&(known_dist_neg>mindist))) {
    list0[list0_size++] = p;
    list_elts[p].SetState( POINT_SET_FRONT);
    //if (fabs(buf[p])<val_min)
    //  val_min = fabs(buf[p]);
      }
      else 
    if (((buf[p]>0)&&(known_dist_pos<maxdist))||
        ((buf[p]<0)&&(known_dist_neg>mindist)))
    {
      list_remaining_trial[list_remaining_trial_size++] = p;
      list_elts[p].SetState( POINT_TRIAL);
    }
    }

    SaveDistance(      distmap_count);
    SaveState(         distmap_count);
    SaveTrajectories3D(distmap_count++);

    //    printf("list0_size = %5d  min = %f, borne max = %f \n",
    //       list0_size, val_min, known_dist+step_dist);


  } // end while

  save_intermediate_images = 1;
  //  SaveProjection(0);
  //  SaveSkeleton(0);

  FreeLists();
  FreeDistanceArray();

} // PropagateDanielsson3D()



//----------------------------------------------------------------------
void vtkImagePropagateDist::SaveTrajectories2D( int num)
{

  if (!save_intermediate_images) return;  

  register int       l;
  register int       nx[8];
  register int       ny[8];
  
  
  vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
  vtkImageData* copyImage = vtkImageData::New();
  float* ptr;
  char name[255];
  int  i,j;
  
  // Initialize neighbors
  l = 0;
  for(i=-1;i<=1;i+=1)
    for(j=-tx;j<=tx;j+=tx)
      {
    if ((i==0)&&(j==0)) continue;
    ((i<0)?nx[l]=-1:((i>0)?nx[l]=1:nx[l]=0));
    ((j<0)?ny[l]=-1:((j>0)?ny[l]=1:ny[l]=0));
    l++;
      }
  
  copyImage->SetScalarType( VTK_FLOAT);
  copyImage->SetNumberOfScalarComponents(1);
  copyImage->SetDimensions( outputImage->GetDimensions());
  copyImage->SetOrigin(     outputImage->GetOrigin());
  copyImage->SetSpacing(    outputImage->GetSpacing());
  
  copyImage->AllocateScalars();
  //     copyImage->CopyAndCastFrom(outputImage,
  //                outputImage->GetExtent());

  // Save X component  
  ptr = (float*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    if (list_elts[i].GetPrevNeighbor()!=-1)
      *ptr = nx[int(list_elts[i].GetPrevNeighbor())];
    else
      *ptr = 0;
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"trajectory%d_X.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  // Save Y component  
  ptr = (float*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    if (list_elts[i].GetPrevNeighbor()!=-1)
      *ptr = ny[int(list_elts[i].GetPrevNeighbor())];
    else
      *ptr = 0;
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"trajectory%d_Y.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();


  copyImage->Delete();
  writer->Delete();

  fprintf(stderr,"%s saved \n",name);

} // SaveTrajectories2D()


//----------------------------------------------------------------------
void vtkImagePropagateDist::SaveTrajectories3D( int num)
{

  if (!save_intermediate_images) return;  

  register int       l;
  register int       nx[26];
  register int       ny[26];
  register int       nz[26];
  
  
  vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
  vtkImageData* copyImage = vtkImageData::New();
  float* ptr;
  char name[255];
  int  i,j,k;
  
  // Initialize neighbors
  l = 0;
  for(i=-1;i<=1;i+=1)
    for(j=-tx;j<=tx;j+=tx)
      for(k=-txy;k<=txy;k+=txy) {
    if ((i==0)&&(j==0)&&(k==0)) continue;
        ((i<0)?nx[l]=-1:((i>0)?nx[l]=1:nx[l]=0));
        ((j<0)?ny[l]=-1:((j>0)?ny[l]=1:ny[l]=0));
        ((k<0)?nz[l]=-1:((k>0)?nz[l]=1:nz[l]=0));
    l++;
      }
  
  copyImage->SetScalarType( VTK_FLOAT);
  copyImage->SetNumberOfScalarComponents(1);
  copyImage->SetDimensions( outputImage->GetDimensions());
  copyImage->SetOrigin(     outputImage->GetOrigin());
  copyImage->SetSpacing(    outputImage->GetSpacing());
  
  copyImage->AllocateScalars();
  //     copyImage->CopyAndCastFrom(outputImage,
  //                outputImage->GetExtent());

  // Save X component  
  ptr = (float*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    if (list_elts[i].GetPrevNeighbor()!=-1)
      *ptr = nx[int(list_elts[i].GetPrevNeighbor())];
    else
      *ptr = 0;
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"trajectory%d_X.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  // Save Y component  
  ptr = (float*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    if (list_elts[i].GetPrevNeighbor()!=-1)
      *ptr = ny[int(list_elts[i].GetPrevNeighbor())];
    else
      *ptr = 0;
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"trajectory%d_Y.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  // Save Z component  
  ptr = (float*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    if (list_elts[i].GetPrevNeighbor()!=-1)
      *ptr = nz[int(list_elts[i].GetPrevNeighbor())];
    else
      *ptr = 0;
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"trajectory%d_Z.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();


  copyImage->Delete();
  writer->Delete();

  fprintf(stderr,"%s saved \n",name);

} // SaveTrajectories3D()


//----------------------------------------------------------------------
void vtkImagePropagateDist::SaveProjection( int num)
{

  if (!save_intermediate_images) return;  

  
  vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
  vtkImageData* copyImageX = vtkImageData::New();
  vtkImageData* copyImageY = vtkImageData::New();
  vtkImageData* copyImageZ = vtkImageData::New();
  float* ptrX;
  float* ptrY;
  float* ptrZ;
  char name[255];
  int  i,track;
  int  x,y,z,p;
  int  x0,y0,z0,p0;
  
  
  copyImageX->SetScalarType( VTK_FLOAT);
  copyImageX->SetNumberOfScalarComponents(1);
  copyImageX->SetDimensions( outputImage->GetDimensions());
  copyImageX->SetOrigin(     outputImage->GetOrigin());
  copyImageX->SetSpacing(    outputImage->GetSpacing());
  copyImageX->AllocateScalars();

  copyImageY->SetScalarType( VTK_FLOAT);
  copyImageY->SetNumberOfScalarComponents(1);
  copyImageY->SetDimensions( outputImage->GetDimensions());
  copyImageY->SetOrigin(     outputImage->GetOrigin());
  copyImageY->SetSpacing(    outputImage->GetSpacing());
  copyImageY->AllocateScalars();

  copyImageZ->SetScalarType( VTK_FLOAT);
  copyImageZ->SetNumberOfScalarComponents(1);
  copyImageZ->SetDimensions( outputImage->GetDimensions());
  copyImageZ->SetOrigin(     outputImage->GetOrigin());
  copyImageZ->SetSpacing(    outputImage->GetSpacing());
  copyImageZ->AllocateScalars();


  ptrX = (float*) copyImageX->GetScalarPointer();
  ptrY = (float*) copyImageY->GetScalarPointer();
  ptrZ = (float*) copyImageZ->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    track = list_elts[i].GetTrack();
    if (track!=-1) {
      p = i;
      x = p%tx;
      p = (p-x)/tx;
      y = p%ty;
      z = (p-y)/ty;

      p0 = track;
      x0 = p0%tx;
      p0 = (p0-x0)/tx;
      y0 = p0%ty;
      z0 = (p0-y0)/ty;

      *ptrX = x0-x;
      *ptrY = y0-y;
      *ptrZ = z0-z;
    }
    else
      *ptrX = *ptrY = *ptrZ = 0;
    
    ptrX++;
    ptrY++;
    ptrZ++;
  }
  
  writer->SetInput(copyImageX);
  sprintf(name,"projection%d_X.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  writer->SetInput(copyImageY);
  sprintf(name,"projection%d_Y.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  writer->SetInput(copyImageZ);
  sprintf(name,"projection%d_Z.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  copyImageX->Delete();
  copyImageY->Delete();
  copyImageZ->Delete();
  writer->Delete();

} // SaveProjectionDir()


//----------------------------------------------------------------------
void vtkImagePropagateDist::SaveState( int num)
{

  if (!save_intermediate_images) return;  

  vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
  vtkImageData* copyImage = vtkImageData::New();
  unsigned char* ptr;
  char name[255];
  int  i;
  
  copyImage->SetScalarType( VTK_UNSIGNED_CHAR);
  copyImage->SetNumberOfScalarComponents(1);
  copyImage->SetDimensions( outputImage->GetDimensions());
  copyImage->SetOrigin(     outputImage->GetOrigin());
  copyImage->SetSpacing(    outputImage->GetSpacing());
  
  copyImage->AllocateScalars();
  //     copyImage->CopyAndCastFrom(outputImage,
  //                outputImage->GetExtent());

  // Save state
  ptr = (unsigned char*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    *ptr = list_elts[i].GetState();
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"State%d.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  copyImage->Delete();
  writer->Delete();


} // SaveState()


//----------------------------------------------------------------------
void vtkImagePropagateDist::SaveSkeleton( int num)
{

  if (!save_intermediate_images) return;  

  
  vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
  vtkImageData* copyImage = vtkImageData::New();
  unsigned char* ptr;
  char name[255];
  int  i;
  
  
  copyImage->SetScalarType( VTK_UNSIGNED_CHAR);
  copyImage->SetNumberOfScalarComponents(1);
  copyImage->SetDimensions( outputImage->GetDimensions());
  copyImage->SetOrigin(     outputImage->GetOrigin());
  copyImage->SetSpacing(    outputImage->GetSpacing());
  
  copyImage->AllocateScalars();
  //     copyImage->CopyAndCastFrom(outputImage,
  //                outputImage->GetExtent());

  // Save skeleton
  ptr = (unsigned char*) copyImage->GetScalarPointer();
  for(i=0;i<imsize;i++) {
    *ptr = list_elts[i].GetSkeleton();
    ptr++;
  }
  
  writer->SetInput(copyImage);
  sprintf(name,"Skeleton%d.vtk",num);
  writer->SetFileName(name);
  writer->SetFileTypeToBinary();
  writer->Write();

  fprintf(stderr,"%s saved \n",name);

  copyImage->Delete();
  writer->Delete();


} // SaveSkeleton()


//----------------------------------------------------------------------
void vtkImagePropagateDist::SaveDistance( int num)
{

  if (!save_intermediate_images) return;  

    vtkStructuredPointsWriter *writer = vtkStructuredPointsWriter::New();
    vtkImageData* copyImage = vtkImageData::New();
    float* ptr;
    char name[255];
    int  i;
    float* buf;

    buf    = (float*) outputImage ->GetScalarPointer();
    
    copyImage->SetScalarType( VTK_FLOAT);
    copyImage->SetNumberOfScalarComponents(1);
    copyImage->SetDimensions( outputImage->GetDimensions());
    copyImage->SetOrigin(     outputImage->GetOrigin());
    copyImage->SetSpacing(    outputImage->GetSpacing());
    
    copyImage->AllocateScalars();
    //     copyImage->CopyAndCastFrom(outputImage,
    //                outputImage->GetExtent());
    
    ptr = (float*) copyImage->GetScalarPointer();
    for(i=0;i<imsize;i++) {
      *ptr = buf[i];
      ptr++;
    }
    
    writer->SetInput(copyImage);
    sprintf(name,"distmap%d.vtk",num);
    writer->SetFileName(name);
    writer->SetFileTypeToBinary();
    writer->Write();

    copyImage->Delete();
    writer->Delete();


    fprintf(stderr,"%s saved \n",name);

} // SaveDistance()



//----------------------------------------------------------------------
void vtkImagePropagateDist::PrintSelf(ostream& os, vtkIndent indent)
{
   vtkImageToImageFilter::PrintSelf(os,indent);

} // PrintSelf()
