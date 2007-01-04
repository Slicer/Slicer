/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMLocalSuperClass.h,v $
  Date:      $Date: 2006/12/08 23:28:23 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMLocalSuperClass
#ifndef __vtkImageEMLocalSuperClass_h
#define __vtkImageEMLocalSuperClass_h 
  
#include "vtkEMSegment.h"

#include "vtkImageEMLocalClass.h"
#include "EMLocalInterface.h" 

class VTK_EMSEGMENT_EXPORT vtkImageEMLocalSuperClass : public vtkImageEMLocalGenericClass
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMLocalSuperClass *New();
  vtkTypeMacro(vtkImageEMLocalSuperClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  int           GetNumClasses() {return this->NumClasses;}

  // Description:
  // Defines the tissue class interaction matrix MrfParams(k,i,j)
  // where k = direction [1...6], 
  //       i = class of the current pixel t+1 
  //       j = class of neighbouring pixel t 
  void          SetMarkovMatrix(double value, int k, int j, int i);

  // Has any subclass PCA*Ptr defined or not 
  int           GetPCAPtrFlag();
 
  // Returns the list with the PCAShape pointer 
  int           GetPCAParametersPtr(void** PCAMeanShapePtr, void*** PCAEigenVectorPtr, int index, int BoundaryType);
  int           GetPCAParameters(float **ShapeParametersList, int *PCAMeanShapeIncY, int *PCAMeanShapeIncZ, 
                                 int **PCAEigenVectorIncY, int **PCAEigenVectorIncZ, double **PCAEigenValues, 
                                 float *PCALogisticSlope, float *PCALogisticBoundary, float *PCALogisticMin, float *PCALogisticMax, int index, int BoundaryType);

  int           GetTotalNumberOfEigenModes();
  int           GetTotalNumberOfProbDataPtr();
  void          GetPCANumberOfEigenModes(int *NumberOfEigenModesList);

  // Has any subclass ProbDataPtr defined or not 
  int           GetProbDataPtrFlag();
  // Which classes should be considered for the normalization of the spatial priors 
  int           GetProbImageDataCount(char *list, int index);

  int           GetProbDataPtrList(void **PointerList,int index, int BoundaryType); 
  int           GetProbDataIncYandZ(int* ProbDataIncY,int* ProbDataIncZ,int index,int BoundaryType);

  void          SetProbDataWeight(float value) {this->ProbDataWeight = value;}
  float         GetProbDataWeight(){return this->ProbDataWeight;} 

  int           GetTotalNumberOfClasses(bool flag); // if flag is set => includes subclasses of type SUPERCLASS
  int           GetAllLabels(short *LabelList, int result,int Max); // Gets all labels from the Substructures
  int          LabelAllSuperClasses(short *TakenLabelList, int Result, int Max);

  //BTX
  void**        GetClassList() {return this->ClassList;}
  void*         GetClassListEntry(int i) {return this->ClassList[i];}
  
  classType*    GetClassListType() {return this->ClassListType;}
  classType     GetClassType(void* active);
  //ETX

  vtkImageEMLocalSuperClass* GetParentClass() {return this->ParentClass;}

  int           GetTissueDefinition(int *LabelList, double** LogMu, double ***InvLogCov, int index);

  double        GetMrfParams(int k,int j,int i) {return this->MrfParams[k][j][i];}
  //BTX
  double***     GetMrfParams() {return this->MrfParams;}
  //ETX

  void AddSubClass(vtkImageEMLocalClass* ClassData, int index) {this->AddSubClass((void*) ClassData, CLASS, index); }
  void AddSubClass(vtkImageEMLocalSuperClass* ClassData, int index) {
    ClassData->ParentClass = this; this->AddSubClass((void*) ClassData, SUPERCLASS, index);}
  

  // Print Functions 
  // Description:
  // Print out the result after how many steps  (-1 == just last result, 0 = No Printing, i> 0 => every i-th slice )
  vtkGetMacro(PrintFrequency, int);
  vtkSetMacro(PrintFrequency, int);

  vtkGetMacro(PrintBias, int);
  vtkSetMacro(PrintBias, int);

  vtkGetMacro(PrintLabelMap, int);
  vtkSetMacro(PrintLabelMap, int);  


  // Description:
  // Prints out the number of voxels changed from last to this EM iteration
  vtkGetMacro(PrintEMLabelMapConvergence, int);  
  vtkSetMacro(PrintEMLabelMapConvergence, int);  

  // Description:
  // Prints out the difference in percent 
  vtkGetMacro(PrintEMWeightsConvergence, int);
  vtkSetMacro(PrintEMWeightsConvergence, int);

  // Description:
  // Prints out the number of voxels changed from last to this MFA iteration
  vtkGetMacro(PrintMFALabelMapConvergence, int);  
  vtkSetMacro(PrintMFALabelMapConvergence, int);  

  // Description:
  // Prints out the difference in percent 
  vtkGetMacro(PrintMFAWeightsConvergence, int);
  vtkSetMacro(PrintMFAWeightsConvergence, int);

  // Description:
  // Prints out the shape cost at each voxel 
  vtkGetMacro(PrintShapeSimularityMeasure, int);
  vtkSetMacro(PrintShapeSimularityMeasure, int);


  // Description:  
  // After which criteria should be stopped   
  // 0 = Fixed iterations (defined by  StopEMMaxIter)
  // 1 = Labelmap measure  (Calculate difference measure by comparing Labelmaps between iterations)  
  // 2 = Weight measure (Calculate difference measure by comparing Weights between iterations)  
  vtkGetMacro(StopEMType,int); 
  vtkSetMacro(StopEMType,int); 
  void SetStopEMTypeToFixed()    {this->StopEMType = EMSEGMENT_STOP_FIXED;}
  void SetStopEMTypeToLabelmap() {this->StopEMType = EMSEGMENT_STOP_LABELMAP;}
  void SetStopEMTypeToWeights()  {this->StopEMType = EMSEGMENT_STOP_WEIGHTS;}
  
  // Description:  
  // What is the value at which the EM iteration should stop in percent (DifferenceMeasure/ Number of Voxels), (so 1% => set StopEMValue to 0.01)
  // Note if the number of iterations extend StopEMMaxIter than stops than
  vtkGetMacro(StopEMValue,float);      
  vtkSetMacro(StopEMValue,float);      

  // Description:  
  // Number of maximum iterations 
  vtkGetMacro(StopEMMaxIter,int);      
  vtkSetMacro(StopEMMaxIter,int);      


  // Description:
  // You can stop the bias calculation after a certain number of iterations
  // By default it is set to -1 which means it never stops
  vtkGetMacro(StopBiasCalculation,int); 
  vtkSetMacro(StopBiasCalculation,int); 

  // Description:  
  // After which criteria should be stopped   
  // 0 = fixed iterations 
  // 1 = Absolut measure 
  // 2 = Relative measure 
  vtkGetMacro(StopMFAType,int); 
  vtkSetMacro(StopMFAType,int); 
  void SetStopMFATypeToFixed()    {this->StopEMType = EMSEGMENT_STOP_FIXED;}
  void SetStopMFATypeToLabelmap() {this->StopEMType = EMSEGMENT_STOP_LABELMAP;}
  void SetStopMFATypeToWeights()  {this->StopEMType = EMSEGMENT_STOP_WEIGHTS;}
  

  // Description:  
  // What is the obundary value, note if the number of iterations 
  // extend MFAiter than stops than
  vtkGetMacro(StopMFAValue,float);      
  vtkSetMacro(StopMFAValue,float);      

  // Description:  
  // Number of maximum iterations 
  vtkGetMacro(StopMFAMaxIter,int);      
  vtkSetMacro(StopMFAMaxIter,int);      

  // Description:  
  // What type of registration is wanted 
  // 0 = No registration
  // 1 = Apply transformation but do not reiterate for better parameters 
  // 2 = Only do global registration 
  // 3 = Do structure specific registration
  // 4 = Do both structure  specifc and global registration simultaneously
  // 5 = Do both structure  specifc and global registration sequentially
  vtkGetMacro(RegistrationType,int);      
  vtkSetMacro(RegistrationType,int);  
  void SetRegistrationTypeToDisabled()       {this->RegistrationType = EMSEGMENT_REGISTRATION_DISABLED;} 
  void SetRegistrationTypeToApply()          {this->RegistrationType = EMSEGMENT_REGISTRATION_APPLY;} 
  void SetRegistrationTypeToGlobalOnly()     {this->RegistrationType = EMSEGMENT_REGISTRATION_GLOBAL_ONLY;} 
  void SetRegistrationTypeToClassOnly()      {this->RegistrationType = EMSEGMENT_REGISTRATION_CLASS_ONLY;} 
  void SetRegistrationTypeToSimultaneous()   {this->RegistrationType = EMSEGMENT_REGISTRATION_SIMULTANEOUS;} 
  void SetRegistrationTypeToSequential()     {this->RegistrationType = EMSEGMENT_REGISTRATION_SEQUENTIAL;} 

  // Desciption:
  // If the flag is defined the spatial distribution of the first class will be automatically generated. 
  // In specifics the spatial distribution at voxel x is defined as 
  // spd(x) = NumberOfTrainingSamples - sum_{all other srructures dependent on the supercals} spd_struct(x) 
  vtkGetMacro(GenerateBackgroundProbability,int);      
  vtkSetMacro(GenerateBackgroundProbability,int);      
  vtkBooleanMacro(GenerateBackgroundProbability,int);


  // Desciption:
  // This flag is for the registration cost function. By default all subclasses are seen as one. 
  // In some cases this causes a loss of contrast within the cost function so that the registration is not as reliable, 
  // e.g. when we define two SuperClasses (FG and BG) which are defined as outside the brain as BG and everything inside the brain as FG, 
  // than we cannot use the ventricles wont be used for the alignment. Hoewever in many cases this structure drives the registration soley so that 
  /// our method is not as rebust. For this specific case we would set the flag for FG and do not set it for BG !
  vtkGetMacro(RegistrationIndependentSubClassFlag,int);      
  vtkSetMacro(RegistrationIndependentSubClassFlag,int);      
  vtkBooleanMacro(RegistrationIndependentSubClassFlag,int);

  // Description:
  // Setting of the immidiate subclasses
  
  void GetRegistrationClassSpecificParameterList(int *RegistrationIndependentSubClassFlag, int *RegistrationClassSpecificRegistrationFlag, int &NumParaSets);

  // Description:
  // If all structures are defined by different PCA models that this flag has to be set.
  // Kilian: Currently this is not fully integrated into hierarchical framework
  // in other words all substructers must all be independently modelled or not 
  // If set to zero than all structures are defined by the same PCA model
  vtkGetMacro(PCAShapeModelType,int); 
  vtkSetMacro(PCAShapeModelType,int); 
  // Each structure has its own PCA model defined 
  void SetPCAShapeModelTypeToClassIndependent() {this->PCAShapeModelType = EMSEGMENT_PCASHAPE_INDEPENDENT;} 
  // The PCA Model is generated over all structures together
  void SetPCAShapeModelTypeToClassDependent() {this->PCAShapeModelType = EMSEGMENT_PCASHAPE_DEPENDENT;} 
  // Do not maximize over the shape setting  - just use the current setting  
  void SetPCAShapeModelTypeToApply() {this->PCAShapeModelType = EMSEGMENT_PCASHAPE_APPLY;} 
 

protected:
  vtkImageEMLocalSuperClass() {this->CreateVariables();}
  ~vtkImageEMLocalSuperClass() {this->DeleteSuperClassVariables();}
  void DeleteSuperClassVariables();
  void CreateVariables();

  void ExecuteData(vtkDataObject *);   

  void AddSubClass(void* ClassData, classType initType, int index);
  int  GetPCANumberOfEigenModesList(int *NumberOfEigenModesList, int index);

  int           NumClasses;             // Number of Sub classes -> Importan for dim of Class List
  void **       ClassList;              // List of Sub Classes
  classType*    ClassListType;
  vtkImageEMLocalSuperClass* ParentClass;    // The parent of this super class if ParentClass == NULL => does not have a parent 
  double***     MrfParams;              // Markov Model Parameters: Matrix3D mrfparams(this->NumClasses,this->NumClasses,4);

  int PrintFrequency;    // Print out the result after how many steps  (-1 == just last result, 0 = No Printing, i> 0 => every i-th slice )
  int PrintBias;         // Should the bias be printed too (Only works for GE)
  int PrintLabelMap;     // Print out inbetween label map   

  int PrintEMLabelMapConvergence;  // Prints out the number of voxels changed from last to this iteration
  int PrintEMWeightsConvergence; // Prints out the difference in percent 

  int StopEMType;       // After which criteria should be stopped   
                        // 0 = fixed iterations 
                        // 1 = Absolut measure 
                        // 2 = Relative measure
  float StopEMValue;    // What is the obundary value, note if the number of iterations 
                        // extend EMiter than stops than
                        // if (StopEMType = 1) than it is percent

  int StopEMMaxIter; // Maximum number of iterations  if StopEMValue is not  is not reached 


  // Same as EM
  int PrintMFALabelMapConvergence;  
  int PrintMFAWeightsConvergence; 
  int   StopMFAType;      
  float StopMFAValue;     // What is the obundary value, note if the number of iterations 
                          // extend MFAiter than stops than
                          // if (StopMFAType = 1) than it is percent
  int   StopMFAMaxIter;   // Maximum number of iterations by the MFA if StopEMValue is not reached 

  int StopBiasCalculation;
  int RegistrationType; 

  int GenerateBackgroundProbability;
  
  int PrintShapeSimularityMeasure;

  int RegistrationIndependentSubClassFlag;

  // If all structures are defined by different PCA models that this flag has to be set. 
  int PCAShapeModelType; 
private:
  vtkImageEMLocalSuperClass(const vtkImageEMLocalSuperClass&);
  void operator=(const vtkImageEMLocalSuperClass&);
};
#endif











