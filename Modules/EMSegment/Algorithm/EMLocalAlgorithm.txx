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
//Includes the functionality related to EM

/* Forward declaration */
template <class T> void EMLocalAlgorithm_PrintVector(T* parameters, int Min,  int Max);
VTK_THREAD_RETURN_TYPE EMLocalAlgorithm_E_Step_Threader_Function(void *arg);

#include "vtkTimeDef.h"
#include "EMLocalAlgorithm_Initialization.txx"
#include "EMLocalAlgorithm_MeanField.txx"
#include "EMLocalAlgorithm_Miscellaneous.txx"
#include "EMLocalAlgorithm_Print.txx"
#include "vtkSimonParameterReaderWriter.h"

#include "itkEMLocalOptimization.h"

// Core Functions
template <class T> EMLocalAlgorithm<T>::~EMLocalAlgorithm() {
  // EStepMultiThreader Variables

 
  if (this->w_mCopy ) {
    for (int i = 0; i < this->NumTotalTypeCLASS; i++) delete[] this->w_mCopy[i];
    delete[] this->w_mCopy;
  }

  if (this->E_Step_Threader_Parameters) {
    for (int i = 0; i < this->E_Step_Threader_Number; i++) {
      if (this->E_Step_Threader_Parameters[i].ProbDataJump)     delete[] this->E_Step_Threader_Parameters[i].ProbDataJump;
      if (this->E_Step_Threader_Parameters[i].PCAMeanShapeJump) delete[] this->E_Step_Threader_Parameters[i].PCAMeanShapeJump;
      if (this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump) {
          for (int j=0; j < this->NumTotalTypeCLASS; j++) {
            if (this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump[j]) delete[] this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump[j];
          }
          delete[] this->E_Step_Threader_Parameters[i].PCAEigenVectorsJump;
    
      }
      this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MAP = NULL;
    }
    delete[] this->E_Step_Threader_Parameters;
  }

  this->E_Step_Threader->Delete();

  // Registration Variables
  if (this->RegistrationTranslation) delete[] this->RegistrationTranslation;
  if (this->RegistrationRotation)    delete[] this->RegistrationRotation;
  if (this->RegistrationScale)       delete[] this->RegistrationScale;

  // Have to delete the prob data that I created to simplify PCA analysis
  if (PCATotalNumOfShapeParameters && (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED)) {
    for (int i = 0; i < NumTotalTypeCLASS; i++) {
      if (PCANumberOfEigenModes[i]) {
    delete[] this->ProbDataPtrStart[i]; 
    this->ProbDataPtrStart[i] = NULL;
      }
    }
  }

  if (this->RegistrationParameterFile) {
    for (int i = 0; i < this->RegistrationParameters->GetNumberOfParameterSets() ; i++) {
      if (this->RegistrationParameterFile[i]) {
    fflush(this->RegistrationParameterFile[i]);
    fclose(this->RegistrationParameterFile[i]);   
      }
    }
    delete[] this->RegistrationParameterFile;
  }

  if (this->RegistrationIndependentSubClassFlag) { delete[] this->RegistrationIndependentSubClassFlag; }
  if (this->RegistrationClassSpecificRegistrationFlag) { delete[] this->RegistrationClassSpecificRegistrationFlag; }

  if (this->ClassToAtlasRotationMatrix) {
    for (int i = this->GenerateBackgroundProbability; i < this->NumClasses; i++) delete[] this->ClassToAtlasRotationMatrix[i];
    delete[] this->ClassToAtlasRotationMatrix;
  }

  if (ClassToAtlasTranslationVector) {
    for (int i = this->GenerateBackgroundProbability; i < this->NumClasses; i++) delete[] this->ClassToAtlasTranslationVector[i];
    delete[] this->ClassToAtlasTranslationVector;
  }
  if (this->ClassToSuperClassRotationMatrix) {
    for (int i = this->GenerateBackgroundProbability; i < this->NumClasses; i++) delete[] this->ClassToSuperClassRotationMatrix[i];
    delete[] this->ClassToSuperClassRotationMatrix;
  }
  if (ClassToSuperClassTranslationVector) {
    for (int i = GenerateBackgroundProbability; i < NumClasses; i++) delete[] ClassToSuperClassTranslationVector[i];
    delete[] ClassToSuperClassTranslationVector;
  }

  if (SuperClassToAtlasRotationMatrix)    delete[] SuperClassToAtlasRotationMatrix;
  if (SuperClassToAtlasTranslationVector) delete[] SuperClassToAtlasTranslationVector;

  delete RegistrationParameters;

  // Shape Variables
  if (this->PCA_ROI_Start) delete[] this->PCA_ROI_Start;

  if (PCAFile) {
    for (int i = 0; i < NumClasses; i++) if (PCAFile[i] != NULL) {
      fflush(PCAFile[i]);
      fclose(PCAFile[i]);   
    }
    delete[] PCAFile;
  }
  delete   ShapeParameters;
  delete[] PCALogisticSlope;
  delete[] PCALogisticBoundary;
  delete[] PCALogisticMax;
  delete[] PCALogisticMin;
  for (int i = 0; i < NumTotalTypeCLASS; i++) {
    if (PCAEigenVectorsPtrStart[i]) delete[] PCAEigenVectorsPtrStart[i];
    if (PCAEigenVectorsIncY[i]) delete[] PCAEigenVectorsIncY[i];
    if (PCAEigenVectorsIncZ[i]) delete[] PCAEigenVectorsIncZ[i];
  }

  delete[] this->PCAEigenVectorsIncY;
  delete[] this->PCAEigenVectorsIncZ;
  delete[] this->PCAEigenVectorsPtrStart; 
  delete[] this->PCAMeanShapeIncY;
  delete[] this->PCAMeanShapeIncZ;
  delete[] this->PCAMeanShapePtrStart; 
  delete[] this->PCANumberOfEigenModes;
  delete[] this->PCAEigenValues;
  for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
    if (this->PCAInverseEigenValues[i]) delete[] this->PCAInverseEigenValues[i];
  }
  delete[] this->PCAInverseEigenValues;

  delete[] this->PCAParameters;
  delete[] this->PCAShapeParameters;

  // Print Variables 
  if (CurrentMFALabelMap) delete[] CurrentMFALabelMap;
  
  if (CurrentMFAWeights) {
    for (int i=0; i<this->NumClasses; i++) delete[] CurrentMFAWeights[i];
    delete[] CurrentMFAWeights;
  }

  if (LabelMapEMDifferenceFile) {
    fflush(LabelMapEMDifferenceFile);
    fclose(LabelMapEMDifferenceFile);  
  }

  if (WeightsEMDifferenceFile != NULL) {
    fflush(WeightsEMDifferenceFile);
    fclose(WeightsEMDifferenceFile);   
  }

  if (CurrentEMLabelMap) delete[] CurrentEMLabelMap;

  if (CurrentEMWeights) {
    for (int i=0; i<NumClasses; i++) delete[] CurrentEMWeights[i];
    delete[] CurrentEMWeights;
  }

  if (QualityFile) {
    for (int i = 0; i < EMSEGMENT_NUM_OF_QUALITY_MEASURE ; i++) {
      if (QualityFile[i] != NULL) {
    fflush(QualityFile[i]);
    fclose(QualityFile[i]);   
      }
    }
    delete[] QualityFile;
  }
  delete[] QualityFlagList;

  // EM Variables
  delete[] NumChildClasses;
  delete[] LabelList;
  delete[] CurrentLabelList;

  for (int i = 0; i < NumTotalTypeCLASS; i++) {
    for (int x = 0; x < NumInputImages; x++)  delete[] InverseWeightedLogCov[i][x]; 
    delete[] InverseWeightedLogCov[i];
  }
  delete[] InverseWeightedLogCov;  
  delete[] InvSqrtDetWeightedLogCov; 
 
  delete[] LogMu;
  delete[] LogCovariance;

  delete[] ExcludeFromIncompleteEStepFlag;
  delete[] VirtualNumInputImages;
  delete[] VirtualOveralInputChannelFlag;

  delete[] TissueProbability;
  
  delete[] ProbDataPtrStart;
  delete[] ProbDataWeight;
  delete[] ProbDataMinusWeight;
  delete[] ProbDataIncZ;
  delete[] ProbDataIncY;

  delete[] this->cY_MPtr;
  delete[] this->OutputVectorPtr;
}


// ---------------------------------------------------
// Multi Threading Interface
// ---------------------------------------------------
template <class T>  void EMLocalAlgorithm<T>::E_Step_Threader_FunctionStart(int CurrentThread) {
  assert(CurrentThread < this->E_Step_Threader_Number);

  EMLocalAlgorithm_E_Step_MultiThreaded_Parameters* ThreadedParameters  =  &(this->E_Step_Threader_Parameters[CurrentThread]); 

  this->E_Step_Weight_Calculation_Threaded(ThreadedParameters->VoxelStart, ThreadedParameters->NumberOfVoxels, ThreadedParameters->DataJump, 
                       ThreadedParameters->PCAMeanShapeJump, ThreadedParameters->PCAEigenVectorsJump, ThreadedParameters->ProbDataJump,
                       ThreadedParameters->PCAMin, ThreadedParameters->PCAMax, &(ThreadedParameters->Registration_ROI_Weight),
                       ThreadedParameters->IncompleteModelVoxelCount, ThreadedParameters->PCA_ROIExactVoxelCount); 
}

VTK_THREAD_RETURN_TYPE EMLocalAlgorithm_E_Step_Threader_Function(void *arg) {
  int CurrentThread = ((ThreadInfoStruct*)(arg))->ThreadID;
  // ID is to high for our setup 
  EMLocalAlgorithm_E_Step_MultiThreaded_SelfPointer* SelfPointer = (EMLocalAlgorithm_E_Step_MultiThreaded_SelfPointer*) (((ThreadInfoStruct*)(arg))->UserData);
  void* self = SelfPointer->self;

  switch (SelfPointer->DataType) {
    vtkTemplateMacro(((EMLocalAlgorithm<VTK_TT>*) self)->E_Step_Threader_FunctionStart(CurrentThread));
    default :
      cout << "Warning: EMLocalAlgorithm_E_Step_Threader_Function: unknown data type " << SelfPointer->DataType << endl;
      exit(0);

  }
  return VTK_THREAD_RETURN_VALUE;
}

// --------------------------------------------------------------------------------------------------------------------------
//  Registration Functions 
// --------------------------------------------------------------------------------------------------------------------------

// double GlobalReg* can be the same as double NewReg*
// The parameters describe the alignement of the atlas to the image space , or image space to structure space 
// The NewRegRotation/Translation describes the alignment of the structure to the atlas space or image to the atlas space 
// Remember we always use the image / structure space coordinate system to retrieve values from the atlas space in the interpolation function 
inline int EMLocalAlgorithm_RegistrationMatrix(double Translation[3], double Rotation[3], double Scale[3], float GlobalRegRotation[9], float GlobalRegTranslation[3], 
                      float NewRegRotation[9], float NewRegTranslation[3],int TwoDFlag) {
  // Note - the parameter define the relstionship from target to source , but the inverse matrix defines the relationship from source to target
  if (TwoDFlag) {
    if (vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(Translation[0], Translation[1], 0.0, 0.0, 0.0, Rotation[2], 
                                             Scale[0], Scale[1], 1.0, NewRegRotation, NewRegTranslation,2)) return 1;  
  } else {
    if (vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(Translation[0], Translation[1], Translation[2], Rotation[0], Rotation[1], Rotation[2], 
                                             Scale[0], Scale[1], Scale[2], NewRegRotation, NewRegTranslation,2)) return 1;  
  }
  vtkSimonParameterReaderWriter::matmult_3x4(GlobalRegRotation, GlobalRegTranslation, NewRegRotation, NewRegTranslation, NewRegRotation, NewRegTranslation);
  return 0;
}

// Registration Function
inline void EMLocalAlgorithm_TransfereRegistrationParameter_ToTranRotSca(double *FinalParameters,double *Translation, double *Rotation, double *Scale, 
                                           EMLocalRegistrationCostFunction *RegistrationParameters) {
  if (RegistrationParameters->GetTwoDFlag()) {
    Translation[0] = FinalParameters[0]; Translation[1] = FinalParameters[1]; Translation[2] = 0.0;
    Rotation[0] = Rotation[1] = 0.0; Rotation[2]    = FinalParameters[2];
    Scale[0]       = FinalParameters[3]; Scale[1]       = FinalParameters[4]; Scale[2] = 0.0;

  } else {
    Translation[0] = FinalParameters[0]; Translation[1] = FinalParameters[1]; Translation[2] = FinalParameters[2];         
    Rotation[0]    = FinalParameters[3]; Rotation[1]    = FinalParameters[4]; Rotation[2]    = FinalParameters[5]; 
    if (RegistrationParameters->GetRigidFlag()) {
      Scale[0] = Scale[1] = Scale[2] = 1.0;
    } else {
      Scale[0]       = FinalParameters[6]; Scale[1]       = FinalParameters[7]; Scale[2]       = FinalParameters[8];
    }
  }
}

template <class T>
inline void EMLocalAlgorithm<T>::DefineForRegistrationRotTranSca(int NumParaSets) {

  this->RegistrationTranslation   = new double*[NumParaSets];
  this->RegistrationRotation      = new double*[NumParaSets];
  this->RegistrationScale         = new double*[NumParaSets];

  int GenerateBackgroundProbability = this->actSupCl->GetGenerateBackgroundProbability();
  void** ClassList                  = this->actSupCl->GetClassList();
  classType* ClassListType          = this->actSupCl->GetClassListType();  
  int NumClasses                    = this->actSupCl->GetNumClasses();
  int RegistrationType              = this->RegistrationParameters->GetRegistrationType();
  vtkNotUsed(int RigidFlag                     = this->RegistrationParameters->GetRigidFlag(););
  int *ClassSpecificRegistrationFlag = this->RegistrationParameters->GetClassSpecificRegistrationFlag();


  // These are the parameters describing the transation from Global Coordinate System to Class 
  int ParaSetIndex = 0; 
  if (RegistrationType != EMSEGMENT_REGISTRATION_CLASS_ONLY) { 
    RegistrationTranslation[0]    =  this->actSupCl->GetRegistrationTranslation();
    RegistrationRotation[0]       =  this->actSupCl->GetRegistrationRotation();
    RegistrationScale[0]          =  this->actSupCl->GetRegistrationScale();
    ParaSetIndex ++;
  }
  if (RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) {
    for (int i = GenerateBackgroundProbability; i < NumClasses; i ++) {   
      if (ClassSpecificRegistrationFlag[i]) {
    if (ClassListType[i] == CLASS) {
      RegistrationTranslation[ParaSetIndex]    = ((vtkImageEMLocalClass*) ClassList[i])->GetRegistrationTranslation();
      RegistrationRotation[ParaSetIndex]       = ((vtkImageEMLocalClass*) ClassList[i])->GetRegistrationRotation();
      RegistrationScale[ParaSetIndex]          = ((vtkImageEMLocalClass*) ClassList[i])->GetRegistrationScale();
    }  else {
      RegistrationTranslation[ParaSetIndex]    = ((vtkImageEMLocalSuperClass*) ClassList[i])->GetRegistrationTranslation();
      RegistrationRotation[ParaSetIndex]       = ((vtkImageEMLocalSuperClass*) ClassList[i])->GetRegistrationRotation();
      RegistrationScale[ParaSetIndex]          = ((vtkImageEMLocalSuperClass*) ClassList[i])->GetRegistrationScale();
    }
    ParaSetIndex ++;
      }
    }
  }
}

template <class T>
void EMLocalAlgorithm<T>::RegistrationInterface(float &Cost) {
  // Initialize Parameters
  int NumParaPerSet    =  this->RegistrationParameters->GetNumberOfParameterPerSet();
  int NumParaTotal     =  this->RegistrationParameters->GetNumberOfParameterSets()*NumParaPerSet; 
  double* FinalParameters = new double[NumParaTotal]; 
  for (int i = 0 ; i <  this->RegistrationParameters->GetNumberOfParameterSets()  ; i++) 
    EMLocalAlgorithm_TransfereTranRotSca_ToRegistrationParameter(this->RegistrationTranslation[i], this->RegistrationRotation[i], 
                                       this->RegistrationScale[i], &FinalParameters[NumParaPerSet*i],
                                       this->RegistrationParameters);
  // this->RegistrationParameters->StartRegistration(FinalParameters,Cost);
  itkEMLocalOptimization_Registration_Start(this->RegistrationParameters,FinalParameters,Cost);

  for (int j = 0; j < this->RegistrationParameters->GetNumberOfParameterSets() ; j++) 
    EMLocalAlgorithm_TransfereRegistrationParameter_ToTranRotSca(&FinalParameters[j* NumParaPerSet],this->RegistrationTranslation[j], 
                                       this->RegistrationRotation[j], this->RegistrationScale[j], 
                                       this->RegistrationParameters);
  delete []FinalParameters; 
}

// Defines all the matrices necessary for EM to do interporalation  
template <class T>
inline int EMLocalAlgorithm<T>::DefineGlobalAndStructureRegistrationMatrix() {

  if (EMLocalAlgorithm_RegistrationMatrix(this->actSupCl->GetRegistrationTranslation(), this->actSupCl->GetRegistrationRotation(), this->actSupCl->GetRegistrationScale(), 
                        this->GlobalRegInvRotation, this->GlobalRegInvTranslation, this->SuperClassToAtlasRotationMatrix, 
                        this->SuperClassToAtlasTranslationVector,this->TwoDFlag)) {
      vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: Cannot invert rotation matrix defined by the pararmeters of actual Super Class");
      return 0 ;      
  }

  if (this->GenerateBackgroundProbability) {
    this->ClassToAtlasRotationMatrix[0]         = NULL;
    this->ClassToAtlasTranslationVector[0]      = NULL;
    this->ClassToSuperClassRotationMatrix[0]    = NULL;
    this->ClassToSuperClassTranslationVector[0] = NULL; 
  }

  for (int i = this->GenerateBackgroundProbability; i < this->NumClasses; i ++) {   
    this->ClassToAtlasRotationMatrix[i]         = new float[9];
    this->ClassToAtlasTranslationVector[i]      = new float[3];
    this->ClassToSuperClassRotationMatrix[i]    = new float[9];
    this->ClassToSuperClassTranslationVector[i] = new float[3];
    
    // These are the parameters describing the transation from SuperClass to Class
    // the Matrices and Vectors before are the inverse of the parameters   
    double *rotation, *translation, *scale;
    
    if (this->ClassListType[i] == SUPERCLASS) {
      translation = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetRegistrationTranslation();
      rotation    = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetRegistrationRotation();
      scale       = ((vtkImageEMLocalSuperClass*) this->ClassList[i])->GetRegistrationScale();
    } else {
      translation = ((vtkImageEMLocalClass*) this->ClassList[i])->GetRegistrationTranslation();
      rotation    = ((vtkImageEMLocalClass*) this->ClassList[i])->GetRegistrationRotation();
      scale       = ((vtkImageEMLocalClass*) this->ClassList[i])->GetRegistrationScale();
    }
    // If it is not defined than we just use _APPLY and wont maximize
    if (this->RegistrationClassSpecificRegistrationFlag && this->RegistrationClassSpecificRegistrationFlag[i]) {
      // make sure parameters are set correctly - if there is an offset I do not believe that powell works correctly anymore bc it essentially ignorese these parameters
       assert(translation[0] == 0.0 && translation[1] == 0.0 && translation[2] == 0.0);
      assert(rotation[0] == 0.0 && rotation[1] == 0.0 && rotation[2] == 0.0);
      assert(scale[0] == 1.0 && scale[1] == 1.0 && scale[2] == 1.0);
    }

    if (vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(translation, rotation, scale, 
                      this->ClassToSuperClassRotationMatrix[i],  this->ClassToSuperClassTranslationVector[i],2,
                      this->TwoDFlag, this->RigidFlag)) {
    vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: Cannot invert rotation matrix defined by the pararmeters of class " << i);
    return 0;
    }

      
    vtkSimonParameterReaderWriter::matmult_3x4(this->SuperClassToAtlasRotationMatrix, this->SuperClassToAtlasTranslationVector, 
                           this->ClassToSuperClassRotationMatrix[i], this->ClassToSuperClassTranslationVector[i], 
                           this->ClassToAtlasRotationMatrix[i], this->ClassToAtlasTranslationVector[i]);
  }
  return 1;
}



// --------------------------------------------------------------------------------------------------------------------------
//  Shape Functions 
// --------------------------------------------------------------------------------------------------------------------------

// Assume that we have prob data with incr 0 for y and z
template  <class Tin> inline void EMLocalAlgorithm<Tin>::Transfere_ShapePara_Into_SpatialPrior(int index) {

  float **EigenVectorsPtr = new float*[this->PCANumberOfEigenModes[index]];
  Tin *ProbData       = this->ProbDataPtrStart[index];
  float *MeanShapePtr =  this->PCAMeanShapePtrStart[index];

  for (int i =0; i < this->PCANumberOfEigenModes[index]; i++) EigenVectorsPtr[i] = this->PCAEigenVectorsPtrStart[index][i];
  // Define spatial prior
  for (int z = 0; z < this->RealMaxZ; z++) {
    for (int y = 0; y < this->RealMaxY; y++) {
      for (int x = 0; x < this->RealMaxX; x++) {
    *ProbData =  (Tin) (this->ShapeParameters->Transfere_DistanceMap_Into_SpatialPrior(EMLocalInterface_CalcDistanceMap(this->PCAShapeParameters[index], 
                                                 EigenVectorsPtr, MeanShapePtr, this->PCANumberOfEigenModes[index],0), 
                                               this->PCALogisticSlope[index], this->PCALogisticBoundary[index], 
                                               this->PCALogisticMin[index], this->PCALogisticMax[index])); 
    ProbData ++;
    for (int l = 0 ; l < this->PCANumberOfEigenModes[index]; l++)  EigenVectorsPtr[l] ++;
    MeanShapePtr ++;
      }
      for (int l = 0 ; l < this->PCANumberOfEigenModes[index]; l++)  EigenVectorsPtr[l] += this->PCAEigenVectorsIncY[index][l]; 
      MeanShapePtr += this->PCAMeanShapeIncY[index];
    }
    for (int l = 0 ; l < this->PCANumberOfEigenModes[index]; l++)  EigenVectorsPtr[l] += this->PCAEigenVectorsIncZ[index][l]; 
    MeanShapePtr += this->PCAMeanShapeIncZ[index];
  }
  delete[] EigenVectorsPtr;
}

template <class T> void EMLocalAlgorithm<T>::UpdatePCASpecificParameters(int iter) {
  if ((iter > 9) || !(this->RegistrationType) || 1) {
    for (int i = 0 ; i <  this->NumTotalTypeCLASS; i++) {
      if (this->PCANumberOfEigenModes[i] && this->PCALogisticSlope[i] < 3.0) this->PCALogisticSlope[i] *=1.1;
    }
  }
    

  if (this->RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) {
    for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
      if (this->PCANumberOfEigenModes[i]) this->Transfere_ShapePara_Into_SpatialPrior(i);
    }
    // Update Prior frame
    if ((this->RegistrationType ==  EMSEGMENT_REGISTRATION_GLOBAL_ONLY) || (this->RegistrationType == EMSEGMENT_REGISTRATION_SEQUENTIAL)) {
      EMLocalRegistrationCostFunction_DefineROI_ProbDataValues(this->RegistrationParameters, this->ProbDataPtrStart);
    }
    cout << "Min " << this->Registration_ROI_ProbData.MinCoord[0] << " " << this->Registration_ROI_ProbData.MinCoord[1] << " "<< this->Registration_ROI_ProbData.MinCoord[2] 
     << endl;
    cout << "Max " << this->Registration_ROI_ProbData.MaxCoord[0] << " " << this->Registration_ROI_ProbData.MaxCoord[1] << " "<< this->Registration_ROI_ProbData.MaxCoord[2] 
     << endl;
  }
}

// --------------------------------------------------------------------------------------------------------------------------
//  Probability Functions 
// --------------------------------------------------------------------------------------------------------------------------

// Close to Sandies original approach +(part of ISBI04)
inline double EMLocalAlgorithm_ConditionalTissueProbability(const double TissueProbability, const double InvSqrtDetLogCov, 
                                                               const float *cY_M,  const double  *LogMu, double **InvLogCov, 
                                                               const int NumInputImages, const int  VirtualNumInputImages) {
   return TissueProbability  * vtkImageEMGeneral::FastGaussMulti(InvSqrtDetLogCov,cY_M, LogMu,InvLogCov,NumInputImages, VirtualNumInputImages); 
}

template  <class T> void EMLocalAlgorithm<T>::E_Step_ExecuteMultiThread() {
  if (1) 
    this->E_Step_Threader->SingleMethodExecute();
  else {
    cout << "===================================" << endl;
    cout << "Debug Threading" << endl;
    for (int i = 0; i < this->E_Step_Threader_Number ; i++ ) {
      cout << "Execute Thread " << i << endl;
      this->E_Step_Threader_FunctionStart(i);
    } 
  }

   int IncompleteModelVoxelCount = 0;
   this->PCA_ROIExactVoxelCount = 0 ;
  
   this->Registration_ROI_Weight.MinCoord[0] = this->BoundaryMaxX;
   this->Registration_ROI_Weight.MinCoord[1] = this->BoundaryMaxY;
   this->Registration_ROI_Weight.MinCoord[2] = this->BoundaryMaxZ;
   this->Registration_ROI_Weight.MaxCoord[0] = this->Registration_ROI_Weight.MaxCoord[1] = this->Registration_ROI_Weight.MaxCoord[2] = 0;

   this->PCAMin[0] = this->BoundaryMaxX;  this->PCAMin[1] = this->BoundaryMaxY;   this->PCAMin[2] = this->BoundaryMaxZ;
   for (int i = 0; i <3; i++) this->PCAMax[i] = 0;

   for (int i = 0 ; i < this->E_Step_Threader_Number; i++) {
     IncompleteModelVoxelCount    += this->E_Step_Threader_Parameters[i].IncompleteModelVoxelCount; 
     this->PCA_ROIExactVoxelCount += this->E_Step_Threader_Parameters[i].PCA_ROIExactVoxelCount;
     for (int j = 0; j < 3; j++) {
       if (this->Registration_ROI_Weight.MinCoord[j] > this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MinCoord[j]) 
     this->Registration_ROI_Weight.MinCoord[j] = this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MinCoord[j];
       if (this->Registration_ROI_Weight.MaxCoord[j] < this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MaxCoord[j]) 
     this->Registration_ROI_Weight.MaxCoord[j] = this->E_Step_Threader_Parameters[i].Registration_ROI_Weight.MaxCoord[j];
       if (this->PCAMin[j] >  this->E_Step_Threader_Parameters[i].PCAMin[j]) this->PCAMin[j] =  this->E_Step_Threader_Parameters[i].PCAMin[j];
       if (this->PCAMax[j] <  this->E_Step_Threader_Parameters[i].PCAMax[j]) this->PCAMax[j] =  this->E_Step_Threader_Parameters[i].PCAMax[j];
     }

   }
   if (IncompleteModelVoxelCount) cout <<"Warning: E-Step counted "<< IncompleteModelVoxelCount <<" voxels not properly captured by the Model !" <<endl;
}

//----------------------------------------------------------------------------
// Class Function
//----------------------------------------------------------------------------
// Calculates the weighted inverse covariance
 
template  <class T> int EMLocalAlgorithm<T>::CalcWeightedCovariance(double** WeightedInvCov, double & SqrtDetWeightedInvCov, float* Weights, double** LogCov, 
                                    int & VirtualDim,int dim) {
  int x,y,Xindex,Yindex;
  // Calculate the weighted coveriance => inpput channels are differently weighted
  VirtualDim = 0; 
  for (x=0;x < dim ; x++)  if (Weights[x] > 0.0) VirtualDim ++;
  for (x=0; x < dim ; x++) memset(WeightedInvCov[x],0,sizeof(double)*dim);
  if (VirtualDim == 0) {
    vtkEMAddWarningMessage("EMLocalAlgorithm<T>::CalcWeightedCovariance:  All Weights are set to 0");
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
// E-Step Functions
//----------------------------------------------------------------------------

template <class T> void EMLocalAlgorithm<T>::InfoOnPrintFlags() { 
  cout << "Print labelmaps:                         " << (this->actSupCl->GetPrintLabelMap() ? "On" : "Off") << endl;
  int WeightFlag     = 0;
  for (int c = 0 ; c < NumClasses; c++) {
    if (((ClassListType[c] == CLASS)      && (((vtkImageEMLocalClass*) this->ClassList[c])->GetPrintWeights())) ||
    ((ClassListType[c] == SUPERCLASS) && (((vtkImageEMLocalSuperClass*) this->ClassList[c])->GetPrintWeights()))) WeightFlag = 1;
  }
  
  cout << "Print Weights (Type: Floats):            " << (WeightFlag ? "On" : "Off") << endl;
 
  cout << "Print Shape Parameters:                  " << (this->PCAFile ? "On" : "Off") << endl;
  cout << "Print Quality Parameters:                " << (this->QualityFile ? "On" : "Off") << endl;
  cout << "Print Registration Parameters:           " << (this->RegistrationParameterFile ? "On" : "Off") << endl;
  cout << "Print Simularity Measure (Type: Double): " << (this->actSupCl->GetPrintRegistrationSimularityMeasure() ? "On" : "Off")  << endl;
}

template  <class T> void EMLocalAlgorithm<T>::Expectation_Step(int iter) {
  // -----------------------------------------      
  // E-Step without MF 

  // If this EM with MF then we have then we initialize w_m first before using MF 
  if (this->Alpha == 0 || iter == 1) {
    float alpha_orig = this->Alpha;
    this->Alpha = 0.0;

    // ------------------- 
    // Calculate Weights 
    this->E_Step_ExecuteMultiThread();
    this->Alpha =  alpha_orig;

    // -----------------------------------------------------------
    // Initialize values for MFA difference calculations
    if (this->Alpha > 0 && (StopMFAType || PrintMFALabelMapConvergence || PrintMFAWeightsConvergence)) {
       int LabelMapMFADifferenceAbsolut  =  0;    
       float LabelMapMFADifferencePercent  =  0.0;    
       float WeightsMFADifferenceAbsolut   =  0.0;    
       float WeightsMFADifferencePercent   =  0.0; 
       int MFAStopFlag;
 
       this->DifferenceMeassure(StopMFAType, PrintMFALabelMapConvergence, PrintMFAWeightsConvergence, 1, CurrentMFALabelMap, this->w_mPtr,
               LabelMapMFADifferenceAbsolut, LabelMapMFADifferencePercent,CurrentMFAWeights, WeightsMFADifferenceAbsolut, 
               WeightsMFADifferencePercent, this->actSupCl->GetStopMFAValue(),MFAStopFlag); 
    } 

  }
  
  // -----------------------------------------      
  // E-Step with Mean Field / Regularization 
  if (this->Alpha > 0.0) this->RegularizeWeightsWithMeanField(iter);

  // -----------------------------------------------------------
  // Calculate convergence Factor
  // -----------------------------------------------------------
  // If we have boundary dinfed on change than check now 
  if (this->StopEMType || this->PrintEMLabelMapConvergence || this->PrintEMWeightsConvergence) {
    // This is the convergence between different EM steps 
    if (iter > 1) cout << "-------- EM Convergence ------- " << endl;
    
    this->DifferenceMeassure(StopEMType, PrintEMLabelMapConvergence, PrintEMWeightsConvergence, iter, CurrentEMLabelMap, this->w_mPtr, 
                 LabelMapEMDifferenceAbsolut, LabelMapEMDifferencePercent, CurrentEMWeights, WeightsEMDifferenceAbsolut, 
                 WeightsEMDifferencePercent, StopEMValue,EMStopFlag);
  }
  // -----------------------------------------------------------
  // Print Out Intermediate Results
  // ----------------------------------------------------------- 
  if  (this->PrintIntermediateFlag || ((this->PrintFrequency == -1) && (this->EMStopFlag || (iter == NumIter )))) {
    cout << "PrintIntermediateResultsToFile " << endl;
    this->Print_E_StepResultsToFile(iter); 
    cout << "End of printing " << endl;
  }
 }


// This function is called if in the E-Step normRow == 0 , which in general means that the spatial distribution of all the classes did not overlap with the intensity pattern of these classes 
// Thus, the pretrained model is not completetly describing our segmentation scenario and we have to handle it in a special way 
template <class T> inline void EMLocalAlgorithm<T>::E_Step_IncompleteModel(int indexX, int indexY, int indexZ, float **w_m_input, float **w_m_output, T **ProbDataPtrCopy, 
                                         float &normRow, float *cY_M, float*** PCAEigenVectorsPtr, float **PCAMeanShapePtr, 
                                         unsigned char OutputVector) { 
  int index = 0;
  float targetX = 0.0;
  float targetY = 0.0;
  float targetZ = 0.0;

  // Order of Criteria Selection in Case of Inomplete Model 
  // 1.) Most likely stetting with respect to Neighborhood => Smooth Segmentation
  // 2.) Use intensity characteristerics => Noiser Segmentation 
  // 3.) Use spatial pariors => wrong spatial priors can mess up the bias calculations.
  
  if (this->Alpha > 0.0 ) {
    for (int j=0; j <  this->NumClasses ; j++) {
      double MeanFieldPotential = this->TissueProbability[j]*this->NeighberhoodEnergy(w_m_input, OutputVector,j);
      for (int k=0; k < this->NumChildClasses[j];k++) { 
    normRow += *w_m_output[index] =   MeanFieldPotential;
    index ++;
      }
    }
    if (normRow > 0.0) return;
  }

  // You can exclude structures from being part of intensity based weight definition as it can produce bad outliers . 
  index = 0;
  for (int j=0; j <  this->NumClasses ; j++) {
    if (!this->ExcludeFromIncompleteEStepFlag[j]) {
      for (int k=0; k < this->NumChildClasses[j];k++) { 
    // The line below is the same as ConditionalTissueProbability is the same as 
    *w_m_output[index] =  (float) EMLocalAlgorithm_ConditionalTissueProbability(this->TissueProbability[j], this->InvSqrtDetWeightedLogCov[index],cY_M, 
                                        this->LogMu[index], this->InverseWeightedLogCov[index], this->NumInputImages, 
                                        this->VirtualNumInputImages[j]);   
    normRow += *w_m_output[index];
    index ++;
      }
    } else {
      index += NumChildClasses[j];
    }
  }

  if (normRow > 0.0) return;

  index = NumTotalTypeCLASS - 1; 
  // 
  // This can mess up the Bias Claculations - that's why we put it second 
  double SumOfAlignedTissueDistribution = 0.0;
  double AlignedTissueDistribution = 0;

  // See normal E-STep for explanation
  for (int i = NumClasses -1 ; i > -1 ; i--) {
    if (i || !GenerateBackgroundProbability) {
      if (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) 
    EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(this->ClassToAtlasRotationMatrix[i], this->ClassToAtlasTranslationVector[i], indexX, indexY, 
                                     indexZ, targetX, targetY, targetZ, targetmidcol, targetmidrow, targetmidslice);
      
    } else {
      AlignedTissueDistribution = double(SumOfAlignedTissueDistribution < NumberOfTrainingSamples ?  (NumberOfTrainingSamples- SumOfAlignedTissueDistribution) : 0.0); 
    }
    for (int k = NumChildClasses[i] -1; k < -1 ;k--) {
      if (i || !GenerateBackgroundProbability) {
    // ------------------------------------------------
    // Look for instructions at original E-Step 
    // ------------------------------------------------
    if (ProbDataPtrStart[index]) {                                                         
      if (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) {                            
        AlignedTissueDistribution = EMLocalInterface_Interpolation(targetX, targetY, targetZ, this->RealMaxX, this->RealMaxY, this->RealMaxZ, ProbDataPtrStart[index],ProbDataIncY[index],ProbDataIncZ[index],RegistrationParameters->GetInterpolationType(), RegistrationParameters->GetImage_Length()); 
      } else {                                                                             
        AlignedTissueDistribution = double(*ProbDataPtrCopy[index]);             
      }                                                                                     
    } else if (PCANumberOfEigenModes[index]) {       
      AlignedTissueDistribution =  ShapeParameters->Transfere_DistanceMap_Into_SpatialPrior(EMLocalInterface_CalcDistanceMap(PCAShapeParameters[index], 
                                                      PCAEigenVectorsPtr[index], PCAMeanShapePtr[index], 
                                                     PCANumberOfEigenModes[index],0), PCALogisticSlope[index], 
                                                PCALogisticBoundary[index], PCALogisticMin[index], PCALogisticMax[index]); 
    } else {                                                                               
      AlignedTissueDistribution = 0.0;                                                       
    }                        
    SumOfAlignedTissueDistribution += AlignedTissueDistribution;
      }
      *w_m_output[index]  += ProbDataMinusWeight[i]  + ProbDataWeight[i] * AlignedTissueDistribution; 
      normRow += *w_m_output[index];
      index --;
    }
  }
}

template <class T> void EMLocalAlgorithm<T>::E_Step_Weight_Calculation_Threaded(int Thread_VoxelStart[3], int Thread_NumberOfVoxels, int Thread_DataJump, 
                                          int *Thread_PCAMeanShapeJump, int** Thread_PCAEigenVectorsJump, int *Thread_ProbDataJump,
                                          int Thread_PCAMin[3], int Thread_PCAMax[3], EMLocalRegistrationCostFunction_ROI *Thread_Registration_ROI_Weight,
                                          int &Thread_IncompleteModelVoxelCount,int &Thread_PCA_ROIExactVoxelCount ) {

  // -----------------------------------------      
  // Initialize Values for the E-Step
  // -----------------------------------------      
  unsigned char* PCA_ROI = (this->PCA_ROI_Start ? this->PCA_ROI_Start + Thread_DataJump : NULL); 
 
  Thread_PCA_ROIExactVoxelCount  = 0;

  // How many voxels are not properly captured by our model 
  Thread_IncompleteModelVoxelCount = 0;
  int Thread_VoxelCount = 1;


  // -----------------------------------------      
  // General EM Variables
  float normRow;     
  float **w_m_input  = new float*[NumTotalTypeCLASS];
  float **w_m_output = new float*[NumTotalTypeCLASS];
  for (int i=0; i<NumTotalTypeCLASS; i++) {
    // Result of Weights after trad. E Step -  dimesion NumTotalTypeClasses x ImageProd
    w_m_output[i] = this->w_m_outputPtr[i] +  Thread_DataJump;     
    w_m_input[i]  = this->w_m_inputPtr[i]  +  Thread_DataJump; 
  }
  
  float *cY_M = this->cY_MPtr + NumInputImages*Thread_DataJump;
  
  double ConditionalTissueProbability;
  double SpatialTissueDistribution = 0;
  double AlignedTissueDistribution;
  double MeanFieldPotential = 1.0;
  
  // Needed for GenerateBackgroundProbability 
  double SumOfAlignedTissueDistribution = 0.0;
  
  
  T** ProbDataPtrCopy = new T*[NumTotalTypeCLASS];
  for (int i =0;i<NumTotalTypeCLASS;i++) ProbDataPtrCopy[i] = (this->ProbDataPtrStart[i] ? this->ProbDataPtrStart[i] + Thread_ProbDataJump[i] : NULL); 
  
  unsigned char* OutputVector = this->OutputVectorPtr + Thread_DataJump;
  
  // -----------------------------------------      
  // Registration Specific Parameters
  float targetX = 0.0;
  float targetY = 0.0;
  float targetZ = 0.0;

  Thread_Registration_ROI_Weight->MinCoord[0] = this->BoundaryMaxX;
  Thread_Registration_ROI_Weight->MinCoord[1] = this->BoundaryMaxY;
  Thread_Registration_ROI_Weight->MinCoord[2] = this->BoundaryMaxZ;
  Thread_Registration_ROI_Weight->MaxCoord[0] = Thread_Registration_ROI_Weight->MaxCoord[1] = Thread_Registration_ROI_Weight->MaxCoord[2] = 0;
  
  char* Reg_ROI_MAP          = Thread_Registration_ROI_Weight->MAP;
  vtkNotUsed(int*  Reg_ROI_MinCoord     = Thread_Registration_ROI_Weight->MinCoord;);
  vtkNotUsed(int*  Reg_ROI_MaxCoord     = Thread_Registration_ROI_Weight->MaxCoord;);
  int   Reg_ROI_ClassOutside = Thread_Registration_ROI_Weight->ClassOutside =  this->Registration_ROI_ProbData.ClassOutside;
  
  int Reg_ROI_FlagX=0, Reg_ROI_FlagY=0, Reg_ROI_FlagZ=0;
  
  // -----------------------------------------      
  // Shape Specific parameters

  Thread_PCAMin[0] = this->BoundaryMaxX;  Thread_PCAMin[1] = this->BoundaryMaxY;   Thread_PCAMin[2] = this->BoundaryMaxZ;
  for (int i = 0; i <3; i++) Thread_PCAMax[i] = 0;

  
  // as in the the row of defined by Y / Slice by Z one voxel that has to be considered;
  int PCA_ROI_FlagY = 0, PCA_ROI_FlagZ = 0;
  
  float**  PCAMeanShapePtr    = new float*[this->NumTotalTypeCLASS];
  float*** PCAEigenVectorsPtr = new float**[this->NumTotalTypeCLASS];
  
  
  for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
    PCAMeanShapePtr[i] = (this->PCAMeanShapePtrStart[i] ? this->PCAMeanShapePtrStart[i] + Thread_PCAMeanShapeJump[i]: NULL); 
    PCAEigenVectorsPtr[i] = new float*[PCANumberOfEigenModes[i]];
    for (int j = 0; j < PCANumberOfEigenModes[i]; j++) PCAEigenVectorsPtr[i][j] = (this->PCAEigenVectorsPtrStart[i][j] ? 
                                           this->PCAEigenVectorsPtrStart[i][j] + Thread_PCAEigenVectorsJump[i][j] : NULL);
  }
 
  // -----------------------------------------      
  // Start Calculations 
  // -----------------------------------------      
  int indexZ = SegmentationBoundaryMin[2] - 1 + Thread_VoxelStart[2];
  for (int z = Thread_VoxelStart[2]; z < BoundaryMaxZ ; z++) {
    int indexY = SegmentationBoundaryMin[1] - 1;
    PCA_ROI_FlagZ = 0;
    Reg_ROI_FlagZ = 0;

    for (int y =  0; y < BoundaryMaxY ; y++) {
      int indexX = SegmentationBoundaryMin[0] - 1;
      int x = 0;

      if ((y ==0) && z ==  Thread_VoxelStart[2]) {
    y = Thread_VoxelStart[1];  
    indexY +=  Thread_VoxelStart[1]; 

    x = Thread_VoxelStart[0];   
        indexX += Thread_VoxelStart[0];   
      }  

      PCA_ROI_FlagY = 0;
      Reg_ROI_FlagY = 0;
      
      for (; x < BoundaryMaxX ; x++) {
#if (EMVERBOSE) 
          if (y > 26 && x > 80) { 
        cout << "============================================================" << endl;
        cout << "Z:" << z << "Y:" << y << "X:" << x << "  cY_M: "; 
        for (int l= 0 ; l <  NumInputImages; l ++) {
           cout << cY_M[l] << " " ;
        }        
        fprintf(stdout, "\nw_m               class SubCl ClaIdx  PrbMinus  PWeight   PrbData  TisProb  IntensProb\n"); 
      }
#endif

      if (*OutputVector < EMSEGMENT_NOTROI) {
        int index = NumTotalTypeCLASS - 1;
            normRow = 0.0;
            SumOfAlignedTissueDistribution = 0.0;
            
            if (PCA_ROI) *PCA_ROI = 0;
            if (Reg_ROI_MAP) *Reg_ROI_MAP = -1;
  
            // We are going backwards so we can calculate implicitly the background if necessary
            if ((RegistrationType ==  EMSEGMENT_REGISTRATION_GLOBAL_ONLY) && (NumClasses > 0)) 
           EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(this->ClassToAtlasRotationMatrix[NumClasses -1], 
                                        this->ClassToAtlasTranslationVector[NumClasses -1], indexX, indexY, indexZ, 
                                        targetX, targetY, targetZ,targetmidcol, targetmidrow, targetmidslice);
            for (int i = NumClasses -1; i > -1 ; i--) {
          // ------------------------------------------------
          // Setup spatially variing parametes that are true across sub-structures 
          // ------------------------------------------------
          // IPMI05 include registration and Generation of Background class
          if (!i &&  GenerateBackgroundProbability) {
        // Generate Background probability by the inverse of the rest
        AlignedTissueDistribution = double(SumOfAlignedTissueDistribution < NumberOfTrainingSamples ? 
                           (this->NumberOfTrainingSamples - SumOfAlignedTissueDistribution) : 0.0); 
        SpatialTissueDistribution = this->ProbDataMinusWeight[i]  + this->ProbDataWeight[i] * AlignedTissueDistribution; 
          } else if ((this->RegistrationType >  EMSEGMENT_REGISTRATION_DISABLED) && (this->RegistrationType !=  EMSEGMENT_REGISTRATION_GLOBAL_ONLY)) {
        EMLocalInterface_findCoordInTargetOfMatchingSourceCentreTarget(ClassToAtlasRotationMatrix[i], ClassToAtlasTranslationVector[i], 
                                         indexX, indexY, indexZ, targetX, targetY, targetZ,targetmidcol, 
                                         targetmidrow, targetmidslice);
          }
          
          if (Reg_ROI_MAP) Reg_ROI_FlagX = 0;

          // Regularize Weights With MeanField
          if (this->Alpha > 0.0 ) MeanFieldPotential = this->NeighberhoodEnergy(w_m_input, *OutputVector,i);
          
          // Work of ISBI04: A superclass is defined by the atlas information of its subclasses         
          // Do not forget to update _IncompleteModel E-Step after making changes to this section 
          for (int k = this->NumChildClasses[i] -1 ; k >  -1 ; k --) {
        // ------------------------------------------------
        // Define Spatial prior for everything but background
        // ------------------------------------------------
        if (i || !GenerateBackgroundProbability) {
          if (this->ProbDataPtrStart[index]) {                                                         
            if (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) {                            
              // ----------------------------------------------------------------------------    
              // IPMI 05 - MICCAI 05 Registration of spatial prior (spatial prior might be generated from shape if the two of them are done 
              //                     together  
              // ----------------------------------------------------------------------------    
              AlignedTissueDistribution = EMLocalInterface_Interpolation(targetX, targetY, targetZ, this->RealMaxX, this->RealMaxY, this->RealMaxZ, 
                                           this->ProbDataPtrStart[index], this->ProbDataIncY[index], 
                                           this->ProbDataIncZ[index], RegistrationInterpolationType, 
                                           this->RegistrationParameters->GetImage_Length()); 
            } else {                                                                             
              // ----------------------------------------------------------------------------      
              // MICCAI 02 - Use non-rigid aligned spatial prior for the segmentation process      
              // ----------------------------------------------------------------------------      
              AlignedTissueDistribution = double(*ProbDataPtrCopy[index]);             
            }
          } else if (this->PCANumberOfEigenModes[index]) {                                             
            // ----------------------------------------------------------------------------      
            // ICCV 05 - Shape Stuff - shape is turned into spatial prior 
            //                       - registration is turned off                       
            // ----------------------------------------------------------------------------      
            AlignedTissueDistribution =  ShapeParameters->Transfere_DistanceMap_Into_SpatialPrior(EMLocalInterface_CalcDistanceMap(
                                      this->PCAShapeParameters[index], PCAEigenVectorsPtr[index], PCAMeanShapePtr[index], 
                                      this->PCANumberOfEigenModes[index],0), this->PCALogisticSlope[index], this->PCALogisticBoundary[index], 
                                      this->PCALogisticMin[index], this->PCALogisticMax[index]); 
          } else {                                                                               
            // ----------------------------------------------------------------------------      
            // Wells 96 - do not use spatial priors                                              
            // ----------------------------------------------------------------------------      
            AlignedTissueDistribution = 0.0;                                                       
          }                        
          SumOfAlignedTissueDistribution += AlignedTissueDistribution;
          // Work of MICCAI02 and ISBI04              
          SpatialTissueDistribution    = ProbDataMinusWeight[i]  + ProbDataWeight[i] * AlignedTissueDistribution; 
        }  //end of if (i || !GenerateBackgroundProbability) {
        
        // ------------------------------------------------
        // Define weights of cluster
        // ------------------------------------------------
        // Multiply things together
        ConditionalTissueProbability =  this->TissueProbability[i]* vtkImageEMGeneral::FastGaussMulti(this->InvSqrtDetWeightedLogCov[index],cY_M, this->LogMu[index],
                                                          this->InverseWeightedLogCov[index],this->NumInputImages, 
                                                          this->VirtualNumInputImages[i]);

        *(w_m_output[index]) = (float)  ConditionalTissueProbability * SpatialTissueDistribution * MeanFieldPotential; 
        
        // Find out where we have to concentrate our PCA anlysis on 
        if (PCANumberOfEigenModes[index] && *w_m_output[index] > 0.0 && (!*PCA_ROI)) {
          *PCA_ROI = 1;
          PCA_ROI_FlagY = 1;
          PCA_ROI_FlagZ =1;
          Thread_PCA_ROIExactVoxelCount ++;
          if (Thread_PCAMin[0] > x) Thread_PCAMin[0] =x;
          if (Thread_PCAMax[0] < x) Thread_PCAMax[0] =x;
        }
        
        if (Reg_ROI_MAP && *w_m_output[index] > 0.0) Reg_ROI_FlagX ++;
        
        normRow += *w_m_output[index];
#if (EMVERBOSE)
        fprintf(stdout, "w_m: %12g i:%2d  k:%2d  ind:%2d  PMW:%3.2f  PDW:%3.2f TP:%3.2f ATD:%3g", *w_m_output[index],i,k,index,ProbDataMinusWeight[i], ProbDataWeight[i],TissueProbability[i], AlignedTissueDistribution); 
        if (TissueProbability[i]) fprintf(stdout, " GID:%5.4f \n", float(ConditionalTissueProbability)/float(TissueProbability[i]));
#endif
        index --;
          } // End of for (k=0;k< NumChildClasses[i];k++) ...
                    
          // Define parameters for registration
          if (Reg_ROI_FlagX) {
        if (*Reg_ROI_MAP > -2) {
          // If independent flag is set we also have to check subclasses - if Reg_ROI_FlagX > 1 than more than two supclasses belong to it so we have to consider it   
          if (RegistrationIndependentSubClassFlag[i] && (Reg_ROI_FlagX  > 1)) {
            *Reg_ROI_MAP = -2;
          }
          else {
            // There are more than on class with weights greater 0 
            if (*Reg_ROI_MAP > -1) *Reg_ROI_MAP = -2;
            else *Reg_ROI_MAP = i;
          }
        }
          }
        } // End of for (i=0; i < NumClasses; 
#if (0 || EMVERBOSE)
        fprintf(stdout, "normRow: %8g \n",normRow);
#endif

        if (Reg_ROI_MAP && (*Reg_ROI_MAP !=  Reg_ROI_ClassOutside)) {
          Reg_ROI_FlagY = 1;
          Reg_ROI_FlagZ = 1;
          if (Thread_Registration_ROI_Weight->MinCoord[0] > x)  Thread_Registration_ROI_Weight->MinCoord[0] = x;
          if (Thread_Registration_ROI_Weight->MaxCoord[0] < x)  Thread_Registration_ROI_Weight->MaxCoord[0] = x;
        }
        
        // -------------------------------------------------------------------------------
        // If normRow == 0 in general means that the spatial distribution of all the classes did not overlap with the intensity pattern of these classes 
        // Thus -> these structures should not be included into the bias caluclation because our current model does not explain them well enough 
        // Therfore we will introduce a new trsh class into the model that collects these points  
        if (normRow == 0.0) {
          this->E_Step_IncompleteModel(indexX, indexY, indexZ, w_m_input, w_m_output, ProbDataPtrCopy, normRow, cY_M, 
                       PCAEigenVectorsPtr, PCAMeanShapePtr, *OutputVector); 
          // Think about how to properly calculate 
          *OutputVector |= EMSEGMENT_INCORRECT_MODEL;
          Thread_IncompleteModelVoxelCount ++;
        } else {
          // if  EMSEGMENT_INCORRECT_MODEL was set returns the bit the 0 
          *OutputVector &= EMSEGMENT_CORRECT_MODEL;
        }
      
        // -------------------------------------------------------------------------------
        // Normalize Rows and determine if we have to recalculate value at next iteration 
        // If only one structure has prob value greater 1 than just forget it 
        if (normRow > 0.0) {
          for (int j=0; j < NumTotalTypeCLASS; j++)  *w_m_output[j] = *w_m_output[j]/normRow;  
        } else { 
          // Probabilityt Map is 0 for every class -> Label is assigned to first class
          **w_m_output = 1.0;
          for (int j=1; j < NumTotalTypeCLASS; j++) *w_m_output[j] = 0.0; 
        }
      }
      // end of if (*OutputVector < EMSEGMENT_NOTROI) .. 
      // Kilian: Changed this at 01-Nov-04 before it was updating all these values for no reason !
      
      // -------------------------------------------------------------------------------
      // Go to next x value 
      cY_M += NumInputImages; OutputVector++; 
      if (PCA_ROI) PCA_ROI++;
      if (Reg_ROI_MAP) Reg_ROI_MAP++;
      
      for (int j=0; j < NumTotalTypeCLASS; j++) w_m_input[j] ++; 
      for (int j=0; j < NumTotalTypeCLASS; j++) w_m_output[j] ++;

      if (RegistrationType >  EMSEGMENT_REGISTRATION_DISABLED) indexX ++;
      else {
        for (int j=0; j < NumTotalTypeCLASS; j++) {
          // Kilian later change this so that non of the PCA is changed
          if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] ++;
          if (PCAEigenVectorsPtr[j]) { for (int l = 0 ; l < PCANumberOfEigenModes[j]; l++)  PCAEigenVectorsPtr[j][l] ++; }
          if (PCAMeanShapePtr[j])  PCAMeanShapePtr[j] ++;
        }
      }
      Thread_VoxelCount ++;

      if (Thread_VoxelCount > Thread_NumberOfVoxels)     break;
    } // End of for (x =0 ; ... 
    if (PCA_ROI_FlagY) {
      if (Thread_PCAMin[1] > y) {
        Thread_PCAMin[1] = y;
      }
      if (Thread_PCAMax[1]< y) {
        Thread_PCAMax[1] = y;
      }
    }
    
    if (Reg_ROI_FlagY) {
      if (Thread_Registration_ROI_Weight->MinCoord[1] > y)  Thread_Registration_ROI_Weight->MinCoord[1] = y;
      if (Thread_Registration_ROI_Weight->MaxCoord[1] < y)  Thread_Registration_ROI_Weight->MaxCoord[1] = y;
    }
    
    if (RegistrationType >  EMSEGMENT_REGISTRATION_DISABLED) indexY ++;
    else {
      for (int j=0; j < NumTotalTypeCLASS; j++) {
        if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] += ProbDataIncY[j];
        if (PCAEigenVectorsPtr[j]) {for (int l = 0 ; l < PCANumberOfEigenModes[j]; l++) PCAEigenVectorsPtr[j][l] += PCAEigenVectorsIncY[j][l];}
        if (PCAMeanShapePtr[j])  PCAMeanShapePtr[j] += PCAMeanShapeIncY[j];
      }
    }
    if (Thread_VoxelCount > Thread_NumberOfVoxels) break;
      } // End of for (y =0 ; ...
      
      if (PCA_ROI_FlagZ) {
    if (Thread_PCAMin[2] > z) Thread_PCAMin[2] = z;
    Thread_PCAMax[2] = z;
      } 
      
      if (Reg_ROI_FlagZ) {
    if (Thread_Registration_ROI_Weight->MinCoord[2] > z)  Thread_Registration_ROI_Weight->MinCoord[2] = z;
    Thread_Registration_ROI_Weight->MaxCoord[2] = z;
      }
      
      if (RegistrationType >  EMSEGMENT_REGISTRATION_DISABLED) indexZ ++;
      else {
    for (int j=0; j < NumTotalTypeCLASS; j++) {
      if (ProbDataPtrCopy[j]) ProbDataPtrCopy[j] += ProbDataIncZ[j];
      if (PCAEigenVectorsPtr[j]) { for (int l = 0 ; l < PCANumberOfEigenModes[j]; l++)  PCAEigenVectorsPtr[j][l] += PCAEigenVectorsIncZ[j][l];}
      if (PCAMeanShapePtr[j])  PCAMeanShapePtr[j] += PCAMeanShapeIncZ[j];
    }
      }
      if (Thread_VoxelCount > Thread_NumberOfVoxels)     break;
    } // End of for (z = 0; z < BoundaryMaxZ ; z++) 
    
    // Delete Variables     
    delete[] ProbDataPtrCopy;
    delete[] PCAMeanShapePtr; 
    for (int i = 0; i < NumTotalTypeCLASS; i++)  if (PCAEigenVectorsPtr[i]) delete[] PCAEigenVectorsPtr[i];
    delete[] PCAEigenVectorsPtr; 
    
    delete[] w_m_input;
    delete[] w_m_output;
   
#if (EMVERBOSE)
  cout << "End of E-Step  " << endl;    
#endif   
}



//------------------------------------------------------------
// M-Step Functions 
//------------------------------------------------------------

//------------------------------------------------------------
// Compute Image Inhomogeneity
// compute weighted residuals 
// r_m  = (w_m.*(repmat(cY_M,[1 num_classes]) - repmat(mu,[prod(imS) 1])))*(ivar)';
// iv_m = w_m * ivar';
// Compare to sandy i = l => he does exactly the same thing as I do

template <class T>
void EMLocalAlgorithm<T>::EstimateImageInhomegeneity(float *skern, EMTriVolume& iv_m, EMVolume *r_m) {
  int VoxelIndex = 0;
  float** InputVector = this->InputVectorPtr;
  unsigned char* OutputVector = this->OutputVectorPtr;
  float temp;

  float **w_m        = new float*[this->NumTotalTypeCLASS];
  for (int i=0; i< this->NumTotalTypeCLASS; i++) w_m[i] = this->w_mPtr[i];

  for (int i = 0; i< BoundaryMaxZ;i++){
    for (int k = 0; k<BoundaryMaxY;k++){
      for (int j = 0; j<BoundaryMaxX;j++){
      // If coordinate is not in Region of interest just do not update bias 
        // Just use the outcome of the last hierarchical level 
      if (*OutputVector < EMSEGMENT_NOTROI) {
        for (int m=0; m<NumInputImages; m++) {
          r_m[m](i,k,j) = 0.0;
          for (int n=0; n<=m; n++) iv_m(m,n,i,k,j) = 0.0;
        } 
        
        for (int l=0; l< NumTotalTypeCLASS; l++) {
          for (int m=0; m<NumInputImages; m++) {  
            for (int n=0; n<NumInputImages; n++) {
          temp =  *w_m[l] * float(InverseWeightedLogCov[l][m][n]);
          r_m[m](i,k,j)     += temp * ((*InputVector)[n] - float(LogMu[l][n]));
          if (n <= m) iv_m(m,n,i,k,j) += temp;
            }
          }
  
          w_m[l]++;
        }
      } else { //  if (*OutputVector < EMSEGMENT_NOTROI) ....
        for (int l=0; l< NumTotalTypeCLASS; l++) {
          w_m[l] ++;
        }
      }
      InputVector ++;
      OutputVector ++;
      VoxelIndex ++;
      }
    } 
  } // End of for (z = 0; z < BoundaryMaxZ ; z++) 

  delete[] w_m;

  
  //------------------------------------------------------------
  // Finalize Bias Parameters  
  // smooth residuals and inv covariances - 3D
  // w(k) = sum(u(j)*v(k+1-j))
  
  // When you change the window size (SegmentationBoundary) then it will have an impact on the convolution  
  // Otherwise there is still a bug in the software
  // 20-Nov-03 I checked it - Printed out the weights -> they7 are correct - then I let it iterate 10 times just commenting out the convolutio part 
  // no difference between Acitvated SegmentationBoundary and not Activated one !  
  
  // Kilian: Does not account for parts were *OutputVector |= EMSEGMENT_INCORRECT_MODEL is set 
  // what should we do ? 
  //------------------------------------------------------------
  iv_m.Conv(skern,SmoothingWidth);
  for (int i=0; i< this->NumInputImages; i++) r_m[i].Conv(skern,SmoothingWidth); 
}

// -----------------------------------------------------------
// Intensity Correction 
// -----------------------------------------------------------
// estimate the smoothed image inhomoginities (bias + resiuduum)
// transform r (smoothed weighted residuals) by iv (smoother inv covariances)
// b_m = r_m./iv_m ;

template <class T> void EMLocalAlgorithm<T>::IntensityCorrection(int PrintIntermediateFlag, int iter, EMTriVolume &iv_m, EMVolume *r_m, float *cY_M) {
  // If needed the bias can also be printed out if ROI != NULL - just have to do slight modifications 

  unsigned char* OutputVector = this->OutputVectorPtr;
  float** InputVector         = this->InputVectorPtr;

  double **iv_mat     = new double*[VirtualOveralInputChannelNum];
  double **inv_iv_mat = new double*[VirtualOveralInputChannelNum];
  for (int i=0; i < VirtualOveralInputChannelNum; i++) {
    iv_mat[i]     = new double[VirtualOveralInputChannelNum];
    inv_iv_mat[i] = new double[VirtualOveralInputChannelNum];
  }

  int mindex, lindex; 
  char** BiasFileName       = NULL;

  bool PrintBiasFlag        = bool(PrintIntermediateFlag && this->BiasPrint && (!this->ROIPtr));  
  float* BiasSlice         = NULL;
  float* BiasSlicePtr      = NULL;
  float Bias;

  if (PrintBiasFlag) {
    BiasSlicePtr = BiasSlice = new float[ImageProd*NumInputImages];
    memset(BiasSlice, 0, sizeof(float)*ImageProd*NumInputImages);
 
    BiasFileName = new char*[this->NumInputImages];
    for (int i = 0 ; i < this->NumInputImages; i++)  BiasFileName[i] = new char[100];  

    for (int l=0; l< this->NumInputImages; l++) {
      if (this->PrintDir) sprintf(BiasFileName[l],"%s/Bias/BiasL%sI%dCh%d",this->PrintDir,this->LevelName,iter,l);
      else sprintf(BiasFileName[l],"Bias/BiasL%sI%dCh%d",this->LevelName,iter,l);
      // If BoundaryMin and Max do not span full length we have to add empty slices 
      for (int i = 1; i < SegmentationBoundaryMin[2]; i++) EMLocalAlgorithm_PrintDataToOutputExtension(this,BiasSlice,VTK_FLOAT,BiasFileName[l],i-SegmentationBoundaryMin[2],0,0);
      for (int i = 1; i <= Extent[5]- Extent[4] + 1 - SegmentationBoundaryMax[2]; i++) EMLocalAlgorithm_PrintDataToOutputExtension(this,BiasSlice,VTK_FLOAT,BiasFileName[l],i-SegmentationBoundaryMin[2],0,0);
    }
  }

  for (int i = 0; i<BoundaryMaxZ;i++){
    // Define Bias Value
    for (int j = 0; j<BoundaryMaxY;j++){
      for (int k = 0; k<BoundaryMaxX;k++){
        if (*OutputVector++ < EMSEGMENT_NOTROI) {
          lindex =0;
        for (int l=0; l< this->VirtualOveralInputChannelNum ; l++) {
             while (!this->VirtualOveralInputChannelFlag[lindex]) lindex ++; 
           iv_mat[l][l] = iv_m(lindex,lindex,i,j,k);
             mindex = 0;
           for (int m = 0; m<= l; m++) {
              while (!VirtualOveralInputChannelFlag[mindex]) mindex ++; 
              iv_mat[m][l] = iv_mat[l][m] = iv_m(lindex,mindex,i,j,k);
                mindex ++;
          }
            lindex ++;
        }
        if (vtkImageEMGeneral::InvertMatrix(iv_mat, inv_iv_mat,VirtualOveralInputChannelNum)) {
          lindex = 0;
          for (int l=0; l< NumInputImages; l++) {
            // Only update those values that are in the region of interest 
            Bias = 0.0;
            if (VirtualOveralInputChannelFlag[l]) {
          mindex = 0;
          for (int m = 0; m< NumInputImages; m++) {
            if (VirtualOveralInputChannelFlag[m]) {
              Bias += inv_iv_mat[lindex][mindex]*r_m[m](i,j,k);
              mindex ++;
            }
          }
          lindex ++;
          (*cY_M ++) = fabs((*InputVector)[l] - double(Bias));
                if (BiasSlice) (*BiasSlice ++) = Bias;
            } else {
          cY_M ++;
              if (BiasSlice) BiasSlice ++;
            }
          }
        } else { 
          for (int l=0; l< NumInputImages; l++) {
          (*cY_M ++) = fabs((*InputVector)[l]);
            if (BiasSlice) (*BiasSlice ++) = 0.0;
          }
        }
        } else {
          cY_M       += NumInputImages;
          if (BiasSlice) BiasSlice  += NumInputImages;
        }
        InputVector++;
      }
    }
    // Print Bias Field if necessary  
    if (PrintBiasFlag) {
      float *BiasSliceInput = new float[ImageProd];
      for (int l=0; l< NumInputImages; l++) {
         BiasSlice = BiasSlicePtr + l;

         for (int m = 0 ; m < imgXY; m ++) {
         BiasSliceInput[m] = *BiasSlice;
         BiasSlice += NumInputImages;
         }
         // Remember for windows always use - BiasFile = fopen(BiasFileName, "wb") - otherwise does not work for double or float    
         EMLocalAlgorithm_PrintDataToOutputExtension(this,BiasSliceInput,VTK_FLOAT,BiasFileName[l],i+1,0,0);
       }
       delete[] BiasSliceInput;
    }
    BiasSlice = BiasSlicePtr;
  }


  if (BiasSlice) delete[] BiasSlice;


  if (BiasFileName) {
    for (int i = 0; i < NumInputImages; i++)  delete[] BiasFileName[i];
    delete[] BiasFileName;
  }

  for (int i=0; i <  VirtualOveralInputChannelNum; i++) {
    delete[] iv_mat[i];
    delete[] inv_iv_mat[i];
  }
  delete[] iv_mat;
  delete[] inv_iv_mat;
}

// Initialize Intensities for EM-Algorithm 
// If iter == 1 => Bias has been defined in the previous hierarchical level 
// cY_M  = fabs(InputVector - b_m) = {b_m ==0} = fabs(InputVector) = InputVector;
// we assume InputVector >= 0
template <class T> void EMLocalAlgorithm<T>::InitializeLogIntensity(int HeadLevelFlag, EMTriVolume& iv_m, EMVolume *r_m, float *cY_M) {
  // Is the top level - bias is not calculated so far
  if (HeadLevelFlag) {
    float** InputVector = this->InputVectorPtr;
    for (int i=0; i< this->ImageProd; i++) {
      for (int l=0; l< this->NumInputImages; l++) (*cY_M ++) = fabs((*InputVector)[l]);
      InputVector++;
    }
  } else this->IntensityCorrection(0, 0, iv_m, r_m, cY_M);
}



//------------------------------------------------------------
// Define new registration parameters   

template <class T>
int EMLocalAlgorithm<T>::EstimateRegistrationParameters(int iter, float &RegistrationCost, float &RegistrationClassSpecificCost) {

  // Only in first registration do global registration
  // Do not move bc we need the weight calculation
  if ((iter == 1) && this->PrintFrequency && (this->actSupCl->GetPrintRegistrationParameters() || this->actSupCl->GetPrintRegistrationSimularityMeasure()) ) 
    this->PrintRegistrationData(this->actSupCl->GetPrintRegistrationSimularityMeasure(), this->RegistrationTranslation, this->RegistrationRotation,this->RegistrationScale,0);
  

  if (this->RegistrationType < EMSEGMENT_REGISTRATION_SEQUENTIAL) {
    if (1) {
      this->RegistrationInterface(RegistrationCost);
  
      if (this->RegistrationType != EMSEGMENT_REGISTRATION_CLASS_ONLY) {
        if (EMLocalAlgorithm_RegistrationMatrix(this->RegistrationTranslation[0],this->RegistrationRotation[0],this->RegistrationScale[0], this->GlobalRegInvRotation, 
                            this->GlobalRegInvTranslation, this->SuperClassToAtlasRotationMatrix, this->SuperClassToAtlasTranslationVector, 
                            this->TwoDFlag)) {
          vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: Cannot invert rotation matrix defined by the pararmeters of actual Super Class");
          return 0 ;      
        }
      }
    } else { cout << "Jump over registration "<< endl; }
  } else {
    // ---------------------------------------------------------------------------------------------------------------
    // SEQUENTIAL REGISTRATION
    // First do global registration and then the structure specficic registration
    // ---------------------------------------------------------------------------------------------------------------
    // Global Registration
    int OrigNumberOfParaSets  =  this->RegistrationParameters->GetNumberOfParameterSets();
    cout << "Kilian: Don't the next to lines go hand in hand" << endl;
    this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_GLOBAL_ONLY); 
    this->RegistrationParameters->SetNumberOfParameterSets(1);
    this->RegistrationInterface(RegistrationCost);
    if (EMLocalAlgorithm_RegistrationMatrix(this->RegistrationTranslation[0],this->RegistrationRotation[0],this->RegistrationScale[0], this->GlobalRegInvRotation, 
                          this->GlobalRegInvTranslation, this->SuperClassToAtlasRotationMatrix, this->SuperClassToAtlasTranslationVector, this->TwoDFlag)){
      vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: Cannot invert rotation matrix defined by the pararmeters of actual Super Class");
      return 0 ;      
    }
  
    // Structure specific Registration
    cout << "Kilian: Don't the next to lines go hand in hand" << endl;
    this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_CLASS_ONLY);
    this->RegistrationParameters->SetNumberOfParameterSets(OrigNumberOfParaSets - 1); 
  
    this->RegistrationTranslation ++;
    this->RegistrationRotation ++;
    this->RegistrationScale ++;
  
    this->RegistrationInterface(RegistrationClassSpecificCost);
  
    this->RegistrationTranslation --;
    this->RegistrationRotation --;
    this->RegistrationScale --;
  
    cout << "Kilian: Don't the next to lines go hand in hand" << endl;
    this->RegistrationParameters->SetRegistrationType(EMSEGMENT_REGISTRATION_SEQUENTIAL);
    this->RegistrationParameters->SetNumberOfParameterSets(OrigNumberOfParaSets);
  
  } // End of seeuqntial segmentation
  
  // -------------------------------------------------------------
  // Update Class Specific Registration
  // -------------------------------------------------------------
  int ParaSetIndex =(this->RegistrationType !=  EMSEGMENT_REGISTRATION_CLASS_ONLY ? 1: 0); 
  for (int i = GenerateBackgroundProbability ; i < this->NumClasses; i++) {
    if ((this->RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY) && this->RegistrationClassSpecificRegistrationFlag[i]) {
      if (vtkSimonParameterReaderWriter::TurnParameteresIntoInverseRotationTranslation(this->RegistrationTranslation[ParaSetIndex],this->RegistrationRotation[ParaSetIndex],
                           this->RegistrationScale[ParaSetIndex], this->ClassToSuperClassRotationMatrix[i],this->ClassToSuperClassTranslationVector[i],
                                                  2, this->TwoDFlag, this->RigidFlag)) {
        vtkEMAddErrorMessage("vtkImageEMLocalAlgorithm: Cannot invert rotation matrix defined by the pararmeters of class " << i);
        return 0;      
      }
      ParaSetIndex ++;
    }
    // In the case where (RegistrationType == EMSEGMENT_REGISTRATION_CLASS_ONLY) && RegistrationClassSpecificRegistration[i] == 0 we perform this step even though nothing is changing
    vtkSimonParameterReaderWriter::matmult_3x4(SuperClassToAtlasRotationMatrix, SuperClassToAtlasTranslationVector,ClassToSuperClassRotationMatrix[i], 
                           ClassToSuperClassTranslationVector[i], ClassToAtlasRotationMatrix[i],ClassToAtlasTranslationVector[i]);
  }
  return 1;
}


//------------------------------------------------------------
// Define Shape parameters 

template <class T>
float EMLocalAlgorithm<T>::EstimateShapeParameters(int iter) {
   float Cost;

   itkEMLocalOptimization_Shape_Start(this->ShapeParameters, this->PCAShapeParameters, this->PCAMax[0], this->PCAMin[0], this->PCAMax[1], this->PCAMin[1], 
                    this->PCAMax[2], this->PCAMin[2], this->SegmentationBoundaryMin[0] -1, this->SegmentationBoundaryMin[1] - 1, 
                    this->SegmentationBoundaryMin[2] -1, this->BoundaryMaxX, this->BoundaryMaxY, this->w_mPtr, this->PCA_ROI_Start, 
                    ((void**) this->ProbDataPtrStart), this->PCAMeanShapePtrStart, this->PCAMeanShapeIncY, this->PCAMeanShapeIncZ, 
                    this->PCAEigenVectorsPtrStart, this->PCAEigenVectorsIncY, this->PCAEigenVectorsIncZ, Cost);

    // ---------------------------------------------------
    // Print out initializing cost  if needed
    if ((iter == 1) && this->PrintFrequency && (this->PCAFile || this->actSupCl->GetPrintShapeSimularityMeasure())) {
      float **initPCAShapeParameters               = new float*[this->NumTotalTypeCLASS];
      for (int i = 0; i < this->NumTotalTypeCLASS; i++) {
      if (this->PCANumberOfEigenModes[i]) {
        initPCAShapeParameters[i] =  new float[this->PCANumberOfEigenModes[i]];
           memset(initPCAShapeParameters[i],0,sizeof(float)*this->PCANumberOfEigenModes[i]);
      } else {
          initPCAShapeParameters[i] = NULL;
      }
      }
 
      float initPCACost = this->PrintShapeData(initPCAShapeParameters,0,this->actSupCl->GetPrintShapeSimularityMeasure());
      if (PCAFile)  EMLocalAlgorithm_PrintPCAParameters(this->ShapeParameters,this->PCAFile, initPCAShapeParameters, this->LabelList, initPCACost);
 
      for (int i = 0; i < NumTotalTypeCLASS; i++) if (initPCAShapeParameters[i]) delete[] initPCAShapeParameters[i];
      delete[] initPCAShapeParameters;
    }
    return Cost;
}

//------------------------------------------------------------
// The Entire Algorithm
//------------------------------------------------------------

template  <class T> void EMLocalAlgorithm<T>::RunAlgorithm(EMTriVolume& iv_m, EMVolume *r_m, int &SegmentLevelSucessfullFlag) {

  cout << endl << "========== vtkImageEMLocalAlgorithm: Start Initialize Variables "<< endl;;
  //cout << "-------------------------------------------------" << endl;
  //cout << "         DEVELOPER VERSION" << endl;
  //cout << "-------------------------------------------------" << endl;

  SegmentLevelSucessfullFlag = 1;
  int iter = 0;

  if (this->PrintFrequency) this->InfoOnPrintFlags(); 
  // cY_M correct log intensity - dimension NumInputImages x ImageProd 
  this->InitializeLogIntensity(!this->ROIPtr,iv_m,r_m, cY_MPtr);     

  // ------------------------------------------------------------
  // M Step Variables 
  // ------------------------------------------------------------ 

  // Done to increase speed 
  double lbound = (-(this->SmoothingWidth-1)/2); // upper bound = - lbound
  float *skern = new float[this->SmoothingWidth];
  for (int i=0; i < this->SmoothingWidth; i++) skern[i] = float(vtkImageEMGeneral::FastGauss(1.0 / SmoothingSigma,i + lbound));

  // Later define a Print function for E and M step sperately so that we can more easily seperate E and M-Step variables 
  float PCACost = 0;
  float RegistrationCost   = 0.0;
  float RegistrationClassSpecificCost   = 0.0;

  // ------------------------------------------------------------
  // Debugging Variables
  // ------------------------------------------------------------ 

#ifndef _WIN32
  START_PRECISE_TIMING;
#endif

  // ------------------------------------------------------------
  // Start Algorithm 
  // ------------------------------------------------------------
  for (;;) {

    iter ++;
    this->PrintIntermediateFlag = ((this->PrintFrequency > 0) && (iter % (this->PrintFrequency) == 0));
  
    // -----------------------------------------------------------
    // E-Step
    // -----------------------------------------------------------
    if (this->StopEMType) cout << "--------------------------------------" << endl;  
    cout << endl << "vtkImageEMLocalAlgorithm: "<< iter << ". E-Step " << endl;
    this->Expectation_Step(iter);
    // -----------------------------------------------------------
    // M-step Part 
    // -----------------------------------------------------------
    if ((iter < NumIter) &&  !this->EMStopFlag) {
  
      cout << "vtkImageEMLocalAlgorithm: M-Step " << endl; 
      // Image Inhomogeneity
      if ((StopBiasCalculation < 0)  ||  (iter <= StopBiasCalculation)) {
        this->EstimateImageInhomegeneity(skern, iv_m, r_m);
        this->IntensityCorrection(PrintIntermediateFlag, iter, iv_m, r_m, cY_MPtr);
      } else cout << "Bias calculation disabled " << endl; 
     
      // Registration
      if (RegistrationType > EMSEGMENT_REGISTRATION_APPLY) {
        SegmentLevelSucessfullFlag = this->EstimateRegistrationParameters(iter, RegistrationCost, RegistrationClassSpecificCost);
        if (!SegmentLevelSucessfullFlag) break; 
        if (PrintIntermediateFlag) this->Print_M_StepRegistrationToFile(iter, RegistrationCost, RegistrationClassSpecificCost); 
      }
     
       // Shape
      if (PCATotalNumOfShapeParameters && (this->PCAShapeModelType < EMSEGMENT_PCASHAPE_APPLY)) {
         PCACost = this->EstimateShapeParameters(iter);
         if (PrintIntermediateFlag) this->Print_M_StepShapeToFile(iter, PCACost);
         // has to do be done  after printing otherwise printing similarity measure is corrupted 
         // as logistic slope is alteres
         this->UpdatePCASpecificParameters(iter); 
      }
  
    } else {
      if (this->PrintFrequency == -1) this->Print_M_StepResultsToFile(iter, PCACost, RegistrationCost, RegistrationClassSpecificCost, iv_m, r_m, cY_MPtr);
      break;
    }
  } // End Of EM-Algorithm ( for (;;) ....
  
#ifndef _WIN32
  END_PRECISE_TIMING;
  SHOW_ELAPSED_PRECISE_TIME;  
#endif

  if (LabelMapEMDifferenceFile) {
    fprintf(LabelMapEMDifferenceFile,"%% Number Of Iterations: %d \n", iter);
    fprintf(LabelMapEMDifferenceFile,"%% Maximum Iteration Border: %d \n", NumIter); 
  }
  
  if (WeightsEMDifferenceFile) {
    fprintf(WeightsEMDifferenceFile,"\n%% Number Of Iterations: %d \n", iter);
    fprintf(WeightsEMDifferenceFile,"\n%% Maximum Iteration Border: %d \n", NumIter);
  }

  delete[] skern;
  cout << "EMLocalAlgorithm::RunAlgorithm: Finished " << endl;
}



