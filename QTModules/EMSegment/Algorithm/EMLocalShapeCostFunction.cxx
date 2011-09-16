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
// All the functions used with respect to introduce shape 
#include "EMLocalShapeCostFunction.h"
#include "assert.h"
#define EMPRIVATE_HEAVISIDE_FCT_FLAG 0



/* Foreward Defintion */
     template  <class Tin> 
inline void EMLocalShapeCostFunction_CalculateCostFunction(EMLocalShapeCostFunction *Shape, Tin **ProbDataPtrOrig, int *VoxelStart, int NumberOfVoxels, 
                                                           int DataJump, int *PCAMeanShapeJump, int **PCAEigenVectorsJump, int *ProbDataJump, float &result);

//----------------------------------------------------------------------------------
// Utility functions
//----------------------------------------------------------------------------------
template  <class Tin> void EMLocalShapeCostFunction_AssignProbDataPtr(EMLocalShapeCostFunction *self, Tin  **ProbDataPtr,int DataJump, int ClassIndex)
{
  self->SetProbDataPtr((void*) (ProbDataPtr[ClassIndex] + DataJump),ClassIndex);
}

// Multi Threading Interface
// This function is called by this->Threader->SingleMethodExecute(). Also look at EMLocalShapeCostFunction::EMLocalShapeCostFunction for definition of this->Threader
VTK_THREAD_RETURN_TYPE EMLocalShapeCostFunction_ShapeCostFunctionMultiThreaded_Function(void *arg)
{
  int CurrentThread = ((ThreadInfoStruct*)(arg))->ThreadID;
  EMLocalShapeCostFunction* shape = (EMLocalShapeCostFunction*) (((ThreadInfoStruct*)(arg))->UserData);
  
  // ID is to high for our setup 
  assert(CurrentThread < shape->GetNumOfThreads()); 
  EMLocalShapeCostFunction_MultiThreadedParameters* ThreadedParameters = &(shape->GetMultiThreadedParameters()[CurrentThread]);
  
  ThreadedParameters->Result = 0.0;
  switch (shape->GetProbDataType())
    {                  
    vtkTemplateMacro(EMLocalShapeCostFunction_CalculateCostFunction(shape, (VTK_TT**) shape->GetProbDataPtr(), ThreadedParameters->VoxelStart, 
                                                                    ThreadedParameters->NumberOfVoxels, ThreadedParameters->DataJump, ThreadedParameters->PCAMeanShapeJump, ThreadedParameters->PCAEigenVectorsJump, 
                                                                    ThreadedParameters->ProbDataJump, ThreadedParameters->Result)); 
    default :
      std::cerr << "Warning: EMLocalShapeCostFunction_ShapeCostFunctionMultiThreaded_Function : unknown data type " << shape->GetProbDataType() << endl;
    }
  return VTK_THREAD_RETURN_VALUE;
}


//----------------------------------------------------------------------------------
// Core Function
//----------------------------------------------------------------------------------
// Published in ICCV 05
// Calculat the value of the cost function in dependency of the parameter Shape
template  <class Tin> 
inline void EMLocalShapeCostFunction_CalculateCostFunction(EMLocalShapeCostFunction *Shape, Tin **ProbDataPtrOrig, int *VoxelStart, int NumberOfVoxels, 
                                                           int DataJump, int *PCAMeanShapeJump, int **PCAEigenVectorsJump, int *ProbDataJump, float &result)
{

  // std::cerr << "Start vtkImageEMLocalSegmenter_CalculateCostFunction " << endl;
  // --------------------------------------
  // Registration Parameters 
  // --------------------------------------
  
  const double* ShapePara = Shape->GetPCAPara();

  float** ClassToAtlasRotationMatrix    = Shape->ClassToAtlasRotationMatrix;
  float** ClassToAtlasTranslationVector = Shape->ClassToAtlasTranslationVector;

  float targetmidcol     = Shape->GetImage_MidX(); 
  float targetmidrow     = Shape->GetImage_MidY();
  float targetmidslice   = Shape->GetImage_MidZ();
  int   RegistrationType = Shape->GetRegistrationType();

  // --------------------------------------
  // Initialiaze Parameters 
  // --------------------------------------
  int NumClasses        = Shape->GetNumClasses();
  int *NumChildClasses  = Shape->GetNumChildClasses();
  int NumberOfTotalTypeCLASS = Shape->GetNumberOfTotalTypeCLASS();
  int PCAShapeModelType = Shape->PCAShapeModelType;
  float **weights = new float*[NumberOfTotalTypeCLASS];
  for (int i = 0; i < NumberOfTotalTypeCLASS; i++)
    {
    weights[i] = Shape->Getweights(i) + DataJump;
    }
  int weightsIncY = Shape->GetweightsIncY();
  int weightsIncZ = Shape->GetweightsIncZ();


  //std::cerr << "Class parameters" << endl;
  //for (int j = 0; j  < NumClasses; j++) {
  // std::cerr << j << endl;
  //  std::cerr << "Class " << j << " " ; vtkImageEMLocalSegmenter_PrintVector(ClassToAtlasRotationMatrix[j],0,8); 
  //  vtkImageEMLocalSegmenter_PrintVector(ClassToAtlasTranslationVector[j],0,2); 
  // }


  // int ROI_MaxZ = Shape->GetROI_MaxZ();
  int ROI_MaxY = Shape->GetROI_MaxY();
  int ROI_MaxX = Shape->GetROI_MaxX();
  // int ROI_MinZ = Shape->GetROI_MinZ();
  int ROI_MinY = Shape->GetROI_MinY();
  int ROI_MinX = Shape->GetROI_MinX();

  Tin **ProbDataPtr = new Tin*[NumberOfTotalTypeCLASS];
  for (int i = 0; i < NumberOfTotalTypeCLASS; i++)
    {
    ProbDataPtr[i] = ProbDataPtrOrig[i];
    if (ProbDataPtr[i])
      {
      ProbDataPtr[i] += ProbDataJump[i];
      }
    }
  int *ProbDataIncY = Shape->GetProbDataIncY();
  int *ProbDataIncZ = Shape->GetProbDataIncZ();
  int *PCANumberOfEigenModes = Shape->GetPCANumberOfEigenModes();

  float ***PCAEigenVectorsPtr    = new float**[NumberOfTotalTypeCLASS];
  for (int i = 0; i < NumberOfTotalTypeCLASS; i++)
    {
    if (PCANumberOfEigenModes[i])
      {
      PCAEigenVectorsPtr[i] = new float*[PCANumberOfEigenModes[i]];
      for (int j = 0; j < PCANumberOfEigenModes[i]; j++)
        {
        PCAEigenVectorsPtr[i][j]  = Shape->GetPCAEigenVectorsPtr(i,j) + PCAEigenVectorsJump[i][j];
        }
      }
    else
      {
      PCAEigenVectorsPtr[i] = NULL;
      }
    }

  float  **PCAMeanShapePtr       = new float*[NumberOfTotalTypeCLASS]; 
  for (int i = 0; i < NumberOfTotalTypeCLASS; i++)
    {
    PCAMeanShapePtr[i] = Shape->GetPCAMeanShapePtr(i);
    if (PCAMeanShapePtr[i]) PCAMeanShapePtr[i] += PCAMeanShapeJump[i]; 
    }
  int   **PCAEigenVectorsIncY   = Shape->GetPCAEigenVectorsIncY(); 
  int   **PCAEigenVectorsIncZ   = Shape->GetPCAEigenVectorsIncZ(); 
  int    *PCAMeanShapeIncY      = Shape->GetPCAMeanShapeIncY();
  int    *PCAMeanShapeIncZ      = Shape->GetPCAMeanShapeIncZ();


  float *PCALogisticSlope      =  Shape->PCALogisticSlope;
  float *PCALogisticBoundary   =  Shape->PCALogisticBoundary;
  float *PCALogisticMin        =  Shape->PCALogisticMin;
  float *PCALogisticMax        =  Shape->PCALogisticMax;

  char *IncludeSpatialPriorForNormalization = Shape->GetIncludeSpatialPriorForNormalization();

  float* SpatialCostFunction = Shape->GetSpatialCostFunction();
  if (SpatialCostFunction) SpatialCostFunction += Shape->GetDataJump() + DataJump;
  int SpatialCostFunctionIncY = Shape->GetDataIncY();
  int SpatialCostFunctionIncZ = Shape->GetDataIncZ();

  unsigned char *ROI = Shape->GetROI() + DataJump;
  int ROIIncY = Shape->GetROIIncY();
  int ROIIncZ = Shape->GetROIIncZ();

  int ShapeIndex = 0;
  // --------------------------------------
  // Determin penalty over all voxels of the image
  // --------------------------------------
  double LogShapeEpsilon = log(0.00001);
  double SpatialPrior;
  double SumVoxelDenominator;
  double SumVoxelNumerator; 

  // To reduce Compuational errors ! 
  double SumVoxelPenality;
  double SumRowPenalty = 0.0;
  double SumSlicePenalty = 0.0;
  double SumImagePenalty = 0.0;


  // The Cost function we calculate here is 

  // Q (S) = \sum_x (\sum_a W(x,a) log(SpatialPrior(x,a))) - log(\sum_a SpatialPrior(x,a)) 
  //       = \sum_x (SumVoxelNumerator(x) - log(SumVoxelDenominator(x)))  = \sum_x SumVoxelPeanlity(x) = SumImagePenalty 
  int z =  VoxelStart[2];
  int y =  VoxelStart[1];
  int x =  VoxelStart[0];
  int VoxelIndex = 0;
  int VoxelJump  = 0; 
  int ClassIndex;
  float targetX, targetY, targetZ;
  float VoxelDistanceMapValue;

  // std::cerr << "---- " << RegistrationType << " " << targetmidcol << " " <<  targetmidrow << " " << targetmidslice << " [[[[ " << ROI_MinZ << " " << ROI_MinY << " " << ROI_MinX <<" +++ " << ROI_MaxZ - ROI_MinZ +1 << " " << ROI_MaxY - ROI_MinY + 1<< " " << ROI_MaxX - ROI_MinX +1 << " " << VoxelIndex << " " << NumberOfVoxels << " " << z << " " << y << " " << x << " " << endl;

  // std::cerr << " Take out debugging stuff " << endl;
  // std::cerr << "NumClassses " << NumClasses << endl;
  //if (RegistrationType >  EMSEGMENT_REGISTRATION_DISABLED) {
  //  std::cerr << "Registration Applied to Atlas Space:" << endl;
  //  std::cerr << "Global Matrix: "; vtkImageEMLocalSegmenter_PrintVector(ClassToAtlasRotationMatrix[NumClasses -1],0,8); vtkImageEMLocalSegmenter_PrintVector(ClassToAtlasTranslationVector[NumClasses -1],0,2);
  // }
  
  while (VoxelIndex < NumberOfVoxels)
    {
    // Debugging right now 
    // std::cerr << VoxelIndex << endl;
    if (*ROI)
      {
      ShapeIndex = 0;
      // The Numerator of the weighted sum in dependency of the voxel
      SumVoxelNumerator = 0.0;
      // The Denominator of the weighted sum in dependency of the voxel
      SumVoxelDenominator = 0.0;
      if (RegistrationType ==  EMSEGMENT_REGISTRATION_GLOBAL_ONLY)
        {
        EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(ClassToAtlasRotationMatrix[NumClasses -1], ClassToAtlasTranslationVector[NumClasses -1], 
                                                                       x, y, z, targetX, targetY, targetZ,targetmidcol, targetmidrow, targetmidslice);
        VoxelJump = EMLocalInterface_InterpolationNearestNeighbourVoxelIndex(targetX, targetY, targetZ, 0,0, Shape->GetImage_Length());
        }
      ClassIndex = 0;
      for (int j = 0; j  < NumClasses; j++)
        {
        // std::cerr << j << endl;
        if ((RegistrationType >  EMSEGMENT_REGISTRATION_DISABLED) && (RegistrationType !=  EMSEGMENT_REGISTRATION_GLOBAL_ONLY))
          {
          EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(ClassToAtlasRotationMatrix[j], ClassToAtlasTranslationVector[j], 
                                                                         x, y, z, targetX, targetY, targetZ,targetmidcol, targetmidrow, targetmidslice);
         
          VoxelJump = EMLocalInterface_InterpolationNearestNeighbourVoxelIndex(targetX, targetY, targetZ, 0,0, Shape->GetImage_Length());
          }
        for (int k = 0; k  < NumChildClasses[j]; k++)
          {
          // std::cerr << k << endl;
          if (PCANumberOfEigenModes[ClassIndex])
            {
            // Kilian: Later create a map of the coordinates that can just be looked up
            // define all these parameters - just do nearest neighbor instead does not matter so much- or recaluclate spatial priors and then just look it up
            // you have to interpolate - which is nasty- bc you do it over 8 value
            VoxelDistanceMapValue =  EMLocalInterface_CalcDistanceMap(&ShapePara[ShapeIndex], PCAEigenVectorsPtr[ClassIndex], PCAMeanShapePtr[ClassIndex],  PCANumberOfEigenModes[ClassIndex],VoxelJump);
            SpatialPrior = double(Shape->Transfere_DistanceMap_Into_SpatialPrior(VoxelDistanceMapValue, PCALogisticSlope[ClassIndex], PCALogisticBoundary[ClassIndex], PCALogisticMin[ClassIndex], PCALogisticMax[ClassIndex])); 

            if (PCAShapeModelType == EMSEGMENT_PCASHAPE_INDEPENDENT)
              { ShapeIndex += PCANumberOfEigenModes[ClassIndex];}
      
            // Be Careful AlignedTissueDistribution \in [0,NumberOfTrainingSamples]
            if (*weights[ClassIndex] > 0.0){
            SumVoxelNumerator += double(*weights[ClassIndex]) * (SpatialPrior > 0.0 ? log(SpatialPrior) : LogShapeEpsilon); 
            }
            SumVoxelDenominator += SpatialPrior;
            }
          else if (ProbDataPtr[ClassIndex])
            {
            SpatialPrior = double(*(ProbDataPtr[ClassIndex] + VoxelJump));

            // Theortical you only have to consider the structures that are shape dependent for the numerator bc the other ones are constant and therefore do not count 
            // But our cost function will mostly be based on those voxels where no structure dependent parameters are present => introduce computational errors
            if (SpatialPrior > 0.0)
              {
              if  (*weights[ClassIndex] > 0.0) SumVoxelNumerator += double(*weights[ClassIndex]) * log(SpatialPrior);
              if (IncludeSpatialPriorForNormalization[ClassIndex]) SumVoxelDenominator += SpatialPrior;
              }
            }
          ClassIndex ++;
          }
        }
      // std::cerr << "Finshed " << endl;
      // Only considere those voxels, where the weights of structures with shape parameters are greater zero 
      // Sumvoxeldenominator not correctly de
      if (SumVoxelDenominator > 0.0)
        { 
        SumVoxelPenality = SumVoxelNumerator - log(SumVoxelDenominator);
    
        // If smaller than they are just numerical errors => do not consider it 
        if (fabs(SumVoxelPenality) > 0.0001)
          {
          SumRowPenalty += SumVoxelPenality;
          if (SpatialCostFunction)  *SpatialCostFunction  = -SumVoxelPenality;
          // if (SpatialCostFunction)  *SpatialCostFunction  = -SumVoxelPenality + 1.0;
          }
        }
      } // end of if (*ROI) .. 
    // Go to next x value 
    // Debug  
    // *SpatialCostFunction  = SumRowPenalty; 

    VoxelIndex ++;
    x ++;
    ROI++;
    if (SpatialCostFunction) SpatialCostFunction ++;

    if (RegistrationType ==  EMSEGMENT_REGISTRATION_DISABLED)
      {
      for (int i = 0; i < NumberOfTotalTypeCLASS; i++)
        {
        if (PCAEigenVectorsPtr[i])
          {
          for (int j = 0 ; j <  PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] ++; }
        if (PCAMeanShapePtr[i]) PCAMeanShapePtr[i] ++;
        if (ProbDataPtr[i]) ProbDataPtr[i] ++;
        }
      }
    for (int i = 0; i < NumberOfTotalTypeCLASS; i++) weights[i] ++;

    //assert(z <= ROI_MaxZ);
    if (x >  ROI_MaxX)
      {
      // Go to next y row
      x = ROI_MinX;
      y ++;

      ROI += ROIIncY;
      if (SpatialCostFunction) SpatialCostFunction += SpatialCostFunctionIncY;
      if (RegistrationType ==  EMSEGMENT_REGISTRATION_DISABLED)
        {
        for (int i=0; i < NumberOfTotalTypeCLASS; i++)
          {
          if (ProbDataPtr[i]) ProbDataPtr[i] += ProbDataIncY[i];
          if (PCAEigenVectorsPtr[i])
            {
            for (int j = 0 ; j < PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] += PCAEigenVectorsIncY[i][j];}
          if (PCAMeanShapePtr[i])  PCAMeanShapePtr[i] += PCAMeanShapeIncY[i];
          } 
        }
      for (int i = 0; i < NumberOfTotalTypeCLASS; i++) weights[i] += weightsIncY;
      SumSlicePenalty += SumRowPenalty;
      SumRowPenalty = 0.0;

      if (y > ROI_MaxY)
        {
        // Go to next slize
        y = ROI_MinY;
        z++;
        ROI += ROIIncZ;
        if (SpatialCostFunction) SpatialCostFunction += SpatialCostFunctionIncZ;
        if (RegistrationType ==  EMSEGMENT_REGISTRATION_DISABLED)
          {
          for (int i=0; i < NumberOfTotalTypeCLASS; i++)
            {
            if (ProbDataPtr[i]) ProbDataPtr[i] += ProbDataIncZ[i];
            if (PCAEigenVectorsPtr[i])
              {
              for (int j = 0 ; j < PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] += PCAEigenVectorsIncZ[i][j];}
            if (PCAMeanShapePtr[i])  PCAMeanShapePtr[i] += PCAMeanShapeIncZ[i];
            }
          }
        for (int i = 0; i < NumberOfTotalTypeCLASS; i++) weights[i] += weightsIncZ;
        // To reduce computational errors 
        SumImagePenalty  += SumSlicePenalty;
        SumSlicePenalty = 0.0;
        }
      }
    }
  SumImagePenalty += SumSlicePenalty + SumRowPenalty; 

  // std::cerr <<"============================== finished "<< endl;
  // --------------------------------------
  // Return the Cost Function
  // --------------------------------------
  delete[] PCAMeanShapePtr;
  for (int i = 0; i < NumberOfTotalTypeCLASS; i++)
    {
    if (PCAEigenVectorsPtr[i])  delete[] PCAEigenVectorsPtr[i];}
  delete[] PCAEigenVectorsPtr;
  delete[] ProbDataPtr;
  delete[] weights;  
  // Because we look for the maximum and powel defines the minimum
  result = float(-1.0 * SumImagePenalty);
}


//----------------------------------------------------------------------------
// Class functions
//----------------------------------------------------------------------------------

EMLocalShapeCostFunction::EMLocalShapeCostFunction(EMLocal_Hierarchical_Class_Parameters* initEMHierarchyParameters, int  *initPCANumberOfEigenModes, int DisableMultiThreading)
{
  this->EMHierarchyParameters  = initEMHierarchyParameters;
  this->NumberOfTotalTypeCLASS = initEMHierarchyParameters->NumTotalTypeCLASS;
  int NumTotalTypeCLASS        = this->NumberOfTotalTypeCLASS;

  this->NumberOfVoxelsInImage = -1;

  this->ProbDataPtr = new void*[NumTotalTypeCLASS];
  this->ProbDataIncY = new int[NumTotalTypeCLASS];
  this->ProbDataIncZ = new int[NumTotalTypeCLASS];
  for (int i = 0; i < NumTotalTypeCLASS; i++)
    {
    this->ProbDataPtr[i] = NULL;
    this->ProbDataIncY[i] = -1;
    this->ProbDataIncZ[i] = -1;
    }

  this->IncludeSpatialPriorForNormalization = new char[NumTotalTypeCLASS];
  memset(this->IncludeSpatialPriorForNormalization,0,sizeof(char)*NumTotalTypeCLASS);

  this->PCANumberOfEigenModes = initPCANumberOfEigenModes;

  // Initialize Multi Threading
  this->NumOfThreads = EMLocalInterface_GetDefaultNumberOfThreads(DisableMultiThreading);
  //std::cerr << "====================================== Debug "<< this->NumOfThreads << endl;

  this->Threader = vtkMultiThreader::New();
  this->Threader->SetNumberOfThreads(this->NumOfThreads);
  this->Threader->SetSingleMethod(EMLocalShapeCostFunction_ShapeCostFunctionMultiThreaded_Function, ((void*) this));

  this->MultiThreadedParameters = new EMLocalShapeCostFunction_MultiThreadedParameters[this->NumOfThreads];
  for (int i=0; i < this->NumOfThreads; i++)
    {
    this->MultiThreadedParameters[i].ProbDataJump       = new int[NumTotalTypeCLASS];
    this->MultiThreadedParameters[i].PCAMeanShapeJump   = new int[NumTotalTypeCLASS];
    this->MultiThreadedParameters[i].PCAEigenVectorsJump = new int*[NumTotalTypeCLASS]; 
    for (int j=0; j< NumTotalTypeCLASS; j ++)
      {
      this->MultiThreadedParameters[i].ProbDataJump[j] = -1;
      this->MultiThreadedParameters[i].PCAMeanShapeJump[j] = -1;

      if (this->PCANumberOfEigenModes[j])
        {
        this->MultiThreadedParameters[i].PCAEigenVectorsJump[j] = new int[PCANumberOfEigenModes[j]]; 
        }
      else
        {
        this->MultiThreadedParameters[i].PCAEigenVectorsJump[j] = NULL; 
        }
      }
    }

  // PCA Parameters
  this->PCAMeanShapePtr      = new float*[NumTotalTypeCLASS];
  this->PCAMeanShapeIncY     = new int[NumTotalTypeCLASS];
  this->PCAMeanShapeIncZ     = new int[NumTotalTypeCLASS];

  this->PCAEigenVectorsPtr   = new float**[NumTotalTypeCLASS];
  this->PCAEigenVectorsIncY  = new int*[NumTotalTypeCLASS];
  this->PCAEigenVectorsIncZ  = new int*[NumTotalTypeCLASS];

  this->weights              = new float*[NumTotalTypeCLASS];
 
  for (int i=0; i< NumTotalTypeCLASS; i ++)
    {
    if (this->PCANumberOfEigenModes[i] > 0)
      { 
      this->PCAEigenVectorsPtr[i]   = new float*[this->PCANumberOfEigenModes[i]];
      this->PCAEigenVectorsIncY[i]  = new int[this->PCANumberOfEigenModes[i]];
      this->PCAEigenVectorsIncZ[i]  = new int[this->PCANumberOfEigenModes[i]];

      for (int k=0; k < PCANumberOfEigenModes[i]; k++)
        {
        this->PCAEigenVectorsPtr[i][k]  = NULL;
        this->PCAEigenVectorsIncY[i][k] = -1; 
        this->PCAEigenVectorsIncZ[i][k] = -1; 
        }
      }  else {
    this->PCAEigenVectorsPtr[i]  = NULL;
    this->PCAEigenVectorsIncY[i] = NULL; 
    this->PCAEigenVectorsIncZ[i] = NULL; 
    }

    this->weights[i]          = NULL;
    this->PCAMeanShapePtr[i]  = NULL;
    this->PCAMeanShapeIncY[i] = -1;
    this->PCAMeanShapeIncZ[i] = -1;
    this->ProbDataPtr[i]   = NULL;
    this->ProbDataIncY[i] = -1;
    this->ProbDataIncZ[i] = -1;
    }

  // Pointer to variables created by other functions 
  this->ROI   = NULL;

  this->ClassToAtlasRotationMatrix     = NULL;
  this->ClassToAtlasTranslationVector  = NULL;

  this->ParaDepVar = new EMLocalShapeCostFunction_IterationSpecificVariables;
  this->ParaDepVar->PCAPara = NULL;
  this->ParaDepVar->SpatialCostFunction = NULL;
  this->ParaDepVar->GaussianPenalty = -1;
  this->ParaDepVar->ImagePenalty = -1;
}

EMLocalShapeCostFunction::~EMLocalShapeCostFunction()  { 
  this->SpatialCostFunctionOff();

  if (this->ProbDataPtr) delete[] this->ProbDataPtr;
  this->ProbDataPtr = NULL;
  if (this->ProbDataIncY) delete[] this->ProbDataIncY;
  this->ProbDataIncY = NULL;
  if (this->ProbDataIncZ) delete[] this->ProbDataIncZ;
  this->ProbDataIncZ = NULL;
    
  if (this->IncludeSpatialPriorForNormalization) delete[] this->IncludeSpatialPriorForNormalization;
  this->IncludeSpatialPriorForNormalization = NULL;

  if (this->Threader)
    {
    this->Threader->Delete();
    this->Threader = NULL;
    }

  if (this->MultiThreadedParameters)
    {
    for (int i = 0; i < this->NumOfThreads; i++)
      {
      if (this->MultiThreadedParameters[i].ProbDataJump)     delete[] this->MultiThreadedParameters[i].ProbDataJump;
      if (this->MultiThreadedParameters[i].PCAMeanShapeJump) delete[] this->MultiThreadedParameters[i].PCAMeanShapeJump;
      if (this->MultiThreadedParameters[i].PCAEigenVectorsJump)
        {
        for (int j=0; j < this->NumberOfTotalTypeCLASS; j++)
          {
          if (this->MultiThreadedParameters[i].PCAEigenVectorsJump[j]) delete[] this->MultiThreadedParameters[i].PCAEigenVectorsJump[j];
          }
        delete[] this->MultiThreadedParameters[i].PCAEigenVectorsJump;
        }
      }
    delete[] this->MultiThreadedParameters;
    this->MultiThreadedParameters =NULL;
    }

  if (this->weights)
    {
    delete[] this->weights; 
    this->weights      = NULL;
    }

  if (this->PCAMeanShapePtr)  {
  delete[] this->PCAMeanShapePtr;
  this->PCAMeanShapePtr = NULL;
  delete[] this->PCAMeanShapeIncY;
  this->PCAMeanShapeIncY = NULL;
  delete[] this->PCAMeanShapeIncZ;
  this->PCAMeanShapeIncZ = NULL;
  }

  if(this->PCAEigenVectorsPtr)
    {
    for (int  i=0 ; i< this->NumberOfTotalTypeCLASS; i ++)
      {
      if (this->PCAEigenVectorsPtr[i])
        {
        delete[] this->PCAEigenVectorsPtr[i];
        delete[] this->PCAEigenVectorsIncY[i];
        delete[] this->PCAEigenVectorsIncZ[i];
        }
      }
    delete[] this->PCAEigenVectorsPtr;
    delete[] this->PCAEigenVectorsIncY;
    delete[] this->PCAEigenVectorsIncZ;
    this->PCAEigenVectorsPtr  = NULL; 
    this->PCAEigenVectorsIncY = NULL;
    this->PCAEigenVectorsIncZ = NULL;
    }
  
  delete this->ParaDepVar;
}

//----------------------------------------------------------------------------
// Set/Get Functions
void EMLocalShapeCostFunction::SpatialCostFunctionOn()
{
  assert(this->NumberOfVoxelsInImage < 0);
  if (this->ParaDepVar->SpatialCostFunction) delete[] this->ParaDepVar->SpatialCostFunction;
  ParaDepVar->SpatialCostFunction = new float[this->NumberOfVoxelsInImage];
}
 
void EMLocalShapeCostFunction::SpatialCostFunctionOff()
{
  if (this->ParaDepVar->SpatialCostFunction) delete[] this->ParaDepVar->SpatialCostFunction;
  this->ParaDepVar->SpatialCostFunction = NULL;
} 


void EMLocalShapeCostFunction::InitRegSpecPara(int initRegistrationType, int LengthX, int LengthY, int LengthZ)  {
  this->RegistrationType = initRegistrationType;

  this->Image_Length[0] = LengthX;
  this->Image_Length[1] = LengthY;
  this->Image_Length[2] = LengthZ;
  
  this->Image_MidX   = (LengthX-1)*0.5;; 
  this->Image_MidY   = (LengthY-1)*0.5;; 
  this->Image_MidZ   = (LengthZ-1)*0.5;; 
}

//This function is necessary so we can calculate the cost function of the shape parameters just in the Box that is important for this analysis 
void EMLocalShapeCostFunction::DefinePCADataPtr(int inPCAMinX, int inPCAMinY, int inPCAMinZ, int inBoundaryMaxX, int inBoundaryMaxY, int inDataIncY,  int inDataIncZ, int &outJump, int &outDataIncY,  int &outDataIncZ)
{  
  int LengthOfXDim = inBoundaryMaxX + inDataIncY;
  int LengthOfYDim = LengthOfXDim*(inBoundaryMaxY) + inDataIncZ;  
  outJump = inPCAMinX + inPCAMinY * LengthOfXDim + LengthOfYDim *inPCAMinZ;
  outDataIncY = LengthOfXDim -  this->ROI_LengthX;
  outDataIncZ = LengthOfYDim -  this->ROI_LengthY *LengthOfXDim;
}

void EMLocalShapeCostFunction::TransferePCAShapeParametersIntoArray(float** PCAShapeParameters,float* PCAParameters)
{
  int ShapeIndex = 0;
  for (int i = 0 ; i <  this->NumberOfTotalTypeCLASS; i++)
    {
    for (int k = 0 ; k < this->PCANumberOfEigenModes[i]; k++)
      {
      PCAParameters[ShapeIndex] = PCAShapeParameters[i][k];
      ShapeIndex ++;             
      }
    // Then all Shape parameters should be the same
    if (this->PCAShapeModelType == EMSEGMENT_PCASHAPE_DEPENDENT) ShapeIndex = 0;
    }
} 

     
void EMLocalShapeCostFunction::TransfereArrayIntoPCAShapeParameters(float* PCAParameters, float** PCAShapeParameters)
{
  int ShapeIndex = 0;
  for (int i = 0 ; i <  this->NumberOfTotalTypeCLASS; i++)
    {
    for (int k = 0 ; k < this->PCANumberOfEigenModes[i]; k++)
      {
      PCAShapeParameters[i][k] = PCAParameters[ShapeIndex];
      ShapeIndex ++;             
      }
    if (this->PCAShapeModelType == EMSEGMENT_PCASHAPE_DEPENDENT) ShapeIndex = 0;
    }
}  



//----------------------------------------------------------------------------
// Distance map calculations 
// Published in ICCV05
// The distance map is transfered to the space pf space conditioned probabilities via the logistic function 
double EMLocalShapeCostFunction::Transfere_DistanceMap_Into_SpatialPrior(double distance, float variance, float boundary, float Min, float Max)
{
  // Currently just doing it because if the maximum value is 0 or 20 then you get something like 0.001 and 0.99993 => error 
#if (EMPRIVATE_HEAVISIDE_FCT_FLAG) 
  if (distance < boundary) return 0.0;
  return double(this->NumberOfTrainingSamples);
#else  
  if (distance < Min) return 0.0;
  if (distance > Max) return double(this->NumberOfTrainingSamples);
  return (double(this->NumberOfTrainingSamples)/(1.0 + exp(-variance  * (distance -  boundary ))));
#endif
}

//----------------------------------------------------------------------------------
// Multi threaded Cost Function 
float EMLocalShapeCostFunction::ComputeCostFunction(const double *initPCAPara) const {
  
  this->ParaDepVar->PCAPara = initPCAPara;
  if (this->ParaDepVar->SpatialCostFunction) memset(this->ParaDepVar->SpatialCostFunction, 0, sizeof(float)*this->NumberOfVoxelsInImage);
  // Start Execution
  this->Threader->SingleMethodExecute();

  float result = 0.0;  
  for (int i = 0; i < this->NumOfThreads; i++)
    {
    result += this->MultiThreadedParameters[i].Result;
    }
  // --------------------------------------
  // Calculate Gaussian Term
  // --------------------------------------
  int ShapeIndex = 0;
  float PCAPenalty =0.0;

  for (int i = 0 ; i <  this->NumberOfTotalTypeCLASS; i++)
    {
    if (this->PCANumberOfEigenModes[i])
      {
      for (int k = 0 ; k < this->PCANumberOfEigenModes[i]; k++)
        {
        // PCA penality is the log of the gaussian.  Based on definition float vtkImagePCAApply::GaussianDistribution(float* shapePara) 
        // this is defined as 
        PCAPenalty += ParaDepVar->PCAPara[ShapeIndex]*ParaDepVar->PCAPara[ShapeIndex]*this->PCAInverseEigenValues[i][k] ;
        ShapeIndex ++;
        }
      // If they are all defined by one PCA model than the Gaussian penality should only be calcluated once
      if (this->PCAShapeModelType == EMSEGMENT_PCASHAPE_DEPENDENT) break;
      }
    }
  // Version 3 used   PCAPenalty  =  float(*ShapeParameters.ROIExactVoxelCount) * 0.0025 * PCAPenalty; 
  // Officially submitted for ICCV 05
  PCAPenalty  =  float(*this->ROIExactVoxelCount) * 0.0025 * PCAPenalty; 
  // Not us good  PCAPenalty  =  float(*ShapeParameters.ROIExactVoxelCount) * 0.005 * PCAPenalty; 
  // PCAPenalty  =  0.5 * PCAPenalty; 
  
  this->ParaDepVar->GaussianPenalty = PCAPenalty;
  this->ParaDepVar->ImagePenalty    = result;
  // printf("Result %15f %15f %15f %15f %15f %15f \n", result + PCAPenalty, PCAPara[0], PCAPara[1], PCAPara[2], PCAPara[3], PCAPara[4]); 
  return (result + PCAPenalty);
}

//----------------------------------------------------------------------------------
// Interface Function of this class 
// it optimizes the current parameter set based on w_m and the pca shape model

void EMLocalShapeCostFunction::InitializeCostFunction(int PCAMaxX, int PCAMinX, int PCAMaxY, int PCAMinY, int PCAMaxZ, int PCAMinZ, 
                                                      int BoundaryMinX, int BoundaryMinY, int BoundaryMinZ, int Boundary_LengthX, int Boundary_LengthY, float** w_m, 
                                                      unsigned char* PCA_ROI, void  **initProbDataPtr, float** initPCAMeanShapePtr, int* initPCAMeanShapeIncY, 
                                                      int *initPCAMeanShapeIncZ, float*** initPCAEigenVectorsPtr, int **initPCAEigenVectorsIncY, 
                                                      int** initPCAEigenVectorsIncZ)
{

  // std::cerr << "Start vtkImageEMLocalSegmenter_ShapeInterface  " << endl;
  // ---------------------------------------------------
  // Initialize
  int RegistrationActive = (this->RegistrationType > EMSEGMENT_REGISTRATION_DISABLED);
 
  // Set the input in such a way that we zoom in on the image the is important -> reduces computational cost 
  // The parameters are given in the space of data extent
   
  this->ROI_MaxZ  = PCAMaxZ + BoundaryMinZ;
  this->ROI_MaxY =  PCAMaxY + BoundaryMinY; 
  this->ROI_MaxX =  PCAMaxX + BoundaryMinX;
 
  this->ROI_MinZ  = PCAMinZ + BoundaryMinZ;
  this->ROI_MinY =  PCAMinY + BoundaryMinY; 
  this->ROI_MinX =  PCAMinX + BoundaryMinX;
 
  this->ROI_LengthZ = PCAMaxZ - PCAMinZ + 1; 
  this->ROI_LengthY = PCAMaxY - PCAMinY + 1; 
  this->ROI_LengthX = PCAMaxX - PCAMinX + 1; 
  this->DefinePCADataPtr(PCAMinX, PCAMinY, PCAMinZ, Boundary_LengthX, Boundary_LengthY, 0, 0, this->DataJump, this->DataIncY, this->DataIncZ);
  // std::cerr << "DataJump " << PCAMaxY << " " <<  PCAMinY << endl; 

  for (int i= 0 ; i < this->NumberOfTotalTypeCLASS; i++)
    {
    this->weights[i]            = w_m[i] + this->DataJump;
    }
  this->weightsIncY             = this->DataIncY;
  this->weightsIncZ             = this->DataIncZ;
   
  this->ROI                     = PCA_ROI + this->DataJump;
  this->ROIIncY                 = this->DataIncY;
  this->ROIIncZ                 = this->DataIncZ;
   
  // Only include those probdataptr where the superclass has no prob data defined -> otherwise only include it once!
   
  for (int i = 0 ; i <  this->NumberOfTotalTypeCLASS; i++)
    {
    if (initProbDataPtr[i])
      { 
      if (RegistrationActive)
        {
        this->ProbDataPtr[i]  = initProbDataPtr[i]; 
        this->ProbDataIncY[i] = this->EMHierarchyParameters->ProbDataIncY[i]; 
        this->ProbDataIncZ[i] = this->EMHierarchyParameters->ProbDataIncZ[i]; 
        }
      else
        { 
        int aDataJump;
        int aDataIncY;
        int aDataIncZ;
        this->DefinePCADataPtr(PCAMinX, PCAMinY, PCAMinZ, Boundary_LengthX, Boundary_LengthY, this->EMHierarchyParameters->ProbDataIncY[i], 
                               this->EMHierarchyParameters->ProbDataIncZ[i], aDataJump, aDataIncY, aDataIncZ);
        switch (this->GetProbDataType())
          {
          vtkTemplateMacro(EMLocalShapeCostFunction_AssignProbDataPtr(this, (VTK_TT**) initProbDataPtr,aDataJump,i));

          default :
            std::cerr << "EMLocalShapeCostFunction::CalculateOptimalParameters Unknown ScalarType " << this->GetProbDataType() << endl;
            return;
          }

        this->ProbDataIncY[i] = aDataIncY;
        this->ProbDataIncZ[i] = aDataIncZ; 
        }
      }

    if (initPCAMeanShapePtr[i])
      {
      if (RegistrationActive)
        {
        this->PCAMeanShapePtr[i]  = initPCAMeanShapePtr[i];
        this->PCAMeanShapeIncY[i] = initPCAMeanShapeIncY[i];
        this->PCAMeanShapeIncZ[i] = initPCAMeanShapeIncZ[i];
        }
      else
        {
        int PCADataJump, PCADataIncY, PCADataIncZ;
        this->DefinePCADataPtr(PCAMinX, PCAMinY, PCAMinZ, Boundary_LengthX, Boundary_LengthY, PCAMeanShapeIncY[i], PCAMeanShapeIncZ[i], PCADataJump, PCADataIncY, PCADataIncZ);
        this->PCAMeanShapePtr[i]  = initPCAMeanShapePtr[i] + PCADataJump;
        this->PCAMeanShapeIncY[i] = PCADataIncY;
        this->PCAMeanShapeIncZ[i] = PCADataIncZ;
        }
      for (int j = 0 ; j < this->PCANumberOfEigenModes[i]; j ++)
        {
        if (RegistrationActive)
          {
          this->PCAEigenVectorsPtr[i][j]  = initPCAEigenVectorsPtr[i][j];
          this->PCAEigenVectorsIncY[i][j] = initPCAEigenVectorsIncY[i][j]; 
          this->PCAEigenVectorsIncZ[i][j] = initPCAEigenVectorsIncZ[i][j]; 
          }
        else
          {
          int PCADataJump, PCADataIncY, PCADataIncZ;
          this->DefinePCADataPtr(PCAMinX, PCAMinY, PCAMinZ, Boundary_LengthX, Boundary_LengthY, initPCAEigenVectorsIncY[i][j], initPCAEigenVectorsIncZ[i][j], PCADataJump, PCADataIncY, PCADataIncZ);
          this->PCAEigenVectorsPtr[i][j]  = initPCAEigenVectorsPtr[i][j] + PCADataJump;
          this->PCAEigenVectorsIncY[i][j] = PCADataIncY; 
          this->PCAEigenVectorsIncZ[i][j] = PCADataIncZ; 
          }
        }
      }
    }
 
  //------------------------------------
  //Now MultiThread the whole code
  EMLocalShapeCostFunction_MultiThreadedParameters *aMultiThreadedParameters = this->MultiThreadedParameters;  
  int ROI_LengthXY       = this->ROI_LengthX * this->ROI_LengthY;
  int ROI_LengthXYZ      = ROI_LengthXY * this->ROI_LengthZ;
  int JobSize     = ROI_LengthXYZ/this->NumOfThreads;
  // std::cerr << "Jobsize " << JobSize << " " << ROI_LengthXYZ << endl;
  int VoxelOffset = 0;
  int VoxelLeftOver;
  for (int i= 0; i < this->NumOfThreads; i++)
    {
    // std::cerr << "Thread " << i <<  endl;
    int *VoxelStart = aMultiThreadedParameters[i].VoxelStart;
    VoxelStart[2] = VoxelOffset/ROI_LengthXY;
    VoxelLeftOver = VoxelOffset % ROI_LengthXY;
    VoxelStart[1] = VoxelLeftOver / this->ROI_LengthX;
    VoxelStart[0] = VoxelLeftOver % this->ROI_LengthX;
 
    if (i < this->NumOfThreads -1) aMultiThreadedParameters[i].NumberOfVoxels = JobSize;
    else aMultiThreadedParameters[i].NumberOfVoxels = JobSize +  ROI_LengthXYZ%this->NumOfThreads;
 
    aMultiThreadedParameters[i].DataJump = EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->ROI_LengthX, this->ROI_LengthY, this->DataIncY, this->DataIncZ);
    // std::cerr << "DataJump " <<  aMultiThreadedParameters[i].DataJump << endl;
    // std::cerr << "Jump " << aMultiThreadedParameters[i].DataJump <<  " " << ROI_LengthX << " " <<  ROI_LengthY << " " <<  this->DataIncY << " " << this->DataIncZ << " " << VoxelOffset << endl; 

    for (int j= 0 ; j < this->NumberOfTotalTypeCLASS; j++)
      {
      // std::cerr << "Class " << j << " " ;

      if (initProbDataPtr[j])
        {
        if (RegistrationActive)
          {
          aMultiThreadedParameters[i].ProbDataJump[j] = 0;
          }
        else
          {
          aMultiThreadedParameters[i].ProbDataJump[j] = EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->ROI_LengthX, this->ROI_LengthY, this->ProbDataIncY[j], this->ProbDataIncZ[j]);
          // std::cerr << aMultiThreadedParameters[i].ProbDataJump[j] << " " ;
          }
        }
      if (initPCAMeanShapePtr[j])
        {
        if (RegistrationActive)
          {
          aMultiThreadedParameters[i].PCAMeanShapeJump[j] = 0; 
          for (int k= 0 ; k < this->PCANumberOfEigenModes[j]; k++)
            {
            aMultiThreadedParameters[i].PCAEigenVectorsJump[j][k] = 0;
            }
          } 
        else
          {
          aMultiThreadedParameters[i].PCAMeanShapeJump[j] = EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->ROI_LengthX, this->ROI_LengthY, this->PCAMeanShapeIncY[j], this->PCAMeanShapeIncZ[j]);
          // std::cerr << aMultiThreadedParameters[i].PCAMeanShapeJump[j] << " " ;

          for (int k= 0 ; k < this->PCANumberOfEigenModes[j]; k++)
            {
            aMultiThreadedParameters[i].PCAEigenVectorsJump[j][k] =  EMLocalInterface_DefineMultiThreadJump(VoxelStart,this->ROI_LengthX, this->ROI_LengthY, this->PCAEigenVectorsIncY[j][k], this->PCAEigenVectorsIncZ[j][k]);
            // std::cerr << aMultiThreadedParameters[i].PCAEigenVectorsJump[j][k] << " " ;
            }
          }
        }
      // std::cerr << endl; 
      }
    // std::cerr << "Finished threadding " << endl;
    //Now put it into ROI region 
    VoxelStart[2] += this->ROI_MinZ;
    VoxelStart[1] += this->ROI_MinY;
    VoxelStart[0] += this->ROI_MinX;

    VoxelOffset += JobSize;
    }
}


// the actual derivative of A(x) = vtkImageEMSegment_Transfere_DistanceMap_Into_SpatialPrior(x) is A'(x)= A(x)^2 * vtkImageEMSegment_PartOf_DeriveOf_ShapeBased_SpatialPrior(x)
///inline double vtkImageEMSegment_PartOf_DeriveOf_ShapeBased_SpatialPrior(double distance) {
//  return (-ShapeParameters.PCASpatialPriorVariance * exp(-ShapeParameters.PCASpatialPriorVariance * (distance - 7.5 ))) /  float(ShapeParameters.NumberOfTrainingSamples);
//}



// template  <class Tin> 
// inline void vtkImageEMLocalSegmenter_CalculateDerivativeShapeCostFunction(float *ShapePara, Tin **ProbDataPtrOrig, float *result) {
// 
//   // --------------------------------------
// 
//   // Initialiaze Parameters 
//   // --------------------------------------
//   int NumClasses        = ShapeParameters.EMHierarchyParameters->NumClasses;
//   int *NumChildClasses  = ShapeParameters.EMHierarchyParameters->NumChildClasses;
//   int NumTotalTypeCLASS = ShapeParameters.EMHierarchyParameters->NumTotalTypeCLASS;
// 
//   float **weights = new float*[NumTotalTypeCLASS];
//   for (int i = 0; i < NumTotalTypeCLASS; i++) weights[i] = ShapeParameters.weights[i]; 
//   int weightsIncY = ShapeParameters.weightsIncY;
//   int weightsIncZ = ShapeParameters.weightsIncZ;
// 
//   unsigned char *OutputVector = ShapeParameters.OutputVector;
//   int OutputVectorIncY = ShapeParameters.OutputVectorIncY;
//   int OutputVectorIncZ = ShapeParameters.OutputVectorIncZ;
// 
//   int Boundary_LengthZ = ShapeParameters.ROI_LengthZ;
//   int Boundary_LengthY = ShapeParameters.ROI_LengthY;
//   int Boundary_LengthX = ShapeParameters.ROI_LengthX;
// 
//   Tin **ProbDataPtr = new Tin*[NumTotalTypeCLASS];
//   for (int i = 0; i < NumTotalTypeCLASS; i++) ProbDataPtr[i] = ProbDataPtrOrig[i];
// 
//   int *ProbDataIncY = ShapeParameters.ProbDataIncY;
//   int *ProbDataIncZ = ShapeParameters.ProbDataIncZ;
// 
//   int *PCANumberOfEigenModes = ShapeParameters.PCANumberOfEigenModes;
//   
//   float **PCAInverseEigenValues = new float*[NumTotalTypeCLASS];
//   for (int i = 0; i < NumTotalTypeCLASS; i++) PCAInverseEigenValues[i] = ShapeParameters.PCAInverseEigenValues[i];
// 
//   float ***PCAEigenVectorsPtr   = new float**[NumTotalTypeCLASS];
// 
//   for (int i = 0; i < NumTotalTypeCLASS; i++) {
//     if (PCANumberOfEigenModes[i]) {
//       PCAEigenVectorsPtr[i] = new float*[PCANumberOfEigenModes[i]];
//       for (int j = 0; j < PCANumberOfEigenModes[i]; j++)  PCAEigenVectorsPtr[i][j] =  ShapeParameters.PCAEigenVectorsPtr[i][j];
//     } else {
//       PCAEigenVectorsPtr[i] = NULL;
//     }
//   }
// 
//   float  **PCAMeanShapePtr       = new float*[NumTotalTypeCLASS]; 
//   for (int i = 0; i < NumTotalTypeCLASS; i++) PCAMeanShapePtr[i] = ShapeParameters.PCAMeanShapePtr[i]; 
// 
//   int   **PCAEigenVectorsIncY   = ShapeParameters.PCAEigenVectorsIncY; 
//   int   **PCAEigenVectorsIncZ   = ShapeParameters.PCAEigenVectorsIncZ; 
//   int    *PCAMeanShapeIncY      = ShapeParameters.PCAMeanShapeIncY;
//   int    *PCAMeanShapeIncZ      = ShapeParameters.PCAMeanShapeIncZ;
//   int   PCATotalNumOfShapeParameters =  ShapeParameters.PCATotalNumOfShapeParameters;
//   // --------------------------------------
//   // Calculate Gaussian Term
//   // --------------------------------------
//   float *PCAPenalty  = new float[PCATotalNumOfShapeParameters];
// 
//   int ShapeIndex = 0;
//   for (int i = 0 ; i <  NumTotalTypeCLASS; i++) {
//     if (PCANumberOfEigenModes[i]) {
//       for (int k = 0 ; k < PCANumberOfEigenModes[i]; k++) {
//     // PCA penalty is the log of the gaussian.  Based on definition float vtkImagePCAApply::GaussianDistribution(float* shapePara) 
//     // this is defined as 
//     PCAPenalty[ShapeIndex] = ShapePara[ShapeIndex]*PCAInverseEigenValues[i][k] ;
//     ShapeIndex ++;
//       }
//     }
//   }
//   
//   // Set up print function 
//   float **DerivedResultVolume = NULL;
//   int DerivedResultVolumeIncY = ShapeParameters.DataIncY;
//   int DerivedResultVolumeIncZ = ShapeParameters.DataIncZ;
// 
//   if (ShapeParameters.DerivedResultVolume) {
//     DerivedResultVolume = new float*[PCATotalNumOfShapeParameters];
//     for (int i = 0;  i  < PCATotalNumOfShapeParameters; i++) {
//       DerivedResultVolume[i] = ShapeParameters.DerivedResultVolume[i] + ShapeParameters.DataJump;
//     }
//   }
//   
// 
//   // --------------------------------------
//   // Determin penalty over all voxels of the image
//   // --------------------------------------
//   float *SumImagePenalty  = new float[PCATotalNumOfShapeParameters];
//   memset(SumImagePenalty, 0, sizeof(float)*PCATotalNumOfShapeParameters);
// 
//   float *VoxelSpatialPrior       =  new float[NumTotalTypeCLASS];
//   float *VoxelDeriveSpatialPrior =  new float[NumTotalTypeCLASS];
//   float SumVoxelSpatialPrior;
//   // If all spatial priors are zero then we do not have to calculate it 
//   int VoxelCalcDerivative;  
//   float VoxelDistanceMap;
//   float VoxelDerivativeTerm; 
//   
//   for (int z = 0; z < Boundary_LengthZ ; z++) {
//     for (int y = 0; y < Boundary_LengthY ; y++) {
//       for (int x = 0; x < Boundary_LengthX ; x++) {
//     if (*OutputVector < EMSEGMENT_NOTROI) {
//       ShapeIndex = 0;
//       VoxelCalcDerivative  = 0;
//       SumVoxelSpatialPrior = 0.0;
//       for (int i = 0 ; i <  NumTotalTypeCLASS; i++) {
//          if (PCANumberOfEigenModes[i]) {
//               // Kilian: Make this more effecient
//           VoxelDistanceMap  =  EMLocalInterface_CalcDistanceMap(&ShapePara[ShapeIndex], PCAEigenVectorsPtr[i], *PCAMeanShapePtr[i],  PCANumberOfEigenModes[i]);
//           VoxelSpatialPrior[i] = vtkImageEMSegment_Transfere_DistanceMap_Into_SpatialPrior(VoxelDistanceMap,ShapeParameters.PCADistanceVariance);  
//           VoxelDeriveSpatialPrior[i]   = vtkImageEMSegment_PartOf_DeriveOf_ShapeBased_SpatialPrior(VoxelDistanceMap);
// 
//           // if ((VoxelSpatialPrior[i] > 0.0) ||  (VoxelDeriveSpatialPrior[i]  > 0.0 )) std::cerr << VoxelSpatialPrior[i] << " ----- " << VoxelDeriveSpatialPrior[i] << " " << VoxelDistanceMap << endl;
//           if ((VoxelSpatialPrior[i] != 0.0) &&  (VoxelDeriveSpatialPrior[i]  != 0.0 )) VoxelCalcDerivative = 1;
//               SumVoxelSpatialPrior += VoxelSpatialPrior[i];
//         } else if (ProbDataPtr[i]) {
//               SumVoxelSpatialPrior += float(*ProbDataPtr[i]);
//         }
//       }
// 
//           // Calculate the derivative as (weight - (VoxelSpatialPrior[i] /SumVoxelSpatialPrior))  * VoxelDeriveSpatialPrior[i] * VoxelSpatialPrior[i] * Eigenvector
//           //                             = VoxelDerivativeTerm * Eigenvector
//       if (VoxelCalcDerivative) {
//         ShapeIndex = 0;
//         for (int i = 0 ; i <  NumTotalTypeCLASS; i++) {
//           if (PCANumberOfEigenModes[i]) {
//         if ((VoxelSpatialPrior[i] != 0.0) &&  (VoxelDeriveSpatialPrior[i]  != 0.0 )) {
//           VoxelDerivativeTerm = (*weights[i] - (VoxelSpatialPrior[i]/SumVoxelSpatialPrior))* VoxelSpatialPrior[i] * VoxelDeriveSpatialPrior[i];
//           if (fabs(VoxelDerivativeTerm) > 0.0001) { 
//             for (int j = 0 ; j < PCANumberOfEigenModes[i] ; j++) {
//               SumImagePenalty[ShapeIndex] += VoxelDerivativeTerm * (*PCAEigenVectorsPtr[i][j]);
//               if (DerivedResultVolume) {
//             *DerivedResultVolume[ShapeIndex] =  VoxelDerivativeTerm * (*PCAEigenVectorsPtr[i][j]);
//             // std::cerr << *DerivedResultVolume[ShapeIndex] << " " ;
//               }
//               ShapeIndex ++;
//             } 
//           }
//         } else {
//           // Do not need to do the calculations
//           ShapeIndex += PCANumberOfEigenModes[i];
//         }
//           }
//         }
//       }
//         } // end of if (*OutputVector < EMSEGMENT_NOTROI) .. 
//  
//        // Go to next x value 
//         OutputVector++;
//     for (int i = 0; i < NumTotalTypeCLASS; i++) {
//       if (PCAEigenVectorsPtr[i]) { for (int j = 0 ; j <  PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] ++; }
//       if (PCAMeanShapePtr[i]) PCAMeanShapePtr[i] ++;
//       if (ProbDataPtr[i]) ProbDataPtr[i] ++;
//       weights[i] ++;
//     }
//         if (DerivedResultVolume) {for (int i = 0;  i  < PCATotalNumOfShapeParameters ; i++) DerivedResultVolume[i] ++;}
//       } // End of for (x =0 ; ... 
//       OutputVector += OutputVectorIncY;
//       if (DerivedResultVolume) {for (int i = 0;  i  < PCATotalNumOfShapeParameters ; i++) DerivedResultVolume[i] += DerivedResultVolumeIncY;}
// 
//       for (int i=0; i < NumTotalTypeCLASS; i++) {
//     if (ProbDataPtr[i]) ProbDataPtr[i] += ProbDataIncY[i];
//     if (PCAEigenVectorsPtr[i]) {for (int j = 0 ; j < PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] += PCAEigenVectorsIncY[i][j];}
//     if (PCAMeanShapePtr[i])  PCAMeanShapePtr[i] += PCAMeanShapeIncY[i];
//         weights[i] += weightsIncY;
//       } 
//     } // End of for (y =0 ; ... 
//     OutputVector += OutputVectorIncZ;
//     if (DerivedResultVolume) {for (int i = 0;  i  < PCATotalNumOfShapeParameters ; i++) DerivedResultVolume[i] += DerivedResultVolumeIncZ;}
// 
//     for (int i=0; i < NumTotalTypeCLASS; i++) {
//       if (ProbDataPtr[i]) ProbDataPtr[i] += ProbDataIncZ[i];
//       if (PCAEigenVectorsPtr[i]) {for (int j = 0 ; j < PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] += PCAEigenVectorsIncZ[i][j];}
//       if (PCAMeanShapePtr[i])  PCAMeanShapePtr[i] += PCAMeanShapeIncZ[i];
//       weights[i] += weightsIncZ;
//     }
//   } // End of for (z =0 ; ... 
//   // --------------------------------------
//   // Return the Cost Function
//   // --------------------------------------
//   // Because we look for the maximum and powel defines the minimum
//   // std::cerr << "---------" << endl;
//   for (int k= 0; k < PCATotalNumOfShapeParameters; k++)  {
//     result[k] =  PCAPenalty[k] - SumImagePenalty[k];
//     // std::cerr << "Blub " << result[k] << endl;
//   }
//   
//   if (DerivedResultVolume) delete[] DerivedResultVolume;
// 
//   delete[] SumImagePenalty;
//   delete[] VoxelSpatialPrior;
//   delete[] VoxelDeriveSpatialPrior;
// 
//   delete[] PCAPenalty;
//   delete[] PCAMeanShapePtr;
//   for (int i = 0; i < NumTotalTypeCLASS; i++) {if (PCAEigenVectorsPtr[i])  delete[] PCAEigenVectorsPtr[i];}
//   delete[] PCAEigenVectorsPtr;
//   delete[] PCAInverseEigenValues;
//   delete[] ProbDataPtr;
//   delete[] weights;  
// } 

//void vtkImageEMLocalSegmenter_DerivativeShapeCostFunction(float *ShapePara, float* result) {
// switch (ShapeParameters.EMHierarchyParameters->ProbDataType) {                  
//  vtkTemplateMacro3(vtkImageEMLocalSegmenter_CalculateDerivativeShapeCostFunction,ShapePara, (VTK_TT**) ShapeParameters.ProbDataPtr, result); 
//  }
//}
