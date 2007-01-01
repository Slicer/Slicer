/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMAtlasSegmenter.cxx,v $
  Date:      $Date: 2006/12/15 19:11:06 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
// Since 22-Apr-02 vtkImageEMAtlas3DSegmenter is called vtkImageEMAtlasSegmenter - Kilian
// EMAtlas =  using EM Algorithm with Local Tissue Class Probability
#include "vtkImageEMAtlasSegmenter.h"
#include "vtkObjectFactory.h"
#include "vtkImageAccumulate.h"
#include "vtkMultiThreader.h" 
#include "assert.h"

//------------------------------------------------------------------------------
// Define Procedures for vtkImageEMAtlasSegmenter
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
vtkImageEMAtlasSegmenter* vtkImageEMAtlasSegmenter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMAtlasSegmenter");
  if(ret)
  {
    return (vtkImageEMAtlasSegmenter*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMAtlasSegmenter;
}

//----------------------------------------------------------------------------
vtkImageEMAtlasSegmenter::vtkImageEMAtlasSegmenter()
{
  this->NumIter = 1;                     // Number of EM-MRF Iterations
  this->NumRegIter = 1;                  // Number of Regularizing Iterations
  this->Alpha = 0.7;                     // 0 <= alpha <= 1. Be carefull - has great influence over outcome 
  this->SmoothingWidth = 11;             // Width for Gausian to regularize weights
  this->SmoothingSigma = 5;              // Sigma paramter for regularizing Gaussian
  this->NumInputImages = 0;              // Number of input images
 
  this->PrintDir = NULL;                 // Directory in which everything should be printed out  
  memset(this->Extent, 0, sizeof(int)*6);  // Need to know the original extent for several functions in the class 

  this->NumberOfTrainingSamples = 0;     // Number of Training Samples Probability Image has been summed up over !
  this->ImageProd = 0;                   // Size of Image = maxX*maxY*maxZ

  this->HeadClass = NULL;
  this->activeSuperClass = NULL;
  this->activeClass      = NULL;
  this->activeClassType  = SUPERCLASS;
}

//------------------------------------------------------------------------------
vtkImageEMAtlasSegmenter::~vtkImageEMAtlasSegmenter(){
  this->DeleteVariables();
}

//----------------------------------------------------------------------------
void vtkImageEMAtlasSegmenter::PrintSelf(ostream& os,vtkIndent indent) {
  int i;
   // vtkImageEMGeneral::PrintSelf(os,indent);
   
   os << indent << "NumIter:                    " << this->NumIter << "\n";
   os << indent << "NumRegIter:                 " << this->NumRegIter << "\n";
   os << indent << "Alpha:                      " << this->Alpha << "\n";
   os << indent << "SmoothingWidth:             " << this->SmoothingWidth << "\n";
   os << indent << "SmoothingSigma:             " << this->SmoothingSigma << "\n";

   os << indent << "NumInputImages:             " << this->NumInputImages << "\n";
   os << indent << "PrintDir:                   " << (this->PrintDir ? this->PrintDir : "(none)") << "\n"; 
   os << indent << "NumberOfTrainingSamples:    " << this->NumberOfTrainingSamples << "\n";

   os << indent << "activeSuperClass:           " ;
   if (this->activeSuperClass) os << this->activeSuperClass->GetLabel() << "\n";
   else os << "(none) \n"; 
   os << indent << "activeClassType:            ";
   if (activeClassType == CLASS) {
     os << "CLASS" << "\n";
     os << indent << "activeClass:                "; 
     if (this->activeClass) os << ((vtkImageEMAtlasClass*)this->activeClass)->GetLabel() << "\n";
     else os << "(none) \n";
   } else {
      os << "SUPERCLASS" << "\n";
      os << indent << "activeClass:                ";
      if (this->activeClass) os << ((vtkImageEMAtlasSuperClass*)this->activeClass)->GetLabel() << "\n";
      else os << "(none) \n";
   }
   os << indent << "Extent:                     " ;
   for (i=0; i < 6; i++) os << this->Extent[i]<< " " ; 
   os << "\n";

   this->HeadClass->PrintSelf(os,indent);
}
//----------------------------------------------------------------------------
// Calculates the weighted inverse covariance 
int EMAtlasSegment_CalcWeightedCovariance(vtkImageEMAtlasSegmenter* self,double** WeightedInvCov, double & SqrtDetWeightedInvCov, float* Weights, double** LogCov, int & VirtualDim,int dim) {
  int x,y,Xindex,Yindex;
  // Calculate the weighted coveriance => inpput channels are differently weighted
  VirtualDim = 0; 
  for (x=0;x < dim ; x++)  if (Weights[x] > 0.0) VirtualDim ++;
  for (x=0; x < dim ; x++) memset(WeightedInvCov[x],0,sizeof(double)*dim);
  if (VirtualDim == 0) {
    vtkEMAddWarningMessageSelf("EMAtlasSegment_CalcWeightedCovariance:  All Weights are set to 0");
    SqrtDetWeightedInvCov =0;
    return 1;
  }
  double** InvLogCov     = new double*[VirtualDim];
  double** VirtualLogCov = new double*[VirtualDim];
  Xindex = 0; 
  // Take out rows with zeros to decrease dimension
  for (x=0; x < VirtualDim ; x++) {
    InvLogCov[x] = new double[VirtualDim];
    VirtualLogCov[x] = new double[VirtualDim];
    while (Weights[Xindex] == 0.0) Xindex ++;
    Yindex = 0;
    for (y=0;y < VirtualDim; y++) {
       while (Weights[Yindex] == 0.0) Yindex ++;
       VirtualLogCov[x][y] = LogCov[Xindex][Yindex];
       Yindex ++; 
    }
    Xindex ++;
  }
  // Calculate Invers of the matrix
  if (vtkImageEMGeneral::InvertMatrix(VirtualLogCov,InvLogCov,VirtualDim) == 0 ) return 0;
 
  // Theory behind calculating weighted inverse 
  // P(x|tissue) = 1/(2pi)^(n/2)  * 1/ Det(S)^0.5 * e^ -0.5( (x-m) S^-1 (x-m) )
  // P(x|tissue,InputWeight) =  1/(2pi)^(Nvirtual/2)  * det(S'^-1)^0.5 * e^ -0.5( (x-m) S'^-1 (x-m) )
  // where (x-m) S'^-1 (x-m) = (x-m) .* InputWeight S^-1 (x-m) .* InputWeight  
  // =>  for n = 2 S'^-1 = ( w1*w1*s11 w1*w2*s12 | w1*w2*s21  w2*w2*s22) where InputWeight = (w1 w2) and S^-1 = (s11 s12 | s21 s22)  
  // Nvirual = # of wi which are non-zero
 
  // Copy over values and multipky them with weights



  Xindex = 0;
  for (x=0; x < VirtualDim ; x++) {
    while (Weights[Xindex] == 0.0) Xindex ++;
    Yindex = 0;
    for (y=0;y < VirtualDim; y++) {
       while (Weights[Yindex] == 0.0) Yindex ++;
       WeightedInvCov[Xindex][Yindex] = InvLogCov[x][y] * double(Weights[Xindex] * Weights[Yindex]); 
       Yindex ++;
    }
    Xindex ++;
  }

  // Calculate the weighted determinant 
  SqrtDetWeightedInvCov = sqrt(vtkImageEMGeneral::determinant(WeightedInvCov,VirtualDim));
  // Take out rows with zeros to dcrease dimension
  for (x=0; x < VirtualDim ; x++) {
    delete[] InvLogCov[x];
    delete[] VirtualLogCov[x];
  }
  delete[] InvLogCov;
  delete[] VirtualLogCov;

  if (SqrtDetWeightedInvCov != SqrtDetWeightedInvCov)  return 0;
  return 1;
}

//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageEMAtlasSegmenterReadInputChannel(vtkImageEMAtlasSegmenter *self,vtkImageData *in1Data, T *in1Ptr,int inExt[6],float** InputVector,int InputIndex)
{

  int idxR, idxY, idxZ;
  vtkIdType inIncX, inIncY, inIncZ;
  int *SegmentationBoundaryMin = self->GetSegmentationBoundaryMin();
  int *SegmentationBoundaryMax = self->GetSegmentationBoundaryMax();
  int ImageMaxZ = self->GetDimensionZ();
  int ImageMaxY = self->GetDimensionY();
  int ImageMaxX = self->GetDimensionX();

  int index = 0; 

  // Get increments to march through data 
  in1Data->GetContinuousIncrements(inExt, inIncX, inIncY, inIncZ);
  // We assume here input is 1 scalar -> it is checked in CheckInputImages

  int LengthOfXDim = inExt[1] - inExt[0]+1 + inIncY;
  int LengthOfYDim = LengthOfXDim*(inExt[3] - inExt[2]+1) + inIncZ;  
  int jump = (SegmentationBoundaryMin[0] - 1) + (SegmentationBoundaryMin[1] - 1) * LengthOfXDim
              + LengthOfYDim *(SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - ImageMaxX;
  int BoundaryDataIncZ = LengthOfYDim - ImageMaxY *LengthOfXDim;

  in1Ptr += jump;

  // cout << "jump " << jump << "BoundaryDataIncY " << BoundaryDataIncY << " BoundaryDataIncZ " << BoundaryDataIncZ << endl;

  for (idxZ = 0; idxZ < ImageMaxZ ; idxZ++) { 
    for (idxY = 0; idxY <  ImageMaxY; idxY++) {
      for (idxR = 0; idxR < ImageMaxX; idxR++) {
    InputVector[index][InputIndex] = log(float(* in1Ptr) +1);
    index ++; 
    in1Ptr++;
      }
      in1Ptr += BoundaryDataIncY;
    }
    in1Ptr += BoundaryDataIncZ;
  }
}

//----------------------------------------------------------------------------
//Could not put it into another file like vtkImageGeneral - then it would seg falt - do not ask me why 
void* vtkImageEMAtlasSegmenter_GetPointerToVtkImageData(vtkImageData *Image, int DataType, int Ext[6]) {
 Image->SetWholeExtent(Ext);
 Image->SetExtent(Ext); 
 Image->SetNumberOfScalarComponents(1);
 Image->SetScalarType(DataType); 
 Image->AllocateScalars(); 
 return Image->GetScalarPointerForExtent(Ext);
}

//----------------------------------------------------------------------------
// Transferes the data wrom the working extent (that is only the area defined by the SegmentationBounday)
// to the Extent of the output
template <class TIn, class TOut>
static void vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(vtkImageEMAtlasSegmenter *self,TIn* inputExtension_Vector, TOut* outputExtension_Ptr, int outputExtent_Inc[3]) {
  // -----------------------------------------------------
  // 1.) Setup Parameteres 
  // -----------------------------------------------------
  // Get increments to march through data
  int *outputExtension = self->GetExtent();
  int *SegmentationBoundaryMin = self->GetSegmentationBoundaryMin();
  int DimensionX = self->GetDimensionX();
  int DimensionY = self->GetDimensionY();
  int DimensionZ = self->GetDimensionZ();

  int idxR, idxY, idxZ;

  // We assume output is 1 scalar 
  TOut* outputExtensionStart_Ptr = outputExtension_Ptr; 

  int LengthOfXDim = outputExtension[1] - outputExtension[0] +1  + outputExtent_Inc[1];
  int LengthOfYDim = LengthOfXDim*(outputExtension[3] - outputExtension[2] + 1) +  outputExtent_Inc[2];  
  // Location of the first voxel 
  outputExtensionStart_Ptr += (SegmentationBoundaryMin[0] - 1) + (SegmentationBoundaryMin[1] - 1) * LengthOfXDim
         + LengthOfYDim *(SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - DimensionX;
  int BoundaryDataIncZ = LengthOfYDim - DimensionY *LengthOfXDim;

  // -----------------------------------------------------
  // 2.) Set output to zero 
  // -----------------------------------------------------

  // Initializa output by setting it to zero
  for (idxZ = outputExtension[4]; idxZ <= outputExtension[5]; idxZ++) {
    for (idxY = outputExtension[2]; idxY <= outputExtension[3]; idxY++) {
      memset(outputExtension_Ptr,0,sizeof(TOut)*(outputExtension[1] - outputExtension[0] + 1));
      outputExtension_Ptr +=  LengthOfXDim;
    }
  }
  outputExtension_Ptr = outputExtensionStart_Ptr ;  


  // -----------------------------------------------------
  // 3.) Define output in working Extension 
  // -----------------------------------------------------
  int index = 0;
  // 3.) Write result in outPtr
  for (idxZ = 0; idxZ < DimensionZ; idxZ++) {
    for (idxY = 0; idxY < DimensionY; idxY++) {
      for (idxR = 0; idxR < DimensionX; idxR++) {
        *outputExtension_Ptr = (TOut) inputExtension_Vector[index];
    index ++; 
        outputExtension_Ptr++;
      }
      outputExtension_Ptr +=  BoundaryDataIncY;
    }
    outputExtension_Ptr +=  BoundaryDataIncZ;
  }
}

//----------------------------------------------------------------------------
// Transferes the data from the  working extent (that is only the area defined by the SegmentationBounday)
// to the Extent of the output and prints it out 

template <class TIn>
static void vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(vtkImageEMAtlasSegmenter *self,TIn* inputExtension_Vector, int outputScalar, char* FileName){
  vtkImageData *OriginalExtension_Data = vtkImageData::New(); 
  int ChangedExtent[6];
  memcpy(ChangedExtent, self->GetExtent(), sizeof(int)*6); 
  // Just move it one up - this is just a shortcut - fix it later 
  // Whatever extent [4] and [5] is set to that will the numbering of the slice number int the print out 
  if (!ChangedExtent[4]) {
    ChangedExtent[4] ++;
    ChangedExtent[5] ++;
  }
  TIn* OriginalExtension_DataPtr = (TIn*) vtkImageEMAtlasSegmenter_GetPointerToVtkImageData(OriginalExtension_Data,outputScalar,ChangedExtent);
  vtkIdType outIncX, OutIncY, outIncZ;
  OriginalExtension_Data->GetContinuousIncrements(self->GetExtent(), outIncX, OutIncY, outIncZ);
  int outInc[3] = {outIncX, OutIncY, outIncZ};
  vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(self,inputExtension_Vector,OriginalExtension_DataPtr ,outInc);
  
  self->GEImageWriter(OriginalExtension_Data,FileName,1);
  OriginalExtension_Data->Delete();    
}

template <class TIn, class TOut>
static void  vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(vtkImageEMAtlasSegmenter  *selfPtr,TIn* inputExtension_Vector, TOut* outputExtension_Ptr, int outputExtent_Inc[3], int SliceNum) {
  // -----------------------------------------------------
  // 1.) Setup Parameteres 
  // -----------------------------------------------------
  // Get increments to march through data
  int outputExtension[6];
  memcpy(outputExtension,selfPtr->GetExtent(), sizeof(int)*6);

  int SegmentationBoundaryMin[3];
  memcpy(SegmentationBoundaryMin,selfPtr->GetSegmentationBoundaryMin(), sizeof(int)*3);

  int DimensionX = selfPtr->GetDimensionX();
  int DimensionY = selfPtr->GetDimensionY();
  int DimensionZ = selfPtr->GetDimensionZ();

  if (SliceNum > 0) {
    outputExtension[4] += SliceNum-1 + SegmentationBoundaryMin[2] -1;
    outputExtension[5] = outputExtension[4];
    SegmentationBoundaryMin[2] = 1;
    DimensionZ = 1;
  }

  int idxR, idxY, idxZ;

  // We assume output is 1 scalar 
  TOut* outputExtensionStart_Ptr = outputExtension_Ptr; 

  int LengthOfXDim = outputExtension[1] - outputExtension[0] +1  + outputExtent_Inc[1];
  int LengthOfYDim = LengthOfXDim*(outputExtension[3] - outputExtension[2] + 1) +  outputExtent_Inc[2];  
  // Location of the first voxel 
  outputExtensionStart_Ptr += (SegmentationBoundaryMin[0] - 1) + (SegmentationBoundaryMin[1] - 1) * LengthOfXDim
         + LengthOfYDim *(SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - DimensionX;
  int BoundaryDataIncZ = LengthOfYDim - DimensionY *LengthOfXDim;

  // -----------------------------------------------------
  // 2.) Set output to zero 
  // -----------------------------------------------------

  // Initializa output by setting it to zero
  for (idxZ = outputExtension[4]; idxZ <= outputExtension[5]; idxZ++) {
    for (idxY = outputExtension[2]; idxY <= outputExtension[3]; idxY++) {
      memset(outputExtension_Ptr,0,sizeof(TOut)*(outputExtension[1] - outputExtension[0] + 1));
      outputExtension_Ptr +=  LengthOfXDim;
    }
  }
  outputExtension_Ptr = outputExtensionStart_Ptr ;  


  // -----------------------------------------------------
  // 3.) Define output in working Extension 
  // -----------------------------------------------------
  int index = 0;
  // 3.) Write result in outPtr
  for (idxZ = 0; idxZ < DimensionZ; idxZ++) {
    for (idxY = 0; idxY < DimensionY; idxY++) {
      for (idxR = 0; idxR < DimensionX; idxR++) {
        *outputExtension_Ptr = (TOut) inputExtension_Vector[index];
    index ++; 
        outputExtension_Ptr++;
      }
      outputExtension_Ptr +=  BoundaryDataIncY;
    }
    outputExtension_Ptr +=  BoundaryDataIncZ;
  }
}


template <class TIn>
static void  vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(vtkImageEMAtlasSegmenter *selfPtr,TIn* inputExtension_Vector, int outputScalar, char* FileName ,int SliceNum, int OriginalExtensionFlag, int PrintOutputFlag){
  vtkImageData *OriginalExtension_Data = vtkImageData::New(); 
  int ChangedExtent[6];
  memcpy(ChangedExtent, selfPtr->GetExtent(), sizeof(int)*6); 
  // Just move it one up - this is just a shortcut - fix it later 
  // Whatever extent [4] and [5] is set to that will the numbering of the slice number int the print out 
  if (!ChangedExtent[4]) {
    ChangedExtent[4] ++;
    ChangedExtent[5] ++;
  }
  if (SliceNum > 0) {
    ChangedExtent[4] += SliceNum-1 + selfPtr->GetSegmentationBoundaryMin()[2] -1 ;
    ChangedExtent[5] = ChangedExtent[4];
  } 
  TIn* OriginalExtension_DataPtr = (TIn*) vtkImageEMAtlasSegmenter_GetPointerToVtkImageData(OriginalExtension_Data,outputScalar,ChangedExtent);
  vtkIdType OutIncX, OutIncY, OutIncZ;
  OriginalExtension_Data->GetContinuousIncrements(selfPtr->GetExtent(), OutIncX, OutIncY, OutIncZ);
  if ( OriginalExtensionFlag) {
    int NumX = ChangedExtent[1] -ChangedExtent[0] + 1;
    for(int z = 0; z <= ChangedExtent[5] -ChangedExtent[4]; z++) {
      for(int y = 0; y <= ChangedExtent[3] -ChangedExtent[2]; y++) {
    memcpy(OriginalExtension_DataPtr, inputExtension_Vector,sizeof(TIn)*NumX);
    inputExtension_Vector += NumX;
    OriginalExtension_DataPtr += NumX+OutIncY;
      }
      OriginalExtension_DataPtr += OutIncZ;
    }
  } else {
    int outInc[3] = {OutIncX, OutIncY, OutIncZ};

     vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(selfPtr,inputExtension_Vector,OriginalExtension_DataPtr ,outInc,SliceNum);
  } 
  selfPtr->GEImageWriter(OriginalExtension_Data,FileName,PrintOutputFlag);
  OriginalExtension_Data->Delete();    
}

//------------------------------------------------------------------------------
int* vtkImageEMAtlasSegmenter::GetSegmentationBoundaryMin() {
  if (!this->HeadClass) {
    vtkEMAddErrorMessage("No Head Class defined");
    return NULL;
  } 
  return this->HeadClass->GetSegmentationBoundaryMin();
} 

int* vtkImageEMAtlasSegmenter::GetSegmentationBoundaryMax() {
  if (!this->HeadClass) {
    vtkEMAddErrorMessage("No Head Class defined");
    return NULL;
  } 
  return this->HeadClass->GetSegmentationBoundaryMax();
} 


int vtkImageEMAtlasSegmenter::GetDimensionX() {
  if (!this->HeadClass) {
    vtkEMAddErrorMessage("No Head Class defined");
    return -1;
  }
  if (this->HeadClass->GetDataDim()[0]) {
    return this->HeadClass->GetDataDim()[0];
  } 
  return (this->HeadClass->GetSegmentationBoundaryMax()[0] - this->HeadClass->GetSegmentationBoundaryMin()[0] +1);
}

int vtkImageEMAtlasSegmenter::GetDimensionY() {
  if (!this->HeadClass) {
    vtkEMAddErrorMessage("No Head Class defined");
    return -1;
  } 
  if (this->HeadClass->GetDataDim()[1]) {
    return this->HeadClass->GetDataDim()[1];
  }
  return (this->HeadClass->GetSegmentationBoundaryMax()[1] - this->HeadClass->GetSegmentationBoundaryMin()[1] +1);
}

int vtkImageEMAtlasSegmenter::GetDimensionZ() {
  if (!this->HeadClass) {
    vtkEMAddErrorMessage("No Head Class defined");
    return -1;
  }
  if (this->HeadClass->GetDataDim()[2]) { 
    return this->HeadClass->GetDataDim()[2];
  }
  return (this->HeadClass->GetSegmentationBoundaryMax()[2] - this->HeadClass->GetSegmentationBoundaryMin()[2] +1);
}

//------------------------------------------------------------------------------
// #if EM_VTK_OLD_SETTINGS
// int vtkImageEMAtlasSegmenter::CheckInputImage(vtkImageData * inData,int DataTypeOrig, float DataSpacingOrig[3], int num) {
//  float DataSpacingNew[3];
// #else 
int vtkImageEMAtlasSegmenter::CheckInputImage(vtkImageData * inData,int DataTypeOrig, vtkFloatingPointType DataSpacingOrig[3], int num) {
  vtkFloatingPointType DataSpacingNew[3];
// #endif

  int inExt[6];

  if (inData == NULL) {
    vtkEMAddErrorMessage("CheckInputImage: Input "<< num << " must be specified.");
    return 1;
  }

  // Check for Data Type if correct : Remember ProbabilityData all has to be of the same data Type
  int DataTypeNew = inData->GetScalarType();
  if (DataTypeOrig != DataTypeNew) 
    vtkEMAddErrorMessage("CheckInputImage: Input "<< num << " has wrong data type ("<< inData->GetScalarType() << ") instead of " 
                            << DataTypeOrig << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT );

  // Check if it is one of the registered types
  if ((DataTypeNew != VTK_DOUBLE) && (DataTypeNew != VTK_FLOAT) && 
      (DataTypeNew != VTK_LONG) && (DataTypeNew != VTK_UNSIGNED_LONG) && (DataTypeNew != VTK_INT) && (DataTypeNew != VTK_UNSIGNED_INT) && 
      (DataTypeNew != VTK_SHORT) && (DataTypeNew != VTK_UNSIGNED_SHORT) && (DataTypeNew != VTK_CHAR) && (DataTypeNew != VTK_UNSIGNED_CHAR)) vtkEMAddErrorMessage("Input "<< num << " has not known data type.");

  // Check for dimenstion of InData
  // Could be easily fixed if needed 
  inData->GetWholeExtent(inExt);
  if ((inExt[1] != this->Extent[1]) || (inExt[0] != this->Extent[0]) || (inExt[3] != this->Extent[3]) || (inExt[2] != this->Extent[2]) || (inExt[5] != this->Extent[5]) || (inExt[4] != this->Extent[4])) 
    vtkEMAddErrorMessage("CheckInputImage: Extension of Input Image " << num << ", " << inExt[0] << "," << inExt[1] << "," << inExt[2] << "," << inExt[3] << "," << inExt[4] << "," << inExt[5] 
                  << "is not alligned with output image "  << this->Extent[0] << "," << this->Extent[1] << "," << this->Extent[2] << "," << this->Extent[3] << "," << this->Extent[4] << " " << this->Extent[5]);

  if (inData->GetNumberOfScalarComponents() != 1) 
    vtkEMAddErrorMessage("CheckInputImage: This filter assumes input to filter is defined with one scalar component. " << num  << " has " << inData->GetNumberOfScalarComponents() 
                  << " Can be easily changed !");

  inData->GetSpacing(DataSpacingNew);
  if ((DataSpacingOrig[0] !=  DataSpacingNew[0]) || (DataSpacingOrig[1] !=  DataSpacingNew[1]) || (DataSpacingOrig[2] !=  DataSpacingNew[2])) 
    vtkEMAddErrorMessage("CheckInputImage: Data Spacing of input images is unequal" );
  // Kilian Check for orientation has to be done in TCL !!!!

  // Needed bc Sylvain sometimes gives me input images with negative values and then claims that my program does not 
  // work anymore 
  vtkImageAccumulate *ia = vtkImageAccumulate::New();
  ia->SetInput(inData);
  ia->Update();
  double *min = ia->GetMin();
  if (min[0] < 0) 
    vtkEMAddErrorMessage("CheckInputImage: input images have negative values"); 
  ia->Delete();

  return this->GetErrorFlag();
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasSegmenter::SetNumInputImages(int number) {
  // Right now can just define it once
  // this->DeleteVariables();
  if (this->NumInputImages > 0) {
     vtkEMAddErrorMessage( "Number of input images was previously defined ! ");
    return;
  }
  this->NumInputImages = number;
}

// --------------------------------------------------------------------------------------------------------------------------
//  Probability Functions 
// --------------------------------------------------------------------------------------------------------------------------

// Close to Sandies original approach +(part of ISBI04)
inline double vtkEMAtlasSegment_ConditionalTissueProbability(const double TissueProbability, const double InvSqrtDetLogCov, 
                                                               const float *cY_M,  const double  *LogMu, double **InvLogCov, 
                                                               const int NumInputImages, const int  VirtualNumInputImages) {
   return TissueProbability  * vtkImageEMGeneral::FastGaussMulti(InvSqrtDetLogCov,cY_M, LogMu,InvLogCov,NumInputImages, VirtualNumInputImages); 
}

// Kilian's MICCAI02 + ISBI04 paper 
template <class T> 
inline double vtkEMAtlasSegment_SpatialIntensityProbability(const double TissueProbability, const float ProbDataMinusWeight, 
                                                              const float ProbDataWeight, const T *ProbDataPtr, const double InvSqrtDetLogCov, 
                                                              const float* cY_M,  const double  *LogMu, double **InvLogCov, 
                                                              const int  NumInputImages, const int VirtualNumInputImages) {
   return  (ProbDataMinusWeight + ProbDataWeight*(ProbDataPtr == NULL ? 0.0 : double(*ProbDataPtr))) 
            * vtkEMAtlasSegment_ConditionalTissueProbability(TissueProbability, InvSqrtDetLogCov, cY_M,  LogMu, InvLogCov, NumInputImages, VirtualNumInputImages); 
} 

// -----------------------------------------------------------
// Calculate MF - parrallelised version 
// -----------------------------------------------------------



template  <class T>
static void vtkImageEMAtlasSegmenter_MeanFieldApproximation3DPrivate(int id,
                     float **w_m_input, 
                     unsigned char* MapVector, 
                     float *cY_M, 
                     int imgX, 
                     int imgY, 
                     int imgXY, 
                     int StartVoxel, 
                     int EndVoxel,
                                     int NumClasses,
                     int NumTotalTypeCLASS,
                     int*  NumChildClasses,
                     int NumInputImages, 
                     double Alpha, 
                     double ***MrfParams,
                     T **ProbDataPtr, 
                     int *ProbDataIncY, 
                     int *ProbDataIncZ,
                     float *ProbDataWeight,
                                     float *ProbDataMinusWeight,
                     double **LogMu, 
                     double ***InvLogCov, 
                     double *InvSqrtDetLogCov,
                     double *TissueProbability,
                     int  *VirtualNumInputImages,
                     float **w_m_output) {

  float normRow;                                 
  double NeighborhoodEnergy;
  double ConditionalTissueProbability;
  double SpatialTissueDistribution;
  int i,j,k,l, index, 
    JumpHorizontal  = imgX,
    JumpSlice       = imgXY;
  // double **wm_input_start = new double[NumTotalTypeCLASS];
  double *wxp = new double[NumClasses],*wxpPtr = wxp,*wxn = new double[NumClasses],*wxnPtr = wxn, *wyn = new double[NumClasses],*wynPtr = wyn,
         *wyp = new double[NumClasses],*wypPtr = wyp,*wzn = new double[NumClasses],*wznPtr = wzn, *wzp = new double[NumClasses],*wzpPtr = wzp;  
  

  // -----------------------------------------------------------
  // neighbouring field values
  // -----------------------------------------------------------
  // The different Matrisses are defined as follow:
  // Matrix[i] = Neighbour in a certain direction
  // In Tcl TK defined in this order: West North Up East South Down 
  // MrfParams[i] :  
  // i = 3 pixel and east neighbour
  // i = 0 pixel and west neighbour
  // i = 4 pixel and south neighbour
  // i = 1 pixel and north neighbour
  // i = 2 pixel and previous neighbour (up)
  // i = 5 pixel and next neighbour (down)

  // -----------------------------------------------------------
  // Calculate Neighbouring Tissue Relationships
  // -----------------------------------------------------------

  j = StartVoxel;
  int ClassIndex;
  while (j< EndVoxel) {
    if (*MapVector < EMSEGMENT_DEFINED) {
      // Is is 0 => it is not an edge -> fast smmothing
      if (*MapVector) {
    for (i=0;i< NumClasses ;i++){
      *wxp=*wxn=*wyn=*wyp=*wzn=*wzp=0;
          ClassIndex = 0;
          for (k=0;k< NumClasses ;k++){
        for (l=0;l< NumChildClasses[k];l++){
          // f(j,l,h-1)
          if (*MapVector&EMSEGMENT_WEST) *wxn += (*w_m_input[ClassIndex])*(float)MrfParams[3][k][i]; 
          else                     *wxn += w_m_input[ClassIndex][-JumpHorizontal]*(float)MrfParams[3][k][i];
          // f(j,l,h+1)
          if (*MapVector&EMSEGMENT_EAST) *wxp += (*w_m_input[ClassIndex])*(float)MrfParams[0][k][i];
          else                     *wxp += w_m_input[ClassIndex][JumpHorizontal]*(float)MrfParams[0][k][i];
          //  Remember: The picture is upside down:
          // Therefore I had to switch the MRF parameters 1 (South) and 4(North)
          // f(j,l-1,h)
          if (*MapVector&EMSEGMENT_NORTH)  *wyn += (*w_m_input[ClassIndex])*(float)MrfParams[1][k][i];                       
          else                     *wyn += w_m_input[ClassIndex][-1]*(float)MrfParams[1][k][i]; 
          // f(j,l+1,h)
          if (*MapVector&EMSEGMENT_SOUTH)  *wyp += (*w_m_input[ClassIndex])*(float)MrfParams[4][k][i];
          else                     *wyp += w_m_input[ClassIndex][1]*(float)MrfParams[4][k][i];
          // f(j-1,l,h)
          if (*MapVector&EMSEGMENT_FIRST) *wzn += (*w_m_input[ClassIndex])*(float)MrfParams[5][k][i];  
          else                     *wzn += w_m_input[ClassIndex][-JumpSlice]*(float)MrfParams[5][k][i]; 
          // f(j+1,l,h)
          if (*MapVector&EMSEGMENT_LAST)  *wzp += (*w_m_input[ClassIndex])*(float)MrfParams[2][k][i]; 
          else                     *wzp += w_m_input[ClassIndex][JumpSlice]*(float)MrfParams[2][k][i]; 

              ClassIndex ++;
        }
      }
      wxn++;wxp++;wyn++;wyp++;wzn++;wzp++;
    }
     } else {
    for (i=0;i<NumClasses ;i++){
      *wxp=*wxn=*wyn=*wyp=*wzn=*wzp=0;
      ClassIndex = 0;
      for (k=0;k<NumClasses ;k++){
        for (l=0;l< NumChildClasses[k];l++){
          // f(j,l,h-1)
          *wxn += w_m_input[ClassIndex][-JumpHorizontal]*(float)MrfParams[3][k][i];
          // f(j,l,h+1)
          *wxp += w_m_input[ClassIndex][JumpHorizontal]*(float)MrfParams[0][k][i];
          // f(j,l-1,h)
          *wyn += w_m_input[ClassIndex][-1]*(float)MrfParams[4][k][i]; 
          // f(j,l+1,h)
          *wyp += w_m_input[ClassIndex][1]*(float)MrfParams[1][k][i];
          // f(j-1,l,h)
          *wzn += w_m_input[ClassIndex][-JumpSlice]*(float)MrfParams[5][k][i]; 
          // f(j+1,l,h)
          *wzp += w_m_input[ClassIndex][JumpSlice]*(float)MrfParams[2][k][i]; 
          ClassIndex ++;
        }
      }
      // cout << "Result " << i << " " << *wxp << " " <<  *wxn  << " " << *wyn << " " << *wyp << " " << *wzn << " " << *wzp << endl;      

      wxn++;wxp++;wyn++;wyp++;wzn++;wzp++;
    }
     }
      wxp = wxpPtr; wxn = wxnPtr; wyn = wynPtr; wyp = wypPtr; wzn = wznPtr; wzp = wzpPtr; 
      // -----------------------------------------------------------
      // Calculate Probabilities and Update Weights
      // -----------------------------------------------------------
      // mp = (ones(prod(imS),1)*p).*(1-alpha + alpha*wxn).*(1- alpha +alpha*wxp)...
      //.*(1- alpha + alpha*wyp).*(1-alpha + alpha*wyn).*(1- alpha + alpha*wzp).*(1-alpha + alpha*wzn);
      // w have to be normalized !
      normRow = 0;
      index = 0;
      // if ((j == 13891) || (j == 13946)) cout << "duda0 " << j << " " << ((*MapVector&EMSEGMENT_WEST) ? 1 : 0) << " " << ((*MapVector&EMSEGMENT_EAST) ? 1 : 0) << " - " << ((*MapVector&EMSEGMENT_FIRST) ? 1 : 0)<< " " << ((*MapVector&EMSEGMENT_LAST) ? 1 : 0) << " " << ((*MapVector&EMSEGMENT_SOUTH) ? 1 : 0) << " " << ((*MapVector&EMSEGMENT_NORTH) ? 1 : 0) << endl;

      for (i=0; i<NumClasses; i++) {
        // Goes a little bit further in the branches - like it better
        // it should be exp(*ProbData)[i] from Tina's thesis I say now bc values with zero probability will be still displayed
        // Kilian May 2002: I believe the form underneath is correct - Tina's PhD thesis displays it exactly like underneath 
    // if (((j == 13891) || (j == 13946)) && i == 0) cout << "duda1 " << i << " " << j << " " << *wxp  << " " <<  *wxn  <<  " " << *wyp  << " " <<  *wyn  <<  " " << *wzp  << " " <<  *wzn  <<  endl;
    // Kilian Mar 06: The following line is not correct - compare with vtkEMLocalSegment. I have not changed this yet bc I would need to alter the template file, which I am hesitent to do.  

    NeighborhoodEnergy = (1-Alpha+Alpha*exp((*wxp++) + (*wxn++) + (*wyp++) + (*wyn++) + (*wzp++) + (*wzn++)));
    for (l=0;l< NumChildClasses[i];l++){
       ConditionalTissueProbability = TissueProbability[i] * vtkImageEMGeneral::FastGaussMulti(InvSqrtDetLogCov[index],cY_M, LogMu[index],InvLogCov[index],NumInputImages, VirtualNumInputImages[i]); 
           SpatialTissueDistribution    = ProbDataMinusWeight[i] + ProbDataWeight[i]*(ProbDataPtr[index] == NULL ? 0.0 : double(*ProbDataPtr[index]));
           w_m_output[index][j] = (float) NeighborhoodEnergy *  SpatialTissueDistribution *  ConditionalTissueProbability;
          // if (((j == 13891) || (j == 13946)) && i == 0) {
        // cout << "duda2 " << index << " " << j << " " << w_m_output[index][j]  << " " <<  mp   << " " << TissueProbability[i] << " " << ProbDataMinusWeight[i] << " " <<ProbDataWeight[i]  << " " << *ProbDataPtr[index]  << " ";
          //  for (int blub2=0; blub2 < 2; blub2++) cout <<  cY_M[blub2] << " "; 
      //  cout << endl;
      // }
      normRow += w_m_output[index][j];
      index ++;
    } 
      }
      wxp = wxpPtr; wxn = wxnPtr; wyn = wynPtr; wyp = wypPtr; wzn = wznPtr; wzp = wzpPtr;
      // I do not know if I should assign it ot first or second term
      if (normRow == 0.0) {
        index = 0;

    for (i=0; i<NumClasses; i++) {
       NeighborhoodEnergy = TissueProbability[i] * exp((*wxp++) + (*wxn++) + (*wyp++) + (*wyn++) + (*wzp++) + (*wzn++));
      for (l=0;l< NumChildClasses[i];l++){
        w_m_output[index][j] =  float( NeighborhoodEnergy); 
        normRow += w_m_output[index][j];
        index ++;
      }
    }
    wxp = wxpPtr; wxn = wxnPtr; wyn = wynPtr; wyp = wypPtr; wzn = wznPtr; wzp = wzpPtr;
    if (normRow == 0.0) {
      index = 0;
      for (i=0; i<NumClasses; i++) {
        for (l=0;l< NumChildClasses[i];l++){
              // the line below is the same as 
              // vtkEMAtlasSegment_ConditionalTissueProbability(TissueProbability[i], InvSqrtDetLogCov[index],cY_M, LogMu[index],InvLogCov[index],NumInputImages, VirtualNumInputImages[i]); 
              // Just changed for speed (about 5% faster )
          w_m_output[index][j] = (float) (float) TissueProbability[i] * vtkImageEMGeneral::FastGaussMulti(InvSqrtDetLogCov[index],cY_M, LogMu[index],InvLogCov[index],NumInputImages, VirtualNumInputImages[i]); 
                
          normRow += w_m_output[index][j];
          index ++;
        }
      } 
    }
    if (normRow == 0.0) {normRow = 1;}
      }
      // Norm Weights
      for (i=0; i<  NumTotalTypeCLASS; i++) {
    w_m_output[i][j] /= normRow; 
      }
    } // End of < EMSEGMENT_DEFINED
    cY_M += NumInputImages;MapVector++;
    for (i=0; i < NumTotalTypeCLASS; i++) {
      w_m_input[i] ++;
      if (ProbDataPtr[i]) ProbDataPtr[i] ++;
    }
    j++;
    if (!(j % imgX)) {
      for (i=0; i < NumTotalTypeCLASS; i++) { if (ProbDataPtr[i]) ProbDataPtr[i] += ProbDataIncY[i];}
      if (!(j % imgXY)) {
         for (i=0; i <  NumTotalTypeCLASS; i++) { 
     if (ProbDataPtr[i]) ProbDataPtr[i] += ProbDataIncZ[i];}
      }
    }
  }

  //j = 0;
  //while (j < NumClasses) { 
  //      // Retrieve the vector with certain slice and class from w_m
  //      for (i = 0; i < imgXY; i++) weight[i] = w_m_output[j+i*NumClasses];
  //      j++;
  //      sprintf(varname,"weight(%d,:)",j);
  //      sprintf(filename,"EMAtlasSegmResult%dWeight%d.m",id,j);
  //      Test.WriteVectorToFile(filename,varname,weight,imgXY);
  //    }
  //vtkImageEMAtlasSegmenter test;
  //char filename[50];
  // sprintf(filename,"ProbDataPtr.%d",id);
  // test.WriteMRIfile(filename,NULL, 0,(short*)*w_m, 256*256);

  // Reset and Delete Variables
  // for (i=0; i< NumTotalTypeCLASS; i++) wm_input[i] = wm_input_start[i];
  // delete[] wm_input_start;
  delete[] wxp;delete[] wxn;delete[] wyn;delete[] wyp;delete[] wzn;delete[] wzp;
}

// -----------------------------------------------------------
// Calculate MF - parrallelised version 
// user interface when called with threads 
// -----------------------------------------------------------
void vtkImageEMAtlasSegmenter_MeanFieldApproximation3DThreadPrivate(void *jobparm) {  EMAtlas_MF_Approximation_Work_Private *job = (EMAtlas_MF_Approximation_Work_Private *)jobparm;
  // Define Type of ProbDataPtr
   switch (job->ProbDataType) {
    vtkTemplateMacro(vtkImageEMAtlasSegmenter_MeanFieldApproximation3DPrivate(job->id, job->w_m_input,job->MapVector,job->cY_M,job->imgX,job->imgY,
                       job->imgXY,job->StartVoxel,job->EndVoxel, job->NumClasses, job->NumTotalTypeCLASS, job->NumChildClasses, 
                       job->NumInputImages, job->Alpha,job->MrfParams, (VTK_TT**) job->ProbDataPtr, job->ProbDataIncY,
                       job->ProbDataIncZ, job->ProbDataWeight, job->ProbDataMinusWeight, job->LogMu,job->InvLogCov,
                       job->InvSqrtDetLogCov, job->TissueProbability, job->VirtualNumInputImages,job->w_m_output));

   }
}

// -----------------------------------------------------------
void vtkImageEMAtlasSegmenter::PrintIntermediateResultsToFile(int iter, float **w_m, short* ROI, unsigned char* OutputVector, int NumTotalTypeCLASS, int* NumChildClasses, 
                                vtkImageEMAtlasSuperClass* actSupCl, char* LevelName, void **ClassList, classType *ClassListType, int* LabelList, 
                                FILE** QualityFile) {
  // -----------------------------------------------------------
  // 1. Print out Weights
  // -----------------------------------------------------------

  // Check if anybody wants to print out weights
  int WeightFlag     = 0; 
  int NumClasses = actSupCl->GetNumClasses();
  { 
    int c = 0 ;
    while ((c < NumClasses) && (!WeightFlag)) {
      if (((ClassListType[c] == CLASS)      && (((vtkImageEMAtlasClass*) ClassList[c])->GetPrintWeights())) ||
      ((ClassListType[c] == SUPERCLASS) && (((vtkImageEMAtlasSuperClass*) ClassList[c])->GetPrintWeights())) ) WeightFlag = 1;
      c++;
    }
  }
  if (WeightFlag) {
    char *FileName = new char [strlen(this->PrintDir) + 29 + this->NumInputImages/10 + strlen(LevelName) +iter/10];
    // Just a dummy so it does not cut of the last directory ! => will create this->PrintDir if needed  
    sprintf(FileName,"%s/weights/blub",this->PrintDir);
    
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the follwoing directory :" << this->PrintDir << "/weights");
      return;
    } 
    cout << "Weights will be printed in directory "<< this->PrintDir << "/weights" << endl;
   
    int index =0;
    float *outputWeight = NULL;
    int PrintClassWeight;
    float *sumWeight = NULL;
    for ( int c = 0 ; c < NumClasses; c++) {
      PrintClassWeight = 0;
      // Define outputWeight
      if (ClassListType[c] == CLASS) {
    if (((vtkImageEMAtlasClass*) ClassList[c])->GetPrintWeights()) {
      outputWeight = w_m[index]; 
      PrintClassWeight = 1;
    }
    index ++;
      } else {
    if (((vtkImageEMAtlasSuperClass*) ClassList[c])->GetPrintWeights() ) {
      PrintClassWeight = 1;
      // Add the weights of all the substructures together
      sumWeight= new float[this->ImageProd]; 
      memcpy(sumWeight,w_m[index],sizeof(float)*this->ImageProd); 
      index ++;
      for (int i = 1 ; i < NumChildClasses[c]; i++) {
        for (int x = 0 ; x < this->ImageProd; x++) sumWeight[x] += w_m[index][x];
        index ++;
      }
      outputWeight = sumWeight; 
    } else {
      index += NumChildClasses[c];
    }
      }
      // Define Filename 
      sprintf(FileName,"%s/weights/EMWeightL%sI%dC%d",this->PrintDir,LevelName,iter,c);
      // Print out data 
      switch (((vtkImageEMAtlasClass*) ClassList[c])->GetPrintWeights()) {
         case 1:  
       vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(this,outputWeight,VTK_FLOAT,FileName);
       break;
         case 2:
       short *outputShortWeight = new short[this->ImageProd];
       for (int i = 0 ; i < this->ImageProd; i++) outputShortWeight[i] = short(1000.0 *outputWeight[i]); 
       vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(this,outputShortWeight,VTK_SHORT,FileName);
       delete[] outputShortWeight;
       break;
      }
      if (sumWeight) {
    delete[] sumWeight;
    sumWeight = NULL;
      }
    }
    delete[] FileName;
  }


  // -----------------------------------------------------------
  // Define Label Map 
  // -----------------------------------------------------------
  vtkImageData *LabelMap_WorkingExtension_Data  = NULL;
  short* LabelMap_WorkingExtension_Ptr = NULL;
 
  // Generate LabelMap if necessary
    if (actSupCl->GetPrintLabelMap() || QualityFile) {
      // Define Label Map with the exetensions defined by the SegmentationBoundary
      int Ext[6] = { 0, this->GetDimensionX()-1, 0, this->GetDimensionY()-1, 0, this->GetDimensionZ()-1};
      LabelMap_WorkingExtension_Data = vtkImageData::New(); 
      LabelMap_WorkingExtension_Ptr = (short*) vtkImageEMAtlasSegmenter_GetPointerToVtkImageData(LabelMap_WorkingExtension_Data ,VTK_SHORT,Ext);    
      this->DetermineLabelMap(LabelMap_WorkingExtension_Ptr, NumTotalTypeCLASS, NumChildClasses, actSupCl, ROI, this->ImageProd, w_m);
    }


 // -----------------------------------------------------------
  //  Print Label Map 
  // -----------------------------------------------------------
  if (actSupCl->GetPrintLabelMap()) {
    // Define File
    char FileName[250];
    cout << "Labelmap will be printed in directory "<< this->PrintDir << "/labelmaps" << endl;
    sprintf(FileName,"%s/labelmaps/EMLabelMapL%sI%d",this->PrintDir,LevelName,iter); 
    if (vtkFileOps::makeDirectoryIfNeeded(FileName) == -1) {
      vtkEMAddErrorMessage( "Could not create the directory :" << this->PrintDir << "/LabelMaps");
      return;
    }

    // Transfere it to output Extension - Do not use the same extension as for Dice measure bc of different extensions !
    vtkImageData *OriginalExtension_Data = vtkImageData::New();
    int ChangedExtent[6];

    memcpy(ChangedExtent, this->Extent, sizeof(int)*6); 
    if (!ChangedExtent[4]) {
      ChangedExtent[4] ++;
      ChangedExtent[5] ++;
    }
    short* OriginalExtension_DataPtr = (short*) vtkImageEMAtlasSegmenter_GetPointerToVtkImageData(OriginalExtension_Data,VTK_SHORT,ChangedExtent);
    vtkIdType outIncX, outIncY, outIncZ;
    OriginalExtension_Data->GetContinuousIncrements(ChangedExtent, outIncX, outIncY, outIncZ);
    int outInc[3] = {outIncX, outIncY, outIncZ};
    vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(this,LabelMap_WorkingExtension_Ptr,OriginalExtension_DataPtr ,outInc,0);
    this->GEImageWriter(OriginalExtension_Data,FileName,1);
    OriginalExtension_Data->Delete();
  }

  // -----------------------------------------------------------
  //  Print Quality Measure 
  // -----------------------------------------------------------
  if (QualityFile) {
    // -------------------------------------------------
    // Transfere it to output Extension
    vtkImageData *OriginalExtension_Data = vtkImageData::New();
    short *OriginalExtension_DataPtr = (short*) vtkImageEMAtlasSegmenter_GetPointerToVtkImageData(OriginalExtension_Data,VTK_SHORT,this->Extent);
    vtkIdType outIncX, outIncY, outIncZ;
    OriginalExtension_Data->GetContinuousIncrements(this->Extent, outIncX, outIncY, outIncZ);
    int outInc[3] = {outIncX, outIncY, outIncZ};
    vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(this,LabelMap_WorkingExtension_Ptr,OriginalExtension_DataPtr ,outInc,0);


    // Write into the file
    cout << "===================================================" << endl;
    int index = 0;
    for ( int c = 0 ; c < NumClasses; c++) {
      if (ClassListType[c] == CLASS) {
    int PrintQuality = ((vtkImageEMAtlasClass*) ClassList[c])->GetPrintQuality();    
     if  (PrintQuality &&  (QualityFile[PrintQuality -1])) {     
      vtkImageData *QualityReference = ((vtkImageEMAtlasClass*) ClassList[c])->GetReferenceStandard();
      switch (PrintQuality) {
      case 1: {
    int *SegmentationBoundaryMax = this->GetSegmentationBoundaryMax();
    int *SegmentationBoundaryMin = this->GetSegmentationBoundaryMin();

    int MinExtent[3] = {SegmentationBoundaryMin[0] -1, SegmentationBoundaryMin[1] -1, SegmentationBoundaryMin[2] -1 };
    int MaxExtent[3] = {SegmentationBoundaryMax[0] -1, SegmentationBoundaryMax[1] -1, SegmentationBoundaryMax[2] -1 };

    fprintf(QualityFile[PrintQuality -1],"%10f ",vtkImageEMGeneral::CalcSimularityMeasure(OriginalExtension_Data,QualityReference, LabelList[c],1, MinExtent, MaxExtent)); 

        break;
      }
        // Fill in other quality standards if needed
      }
      cout << endl;

      }
    // Fill the other spaces 
        index ++;
      } else {
         index += NumChildClasses[c];
      }
    }
    cout << "===================================================" << endl;
    for (int i = 0; i < EMSEGMENT_NUM_OF_QUALITY_MEASURE ; i++) if (QualityFile[i]) fprintf(QualityFile[i],"\n");
    OriginalExtension_Data->Delete();
  }

  if (LabelMap_WorkingExtension_Data) LabelMap_WorkingExtension_Data->Delete();
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasSegmenter::DetermineLabelMap(short* LabelMap, int NumTotalTypeCLASS, int* NumChildClasses, vtkImageEMAtlasSuperClass* head, short* ROI, int ImageMax, float **w_m) { 
  int    NumClasses        = head->GetNumClasses();
  void** ClassList         = head->GetClassList(); 
  classType* ClassListType = head->GetClassListType(); 
  short HeadLabel          = head->GetLabel();
  int    idx,l,k,MaxProbIndex, ClassIndex;
  float  MaxProbValue, temp;
  short *Label             = new short[NumClasses];
  float **w_mPtr = new float*[NumTotalTypeCLASS];
  for (l=0;l< NumTotalTypeCLASS;l++) w_mPtr[l] = w_m[l];

  for (idx = 0; idx <NumClasses; idx ++) {
    if (ClassListType[idx] == CLASS) Label[idx] = ((vtkImageEMAtlasClass*) ClassList[idx])->GetLabel();
    else Label[idx] = ((vtkImageEMAtlasSuperClass*) ClassList[idx])->GetLabel();
  }
  for (idx = 0; idx < ImageMax ; idx++) {
    if ((ROI == NULL) || (*ROI++ == HeadLabel)) {
      MaxProbValue = -1;
      MaxProbIndex = 0;
      ClassIndex = 0;
      for (l=0; l< NumClasses; l++) {
    temp = 0;
        // cout << " | ";
    for (k=0; k < NumChildClasses[l]; k++) {
      temp += *w_m[ClassIndex];
          // cout << *w_m[ClassIndex] << " " ;
          w_m[ClassIndex]++;
          ClassIndex ++;
    }
        // cout << endl;
    if (!(temp == temp)) {
      cout << "Error in index "<< idx << " Produced a nan " << endl;
      exit(1);
    }
          if ( temp > MaxProbValue) {MaxProbValue = temp; MaxProbIndex = l;}
      } 
      *LabelMap++ = Label[MaxProbIndex];
    } else {
      *LabelMap++ = 0;
      for (l=0;l< NumTotalTypeCLASS;l++) w_m[l] ++;
    } 
  }
  for (l=0;l< NumTotalTypeCLASS;l++) w_m[l] = w_mPtr[l];
  delete[] w_mPtr;
  delete[] Label;
 
}



//------------------------------------------------------------------------------
// Special Vector and Matrix functions
void vtkImageEMAtlasSegmenter::DeleteVariables() {
  if (this->PrintDir) delete[] this->PrintDir;

  this->NumInputImages = 0 ;
  this->PrintDir = NULL;this->activeSuperClass = NULL;this->activeClass = NULL;

}

// -----------------------------------------------------------
// Calculate MF - necessary for parrallising algorithm
// -----------------------------------------------------------
int vtkImageEMAtlasSegmenter::MF_Approx_Workpile(float **w_m_input,unsigned char* MapVector, float *cY_M, int imgXY,
                           double ***InvLogCov,double *InvSqrtDetLogCov,  int NumTotalTypeCLASS, 
                                                   int* NumChildClasses, int NumClasses, void** ProbDataPtr, int* ProbDataIncY, 
                           int* ProbDataIncZ, float *ProbDataWeight, float *ProbDataMinusWeight,
                                                   double** LogMu, double* TissueProbability, int *VirtualNumInputImages, vtkImageEMAtlasSuperClass* head, float **w_m_output) {
  #define MAXMFAPPROXIMATIONWORKERTHREADS 32
  int numthreads = 0;
  int jobsize,i,j;
  int StartIndex = 0;
  int StartPointer;
  int NumInputImages = this->NumInputImages;

  EMAtlas_MF_Approximation_Work_Private job[MAXMFAPPROXIMATIONWORKERTHREADS];

  // Sylvain did not like this because different results are produced on different machines - Kilian
  // I threw threaded function out - however to reduce labor I did not change the structure, which I should

  numthreads = 1;
  // numthreads = vtkMultiThreader::GetGlobalDefaultNumberOfThreads(); 
  // cout << "Debug: Kilian: Number of Threads: " <<   numthreads << endl;

  jobsize = this->ImageProd/numthreads;
  for (i = 0; i < numthreads; i++) {
    job[i].id                = i;
    job[i].w_m_input         = new float*[NumTotalTypeCLASS];
    for (j=0; j <NumTotalTypeCLASS; j++) job[i].w_m_input[j] = w_m_input[j]  + StartIndex;
    job[i].MapVector         = MapVector  + StartIndex;
    job[i].cY_M              = cY_M       + StartIndex*NumInputImages;
    job[i].imgX              = this->GetDimensionX();
    job[i].imgY              = this->GetDimensionY();
    job[i].imgXY             = imgXY;
    job[i].StartVoxel        = StartIndex;
    job[i].EndVoxel          = StartIndex + jobsize;
    job[i].NumClasses        = NumClasses;
    job[i].NumTotalTypeCLASS = NumTotalTypeCLASS;
    job[i].NumChildClasses   = NumChildClasses;
    job[i].NumInputImages    = NumInputImages;
    job[i].Alpha             = this->Alpha;
    job[i].MrfParams         = head->GetMrfParams();      
    // --- Begin ProbDataPtr Definition
    job[i].ProbDataType      = this->GetInput(0)->GetScalarType(); 
    job[i].ProbDataPtr       = new void*[NumTotalTypeCLASS];
 
    for (j=0; j <NumTotalTypeCLASS; j++) {
      StartPointer  = ((this->GetDimensionX() + ProbDataIncY[j])*this->GetDimensionY() + ProbDataIncZ[j])*(StartIndex/imgXY); // Z direction
      StartPointer += (this->GetDimensionX() + ProbDataIncY[j])*((StartIndex/this->GetDimensionX()) % this->GetDimensionY()); // Y direction
      StartPointer += StartIndex % this->GetDimensionX(); // X Direction 
      if (ProbDataPtr[j]) {
    switch (job[i].ProbDataType) {
          case VTK_DOUBLE:         job[i].ProbDataPtr[j] =  (void *) ((double *) ProbDataPtr[j] + StartPointer); break;
          case VTK_FLOAT:          job[i].ProbDataPtr[j] =  (void *) ((float *) ProbDataPtr[j] + StartPointer); break;
          case VTK_LONG:           job[i].ProbDataPtr[j] =  (void *) ((long *) ProbDataPtr[j] + StartPointer); break;
          case VTK_UNSIGNED_LONG:  job[i].ProbDataPtr[j] =  (void *) ((unsigned long *) ProbDataPtr[j] + StartPointer); break;
          case VTK_INT:            job[i].ProbDataPtr[j] =  (void *) ((int *) ProbDataPtr[j] + StartPointer); break;
          case VTK_UNSIGNED_INT:   job[i].ProbDataPtr[j] =  (void *) ((unsigned int *) ProbDataPtr[j] + StartPointer); break;
    case VTK_SHORT:            job[i].ProbDataPtr[j] =  (void *) ((short *) ProbDataPtr[j] + StartPointer); break;
          case VTK_UNSIGNED_SHORT: job[i].ProbDataPtr[j] =  (void *) ((unsigned short *) ProbDataPtr[j] + StartPointer); break;
          case VTK_CHAR:           job[i].ProbDataPtr[j] =  (void *) ((char *) ProbDataPtr[j] + StartPointer); break;
          case VTK_UNSIGNED_CHAR:  job[i].ProbDataPtr[j] =  (void *) ((unsigned char *) ProbDataPtr[j] + StartPointer); break;
    default:
      cout << "Execute: Unknown ScalarType" << endl;
      return 1;
    }
      } else {
    job[i].ProbDataPtr[j] = NULL;
      }    
    }               
    job[i].ProbDataIncY        = ProbDataIncY;
    job[i].ProbDataIncZ        = ProbDataIncZ;
    job[i].ProbDataWeight      = ProbDataWeight;
    job[i].ProbDataMinusWeight = ProbDataMinusWeight;
    // --- End ProbDataPtr Definition
    job[i].w_m_output            = new float*[NumTotalTypeCLASS];
    for (j=0; j <NumTotalTypeCLASS; j++) job[i].w_m_output[j] = w_m_output[j];
    job[i].LogMu                 = LogMu;
    job[i].InvLogCov             = InvLogCov;
    job[i].InvSqrtDetLogCov      = InvSqrtDetLogCov;
    job[i].TissueProbability     = TissueProbability;
    job[i].VirtualNumInputImages =  VirtualNumInputImages;
    StartIndex += jobsize;

    vtkImageEMAtlasSegmenter_MeanFieldApproximation3DThreadPrivate(&job[i]);
  }

  // Delete here
  for (i=0; i <numthreads; i++) {
    delete[] job[i].w_m_input;
    delete[] job[i].ProbDataPtr;        
    delete[] job[i].w_m_output;

  }
  return 0; /* Success */
}

template  <class Tin>
static void vtkImageEMAtlasAlgorithm(vtkImageEMAtlasSegmenter *self,Tin **ProbDataPtrStart, float** InputVector, short *ROI, EMTriVolume& iv_m, 
                     EMVolume *r_m, float **w_m, char *LevelName,  int &SegmentLevelSucessfullFlag) {
  cout << "vtkImageEMAtlasAlgorithm: Initialize Variables for " << LevelName <<  endl;

  SegmentLevelSucessfullFlag = 1;
  // ------------------------------------------------------
  // General Variables 
  // ------------------------------------------------------
  int i,j,k,l,m,n,x,y,z;
  int iter,regiter, index, lindex, mindex;

  // ------------------------------------------------------
  // Variables defined by current SuperClass and this Filter
  // ------------------------------------------------------
  int ImageProd             = self->GetImageProd();
  vtkImageEMAtlasSuperClass* actSupCl    = self->GetActiveSuperClass();
  void** ClassList          = actSupCl->GetClassList();
  classType* ClassListType  = actSupCl->GetClassListType();
  
  int NumClasses            = actSupCl->GetNumClasses();
  int NumInputImages        = self->GetNumInputImages();
  int NumTotalTypeCLASS     = actSupCl->GetTotalNumberOfClasses(false);

  int ImageMaxZ             = self->GetDimensionZ();
  int ImageMaxY             = self->GetDimensionY();
  int ImageMaxX             = self->GetDimensionX();

  int imgXY                 = ImageMaxY*ImageMaxX;

  // Kilian: Jan 06 : Now we can define iteration sequnce at each level of the hierarchy
  //int NumIter               = self->GetNumIter();
  //int NumRegIter            = self->GetNumRegIter();
  int NumIter               = actSupCl->GetStopEMMaxIter(); 
  int NumRegIter            = actSupCl->GetStopMFAMaxIter();
 
  double Alpha              = self->GetAlpha();

  int *ProbDataIncZ = new int[NumTotalTypeCLASS];           
  int *ProbDataIncY = new int[NumTotalTypeCLASS];       
  actSupCl->GetProbDataIncYandZ(ProbDataIncY,ProbDataIncZ,0);
  
  double *TissueProbability        = new double[NumClasses];
  float  *ProbDataWeight           = new float[NumClasses];
  float  *ProbDataMinusWeight      = new float[NumClasses];

  // if on of the weights is set to 0 the virtual dimension is one minus the real dimensio = NimInputImages -> necessary to calculate gaussian curve 
  // -> otherwise you get different reuslts from segmenting something with weight =(1,0) in comaprison to the equivalent one dimensional input channel segmentation 
  int *VirtualNumInputImages       = new int[NumClasses];
  float *InputChannelWeights;

  Tin **ProbDataPtrCopy = new Tin*[NumTotalTypeCLASS];
  for (i =0;i<NumTotalTypeCLASS;i++)  ProbDataPtrCopy[i] = ProbDataPtrStart[i]; 


  double **LogMu                   = new double*[NumTotalTypeCLASS];
  double ***LogCovariance          = new double**[NumTotalTypeCLASS];
 
  // double **WeightedLogMu           = new double*[NumTotalTypeCLASS];
  double ***InverseWeightedLogCov  = new double**[NumTotalTypeCLASS];
  double *InvSqrtDetWeightedLogCov = new double[NumTotalTypeCLASS];
  
  int NumberOfTrainingSamples       =  self->GetNumberOfTrainingSamples();
  int *LabelList                    = new int[NumTotalTypeCLASS];
  int *CurrentLabelList             = new int[NumClasses];

  // Define Files that should be printed in 
  for (i = 0; i < NumClasses; i ++) {
    if (ClassListType[i] == CLASS) {
      CurrentLabelList[i] = ((vtkImageEMAtlasClass*) ClassList[i])->GetLabel();
    } else {
      CurrentLabelList[i] = ((vtkImageEMAtlasSuperClass*) ClassList[i])->GetLabel();
    }
  }

  // ------------------------------------------------------
  // Variables defined by Subclasses
  // ------------------------------------------------------
  for (i = 0; i < NumTotalTypeCLASS; i++) {
    InverseWeightedLogCov[i]  = new double*[NumInputImages];
    for (x = 0; x < NumInputImages; x++)     InverseWeightedLogCov[i][x]  = new double[NumInputImages];
  }

  // Kilian Get the LogCovariance - change everyhting from here 
  actSupCl->GetTissueDefinition(LabelList,LogMu, LogCovariance,0);

  classType *ClassType = new classType[NumClasses] ;
  // Number of child classes => if class is of Type CLASS => is set to 1
  // otherwise it is ther number of children of TYPE CLASS
  int *NumChildClasses = new int[NumClasses];

  // VirtualOveralInputChannelFlag[i] = 0 => input channel i is not used by any class 
  // VirtualOveralInputChannelFlag[i] = 1 => input channel i is used by at least one class 
  int *VirtualOveralInputChannelFlag = new int[NumInputImages];
  memset(VirtualOveralInputChannelFlag, 0, sizeof(int)*NumInputImages);    

  // VirtualOveralInputChannelNum defines the number of real input channels analyzed by all tissue classes together 
  // if VirtualOveralInputChannelNum < NumInputChannel => at least one input channel is not used by any tissue class

  int VirtualOveralInputChannelNum = 0;
  index = 0;
  for (i=0; i < NumClasses; i++) {
    if (ClassListType[i] == CLASS) {
      NumChildClasses[i]   = 1;
      TissueProbability[i]  = ((vtkImageEMAtlasClass*) ClassList[i])->GetTissueProbability();

      ProbDataWeight[i]     = ((vtkImageEMAtlasClass*) ClassList[i])->GetProbDataWeight();      
      ProbDataMinusWeight[i] =  float(NumberOfTrainingSamples)*(1.0 - ProbDataWeight[i]);

      InputChannelWeights = ((vtkImageEMAtlasClass*) ClassList[i])->GetInputChannelWeights();
 
      if (EMAtlasSegment_CalcWeightedCovariance(self,InverseWeightedLogCov[index], InvSqrtDetWeightedLogCov[index], InputChannelWeights, LogCovariance[index], VirtualNumInputImages[i],NumInputImages) == 0) {
    vtkEMAddErrorMessageSelf("vtkImageEMAtlasAlgorithm: weighted covariance has a non positive determinante  for class with index "<< index << ". See shell for more specific output!");
    cout << "InvSqrtDetWeightedLogCov: " << InvSqrtDetWeightedLogCov[index] << " VirtualNumInputImages: " <<  VirtualNumInputImages[i] << endl  << "LogCovariance :";  
    for (x =0 ; x <NumInputImages; x ++ ) {
      for (y =0 ; y <NumInputImages; y ++ ) cout << LogCovariance[index][x][y] << " ";
      cout << " | ";
    }
    cout << endl << "Weighted LogCovariance : "; 
    for (x =0 ; x <NumInputImages; x ++ ) {
      for (y =0 ; y <NumInputImages; y ++ ) cout << LogCovariance[index][x][y]*InputChannelWeights[x]*InputChannelWeights[y] << " ";
      cout << " | ";
    }
         
        cout << endl;  
 
    SegmentLevelSucessfullFlag = 0;
      }

#if (EMVERBOSE)
      cout << "=========== "<< index << " =============" << endl;
      cout << "InvSqrtDetWeightedLogCov: " << InvSqrtDetWeightedLogCov[index] << " VirtualNumInputImages: " <<  VirtualNumInputImages[i] << endl  << "InverseWeightedLogCov :";  ;
      for (x =0 ; x <NumInputImages; x ++ ) {
    for (y =0 ; y <NumInputImages; y ++ ) fprintf(stdout, "%8.5f ", InverseWeightedLogCov[index][x][y]);
    cout << " | ";
      }
      cout << endl;
#endif
      index ++;

    } else  {
      NumChildClasses[i]   = ((vtkImageEMAtlasSuperClass*) ClassList[i])->GetTotalNumberOfClasses(false);
      TissueProbability[i] = ((vtkImageEMAtlasSuperClass*) ClassList[i])->GetTissueProbability();
      ProbDataWeight[i]    = ((vtkImageEMAtlasSuperClass*) ClassList[i])->GetProbDataWeight();
      if (NumChildClasses[i]) ProbDataMinusWeight[i] =  float(NumberOfTrainingSamples)* (1.0 - ProbDataWeight[i])/float(NumChildClasses[i]);
      else ProbDataMinusWeight[i] = 0.0;

      InputChannelWeights = ((vtkImageEMAtlasSuperClass*) ClassList[i])->GetInputChannelWeights();

      for (k = 0;k < NumChildClasses[i]; k++) {
    if (EMAtlasSegment_CalcWeightedCovariance(self,InverseWeightedLogCov[index], InvSqrtDetWeightedLogCov[index], InputChannelWeights, LogCovariance[index], VirtualNumInputImages[i],NumInputImages) == 0) {
          vtkEMAddErrorMessageSelf("vtkImageEMAtlasAlgorithm: weighted covariance has a non positive determinante  for class with index "<< index << ". See shell for more specific output!");
          cout << "InvSqrtDetWeightedLogCov: " << InvSqrtDetWeightedLogCov[index] << " VirtualNumInputImages: " <<  VirtualNumInputImages[i] << endl  << "LogCovariance :";  
          for (x =0 ; x <NumInputImages; x ++ ) {
         for (y =0 ; y <NumInputImages; y ++ ) cout << LogCovariance[index][x][y] << " ";
          cout << " | ";
           }
      cout << endl << "Weighted LogCovariance : "; 
          for (x =0 ; x <NumInputImages; x ++ ) {
         for (y =0 ; y <NumInputImages; y ++ ) cout << LogCovariance[index][x][y]*InputChannelWeights[x]*InputChannelWeights[y] << " ";
          cout << " | ";
           }
           
          cout << endl;  
      SegmentLevelSucessfullFlag = 0;
    }
#if (EMVERBOSE)
    cout << "=========== "<< index << " =============" << endl;
    cout << "InvSqrtDetWeightedLogCov: " << InvSqrtDetWeightedLogCov[index] << " VirtualNumInputImages: " <<  VirtualNumInputImages[i] << endl << "InverseWeightedLogCov: ";  
    for (x =0 ; x <NumInputImages; x ++ ) {
      for (y =0 ; y <NumInputImages; y ++ ) fprintf(stdout, "%8.5f ", InverseWeightedLogCov[index][x][y]);
      cout << " | ";
    }
    cout << endl;
#endif
    index ++; 
      }
    }


    for (k= 0; k < NumInputImages; k++) { 
       if (InputChannelWeights[k] > 0.0) VirtualOveralInputChannelFlag[k] = 1; 
    }
  } 

  for (k= 0; k < NumInputImages; k++) { 
    if (VirtualOveralInputChannelFlag[k]) VirtualOveralInputChannelNum ++;
  }


  int SmoothingWidth        = self->GetSmoothingWidth();
  int SmoothingSigma        = self->GetSmoothingSigma();
  // Bias Information
  int BiasPrint             = actSupCl->GetPrintBias();

  // Kilian: Jan06: InitialBias_FilePrefix allows initializing a bias field with a precomputed from outside 
  // - carefull Bias Field has to be in little Endian                
  char *InitialBias_FilePrefix = actSupCl->GetInitialBiasFilePrefix();

  // -----------------------------------------------------------
  // Print Quality Measure  Setup 
  // -----------------------------------------------------------
  int QualityFlag = 0;
  FILE **QualityFile  = NULL; 
 
  {
    i = 0;
    while ((i < NumClasses) && !QualityFlag) {
      if ((ClassListType[i]== CLASS) && (((vtkImageEMAtlasClass*) ClassList[i])->GetPrintQuality())) QualityFlag = 1;
      i++;
    }
  }
  if (QualityFlag) {
    QualityFile = new FILE*[EMSEGMENT_NUM_OF_QUALITY_MEASURE];
    char FileName[250]; 
    for (i = 0; i < EMSEGMENT_NUM_OF_QUALITY_MEASURE ; i++) QualityFile[i] = NULL;
    for (int c = 0; c < NumClasses; c++) {
      if (ClassListType[c]== CLASS) {
    int PrintQuality = ((vtkImageEMAtlasClass*) ClassList[c])->GetPrintQuality();
    if  (PrintQuality  && (!QualityFile[PrintQuality -1])) { 
      switch (((vtkImageEMAtlasClass*) ClassList[c])->GetPrintQuality ()) {
        case 1 :  sprintf(FileName,"%s/DiceL%s.txt",self->GetPrintDir(),LevelName); break;
          // Later fill in the names of all the other quality measures 
      }
      QualityFile[PrintQuality-1] = fopen(FileName, "w");
      if (QualityFile[PrintQuality-1] == NULL ) {
        vtkEMAddErrorMessageSelf("Could not open file " << FileName);
        return;
      }
      cout << "Quality Parameters will be saved in : " << FileName << endl;
      for (k= 0 ; k < NumClasses ; k++) 
        if ((ClassListType[k]== CLASS) && (((vtkImageEMAtlasClass*) ClassList[k])->GetPrintQuality() == PrintQuality) )
          fprintf(QualityFile[PrintQuality-1],"     %2d    ", LabelList[k]);
      fprintf(QualityFile[PrintQuality-1],"\n"); 
    }
      }
    }
  }

  // ------------------------------------------------------
  // Setting up Class Distribution 
  // Calculate the mean of the log(greyvalue+1), 
  // currently we have the mean over the grey scale values !
  // ------------------------------------------------------

  // ------------------------------------------------------------
  // EM-MF Variables 
  // ------------------------------------------------------------
  // bias for emseg+mrf - got rid of it to save memory 
  // double *b_m = new double[NumInputImages*ImageProd], *b_mPtr = b_m; // dimension NumInputImages x ImageProd 
  // memset(b_m[i], 0, sizeof(double)*NumInputImages*ImageProd);
  double b_m;
  double ConditionalTissueProbability;
  double SpatialTissueDistribution;

  float **w_mPtr        = new float*[NumTotalTypeCLASS];
  // Needed for Parallelising MF-Approximation
  float **w_m_second    = new float*[NumTotalTypeCLASS];
  float **w_m_secondPtr = new float*[NumTotalTypeCLASS];
  for (i=0; i<NumTotalTypeCLASS; i++) {
    w_mPtr[i] = w_m[i]; // Result of Weights after trad. E Step -  dimesion NumClasses x ImageProd
    w_m_second[i] = new float[ImageProd];
    w_m_secondPtr[i] = w_m_second[i]; 
  }

  float *cY_M = new float[NumInputImages* ImageProd], *cY_MPtr = cY_M; // cY_M correct log intensity - dimension NumInputImages x ImageProd

  unsigned char *OutputVector = new unsigned char[ImageProd];
  unsigned char *OutputVectorPtr = OutputVector;
  short *ROIPtr = ROI;
  float       **InputVectorPtr = InputVector;
  
  int ProbValue;
  float normRow;   

  memset(OutputVector, 0, ImageProd*sizeof(unsigned char));
  // EMSEGMENT_DEFINED = this is an already defined voxel from previous segmentation
  //                     or only one tissue class has acceptable values (see later)   
  if (ROI) {
    // 1.) Check what the region of interest is 
    for (i=0; i < ImageProd; i++) {
      if ((*ROI++) != actSupCl->GetLabel()) *OutputVector |= EMSEGMENT_NOTROI; 
      OutputVector ++;
    }
    OutputVector = OutputVectorPtr; ROI = ROIPtr;
  }
  // 2.) Check every voxel if it has a defined neighbor or an edge
  for (i=0; i < ImageProd; i++) {
    if (*OutputVector < EMSEGMENT_NOTROI) {
      if (i % ImageMaxX)                  {
           if (OutputVector[-1]&EMSEGMENT_NOTROI)          *OutputVector |= EMSEGMENT_NORTH;
      } else { *OutputVector |= EMSEGMENT_NORTH;}

      if ((i+1) % ImageMaxX)              {if (OutputVector[1]&EMSEGMENT_NOTROI)           *OutputVector |= EMSEGMENT_SOUTH;
      } else { *OutputVector |= EMSEGMENT_SOUTH;}
             
      if (int(i/ImageMaxX)%ImageMaxY)     {if  (OutputVector[-ImageMaxX]&EMSEGMENT_NOTROI) *OutputVector |= EMSEGMENT_WEST;
      } else { *OutputVector |= EMSEGMENT_WEST;}
  
      if (int(i/ImageMaxX + 1)%ImageMaxY) {if (OutputVector[ImageMaxX]&EMSEGMENT_NOTROI)   *OutputVector |= EMSEGMENT_EAST;
      } else { *OutputVector |= EMSEGMENT_EAST;}

      if (int(i/imgXY))                   {if (OutputVector[-imgXY]&EMSEGMENT_NOTROI)      *OutputVector |= EMSEGMENT_FIRST;
      } else { *OutputVector |= EMSEGMENT_FIRST;}

      if (int(i/imgXY +1)% ImageMaxZ )    {if (OutputVector[+imgXY]&EMSEGMENT_NOTROI)      *OutputVector |= EMSEGMENT_LAST;
      } else { *OutputVector |= EMSEGMENT_LAST;}
    }
    OutputVector ++;
  }
  OutputVector = OutputVectorPtr;

  // ------------------------------------------------------------
  // M Step Variables 
  // ------------------------------------------------------------
  double lbound = (-(SmoothingWidth-1)/2); // upper bound = - lbound
  float temp;
  float *skern = new float[SmoothingWidth];
  // Kilian change to normal gaussian 
  for (i=0; i < SmoothingWidth; i++) skern[i] = float(vtkImageEMGeneral::FastGauss(1.0 / SmoothingSigma,i + lbound));

  double **iv_mat     = new double*[VirtualOveralInputChannelNum];
  double **inv_iv_mat = new double*[VirtualOveralInputChannelNum];
  for (i=0; i < VirtualOveralInputChannelNum; i++) {
    iv_mat[i]     = new double[VirtualOveralInputChannelNum];
    inv_iv_mat[i] = new double[VirtualOveralInputChannelNum];
  }

   if (SegmentLevelSucessfullFlag) {
    // ------------------------------------------------------------
    // Start Algorithm 
    // ------------------------------------------------------------
    for (iter=1; iter <= NumIter;iter++){
      // -----------------------------------------------------------
      // Intensity Correction 
      // -----------------------------------------------------------
      // cout << "Iter " << iter << endl;
      // estimate the smoothed image inhomoginities (bias + resiuduum)
      // transform r (smoothed weighted residuals) by iv (smoother inv covariances)
      // b_m = r_m./iv_m ;
      // Bias can only be calculated in the top level => The rest we leave out
      // Kilian - Jan 06 : changed this line bc we now calculate iv_m anr r_m even if ROI = NULL and NumIter = 1  
      // Now the exception is called if iter == 0 and InitialBias_FilePrefix is defined or ROI is not defined 
      if (((ROI) && (!InitialBias_FilePrefix)) || (iter > 1)) {
          // cout << "-------------------- Bias Field Correction -------------------" << endl; 
          // Kilian - Jan 06 : changed this line bc Bias is also defined in areas outside the ROI 
          char** BiasFileName       = NULL;
          double* BiasSlice         = NULL;
          double* BiasSlicePtr      = NULL;
          bool PrintBiasFlag = bool((iter ==  NumIter) && BiasPrint);

          if (PrintBiasFlag) {

        char *BiasDirectory = new char[250];
        if (self->GetPrintDir() != NULL) sprintf(BiasDirectory,"%s/Bias/blub",self->GetPrintDir());
        else sprintf(BiasDirectory,"Bias/blub");
        
        if (vtkFileOps::makeDirectoryIfNeeded(BiasDirectory) == -1) {
          vtkEMAddErrorMessageSelf( "Could not create the directory :" << self->GetPrintDir() << "/Bias");
          PrintBiasFlag = 0;
        } else {
          cout << "vtkImageEMAtlasAlgorithm: Print Bias (Type: Double) to " << self->GetPrintDir() << "/Bias" << endl;

          BiasSlicePtr = BiasSlice = new double[ImageProd*NumInputImages];
          memset(BiasSlice, 0, sizeof(double)*ImageProd*NumInputImages);
 
          BiasFileName = new char*[NumInputImages];
          for (int i = 0 ; i < NumInputImages; i++)  BiasFileName[i] = new char[250];  
          
          int *SegmentationBoundaryMin = self->GetSegmentationBoundaryMin();
          int *SegmentationBoundaryMax = self->GetSegmentationBoundaryMax();
          int *Extent               = self->GetExtent();
          for (int l=0; l< NumInputImages; l++) {
        if (self->GetPrintDir()) sprintf(BiasFileName[l],"%s/Bias/BiasL%sCh%d",self->GetPrintDir(),LevelName,l);
        else sprintf(BiasFileName[l],"Bias/BiasL%sCh%d",LevelName,l);
        
        // If BoundaryMin and Max do not span full length we have to add empty slices 
        for (int i = 1; i < SegmentationBoundaryMin[2]; i++) vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(self,BiasSlice,VTK_DOUBLE,BiasFileName[l],i-SegmentationBoundaryMin[2],0,0);
        for (int i = 1; i <= Extent[5]- Extent[4] + 1 - SegmentationBoundaryMax[2]; i++) vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(self,BiasSlice,VTK_DOUBLE,BiasFileName[l],i-SegmentationBoundaryMin[2],0,0);
          }
        }
        delete[] BiasDirectory;
      }

          for (i = 0; i<ImageMaxZ;i++) {
            // Define Bias Value
        for (j = 0; j<ImageMaxY;j++){
          for (k = 0; k<ImageMaxX;k++){
            if (*OutputVector++ < EMSEGMENT_NOTROI) {
                  lindex =0;
          for (l=0; l< VirtualOveralInputChannelNum ; l++) {
                    while (!VirtualOveralInputChannelFlag[lindex]) lindex ++; 
            iv_mat[l][l] = iv_m(lindex,lindex,i,j,k);
                    mindex = 0;
            for (m = 0; m<= l; m++) {
              while (!VirtualOveralInputChannelFlag[mindex]) mindex ++; 
              iv_mat[m][l] = iv_mat[l][m] = iv_m(lindex,mindex,i,j,k);
                      mindex ++;
            }
                    lindex ++;
          }
          if (vtkImageEMGeneral::InvertMatrix(iv_mat, inv_iv_mat,VirtualOveralInputChannelNum)) {
            lindex = 0;
            for (l=0; l< NumInputImages; l++) {
              b_m = 0.0;
              if (VirtualOveralInputChannelFlag[l]) {
                mindex = 0;
                for (m = 0; m< NumInputImages; m++) {
                if (VirtualOveralInputChannelFlag[m]) {
                  b_m += inv_iv_mat[lindex][mindex]*r_m[m](i,j,k);
                  mindex ++;
                }
              }
              lindex ++;
              (*cY_M ++) = fabs((*InputVector)[l] - b_m);
              if (BiasSlice) (*BiasSlice ++) = b_m;
              } else {
                cY_M ++;
                if (BiasSlice) BiasSlice ++;
              }
            }
          } else { 
            for (l=0; l< NumInputImages; l++) {
          (*cY_M ++) = fabs((*InputVector)[l]);
          if (BiasSlice) (*BiasSlice ++) = 0.0;
            }
          }
        } else {
        // Kilian - Jan 06 Now we can print out the bias even if ROI != NULL ! 
    if (BiasSlice) {
      for (l=0; l< NumInputImages; l++) (*BiasSlice ++) = 0.0;
    }
        cY_M += NumInputImages;
    }
        InputVector++;
      }
        }
         // Print Bias Field if necessary  
       if (PrintBiasFlag) {
      double *BiasSliceInput = new double[ImageProd];
      for (int l=0; l< NumInputImages; l++) {
         BiasSlice = BiasSlicePtr + l;

         for (int m = 0 ; m < imgXY; m ++) {
         BiasSliceInput[m] = *BiasSlice;
         BiasSlice += NumInputImages;
         }
         // Remember for windows always use - BiasFile = fopen(BiasFileName, "wb") - otherwise does not work for double or float    
         vtkImageEMAtlasSegmenter_PrintDataToOutputExtension(self,BiasSliceInput,VTK_DOUBLE,BiasFileName[l],i+1,0,0);
       }
       delete[] BiasSliceInput;
    }
    BiasSlice = BiasSlicePtr;



      } // End of Z

  if (BiasSlice) delete[] BiasSlice;


  if (BiasFileName) {
    for (int i = 0; i < NumInputImages; i++)  delete[] BiasFileName[i];
    delete[] BiasFileName;
  }

        } else {
      // Option 1. : Bias Field is initialized with outside source
      if (InitialBias_FilePrefix) {
        cout << "Initializing Bias field with volumes defined by " << InitialBias_FilePrefix << endl;
        // Make sure that the bias field are written with little Endian
            // The initial volumes have to defined as following <>Ch%d.%03d , 
        // e.g. InitialBias_FilePrefix = "Bias_1.0" => For first input channel Volume has to exists with the name Bias_1.0Ch0.*
        char *PrefixName = new char[int(strlen(InitialBias_FilePrefix)) + 5];
    int ChangedExtent[6] = {self->GetExtent()[0],self->GetExtent()[1],self->GetExtent()[2],self->GetExtent()[3], self->GetExtent()[4] + 1 ,self->GetExtent()[5] + 1 };
    cout <<"Loading initial Image Inhomogoneity setting from slices " << ChangedExtent[4] << " to " << ChangedExtent[5] << endl;
        for (j = 0 ; j < NumInputImages; j ++ ) {
           vtkImageReader  *InitialBias = vtkImageReader::New(); 
           sprintf(PrefixName,"%sCh%d",InitialBias_FilePrefix,j);

           self->GEImageReader(InitialBias,PrefixName,ChangedExtent[4],ChangedExtent[5],VTK_DOUBLE);
           double* InitialBiasPtr = (double*) self->GetPointerToVtkImageData(InitialBias->GetOutput(),VTK_DOUBLE,ChangedExtent);
       cout << "Initializing Image inhomogeneity for channel " << j << endl;
           cY_M += j;
           for (i=0; i< ImageProd; i++) {
             *cY_M = fabs((*InputVector)[j] - *InitialBiasPtr);
             InputVector ++;
             InitialBiasPtr ++;
             cY_M += NumInputImages;
           }
           InitialBias->Delete();
           cY_M = cY_MPtr;InputVector = InputVectorPtr;
        }
        delete []PrefixName;
        
      } else {
        // The first time !
        // cY_M  = fabs(InputVector - b_m) = {b_m ==0} = fabs(InputVector) = InputVector;
        // we assume InputVector >= 0
        for (i=0; i< ImageProd; i++) {
          // Kili change it if you have too
          memcpy(cY_M,(*InputVector),sizeof(float)*NumInputImages);
          cY_M += NumInputImages;
          InputVector++;
        }
      }
      // Kilian - Jan 06 : Make sure the case is covered if ROI == NULL and NumIter = 1 
      //                   => iv_m and r_m have to be defined for next hierarchical segmentation
      if (NumIter == 1) {
        iv_m.SetValue(0.0); 
        for (m=0; m<NumInputImages; m++) r_m[m].SetValue(0.0);
      }
          // cout << "-------------------- Bias Field Correction -------------------" << endl; 
 

        }
  
        cY_M = cY_MPtr;OutputVector = OutputVectorPtr;InputVector = InputVectorPtr;
  
    // -----------------------------------------------------------
    // E-Step
    // -----------------------------------------------------------
    // cY_m = abs(Y - b_m(:));  -> corrected log intensities we will do this now at the end of the M-Step to save memory 
    // Ininitialize first iteration
    // This is the EM Algorithm with out MF Part -> The Regulizing part is the part where 
    // the MF part is added
  
    cout << "vtkImageEMAtlasAlgorithm: "<< iter << ". Estep " << endl;
    if ((iter == 1) || (Alpha == 0)) { 
      for (z = 0; z < ImageMaxZ ; z++) {
        for (y = 0; y < ImageMaxY ; y++) {
          for (x = 0; x < ImageMaxX ; x++) {
#if (EMVERBOSE)
        cout << "============================================================" << endl;
        cout << "Z:" << z << "Y:" << y << "X:" << x << "  cY_M: "; 
        for (l= 0 ; l <  NumInputImages; l ++) {
          cout << cY_M[l] << " " ;
        }
        
        fprintf(stdout, "\nw_m               class SubCl ClaIdx  PrbMinus  PWeight   PrbData  TisProb  IntensProb\n"); 
#endif
        if (*OutputVector < EMSEGMENT_NOTROI) {
          normRow = 0;
          ProbValue = -2;
          index = 0;
          for (i=0; i < NumClasses; i++) {
            
            for (k=0;k< NumChildClasses[i];k++) {
              // check down a little bit later
              // (*w_m)[j] = (*ProbabilityData)[j]*vtkImageEMGeneral::LookupGauss(GaussLookupTable[j],TableLBound[j],TableUBound[j],TableResolution[j],*cY_M, NumInputImages);
              // (*w_m)[j] = ProbabilityData[i][j]*vtkImageEMGeneral::FastGaussMulti(InvSqrtDetWeightedLogCov[j],*cY_M, LogMu[j],InvLogCov[j],NumInputImages);
              // If we have a superclass we have to calculate differently!
              // The proability atlas of a super class is the sum of all its sub classes 
              // Atlas[SC] = 1 - weight + weight * sum_{sub\in SC}(Atlas[sub])
              //           = sum_{sub\in SC}((1 - weight) / (# of Sub)  + weight * Atlas[sub])
              ConditionalTissueProbability =  TissueProbability[i]* vtkImageEMGeneral::FastGaussMulti(InvSqrtDetWeightedLogCov[index],cY_M, LogMu[index],
                                                          InverseWeightedLogCov[index],NumInputImages, VirtualNumInputImages[i]);
              
              // Work of MICCAI02 and ISBI04 
              SpatialTissueDistribution    = ProbDataMinusWeight[i]  + ProbDataWeight[i] *float(ProbDataPtrCopy[index] == NULL ? 0.0 : *ProbDataPtrCopy[index]);
  
              *w_m[index] = (float)  ConditionalTissueProbability * SpatialTissueDistribution; 
              normRow += *w_m[index];
#if (EMVERBOSE)
              fprintf(stdout, "w_m: %12g i:%2d  k:%2d  ind:%2d  PMW:%3.2f  PDW:%3.2f  PDP:", *w_m[index],i,k,index,ProbDataMinusWeight[i], ProbDataWeight[i]); 
              if (ProbDataPtrCopy[index] == NULL) cout << "NULL ";
              else  fprintf(stdout, "%3d ", *ProbDataPtrCopy[index]);
              fprintf(stdout, " TP:%3.2f  GID:%5.4f \n", TissueProbability[i], 
                  (float)vtkImageEMGeneral::FastGaussMulti(InvSqrtDetWeightedLogCov[index], cY_M, LogMu[index],InverseWeightedLogCov[index],
                                       NumInputImages, VirtualNumInputImages[i]));
#endif
              // Predefine those areas where only one Atlas map has a value
              if (ProbValue != -1) {
            if ((ProbDataPtrCopy[index] && (*ProbDataPtrCopy[index] > 0)) || (ProbDataWeight[i] < 1.0)) {
              if (ProbValue == -2) ProbValue = i; //no other class has (*ProbDataPtrCopy[j]) > 0 so far
              else if (ProbValue !=i) ProbValue = -1;                //more than one other class has (*ProbDataPtrCopy[j]) > 0 so far
            } 
              }
              index ++;
            }
          }
        }
        // There is a problem if normRow is 0 -> happens quite frequently with local priors
        // Just assing it to local prior for inital setting. The second term is not of such great value
        // because we do not know anything about bias -> Probability Map is more precise
  #if (EMVERBOSE)
        fprintf(stdout, "normRow: %8g \n",normRow);
  #endif
        if (normRow == 0.0) {
          index = 0;
          for (i=0; i < NumClasses; i++) {
            for (k=0;k< NumChildClasses[i];k++) {
              if (ProbDataPtrCopy[index]) 
            *w_m[index] += ProbDataMinusWeight[i] + ProbDataWeight[i] * float(*ProbDataPtrCopy[index]);
              else if (ProbDataWeight[i] > 0.0) *w_m[index] += ProbDataMinusWeight[i];
              normRow += *w_m[index];
              index ++; 
            }
          }
          if (normRow == 0.0) {
            index = 0;
            for (j=0; j <  NumClasses ; j++) {
              for (k=0;k< NumChildClasses[j];k++) { 
            // The line below is the same as ConditionalTissueProbability is the same as 
            (float) vtkEMAtlasSegment_ConditionalTissueProbability(TissueProbability[j], InvSqrtDetWeightedLogCov[index],cY_M, LogMu[index],
                                         InverseWeightedLogCov[index],NumInputImages, VirtualNumInputImages[j]);   
            *w_m[index] = (float) (float) TissueProbability[j]* vtkImageEMGeneral::FastGaussMulti(InvSqrtDetWeightedLogCov[index],cY_M, LogMu[index],
                                                          InverseWeightedLogCov[index],NumInputImages, VirtualNumInputImages[j]); 
            normRow += *w_m[index];
            index ++;
              }
            }
          }
          if (normRow == 0.0) {normRow = 1;}
        }
        // Normalize Rows and find Maxium of every Row and write it in Matrix        
        if (ProbValue > -2) {
          for (j=0; j < NumTotalTypeCLASS; j++) *w_m[j] /= normRow;
          // Label for this pixel ia lready defined
          if (ProbValue > -1) {
            // Has to be done that way otherwise w_m_second is not defined at those places and that is deadramental in the M Step (think about w_m[-ImgY])
            *OutputVector |= EMSEGMENT_DEFINED;
            for (j=0; j < NumTotalTypeCLASS; j++) *w_m_second[j] = *w_m[j];
          }
          
        } else { 
          // Probabilityt Map is 0 for every class -> Label is assigned to first class
          **w_m = **w_m_second = 1.0;
          for (j=1; j < NumTotalTypeCLASS; j++) *w_m[j] = *w_m_second[j] = 0.0;
          
          // *OutputVector = (int) Label[0]; 
          *OutputVector |= EMSEGMENT_DEFINED; 
        }
        cY_M += NumInputImages; OutputVector++;
        for (j=0; j < NumTotalTypeCLASS; j++) { 
          w_m[j] ++; w_m_second[j]++; 
          if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] ++;
        }
          }
          for (j=0; j < NumTotalTypeCLASS; j++) {
        if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] += ProbDataIncY[j];
          }
        }
        for (j=0; j < NumTotalTypeCLASS; j++) {
          if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] += ProbDataIncZ[j];
        }
      } // End of for (z = 0; z < ImageMaxZ ; z++) 

      cY_M = cY_MPtr;OutputVector = OutputVectorPtr;
      for (j=0; j < NumTotalTypeCLASS; j++) {
        w_m[j]                         = w_mPtr[j];
        w_m_second[j]                  = w_m_secondPtr[j];
        ProbDataPtrCopy[j]             = ProbDataPtrStart[j];
      }
    }   // End of if ((iter == 1) || (Alpha == 0)) ...  
    // -----------------------------------------------------------
    // regularize weights using a mean-field approximation
    // -----------------------------------------------------------
    if (Alpha > 0) {
      for (regiter=1; regiter <= NumRegIter; regiter++) {
        cout << "vtkImageEMAtlasAlgorithm: "<< regiter << ". EM - MF Iteration" << endl;
        if (regiter%2) self->MF_Approx_Workpile(w_m,OutputVector,cY_M,imgXY,InverseWeightedLogCov,InvSqrtDetWeightedLogCov,NumTotalTypeCLASS,
                            NumChildClasses,NumClasses,(void**) ProbDataPtrStart, ProbDataIncY,ProbDataIncZ,
                            ProbDataWeight, ProbDataMinusWeight, LogMu,TissueProbability,VirtualNumInputImages, actSupCl, w_m_second);
        else self->MF_Approx_Workpile(w_m_second,OutputVector,cY_M,imgXY,InverseWeightedLogCov,InvSqrtDetWeightedLogCov,NumTotalTypeCLASS,NumChildClasses,
                      NumClasses,(void**) ProbDataPtrStart,ProbDataIncY,ProbDataIncZ,ProbDataWeight,
                      ProbDataMinusWeight, LogMu,TissueProbability, VirtualNumInputImages, actSupCl, w_m);
      }
      // if it is an odd number of iterations w_m_second holds the current result ! -> Therefore we have to change it !
      if (NumRegIter%2) {
        for (j=0; j < NumTotalTypeCLASS; j++) memcpy(w_m[j],w_m_second[j],sizeof(float)*ImageProd);
      }
    }
#if (EMVERBOSE)
    cout << "End of E-Step with MFA " << endl;    
#endif   
    // -----------------------------------------------------------
    // M-step
    // -----------------------------------------------------------
    if (iter < NumIter) {
      cout << "vtkImageEMAtlasAlgorithm: Determine Image Inhomogeneity" << endl;
      // cout << "vtkImageEMAtlasAlgorithm: Mstep " << endl;
      // compute weighted residuals 
      // r_m  = (w_m.*(repmat(cY_M,[1 num_classes]) - repmat(mu,[prod(imS) 1])))*(ivar)';
      // iv_m = w_m * ivar';
      // Compare to sandy i = l => he does exactly the same thing as I do
      
             int VoxelIndex = 0;
      for (i = 0; i< ImageMaxZ;i++){
        for (k = 0; k<ImageMaxY;k++){
          for (j = 0; j<ImageMaxX;j++){
        if (*OutputVector < EMSEGMENT_NOTROI) {
          for (m=0; m<NumInputImages; m++) {
            r_m[m](i,k,j) = 0.0;
            for (n=0; n<=m; n++) iv_m(m,n,i,k,j) = 0.0;
          } 
          
          for (l=0; l< NumTotalTypeCLASS; l++) {
            //------------------------------------------------------------
            // Calculate Bias Parameters  
            //------------------------------------------------------------
            for (m=0; m<NumInputImages; m++) {  
              for (n=0; n<NumInputImages; n++) {
            temp =  *w_m[l] * float(InverseWeightedLogCov[l][m][n]);
            r_m[m](i,k,j)     += temp * ((*InputVector)[n] - float(LogMu[l][n]));
            if (n <= m) iv_m(m,n,i,k,j) += temp;
              }
            }
            w_m[l]++;
          }
        } else { 
          for (l=0; l< NumTotalTypeCLASS; l++) {
            w_m[l] ++;
          }
        }
        InputVector ++;
        OutputVector ++;
        VoxelIndex ++;
          }
        } 
      } // End of for (z = 0; z < ImageMaxZ ; z++) 
      for (l=0; l< NumTotalTypeCLASS; l++) {
        w_m[l] = w_mPtr[l];
      }
      OutputVector = OutputVectorPtr;InputVector = InputVectorPtr;
      //------------------------------------------------------------
      // Finalize Bias Parameters  
      //------------------------------------------------------------
      //  smooth residuals and inv covariances - 3D
      // w(k) = sum(u(j)*v(k+1-j))
      
      // Kilian - when you change the window size (SegmentationBoundary) then it will have an impact on the convolution  
      // Otherwise there is still a bug in the software
      // 20-Nov-03 I checked it - Printed out the weights -> they7 are correct - then I let it iterate 10 times just commenting out the convolutio part 
      // no difference between Acitvated SegmentationBoundary and not Activated one !  
      iv_m.Conv(skern,SmoothingWidth);
      for (i=0; i<NumInputImages; i++) r_m[i].Conv(skern,SmoothingWidth);  
    } // End of M-Step (if (iter < NumIter) ..)
#if (EMVERBOSE)
    cout << "End of M-Step " << endl;
#endif   
    // -----------------------------------------------------------
    // Print out Parameters
    // -----------------------------------------------------------
    if ( ( (actSupCl->GetPrintFrequency() > 0) && (iter % (actSupCl->GetPrintFrequency()) == 0) ) || ((iter == NumIter ) && (actSupCl->GetPrintFrequency() == -1)) ) {
      cout << "vtkImageEMAtlasAlgorithm: Print intermediate result to " <<self->GetPrintDir() << endl;
      self->PrintIntermediateResultsToFile(iter, w_m, ROI, OutputVector, NumTotalTypeCLASS, NumChildClasses, actSupCl, LevelName, ClassList, ClassListType, LabelList, QualityFile);
      cout << "vtkImageEMAtlasAlgorithm: Return to Algorithm " << endl;
    }
      } // End Of EM-Algorithm ( for (iter=1; iter <= NumIter;iter++) ....
  } //  if (SegmentLevelSucessfullFlag)  ...

#if (EMVERBOSE)
  cout << "Start Deleting Classes " << endl;   
#endif

  if (QualityFile) {
    for (i = 0; i < EMSEGMENT_NUM_OF_QUALITY_MEASURE ; i++) {
      if (QualityFile[i] != NULL) {
    fflush(QualityFile[i]);
    fclose(QualityFile[i]);   
      }
    }
    delete[] QualityFile;
  }

  delete[] LabelList;
  delete[] CurrentLabelList;
  delete[] ProbDataIncY;
  delete[] ProbDataIncZ;
  delete[] TissueProbability;
  delete[] ProbDataWeight;
  delete[] ProbDataMinusWeight;
  delete[] ClassType;
  delete[] NumChildClasses;
  delete[] LogMu;
  delete[] LogCovariance;
  delete[] VirtualNumInputImages;
  delete[] VirtualOveralInputChannelFlag;

  for (i = 0; i < NumTotalTypeCLASS; i++) {
    for (x = 0; x < NumInputImages; x++)  delete[] InverseWeightedLogCov[i][x]; 
    delete[] InverseWeightedLogCov[i];
  }
  delete[] InverseWeightedLogCov;  
  delete[] InvSqrtDetWeightedLogCov; 

  delete[] OutputVector;
  delete[] ProbDataPtrCopy;
   for (i=0;i < NumTotalTypeCLASS; i++)  delete[] w_m_second[i];
  delete[] w_mPtr;
  delete[] w_m_second;
  delete[] w_m_secondPtr;

  delete[] cY_M;
  delete[] skern;

  for (i=0; i <  VirtualOveralInputChannelNum; i++) {
    delete[] iv_mat[i];
    delete[] inv_iv_mat[i];
  }
  delete[] iv_mat;
  delete[] inv_iv_mat;

  // cout << "vtkImageEMAtlasAlgorithm: Finished " << endl;
 }
//------------------------------------------------------------------------------
// Needed to define hierarchies! => this will be done at a later point int time at vtkImageEMAtlasSuperClass
// I did this design to multi thread it later
// If you start it always set ROI == NULL
int vtkImageEMAtlasSegmenter::HierarchicalSegmentation(vtkImageEMAtlasSuperClass* head, float** InputVector,short *ROI, short *OutputVector, EMTriVolume & iv_m, EMVolume *r_m,char* LevelName) {
  cout << "Start vtkImageEMAtlasSegmenter::HierarchicalSegmentation"<< endl;  
  // Nothing to segment
  if (head->GetNumClasses() ==0) {
    if (ROI == NULL) memset(OutputVector,0, sizeof(short)*this->ImageProd);
    return 0;
  }

  // ---------------------------------------------------------------
  // 1. Define Variables
  // ---------------------------------------------------------------
  cout <<"====================================== Segmenting Level " << LevelName << " ==========================================" << endl;
  int       i;
  void      **ClassList = head->GetClassList();
  classType *ClassListType = head->GetClassListType();
  int       NumClasses = head->GetNumClasses();
  int       NumTotalTypeCLASS = head->GetTotalNumberOfClasses(false); // This includes only classes from Type CLASSES  
  int       SegmentLevelSucessfullFlag;

  // The follwoing division is done for multi threading purposes -> even though it is currently not implemented 
  // you would also have to make a copy of iv_m and r_m . I currently do not do it because it takes to much space
  // Also first install zoom function before doing it !
  // I will do it sometime
  // Note: ROI is Region of Interest => Read Only ! OutputVecotr is Write only !  
  // It needs to be a class of vtkImageEMAtlasSegmenter => therefore we cannot use outPtr instead of OutputVactor 
  short *SegmentationResult = new short[this->ImageProd],
        *ROIPtr             = ROI, 
        *SegResultPtr       = SegmentationResult,
        *OutputVectorPtr    = OutputVector;

  // Kilian : Jan 06 We simply jump over segmentation and use previous calculated results - make generation of optimal parameters simpler
  if (head->GetPredefinedLabelMapPrefix()) {
    SegmentLevelSucessfullFlag = 1;

    // Make sure it is of type short and little endian
     int ChangedExtent[6] = {this->Extent[0],this->Extent[1],this->Extent[2],this->Extent[3], this->Extent[4] +1 ,this->Extent[5] + 1 };
    cout <<"Loading Predefined LabelMap from slices " << ChangedExtent[4] << " to " << ChangedExtent[5] << endl;
    vtkImageReader *PredefinedLabelMap = vtkImageReader::New();
    this->GEImageReader(PredefinedLabelMap,head->GetPredefinedLabelMapPrefix(),ChangedExtent[4],ChangedExtent[5], VTK_SHORT);

    memcpy(SegmentationResult,this->GetPointerToVtkImageData(PredefinedLabelMap->GetOutput(),VTK_SHORT,ChangedExtent),sizeof(short)*this->ImageProd);
    PredefinedLabelMap->Delete();
    cout << "Skipping segmentation" << endl;

  } else {
    // Run EM Algorithm and determine labelmap 
    memset(SegmentationResult,0,sizeof(short)*this->ImageProd);
    float **w_m            = new float*[NumTotalTypeCLASS];
    for (i=0; i<NumTotalTypeCLASS; i++) w_m[i] = new float[this->ImageProd];
    void  **ProbDataPtr    = new void*[NumTotalTypeCLASS];
    head->GetProbDataPtr(ProbDataPtr,0);

    // This is retroactive - it gives the warning here that the superclass ignores prob data even though that was important on the 
    // last level segmentation where the super class was involved - this is just simpler to program
    if ((head->GetProbDataWeight() == 0.0) && (this->GetHeadClass() != head)) {
      for (i=0; i<NumTotalTypeCLASS; i++) {
    if (ProbDataPtr[i]) {
      i = NumTotalTypeCLASS;
      vtkEMAddWarningMessage("Super Class segmented on Level " << LevelName  << " has ProbDataWeight == 0.0, but there are sub classes that have probability maps defined\n          => Probability Maps will be ignored!");
    } 
      }
    }

    this->activeSuperClass = head;
    // ---------------------------------------------------------------
    // 2. Segment Subject
    // ---------------------------------------------------------------

     switch (this->GetInput(0)->GetScalarType()) {
        vtkTemplateMacro9(vtkImageEMAtlasAlgorithm,this,(VTK_TT**) ProbDataPtr, InputVector, ROI,iv_m,r_m,w_m, LevelName, SegmentLevelSucessfullFlag);
     }

     if (SegmentLevelSucessfullFlag) { 
      // ---------------------------------------------------------------
      // 3. Analyze weights to determine label map and clean up 
      // ---------------------------------------------------------------
       int  *NumChildClasses = new int[NumClasses];
        for (i=0; i < NumClasses; i++) NumChildClasses[i]   = ((ClassListType[i] == CLASS) ? 1 : ((vtkImageEMAtlasSuperClass*) ClassList[i])->GetTotalNumberOfClasses(false));
       this->DetermineLabelMap(SegmentationResult, NumTotalTypeCLASS, NumChildClasses, this->activeSuperClass, ROI, this->ImageProd, w_m);
       delete[] NumChildClasses;
     }
     delete[] ProbDataPtr;   
 
     for (i=0; i<NumTotalTypeCLASS; i++) delete[] w_m[i]; 
     delete []w_m  ;
  }


  if (SegmentLevelSucessfullFlag) { 
    // ---------------------------------------------------------------
    // 4. Segment Subclasses
    // ---------------------------------------------------------------
    // Super Class Labels have to be unique (except HeadClass => not defined !) and not equal to zero
    // Transfere segmentation results to OutputVector  
    for (i=0; i < this->ImageProd; i++) {
      // Remeber ROI is initiated with NULL -> only on second level it gets extended 
      if ((ROIPtr == NULL) || (*ROIPtr++ == head->GetLabel())) *OutputVectorPtr = *SegResultPtr; 
      OutputVectorPtr++;
      SegResultPtr++; 
    }
  
    OutputVectorPtr = OutputVector;SegResultPtr = SegmentationResult; ROIPtr = ROI;
  
    // 4.) Run it for all sub Superclasses
    char      *NewLevelName = new char[strlen(LevelName)+5];

    for (i=0; i <NumClasses; i++) {
      // need to save results
      if (ClassListType[i] == SUPERCLASS &&  SegmentLevelSucessfullFlag) {
        sprintf(NewLevelName,"%s.%d",LevelName,i);
        // we should really create a copy of iv_m and r_m !! otherwise things can go wrong here
         SegmentLevelSucessfullFlag = this->HierarchicalSegmentation((vtkImageEMAtlasSuperClass*) ClassList[i],InputVector,SegmentationResult,OutputVector,iv_m,r_m,NewLevelName);
      } 
    }
    delete []NewLevelName;

  }
  delete []SegmentationResult;
  cout << "End vtkImageEMAtlasSegmenter::HierachicalSegmentation"<< endl; 
  return SegmentLevelSucessfullFlag;
}


//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class TOut>
static void vtkImageEMAtlasSegmenterExecute(vtkImageEMAtlasSegmenter *self,float **InputVector,vtkImageData *outData, TOut *outPtr,int outExt[6])
{
  // -----------------------------------------------------
  // 1.) Setup  Hierarchical Segmentation
  // -----------------------------------------------------
  int NumInputImages = self->GetNumInputImages();
  short *OutputVector = new short[self->GetImageProd()];
  char LevelName[3];
  int DimensionX = self->GetDimensionX();
  int DimensionY = self->GetDimensionY();
  int DimensionZ = self->GetDimensionZ();

  EMTriVolume iv_m(NumInputImages,DimensionZ,DimensionY,DimensionX); // weighted inverse covariances 
  EMVolume *r_m  = new EMVolume[NumInputImages]; // weighted residuals
  for (int i=0; i < NumInputImages; i++) r_m[i].Resize(DimensionZ,DimensionY,DimensionX);
  // Print information

  if ( (DimensionX != (outExt[1] - outExt[0] +1)) ||(DimensionY != (outExt[3] - outExt[2] +1)) ||(DimensionZ != (outExt[5] - outExt[4] +1)))  
    cout << "Segmentation Boundary is activated (" <<DimensionX  <<"," << DimensionY << "," << DimensionZ <<") !" << endl;    

  // Label All SuperClasses 
  {
    int TotalNumClasses = self->GetHeadClass()->GetTotalNumberOfClasses(1);
      short *LabelList = new short[TotalNumClasses];
      memset(LabelList,0,sizeof(short)*TotalNumClasses);
      // Get all existing labels
      int index = self->GetHeadClass()->GetAllLabels(LabelList,0,TotalNumClasses);
      // Otherwise no classes defined
      assert(index);
      // Label all super classes
      self->GetHeadClass()->LabelAllSuperClasses(LabelList,index, TotalNumClasses);
      delete[] LabelList;
  }

  // self->GetHeadClass()->DefineAllLogInvCovParamters();
  // self->GetHeadClass()->Print("");  
  sprintf(LevelName,"1");

  // -----------------------------------------------------
  // 2.) Run  Hierarchical Segmentation
  // -----------------------------------------------------
  if (self->HierarchicalSegmentation(self->GetHeadClass(),InputVector, NULL, OutputVector,iv_m,r_m,LevelName) == 0) {
      memset(OutputVector,0,sizeof(short)*self->GetImageProd());
  }

  // -----------------------------------------------------
  // 3.) Define   int outIncX, outIncY, outIncZ;
  // -----------------------------------------------------
  vtkIdType outIncX, outIncY, outIncZ;
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  int outInc[3] = {outIncX, outIncY, outIncZ};
  vtkImageEMAtlasSegmenter_TransfereDataToOutputExtension(self,OutputVector,outPtr,outInc);

  delete[] OutputVector;
  delete[] r_m;

  cout << "End vtkImageEMAtlasSegmenterExecute "<< endl;
}



//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageEMAtlasSegmenter::ExecuteData(vtkDataObject *)
{
  cout << "EMAtlasBrainClassifier Version" << endl;
  void *outPtr;
  int idx1, i;
  int NumProbMap = 0;
  int FirstProbMapDef = -1;
  // vtk4 - to find out more about the next couple of lines look at vtkImageEMGenericClass.cxx   
  vtkImageData **inData  = (vtkImageData **) this->GetInputs();
  vtkImageData *outData = this->GetOutput();
  // Magically the extent corresponds with the input extent !
  outData->GetWholeExtent(this->Extent);

  outData->SetExtent(this->Extent); 
  outData->AllocateScalars(); 
  // vtk4
  // -----------------------------------------------------
  // Define and Check General Parameters
  // -----------------------------------------------------
  vtkDebugMacro(<< "Execute: inData = " << inData << ", outData = " << outData);

  this->ResetErrorMessage();
  this->ResetWarningMessage();

  // this->NumberOfInputs = Actual number of inputs defined
  if (this->NumInputImages > this->NumberOfInputs) {
    vtkEMAddErrorMessage( "NumOfInputs ("<< this->NumberOfInputs << ") is greater than the number of Input images defined ("<<this->NumberOfInputs<<")!");
    return;
  } 

  if (this->NumberOfTrainingSamples < 1) {
     vtkEMAddErrorMessage( "Number of Training Samples taken for the probability map has to be defined first!");
    return;
  }

  if (outData == NULL) {
     vtkEMAddErrorMessage( "Output must be specified.");
    return;
  }

  // Did we define a superclass
  if (!this->HeadClass) {
    vtkEMAddErrorMessage( "No Head Class is defined !");
    return;
  } 

  this->HeadClass->Update();
  if (this->HeadClass->GetErrorFlag()) {
    vtkEMAddErrorMessage( "The following Error's occured during the class definition:" << endl << this->HeadClass->GetErrorMessages());
    return;
  }

  if (this->HeadClass->GetWarningFlag()) 
    vtkEMAddWarningMessage( "The following Warning's occured during the class definition:" << endl << this->HeadClass->GetWarningMessages());
  
  // Check if everything coresponds to each other
  if (!inData[0]) {
    vtkEMAddErrorMessage( "First image input is not defined !");
    return;
  }
  if ((this->HeadClass->GetProbDataScalarType() > -1) && (this->HeadClass->GetProbDataScalarType() !=  inData[0]->GetScalarType())) {
    vtkEMAddErrorMessage( "Scalar Type of Probability maps defined in classes does not correspond to scalar type of input image1");
    return;
  }
  if (this->HeadClass->GetNumInputImages() != this->NumInputImages) {
    vtkEMAddErrorMessage("Number of Input images of classes differes from number of images defined for this filter!");
    return;
  }
 
  // -----------------------------------------------------
  // Define Image Parameters
  // -----------------------------------------------------
  // this->GetInput(StartInputImages)  = first intensity input
  this->ImageProd = this->GetDimensionX() * this->GetDimensionY() * this->GetDimensionZ();

  // -----------------------------------------------------
  // Checking dimension with start and end slice 
  // -----------------------------------------------------
  // Making sure values are set correctly
  { 
    int* BoundaryMin = this->GetSegmentationBoundaryMin();
    int* BoundaryMax = this->GetSegmentationBoundaryMax();
    for (i=0; i < 3; i++) {
      int Dimension = this->Extent[i*2+1] - this->Extent[i*2] + 1;  
      if ((BoundaryMin[i] > Dimension) || (BoundaryMin[i] < 1)) {
    vtkEMAddErrorMessage( "SegmentationBoundaryMin[" <<i<<"]=" << BoundaryMin[i] << " is not defined correctly ! Should not be smaller than "<<  Dimension << " and larger 0 !");
    return;
      }
      if ((BoundaryMax[i] > Dimension) || (BoundaryMax[i] < BoundaryMin[i]) ) {
    vtkEMAddErrorMessage( "SegmentationBoundaryMax[" <<i<<"]=" << BoundaryMax[i] << " is not defined correctly ! Should not be larger than "<<  Dimension 
                  << " and not smaller than SegmentationBoundaryMin (" << BoundaryMin[i] << ") !");
    return;
      }
    }
  }
  // -----------------------------------------------------
  // Read Input Images
  // -----------------------------------------------------
  float **InputVector = new float*[this->ImageProd];
  for(idx1 = 0; idx1 <this->ImageProd; idx1++) InputVector[idx1] = new float[this->NumInputImages];
 
  for (idx1 = 0; idx1 < this->NumInputImages ; idx1++){  
    if (this->CheckInputImage(inData[idx1],this->GetInput(0)->GetScalarType(), this->GetInput(0)->GetSpacing(), idx1+1)) return;
    switch (this->GetInput(idx1)->GetScalarType()) {
      vtkTemplateMacro6(vtkImageEMAtlasSegmenterReadInputChannel,this, inData[idx1], (VTK_TT *)(inData[idx1]->GetScalarPointerForExtent(this->Extent)),this->Extent,InputVector,idx1);
    default:
      vtkEMAddErrorMessage( "Execute: Unknown ScalarType");
      return;
    } 
  }

  // -----------------------------------------------------
  // Read in Debugging Data 
  // -----------------------------------------------------

#if (EMVERBOSE)
 {
    vtkIndent indent;
    this->PrintSelf(cout,indent); 
 }
#endif
  // -----------------------------------------------------
  // Execute Segmentation Algorithmm
  // -----------------------------------------------------
  outPtr = outData->GetScalarPointerForExtent(outData->GetExtent());
  switch (this->GetOutput()->GetScalarType()) {
    vtkTemplateMacro5(vtkImageEMAtlasSegmenterExecute, this, InputVector, outData, (VTK_TT*)outPtr,this->Extent);
  default:
    vtkEMAddErrorMessage("Execute: Unknown ScalarType");
    return;
  }
  for(idx1 = 0; idx1 <this->ImageProd; idx1++) delete[] InputVector[idx1];
  delete[] InputVector;
}

