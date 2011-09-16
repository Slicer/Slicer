/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageIsoContourDist.cxx,v $
  Date:      $Date: 2006/01/13 16:04:03 $
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


#include "vtkImageIsoContourDist.h"
#include "vtkObjectFactory.h"
//#include "vtkScalars.h"
#include "vtkFloatArray.h"
#include "vtkMultiThreader.h"
#include "vtkPointData.h"
#include "vtkImageData.h"
#include "vtkStructuredPointsWriter.h"

#define TRUE    1
#define FALSE   0
#define EPSILON 1E-5

#define min(x,y) (((x)<(y))?(x):(y))
#define max(x,y) (((x)>(y))?(x):(y))

#ifdef _SOLARIS_
/* Must be a Sun machine */
#include <thread.h>
#endif

//
//---------------------------------------------------------------------------
vtkImageIsoContourDist* vtkImageIsoContourDist::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageIsoContourDist");
  if(ret)
    {
    return (vtkImageIsoContourDist*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageIsoContourDist;
}


//----------------------------------------------------------------------------
// Constructor sets default values
vtkImageIsoContourDist::vtkImageIsoContourDist()
{

  threshold   = 0.0;
  farvalue    = 10;

  inputImage           = NULL;
  inputImage_allocated = 0;

  //  modify_input = FALSE;

  narrowband = NULL;
  bandsize   = 0;

  min_x = NULL;
  max_x = NULL;

  output_array    = NULL;

  float_array = NULL;
  float_array_allocated = 0;

} // vtkImageIsoContourDist()


//---------------------------------------------------------------------------
// Destructor
vtkImageIsoContourDist::~vtkImageIsoContourDist()
//              ---------------
{
  if (inputImage_allocated) {
    inputImage->Delete();
    inputImage=NULL;
  }

  if (float_array_allocated) {
    float_array->Delete();
    float_array = NULL;
    float_array_allocated = 0;
  }

} // ~vtkImageIsoContourDist()


//---------------------------------------------------------------------------
// Init params and copy data to float
void vtkImageIsoContourDist::InitParam( )
//                   ---------
{
  int type;

  inputImage = this->GetInput();

  if (inputImage == NULL) {
    vtkErrorMacro("Missing input");
    return;
  }
  else {

    // check the image is in float format, or convert
    type = inputImage->GetScalarType();
    if (type != VTK_FLOAT) {
      vtkDebugMacro(<<"making a copy of the input into float format");
      // Create a copy of the data
      inputImage = vtkImageData::New();

      inputImage->SetScalarType( VTK_FLOAT);
      inputImage->SetNumberOfScalarComponents(1);
      inputImage->SetDimensions( this->GetInput()->GetDimensions());
      inputImage->SetOrigin(     this->GetInput()->GetOrigin());
      inputImage->SetSpacing(    this->GetInput()->GetSpacing());
      
      inputImage->CopyAndCastFrom(this->GetInput(),
                  this->GetInput()->GetExtent());
      inputImage_allocated = 1;
    }
    else {
      inputImage = this->GetInput();
      inputImage_allocated = 0;
    }
    //    fprintf(stderr,"vtkImageIsoContourDist \t inputImage allocated %d \n",
    //        inputImage_allocated);

    tx = this->inputImage->GetDimensions()[0];
    ty = this->inputImage->GetDimensions()[1];
    tz = this->inputImage->GetDimensions()[2];
    txy = tx*ty;
    imsize = txy*tz;
    
    //    if (!modify_input) {
    //--- outputImage
    outputImage      = this->GetOutput();
      
    outputImage->SetDimensions(inputImage->GetDimensions() );
    outputImage->SetSpacing(   inputImage->GetSpacing() );
    outputImage->SetScalarType(VTK_FLOAT); 
    outputImage->SetNumberOfScalarComponents(1);
     
    
    if (output_array != NULL) {
      if (float_array_allocated) {
    float_array->Delete();
    float_array = NULL;
    float_array_allocated = 0;
      }
      float_array = vtkFloatArray::New();
      float_array_allocated = 1;
      float_array->SetArray(output_array,imsize,1);
      outputImage->GetPointData()->SetScalars(float_array);
    } 
    else {
      outputImage->AllocateScalars();
    }
    
    if (output_array == NULL) {
      outputImage->CopyAndCastFrom(this->inputImage,
                   this->inputImage->GetExtent());
    }
  }
    
} //  InitParam()


//----------------------------------------------------------------------------
void vtkImageIsoContourDist::SetNarrowBand( int* band, int size)
{
  narrowband = band;
  bandsize = size;
}


//----------------------------------------------------------------------------
void vtkImageIsoContourDist::SetMinMaxX( int** minx, int** maxx)
{
  min_x = minx;
  max_x = maxx;
}


//----------------------------------------------------------------------------
// This method is passed  input and output data, and executes the filter
// algorithm to fill the output from the input.
void vtkImageIsoContourDist::ExecuteData(vtkDataObject *outData)
//                   -------
{


  InitParam( );

  if (tz == 1) {
    IsoSurfDist2D();
  }
  else {
    if (narrowband==NULL)
      IsoSurfDist3D();
    else
      IsoSurfDist3D_band();
  }


} // Execute()


//----------------------------------------------------------------------
void vtkImageIsoContourDist::IsoSurfDist2D( )
{

  register int          x,y;
  register int          n;
  //  register int          i;
  unsigned long         i;
  register int          sign, neigh_sign;
  register float        val,val0,val1,diff;
  register float        val0_new,val1_new;
  register int          displace[2];  
  register float        Grad[2];
  register vtkFloatingPointType        vs[3];
  register float        norm;
  unsigned char         grad_computed;
  register float*       inPtr;
  register float*       outPtr;

  this->inputImage->GetSpacing(vs);

  inPtr  = (float*) inputImage ->GetScalarPointer();
  outPtr = (float*) outputImage->GetScalarPointer();
  for (i=0; i<imsize; i++) {
    if (*inPtr >= threshold) *outPtr =  farvalue;
    else                     *outPtr = -farvalue;
    inPtr++;
    outPtr++;
  }

  displace[0] = 1;
  displace[1] = tx;

  for(y=0; y<=ty-2; y++) {
  inPtr  = (float*) inputImage ->GetScalarPointer(0,y,0);
  for(x=0; x<=tx-2; x++) {

    val0 = *inPtr-threshold;
    sign = (val0>0); 
    grad_computed = FALSE;

    for(n=0;n<=1;n++) {

      val1 = *(inPtr+displace[n])-threshold;
      neigh_sign =  (val1>0);

      if (sign != neigh_sign) {

    if (!grad_computed) {
      // gradient estimation
      Grad[0] = (*(inPtr+1)  - *inPtr)/vs[0];
      Grad[1] = (*(inPtr+tx) - *inPtr)/vs[1];
      
      // Normalization
      norm = sqrt(Grad[0]*Grad[0]+Grad[1]*Grad[1]);
      grad_computed = TRUE;
      if (norm<EPSILON) 
        fprintf(stderr, " %d %d %d norm=%f \n",x,y,0,norm);
    }

    if (sign)
      diff = val0-val1;
    else
      diff = val1-val0;

        if (diff<EPSILON) {
      fprintf(stderr," %d %d %d %d  %f diff< %f \n",x,y,0,n,diff, 
          EPSILON);
      continue;
    }

    if (norm>EPSILON) {
          val = fabs(Grad[n])*vs[n]/norm/diff;
      val0_new = val0*val;
      val1_new = val1*val;

      if (fabs(val0_new)*vs[n]>1) {
        fprintf(stderr," %d %d %d %d  val0=%f val1=%f val0_new=%f \n",
            x,y,0,n,val0,val1,val0_new);
      }

      if (fabs(val1_new)*vs[n]>1) {
        fprintf(stderr," %d %d %d %d  val0=%f val1=%f val1_new=%f \n",
            x,y,0,n,val0,val1,val1_new);
      }

      outPtr  = (float*) outputImage ->GetScalarPointer(x,y,0);
      if (fabs(val0_new)<fabs(*outPtr)) {
        *outPtr = val0_new;
      } // end if
      
      outPtr  +=  displace[n];
      if (fabs(val1_new)<fabs(*outPtr)) {
        *outPtr  = val1_new;
      } // end if

    } else
      fprintf(stderr,"Func_IsoSurfDist2D() \t norm<=EPSILON \n");

      } // end if (sign != sign_neigh)
    } // end for n

    inPtr++;

  } // end for x
  } // end for y


} // IsoSurfDist2D()


//----------------------------------------------------------------------
void vtkImageIsoContourDist::IsoSurfDist3D( )
{

  register int          x,y,z;
  register int          n;
  register int          sign, neigh_sign;
  register float        val,val0,val1,diff;
  register float        val0_new,val1_new;
  register int          displace[3];  
  register float        Grad0[3];
  register float        Grad1[3];
  register float        Grad[3];
  register vtkFloatingPointType        vs[3];
  register float        vs0_2;
  register float        vs1_2;
  register float        vs2_2;
  register float        alpha_0;
  register float        alpha_1;
  register float        norm;
  unsigned char         grad0_computed;
  register float*       inPtr;
  register float*       inPtr1;
  register float*       outPtr;

  //  fprintf(stderr,"IsoSurf3D() begin \n");

  this->inputImage->GetSpacing(vs);

  vs0_2 = 2*vs[0];
  vs1_2 = 2*vs[1];
  vs2_2 = 2*vs[2];

  //  fprintf(stderr,"IsoSurf3D() initfar \n");

  IsoSurfDistInit( );

  displace[0] = 1;
  displace[1] = tx;
  displace[2] = txy;

  //  fprintf(stderr,"IsoSurf3D() compute \n");

  for(z=1; z<=tz-2; z++) {
  for(y=1; y<=ty-2; y++) {
  inPtr  = (float*) inputImage ->GetScalarPointer(1,y,z);
  for(x=1; x<=tx-2; x++) {

    val0 = *inPtr-threshold;
    sign = (val0>0); 
    grad0_computed = FALSE;

    for(n=0;n<=2;n++) {

      inPtr1 = inPtr+displace[n];
      val1 = *inPtr1-threshold;
      neigh_sign =  (val1>0);

      if (sign != neigh_sign) {

    if (!grad0_computed) {
      // gradient estimation
      Grad0[0] = (inPtr[1]   - inPtr[-1 ] ); // /vs[0];
      Grad0[1] = (inPtr[tx]  - inPtr[-tx] ); // /vs[1];
      Grad0[2] = (inPtr[txy] - inPtr[-txy]); // /vs[2];
      grad0_computed = TRUE;
    }
      
    Grad1[0] = (inPtr1[1]   - inPtr1[-1  ] );
    Grad1[1] = (inPtr1[tx]  - inPtr1[-tx ] );
    Grad1[2] = (inPtr1[txy] - inPtr1[-txy] );


    if (sign)
      diff = val0-val1;
    else
      diff = val1-val0;

        if (diff<EPSILON) {
      fprintf(stderr," %d %d %d %d  %f diff< %f \n",x,y,z,n,diff,
          EPSILON);
      continue;
    }

    // Interpolated gradient
    alpha_1 = 0.5; //val1/(val1-val0);
    alpha_0 = 0.5; //1-alpha_0;

    Grad[0] = (Grad0[0]*alpha_0+Grad1[0]*alpha_1)/vs0_2;
    Grad[1] = (Grad0[1]*alpha_0+Grad1[1]*alpha_1)/vs1_2;
    Grad[2] = (Grad0[2]*alpha_0+Grad1[2]*alpha_1)/vs2_2;

    // Normalization
    norm = sqrt(Grad[0]*Grad[0]+Grad[1]*Grad[1]+Grad[2]*Grad[2]);
    if (norm<EPSILON)
      fprintf(stderr, " %d %d %d norm=%f \n",x,y,z,norm);


    if (norm>EPSILON) {
          val = fabs(Grad[n])*vs[n]/norm/diff;

      val0_new = val0*val;
      val1_new = val1*val;

      /*
      if (fabsf(val0_new)>1+EPSILON) {
        fprintf(stderr," %d %d %d %d  val0=%f val1=%f val0_new=%f \n",x,y,z,n,val0,val1,val0_new);
      }
      
      if (fabsf(val1_new)>1+EPSILON) {
        fprintf(stderr," %d %d %d %d  val0=%f val1=%f val1_new=%f \n",x,y,z,n,val0,val1,val1_new);
      }
      */
      
      outPtr  = (float*) outputImage ->GetScalarPointer(x,y,z);
      if (fabs(val0_new)<fabs(*outPtr)) {
        *outPtr = val0_new;
      } // end if
      
      outPtr  +=  displace[n];
      if (fabs(val1_new)<fabs(*outPtr)) {
        *outPtr  = val1_new;
      } // end if
    } else
      fprintf(stderr,"Func_IsoSurfDist3D() \t norm<=EPSILON \n");

      } // end if  (sign != sign_neigh)
    } // end for n

    inPtr++;

  } // end for x
  } // end for y
  } // end for z

  //  fprintf(stderr,"IsoSurf3D() end \n");

} // IsoSurfDist3D()


//----------------------------------------------------------------------
void vtkImageIsoContourDist::IsoSurfDistInit( )
{

  register float*   inPtr;
  register float*   outPtr;
  register int      i,p;
  unsigned long     counter;

  inPtr  = (float*) inputImage ->GetScalarPointer();
  outPtr = (float*) outputImage->GetScalarPointer();

  if (narrowband==NULL) {
    for (counter=0; counter<imsize; counter++) {
      if (*inPtr > threshold)      
        *outPtr = +farvalue;
      else
      if (*inPtr < threshold)      
        *outPtr = -farvalue;
      else                   
        *outPtr = 0;
      inPtr++;
      outPtr++;
    }
  } else {
    // We only need to initialize inside the narrow band ...

    //    memcpy(outPtr,inPtr,imsize*sizeof(float));

    for(i=0;i<bandsize;i++) {
      p = narrowband[i];
      if (inPtr[p] > threshold)      
        outPtr[p] = +farvalue;
      else
      if (inPtr[p] < threshold)      
        outPtr[p] = -farvalue;
      else                   
        outPtr[p] = 0;
    }

  }
} // IsoSurfDistInit( )


//---------------------------------------------------------------------------
VTK_THREAD_RETURN_TYPE vtkImageIsoContourDist_ThreadedBand3D( void *arg )
{
    vtkImageIsoContourDist* This;
    int             first,last, total;
    int             threadId;
    int             threadCount;

  threadId    = ((ThreadInfoStruct *)(arg))->ThreadID;
  threadCount = ((ThreadInfoStruct *)(arg))->NumberOfThreads;

  This = (vtkImageIsoContourDist *)(((ThreadInfoStruct *)(arg))->UserData);

  total = This->SplitBand(first, last, threadId, threadCount);

  if (threadId < total) This->IsoSurfDist3D_band(first,last);
  
  return VTK_THREAD_RETURN_VALUE;

} // vtkImageIsoContourDist_ThreadedBand3D()


//----------------------------------------------------------------------
void vtkImageIsoContourDist::IsoSurfDist3D_band( )
{

  //  fprintf(stderr,"IsoSurfDist3D_band() threads %d \n",this->GetNumberOfThreads());

  IsoSurfDistInit( );

  if (this->GetNumberOfThreads()<=1)
    IsoSurfDist3D_band(0,this->bandsize-1);
  else {
  
#ifdef _SOLARIS_
    int code;
    fprintf(stderr,"thr_setconurrency(%d) \n",this->GetNumberOfThreads());
    code = thr_setconcurrency(this->GetNumberOfThreads());
#endif

    vtkMultiThreader* threader = vtkMultiThreader::New();

    // Threaded execution
    threader->SetNumberOfThreads(this->GetNumberOfThreads());

    // setup threading and the invoke threadedExecute
    threader->SetSingleMethod(vtkImageIsoContourDist_ThreadedBand3D, this);
    threader->SingleMethodExecute();
  }

  //  fprintf(stderr,"; "); fflush(stderr);

} // IsoSurfDist3D_band()


//----------------------------------------------------------------------------
// For threads.  
// Splits Narrow Band output into num pieces.
// This method returns the number of pieces resulting from a successful split.
//
// total is the total number of threads
//
int vtkImageIsoContourDist::SplitBand(int& first, int& last, int num, int total)
//                          ---------
{
  // determine the actual number of pieces that will be generated
  int valuesPerThread = (int)ceil(this->bandsize/(double)total);
  int maxThreadIdUsed = (int)ceil(this->bandsize/(double)valuesPerThread) - 1;

  first =  num*valuesPerThread;
  if (num < maxThreadIdUsed)
    last = first + valuesPerThread - 1;
  else
    last = this->bandsize-1;
  
  return maxThreadIdUsed + 1;
}


//----------------------------------------------------------------------
void vtkImageIsoContourDist::IsoSurfDist3D_band( int first_band, int last_band)
{

  register int          x,y,z,p,p1;
  register int          i,n;
  register int          sign, neigh_sign;
  register float        val,val0,val1,diff;
  register float        val0_new,val1_new;
  register int          displace[3];  
  register float        Grad[3];
  register vtkFloatingPointType        vs[3];
  register float        norm;
  unsigned char         grad_computed;
  register float*       inPtr0;
  register float*       outPtr0;
  register float*       inPtr;
  register float*       outPtr;



  this->inputImage->GetSpacing(vs);

  displace[0] = 1;
  displace[1] = tx;
  displace[2] = txy;

  inPtr0   = (float*) inputImage  ->GetScalarPointer();
  outPtr0  = (float*) outputImage ->GetScalarPointer();

  for(i=first_band;i<=last_band;i++) {

    p = narrowband[i];
    inPtr  = inPtr0+p;

    // get coordinates if we need then ...
    if (min_x!=NULL) {
      x = p%tx;
      p1 = (p-x)/tx;
      y = p1%ty;
      z = (p1-y)/ty;
    }
    // The narrow band doesn't contain border points ...

    val0 = *inPtr-threshold;
    sign = (val0>0); 
    grad_computed = FALSE;

    for(n=0;n<=2;n++) {

      val1 = *(inPtr+displace[n])-threshold;
      neigh_sign =  (val1>0);

      if (sign != neigh_sign) {

    if (!grad_computed) {
      // gradient estimation
      Grad[0] = (*(inPtr+1)  - *inPtr)/vs[0];
      Grad[1] = (*(inPtr+tx) - *inPtr)/vs[1];
      Grad[2] = (*(inPtr+txy)- *inPtr)/vs[2];
      
      // Normalization
      norm = sqrt(Grad[0]*Grad[0]+Grad[1]*Grad[1]+Grad[2]*Grad[2]);
      grad_computed = TRUE;
      if (norm<EPSILON)
        fprintf(stderr, "p=%d norm=%f \n",p,norm);
    }

    if (sign)
      diff = val0-val1;
    else
      diff = val1-val0;

        if (diff<EPSILON) {
      fprintf(stderr,"p=%d  n=%d  diff=%f diff< %f \n",p,n,diff,
          EPSILON);
      continue;
    }

    if (norm>EPSILON) {
          val = fabs(Grad[n])*vs[n]/norm/diff;

      val0_new = val0*val;
      val1_new = val1*val;

      outPtr  = outPtr0+p;
      if (fabs(val0_new)<fabs(*outPtr)) {
        *outPtr = val0_new;
        if (min_x!=NULL) {
          min_x[y][z] = min(min_x[y][z],x);
          max_x[y][z] = max(max_x[y][z],x);
        }
      } // end if
      
      outPtr  +=  displace[n];
      if (fabs(val1_new)<fabs(*outPtr)) {
        *outPtr  = val1_new;
        if (min_x!=NULL) {
          switch(n) {
          case 0:
        min_x[y][z] = min(min_x[y][z],x+1);
        max_x[y][z] = max(max_x[y][z],x+1);
        break;
          case 1:
        min_x[y+1][z] = min(min_x[y+1][z],x);
        max_x[y+1][z] = max(max_x[y+1][z],x);
        break;
          case 2:
        min_x[y][z+1] = min(min_x[y][z+1],x);
        max_x[y][z+1] = max(max_x[y][z+1],x);
        break;
          }
        }
      } // end if
    } else
      fprintf(stderr,"Func_IsoSurfDist3D_band() \t norm<=EPSILON \n");

      } // end if  (sign != sign_neigh)
    } // end for n


  } // end for i , parsing narrow band

  //  fprintf(stderr,"; "); fflush(stderr);

} // IsoSurfDist3D_band()


//----------------------------------------------------------------------
void vtkImageIsoContourDist::PrintSelf(ostream& os, vtkIndent indent)
{
   vtkImageToImageFilter::PrintSelf(os,indent);

} // PrintSelf()
