/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMGenericClass.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMGenericClass
// Definnition of all variables owned by both vtkImageEMSuperClass and vtkImageEMClass
//                      vtkImageEMGenericClass
//                        //                 //  
//           vtkImageEMSuperClass <- - - vtkImageEMClass

#ifndef __vtkImageEMGenericClass_h
#define __vtkImageEMGenericClass_h 
  
#include "vtkEMSegment.h"
#include "vtkImageMultipleInputFilter.h"
#include "vtkImageEMGeneral.h"

// #ifndef EM_VTK_OLD_SETTINGS
// #if (VTK_MAJOR_VERSION == 4 && (VTK_MINOR_VERSION >= 3 || (VTK_MINOR_VERSION == 2 && VTK_BUILD_VERSION > 5)))
//   #define EM_VTK_OLD_SETTINGS 0 
// #else
//   #define EM_VTK_OLD_SETTINGS 1
// #endif
// #endif

class VTK_EMSEGMENT_EXPORT vtkImageEMGenericClass : public vtkImageMultipleInputFilter
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMGenericClass *New();
  vtkTypeMacro(vtkImageEMGenericClass,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Global Tissue Probability
  vtkSetMacro(TissueProbability, double);
  vtkGetMacro(TissueProbability, double);

  // Description:
  // How much influence should the LocalPriorData have in the segmentation process 
  // 0   = no Probability data is used => if class is from type EMClass no ProbDataPtr will be loaded
  // 1.0 = complete trust is in ProbDataPtr => Whenever the ProbDataPtr = 0 => voxel will not be assigned to tissue class
  vtkSetMacro(ProbDataWeight,float);
  vtkGetMacro(ProbDataWeight,float);

  // Description
  // Scalar type of ProbData 
  vtkGetMacro(ProbDataScalarType,int);

  // Description:
  // Labels of classes. Please not that label for super classes are set automatically
  vtkGetMacro(Label,short);

  // Description:
  // Important for several input parameters such as size of LogCovariance
  // How much weight is given to each channel  
  // 0   = input is ignored 
  // 1.0 = input is fully considered  
  void SetInputChannelWeights(float val, int x);
  //BTX
  float* GetInputChannelWeights() {return this->InputChannelWeights;}
  //ETX

  // Description:
  // Number of input images for the segmentation - Has to be defined before defining any class specific setting 
  // Otherwise they get deleted
  // Be carefull: this is just the number of images not attlases, 
  // e.g. I have 5 tissue classes and 3 Inputs (T1, T2, SPGR) -> NumInputImages = 3
  void SetNumInputImages(int number);
  vtkGetMacro(NumInputImages,int);

  //Description:
  // Dimension of image Data 
  vtkGetVector3Macro(DataDim,int);

  //Description:
  // Spacing of image data
  vtkGetVector3Macro(DataSpacing, float);                 

  // Description:
  // Define the Segmentation areae.g. have the image of size 256x256x124
  // now you want to segment a box of 70x 70 x 70 around the center of the image
  // -> Define  SegmentationBoundaryMin((256-70)/2,(256-70)/2,(124-70)/2)
  //            SegmentationBoundaryMax((256+70)/2,(256+70)/2,(124+70)/2)  
  vtkSetVector3Macro(SegmentationBoundaryMin,int);
  vtkSetVector3Macro(SegmentationBoundaryMax,int);

  vtkGetVector3Macro(SegmentationBoundaryMin,int);
  vtkGetVector3Macro(SegmentationBoundaryMax,int);

  // Description:
  // Make sure older version get an error message
  void SetInputIndex(vtkImageData *, int )  {
    vtkErrorMacro(<< "In the new version SetInputIndex is disabled! Please look into vtkImageEM*Class.h to find out how to set the given parameters !");
    return;
  }

  // =============================
  // For Message Protocol
  // So we can also enter streams for functions outside vtk
  char* GetErrorMessages();
  int GetErrorFlag();
  void ResetErrorMessage();

  //BTX
  ProtocolMessages* GetErrorMessagePtr(){return &this->ErrorMessage;}
  //ETX

  char* GetWarningMessages() {return this->WarningMessage.GetMessages(); }
  int GetWarningFlag() {return  this->WarningMessage.GetFlag();}
  void ResetWarningMessage() {this->WarningMessage.ResetParameters();}

  //BTX
  ProtocolMessages* GetWarningMessagePtr(){return &this->WarningMessage;}
  //ETX

  // Description:
  // Print out Weights (1 = Normal 2=as shorts normed to 1000)   
  vtkSetMacro(PrintWeights,int);
  vtkGetMacro(PrintWeights,int);

protected:
  vtkImageEMGenericClass();
  ~vtkImageEMGenericClass(){
    this->DeleteVariables(); 
  } 

  // Disable this function
  // vtkImageData* GetOutput() {return NULL;}

  void DeleteVariables(); 
  //BTX
  void ThreadedExecute(vtkImageData **vtkNotUsed(inData),
                       vtkImageData *vtkNotUsed(outData),
                       int vtkNotUsed(outExt)[6], int vtkNotUsed(id)) {};
  //ETX
  
 
  // Checks if all the parameters are setrectly
  // We do not have any input here
  //void  ExecuteData(vtkDataObject *) {this->Execute();}
  //void  Execute();
  void  ExecuteData(vtkDataObject *) ;

  float  ProbDataWeight;              // How much influence should the LocalPriorData have in the segmentation process 
  int    ProbDataScalarType;          // Scalar Type of ProbData

  double TissueProbability;           // Global Tissue Probability
  short  Label;
  float* InputChannelWeights;         // You can define different weights of input channels
  int    NumInputImages;              // Important for several input parameters such as size of LogCovariance 

  int    DataDim[3];                  // Dimension of image Data 
  float  DataSpacing[3];              // Spacing of image data
  int    SegmentationBoundaryMin[3];  // Boundary Box for the area of interest for the segmentation
  int    SegmentationBoundaryMax[3];  // Boundary Box for the area of interest for the segmentation 
  
  ProtocolMessages ErrorMessage;      // Lists all the error messges -> allows them to be displayed in tcl too 
  ProtocolMessages WarningMessage;    // Lists all the error messges -> allows them to be displayed in tcl too 

  int PrintWeights;                   // Print out Weights (1 = Normal 2=as shorts normed to 1000)   
private:
  vtkImageEMGenericClass(const vtkImageEMGenericClass&); // Not implemented
  void operator=(const vtkImageEMGenericClass&);  // Not implemented
};

#endif











