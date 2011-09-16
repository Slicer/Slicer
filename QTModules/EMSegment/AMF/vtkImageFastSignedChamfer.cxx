/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageFastSignedChamfer.cxx,v $
  Date:      $Date: 2006/01/06 17:57:54 $
  Version:   $Revision: 1.10 $

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


#include "vtkImageFastSignedChamfer.h"
#include "vtkObjectFactory.h"
//#include "vtkScalars.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkFloatArray.h"

#define TRUE    1
#define FALSE   0
#define EPSILON 1E-5


#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))

//
//---------------------------------------------------------------------------
vtkImageFastSignedChamfer* vtkImageFastSignedChamfer::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageFastSignedChamfer");
  if(ret)
    {
    return (vtkImageFastSignedChamfer*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageFastSignedChamfer;
}


//----------------------------------------------------------------------------
// Constructor sets default values
vtkImageFastSignedChamfer::vtkImageFastSignedChamfer()
{

  maxdist   = 10.0;

  // by default set the coefficients given in Borgefors96
  /*
  coeff_a = 0.92644;
  coeff_b = 1.34065;
  coeff_c = 1.65849;
  */
  //  coeff_b = sqrt(2);
  //  coeff_c = sqrt(3);

  inputImage           = NULL;
  inputImage_allocated = 0;

  input_output_array    = NULL;
  local_floatarray      = NULL;

  min_x = NULL;
  max_x = NULL;

  noborder = 0;

} // vtkImageFastSignedChamfer()


//---------------------------------------------------------------------------
// Destructor
vtkImageFastSignedChamfer::~vtkImageFastSignedChamfer()
//              ---------------
{
  if (inputImage_allocated) {
    inputImage->Delete();
    inputImage=NULL;
  }

  if (local_floatarray != NULL) {
    local_floatarray->Delete();
  } 

} // ~vtkImageFastSignedChamfer()


//---------------------------------------------------------------------------
// Init params and copy data to float
void vtkImageFastSignedChamfer::InitParam( vtkImageData* input, vtkImageData* output)
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
      local_floatarray = vtkFloatArray::New();
      local_floatarray->SetArray(input_output_array,imsize,1);
      outputImage->GetPointData()->SetScalars(local_floatarray);
    } 
    else {
      outputImage->AllocateScalars();
    }

    //    outputImage->CopyAndCastFrom(this->inputImage,
    //                                 this->inputImage->GetExtent());
  }
    
} //  InitParam()


//----------------------------------------------------------------------------
void vtkImageFastSignedChamfer::SetMinMaxX( int** minx, int** maxx)
{
  min_x = minx;
  max_x = maxx;
}


//----------------------------------------------------------------------------
// This method is passed  input and output data, and executes the filter
// algorithm to fill the output from the input.
void vtkImageFastSignedChamfer::ExecuteData(vtkDataObject *outData)
//                   -------
{

  InitParam( this->GetInput(), this->GetOutput());

  if (tz == 1) {
    coeff_a = 1.;
    //    coeff_b = 1.32;
    coeff_b = sqrt((double)2);
    FastSignedChamfer2D();
  }
  else {
    coeff_a = 1;
    //    coeff_b = sqrt(2);
    //  coeff_c = sqrt(3);
    coeff_b = 1.314;
    coeff_c = 1.628;
    FastSignedChamfer3D();
  }


} // ExecuteData()


//----------------------------------------------------------------------
void vtkImageFastSignedChamfer::FastSignedChamfer2D( )
{

  register int     x,y,i,n;
  register int     j;
  int              i1,i2;
  register int     neighbor[4];
  register int     neighbor1[2];
  register int     neighbor2[2];
  register float   min,val=0;
  register float*  buf;
  register float*  buf1;
  register float*  buf2;
  register int     imin,imax,jmin,jmax;

  i = 0;
  i1 = i2 = 0;

  for (y=-1; y<=1; y++) {
      for (x=-1; x<=1; x++) {
          if ( i>3 ) break;
          n = abs(x)+abs(y);
          neighbor[i]      = y*tx + x;
          switch ( n ) {
          case 1: 
              neighbor1[i1] = neighbor[i];
              i1++;
              break;
          case 2: 
              neighbor2[i2] = neighbor[i];
              i2++;
              break;
          default: fprintf(stderr,"Func_Chamfer2_2D() \t Chamfer error n = %d (%d,%d)\n",n,x,y);
          }
          i++;
      }
  }

  // The copy is already done in the InitParam() function
  outputImage->CopyAndCastFrom(this->inputImage,
                               this->inputImage->GetExtent());

  // First pass


  for(y=1;y<ty-1;y++) {
    buf = (float*) outputImage ->GetScalarPointer(1,y,0);
    for(x=1;x<tx-1;x++) {
      
      if (*buf>= maxdist) { buf++; continue; }
      if (*buf<=-maxdist) { buf++; continue; }

      if (*buf>-coeff_a) {
    // C4 neighbors
    val = *buf+coeff_a;
    buf1 = buf-neighbor1[0]; if (val < *buf1) *buf1 = val;
    buf1 = buf-neighbor1[1]; if (val < *buf1) *buf1 = val;
    
    // C8 neighbors
    val = *buf+coeff_b;
    buf1 = buf-neighbor2[0]; if (val < *buf1) *buf1 = val;
    buf1 = buf-neighbor2[1]; if (val < *buf1) *buf1 = val;
      }
      if (*buf<coeff_a) {
    // C4 neighbors
    val = *buf-coeff_a;
    buf1 = buf-neighbor1[0]; if (val > *buf1) *buf1 = val;
    buf1 = buf-neighbor1[1]; if (val > *buf1) *buf1 = val;
    
    // C8 neighbors
    val = *buf-coeff_b;
    buf1 = buf-neighbor2[0]; if (val > *buf1) *buf1 = val;
    buf1 = buf-neighbor2[1]; if (val > *buf1) *buf1 = val;
      }
    
      buf++;
    }  // end for x
  }  // end for y

  // Second pass

  for(y=ty-2;y>0;y--) {
    buf = (float*) outputImage ->GetScalarPointer(tx-2,y,0);
    for(x=tx-2;x>0;x--) {

      if (*buf>= maxdist) { buf--; continue; }
      if (*buf<=-maxdist) { buf--; continue; }

      if (*buf>-coeff_a) {
    // C4 neighbors
    val = *buf+coeff_a;
    buf1 = buf+neighbor1[0]; if (val < *buf1) *buf1 = val;
    buf1 = buf+neighbor1[1]; if (val < *buf1) *buf1 = val;
    
    // C8 neighbors
    val = *buf+coeff_b;
    buf1 = buf+neighbor2[0]; if (val < *buf1) *buf1 = val;
    buf1 = buf+neighbor2[1]; if (val < *buf1) *buf1 = val;
      }
      if (*buf<coeff_a) {
    // C4 neighbors
    val = *buf-coeff_a;
    buf1 = buf+neighbor1[0]; if (val > *buf1) *buf1 = val;
    buf1 = buf+neighbor1[1]; if (val > *buf1) *buf1 = val;
    
    // C8 neighbors
    val = *buf-coeff_b;
    buf1 = buf+neighbor2[0]; if (val > *buf1) *buf1 = val;
    buf1 = buf+neighbor2[1]; if (val > *buf1) *buf1 = val;
      }

      buf--;
    } // end for x
  } // end for y


  buf = (float*) outputImage ->GetScalarPointer();

  // Compute the borders
  for(y=0;y<=ty-1;y++)
    for(x=0;x<=tx-1;x++) {
      if ((x==0)||(x==tx-1)||
      (y==0)||(y==ty-1)){
    
    if (x==0) imin = 0; else imin = -1;
    if (y==0) jmin = 0; else jmin = -1;
    
    if (x==tx-1) imax = 0; else imax = 1;
    if (y==ty-1) jmax = 0; else jmax = 1;
          
    min = *buf;

    buf1 = buf + imin;
    for (i=imin; i<=imax; i++) {
        buf2 = buf1;
        if (jmin==-1) buf2 -= tx;
        for (j=jmin; j<=jmax; j++) {
            n = abs(i)+abs(j);
            if ( n==0 ) continue;
            switch ( n ) {
            case 1:  val = *buf2+coeff_a; break;
            case 2:  val = *buf2+coeff_b; break;
            default: 
                fprintf(stderr,
                        "Func_Chamfer2_2D() \t Chamfer error (%d,%d) \n",x,y);
            }
            if ( val<min ) min = val;
            buf2 += tx;
        }
        buf1++;
    }
    
        *buf = min;
      
      } // end if

      buf++;
    } // end for x,y

} // FastSignedChamfer2D()


//----------------------------------------------------------------------
void vtkImageFastSignedChamfer::FastSignedChamfer3DOld( )
{

  register int    x,y,z,i,n;
  register int    j,k;
  int             i1,i2,i3;
  register int    neighbor[13];
  register int    neighbor1[3];
  register int    neighbor2[6];
  register int    neighbor3[4];
  register float  min,val=0;
  register float* buf;
  register float* buf1;
  register float* buf2;
  register float* buf3;
  register int    imin,imax,jmin,jmax,kmin,kmax;

  //  fprintf(stderr,"FastSignedChamfer3D() \t initialization() \n");
  fprintf(stderr,"FastSignedChamfer3DOld() .");
  fflush(stderr);


  i = 0;
  i1 = i2 = i3 = 0;
  for (z=-1; z<= 1; z++) {
      for (y=-1; y<=1; y++) {
          for (x=-1; x<=1; x++) {
              if ( i>12 ) break;
              n = abs(x)+abs(y)+abs(z);
              neighbor[i]      = (z*ty+y)*tx + x;
              switch ( n ) {
              case 1: 
                  neighbor1[i1] = neighbor[i];
                  i1++;
                  break;
              case 2: 
                  neighbor2[i2] = neighbor[i];
                  i2++;
                  break;
              case 3: 
                  neighbor3[i3] = neighbor[i];
                  i3++;
                  break;
              default: fprintf(stderr,"FastSignedChamfer3DOld() \t Chamfer error \n");
              }
              i++;
          }
      }
  }

  // The copy is already done in the InitParam() function
  outputImage->CopyAndCastFrom(this->inputImage,
                               this->inputImage->GetExtent());

  // First pass

  fprintf(stderr, ".");fflush(stderr);
  //  fprintf(stderr,"FastSignedChamfer3D() \t first pass \n");

  for(z=1;z<tz-1;z++)
    for(y=1;y<ty-1;y++) {
      buf = (float*) outputImage ->GetScalarPointer(1,y,z);
      for(x=1;x<tx-1;x++) {

    if (*buf>= maxdist) { buf++; continue; }
    if (*buf<=-maxdist) { buf++; continue; }

    if (*buf>-coeff_a) {
      // C6 neighbors
      val = *buf+coeff_a;
      
      buf1 = buf-neighbor1[0]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor1[1]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor1[2]; if (val < *buf1) *buf1 = val;
      
      // C18 neighbors
      val = *buf+coeff_b;
      
      buf1 = buf-neighbor2[0]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor2[1]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor2[2]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor2[3]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor2[4]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor2[5]; if (val < *buf1) *buf1 = val;
      
      // C26 neighbors
      val = *buf+coeff_c;
      
      buf1 = buf-neighbor3[0]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor3[1]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor3[2]; if (val < *buf1) *buf1 = val;
      buf1 = buf-neighbor3[3]; if (val < *buf1) *buf1 = val;
    }
    if (*buf<coeff_a) {
      // C6 neighbors
      val = *buf-coeff_a;
      
      buf1 = buf-neighbor1[0]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor1[1]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor1[2]; if (val > *buf1) *buf1 = val;
      
      // C18 neighbors
      val = *buf-coeff_b;
      
      buf1 = buf-neighbor2[0]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor2[1]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor2[2]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor2[3]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor2[4]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor2[5]; if (val > *buf1) *buf1 = val;
      
      // C26 neighbors
      val = *buf-coeff_c;
      
      buf1 = buf-neighbor3[0]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor3[1]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor3[2]; if (val > *buf1) *buf1 = val;
      buf1 = buf-neighbor3[3]; if (val > *buf1) *buf1 = val;
    }
    buf++;
      }
    }

  // Second pass
  //  fprintf(stderr,"FastSignedChamfer3D() \t second pass \n");
  fprintf(stderr, ".");fflush(stderr);

  for(z=tz-2;z>0;z--)
    for(y=ty-2;y>0;y--) {
      buf = (float*) outputImage ->GetScalarPointer(tx-2,y,z);
      for(x=tx-2;x>0;x--) {

    if (*buf>= maxdist) { buf--; continue; }
    if (*buf<=-maxdist) { buf--; continue; }

        if (*buf>-coeff_a) {

      // C6 neighbors
      val = *buf+coeff_a;
      
      buf1 = buf+neighbor1[0]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor1[1]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor1[2]; if (val < *buf1) *buf1 = val;
      
      // C18 neighbors
      val = *buf+coeff_b;
      
      buf1 = buf+neighbor2[0]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor2[1]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor2[2]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor2[3]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor2[4]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor2[5]; if (val < *buf1) *buf1 = val;
      
      // C26 neighbors
      val = *buf+coeff_c;
      
      buf1 = buf+neighbor3[0]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor3[1]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor3[2]; if (val < *buf1) *buf1 = val;
      buf1 = buf+neighbor3[3]; if (val < *buf1) *buf1 = val;
    }
        if (*buf<coeff_a) {

      // C6 neighbors
      val = *buf-coeff_a;
      
      buf1 = buf+neighbor1[0]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor1[1]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor1[2]; if (val > *buf1) *buf1 = val;
      
      // C18 neighbors
      val = *buf-coeff_b;
      
      buf1 = buf+neighbor2[0]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor2[1]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor2[2]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor2[3]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor2[4]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor2[5]; if (val > *buf1) *buf1 = val;
      
      // C26 neighbors
      val = *buf-coeff_c;
      
      buf1 = buf+neighbor3[0]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor3[1]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor3[2]; if (val > *buf1) *buf1 = val;
      buf1 = buf+neighbor3[3]; if (val > *buf1) *buf1 = val;
    }

    buf--;
      }
    }

  //  fprintf(stderr,"FastSignedChamfer3D() \t borders \n");
  fprintf(stderr, ".");fflush(stderr);


  buf = (float*) outputImage ->GetScalarPointer();

  // Compute the borders: too slow, and not really correct ...
  // should compute 6 2D distances for each face ...
  for(z=0;z<=tz-1;z++) 
    for(y=0;y<=ty-1;y++)
      for(x=0;x<=tx-1;x++) {
    if ((x==0)||(x==tx-1)||
            (y==0)||(y==ty-1)||
            (z==0)||(z==tz-1)) {

      min =*buf;

      if (x==0) imin = 0; else imin = -1;
      if (y==0) jmin = 0; else jmin = -1;
      if (z==0) kmin = 0; else kmin = -1;

      if (x==tx-1) imax = 0; else imax = 1;
      if (y==ty-1) jmax = 0; else jmax = 1;
      if (z==tz-1) kmax = 0; else kmax = 1;

      buf1 = buf + imin;
      for(i=imin; i<=imax; i++) {
          buf2 = buf1;
          if (jmin==-1) buf2 -= tx;
          for(j=jmin; j<=jmax; j++) {
            buf3 = buf2;
            if (kmin==-1) buf3 -= txy;
            for (k=kmin; k<=kmax; k++) {
                n = abs(i)+abs(j)+abs(k);
                switch ( n ) {
                case 0: continue;
                case 1: val = *buf3+coeff_a; break;
                case 2: val = *buf3+coeff_b; break;
                case 3: val = *buf3+coeff_c; break;
                default: fprintf(stderr,"FastSignedChamfer3DOld() \tChamfer error \n");
                }
                if ( val<min ) min = val;
                buf3 += txy;
              }
            buf2 += tx;
          }
          buf1++;
      }


      *buf = min;
      buf++;
      
    }
        else

      // jump directly to the last voxel of the line
      if (x==1) {
        x   += tx-3;
        buf += tx-3;
        buf++;
      }
      else {
        fprintf(stderr,"FastSignedChamfer3DOld() \t  we should not get here !!! \n");

        buf++;

      } // end else (if not border voxel)

      } // end for x,y,z

  fprintf(stderr,";\n");

} // FastSignedChamfer3DOld()


//----------------------------------------------------------------------
void vtkImageFastSignedChamfer::FastSignedChamfer3D( )
{

  register int    x,y,z;
  register int    n10,n11,n12;
  register int    n20,n21,n22,n23,n24,n25;
  register int    n30,n31,n32,n33;
  register float  val;
  register float* buf0;
  register float* bufy;
  register float* buf;
  register float* buf1;

  // bounding box of uses points
  register int    xmin,xmax;
  register int    ymin,ymax;
  register int    zmin,zmax;
  register unsigned char y_used,z_used;
  register int    extent_inc;
  register float  maxdist1;

  //  fprintf(stderr,"FastSignedChamfer3D() \t initialization() \n");
  //  fprintf(stderr,"FSCh3D() .");
  //  fflush(stderr);


  // indices
  // nxy : x is the type of neighbor (1,2 or 3)
  //       y is the indice of this neighbor for its neighbor type

  n30 =  -txy-tx-1;
  n31 =  -txy-tx+1;
  n32 =  -txy+tx-1;
  n33 =  -txy+tx+1;

  n20 =  -txy-tx;
  n21 =  -txy-1;
  n22 =  -txy+1;
  n23 =  -txy+tx;
  n24 =  -tx-1;
  n25 =  -tx+1;

  n10 =  -txy;
  n11 =  -tx;
  n12 =  -1;

  // The copy is already done in the InitParam() function

  if (input_output_array==NULL) 
      memcpy(outputImage->GetScalarPointer(),
           inputImage->GetScalarPointer(),
           imsize*sizeof(float));
  //  outputImage->CopyAndCastFrom(this->inputImage,
  //                               this->inputImage->GetExtent());

  // First pass

  //  fprintf(stderr, ".");fflush(stderr);
  //  fprintf(stderr,"FastSignedChamfer3D() \t first pass \n");

  xmin = tx;
  xmax = 0;
  ymin = ty;
  ymax = 0;
  zmin = tz;
  zmax = 0;

  maxdist1 = maxdist-coeff_a;

  buf0 = (float*) outputImage ->GetScalarPointer();
  for(z=0;z<tz-1;z++) {
    z_used = 0;
    bufy = buf0 + z*txy + tx;
    for(y=1;y<ty-1;y++,bufy+=tx) {
      y_used = 0;
      buf = bufy+1;
      for(x=1;x<tx-1;x++) {

    if (*buf>= maxdist1) { buf++; continue; }
    if (*buf<=-maxdist1) { buf++; continue; }

    y_used = 1;
    if (x<xmin) xmin = x;
    if (x>xmax) xmax = x;

    // Update Min/Max ?


    if (*buf>-coeff_a) {
      // C6 neighbors
      val = *buf+coeff_a;
      
      buf1 = buf-n10; if (val < *buf1) *buf1 = val;
      buf1 = buf-n11; if (val < *buf1) *buf1 = val;
      buf1 = buf-n12; if (val < *buf1) *buf1 = val;
      
      // C18 neighbors
      val = *buf+coeff_b;
      
      buf1 = buf-n20; if (val < *buf1) *buf1 = val;
      buf1 = buf-n21; if (val < *buf1) *buf1 = val;
      buf1 = buf-n22; if (val < *buf1) *buf1 = val;
      buf1 = buf-n23; if (val < *buf1) *buf1 = val;
      buf1 = buf-n24; if (val < *buf1) *buf1 = val;
      buf1 = buf-n25; if (val < *buf1) *buf1 = val;
      
      // C26 neighbors
      val = *buf+coeff_c;
      
      buf1 = buf-n30; if (val < *buf1) *buf1 = val;
      buf1 = buf-n31; if (val < *buf1) *buf1 = val;
      buf1 = buf-n32; if (val < *buf1) *buf1 = val;
      buf1 = buf-n33; if (val < *buf1) *buf1 = val;
    }
    if (*buf<coeff_a) {
      // C6 neighbors
      val = *buf-coeff_a;
      
      buf1 = buf-n10; if (val > *buf1) *buf1 = val;
      buf1 = buf-n11; if (val > *buf1) *buf1 = val;
      buf1 = buf-n12; if (val > *buf1) *buf1 = val;
      
      // C18 neighbors
      val = *buf-coeff_b;
      
      buf1 = buf-n20; if (val > *buf1) *buf1 = val;
      buf1 = buf-n21; if (val > *buf1) *buf1 = val;
      buf1 = buf-n22; if (val > *buf1) *buf1 = val;
      buf1 = buf-n23; if (val > *buf1) *buf1 = val;
      buf1 = buf-n24; if (val > *buf1) *buf1 = val;
      buf1 = buf-n25; if (val > *buf1) *buf1 = val;
      
      // C26 neighbors
      val = *buf-coeff_c;
      
      buf1 = buf-n30; if (val > *buf1) *buf1 = val;
      buf1 = buf-n31; if (val > *buf1) *buf1 = val;
      buf1 = buf-n32; if (val > *buf1) *buf1 = val;
      buf1 = buf-n33; if (val > *buf1) *buf1 = val;
    }
    buf++;
      } // end for x
      if (y_used) {
    z_used = 1;
    if (y<ymin) ymin = y;
    if (y>ymax) ymax = y;
      }
    } // end for y
    if (z_used) {
      if (z<zmin) zmin = z;
      if (z>zmax) zmax = z;
    }
  } // end for z

  //  fprintf(stderr, "[%3d, %3d] x [%3d, %3d] x [%3d, %3d] \n",
  //  xmin,xmax,ymin,ymax,zmin,zmax);

  // Second pass
  //  fprintf(stderr,"FastSignedChamfer3D() \t second pass \n");
  //  fprintf(stderr, ".");fflush(stderr);

  if (ymax==ty-1) ymax = ty-2;
  if (xmax==tx-1) xmax = tx-2;

  extent_inc = (int) (maxdist/coeff_a+1.0);
  if (zmin-extent_inc>=0) zmin -= extent_inc; else zmin = 0;
  if (ymin-extent_inc>=0) ymin -= extent_inc; else ymin = 0;
  if (xmin-extent_inc>=0) xmin -= extent_inc; else xmin = 0;

  //  fprintf(stderr, "[%3d, %3d] x [%3d, %3d] x [%3d, %3d] \n",
  //      xmin,xmax,ymin,ymax,zmin,zmax);

  for(z=zmax;z>zmin;z--) {
    for(y=ymax;y>ymin;y--) {
      buf = (float*) outputImage ->GetScalarPointer(xmax,y,z);
      for(x=xmax;x>xmin;x--) { 

    if (*buf>= maxdist1) { buf--; continue; }
    if (*buf<=-maxdist1) { buf--; continue; }

    if (*buf>-coeff_a) {

      // C6 neighbors
      val = *buf+coeff_a;
      
      buf1 = buf+n10; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n11; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n12; if (val < *buf1)  *buf1 = val;
      
      // C18 neighbors
      val = *buf+coeff_b;
      
      buf1 = buf+n20; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n21; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n22; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n23; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n24; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n25; if (val < *buf1)  *buf1 = val;
      
      // C26 neighbors
      val = *buf+coeff_c;
      
      buf1 = buf+n30; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n31; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n32; if (val < *buf1)  *buf1 = val;
      buf1 = buf+n33; if (val < *buf1)  *buf1 = val;
    }
        if (*buf<coeff_a) {

      // C6 neighbors
      val = *buf-coeff_a;
      
      buf1 = buf+n10; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n11; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n12; if (val > *buf1)  *buf1 = val;
      
      // C18 neighbors
      val = *buf-coeff_b;
      
      buf1 = buf+n20; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n21; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n22; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n23; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n24; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n25; if (val > *buf1)  *buf1 = val;
      
      // C26 neighbors
      val = *buf-coeff_c;
      
      buf1 = buf+n30; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n31; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n32; if (val > *buf1)  *buf1 = val;
      buf1 = buf+n33; if (val > *buf1)  *buf1 = val;
    }

    buf--;
      } // end for x
    } // end for y
  } // end for z

  //  fprintf(stderr,"FastSignedChamfer3D() \t borders \n");
  if (!noborder) {
    //    fprintf(stderr, ".");fflush(stderr);
    FastSignedChamfer3DBorders();
  }
  //  fprintf(stderr,";\n");

  extent[0] = xmin;
  extent[1] = xmax;
  extent[2] = ymin;
  extent[3] = ymax;
  extent[4] = zmin;
  extent[5] = zmax;

} // FastSignedChamfer3D()


//----------------------------------------------------------------------
void vtkImageFastSignedChamfer::FastSignedChamfer3DBorders( )
{

  register int    x,y,z,i,n;
  register int    j,k;
  register float  min,val=0;
  register float* buf;
  register float* buf1;
  register float* buf2;
  register float* buf3;
  register int    imin,imax,jmin,jmax,kmin,kmax;

  fprintf(stderr,"FastSignedChamfer3DBorders() \n");

  buf = (float*) outputImage ->GetScalarPointer();

  // Compute the borders: too slow, and not really correct ...
  // should compute 6 2D distances for each face ...
  for(z=0;z<=tz-1;z++) 
    for(y=0;y<=ty-1;y++)
      for(x=0;x<=tx-1;x++) {
     if ((x==0)||(x==tx-1)||
            (y==0)||(y==ty-1)||
            (z==0)||(z==tz-1)) {

      min =*buf;

      if (x==0) imin = 0; else imin = -1;
      if (y==0) jmin = 0; else jmin = -1;
      if (z==0) kmin = 0; else kmin = -1;

      if (x==tx-1) imax = 0; else imax = 1;
      if (y==ty-1) jmax = 0; else jmax = 1;
      if (z==tz-1) kmax = 0; else kmax = 1;

      buf1 = buf + imin;
      for(i=imin; i <= imax; i++) {
          buf2 = buf1;
          if (jmin==-1) buf2 -= tx;
          for(j=jmin; j<=jmax; j++) {
              buf3 = buf2;
              if (kmin==-1) buf3 -= txy;
              for (k=kmin; k<=kmax; k++) {
                  n = abs(i)+abs(j)+abs(k);
                  switch ( n ) {
                  case 0: continue;
                  case 1: val = *buf3+coeff_a; break;
                  case 2: val = *buf3+coeff_b; break;
                  case 3: val = *buf3+coeff_c; break;
                  default: fprintf(stderr,"FastSignedChamfer3D() \tChamfer error \n");
                  }
                  if ( val<min ) min = val;
                  buf3 += txy;
              }
              buf2 += tx;
          }
          buf1++;
      }


      *buf = min;
      buf++;
      
    }
        else

      // jump directly to the last voxel of the line
      if (x==1) {
        x   += tx-3;
        buf += tx-3;
        buf++;
      }
      else {
        fprintf(stderr,"FastSignedChamfer3D() \t  we should not get here !!! \n");

        buf++;

      } // end else (if not border voxel)

      } // end for x,y,z

} // FastSignedChamfer3DBorders()


//----------------------------------------------------------------------
void vtkImageFastSignedChamfer::PrintSelf(ostream& os, vtkIndent indent)
{
   vtkImageToImageFilter::PrintSelf(os,indent);

} // PrintSelf()
