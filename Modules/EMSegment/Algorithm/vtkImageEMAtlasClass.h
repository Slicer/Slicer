/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMAtlasClass.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMAtlasClass
#ifndef __vtkImageEMAtlasClass_h
#define __vtkImageEMAtlasClass_h 
  
#include "vtkEMSegment.h"

#include "vtkImageEMGenericClass.h"
//!!!bcd!!!#include "vtkSlicer.h"

class VTK_EMSEGMENT_EXPORT vtkImageEMAtlasClass : public vtkImageEMGenericClass
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMAtlasClass *New();
  vtkTypeMacro(vtkImageEMAtlasClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // ----------------------------------------- 
  // Image Data input 

  // Description:
  // Probability Data defining the spatial conditional label distribution
  void SetProbDataPtr(vtkImageData *image) {this->SetInput(1,image);}
  //BTX
  void SetProbDataPtr(void* initptr) {this->ProbDataPtr = initptr;}
  void* GetProbDataPtr() {return this->ProbDataPtr;}
  //ETX

  // Description:
  // Increments for probability data in Y and Z Dimension 
  vtkGetMacro(ProbDataIncY,int);
  vtkGetMacro(ProbDataIncZ,int);

  // Description:
  // Reference standard to define the performance of the algorithm, e.g. output is compared to the Dice Measure 
  void SetReferenceStandard(vtkImageData *image) {this->ReferenceStandardPtr = image;}
  //BTX
  vtkImageData* GetReferenceStandard() {return this->ReferenceStandardPtr;}
  //ETX

  // --------------------------------------------
  // Normal Data Input

  // Description:
  // Set label for the class
  vtkSetMacro(Label,short);

  // Description:
  // Set the Number of Input Images for subclasses
  void SetNumInputImages(int number); 

   void     SetLogCovariance(double value, int y, int x);
  //BTX
   double** GetLogCovariance() {return this->LogCovariance;}
   double*  GetLogCovariance(int i) {return this->LogCovariance[i];}
  //ETX

   void     SetLogMu(double mu, int x); 
   //BTX
   double*  GetLogMu(){ return this->LogMu;}
  //ETX

  // -------------------------------------------------------
  // Print Functions
 
  // Description:
  // Currenly only the following values defined 
  // 0 = Do not Print out any print quality 
  // 1 = Do a DICE comparison
  void SetPrintQuality(int init);

  vtkGetMacro(PrintQuality,int);
  
  // Check if the input image data is consistence with what we expect
  int CheckInputImage(vtkImageData * inData,int DataTypeOrig, int num, int outExt[6]);
  // Check and Assign ImageData to the different parameters 
  int CheckAndAssignImageData(vtkImageData *inData, int outExt[6]);

protected:
  vtkImageEMAtlasClass();
  vtkImageEMAtlasClass(const vtkImageEMAtlasClass&) {};
  ~vtkImageEMAtlasClass() {this->DeleteClassVariables();}
  void DeleteClassVariables();

  void operator=(const vtkImageEMAtlasClass&) {};
  void ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id) {};
  void ExecuteData(vtkDataObject *);   

  double *LogMu;                 // Intensity distribution of the classes (changed for Multi Dim Version)
  double **LogCovariance;        // Intensity distribution of the classes (changed for Multi Dim Version) -> This is the Coveriance Matrix
                                 // Be careful: the Matrix must be symmetric and positiv definite,
  void   *ProbDataPtr;           // Pointer to Probability Data 
  int    ProbDataIncY;           // Increments for probability data in Y Dimension 
  int    ProbDataIncZ;           // Increments for probability data in Z Dimension

  vtkImageData*  ReferenceStandardPtr;  // Reference Standard for measuring the performance of the algorithm

  int PrintQuality;        // Prints out a quality measure of the current result ( 1=  Dice )
};
#endif











