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

#ifndef _EMLOCALREGISTRATIONCOSTFUNCTION_H_INCLUDED
#define _EMLOCALREGISTRATIONCOSTFUNCTION_H_INCLUDED 1

#include "assert.h"

#include "vtkEMSegment.h"
#include "vtkMultiThreader.h"
#include "vtkImageEMGenericClass.h" 
#include "EMLocalInterface.h" 
#include "assert.h"

// Registration Procedures originally defined by Simon but changed by me 

// The Registration Parameters 
// A(From Atlas To Structure) = A(Global To Structure) * A(Atlas to Global)

// #define EMSEGMENT_REGISTRATION_.. is defined in vtkImageEMLocalSuperClass.h

#define EMLOCALREGISTRATION_MAX_PENALITY float(1.0e20)


// --------------------------------------
// Threader Structure
// --------------------------------------
typedef struct {
  double Result;
  int Real_VoxelStart[3];
  int Boundary_VoxelOffset;
  // Job specifc number of Voxels in Image Space (not considering boundaries); 
  int ROI_NumVoxels;
  //int DebugMin[3];
  //int DebugMax[3];

} EMLocalRegistrationCostFunction_MultiThreadedParameters;

// --------------------------------------
// Cost Function Related Structure
// The parameters are updated whenever the cost function is called 
// --------------------------------------
typedef struct {
  float** ClassToAtlasRotationMatrix;   
  float** ClassToAtlasTranslationVector; 

  // -----------------------------------------------------
  // Registration Specific Region - Defined by Voxel that can influence the Registration 
  int     ROI_MinZ;
  int     ROI_MinY;
  int     ROI_MinX;

  int     ROI_MaxZ;
  int     ROI_MaxY;
  int     ROI_MaxX;

  // Defined By difference between length of the Segmentation and Registration Space
  int     Boundary_OffsetY;
  int     Boundary_OffsetZ;

  // MinWeightAtlasCost =sum_x SpatialCostFunction(x)
  double MinWeightAtlasCost;
  // MinGaussianCost = G(parameters,ClassInvCovariance)
  double MinGaussianCost;
} EMLocalRegistrationCostFunction_IterationSpecificVariables;


// --------------------------------------
// ROI Related Structure
// Defines Region of interest for the cost function 
// --------------------------------------
class EMLocalRegistrationCostFunction_ROI {
  public:
  char *MAP;
  int MinCoord[3];
  int MaxCoord[3];
  char ClassOutside;
  void CreateMAP(int size);

  EMLocalRegistrationCostFunction_ROI() { this->MAP = NULL; memset(MaxCoord,0,sizeof(int)*3); memset(MinCoord,0,sizeof(int)*3);ClassOutside = -1;}
  ~EMLocalRegistrationCostFunction_ROI() {if (this->MAP) {delete[] this->MAP; this->MAP = NULL;}}
}; 

// --------------------------------------
// ROI Related Structure
// Main Class 
// --------------------------------------
class VTK_EMSEGMENT_EXPORT EMLocalRegistrationCostFunction { 
public:
  // -----------------------------------------------
  // Core Functions 
 
 // Call this function before starting optimization
  void  InitializeCostFunction();

  // This calculates the metric defined by KL Divergence.
  // Call this function by optimization method 
  float ComputeCostFunction(const double* parameters) const;

  // Call this function when optimization is ended
  void  FinalizeCostFunction(double* Parameters, int NumOfFunctionEvaluations); 
  
  // Is called from threaded function
  void CostFunction_Sum_WeightxProbability(int Real_VoxelStart[3], int Boundary_VoxelOffset, int ROI_NumVoxels, double &result);

  // -----------------------------------------
  // Setting Parameters 
  int GetROI_MinZ(){return this->ParaDepVar->ROI_MinZ;}
  int GetROI_MinY(){return this->ParaDepVar->ROI_MinY;}
  int GetROI_MinX(){return this->ParaDepVar->ROI_MinX;}
  int GetROI_MaxZ(){return this->ParaDepVar->ROI_MaxZ;}
  int GetROI_MaxY(){return this->ParaDepVar->ROI_MaxY;}
  int GetROI_MaxX(){return this->ParaDepVar->ROI_MaxX;}

  int GetBoundary_OffsetY(){return this->ParaDepVar->Boundary_OffsetY;}
  int GetBoundary_OffsetZ(){return this->ParaDepVar->Boundary_OffsetZ;}
   
  // Boudnary is the area defined by SegmentationBoundary 
  void SetBoundary(int MinX, int MinY, int MinZ, int MaxX, int MaxY, int MaxZ); 
  int* GetBoundary_Min() {return this->Boundary_Min;}
  int* GetBoundary_Max() {return this->Boundary_Max;}
  int  GetBoundary_LengthX() {return this->Boundary_LengthX;}
  int  GetBoundary_LengthY() {return this->Boundary_LengthY;} 
  int  GetBoundary_LengthXYZ() {return this->Boundary_LengthXYZ;} 

  // Real is the area defined by Extent of image data
  void SetImage_Length(int LengthX, int LengthY, int LengthZ);
  int* GetImage_Length() {return this->Image_Length;}
 
  // Corresponds to Simon with Image_MidX = targetmidcol
  float GetImage_MidX() {return this->Image_MidX;}
  float GetImage_MidY() {return this->Image_MidY;}
  float GetImage_MidZ() {return this->Image_MidZ;}

  // Parameter related function 
  void SetDimensionOfParameter(int inNumberOfParameterSets, int inTwoDFlag, int inRigidFlag);

  int GetRigidFlag() {return this->RigidFlag;}
  int GetTwoDFlag() {return this->TwoDFlag;}

  void SetNumberOfParameterSets(int init) {this->NumberOfParameterSets = init;} 
  int  GetNumberOfParameterSets() {return this->NumberOfParameterSets;} 
  int  GetNumberOfParameterPerSet() {return this->NumberOfParameterPerSet;}
  int  GetNumberOfTotalParameters() { return this->NumberOfParameterSets*this->NumberOfParameterPerSet; }

  // Utility Function 
  void SetROI_Weight(EMLocalRegistrationCostFunction_ROI* init) {this->ROI_Weight = init;}
  void SetROI_ProbData(EMLocalRegistrationCostFunction_ROI* init) {this->ROI_ProbData = init;}

  void InitializeParameters();

  void ClassInvCovariance_Define(classType* ClassListType, void** ClassList);
  void ClassInvCovariance_Print();
  void ClassInvCovariance_Delete();

  void DefineRegistrationParametersForThreadedCostFunction(int ROI_MinX, int ROI_MinY, int ROI_MinZ, int ROI_MaxX, int ROI_MaxY, int ROI_MaxZ) const ;

  // -----------------------------
  // Registration Specifc Parameters

  // Global Subject (image) corrdinate system to Atlas Coordinate system
  // Rember the parameters just describe it the other way around 

  void SetGlobalToAtlasRotationMatrix(float* init)    {this->GlobalToAtlasRotationMatrix = init;}
  void SetGlobalToAtlasTranslationVector(float* init) {this->GlobalToAtlasTranslationVector = init;}

  void SetSuperClassToAtlasRotationMatrix(float *init) {this->SuperClassToAtlasRotationMatrix = init;}
  void SetSuperClassToAtlasTranslationVector(float *init) {this->SuperClassToAtlasTranslationVector = init;}

  void SetIndependentSubClassFlag(int* init) {this->IndependentSubClassFlag = init;}
  void SetClassSpecificRegistrationFlag(int*init) {this->ClassSpecificRegistrationFlag = init;}

  int  GetRegistrationType() {return RegistrationType;} 
  void SetRegistrationType(int init) {this->RegistrationType = init;} 

  void SetInterpolationType(int init) {this->InterpolationType = init;}
  void SetGenerateBackgroundProbability(int init) { this->GenerateBackgroundProbability = init;}
  void SetNumberOfTrainingSamples(int init) {this->NumberOfTrainingSamples = NumberOfTrainingSamples;}

  void SetBoundary_NumberOfROIVoxels(int init) {this->Boundary_NumberOfROIVoxels = init;}

  // For debugging purposes 
  void DebugOn(){this->Debug = 1;}   
  void DebugOff(){this->Debug = 0;}   

  // -----------------------------
  // Class Specifc Parmeters
  void SetEMHierarchyParameters(EMLocal_Hierarchical_Class_Parameters init) {this->EMHierarchyParameters.Copy(init);}  
  void Setweights(float** init)            {this->weights = init;}
  void SetBoundary_ROIVector(unsigned char * init) {this->Boundary_ROIVector = init;}

  void MultiThreadDelete();
  void MultiThreadDefine(int DisableFlag); 

  // -----------------------------
  // Do you want to Keep the result for each voxel 
  void    SpatialCostFunctionOn(); 
  double* GetSpatialCostFunction() {return this->SpatialCostFunction;} 
  void    SpatialCostFunctionOff(); 

  // Default functions
  EMLocalRegistrationCostFunction() {this->InitializeParameters();}
  ~EMLocalRegistrationCostFunction();

  // =====================
  int GetNumberOfThreads() { return this->NumberOfThreads;} 
  EMLocalRegistrationCostFunction_MultiThreadedParameters* GetMultiThreadedParameters() {return MultiThreadedParameters;}

  int    GetProbDataType() { return this->EMHierarchyParameters.ProbDataType;}
  void   SetProbDataPtr(void** init)  {this->ProbDataPtr = init;}
  void** GetProbDataPtr()            {return this->ProbDataPtr;}
  void*  GetProbDataPtr(int index) {return this->ProbDataPtr[index];}

  int GetDebug() {return this->Debug;}
 
  unsigned char* GetBoundary_ROIVector() {return this->Boundary_ROIVector;}
  int GetBoundary_NumberOfROIVoxels() {return this->Boundary_NumberOfROIVoxels;}
 
  int    GetNumClasses() const    {return this->EMHierarchyParameters.NumClasses; } 
  int*   GetNumChildClasses()     {return this->EMHierarchyParameters.NumChildClasses; } 
  int    GetNumTotalTypeCLASS()   {return this->EMHierarchyParameters.NumTotalTypeCLASS;}
  int*   GetProbDataIncY()        {return this->EMHierarchyParameters.ProbDataIncY;}
  int*   GetProbDataIncZ()        {return this->EMHierarchyParameters.ProbDataIncZ;}
  float* GetProbDataMinusWeight() {return this->EMHierarchyParameters.ProbDataMinusWeight;}
  float* GetProbDataWeight ()     {return this->EMHierarchyParameters.ProbDataWeight;}

  EMLocalRegistrationCostFunction_ROI* GetROI_Weight() {return this->ROI_Weight;}
  EMLocalRegistrationCostFunction_ROI* GetROI_ProbData() {return this->ROI_ProbData;}

  int GetGenerateBackgroundProbability() {return this->GenerateBackgroundProbability;}
  int* GetClassSpecificRegistrationFlag() {return this->ClassSpecificRegistrationFlag;}

  vtkMultiThreader* GetThreader() { return this->Threader;}

  double **GetClassInvCovariance() {return this->ClassInvCovariance;}

  double GetMinCost() const {return this->ParaDepVar->MinWeightAtlasCost + this->ParaDepVar->MinGaussianCost;} 
  double GetMinWeightAtlasCost() {return this->ParaDepVar->MinWeightAtlasCost;} 
  double GetMinGaussianCost() {return this->ParaDepVar->MinGaussianCost;} 

  void ResetMinWeightAtlasCost() {this->ParaDepVar->MinWeightAtlasCost = EMLOCALREGISTRATION_MAX_PENALITY; }
  void ResetMinCost() {this->ResetMinWeightAtlasCost();this->ParaDepVar->MinGaussianCost =0;}  

  float** Getweights() { return this->weights; }

  int* GetIndependentSubClassFlag() {return this->IndependentSubClassFlag;}
  int GetNumberOfTrainingSamples() {return this->NumberOfTrainingSamples;}

  int  GetInterpolationType() {return this->InterpolationType;}

private:
  // Utility Functions
  void ScaleRotationValues(double *FinalParameters);

  void **ProbDataPtr;

  // -----------------------------
  // Image Specific Region - defined by Extent (For ProbData)
  int     Image_Length[3]; 

  // MidX = targetmidcol in simon code
  // MidY = targetmidRow in simon code
  // MidZ = targetmidSlice in simon code
  float   Image_MidX; 
  float   Image_MidY;
  float   Image_MidZ;

  // --------------------------------------------------------
  // Segmentation Specific Region - defined by SegmentationBoundary* (such as w_m)  
  int     Boundary_Min[3];
  int     Boundary_Max[3];
  
  int     Boundary_LengthX;
  int     Boundary_LengthY;
  int     Boundary_LengthXYZ;

  // Dimension of Boundary_ROIVector is defined by Image_Length -> Defines at each voxel if is it part of the segmentation environment or not 
  unsigned char *Boundary_ROIVector;
  // Number of Voxels in Boundary_ROIVector that belong to the Region of Interest in the Segmentation Environment
  int    Boundary_NumberOfROIVoxels;

  // Each voxel is characterized by the following values 
  //  >-1 = one class is only present => can ignore it for registration if Weight and ProbData agree
  //  -1  = None of the classes have values greater zero
  //  -2  = Multiple classes have values greater zero 
  // Makes the algorithm a little bit faster 
  EMLocalRegistrationCostFunction_ROI* ROI_Weight;
  EMLocalRegistrationCostFunction_ROI* ROI_ProbData;


  // -----------------------------------------------------
  // Parameter Dimensions 

  // How many sets of Registration Parameters do we have to maximize
  int    NumberOfParameterSets;
  // How many paramters  per set 
  // => Total Number of parameters to be optimized = NumberOfParameterSets * NumberOfParameterPerSet 
  int    NumberOfParameterPerSet; 
  // The value of NumberOfParameterPerSet is defined by (TwoDFlag, RigidFlag)
  // (1,1) = 3, (1,0) = 5, (0,1) = 6, (1,1) = 9 
  int    TwoDFlag;
  int    RigidFlag;

  // Global Subject (image) corrdinate system to Atlas Coordinate system
  // Rember the parameters just describe it the other way around 

  //Define affine mapping between two spaces 
  float  *GlobalToAtlasRotationMatrix;
  float  *GlobalToAtlasTranslationVector;

  float  *SuperClassToAtlasRotationMatrix;
  float  *SuperClassToAtlasTranslationVector;

  // This is necessary so that we can itegrate our method into itk
  EMLocalRegistrationCostFunction_IterationSpecificVariables *ParaDepVar;

  // -----------------------------
  // Registration Specifc Parameters

  // Is registration performed for each subclass individually or together
  int   *IndependentSubClassFlag;

  // A class is defined by its own set of registration parameters if ClassSpecificRegistrationFlag is set
  int   *ClassSpecificRegistrationFlag;

  // RegistrationType can be of the form:
  // EMSEGMENT_REGISTRATION_GLOBAL_ONLY, EMSEGMENT_REGISTRATION_CLASS_ONLY, EMSEGMENT_REGISTRATION_SIMULTANEOUS
  int    RegistrationType;

  // InterpolationType can be of the form:
  // EMSEGMENT_REGISTRATION_INTERPOLATION_NEIGHBOUR, EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR
  int    InterpolationType; 

  int    GenerateBackgroundProbability;
  
  // For debugging purposes 
  int    Debug;  

  // -----------------------------
  // Captures the cost at each voxel 
  double *SpatialCostFunction;

  // Global Cost Values defined by the transformation (capturered by parameters) minimizing it  
  // MinCost = MinWeightAtlasCost + MinGaussianCost
  //double MinCost;
  // MinWeightAtlasCost =sum_x SpatialCostFunction(x)
  //double MinWeightAtlasCost;
  // MinGaussianCost = G(parameters,ClassInvCovariance)
  //double MinGaussianCost;
  double **ClassInvCovariance;

  // -----------------------------
  // Thread Specifc Parameters 
  int NumberOfThreads;
  EMLocalRegistrationCostFunction_MultiThreadedParameters* MultiThreadedParameters;
  vtkMultiThreader *Threader;

  // -----------------------------
  // Input to Optimization Function - Defined by Segmentation Environment
  EMLocal_Hierarchical_Class_Parameters EMHierarchyParameters;
  float** weights;
  int     NumberOfTrainingSamples;
};

template <class T> 
inline void EMLocalRegistrationCostFunction_DefineROI_ProbDataValues(EMLocalRegistrationCostFunction* self, T** ProbDataPtr) {
  // You have to first create the MAP // => it has to be defined 
  assert(((T**) self->GetProbDataPtr()) ==  ProbDataPtr);

  int *Image_Length = self->GetImage_Length();

  EMLocalRegistrationCostFunction_ROI* ROI_ProbData = self->GetROI_ProbData();
  assert(ROI_ProbData);
  if (!ROI_ProbData->MAP)  ROI_ProbData->CreateMAP(self->GetImage_Length()[0]*self->GetImage_Length()[1]*self->GetImage_Length()[2]);

  ROI_ProbData->MinCoord[0] = Image_Length[0];
  ROI_ProbData->MinCoord[1] = Image_Length[1];
  ROI_ProbData->MinCoord[2] = Image_Length[2];
  ROI_ProbData->MaxCoord[0] = ROI_ProbData->MaxCoord[1] = ROI_ProbData->MaxCoord[2] = 0;
  ROI_ProbData->ClassOutside = -1;


  char ProbDataFlagX;
  int  ProbDataFlagY;
  int  ProbDataFlagZ = 0;

  int  NumTotalTypeCLASS  = self->GetNumTotalTypeCLASS();
  int  NumClasses         = self->GetNumClasses();
  int* NumChildClasses    = self->GetNumChildClasses();

  int  ClassStart = self->GetGenerateBackgroundProbability(); 
  T** ProbDataPtrCopy = new T*[NumTotalTypeCLASS];
  char *MAP = ROI_ProbData->MAP;

  int* ProbDataIncY = self->GetProbDataIncY();
  int* ProbDataIncZ = self->GetProbDataIncZ();

  int index = 0;
  ProbDataPtrCopy[0] = ProbDataPtr[0];
  for (int i = ClassStart; i  < NumClasses; i++) {
    ProbDataFlagX = 0;
    for (int k = 0; k < NumChildClasses[i]; k++) {
      ProbDataPtrCopy[index] = ProbDataPtr[index];
      if (!ProbDataPtrCopy[index] || *ProbDataPtrCopy[index] > 0.0) ProbDataFlagX = 1;
      index ++;
    }
    if (ProbDataFlagX) {
      if (ROI_ProbData->ClassOutside > -1) {
        ROI_ProbData->ClassOutside = -3;
    break;
      }
      else ROI_ProbData->ClassOutside = i;
    }
  }
  //  cout << "Here " << endl;
  for (int z = 0; z < Image_Length[2] ; z++) {
    // cout << z << endl;
    ProbDataFlagZ = 0;
    for (int y = 0; y <  Image_Length[1] ; y++) {
      ProbDataFlagY = 0;
      // cout << y << endl;
      for (int x = 0; x < Image_Length[0]; x++) {
    index = (self->GetGenerateBackgroundProbability() ? NumChildClasses[0] : 0);
        *MAP = -1;
    for (int i = ClassStart; i  < NumClasses; i++) {
      ProbDataFlagX = 0;
      for (int k = 0; k < NumChildClasses[i]; k++) {
        if (!ProbDataPtrCopy[index] || *ProbDataPtrCopy[index] > 0.0) ProbDataFlagX = 1;
        index ++;
      }
      if (ProbDataFlagX) {
        if (*MAP > -1) {
          *MAP = -1;
          break;
        } else  *MAP = i;   
      }
    }
    if (*MAP != ROI_ProbData->ClassOutside) {
      ProbDataFlagY = 1;
      ProbDataFlagZ = 1;
      if (ROI_ProbData->MinCoord[0] > x)  {
        ROI_ProbData->MinCoord[0] = x;
      }
      if (ROI_ProbData->MaxCoord[0] < x) {
        ROI_ProbData->MaxCoord[0] = x;
      }
    }
    MAP++;
    for (int j=0; j < NumTotalTypeCLASS; j++) { if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] ++; }
      }
      if (ProbDataFlagY) {
    if (ROI_ProbData->MinCoord[1] > y)  ROI_ProbData->MinCoord[1] = y;
    if (ROI_ProbData->MaxCoord[1] < y)  ROI_ProbData->MaxCoord[1] = y;
      }
      for (int j=0; j < NumTotalTypeCLASS; j++) { if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] += ProbDataIncY[j]; }
    }
    if (ProbDataFlagZ) {
      if (ROI_ProbData->MinCoord[2] > z)  ROI_ProbData->MinCoord[2] = z;
      ROI_ProbData->MaxCoord[2] = z;
    }
    for (int j=0; j < NumTotalTypeCLASS; j++) { if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] += ProbDataIncZ[j]; }
  }
  // Should not be necessary
  // for (int i = 0; i  < NumTotalTypeCLASS; i++)  ProbDataPtrCopy[i] = ProbDataPtr[i];

  delete[]  ProbDataPtrCopy;
}


// ===========================================================
//
//


#endif
