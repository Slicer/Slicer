/*=auto=========================================================================

(c) Copyright 2001 Massachusetts Institute of Technology 

Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for any purpose, 
provided that the above copyright notice and the following three paragraphs 
appear on all copies of this software.  Use of this software constitutes 
acceptance of these terms and conditions.

IN NO EVENT SHALL MIT BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, 
INCIDENTAL, OR CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE 
AND ITS DOCUMENTATION, EVEN IF MIT HAS BEEN ADVISED OF THE POSSIBILITY OF 
SUCH DAMAGE.

MIT SPECIFICALLY DISCLAIMS ANY EXPRESS OR IMPLIED WARRANTIES INCLUDING, 
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR 
A PARTICULAR PURPOSE, AND NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED "AS IS."  MIT HAS NO OBLIGATION TO PROVIDE 
MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

=========================================================================auto=*/

#include "EMLocalRegistrationCostFunction.h"
#include "vtkSimonParameterReaderWriter.h"
#include "vtkImageEMLocalSuperClass.h"
#include "vtkImageEMGeneral.h"

// Needed for scaling 
#define EMREGISTRATION_MIN_SCALING 0.70
#define EMREGISTRATION_MAX_SCALING 1.22
#define EMREGISTRATION_MIN_OFFSET 0.1


template <class T> void EMLocalRegistrationCostFunction_PrintVector(T* parameters, int Min,  int Max) {
  for (int i = Min; i <= Max ; i++)  std::cerr << parameters[i] << " ";
  std::cerr << endl;
}


void EMLocalRegistrationCostFunction_ROI::CreateMAP(int size)  {
  assert(size > 0);
  assert(!this->MAP);
  this->MAP = new char[size];
  for(int i= 0 ; i < size; i++) this->MAP[i] = -1;
} 

//------------------------------------------------------

void EMLocalRegistrationCostFunction::InitializeParameters() {
  // std::cerr << "EMLocalRegistrationCostFunction::InitializeParameters initialize with all the ProbData stuff" << endl;

  ProbDataPtr                        = NULL;
  GlobalToAtlasRotationMatrix        = NULL;
  GlobalToAtlasTranslationVector     = NULL;

  SuperClassToAtlasRotationMatrix    = NULL;
  SuperClassToAtlasTranslationVector = NULL;

  // Do not know yet 
  IndependentSubClassFlag            = NULL;
  ClassSpecificRegistrationFlag      = NULL;
  ROI_ProbData                       = NULL;
  ROI_Weight                         = NULL;

  // Gaussian Penalty for parmaters  
  ClassInvCovariance                 = NULL;
 
  // MultiThreading
  MultiThreadedParameters            = NULL;
  Threader                           = NULL;

  // Input Parameters to be aligned
  weights                            = NULL;

  // Defines Region of Interest
  Boundary_ROIVector                 = NULL;

  //Captures simularity measure at  each voxel
  SpatialCostFunction                = NULL;

  // General Registration Parameters
  RegistrationType                   = EMSEGMENT_REGISTRATION_DISABLED;
  Debug                              = 0;  
  GenerateBackgroundProbability      = -1;
  memset(Image_Length,0,sizeof(int)*3); 

  Image_MidX = -1; 
  Image_MidY = -1;
  Image_MidZ = -1;

  memset(Boundary_Min,0,sizeof(int)*3);
  memset(Boundary_Max,0,sizeof(int)*3);
  Boundary_LengthX = 0;   
  Boundary_LengthY = 0;   
  Boundary_LengthXYZ = 0; 

  // -----------------------------------------------------
  // Parameter Dimensions 
  NumberOfParameterSets = -1;
  NumberOfParameterPerSet = -1; 
  TwoDFlag = -1;
  RigidFlag = -1;

  Boundary_NumberOfROIVoxels = -1;
  

  InterpolationType = -1; 
  NumberOfTrainingSamples = -1;

  NumberOfThreads = -1;

  // This is necessary so that we can itegrate our method into itk
  this->ParaDepVar = new EMLocalRegistrationCostFunction_IterationSpecificVariables;
  this->ParaDepVar->ClassToAtlasRotationMatrix = NULL;
  this->ParaDepVar->ClassToAtlasTranslationVector = NULL;
  this->ParaDepVar->MinWeightAtlasCost = -1;
  this->ParaDepVar->MinGaussianCost = -1;

  this->ParaDepVar->ROI_MinZ = -1;
  this->ParaDepVar->ROI_MinY = -1;
  this->ParaDepVar->ROI_MinX = -1;
  
  this->ParaDepVar->ROI_MaxZ = -1;
  this->ParaDepVar->ROI_MaxY = -1;
  this->ParaDepVar->ROI_MaxX = -1;

  this->ParaDepVar->Boundary_OffsetY = -1;
  this->ParaDepVar->Boundary_OffsetZ = -1;
 
}

// Registration Procedures originally defined by Simon but changed by me 

// The Registration Parameters 
// A(From Atlas To Structure) = A(Global To Structure) * A(Atlas to Global)

// #define EMSEGMENT_REGISTRATION_.. is defined in vtkImageEMLocalSuperClass.h

EMLocalRegistrationCostFunction::~EMLocalRegistrationCostFunction() {
    this->ClassInvCovariance_Delete(); 
    this->SpatialCostFunctionOff();
    this->MultiThreadDelete();
    delete this->ParaDepVar; 
}

void EMLocalRegistrationCostFunction::SetBoundary(int MinX, int MinY, int MinZ, int MaxX, int MaxY, int MaxZ) {
  this->Boundary_Min[0] = MinX;
  this->Boundary_Min[1] = MinY;
  this->Boundary_Min[2] = MinZ;

  this->Boundary_Max[0] = MaxX;
  this->Boundary_Max[1] = MaxY;
  this->Boundary_Max[2] = MaxZ;

  for (int i = 0; i < 3; i++) assert(this->Boundary_Max[i] >= this->Boundary_Min[i]);
  this->Boundary_LengthX =  this->Boundary_Max[0] - this->Boundary_Min[0] + 1;
  this->Boundary_LengthY =  this->Boundary_Max[1] - this->Boundary_Min[1] + 1;
  this->Boundary_LengthXYZ = this->Boundary_LengthX * this->Boundary_LengthY * (this->Boundary_Max[2] - this->Boundary_Min[2] + 1);
}

void EMLocalRegistrationCostFunction::SetImage_Length(int LengthX, int LengthY, int LengthZ) 
 {
    this->Image_Length[0] = LengthX;
    this->Image_Length[1] = LengthY;
    this->Image_Length[2] = LengthZ;
  
    this->Image_MidX   = (LengthX-1)*0.5;; 
    this->Image_MidY   = (LengthY-1)*0.5;; 
    this->Image_MidZ   = (LengthZ-1)*0.5;; 
  }

void EMLocalRegistrationCostFunction::SetDimensionOfParameter(int inNumberOfParameterSets, int inTwoDFlag, int inRigidFlag) {
    // Define this later directly based on Boundary_LengthZ;
    this->RigidFlag = inRigidFlag;
    this->TwoDFlag  = inTwoDFlag;
    this->NumberOfParameterSets = inNumberOfParameterSets;
    if (this->TwoDFlag) {
      this->NumberOfParameterPerSet = (this->RigidFlag ? 3:5);
    } else {
      this->NumberOfParameterPerSet = (this->RigidFlag ? 6:9);

    }
  }

void EMLocalRegistrationCostFunction::ClassInvCovariance_Define(classType* ClassListType, void** ClassList) {
  assert(this->EMHierarchyParameters.NumClasses > -1);
  assert(this->GenerateBackgroundProbability > -1);
  if (this->ClassInvCovariance) delete[] this->ClassInvCovariance;
  this->ClassInvCovariance  = new double*[this->EMHierarchyParameters.NumClasses];
 
  if (this->GenerateBackgroundProbability) this->ClassInvCovariance[0] = NULL;
  for (int i = this->GenerateBackgroundProbability; i < this->EMHierarchyParameters.NumClasses; i ++) {
    this->ClassInvCovariance[i]  = new double[9];
    if (ClassListType[i] == CLASS) 
    memcpy(this->ClassInvCovariance[i],((vtkImageEMLocalClass*) ClassList[i])->GetRegistrationInvCovariance(), 9*sizeof(double)); 
    else
    memcpy(this->ClassInvCovariance[i],((vtkImageEMLocalSuperClass*) ClassList[i])->GetRegistrationInvCovariance(), 9*sizeof(double)); 

  }
}

void EMLocalRegistrationCostFunction::ClassInvCovariance_Print() {
     for (int i = this->GenerateBackgroundProbability; i < this->EMHierarchyParameters.NumClasses; i ++) {
       std::cerr << "Class " << i << " " ; 
       for (int j = 0; j < 9 ; j++) std::cerr << this->ClassInvCovariance[i][j] << " "; 
       std::cerr << endl;
     }
}

// All cordinates are given in absolute parameteres
void EMLocalRegistrationCostFunction::DefineRegistrationParametersForThreadedCostFunction(int initROI_MinX, int initROI_MinY, int initROI_MinZ, int initROI_MaxX, int initROI_MaxY, int initROI_MaxZ) const {
  int Boundary_MinX = this->Boundary_Min[0];
  int Boundary_MinY = this->Boundary_Min[1];
  int Boundary_MinZ = this->Boundary_Min[2];
  int Boundary_MaxX = this->Boundary_Max[0];
  int Boundary_MaxY = this->Boundary_Max[1];
  int Boundary_MaxZ = this->Boundary_Max[2];

  if (initROI_MinX < Boundary_MinX ) this->ParaDepVar->ROI_MinX = Boundary_MinX;
  else this->ParaDepVar->ROI_MinX = initROI_MinX;

  if (initROI_MinY < Boundary_MinY) this->ParaDepVar->ROI_MinY = Boundary_MinY;
  else this->ParaDepVar->ROI_MinY = initROI_MinY;

  if (initROI_MinZ < Boundary_MinZ) this->ParaDepVar->ROI_MinZ = Boundary_MinZ;
  else this->ParaDepVar->ROI_MinZ = initROI_MinZ;


  if (initROI_MaxX > Boundary_MaxX) this->ParaDepVar->ROI_MaxX = Boundary_MaxX;
  else this->ParaDepVar->ROI_MaxX = initROI_MaxX;
  if (initROI_MaxY > Boundary_MaxY) this->ParaDepVar->ROI_MaxY = Boundary_MaxY;
  else this->ParaDepVar->ROI_MaxY = initROI_MaxY;
  if (initROI_MaxZ > Boundary_MaxZ) this->ParaDepVar->ROI_MaxZ = Boundary_MaxZ;
  else this->ParaDepVar->ROI_MaxZ = initROI_MaxZ;

  // --------------------------------------------------
  // Define Registration ROI Specific Parameters
  int ROI_LengthX           = this->ParaDepVar->ROI_MaxX -  this->ParaDepVar->ROI_MinX + 1;
  int ROI_LengthY           = this->ParaDepVar->ROI_MaxY -  this->ParaDepVar->ROI_MinY + 1;
  int ROI_LengthZ           = this->ParaDepVar->ROI_MaxZ -  this->ParaDepVar->ROI_MinZ + 1;
  int ROI_LengthXY          = ROI_LengthX * ROI_LengthY;
  int ROI_LengthXYZ         = ROI_LengthXY * ROI_LengthZ;

  // Job size with respect to ROI of registration 
  int ROI_JobSize           = ROI_LengthXYZ/this->NumberOfThreads;
  vtkNotUsed(int ROI_JobSizeXY         = ROI_JobSize % ROI_LengthXY;);

  // std::cerr << " nlun " <<  " " << ROI_MinX << " " << ROI_MinY << " " << ROI_MinZ << " " << ROI_MaxX << " " << ROI_MaxY << " " << ROI_MaxZ  << endl;

  // --------------------------------------------------
  // Define Real Image Space Parameters (For ProbData) - SegmentationBoundary is ignored
  int Image_LengthX  = this->Image_Length[0];
  int Image_LengthY  = this->Image_Length[1];
  int Image_LengthXY = Image_LengthX*Image_LengthY;
  // Real_ Means in Real_ Index parameters which takes boundary into consideration
  int Image_VoxelOffset = this->ParaDepVar->ROI_MinZ*Image_LengthXY + this->ParaDepVar->ROI_MinY * Image_LengthX + this->ParaDepVar->ROI_MinX;    
  
  // --------------------------------------------------
  // Define Segmenation Space Specific Parameters - SegmentationBoundary is not ignored 
  // Dimension of ROI for segmentation (Image with SegmentationBoundary cut out) 
  int Boundary_LengthXY     = this->Boundary_LengthX*this->Boundary_LengthY;

  // Offset of segmentation data in registration ROI
  this->ParaDepVar->Boundary_OffsetZ     = (this->Boundary_LengthY - ROI_LengthY)*this->Boundary_LengthX;
  this->ParaDepVar->Boundary_OffsetY     = this->Boundary_LengthX - ROI_LengthX;
  int Boundary_VoxelOffset      = (this->ParaDepVar->ROI_MinX - Boundary_MinX) + (this->ParaDepVar->ROI_MinY - Boundary_MinY)* Boundary_LengthX + (this->ParaDepVar->ROI_MinZ - Boundary_MinZ)* Boundary_LengthXY;

  // --------------------------------------------------
  // Thread Specific Paramaters
  int ROI_VoxelThreadStart = 0;
  int ROI_VoxelThreadStartZ;
  int ROI_VoxelThreadStartY;
  int ROI_VoxelThreadStartX;
  int ROI_VoxelThreadStartXY;

  int Image_VoxelStart;
  int Image_VoxelStartXY;

  for (int i = 0 ; i < this->NumberOfThreads; i++) {                                    
    if (i < this->NumberOfThreads-1) this->MultiThreadedParameters[i].ROI_NumVoxels = ROI_JobSize;
    else { this->MultiThreadedParameters[i].ROI_NumVoxels = ROI_JobSize + (ROI_LengthXYZ - ROI_JobSize * this->NumberOfThreads);}

    // Transform from ROI Corrdinates into Real and Boundary coordinates 
    ROI_VoxelThreadStartXY = ROI_VoxelThreadStart % ROI_LengthXY;
    ROI_VoxelThreadStartZ =  ROI_VoxelThreadStart/ROI_LengthXY;
    ROI_VoxelThreadStartY =  ROI_VoxelThreadStartXY/ROI_LengthX;
    ROI_VoxelThreadStartX =  ROI_VoxelThreadStartXY%ROI_LengthX;
 
    Image_VoxelStart = Image_VoxelOffset +  ROI_VoxelThreadStartZ * Image_LengthXY + ROI_VoxelThreadStartY * Image_LengthX + ROI_VoxelThreadStartX;
    Image_VoxelStartXY  = Image_VoxelStart %  Image_LengthXY ;

    this->MultiThreadedParameters[i].Real_VoxelStart[2] = Image_VoxelStart / Image_LengthXY;
    this->MultiThreadedParameters[i].Real_VoxelStart[1] = Image_VoxelStartXY / Image_LengthX;
    this->MultiThreadedParameters[i].Real_VoxelStart[0] = Image_VoxelStartXY % Image_LengthX;

    this->MultiThreadedParameters[i].Boundary_VoxelOffset = Boundary_VoxelOffset + ROI_VoxelThreadStartZ * Boundary_LengthXY + ROI_VoxelThreadStartY * Boundary_LengthX + ROI_VoxelThreadStartX;
    
    ROI_VoxelThreadStart += ROI_JobSize;
    
    //RegistrationParameters.MultiThreadedParameters[i].DebugMin[0] =  ROI_MaxX;
    //RegistrationParameters.MultiThreadedParameters[i].DebugMin[1] =  ROI_MaxY;
    //RegistrationParameters.MultiThreadedParameters[i].DebugMin[2] =  ROI_MaxZ;

    // RegistrationParameters.MultiThreadedParameters[i].DebugMax[0] =  ROI_MinX;
    // RegistrationParameters.MultiThreadedParameters[i].DebugMax[1] =  ROI_MinY;
    // RegistrationParameters.MultiThreadedParameters[i].DebugMax[2] =  ROI_MinZ;
  }
}



void EMLocalRegistrationCostFunction::MultiThreadDelete() { 
    this->NumberOfThreads = 0;
    if (this->MultiThreadedParameters) delete[] this->MultiThreadedParameters; 
    this->MultiThreadedParameters = NULL;
    if (this->Threader) this->Threader->Delete(); 
    this->Threader = NULL;
}

void EMLocalRegistrationCostFunction::SpatialCostFunctionOn() { 
    assert(this->Boundary_LengthXYZ > 0);
    
    if (this->SpatialCostFunction) delete[] this->SpatialCostFunction;
    SpatialCostFunction = new double[this->Boundary_LengthXYZ];
}
  
void EMLocalRegistrationCostFunction::SpatialCostFunctionOff() { 
    if (this->SpatialCostFunction) delete[] this->SpatialCostFunction;
    this->SpatialCostFunction = NULL;
  }

void EMLocalRegistrationCostFunction::ClassInvCovariance_Delete() {    
    if (this->ClassInvCovariance) {
      for (int i = this->GenerateBackgroundProbability; i < this->EMHierarchyParameters.NumClasses; i ++) 
    delete[] this->ClassInvCovariance[i]; 
      delete[] this->ClassInvCovariance;
    }
    this->ClassInvCovariance = NULL;
}

VTK_THREAD_RETURN_TYPE EMLocalRegistrationCostFunction_CostFunction_Sum_WeightxProbability_Thread(void *arg) {
  int CurrentThread = ((ThreadInfoStruct*)(arg))->ThreadID;
  EMLocalRegistrationCostFunction* self = (EMLocalRegistrationCostFunction*) (((ThreadInfoStruct*)(arg))->UserData);

  assert(CurrentThread < self->GetNumberOfThreads()); 

  EMLocalRegistrationCostFunction_MultiThreadedParameters* ThreadedParameters = &(self->GetMultiThreadedParameters()[CurrentThread]);
  ThreadedParameters->Result = 0.0;
  self->CostFunction_Sum_WeightxProbability(ThreadedParameters->Real_VoxelStart, ThreadedParameters->Boundary_VoxelOffset, ThreadedParameters->ROI_NumVoxels, ThreadedParameters->Result);
  return VTK_THREAD_RETURN_VALUE;
}


// Define parameters for threaded cost function 
void EMLocalRegistrationCostFunction::MultiThreadDefine(int DisableFlag) {
  this->MultiThreadDelete();
  
  this->NumberOfThreads = EMLocalInterface_GetDefaultNumberOfThreads(DisableFlag); 
 
  //std::cerr << "Registration Debug NumverOf Treads " << NumberOfThreads << endl;
  this->MultiThreadedParameters = new EMLocalRegistrationCostFunction_MultiThreadedParameters[this->NumberOfThreads];
  this->Threader = vtkMultiThreader::New();
  this->Threader->SetNumberOfThreads(NumberOfThreads);
  this->Threader->SetSingleMethod(EMLocalRegistrationCostFunction_CostFunction_Sum_WeightxProbability_Thread, this); 
}

// Scales rotation values between -180 and 180 after they have been optimized with MAP algorithm
void EMLocalRegistrationCostFunction::ScaleRotationValues(double *FinalParameters) {
  int ParaIndex        = (this->TwoDFlag ? 3 : 4);
  for (int j = 0; j < this->NumberOfParameterSets; j++) {      
    FinalParameters[ParaIndex] = fmod(FinalParameters[ParaIndex],(double)180.0);
    if (!this->TwoDFlag) {
      FinalParameters[ParaIndex + 1] = fmod(FinalParameters[ParaIndex + 1],(double)180.0);
      FinalParameters[ParaIndex + 2] = fmod(FinalParameters[ParaIndex + 2],(double)180.0);
    }
    ParaIndex += this->NumberOfParameterPerSet;
  }
} 

//------------------------------------------------------
// Cost functions for Registration methods 
//------------------------------------------------------

inline void EMLocalRegistrationCostFunction::CostFunction_Sum_WeightxProbability(int Real_VoxelStart[3], int Boundary_VoxelOffset, int ROI_NumVoxels, double &result) {
  // Assign ProbDataPtr before starting this function
  assert(this->ProbDataPtr); 

  // -----------------------------------------------------------
  // EMHierarchyParameters 
  int NumClasses               = this->GetNumClasses(); 
  int NumClassesMinusOne       = NumClasses -1;

  int*   NumChildClasses       = this->GetNumChildClasses();
  int    NumTotalTypeCLASS     = this->GetNumTotalTypeCLASS();
  int*   ProbDataIncY          = this->GetProbDataIncY();
  int*   ProbDataIncZ          = this->GetProbDataIncZ();

  // Pointer variables 
  unsigned char *Boundary_ROIVectorPtr   = this->Boundary_ROIVector;
  double *SpatialCostFunctionPtr = this->SpatialCostFunction;

  float **WeightsCopy = new float*[NumTotalTypeCLASS];
  for (int i = 0; i < NumTotalTypeCLASS; i++) WeightsCopy[i] = this->weights[i]; 

  char* ROI_Weight_MAP        = (this->ROI_Weight ?    this->ROI_Weight->MAP: NULL ); 
  char* ROI_ProbData_MAP      = (this->ROI_ProbData ?  this->ROI_ProbData->MAP : NULL); 
 
  // Function Specific Variables 
  double SubClassAlignedProbability = 0.0;
  double SumOverAlignedProbData; 
  double SumOverWeightsAndAlignedProbData;

  double SumOverClassAlignedProbData; 
  double SumOverClassWeights;
  int    NumOfWeightsGreaterZero;
  int ClassIndex;

  float targetX = 0.0;
  float targetY = 0.0;
  float targetZ = 0.0;
 
  double VoxelResult;
  // -----------------------------------------------------------
  // Initialization of parameters 

  result = 0.0;

  double RowResult   = 0.0;
  double SliceResult = 0.0;

  double RegistrationEpsilon    = 0.05;
  double LogRegistrationEpsilon = log(RegistrationEpsilon);

  // Run CostFunction
  int  ROI_Flag = 1;
  double Voxel_ProbData_MAP = -1;

  int x = Real_VoxelStart[0]; 
  int y = Real_VoxelStart[1]; 
  int z = Real_VoxelStart[2]; 

  for (int i = 0; i <NumTotalTypeCLASS; i++) WeightsCopy[i] += Boundary_VoxelOffset;
  Boundary_ROIVectorPtr += Boundary_VoxelOffset;
  if (ROI_Weight_MAP) ROI_Weight_MAP += Boundary_VoxelOffset;

  if (SpatialCostFunctionPtr) SpatialCostFunctionPtr += Boundary_VoxelOffset;

  // -----------------------------------------------------------
  // Check Variables

  // Otherwise there is nothing to register;
  assert(NumClasses);
  // Make sure that we have enough classes defined
  assert(!this->GenerateBackgroundProbability || NumClasses > 1);

  // Make sure that both are NULL or non-NULL
  assert (!this->ROI_Weight == !this->ROI_ProbData);

  // Make sure that both are NULL or non-NULL
  assert (!ROI_ProbData_MAP == !ROI_Weight_MAP);

  // -----------------------------------------------------------
  // Start of Costfunction
  // -----------------------------------------------------------
  for (int voxel = 0; voxel < ROI_NumVoxels; voxel++) {
    if (*Boundary_ROIVectorPtr < EMSEGMENT_INCORRECT_MODEL) {
      // std::cerr << voxel << endl;
      ClassIndex =  NumTotalTypeCLASS - 1;
      SumOverAlignedProbData           = 0.0;
      SumOverWeightsAndAlignedProbData = 0.0;
      NumOfWeightsGreaterZero          = 0;
      if (this->RegistrationType == EMSEGMENT_REGISTRATION_GLOBAL_ONLY)  {
    EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(this->ParaDepVar->ClassToAtlasRotationMatrix[NumClassesMinusOne], this->ParaDepVar->ClassToAtlasTranslationVector[NumClassesMinusOne], 
                                                                                  x, y, z, targetX, targetY, targetZ,this->Image_MidX, this->Image_MidY, this->Image_MidZ);
         Voxel_ProbData_MAP = EMLocalInterface_Interpolation(targetX, targetY, targetZ, this->Image_Length[0], this->Image_Length[1], this->Image_Length[2], ROI_ProbData_MAP,0,0,this->InterpolationType,this->Image_Length);
    ROI_Flag = ((Voxel_ProbData_MAP != double(*ROI_Weight_MAP) ) || (Voxel_ProbData_MAP < 0));
      }
      if (ROI_Flag) { 

    for (int h = NumClassesMinusOne; h > -1; h--) {
      SumOverClassAlignedProbData =0.0; 
      SumOverClassWeights         = 0.0;
     
      if (h || !this->GenerateBackgroundProbability) {
        // The function calculates the coordinates in target space (in our case the atlas space [targetX, targetY, targetZ]) from the image space (indexX, indexY, indexZ)
        if (this->RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
          EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(this->ParaDepVar->ClassToAtlasRotationMatrix[h], this->ParaDepVar->ClassToAtlasTranslationVector[h], x, y, z, 
                                                                                        targetX, targetY, targetZ,this->Image_MidX, this->Image_MidY, this->Image_MidZ);
        }
            // Kilian: Right now we sum over all spatial priors even if one super class has only one spatial prior defined ! Change this like you did for shape
        for (int i = NumChildClasses[h] -1 ; i  > -1; i--) {
          if (this->ProbDataPtr[ClassIndex])  {
                switch (this->GetProbDataType()) {
          vtkTemplateMacro(EMLocalInterface_Interpolation(targetX, targetY, targetZ, this->Image_Length[0], this->Image_Length[1], this->Image_Length[2], 
                                                                  ((VTK_TT**) this->ProbDataPtr)[ClassIndex], ProbDataIncY[ClassIndex],ProbDataIncZ[ClassIndex],
                                  this->InterpolationType, this->Image_Length,SubClassAlignedProbability));
        }

        SumOverClassAlignedProbData += SubClassAlignedProbability;
        // ----------------------------------------------------------------------------
        // Define Cost Function at the voxel
        // (when subclasses are indendent) 
                // The seperation is important bc otherwise if your registration might be not as good,
                // e.g. CSF , white matter and Grey matter are the forground -> only difference 
                // in skull are is used for registration which is not very reliable -> instead 
                // we would like to include the ventricles 
        // ----------------------------------------------------------------------------
        if (*WeightsCopy[ClassIndex]) {
          if (this->IndependentSubClassFlag[h]) {
            SumOverWeightsAndAlignedProbData +=  double(*WeightsCopy[ClassIndex]) * (SubClassAlignedProbability > 0.0 ? log(SubClassAlignedProbability) : LogRegistrationEpsilon);
            NumOfWeightsGreaterZero ++;
          }  else {
            SumOverClassWeights += double(*WeightsCopy[ClassIndex]);
          }
        }
          }
              // if ((x == 201) && (y == 118) && ( z == 3)) std::cerr << ClassIndex << " ++++ " << SumOverClassAlignedProbData << " " << (*WeightsCopy[ClassIndex]) << endl;
          ClassIndex --;
        }
      } else {
        SumOverClassAlignedProbData   = (SumOverAlignedProbData < this->NumberOfTrainingSamples ?  (this->NumberOfTrainingSamples- SumOverAlignedProbData) : 0.0);
        for (int i = NumChildClasses[0] -1 ; i  > -1; i--) {
          if (*WeightsCopy[ClassIndex]) {
        if (this->IndependentSubClassFlag[h]) {
          SumOverWeightsAndAlignedProbData +=  double(*WeightsCopy[ClassIndex]) * (SumOverClassAlignedProbData  > 0.0 ? log(SumOverClassAlignedProbData) : LogRegistrationEpsilon);
          NumOfWeightsGreaterZero ++;
        }  else {
          SumOverClassWeights += double(*WeightsCopy[ClassIndex]);
        }
          }
          ClassIndex --;
        }
      }
      SumOverAlignedProbData += SumOverClassAlignedProbData;

          // ----------------------------------------------------------------------------
      //  Define Cost Function at the voxel
          // (when subclasses should be counted together)
          // ----------------------------------------------------------------------------
      if (SumOverClassWeights && !this->IndependentSubClassFlag[h]) {
          SumOverWeightsAndAlignedProbData +=  SumOverClassWeights * (SumOverClassAlignedProbData > 0.0 ? log(SumOverClassAlignedProbData) : LogRegistrationEpsilon);
          NumOfWeightsGreaterZero ++;
      }
    } // End of  for (int h = NumClassesMinusOne ...
      
    // Special Case if no structure had any Probablity Data above zero
    // If GenerateBackgroundProbability = 1 => this is never going to happen 
    if (NumOfWeightsGreaterZero) {
      if (SumOverAlignedProbData == 0.0) SumOverAlignedProbData = double(NumOfWeightsGreaterZero)*RegistrationEpsilon;
      
      // This is the definition of the cost function for each voxel 
      VoxelResult = SumOverWeightsAndAlignedProbData - log(SumOverAlignedProbData);
      // Ignore otherwise bc it is just numerical errors 
      if (VoxelResult < -0.0001) {
        RowResult += VoxelResult;
        if (SpatialCostFunctionPtr) *SpatialCostFunctionPtr =  -VoxelResult; 
        // if (x == 201 && y == 118 & z == 3) std::cerr << z << " " <<  y << " " << x << " " << SumOverWeightsAndAlignedProbData << " - " << log(SumOverAlignedProbData) << " " << SumOverAlignedProbData << " " << VoxelResult << " ===== " << targetX <<  " " << targetY << " " << targetZ << endl; 
        // It is negative bc we look at minus values later to find minimum even though we really search of positive maximimum

        // if (DebugMin[0] > x)  DebugMin[0] = x;
        // if (DebugMax[0] < x)  DebugMax[0] = x;
        // Debug_FlagY = 1;
        // Debug_FlagZ = 1;
      }
    }
      } 
    } 
    // if (SpatialCostFunctionPtr) *SpatialCostFunctionPtr =  double(*ROI_Weight_MAP); 
    for (int i = 0; i < NumTotalTypeCLASS; i++) WeightsCopy[i] ++;
    Boundary_ROIVectorPtr ++;
    if (SpatialCostFunctionPtr) SpatialCostFunctionPtr ++;
    if (ROI_Weight_MAP) ROI_Weight_MAP ++;
    x++;

    // Sanity Check - if this assert is triggered than we went over the boundary
    assert(z <= this->ParaDepVar->ROI_MaxZ);

    if (x > this->ParaDepVar->ROI_MaxX) {
      x = this->ParaDepVar->ROI_MinX; 
      Boundary_ROIVectorPtr += this->ParaDepVar->Boundary_OffsetY; 
      if (SpatialCostFunctionPtr) SpatialCostFunctionPtr += this->ParaDepVar->Boundary_OffsetY;
      if (ROI_Weight_MAP) ROI_Weight_MAP += this->ParaDepVar->Boundary_OffsetY;
      for (int i = 0; i < NumTotalTypeCLASS; i++) WeightsCopy[i] += this->ParaDepVar->Boundary_OffsetY;
      
      SliceResult += RowResult;
      RowResult = 0.0;

      
      //if (Debug_FlagY) {
      //    if (DebugMin[1] > y)  DebugMin[1] = y;
      //    if (DebugMax[1] < y)  DebugMax[1] = y;
      //    Debug_FlagY =0 ;
      // }   
      y++;
  
      if (y > this->ParaDepVar->ROI_MaxY) {
    y = this->ParaDepVar->ROI_MinY;
    Boundary_ROIVectorPtr += this->ParaDepVar->Boundary_OffsetZ; 
    if (SpatialCostFunctionPtr) SpatialCostFunctionPtr += this->ParaDepVar->Boundary_OffsetZ;
    if (ROI_Weight_MAP) ROI_Weight_MAP += this->ParaDepVar->Boundary_OffsetZ;
    for (int i = 0; i < NumTotalTypeCLASS; i++) WeightsCopy[i] += this->ParaDepVar->Boundary_OffsetZ;
        result += SliceResult;
    SliceResult = 0.0;

    //if (Debug_FlagZ) {
    //  if (DebugMin[2] > z)  DebugMin[2] = z;
    //  DebugMax[2] = z;
    //  Debug_FlagZ = 0;
    // } 

    z++;
      }
    }  
  } // End of voxel

  result += SliceResult + RowResult;

  // std::cerr << "ggggggggggggr " << x << " " << y << " " << z << endl;
  // -----------------------------------------------------------
  // Clean up 
  delete[] WeightsCopy;

  // std::cerr << "Cost function of class " << result << endl; 
}




// Remember: The parameters describe the mapping from global to SuperClass  but the Matrices are inverted and therefore describe the relationship from SuperClass to Global 
// the way things are interpolated are always the following : input is image or better  superclass coordinate system - output are the coordinate in atlas space 
float EMLocalRegistrationCostFunction::ComputeCostFunction(const double* parameters) const  {
  // if (this->Debug)  std::cerr << "============ Start EMLocalRegistrationCostFunction::RegistrationCostFunction ===============" << endl;

  // --------------------------------------------------------------------------------------------------
  // Check Scaling Parameters  
  assert ((this->RegistrationType <= EMSEGMENT_REGISTRATION_SIMULTANEOUS) && (this->RegistrationType >  EMSEGMENT_REGISTRATION_APPLY));
  assert(this->Boundary_NumberOfROIVoxels);

  // Otherwise unknown registration type - or should not register
  int ScaleParameterIndex = 0;

  // make sure scaling parameters are not to small otherwise we cannot invert the matrix
  // hwen rigid registration than no scaling is involved
  if (!this->RigidFlag) {
    for (int h = 0; h < this->NumberOfParameterSets; h++) {
      // Kilian - I am debugging right now 
      if (this->TwoDFlag) {
    if ((parameters[ScaleParameterIndex +3] < EMREGISTRATION_MIN_SCALING) || (parameters[ScaleParameterIndex + 4] < EMREGISTRATION_MIN_SCALING) || 
        (parameters[ScaleParameterIndex +3] > EMREGISTRATION_MAX_SCALING) || (parameters[ScaleParameterIndex + 4] > EMREGISTRATION_MAX_SCALING)) {
      return EMLOCALREGISTRATION_MAX_PENALITY;
    }
      } else {
    if ((parameters[ScaleParameterIndex +6] < EMREGISTRATION_MIN_SCALING) || (parameters[ScaleParameterIndex + 7] < EMREGISTRATION_MIN_SCALING) || (parameters[ScaleParameterIndex + 8] < EMREGISTRATION_MIN_SCALING) 
       || (parameters[ScaleParameterIndex +6] > EMREGISTRATION_MAX_SCALING) || (parameters[ScaleParameterIndex + 7] > EMREGISTRATION_MAX_SCALING) || (parameters[ScaleParameterIndex + 8] > EMREGISTRATION_MAX_SCALING)) {
      return EMLOCALREGISTRATION_MAX_PENALITY;
    }
      }
      ScaleParameterIndex += this->NumberOfParameterPerSet;
    }
  }
  // --------------------------------------------------------------------------------------------------
  // The Turn Registration Parameters Into Rotation Matrix and Translation Vector
  // A(Structure To Atlas) = A(Global to Atlas) * A(SuperClass To Global)  * A(Structure to Superclass) 

  // Build Global Transformation Matrix
  // Note that numerical recepeces always starts at 1 ! 
  int ParameterIndex = 0;

  if (this->RegistrationType != EMSEGMENT_REGISTRATION_CLASS_ONLY) {
    float SuperClassToGlobalRotationMatrix[9];
    float SuperClassToGlobalTranslationVector[3];

    if (vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(parameters, SuperClassToGlobalRotationMatrix, SuperClassToGlobalTranslationVector, 2, this->TwoDFlag,this->RigidFlag)) {
      std::cerr << "inline float EMLocalRegistrationCostFunction::RegistrationCostFunction:: Could not calculate inverse of the following parameter setting :" << endl;
      EMLocalRegistrationCostFunction_PrintVector(parameters,0,this->NumberOfParameterPerSet-1);
      exit(1);
    } 

    vtkSimonParameterReaderWriter::matmult_3x4(this->GlobalToAtlasRotationMatrix, this->GlobalToAtlasTranslationVector,
                           SuperClassToGlobalRotationMatrix, SuperClassToGlobalTranslationVector, this->SuperClassToAtlasRotationMatrix, this->SuperClassToAtlasTranslationVector);
    ParameterIndex += this->NumberOfParameterPerSet; 
  }

  // When we only want to calculate global registration parameters
  // The identity defines the ClassToSuperClass Registration matrix 

  // They have to NULL otherwise a mistake happened and you should debug
  assert(!this->ParaDepVar->ClassToAtlasRotationMatrix);
  assert(!this->ParaDepVar->ClassToAtlasTranslationVector);



  this->ParaDepVar->ClassToAtlasRotationMatrix    = new float*[this->GetNumClasses()];
  this->ParaDepVar->ClassToAtlasTranslationVector = new float*[this->GetNumClasses()];

  if (this->RegistrationType == EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
    // Kilian: do this in here bc now we define different emhieryparameters 
    // Kilian: Note that if we do sequential global and class registration we will ignore here the structure specific registration
    // => That should stabalize registration process - but if we have problems change this here and include structrue specific registration
    for (int h = 0; h < this->GetNumClasses(); h++) {
      this->ParaDepVar->ClassToAtlasRotationMatrix[h]    = this->SuperClassToAtlasRotationMatrix;
      this->ParaDepVar->ClassToAtlasTranslationVector[h] = this->SuperClassToAtlasTranslationVector;
    }

    // Define work area 
    float ROI_Aligned_ProbData_MinCoord[3];
    float ROI_Aligned_ProbData_MaxCoord[3];

    // Figure out where to start
    // We need to invert it bc the coordinates (egistrationParameters.ROI_ProbData->M*Coord) are given in the atlas space and we need to find out what they are in the image space 
    // It is a little bit confusing but I found it out the hard way

    float AtlasToSuperClassRotationMatrix[9];
    vtkSimonParameterReaderWriter::fast_invert_3x3_matrix(this->SuperClassToAtlasRotationMatrix,AtlasToSuperClassRotationMatrix);
    float AtlasToSuperClassTranslationVector[3];
    AtlasToSuperClassTranslationVector[0] = -this->SuperClassToAtlasTranslationVector[0];
    AtlasToSuperClassTranslationVector[1] = -this->SuperClassToAtlasTranslationVector[1];
    AtlasToSuperClassTranslationVector[2] = -this->SuperClassToAtlasTranslationVector[2];
     
    // Minimum
    EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(AtlasToSuperClassRotationMatrix, AtlasToSuperClassTranslationVector, 
        this->ROI_ProbData->MinCoord[0], this->ROI_ProbData->MinCoord[1], 
        this->ROI_ProbData->MinCoord[2], ROI_Aligned_ProbData_MinCoord[0],  ROI_Aligned_ProbData_MinCoord[1], 
        ROI_Aligned_ProbData_MinCoord[2], this->Image_MidX, this->Image_MidY, this->Image_MidZ);

    // Maximum 
    EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(AtlasToSuperClassRotationMatrix, AtlasToSuperClassTranslationVector,  
       this->ROI_ProbData->MaxCoord[0], this->ROI_ProbData->MaxCoord[1], 
       this->ROI_ProbData->MaxCoord[2], ROI_Aligned_ProbData_MaxCoord[0],  ROI_Aligned_ProbData_MaxCoord[1], 
       ROI_Aligned_ProbData_MaxCoord[2], this->Image_MidX, this->Image_MidY, this->Image_MidZ);

    // Transfere to image space 
    int BoundaryMin[3];
    int BoundaryMax[3];
    int Aligned_ProbData_MinCoord;
    int Aligned_ProbData_MaxCoord;
    // Are they plus one or nor the boundary max
    for (int i = 0 ; i < 3; i++) {
      // To make sure you get the same result bc of rounding errors we just include two more dimensions than necessary
      Aligned_ProbData_MinCoord = int(ROI_Aligned_ProbData_MinCoord[i]) -2;
      Aligned_ProbData_MaxCoord = int(ceil(ROI_Aligned_ProbData_MaxCoord[i])) + 2; 
      // std::cerr <<  Aligned_ProbData_MinCoord << " " <<  Aligned_ProbData_MaxCoord << endl;
      BoundaryMin[i] = (Aligned_ProbData_MinCoord < this->ROI_Weight->MinCoord[i] ?  Aligned_ProbData_MinCoord :  this->ROI_Weight->MinCoord[i]);
      BoundaryMax[i] = (Aligned_ProbData_MaxCoord > this->ROI_Weight->MaxCoord[i] ?  Aligned_ProbData_MaxCoord :  this->ROI_Weight->MaxCoord[i]);
      assert(BoundaryMax[i] >= BoundaryMin[i]);
    }
    // set up parameters for threaded stuff
    // Not nexessary when we clas specific registration bc we do not know what BoundaryPCAMin is -> not worth it to find out bc normally we just do it for inside  the brain where there are not a lot of voxels left
    // It uses initialization from vtkImageEMLocalSegment
    this->DefineRegistrationParametersForThreadedCostFunction(BoundaryMin[0],  BoundaryMin[1],  BoundaryMin[2], BoundaryMax[0],  BoundaryMax[1],  BoundaryMax[2]);

  } else {
    float ClassToSuperClassRotationMatrix[9];
    float ClassToSuperClassTranslationVector[3];
    if (this->GenerateBackgroundProbability) {
      this->ParaDepVar->ClassToAtlasRotationMatrix[0]    = NULL;
      this->ParaDepVar->ClassToAtlasTranslationVector[0] = NULL;
    }

    for (int h = this->GenerateBackgroundProbability; h < this->GetNumClasses(); h++) {
      if (this->ClassSpecificRegistrationFlag[h]) {
    this->ParaDepVar->ClassToAtlasRotationMatrix[h] = new float[9];
    this->ParaDepVar->ClassToAtlasTranslationVector[h] = new float[3];

    if (vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(&parameters[ParameterIndex], ClassToSuperClassRotationMatrix, ClassToSuperClassTranslationVector, 2, this->TwoDFlag,this->RigidFlag)) {
      std::cerr << "inline float EMLocalRegistrationCostFunction::RegistrationCostFunction:: Could not calculate inverse of the following parameter setting :" << endl;
      EMLocalRegistrationCostFunction_PrintVector(&parameters[ParameterIndex],0,this->NumberOfParameterPerSet-1);
      exit(1);
    }

    vtkSimonParameterReaderWriter::matmult_3x4(this->SuperClassToAtlasRotationMatrix, this->SuperClassToAtlasTranslationVector, ClassToSuperClassRotationMatrix, 
                           ClassToSuperClassTranslationVector, this->ParaDepVar->ClassToAtlasRotationMatrix[h], 
                           this->ParaDepVar->ClassToAtlasTranslationVector[h]);    
    ParameterIndex += this->NumberOfParameterPerSet; 
      } else {
        // We  do not change anything ! - not part of class specific registration
    this->ParaDepVar->ClassToAtlasRotationMatrix[h]    = this->SuperClassToAtlasRotationMatrix; 
    this->ParaDepVar->ClassToAtlasTranslationVector[h] = this->SuperClassToAtlasTranslationVector;
      }
    }
  }
  // --------------------------------------------------------------------------------------------------
  // Run Cost Function

  if (this->SpatialCostFunction) memset(this->SpatialCostFunction,0,this->Boundary_LengthXYZ*sizeof(double));
  // Call Thread to start cost function
  // std::cerr << "We start now " << endl;
  this->Threader->SingleMethodExecute();
  double result = 0.0; 
  for (int i = 0; i < this->NumberOfThreads; i++) result += this->MultiThreadedParameters[i].Result;


  // -----------------------------------------------------------
  // PDF of Registration must be added to result 

  // We assume Gaussian Distribution for structure specifc parameters 
  // All Numerical Recepeise Vectors have to start at 1 ! 
  // result <= 0 so we by doing - we add to it
  double GaussianPenalty = 0.0;

  if (this->RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
    ScaleParameterIndex = (this->RegistrationType > EMSEGMENT_REGISTRATION_CLASS_ONLY ?  this->NumberOfParameterPerSet : 0);
    for (int h = this->GenerateBackgroundProbability; h < this->GetNumClasses(); h++) {
      // Only considere those classes where something was changed 
      if (this->ClassSpecificRegistrationFlag[h]) {
    if (this->TwoDFlag) {
      GaussianPenalty -= (     ClassInvCovariance[h][0] * double(parameters[ScaleParameterIndex]  * parameters[ScaleParameterIndex])
                   + ClassInvCovariance[h][1] * double(parameters[ScaleParameterIndex + 1]  * parameters[ScaleParameterIndex + 1])
                   + ClassInvCovariance[h][5] * double(parameters[ScaleParameterIndex + 2]  * parameters[ScaleParameterIndex + 2]));
      if (!this->RigidFlag) {
        GaussianPenalty -= (   ClassInvCovariance[h][6] * double((parameters[ScaleParameterIndex + 3] - 1.0)* (parameters[ScaleParameterIndex + 3] - 1.0))
                 + ClassInvCovariance[h][7] * double((parameters[ScaleParameterIndex + 4] - 1.0)* (parameters[ScaleParameterIndex + 4] - 1.0)));
      }
    } else {
      // Kilian: This is just so I can compare it to the old approach - I had a bug in here and it should be changed 
      for (int i = 0 ; i < 6; i++ ) GaussianPenalty -=  ClassInvCovariance[h][i] * double(parameters[ScaleParameterIndex + i] * parameters[ScaleParameterIndex + i]) / 1000.0;
      if (!this->RigidFlag) {
        GaussianPenalty -= (    ClassInvCovariance[h][6] * double((parameters[ScaleParameterIndex + 6] - 1.0) * (parameters[ScaleParameterIndex + 6] -1.0))
                    + ClassInvCovariance[h][7] * double((parameters[ScaleParameterIndex + 7] - 1.0) * (parameters[ScaleParameterIndex + 7] -1.0))
                    + ClassInvCovariance[h][8] * double((parameters[ScaleParameterIndex + 8] - 1.0) * (parameters[ScaleParameterIndex + 8] -1.0)));
      }
    }
    ScaleParameterIndex += this->NumberOfParameterPerSet;
      }
    }
    //* double(RegistrationParameters.Boundary_NumberOfROIVoxels)
    GaussianPenalty *= double(this->Boundary_NumberOfROIVoxels) * 1000.0/(2.0 * 600000);

  } else  {
    // Kilian : Have to later integrate it - this is just hacking right :
    // Idea beasically if we do not segment the full head the registration appraoch extends in Z direction =>
    // This adds an error rate to it   
    if (!this->TwoDFlag && !this->RigidFlag) { 
      // 3342336 is just image prod  - 100 is the same as CovPara =  0.1 
      GaussianPenalty = -(double(parameters[8] - 1.0) * 1000.0  * double(parameters[8] - 1.0)) * 1000.0/2.0;
     }
  }

  // So we can track the influence of the Gauss parameters
  if ( -(result + GaussianPenalty) <  this->GetMinCost()) {
    this->ParaDepVar->MinWeightAtlasCost = -result;
    this->ParaDepVar->MinGaussianCost    = -GaussianPenalty;
    
  }
  result += GaussianPenalty;

  // --------------------------------------------------------------------------------------------------
  // Clean up 
  if (this->RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
    for (int h = this->GenerateBackgroundProbability; h < this->GetNumClasses(); h++) {
      if (this->ClassSpecificRegistrationFlag[h]) {
    delete[] this->ParaDepVar->ClassToAtlasRotationMatrix[h];
    delete[] this->ParaDepVar->ClassToAtlasTranslationVector[h];
      }
    }
  }
  delete[] this->ParaDepVar->ClassToAtlasRotationMatrix;
  delete[] this->ParaDepVar->ClassToAtlasTranslationVector; 

  this->ParaDepVar->ClassToAtlasRotationMatrix    = NULL;
  this->ParaDepVar->ClassToAtlasTranslationVector = NULL;

  return -float(result); 
}

// Call Function before starting optimization with cost function
void  EMLocalRegistrationCostFunction::InitializeCostFunction() {
  // ----------------------------------------------
  // Info 
  // ----------------------------------------------
  switch (this->RegistrationType)  {
    case EMSEGMENT_REGISTRATION_GLOBAL_ONLY  : std::cerr << "Global Registration " << endl; break;
    case EMSEGMENT_REGISTRATION_CLASS_ONLY   : std::cerr << "Class Registration " << endl; break;
    case EMSEGMENT_REGISTRATION_SIMULTANEOUS : std::cerr << "Global + Class Registration " << endl; break;
    default : 
      std::cerr << "Unknown Registration Type " << endl;
      return;
  }
  
  // ----------------------------------------------
  // Initilize variables
  // ----------------------------------------------
  this->ResetMinCost(); 
  // Transfere into image space
  if (this->ROI_Weight) {
     this->ROI_Weight->MinCoord[0] += this->Boundary_Min[0]; 
     this->ROI_Weight->MinCoord[1] += this->Boundary_Min[1]; 
     this->ROI_Weight->MinCoord[2] += this->Boundary_Min[2]; 
     this->ROI_Weight->MaxCoord[0] += this->Boundary_Min[0]; 
     this->ROI_Weight->MaxCoord[1] += this->Boundary_Min[1]; 
     this->ROI_Weight->MaxCoord[2] += this->Boundary_Min[2];
     std::cerr << "EMLocalRegistrationCostFunction::StartRegistration:this->ROI_Weight->MinCoord " 
          << this->ROI_Weight->MinCoord[0] << " " << this->ROI_Weight->MinCoord[1] << " " << " " << this->ROI_Weight->MinCoord[2] << endl;
     std::cerr << "EMLocalRegistrationCostFunction::StartRegistration:this->ROI_Weight->MaxCoord " 
          << this->ROI_Weight->MaxCoord[0] << " " << this->ROI_Weight->MaxCoord[1] << " " << " " << this->ROI_Weight->MaxCoord[2] << endl;
  }

}

void  EMLocalRegistrationCostFunction::FinalizeCostFunction(double* Parameters, int NumOfFunctionEvaluations) {
  // ----------------------------------------------
  this->ScaleRotationValues(Parameters);

  // ----------------------------------------------
  // Print out results
  // ----------------------------------------------
  std::cerr << "Number of Evaluations :" << NumOfFunctionEvaluations << endl;
  std::cerr << "Final Result:" << endl;
  if (this->RegistrationType != EMSEGMENT_REGISTRATION_CLASS_ONLY) {
    std::cerr << "Global Parameters:    " ;EMLocalRegistrationCostFunction_PrintVector(Parameters,0,this->NumberOfParameterPerSet-1);
  }

  if (this->RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
    for (int i = (RegistrationType > EMSEGMENT_REGISTRATION_CLASS_ONLY ? 1 : 0) ; i <  this->NumberOfParameterSets ; i++) {
        std::cerr << "Structure Parameter " << i << ": ";  EMLocalRegistrationCostFunction_PrintVector(Parameters,i*this->NumberOfParameterPerSet,(i+1)*this->NumberOfParameterPerSet-1);
    }
  }

  std::cerr << "WeightAtlas Cost: " << this->GetMinWeightAtlasCost() << endl;
  std::cerr << "GaussianPenality: " << this->GetMinGaussianCost() << endl;
  std::cerr << "Min Cost :        " << this->GetMinCost() << endl;
}

// The idea is MAP defines the class, to which the voxel is assigned already 
// if MAP = -1 => then the voxel is not predefined to any structure
// MinCoord and MaxCoord define a bounding box around the region of interest 
// ClassOutside defines the class that is outside the box; 
// if ClassOutside is negative than bounding box is disabled 

 
// The following variable is beeded to define parameters for Registration Cost Function 
// Can not be part of Class definition because otherwise  
// vtkImageEMLocalSegmenter_ClassSum_WeightxProbabilityForSimplex must be part of class definition 
// -> simplex::amoeba can not evaluate it -> you get an error message when compiling 

// Take it out 
// EMLocalRegistrationCostFunction RegistrationParameters;

// #include "simplex.h"
// Just kept it so you konw how to integrate simplex algorithm 
// // Generating intital simplex for the simplex algorithm
// void vtkImageEMLocalSegmenter::SimplexInterface(float *FinalParameters, float &ParameterOffset, float &ParameterTolerance, float &ParameterMaxDifference, float &FinalCost) {
//   if (RegistrationParameters.GetRigidFlag()) {
//     std::cerr << "Have not updated this function for Rigid registration " << endl; 
//     exit(0);
//   }
// 
//   std::cerr << "Activate Simplex Algorithm" << endl;
//   int NumParaSets      =  RegistrationParameters.GetNumberOfParameterSets();
//   int NumParaPerSet    =  RegistrationParameters.GetNumberOfParameterPerSet();
//   int RegistrationType =  RegistrationParameters.GetRegistrationType();
//   int NumParaTotal     =  NumParaSets*NumParaPerSet;
// 
//   int    ResultFlag;
//   int    NumOfFunctionEvaluations = 0;
//   float  SumDifference;
//   int    Iterations  = 0;
//   float  simplex_epsilon = 0.005;
//   int    changed = 0;
//   int    Debug     =  RegistrationParameters.GetDebug();
//   int    TwoDFlag  =  RegistrationParameters.GetTwoDFlag(); 
//   int    StartScalePara =  (TwoDFlag ? 4:7);
//   double OffsetScaleFactor = 0.01;
// 
//   float SimplexOffset = ParameterOffset;
// 
//   if (Debug) {
//     std::cerr << "ParameterOffset: "  << ParameterOffset << " Tolerance: " << ParameterTolerance << " ParameterMaxDifference: " << ParameterMaxDifference << endl;
//     std::cerr << "Initial Parameters: "; vtkImageEMLocalSegmenter_PrintVector(FinalParameters,0, NumParaSets*NumParaPerSet-1);
//   }
// 
//   float *InitialParameters = new float[NumParaTotal];
//   memcpy(InitialParameters,FinalParameters, sizeof(float)*NumParaTotal);
// 
// 
//   float **SimplexParameter;
//   SimplexParameter= new float*[NumParaTotal + 1];
//   SimplexParameter --;
//   for (int i=1; i < NumParaTotal + 2 ; i++ ) {
//     SimplexParameter[i] = new float[NumParaTotal]; 
//     SimplexParameter[i] --;
//   }
//   memcpy(&SimplexParameter[1][1],FinalParameters,sizeof(float)* NumParaTotal);
//   float *SimplexCost = new float[NumParaTotal+1]; SimplexCost --;
// 
//   for (;;) {
//     // ----------------------------------------------
//     // Definie initial simplex
//     // ----------------------------------------------
//     Iterations ++;
//     SimplexCost[1] = RegistrationParameters.ComputeCostFunction(SimplexParameter[1]);
//     if (Iterations == 1) FinalCost  = SimplexCost[1];
//     int ParaIndex = 2;
//     int ParaIndexMinus =1;
//     for (int j=0; j < NumParaSets ; j++) {
//       // For Classes we want smaller offset - first parameter set defined global registration
//       if ((j == 1) && (RegistrationType == EMSEGMENT_REGISTRATION_SIMULTANEOUS)) SimplexOffset = SimplexOffset / 4.0;
//  
//       for (int k=1; k <= NumParaPerSet; k++ ) {
//     memcpy(&SimplexParameter[ParaIndex][1],&SimplexParameter[1][1], sizeof(float)*NumParaTotal);
//       
//     // Scaling is a percentage thing so offset should be defined differently
//     if (k < StartScalePara) SimplexParameter[ParaIndex][ParaIndexMinus] += SimplexOffset;
//     else {
//       SimplexParameter[ParaIndex][ParaIndexMinus] +=  SimplexOffset * OffsetScaleFactor;
//       if (SimplexParameter[ParaIndex][ParaIndexMinus] < EMREGISTRATION_MIN_SCALING) SimplexParameter[ParaIndex][ParaIndexMinus] = EMREGISTRATION_MIN_SCALING + SimplexOffset * OffsetScaleFactor ;
//     }
//     SimplexCost[ParaIndex] = RegistrationParameters.ComputeCostFunction(SimplexParameter[ParaIndex]);
//     ParaIndex ++;
//     ParaIndexMinus ++;
//       }
//     }
// 
//     // ----------------------------------------------
//     // Run simplex
//     // ----------------------------------------------
//     ResultFlag = Simplex::amoeba(SimplexParameter, SimplexCost, NumParaTotal, ParameterTolerance, RegistrationParameters.RegistrationCostFunction,  &NumOfFunctionEvaluations); 
//     std::cerr << "Number of Evaluations for " << Iterations << ". Iteration: "  << NumOfFunctionEvaluations << endl;  
//     
//     SimplexParameter[1] ++ ;
//     this->ScaleRotationValues(SimplexParameter[1]);
//     SimplexParameter[1] -- ;
// 
//     // ----------------------------------------------
//     // Update Parameter setting
//     // ----------------------------------------------
//     {
//       int index = 0; 
//       float temp;
//       SumDifference = 0.0;
// 
//       for (int j=0; j < NumParaSets ; j++) {
//     for (int k= 0 ; k < NumParaPerSet; k++ ) {
//       temp = SimplexParameter[1][index+1] - FinalParameters[index];
//       if (k >= StartScalePara) temp *= 1.0/OffsetScaleFactor; 
//       SumDifference += temp*temp; 
//       index ++;
//     }
//       }
//     }
// 
//     // Currently not activated
//     if (!changed && 0 && (SimplexCost[1] > FinalCost)) {
//       // Try opposite direction
//       // currently disabled - lets see if we needed 
//       std::cerr << ">>>> Try different Direction" << endl;
//       SimplexOffset = -SimplexOffset;
//       memcpy(&SimplexParameter[1][1], FinalParameters,sizeof(float)*NumParaTotal);
//       SimplexCost[1] = FinalCost -1.0;
// 
//       SumDifference = 1.0;
//       Iterations = 0;
//       changed = 1;
//     } else {
//       // vtkImageEMLocalSegmenter_PrintVector(SimplexParameter[1],1,NumParaSets*NumParaPerSet);
//       if (SimplexCost[1] < FinalCost) {
//     FinalCost = SimplexCost[1];
//     memcpy(FinalParameters, &SimplexParameter[1][1],sizeof(float)*NumParaTotal);
// 
//     // End of Loop condition 
//     if (SumDifference < ParameterMaxDifference ) break;
//     if (Iterations > 10) break;
//     // Kilian Do it the other way around  
//     if (RegistrationType < EMSEGMENT_REGISTRATION_SIMULTANEOUS)  SimplexOffset /= 2.0;
//     else  SimplexOffset *= 2.0;
// 
//     // See if at next generation we get the same result
//     
//     for (int i = 1 ; i <= NumParaTotal ; i++) SimplexParameter[1][i] +=  simplex_epsilon;
//       } else { break;}
//     }
//   } // End of while loop
// 
//   // ----------------------------------------------
//   // Clean Up  
//   // ----------------------------------------------
//   std::cerr << "Kilian: Change abosulte parameter difference to account for scaling" << endl;
//   float AbsoluteParameterDifference = 0.0;
//   for (int j = 0; j < NumParaTotal; j++) AbsoluteParameterDifference += fabs(InitialParameters[j] - FinalParameters[j]);
// 
//   if ((AbsoluteParameterDifference <= ParameterMaxDifference) && (ParameterOffset > EMREGISTRATION_MIN_OFFSET) && (ResultFlag || Iterations >  1)) {
//     ParameterOffset /= 2.0;
//     ParameterTolerance /= 2.0;
//     ParameterMaxDifference /= 2.0;
//   }
// 
//   delete []InitialParameters;
// 
//   SimplexCost ++;
//   delete[] SimplexCost;
//   for (int j = 1 ; j < NumParaTotal + 2; j ++ ) {
//     SimplexParameter[j] ++;
//     delete[] SimplexParameter[j];
//   }
//   SimplexParameter ++;
//   delete[] SimplexParameter;
// }

