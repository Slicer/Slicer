/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkTclHelper.cxx,v $
  Date:      $Date: 2006/01/06 17:58:00 $
  Version:   $Revision: 1.4 $

  =========================================================================auto=*/

/* 
 * vtkTclHelper allows access to Tcl and vtk routines in the same class
 * inspired by vtkTkRenderWidget and similar classes.
 */


#include <stdlib.h>

#include "vtkTclHelper.h"
#include "vtkObject.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include "vtkTclUtil.h"
#include "vtkPointData.h"
#include "vtkDataArray.h"
#include "vtkFloatArray.h"
#include "vtkDoubleArray.h"
#include "vtkMath.h"

vtkCxxRevisionMacro(vtkTclHelper, "$Revision: 1.4 $");
vtkStandardNewMacro(vtkTclHelper);


vtkTclHelper::vtkTclHelper()
{
  this->Interp = NULL;
  this->ImageData = NULL;
  this->VolumeNode = NULL;
  this->MeasurementFrame = vtkMatrix4x4::New();
  this->MeasurementFrame->Identity();
}


vtkTclHelper::~vtkTclHelper() 
{ 
}

/* Get the normalized and y-flipped RASToVTK Matrix*/
void
vtkTclHelper::GetRASToVTKMatrix(vtkMatrix4x4 *RASToVTK)
{ 
  RASToVTK->Identity();
 
  //GetRASToVTK Matrix from the node
  this->VolumeNode->GetRASToIJKMatrix(RASToVTK);
  //normalize matrix RASToVTK       
  double RasToVTK_tmp[9];
  int iii, jjj;
  for (iii = 0; iii < 3; iii++) {
    for (jjj = 0; jjj < 3; jjj++) {
      RasToVTK_tmp[iii*3+jjj] = RASToVTK->GetElement(iii,jjj);
    }
    vtkMath::Normalize(RasToVTK_tmp+iii*3);
  }         
  for (iii = 0; iii < 3; iii++) {
    for (jjj = 0; jjj < 3; jjj++) {
      RASToVTK->SetElement(iii,jjj,RasToVTK_tmp[iii*3+jjj]); 
    }
  }        
  for (int j = 0; j < 3; j++) {
    // famous Y flip -- to get RasToVtk
    RASToVTK->SetElement(1,j,(-1.* RASToVTK->GetElement(1,j)));
    //make sure 3rd row and column are zeros:
    RASToVTK->SetElement(j,3,0.);
    RASToVTK->SetElement(3,j,0.);
  }
  //make sure (3,3) is one:
  RASToVTK->SetElement(3,3,1.);
}

void 
vtkTclHelper::SetInterpFromCommand(unsigned long tag)
{
  vtkCommand *c = this->GetCommand(tag);
  vtkTclCommand *tc = (vtkTclCommand *) c;

  this->Interp = tc->Interp;
}

void 
vtkTclHelper::SendImageDataScalars(char *sockname)
{
  int mode;
  Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);

  if ( ! (mode & TCL_WRITABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not writable\n");
      return;
    }

  if ( this->ImageData == NULL )
    {   vtkErrorMacro("Image Data is NULL");
      return;
    }

  int dims[3];
  this->ImageData->GetDimensions(dims);
  int bytes = this->ImageData->GetScalarSize() * 
    this->ImageData->GetNumberOfScalarComponents() * 
    dims[0] * dims[1] * dims[2];

  int written = Tcl_WriteRaw(channel, (char *) this->ImageData->GetScalarPointer(), bytes);
  Tcl_Flush(channel);

  if ( written != bytes )
    {   vtkErrorMacro ("Only wrote " << written << " but expected to write " << bytes << "\n");
      return;
    }
}

void
vtkTclHelper::SendImageDataTensors(char *sockname)
{  
  int mode, elementSize, numTuples, written;
  int bytes, tensorArraySize;
  Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);
  int dims[3];
  vtkFloatingPointType tensor[3][3];
  vtkDataArray *tensorArray;
  vtkMatrix4x4 *t;

  t = vtkMatrix4x4::New();
  
  if ( ! (mode & TCL_WRITABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not writable\n");
      return;
    }
  
  if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
      return;
    }
    
  this->ImageData->GetDimensions(dims);
  tensorArray = this->ImageData->GetPointData()->GetTensors();
  elementSize = this->ImageData->GetScalarSize();
  numTuples = tensorArray->GetNumberOfTuples();
 
  if (!tensorArray)
    {
      vtkGenericWarningMacro("No tensor data to send!");
      return;
    }
 
  /* sending data as a "3D-masked-symmetric-matrix", which 
     has 7 values: mask Mxx Mxy Mxz Myy Myz Mzz */
  tensorArraySize =  dims[0] * dims[1] * dims[2] * 7 ;
  bytes = elementSize * tensorArraySize;
    
  if (this->ImageData->GetScalarType() == 10) 
    { // type float 
      float *serializedTensorsP, *serializedTensorsP_copy;
      serializedTensorsP = new float[tensorArraySize];
      serializedTensorsP_copy = serializedTensorsP;
     
      for (int i = 0; i < numTuples; i++)
        {
          tensorArray->GetTuple(i, (vtkFloatingPointType *) tensor);
         
          // sending out tensor in this form:  mask Mxx Mxy Mxz Myy Myz Mzz
          // mask for now is always 1
          serializedTensorsP[7*i+0] = (float) 1;
          serializedTensorsP[7*i+1] = (float)(tensor[0][0]);
          serializedTensorsP[7*i+2] = (float)(tensor[0][1]);
          serializedTensorsP[7*i+3] = (float)(tensor[0][2]); 
          serializedTensorsP[7*i+4] = (float)(tensor[1][1]);
          serializedTensorsP[7*i+5] = (float)(tensor[1][2]);
          serializedTensorsP[7*i+6] = (float)(tensor[2][2]);
        }

      written = Tcl_WriteRaw(channel, (char *)(serializedTensorsP_copy), bytes);
    
    } else if (this->ImageData->GetScalarType() == 11) 
    { // type double
      double *serializedTensorsP, *serializedTensorsP_copy;
      serializedTensorsP = new double[tensorArraySize];
      serializedTensorsP_copy = serializedTensorsP;
        
      for (int i = 0; i < numTuples; i++)
        {
          tensorArray->GetTuple(i,(vtkFloatingPointType *)tensor);
          // sending out tensor in this form:  mask Mxx Mxy Mxz Myy Myz Mzz
          // mask for now is always 1
          
          serializedTensorsP[7*i+0] = (double) 1;
          serializedTensorsP[7*i+1] = (double) (tensor[0][0]); 
          serializedTensorsP[7*i+2] = (double) (tensor[0][1]);
          serializedTensorsP[7*i+3] = (double) (tensor[0][2]);
          serializedTensorsP[7*i+4] = (double) (tensor[1][1]);
          serializedTensorsP[7*i+5] = (double) (tensor[1][2]);
          serializedTensorsP[7*i+6] = (double) (tensor[2][2]);
        }  
      
      written = Tcl_WriteRaw(channel, (char *)(serializedTensorsP_copy), bytes);
      
    } else 
    { //other tensor data types we don't support here
      vtkGenericWarningMacro("Tensor type (vtk type !" << 
                             this->ImageData->GetScalarType()<< 
                             " not supported." << "\n");
      return;
    } 
  
  Tcl_Flush(channel);
   
  if ( written != bytes )
    {   vtkErrorMacro ("Only wrote " << written << " but expected to write " << bytes << "\n");
      return;
    }
}

/*Meeting with Steve and Raul (05-26-07): decision that in Matlab you'll get
  whatever Slicer has im memory. Don't worry for the measurement frame and space
  directions reduction that has been done to the data. This is the old method, 
  that tried to undo measurement frame and space directions reduction.*/

void
vtkTclHelper::SendImageDataTensors_UndoSlicerTransform(char *sockname)
{
  int mode, elementSize, numTuples, written;
  int bytes, tensorArraySize;
  Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);
  int dims[3];
  vtkFloatingPointType tensor[3][3];
  vtkDataArray *tensorArray;
  vtkMatrix4x4 *RASToVTK;
  vtkMatrix4x4 *measurementFrame_T;
  vtkMatrix4x4 *measurementFrame_inv;
  vtkMatrix4x4 *measurementFrame_T_inv;
  vtkMatrix4x4 *RASToVTK_inv;
  vtkMatrix4x4 *RASToVTK_T_inv;
  vtkMatrix4x4 *tmp;
  vtkMatrix4x4 *t;

  tmp = vtkMatrix4x4::New();
  t = vtkMatrix4x4::New();
  
  RASToVTK = vtkMatrix4x4::New();
  GetRASToVTKMatrix(RASToVTK);                 
                         
  RASToVTK_inv = vtkMatrix4x4::New();
  RASToVTK_inv->Identity();
  vtkMatrix4x4::Invert(RASToVTK,RASToVTK_inv);
  RASToVTK_T_inv = vtkMatrix4x4::New();
  RASToVTK_T_inv->Identity();
  vtkMatrix4x4::Transpose(RASToVTK,RASToVTK_T_inv);
  RASToVTK_T_inv->Invert();
  
  measurementFrame_inv = vtkMatrix4x4::New();
  measurementFrame_inv->Identity();
  vtkMatrix4x4::Invert(this->MeasurementFrame,measurementFrame_inv);
  measurementFrame_T_inv = vtkMatrix4x4::New();
  measurementFrame_T_inv->Identity();
  vtkMatrix4x4::Transpose(this->MeasurementFrame,measurementFrame_T_inv);
  measurementFrame_T_inv->Invert();
  
  if ( ! (mode & TCL_WRITABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not writable\n");
      return;
    }
  
  if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
      return;
    }
    
  this->ImageData->GetDimensions(dims);
  tensorArray = this->ImageData->GetPointData()->GetTensors();
  elementSize = this->ImageData->GetScalarSize();
  numTuples = tensorArray->GetNumberOfTuples();
 
  if (!tensorArray)
    {
      vtkGenericWarningMacro("No tensor data to send!");
      return;
    }
 
  /* sending data as a "3D-masked-symmetric-matrix", which 
     has 7 values: mask Mxx Mxy Mxz Myy Myz Mzz */
  tensorArraySize =  dims[0] * dims[1] * dims[2] * 7 ;
  bytes = elementSize * tensorArraySize;
    
  if (this->ImageData->GetScalarType() == 10) 
    { // type float 
      float *serializedTensorsP, *serializedTensorsP_copy;
      serializedTensorsP = new float[tensorArraySize];
      serializedTensorsP_copy = serializedTensorsP;
     
      for (int i = 0; i < numTuples; i++)
        {
          tmp->Identity();
          t->Identity();
          tensorArray->GetTuple(i, (vtkFloatingPointType *) tensor);
          
          t->SetElement(0,0,(double)(tensor[0][0]));
          t->SetElement(0,1,(double)(tensor[0][1]));
          t->SetElement(0,2,(double)(tensor[0][2]));
          t->SetElement(1,0,(double)(tensor[1][0]));
          t->SetElement(1,1,(double)(tensor[1][1]));
          t->SetElement(1,2,(double)(tensor[1][2]));
          t->SetElement(2,0,(double)(tensor[2][0]));
          t->SetElement(2,1,(double)(tensor[2][1]));
          t->SetElement(2,2,(double)(tensor[2][2]));
        
          vtkMatrix4x4::Multiply4x4(measurementFrame_inv, RASToVTK_inv, tmp);
          vtkMatrix4x4::Multiply4x4(tmp, t, tmp);
          vtkMatrix4x4::Multiply4x4(tmp, RASToVTK_T_inv, tmp);
          vtkMatrix4x4::Multiply4x4(tmp, measurementFrame_T_inv, t);
          
        
          // sending out tensor in this form:  mask Mxx Mxy Mxz Myy Myz Mzz
          // mask for now is always 1
          serializedTensorsP[7*i+0] = (float) 1;
          serializedTensorsP[7*i+1] = (float) (t->GetElement(0,0));
          serializedTensorsP[7*i+2] = (float) (t->GetElement(0,1));
          serializedTensorsP[7*i+3] = (float) (t->GetElement(0,2));
          serializedTensorsP[7*i+4] = (float) (t->GetElement(1,1));
          serializedTensorsP[7*i+5] = (float) (t->GetElement(1,2));
          serializedTensorsP[7*i+6] = (float) (t->GetElement(2,2));
        }

      written = Tcl_WriteRaw(channel, (char *)(serializedTensorsP_copy), bytes);
    
    } else if (this->ImageData->GetScalarType() == 11) 
    { // type double
      double *serializedTensorsP, *serializedTensorsP_copy;
      serializedTensorsP = new double[tensorArraySize];
      serializedTensorsP_copy = serializedTensorsP;
        
      for (int i = 0; i < numTuples; i++)
        {
          tensorArray->GetTuple(i,(vtkFloatingPointType *)tensor);
          // sending out tensor in this form:  mask Mxx Mxy Mxz Myy Myz Mzz
          // mask for now is always 1
          
          serializedTensorsP[7*i+0] = (double) 1;
          serializedTensorsP[7*i+1] = (double) (tensor[0][0]); 
          serializedTensorsP[7*i+2] = (double) (tensor[0][1]);
          serializedTensorsP[7*i+3] = (double) (tensor[0][2]);
          serializedTensorsP[7*i+4] = (double) (tensor[1][1]);
          serializedTensorsP[7*i+5] = (double) (tensor[1][2]);
          serializedTensorsP[7*i+6] = (double) (tensor[2][2]);
        }  
      
      written = Tcl_WriteRaw(channel, (char *)(serializedTensorsP_copy), bytes);
      
    } else 
    { //other tensor data types we don't support here
      vtkGenericWarningMacro("Tensor type (vtk type !" << 
                             this->ImageData->GetScalarType()<< 
                             " not supported." << "\n");
      return;
    } 
  
  Tcl_Flush(channel);
   
  if ( written != bytes )
    {   vtkErrorMacro ("Only wrote " << written << " but expected to write " << bytes << "\n");
      return;
    }
}

void 
vtkTclHelper::ReceiveImageDataScalars(char *sockname)
{
  int mode;

  Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);
    
  if ( ! (mode & TCL_READABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not readable" << "\n");
      return;
    }

  if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
      return;
    }

  int dims[3];
  this->ImageData->GetDimensions(dims);
  int bytes = this->ImageData->GetScalarSize() * 
    this->ImageData->GetNumberOfScalarComponents() * 
    dims[0] * dims[1] * dims[2];

  int read = Tcl_Read(channel, (char *) this->ImageData->GetScalarPointer(), bytes);

  if ( read != bytes )
    {   vtkErrorMacro ("Only read " << read << " but expected to read " << bytes << "\n");
      return;
    }
}

void 
vtkTclHelper::ReceiveImageDataTensors(char *sockname)
{
  int mode, elementSize, numberOfTuples, tensorArraySize, bytes;
  int dims[3];
  float *serializedTensorsP;
  float mxx, mxy, mxz, myy, myz, mzz; 
  vtkFloatArray* tensorArray;
  Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);
  
  if ( ! (mode & TCL_READABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not readable" << "\n");
      return;
    }

  if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
      return;
    }
  this->ImageData->GetDimensions(dims);
  elementSize = this->ImageData->GetScalarSize();
  numberOfTuples = dims[0] * dims[1] * dims[2]; 
  tensorArraySize = numberOfTuples * 7;
  bytes = elementSize * tensorArraySize;
  serializedTensorsP = new float[tensorArraySize];
  
  if (this->ImageData->GetScalarType() != 10) {
    //only accept type float!
    vtkErrorMacro ("Attempt to send tensor data that is not type float." <<
                   " Sorry, not supported yet!\n");
    return;
  } else { // type float
    tensorArray = vtkFloatArray::New();
    // assume number of components will always be 9 for tensors in slicer
    tensorArray->SetNumberOfComponents(9);
    tensorArray->SetNumberOfTuples(numberOfTuples);
    
    int read = Tcl_Read(channel, (char *)serializedTensorsP, bytes);
    
    if ( read != bytes )
      {   vtkErrorMacro ("Only read " << read << " but expected to read " << bytes << "\n");
        return;
      }
    for (int i=0; i < tensorArray->GetNumberOfTuples(); i++)
      {
        /* throw away every 7th value because that is the mask that we don't pay
           attention to for now */
        serializedTensorsP++;
        mxx = *serializedTensorsP; serializedTensorsP++;
        mxy = *serializedTensorsP; serializedTensorsP++;
        mxz = *serializedTensorsP; serializedTensorsP++;
        myy = *serializedTensorsP; serializedTensorsP++;
        myz = *serializedTensorsP; serializedTensorsP++;
        mzz = *serializedTensorsP; serializedTensorsP++;
        
        tensorArray->SetComponent(i,0,mxx);
        tensorArray->SetComponent(i,1,mxy);
        tensorArray->SetComponent(i,2,mxz);
        tensorArray->SetComponent(i,3,mxy);
        tensorArray->SetComponent(i,4,myy);
        tensorArray->SetComponent(i,5,myz);
        tensorArray->SetComponent(i,6,mxz);
        tensorArray->SetComponent(i,7,myz);
        tensorArray->SetComponent(i,8,mzz);
      }
      
      this->ImageData->GetPointData()->SetTensors(tensorArray);
  }  
}

/*Meeting with Steve and Raul (05-26-07): decision that the measurement frame
  reduce and space directions reduce transformations that slicer does with
  tensordata has to be handled by the volume node. The plan is to have a method
  in the volumeNodeClass that will be in charge of doing what this method tries 
  to do.*/
void 
vtkTclHelper::ReceiveImageDataTensors_DoSlicerTransform(char *sockname)
{
  int mode, elementSize, numberOfTuples, tensorArraySize, bytes;
  int dims[3];
  float *serializedTensorsP;
  float mxx, mxy, mxz, myy, myz, mzz; 
  vtkFloatArray* tensorArray;
  vtkMatrix4x4 *RASToVTK;
  vtkMatrix4x4 *measurementFrame_T;
  vtkMatrix4x4 *RASToVTK_T;
  vtkMatrix4x4 *tmp;
  vtkMatrix4x4 *T_nrrd;
     
  Tcl_Channel channel = Tcl_GetChannel(this->Interp, sockname, &mode);
  
  if ( ! (mode & TCL_READABLE) )
    {   vtkErrorMacro ("Socket " << sockname << " is not readable" << "\n");
      return;
    }

  if ( this->ImageData == NULL )
    {   vtkErrorMacro ("Image Data is NULL");
      return;
    }
  RASToVTK = vtkMatrix4x4::New();
  GetRASToVTKMatrix(RASToVTK);           
  RASToVTK_T = vtkMatrix4x4::New();
  RASToVTK_T->Identity();
  vtkMatrix4x4::Transpose(RASToVTK,RASToVTK_T);
  measurementFrame_T = vtkMatrix4x4::New();
  measurementFrame_T ->Identity();
  vtkMatrix4x4::Transpose(this->MeasurementFrame, measurementFrame_T);
 
  this->ImageData->GetDimensions(dims);
  elementSize = this->ImageData->GetScalarSize();
  numberOfTuples = dims[0] * dims[1] * dims[2]; 
  tensorArraySize = numberOfTuples * 7;
  bytes = elementSize * tensorArraySize;
  serializedTensorsP = new float[tensorArraySize];
  
  if (this->ImageData->GetScalarType() != 10) {
    //only accept type float!
    vtkErrorMacro ("Attempt to send tensor data that is not type float." <<
                   " Sorry, not supported yet!\n");
    return;
  } else { // type float
    tensorArray = vtkFloatArray::New();
    // assume number of components will always be 9 for tensors in slicer
    tensorArray->SetNumberOfComponents(9);
    tensorArray->SetNumberOfTuples(numberOfTuples);
    
    int read = Tcl_Read(channel, (char *)serializedTensorsP, bytes);
    
    if ( read != bytes )
      {   vtkErrorMacro ("Only read " << read << " but expected to read " << bytes << "\n");
        return;
      }
    
    T_nrrd = vtkMatrix4x4::New();
    tmp = vtkMatrix4x4::New();
    for (int i=0; i < tensorArray->GetNumberOfTuples(); i++)
      {
        /* throw away every 7th value because that is the mask that we don't pay
           attention to for now */
        serializedTensorsP++;
        mxx = *serializedTensorsP; serializedTensorsP++;
        mxy = *serializedTensorsP; serializedTensorsP++;
        mxz = *serializedTensorsP; serializedTensorsP++;
        myy = *serializedTensorsP; serializedTensorsP++;
        myz = *serializedTensorsP; serializedTensorsP++;
        mzz = *serializedTensorsP; serializedTensorsP++;
        
        T_nrrd->Identity(); 
        T_nrrd->SetElement(0,0,mxx);
        T_nrrd->SetElement(0,1,mxy);
        T_nrrd->SetElement(1,0,mxy);
        T_nrrd->SetElement(0,2,mxz);
        T_nrrd->SetElement(2,0,mxz); 
        T_nrrd->SetElement(1,1,myy);
        T_nrrd->SetElement(1,2,myz);
        T_nrrd->SetElement(2,1,myz); 
        T_nrrd->SetElement(2,2,mzz);
        
        tmp->Identity();
        vtkMatrix4x4::Multiply4x4(RASToVTK, this->MeasurementFrame, tmp);
        vtkMatrix4x4::Multiply4x4(tmp, T_nrrd, tmp);
        vtkMatrix4x4::Multiply4x4(tmp, measurementFrame_T, tmp);
        vtkMatrix4x4::Multiply4x4(tmp,RASToVTK_T, tmp);
       
        tensorArray->SetComponent(i,0,tmp->GetElement(0,0));
        tensorArray->SetComponent(i,1,tmp->GetElement(0,1));
        tensorArray->SetComponent(i,2,tmp->GetElement(0,2));
        tensorArray->SetComponent(i,3,tmp->GetElement(1,0));
        tensorArray->SetComponent(i,4,tmp->GetElement(1,1));
        tensorArray->SetComponent(i,5,tmp->GetElement(1,2));
        tensorArray->SetComponent(i,6,tmp->GetElement(2,0));
        tensorArray->SetComponent(i,7,tmp->GetElement(2,1));
        tensorArray->SetComponent(i,8,tmp->GetElement(2,2));
      }
      
      this->ImageData->GetPointData()->SetTensors(tensorArray);
  }  
}

const char *
vtkTclHelper::Execute (char *Command)
{
  int res;
#if TCL_MAJOR_VERSION == 8 && TCL_MINOR_VERSION <= 2
  res = Tcl_GlobalEval(this->Interp, this->Command);
#else
  res = Tcl_EvalEx(this->Interp, Command, -1, TCL_EVAL_GLOBAL);
#endif  

  return Tcl_GetStringResult (this->Interp);
}


void vtkTclHelper::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Interp: " << this->Interp << "\n";
  os << indent << "ImageData: " << this->ImageData << "\n";

}

