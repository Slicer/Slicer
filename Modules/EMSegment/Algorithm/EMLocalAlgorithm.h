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

#ifndef _EMLOCALALGORITHM_H_INCLUDED
#define _EMLOCALALGORITHM_H_INCLUDED 1

#include "EMLocalShapeCostFunction.h"
#include "EMLocalRegistrationCostFunction.h"

// -----------------------------------------------------------
// Structures needed for MultiThreading 
// -----------------------------------------------------------
  
typedef struct {
  int   VoxelStart[3];
  int   DataJump;
  int   *PCAMeanShapeJump;
  int   **PCAEigenVectorsJump;
  int   *ProbDataJump;
  // Job specifc number of Voxels in Image Space (not considering boundaries); 
  int   NumberOfVoxels;
  int   PCAMin[3];
  int   PCAMax[3];
  EMLocalRegistrationCostFunction_ROI Registration_ROI_Weight;
  int   IncompleteModelVoxelCount;
  int   PCA_ROIExactVoxelCount;
} EMLocalAlgorithm_E_Step_MultiThreaded_Parameters; 

typedef struct {
  void *self;
  int  DataType;
} EMLocalAlgorithm_E_Step_MultiThreaded_SelfPointer;


// -----------------------------------------------------------
// Main Class Definition 
// -----------------------------------------------------------
template <class T> class EMLocalAlgorithm {
public:
  // -----------------------------------------------------------
  // Core Functions 
  // -----------------------------------------------------------
   void RunAlgorithm(EMTriVolume& iv_m, EMVolume *r_m, int &SegmentLevelSucessfullFlag);
  // Weight Calculation of E-Step
   void E_Step_Threader_FunctionStart(int CurrentThread);

  // Description:
  // Defines the Label map of a given image
  void DetermineLabelMap(short* LabelMap); 


   int Initialize(vtkImageEMLocalSegmenter *vtk_filter, T **ProbDataPtrStart,float** initInputVector, short *initROI, float **initw_m, char *initLevelName, 
                float initGlobalRegInvRotation[9], float initGlobalRegInvTranslation[3], int initRegistrationType, int DataType);

   EMLocalAlgorithm(vtkImageEMLocalSegmenter *vtk_filter, T **ProbDataPtrStart,float** initInputVector, short *initROI, float **initw_m, char *initLevelName, 
                 float initGlobalRegInvRotation[9], float initGlobalRegInvTranslation[3], int initRegistrationType, int DataType, int &SuccessFlag) {
     SuccessFlag = this->Initialize(vtk_filter, ProbDataPtrStart, initInputVector, initROI, initw_m, initLevelName, initGlobalRegInvRotation, initGlobalRegInvTranslation, 
                          initRegistrationType, DataType);
   }
   ~EMLocalAlgorithm();


  // -----------------------------------------------------------
  // Get Functions 
  // -----------------------------------------------------------

  int GetBoundaryMaxX() {return this->BoundaryMaxX;}
  // For legacy with older code
  int GetDimensionX() {return this->BoundaryMaxX;}
  int GetBoundaryMaxY() {return this->BoundaryMaxY;}
  int GetDimensionY() {return this->BoundaryMaxY;}
  int GetBoundaryMaxZ() {return this->BoundaryMaxZ;}
  int GetDimensionZ() {return this->BoundaryMaxZ;}

  int* GetExtent() {return this->Extent;}
  int* GetSegmentationBoundaryMin() {return this->SegmentationBoundaryMin;}

  // =============================
  // For Message Protocol
  // So we can also enter streams for functions outside vtk
  char* GetErrorMessages() {return this->ErrorMessage.GetMessages(); }
  int GetErrorFlag() {return  this->ErrorMessage.GetFlag();}

  char* GetWarningMessages() {return this->WarningMessage.GetMessages(); }
  int GetWarningFlag() {return  this->WarningMessage.GetFlag();}

private:
  // -----------------------------------------------------
  // Initialization Functions 
  // -----------------------------------------------------

  void InitializeEM(vtkImageEMLocalSegmenter* vtk_filter, char* initLevelName, int initRegistrationType, float** initInputVector, short *initROI, 
              int ROI_Label, float **initw_m);
  int InitializeClass(vtkImageEMLocalSuperClass* initactSupCl, T** ProbDataPtrStart);
  void InitializeHierarchicalParameters();
  void InitializeBias();
  void InitializePrint();
  int InitializeShape(); 
  int InitializeRegistration(float initGlobalRegInvRotation[9], float initGlobalRegInvTranslation[3]);
  void InitializeEStepMultiThreader(int DataType);


  // -----------------------------------------------------
  // Utility Functions 
  // -----------------------------------------------------

  void InfoOnPrintFlags();
  int CalcWeightedCovariance(double** WeightedInvCov, double & SqrtDetWeightedInvCov, float* Weights, double** LogCov, int & VirtualDim,int dim);
  // Shape functions 
  void Transfere_ShapePara_Into_SpatialPrior(int index);
  // Registration Related Functions 
  void DefineForRegistrationRotTranSca(int NumParaSets);
  int DefineGlobalAndStructureRegistrationMatrix();
  void RegistrationInterface(float &Cost);

  void DifferenceMeassure(int StopType, int PrintLabelMapConvergence, int PrintWeightsConvergence, int iter, short *CurrentLabelMap, float** w_m, 
              int &LabelMapDifferenceAbsolut, float &LabelMapDifferencePercent, float **CurrentWeights, float &WeightsDifferenceAbsolut, 
              float &WeightsDifferencePercent, float StopValue, int &StopFlag);

  // -----------------------------------------------------
  // EM Algorithm Functions 
  // -----------------------------------------------------
  void Expectation_Step(int iter); 
  void RegularizeWeightsWithMeanField(int iter);

  void E_Step_Weight_Calculation_Threaded(int Thread_VoxelStart[3], int Thread_NumberOfVoxels, int Thread_DataJump, 
                      int *Thread_PCAMeanShapeJump, int** Thread_PCAEigenVectorsJump, int *Thread_ProbDataJump,
                      int Thread_PCAMin[3], int Thread_PCAMax[3], EMLocalRegistrationCostFunction_ROI *Thread_Registration_ROI_Weight,
                      int &Thread_IncompleteModelVoxelCount,int &Thread_PCA_ROIExactVoxelCount);
  void E_Step_IncompleteModel(int indexX, int indexY, int indexZ, float **w_m_input, float **w_m_output, T **ProbDataPtrCopy, 
                  float &normRow, float *cY_M, float*** PCAEigenVectorsPtr, float **PCAMeanShapePtr, 
                  unsigned char OutputVector);

  void E_Step_ExecuteMultiThread();
  double NeighberhoodEnergy(float **w_m_input, unsigned char MapVector, int CurrentClass);

  // Mstep 
  //  - Bias
  void  EstimateImageInhomegeneity(float* skern, EMTriVolume& iv_m, EMVolume *r_m);
  // Have to do &iv_m bc have not programmed copy function
  void IntensityCorrection(int PrintIntermediateFlag, int iter, EMTriVolume &iv_m, EMVolume *r_m, float *cY_M);
  void InitializeLogIntensity(int HeadLevelFlag, EMTriVolume& iv_m, EMVolume *r_m, float *cY_M);
  //  - Registration
  int  EstimateRegistrationParameters(int iter, float &RegistrationCost, float &RegistrationClassSpecificCost);
  //  - shape
  float EstimateShapeParameters(int iter);
  void UpdatePCASpecificParameters(int iter);

  // -----------------------------------------------------
  // Print Functions
  // -----------------------------------------------------

  int DefinePrintRegistrationParameters(int NumParaSets);
  FILE* OpenTextFile(const char FileName[], int Label, int LabelFlag, int LevelNameFlag, int iter, int IterFlag, const char FileSucessMessage[]);

  float PrintShapeData(float **PCAShapeParameters, int iter , int PrintSimulatingFlag);
  void PrintRegistrationData(int SimularityFlag, double **SimplexTranslation, double **SimplexRotation,  double **SimplexScale, int iter);

  void Print_E_StepResultsToFile(int iter);
  void Print_M_StepRegistrationToFile(int iter, float RegistrationCost, float RegistrationClassSpecificCost);
  void Print_M_StepShapeToFile(int iter, float PCACost);
  void Print_M_StepResultsToFile(int iter, float PCACost, float RegistrationCost, float RegistrationClassSpecificCost, EMTriVolume& iv_m, EMVolume *r_m, float *cY_M); 

  // ------------------------------------------------------
  // Variables defined by current SuperClass and this Filter
  // ------------------------------------------------------
  // Based on Boundary Dimensions
  int NumInputImages;
  // Needed to print out empty slices for bias and registration 
  int *SegmentationBoundaryMin;
  int *SegmentationBoundaryMax;
  int *Extent;    

  // BoundaryMaxZ = DimensionZ = SegmentationBoundaryMax[2] - SegmentationBoundaryMin[2] + 1;
  // See  int vtkImageEMLocalSegmenter::GetDimensionZ in SetGetFunctions.cxx
  int BoundaryMaxZ;             
  int BoundaryMaxY;             
  int BoundaryMaxX;       
  // imgXY = BoundaryMaxY * BoundaryMaxX
  int imgXY;                    
  // ImageProd = BoundaryMaxZ * imgXY 
  int ImageProd;
                     
  int RealMaxX;                 
  int RealMaxY;                 
  int RealMaxZ;     

  double Alpha;                 
  int    NumberOfTrainingSamples;  
  int    RegistrationInterpolationType;

  char*  LevelName;  
  int    RegistrationType;
  int    DisableMultiThreading;

  int SmoothingWidth;
  int SmoothingSigma;

  ProtocolMessages ErrorMessage;    // Lists all the error messges -> allows them to be displayed in tcl too 
  ProtocolMessages WarningMessage;  // Lists all the error messges -> allows them to be displayed in tcl too 

  float  *cY_MPtr;

  char* PrintDir;

 
  // --------------------------------
  // Class Related Variables
  // --------------------------------
  vtkImageEMLocalSuperClass* actSupCl;
  void** ClassList;                  
  classType* ClassListType;            
  int NumClasses;                    
  int NumTotalTypeCLASS;             
  int GenerateBackgroundProbability; 
  int NumIter;                       
  int PrintFrequency;


  int *ProbDataIncZ;
  int *ProbDataIncY;

  double *TissueProbability;  
  float  *ProbDataWeight;     
  float  *ProbDataMinusWeight;

  int *VirtualNumInputImages; 

  T **ProbDataPtrStart; 

  double **LogMu;         
  double ***LogCovariance;
  double ***InverseWeightedLogCov;  
  double *InvSqrtDetWeightedLogCov; 
                      
  int *LabelList;                   
  int *CurrentLabelList;            

  // Number of child classes => if class is of Type CLASS => is set to 1
  // otherwise it is ther number of children of TYPE CLASS
  int *NumChildClasses; 

  // VirtualOveralInputChannelFlag[i] = 0 => input channel i is not used by any class 
  // VirtualOveralInputChannelFlag[i] = 1 => input channel i is used by at least one class 
  int *VirtualOveralInputChannelFlag;

  // VirtualOveralInputChannelNum defines the number of real input channels analyzed by all tissue classes together 
  // if VirtualOveralInputChannelNum < NumInputChannel => at least one input channel is not used by any tissue class

  int VirtualOveralInputChannelNum;


  FILE *LabelMapMFADifferenceFile;  
  FILE *WeightsMFADifferenceFile;   
                    
  int PrintMFALabelMapConvergence;  
  int PrintMFAWeightsConvergence ;  
                    
  int NumRegIter;                   
  int StopMFAType;                  
                    
  short *CurrentMFALabelMap;        
  float **CurrentMFAWeights;        
  
  double ***MRFParams;

  // This variable is set so that we can exclude classed from the incomplete E-Step 
  int *ExcludeFromIncompleteEStepFlag;

  // -----------------------------------------------------------
  // Interface between different functions 
  // -----------------------------------------------------------
  EMLocal_Hierarchical_Class_Parameters HierarchicalParameters;

  // -----------------------------------------------------------
  // Variables defined for Bias
  // -----------------------------------------------------------
  int StopBiasCalculation;
  int BiasPrint;     

  // -----------------------------------------------------------
  // Print Converegence measures  
  // -----------------------------------------------------------
  FILE **QualityFile;
  int *QualityFlagList;

  int PrintEMLabelMapConvergence;
  int PrintEMWeightsConvergence; 
  int StopEMType;                
  float StopEMValue;             

  int   LabelMapEMDifferenceAbsolut; 
  float LabelMapEMDifferencePercent;   
  short *CurrentEMLabelMap;         
  FILE *LabelMapEMDifferenceFile;   
                   
  float WeightsEMDifferenceAbsolut;    
  float WeightsEMDifferencePercent;    
  float **CurrentEMWeights;         
  FILE *WeightsEMDifferenceFile;    
                   
  int   NumROIVoxels;               
  int   EMStopFlag;                   

  int  PrintIntermediateFlag;
 
  // -----------------------------------------------------------
  //    Variables Neded for Shape Modelling
  // -----------------------------------------------------------
  float **PCAShapeParameters;               
  int    PCATotalNumOfShapeParameters;      
  double **PCAParameters;                   
                       
  float  **PCAInverseEigenValues;           
  double **PCAEigenValues;                  
                        
  int    *PCAMeanShapeIncY;                 
  int    *PCAMeanShapeIncZ;                 
  float  **PCAMeanShapePtrStart;            
                       
  int    **PCAEigenVectorsIncY;             
  int    **PCAEigenVectorsIncZ;  

  float  ***PCAEigenVectorsPtrStart;
                   
  float *PCALogisticSlope;          
  float *PCALogisticBoundary;       
  float *PCALogisticMax;     
  float *PCALogisticMin; 
  
  unsigned char *PCA_ROI_Start;

  int PCA_ROIExactVoxelCount;

  FILE **PCAFile; 

  int* PCANumberOfEigenModes;
  // Minimum index which has to be considered by PCA analysis;
  int PCAMin[3]; 
  int PCAMax[3];

  int PCAShapeModelType;
  EMLocalShapeCostFunction *ShapeParameters;
  
  // -----------------------------------------------------------
  // Registration Variables 
  // -----------------------------------------------------------

  // Should be defined later in EM-Varaible Section but needed for CostFunctionParameters
  unsigned char *OutputVectorPtr; 
  short *ROIPtr;
  float **InputVectorPtr;

  float   *SuperClassToAtlasRotationMatrix;    
  float   *SuperClassToAtlasTranslationVector; 
                          
  float   **ClassToAtlasRotationMatrix;        
  float   **ClassToAtlasTranslationVector;     
                          
  float   **ClassToSuperClassRotationMatrix;   
  float   **ClassToSuperClassTranslationVector;

  float    *GlobalRegInvTranslation;
  float    *GlobalRegInvRotation;

  // Should each sub class be considered independent in the registration cost functon of not 
  int     *RegistrationIndependentSubClassFlag;     
  //Should we register class when we do class specific registration
  int     *RegistrationClassSpecificRegistrationFlag; 
  double  **RegistrationTranslation;  
  double  **RegistrationRotation;     
  double  **RegistrationScale;        
  FILE    **RegistrationParameterFile; 

  // We assume atlas and source have same dimension in terms of voxel size and number 
  // also we start at voxel with number 0 not 1 !
  float targetmidcol;  
  float targetmidrow;  
  float targetmidslice;

  int TwoDFlag; 
  int RigidFlag; 
  
  EMLocalRegistrationCostFunction_ROI Registration_ROI_Weight;
  EMLocalRegistrationCostFunction_ROI Registration_ROI_ProbData;
  EMLocalRegistrationCostFunction* RegistrationParameters;



  // -----------------------------------------------------------
  // Variables Needed for MultiThreading 
  // -----------------------------------------------------------
  float **w_mPtr;
  // Needed for threading
  float **w_mCopy; 
  float **w_m_inputPtr;
  float **w_m_outputPtr;
  

  // MultiThreading of E-Step 
  vtkMultiThreader *E_Step_Threader;
  EMLocalAlgorithm_E_Step_MultiThreaded_Parameters *E_Step_Threader_Parameters;
  EMLocalAlgorithm_E_Step_MultiThreaded_SelfPointer E_Step_Threader_SelfPointer;
  int E_Step_Threader_Number;
};

#include "EMLocalAlgorithm.txx"

#endif
 
