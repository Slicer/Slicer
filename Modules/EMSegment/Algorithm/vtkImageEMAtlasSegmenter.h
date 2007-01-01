/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMAtlasSegmenter.h,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
// .NAME vtkImageEMAtlasSegmenter
// Since 22-Apr-02 vtkImageEMAtlas3DSegmenter is called vtkImageEMAtlasSegmenter - Kilian
// EMAtlas =  using EM Algorithm with Private Tissue Class Probability
#ifndef __vtkImageEMAtlasSegmenter_h
#define __vtkImageEMAtlasSegmenter_h 
  
#include "vtkImageEMGeneral.h" 
#include "vtkEMSegment.h"
#include "vtkImageData.h"
#include "vtkImageEMAtlasSuperClass.h"

// ---------------------------------------------------

//BTX  
// Needed to parallelise the algorithm
typedef struct {
  int           id ;
  float         **w_m_input;
  unsigned char *MapVector;
  float         *cY_M; 
  int           imgX;
  int           imgY; 
  int           imgXY;
  int           StartVoxel;
  int           EndVoxel;
  int           NumClasses;
  int           NumTotalTypeCLASS;
  int*          NumChildClasses;
  int           NumInputImages;
  double        Alpha;
  double        ***MrfParams;
  void          ** ProbDataPtr;
  int           *ProbDataIncY; 
  int           *ProbDataIncZ; 
  float         *ProbDataWeight;
  float         *ProbDataMinusWeight;
  int           ProbDataType;
  double        **LogMu;
  double        ***InvLogCov;
  double        *InvSqrtDetLogCov;
  double        *TissueProbability;
  int           *VirtualNumInputImages;
  float         **w_m_output;
} EMAtlas_MF_Approximation_Work_Private;

//ETX 
class VTK_EMSEGMENT_EXPORT vtkImageEMAtlasSegmenter : public vtkImageEMGeneral
{
  public:
  // -----------------------------------------------------
  // Genral Functions for the filter
  // -----------------------------------------------------
  static vtkImageEMAtlasSegmenter *New();
  vtkTypeMacro(vtkImageEMAtlasSegmenter,vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);
  // -----------------------------------------------------
  // Setting Algorithm 
  // -----------------------------------------------------
  vtkSetMacro(NumIter, int);
  vtkGetMacro(NumIter, int);

  vtkSetMacro(NumRegIter, int);
  vtkGetMacro(NumRegIter, int);

  vtkSetMacro(Alpha, double);
  vtkGetMacro(Alpha, double);

  vtkSetMacro(SmoothingWidth, int);
  vtkGetMacro(SmoothingWidth, int);

  vtkSetMacro(SmoothingSigma, int);
  vtkGetMacro(SmoothingSigma, int);

  void SetNumberOfTrainingSamples(int Number) {this->NumberOfTrainingSamples = Number;}
  vtkGetMacro(NumberOfTrainingSamples, int);

  int* GetSegmentationBoundaryMin();
  int* GetSegmentationBoundaryMax();

  int* GetExtent(){return this->Extent;}

  // -----------------------------------------------------
  // Print functions 
  // -----------------------------------------------------
  vtkGetStringMacro(PrintDir);
  vtkSetStringMacro(PrintDir);

  // Description:
  // Number of input images for the segmentation - Has to be defined before defining any class specific setting 
  // Otherwise they get deleted
  // Be carefull: this is just the number of images not attlases, 
  // e.g. I have 5 tissue classes and 3 Inputs (T1, T2, SPGR) -> NumInputImages = 3
  void SetNumInputImages(int number);
  int GetNumInputImages() {return this->NumInputImages;} 

  void SetImageInput(int index, vtkImageData *image) {this->SetInput(index,image);}

  vtkGetMacro(ImageProd, int); 

  // Desciption:
  // Dimension of work area => SegmentationBoundaryMax[i] - SegmentationBoundaryMin[i] + 1 
  int GetDimensionX();
  int GetDimensionY();
  int GetDimensionZ();

  // -----------------------------------------------------
  // EM-MF Function 
  // -----------------------------------------------------  
  // Description:
  // Defines the Label map of a given image
  static void DetermineLabelMap(short *LabelMap, int NumTotalTypeCLASS, int* NumChildClasses, vtkImageEMAtlasSuperClass* head,  short* ROI,int ImageMax, float **w_m);

  // Desciption:
  // Special function for parallelise MF part -> Creating Threads 
  int MF_Approx_Workpile(float **w_m_input,unsigned char* MapVector, float *cY_M, int imgXY,double ***InvLogCov,
                         double *InvSqrtDetLogCov, int NumTotalTypeCLASS, int* NumChildClasses, int NumClasses, void** ProbDataPtr, 
                         int* ProbDataIncY, int* ProbDataIncZ, float* ProbDataWeight, float *ProbDataMinusWeight, double** LogMu, 
                         double* TissueProbability, int *VirtualNumInputImages, vtkImageEMAtlasSuperClass* head, float **_m_output);

  // Description:
  // Print out intermediate result of the algorithm in a  file
  // The file is called  this->PrintIntermediateDir/EM*.m
  void PrintIntermediateResultsToFile(int iter, float **w_m, short *ROI, unsigned char* OutputVector, int NumTotalTypeCLASS, int* NumChildClasses, 
                      vtkImageEMAtlasSuperClass* actSupCl, char* LevelName, void **ClassList, classType *ClassListType, int* LabelList, FILE** QualityFile);

  // -----------------------------------------------------
  // Intensity Correction 
  // -----------------------------------------------------

  //BTX
  vtkImageEMAtlasSuperClass* GetActiveSuperClass() {return this->activeSuperClass;}
  vtkImageEMAtlasSuperClass* GetHeadClass() {return this->HeadClass;}
  //ETX
  int HierarchicalSegmentation(vtkImageEMAtlasSuperClass* head, float** InputVector,short *ROI, short *OutputVector, EMTriVolume & iv_m, EMVolume *r_m, char* LevelName);

  //Kilian rewrite it 
  void PrintSuperClass () {
    printf("Current: %f Prob: %f Label: %d Parent: %f \n", activeSuperClass, activeSuperClass->GetTissueProbability(), activeSuperClass->GetLabel(), activeSuperClass->GetParentClass());
  } 

  // Desciption:
  // Head Class is the inital class under which all subclasses are attached  
  void SetHeadClass(vtkImageEMAtlasSuperClass *InitHead) {
    InitHead->Update(); 
    if (InitHead->GetErrorFlag()) {
      // This is done before this->Update() so we cannot use Error Message Report;
      vtkErrorMacro(<<"Cannot set HeadClass because the class given has its ErrorFlag activated !");
      return;
    }
    this->HeadClass   = InitHead;
    this->activeClass = (void*) InitHead;
    this->activeClassType  = SUPERCLASS;
  }

  // =============================
  // For Message Protocol
  char* GetErrorMessages() {return this->ErrorMessage.GetMessages(); }
  int GetErrorFlag() {return  this->ErrorMessage.GetFlag();}
  void ResetErrorMessage() {this->ErrorMessage.ResetParameters();}
  // So we can also enter streams for functions outside vtk
  ProtocolMessages* GetErrorMessagePtr(){return &this->ErrorMessage;}

  char* GetWarningMessages() {return this->WarningMessage.GetMessages(); }
  int GetWarningFlag() {return  this->WarningMessage.GetFlag();}
  void ResetWarningMessage() {this->WarningMessage.ResetParameters();}
  ProtocolMessages* GetWarningMessagePtr(){return &this->WarningMessage;}
protected:
  vtkImageEMAtlasSegmenter();
  vtkImageEMAtlasSegmenter(const vtkImageEMAtlasSegmenter&) {};
  ~vtkImageEMAtlasSegmenter(); 
  void DeleteVariables();

  void operator=(const vtkImageEMAtlasSegmenter&) {};
  void ExecuteData(vtkDataObject *);   

  //BTX
  // Description:
  // Checks all intput image if they have coresponding dimensions 
// #if EM_VTK_OLD_SETTINGS
//   int CheckInputImage(vtkImageData * inData,int DataTypeOrig, float DataSpacingOrig[3], int num);
// #else 
  int CheckInputImage(vtkImageData * inData,int DataTypeOrig, vtkFloatingPointType DataSpacingOrig[3], int num);
// #endif
  // Description:
  // Resets the error flag and messages 
  void ResetMessageSettings();  

  int NumIter;         // Number of EM-iterations
  int NumRegIter;      // Number of iteration in E- Step to regularize weights 
  double Alpha;        // alpha - Paramter 0<= alpaha <= 1

  int SmoothingWidth;  // Width for Gausian to regularize weights   
  int SmoothingSigma;  // Sigma paramter for regularizing Gaussian

  int NumInputImages;               // Number of input images  
 
  // These are defined in vtkEMImagePrivateSegment
  char* PrintDir;        // In which directory should the results be printed  

  // New Variables for Local Version 
  int ImageProd;                   // Size of Image = DimensionX*DimensionY*DimensionZ

  int Extent[6];                  // Extent of images - needed for several inputs 

  int NumberOfTrainingSamples;    // Number of Training Samples Probability Image has been summed up over !  

  vtkImageEMAtlasSuperClass *activeSuperClass;   // Currently Active Super Class -> Important for interface with TCL
  classType    activeClassType;

  vtkImageEMAtlasSuperClass *HeadClass;          // Initial Class

  void *activeClass;               // Currently Active Class -> Important for interface with TCL

  ProtocolMessages ErrorMessage;    // Lists all the error messges -> allows them to be displayed in tcl too 
  ProtocolMessages WarningMessage;  // Lists all the warning messges -> allows them to be displayed in tcl too 
  //ETX
};
#endif











