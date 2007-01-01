/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMAtlasSuperClass.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMAtlasSuperClass
#ifndef __vtkImageEMAtlasSuperClass_h
#define __vtkImageEMAtlasSuperClass_h 
  
#include "vtkEMSegment.h"

#include "vtkImageEMAtlasClass.h"
//!!!bcd!!!#include "vtkSlicer.h"

class VTK_EMSEGMENT_EXPORT vtkImageEMAtlasSuperClass : public vtkImageEMGenericClass
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMAtlasSuperClass *New();
  vtkTypeMacro(vtkImageEMAtlasSuperClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  int           GetNumClasses() {return this->NumClasses;}

  // Description:
  // Defines the tissue class interaction matrix MrfParams(k,i,j)
  // where k = direction [1...6], 
  //       i = class of the current pixel t+1 
  //       j = class of neighbouring pixel t 
  void          SetMarkovMatrix(double value, int k, int j, int i);

  int           GetTotalNumberOfProbDataPtr();

  // Has any subclass ProbDataPtr defined or not 
  int           GetProbDataPtrFlag();

  int           GetProbDataPtr(void **PointerList,int index); 
  int           GetProbDataIncYandZ(int* ProbDataIncY,int* ProbDataIncZ,int index);

  void          SetProbDataWeight(float value) {this->ProbDataWeight = value;}
  float         GetProbDataWeight(){return this->ProbDataWeight;} 

  int           GetTotalNumberOfClasses(bool flag); // if flag is set => includes subclasses of type SUPERCLASS
  int           GetAllLabels(short *LabelList, int result,int Max); // Gets all labels from the Substructures
  int           LabelAllSuperClasses(short *TakenLabelList, int Result, int Max);

  //BTX
  void**        GetClassList() {return this->ClassList;}
  void*         GetClassListEntry(int i) {return this->ClassList[i];}
  
  classType*    GetClassListType() {return this->ClassListType;}
  classType     GetClassType(void* active);
  //ETX

  vtkImageEMAtlasSuperClass* GetParentClass() {return this->ParentClass;}

  int           GetTissueDefinition(int *LabelList, double** LogMu, double ***InvLogCov, int index);

  double        GetMrfParams(int k,int j,int i) {return this->MrfParams[k][j][i];}
  //BTX
  double***     GetMrfParams() {return this->MrfParams;}
  //ETX

  void AddSubClass(vtkImageEMAtlasClass* ClassData, int index) {this->AddSubClass((void*) ClassData, CLASS, index); }
  void AddSubClass(vtkImageEMAtlasSuperClass* ClassData, int index) {ClassData->ParentClass = this; this->AddSubClass((void*) ClassData, SUPERCLASS, index);}
  

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
  // Kilian: Jan06: InitialBias_FilePrefix allows initializing a bias field with a precomputed one 
  // - carefull Bias Field has to be in little Endian   
  vtkSetStringMacro(InitialBiasFilePrefix);
  vtkGetStringMacro(InitialBiasFilePrefix);

  // Description:
  // Kilian: Jan06: This allows you to "jump" over the hirarchical segmentation level by providing an already existing 
  // labelmap of the region of interes 
  vtkGetStringMacro(PredefinedLabelMapPrefix); 
  vtkSetStringMacro(PredefinedLabelMapPrefix); 


  // Description:  
  // Kilian: Jan 06: We cano now define different iteration sequnces at differnt hierarchies
  // Number of maximum iterations - similar to NumIter
  vtkGetMacro(StopEMMaxIter,int);      
  vtkSetMacro(StopEMMaxIter,int);
 
  // Description:  
  // What is the obundary value, note if the number of iterations 
  // extend MFAiter than stops than - similar to RegIter 
  vtkGetMacro(StopMFAMaxIter,int);      
  vtkSetMacro(StopMFAMaxIter,int);      

  vtkImageEMAtlasSuperClass() {this->CreateVariables();}
  ~vtkImageEMAtlasSuperClass() {this->DeleteSuperClassVariables();}

protected:
  vtkImageEMAtlasSuperClass(const vtkImageEMAtlasSuperClass&) {};
  void DeleteSuperClassVariables();
  void CreateVariables();

  void operator=(const vtkImageEMAtlasSuperClass&) {};
  void ExecuteData(vtkDataObject *);   

  void AddSubClass(void* ClassData, classType initType, int index);

  int           NumClasses;             // Number of Sub classes -> Importan for dim of Class List
  void **       ClassList;              // List of Sub Classes
  classType*    ClassListType;
  vtkImageEMAtlasSuperClass* ParentClass;    // The parent of this super class if ParentClass == NULL => does not have a parent 
  double***     MrfParams;              // Markov Model Parameters: Matrix3D mrfparams(this->NumClasses,this->NumClasses,4);

  int PrintFrequency;    // Print out the result after how many steps  (-1 == just last result, 0 = No Printing, i> 0 => every i-th slice )
  int PrintBias;         // Should the bias be printed too (Only works for GE)
  int PrintLabelMap;     // Print out inbetween label map   
  char* InitialBiasFilePrefix;     // Initialize Bias field with outside source 
  char* PredefinedLabelMapPrefix;  // This allows you to "jump" over the hirarchical segmentation level by providing an already existing 
                                   // labelmap of the region of interest 

  int StopEMMaxIter; // Maximum number of iterations  if StopEMValue is not  is not reached 
  int  StopMFAMaxIter;   // Maximum number of iterations by the MFA if StopEMValue is not reached 

};
#endif











