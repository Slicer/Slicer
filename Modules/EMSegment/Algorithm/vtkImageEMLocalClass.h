/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMLocalClass.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMLocalClass
#ifndef __vtkImageEMLocalClass_h
#define __vtkImageEMLocalClass_h 
  
#include "vtkEMSegment.h"

#include "vtkImageEMLocalGenericClass.h"
#include "assert.h"

#define EMSEGMENT_NUM_OF_QUALITY_MEASURE 1

class VTK_EMSEGMENT_EXPORT vtkImageEMLocalClass : public vtkImageEMLocalGenericClass
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMLocalClass *New();
  vtkTypeMacro(vtkImageEMLocalClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // ----------------------------------------- 
  // Image Data input 

  // Description:
  // Reference standard to define the performance of the algorithm, e.g. output is compared to the Dice Measure 
  void SetReferenceStandard(vtkImageData *image) {this->ReferenceStandardPtr = image;}
  //BTX
  vtkImageData* GetReferenceStandard() {return this->ReferenceStandardPtr;}
  //ETX

  // Description:
  // The Eigen Vectors of the PCA analysis
  void SetPCAEigenVector(vtkImageData *image, int index);

  //BTX
  // Description:
  // SegmentationBoundary condition are taken into account (Type == 1) or not (Type == 0);
  void* GetPCAEigenVectorPtr(int index, int type) { assert(this->PCAEigenVectorImageData); return this->GetDataPtr(this->PCAEigenVectorImageData[index],type); }

  // Description:
  // Increments for PCAEigenVector in Y and Z Dimension 
  int   GetPCAEigenVectorIncY(int index, int type) { return this->GetImageDataInc(this->PCAEigenVectorImageData[index],type,0);}
  int   GetPCAEigenVectorIncZ(int index, int type) { return this->GetImageDataInc(this->PCAEigenVectorImageData[index],type,1);}

  //ETX  

  // Description:
  // Probability Data defining the spatial conditional label distribution
  void   SetPCAMeanShape(vtkImageData *image) {this->SetInput(2,image);}

  //BTX
  void* GetPCAMeanShapePtr(int Type) { return this->GetDataPtr(this->PCAMeanShapeImageData,Type);}
  //ETX

  // Description:
  // Increments for PCAMeanShape in Y and Z Dimension, where 
  // SegmentationBoundary condition are taken into account (Type == 1) or not (Type == 0);
  int GetPCAMeanShapeIncY(int Type) {return this->GetImageDataInc(this->PCAMeanShapeImageData, Type,0);}
  int GetPCAMeanShapeIncZ(int Type) {return this->GetImageDataInc(this->PCAMeanShapeImageData, Type,1);}

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
  // PCA Stuff
  void     SetPCAEigenValues(int index , double value) { this->PCAEigenValues[index] = value; }
  //BTX
  double   *GetPCAEigenValues() {return this->PCAEigenValues;}
  //ETX

  void     SetPCANumberOfEigenModes(int init);
  int      GetPCANumberOfEigenModes() {return this->PCANumberOfEigenModes;}

  
  void     SetPCAShapeParameters(float* init) {memcpy(this->PCAShapeParameters,init, sizeof(float)*this->PCANumberOfEigenModes);} 
  //BTX
  float*  GetPCAShapeParameters() {return this->PCAShapeParameters;} 
  //ETX
  void     PrintPCAParameters(ostream& os,vtkIndent indent);
  
  // -------------------------------------------------------
  // Print Functions
 
  // Description:
  // Currenly only the following values defined 
  // 0 = Do not Print out any print quality 
  // 1 = Do a DICE comparison
  void SetPrintQuality(int init);

  vtkGetMacro(PrintQuality,int);
  
  vtkSetMacro(PrintPCA,int);
  vtkGetMacro(PrintPCA,int);
  
  
  // Check and Assign ImageData to the different parameters 
  int CheckAndAssignPCAImageData(vtkImageData *inData, int ImageIndex);

  // Description: 
  // From which value should we assign the probability 1 to it 
  vtkSetMacro(PCALogisticMax,float) ;     
  vtkGetMacro(PCALogisticMax,float) ;  

  // Description: 
  // From which distanc should we assign the probability 0 (e.g. 5)
  vtkSetMacro(PCALogisticMin,float) ;     
  vtkGetMacro(PCALogisticMin,float) ;     

  // Description:
  // Which value should 0.5 be assigned to 
  vtkSetMacro(PCALogisticBoundary,float);
  vtkGetMacro(PCALogisticBoundary,float);
 
  // Description:
  // What is the stepness of the slope 
  vtkSetMacro(PCALogisticSlope,float);
  vtkGetMacro(PCALogisticSlope,float);

protected:
  vtkImageEMLocalClass();
  ~vtkImageEMLocalClass() {this->DeleteClassVariables();}
  vtkImageEMLocalClass(const vtkImageEMLocalClass&);
  void operator=(const vtkImageEMLocalClass&);
  void DeleteClassVariables();
  void DeletePCAParameters(); 

  void ThreadedExecute(vtkImageData **inData, vtkImageData *outData,int outExt[6], int id) {};
  void ExecuteData(vtkDataObject *);   

  double *LogMu;                 // Intensity distribution of the classes (changed for Multi Dim Version)
  double **LogCovariance;        // Intensity distribution of the classes (changed for Multi Dim Version) -> This is the Coveriance Matrix
                                 // Be careful: the Matrix must be symmetric and positiv definite,
  vtkImageData*  ReferenceStandardPtr;  // Reference Standard for measuring the performance of the algorithm
   // -------------------------------------------------------
   // PCA Stuff 
   float *PCAShapeParameters;    // Defines the shape parameter for a model. If all parameters are 0 => MeanShape
   int    PCANumberOfEigenModes;  // Number of Eigenmodes used for shape model -> by the way can be different from NumberOfEigenModes defined by PCAShapePtr

   vtkImageData **PCAEigenVectorImageData;
   vtkImageData *PCAMeanShapeImageData;

   double *PCAEigenValues;

   int PrintQuality;        // Prints out a quality measure of the current result ( 1=  Dice )
   int PrintPCA;            // Print out PCA Parameters at each step 
  
   float  PCALogisticMax;     // From which value should we assign the probability 1 to it 
   float  PCALogisticMin;     // From which value should we assign the probability 0 
   float  PCALogisticBoundary; // Which value should 0.5 be assigned to 
   float  PCALogisticSlope;    // Ehat is the stepness of the slope 
};
#endif











