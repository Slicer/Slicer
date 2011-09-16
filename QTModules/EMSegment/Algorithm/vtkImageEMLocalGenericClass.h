/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMLocalGenericClass.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMLocalGenericClass
// Definition of all variables owned by both vtkImageEMSuperClass and vtkImageEMClass
//
//                                    vtkImageEMGenericClass
//                                    /              \      \ ___________________________
//                                   /                \                                  \ .
//            vtkImageEMLocalGenericClass   vtkImageEMLocalSuperClass <- - - vtkImageEMLocalClass
//                        /                    \.
//             vtkImageEMLocalSuperClass <- - - vtkImageEMLocalClass

#ifndef __vtkImageEMLocalGenericClass_h
#define __vtkImageEMLocalGenericClass_h 
  
#include "vtkEMSegment.h"
#include "vtkImageEMGenericClass.h"
#include "vtkImageData.h"
#include "vtkOStreamWrapper.h"

// Do you want to run the code with all the print outs (set 1) or not (0)
#define EMVERBOSE 0

class VTK_EMSEGMENT_EXPORT vtkImageEMLocalGenericClass : public vtkImageEMGenericClass
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMLocalGenericClass *New();
  vtkTypeMacro(vtkImageEMLocalGenericClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // ----------------------------------------- 
  // Image Data input 

  void* GetDataPtr(vtkImageData* ImageData, int BoundaryType);  // Returns the data pointer  where the SegmentationBoundary condition are taken 
                                                                // into account (BoundaryType == 1) or not (BoundaryType == 0);;

  // Check if the input image data is consistence with what we expect
  int CheckInputImage(vtkImageData * inData,int DataTypeOrig, int num);

  int CheckAndAssignProbImageData(vtkImageData *inData); 

  int GetImageDataInc(vtkImageData* ImageData, int BoundaryType, int IncType); // If IncType = 0 => Returns DataIncY (BoundaryType = 0) or BoundaryDataIncY (BoundaryType = 1)
                                                                // If IncType = 1 => Returns DataIncZ (BoundaryType = 0) or BoundaryDataIncZ (BoundaryType = 1)

  // Description:
  // Probability Data defining the spatial conditional label distribution
  void SetProbDataPtr(vtkImageData *image) {this->SetInput(1,image);}
  //BTX
  // Description:
  // SegmentationBoundary condition are taken into account (Type == 1) or not (Type == 0);
  void* GetProbDataPtr(int Type) { return this->GetDataPtr(this->ProbImageData,Type);}
  //ETX

  // Description:
  // Increments for probability data in Y and Z Dimension, where 
  // SegmentationBoundary condition are taken into account (Type == 1) or not (Type == 0);
  int GetProbDataIncY(int Type) {return this->GetImageDataInc(this->ProbImageData, Type,0);}
  int GetProbDataIncZ(int Type) {return this->GetImageDataInc(this->ProbImageData, Type,1);}

  // ----------------------------------------- 
  // Registration Data 

 
  // Description:  
  // Translation from patient case to atlas space   
  vtkGetVector3Macro(RegistrationTranslation, double);
  double GetRegistrationTranslation(int index) {return this->RegistrationTranslation[index];}
  vtkSetVector3Macro(RegistrationTranslation, double);

  // Description:
  // Rotation from patient case to atlas space   
  vtkGetVector3Macro(RegistrationRotation, double);
  double GetRegistrationRotation(int index) {return this->RegistrationRotation[index];}
  vtkSetVector3Macro(RegistrationRotation, double);

  // Description:
  // Scale from patient case to atlas space   
  vtkGetVector3Macro(RegistrationScale, double);
  double GetRegistrationScale(int index) {return this->RegistrationScale[index];}
  vtkSetVector3Macro(RegistrationScale, double);

  // Description:
  // Diagonal Covariance Matrix (describing the zero Mean Gaussian distribution of the class registration parameters 
  vtkGetVectorMacro(RegistrationInvCovariance,double,9); 
  vtkSetVectorMacro(RegistrationInvCovariance,double,9);

  // Description:
  // If the class specific registration is activated by the superclass should this structure be optimizaed or ignored !
  // By default it is ignored (set to 0)
  vtkGetMacro(RegistrationClassSpecificRegistrationFlag,int); 
  vtkSetMacro(RegistrationClassSpecificRegistrationFlag,int); 
  vtkBooleanMacro(RegistrationClassSpecificRegistrationFlag,int); 

 // Description:
 // Prints out the registration parameters translation - rotation -scaling 
  vtkGetMacro(PrintRegistrationParameters, int);
  vtkSetMacro(PrintRegistrationParameters, int);

  // Description:
  // Prints out the registration cost at each voxel 
  vtkGetMacro(PrintRegistrationSimularityMeasure, int);
  vtkSetMacro(PrintRegistrationSimularityMeasure, int);
  

  // Description:
  // When the atlas does not match the segmentation environment all structures might have the weight zero 
  // To cope with this problem, the E-Step than assigns the wieghts just looking at the intensities. This however might produce 
  // unwanted outliers -> Set this flag if it is important that a class does not have these outliers
  vtkGetMacro(ExcludeFromIncompleteEStepFlag,int); 
  vtkSetMacro(ExcludeFromIncompleteEStepFlag,int); 
  vtkBooleanMacro(ExcludeFromIncompleteEStepFlag,int); 


  void SetRegistrationCovariance(double Init[9]); 
  void SetRegistrationCovariance(double CovTran1, double CovTran2, double CovTran3, double CovRot1, double CovRot2, double CovRot3, double CovSca1, double CovSca2, double CovSca3) {
    double temp[9] = {CovTran1, CovTran2, CovTran3, CovRot1, CovRot2, CovRot3, CovSca1, CovSca2, CovSca3};return this-> SetRegistrationCovariance(temp); }

   // Description:
  // Saves the weights/posterior computed by EM so that one can use them for further processing 
  vtkGetObjectMacro(PosteriorImageData, vtkImageData);
  vtkSetObjectMacro(PosteriorImageData, vtkImageData);

protected:
  vtkImageEMLocalGenericClass();
  ~vtkImageEMLocalGenericClass(); 

  vtkImageEMLocalGenericClass(const vtkImageEMLocalGenericClass&);
  void operator=(const vtkImageEMLocalGenericClass&);
 
  void  ExecuteData(vtkDataObject *) ;

  int Extent[6];                 // Extent of input 
  vtkImageData *ProbImageData;   // Pointer to the vtkImageData defining ProbData 


  // Thes are the parameters describing the transition from 
  // (Very First SuperClass - Head) From Atlas to Global Coordinate system 
  // (Next class of super class)  From Global to SuperClass Coordinate System
  // (For CLASS) For Class to SuperClass Coordinate System 
   
  double RegistrationTranslation[3];
  double RegistrationRotation[3];
  double RegistrationScale[3];
  double RegistrationInvCovariance[9];
  int RegistrationClassSpecificRegistrationFlag; 

  int ExcludeFromIncompleteEStepFlag;
  int PrintRegistrationParameters;
  int PrintRegistrationSimularityMeasure;

private:
  vtkImageData*  PosteriorImageData;
};

#endif











