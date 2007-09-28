/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See Doc/copyright/copyright.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkImageEMLocalSegmenter.cxx,v $
Date:      $Date: 2006/12/15 19:11:06 $
Version:   $Revision: 1.2 $

=========================================================================auto=*/
// Since 22-Apr-02 vtkImageEMLocal3DSegmenter is called vtkImageEMLocalSegmenter - Kilian
// EMLocal =  using EM Algorithm with Local Tissue Class Probability
#include "vtkImageEMLocalSegmenter.h"
#include "vtkObjectFactory.h"
#include "EMLocalAlgorithm.h"
#include "assert.h"
//------------------------------------------------------------------------------
// General vtkImageEMLocalSegmenter functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
vtkImageEMLocalSegmenter* vtkImageEMLocalSegmenter::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMLocalSegmenter");
  if(ret)
    {
    return (vtkImageEMLocalSegmenter*)ret;
    }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMLocalSegmenter;
}

//----------------------------------------------------------------------------
vtkImageEMLocalSegmenter::vtkImageEMLocalSegmenter()
{
  this->Alpha = 0.7;                     // 0 <= alpha <= 1. Be carefull - has great influence over outcome 
  this->SmoothingWidth = 11;             // Width for Gausian to regularize weights
  this->SmoothingSigma = 5;              // Sigma paramter for regularizing Gaussian
  this->NumInputImages = 0;              // Number of input images
  this->DisableMultiThreading = 0;       // For validation purposes you might want to disable MultiThreading 
                                         // so that you get the same results on different machines. If disabled 
                                         // and run on multi processor machines it will lower the performance 
 
  this->PrintDir = NULL;                 // Directory in which everything should be printed out  
  memset(this->Extent, 0, sizeof(int)*6);  // Need to know the original extent for several functions in the class 

  this->NumberOfTrainingSamples = 0;     // Number of Training Samples Probability Image has been summed up over !
  this->ImageProd = 0;                   // Size of Image = maxX*maxY*maxZ

  this->HeadClass = NULL;
  this->activeSuperClass = NULL;
  this->activeClass      = NULL;
  this->activeClassType  = SUPERCLASS;
  
  this->RegistrationInterpolationType = 0;

  this->DebugImage       = NULL; 

}

//------------------------------------------------------------------------------
vtkImageEMLocalSegmenter::~vtkImageEMLocalSegmenter(){
  this->DeleteVariables();
}

void vtkImageEMLocalSegmenter::DeleteVariables() {
  if (this->PrintDir) delete[] this->PrintDir;
  this->PrintDir = NULL;

  this->NumInputImages = 0 ;
  this->activeSuperClass = NULL;
  this->activeClass = NULL;

  if (this->DebugImage) delete[] this->DebugImage;
  this->DebugImage = NULL;

  if (this->HeadClass)
    {
    this->HeadClass->Delete();
    }
}

void vtkImageEMLocalSegmenter::PrintSelf(ostream& os,vtkIndent indent) {
  int i;
  // vtkImageEMGeneral::PrintSelf(os,indent);
   
  os << indent << "Alpha:                      " << this->Alpha << "\n";
  os << indent << "SmoothingWidth:             " << this->SmoothingWidth << "\n";
  os << indent << "SmoothingSigma:             " << this->SmoothingSigma << "\n";

  os << indent << "NumInputImages:             " << this->NumInputImages << "\n";
  os << indent << "PrintDir:                   " << (this->PrintDir ? this->PrintDir : "(none)") << "\n"; 
  os << indent << "NumberOfTrainingSamples:    " << this->NumberOfTrainingSamples << "\n";
  os << indent << "activeSuperClass:           " ;
  if (this->activeSuperClass) os << this->activeSuperClass->GetLabel() << "\n";
  else os << "(none) \n"; 
  os << indent << "activeClassType:            ";
  if (activeClassType == CLASS) {
  os << "CLASS" << "\n";
  os << indent << "activeClass:                "; 
  if (this->activeClass) os << ((vtkImageEMLocalClass*)this->activeClass)->GetLabel() << "\n";
  else os << "(none) \n";
  } else {
  os << "SUPERCLASS" << "\n";
  os << indent << "activeClass:                ";
  if (this->activeClass) os << ((vtkImageEMLocalSuperClass*)this->activeClass)->GetLabel() << "\n";
  else os << "(none) \n";
  }
  os << indent << "Extent:                     " ;
  for (i=0; i < 6; i++) os << this->Extent[i]<< " " ; 
  os << "\n";
  os << indent << "RegistrationInterpolationType: " << this->RegistrationInterpolationType  << "\n";

  this->HeadClass->PrintSelf(os,indent);
}

// --------------------------------------------------------------------------------------------------------------------------
//  Set/Get Functions
// --------------------------------------------------------------------------------------------------------------------------
int* vtkImageEMLocalSegmenter::GetSegmentationBoundaryMin() {
  if (!this->HeadClass) {
  vtkEMAddErrorMessage("No Head Class defined");
  return NULL;
  } 
  return this->HeadClass->GetSegmentationBoundaryMin();
} 

int* vtkImageEMLocalSegmenter::GetSegmentationBoundaryMax() {
  if (!this->HeadClass) {
  vtkEMAddErrorMessage("No Head Class defined");
  return NULL;
  } 
  return this->HeadClass->GetSegmentationBoundaryMax();
} 


int vtkImageEMLocalSegmenter::GetDimensionX() {
  if (!this->HeadClass) {
  vtkEMAddErrorMessage("No Head Class defined");
  return -1;
  }
  if (this->HeadClass->GetDataDim()[0]) {
  return this->HeadClass->GetDataDim()[0];
  } 
  return (this->HeadClass->GetSegmentationBoundaryMax()[0] - this->HeadClass->GetSegmentationBoundaryMin()[0] +1);
}

int vtkImageEMLocalSegmenter::GetDimensionY() {
  if (!this->HeadClass) {
  vtkEMAddErrorMessage("No Head Class defined");
  return -1;
  } 
  if (this->HeadClass->GetDataDim()[1]) {
  return this->HeadClass->GetDataDim()[1];
  }
  return (this->HeadClass->GetSegmentationBoundaryMax()[1] - this->HeadClass->GetSegmentationBoundaryMin()[1] +1);
}

int vtkImageEMLocalSegmenter::GetDimensionZ() {
  if (!this->HeadClass) {
  vtkEMAddErrorMessage("No Head Class defined");
  return -1;
  }
  if (this->HeadClass->GetDataDim()[2]) { 
  return this->HeadClass->GetDataDim()[2];
  }
  return (this->HeadClass->GetSegmentationBoundaryMax()[2] - this->HeadClass->GetSegmentationBoundaryMin()[2] +1);
}


//------------------------------------------------------------------------------
void vtkImageEMLocalSegmenter::SetNumInputImages(int number) {
  // Right now can just define it once
  // this->DeleteVariables();
  if (this->NumInputImages > 0) {
  vtkEMAddErrorMessage( "Number of input images was previously defined ! ");
  return;
  }
  this->NumInputImages = number;
}

//----------------------------------------------------------------------------
// SuperClass Function
//----------------------------------------------------------------------------
void vtkImageEMLocalSegmenter::SetHeadClass(vtkImageEMLocalSuperClass *InitHead) {
  if (this->HeadClass == InitHead)
    {
    return;
    }

  InitHead->Update(); 
  if (InitHead->GetErrorFlag()) {
  // This is done before this->Update() so we cannot use Error Message Report;
  vtkErrorMacro(<<"Cannot set HeadClass because the class given has its ErrorFlag activated !");
  return;
  }

  if (this->HeadClass)
    {
    this->HeadClass->Delete();
    }

  InitHead->Register(this);
  this->HeadClass   = InitHead;
  this->activeClass = (void*) InitHead;
  this->activeClassType  = SUPERCLASS;
}


//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class T>
static void vtkImageEMLocalSegmenterReadInputChannel(vtkImageEMLocalSegmenter *self,vtkImageData *in1Data, T *in1Ptr,int inExt[6],float** InputVector,int InputIndex)
{

  int idxR, idxY, idxZ;
  vtkIdType inIncX, inIncY, inIncZ;
  int *SegmentationBoundaryMin = self->GetSegmentationBoundaryMin();
  vtkNotUsed(int *SegmentationBoundaryMax = self->GetSegmentationBoundaryMax(););
  int ImageMaxZ = self->GetDimensionZ();
  int ImageMaxY = self->GetDimensionY();
  int ImageMaxX = self->GetDimensionX();

  int index = 0; 
  double IntensityCorrection = 0.0;

  // Get increments to march through data 
  in1Data->GetContinuousIncrements(inExt, inIncX, inIncY, inIncZ);
  // We assume here input is 1 scalar -> it is checked in CheckInputImages

  int LengthOfXDim = inExt[1] - inExt[0]+1 + inIncY;
  int LengthOfYDim = LengthOfXDim*(inExt[3] - inExt[2]+1) + inIncZ;  
  int jump = (SegmentationBoundaryMin[0] - 1) + (SegmentationBoundaryMin[1] - 1) * LengthOfXDim
    + LengthOfYDim *(SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - ImageMaxX;
  int BoundaryDataIncZ = LengthOfYDim - ImageMaxY *LengthOfXDim;

  in1Ptr += jump;

  // cout << "-- jump " << jump << "BoundaryDataIncY " << BoundaryDataIncY << " BoundaryDataIncZ " << BoundaryDataIncZ << endl;
  for (idxZ = 0; idxZ < ImageMaxZ ; idxZ++) { 
  for (idxY = 0; idxY <  ImageMaxY; idxY++) {
  for (idxR = 0; idxR < ImageMaxX; idxR++) {
  if (double(* in1Ptr) >  IntensityCorrection) {
  InputVector[index][InputIndex] = log(float(* in1Ptr) +1);
  } else {
  InputVector[index][InputIndex] = 0.0;
  }
  index ++; 
  in1Ptr++;
  }
  in1Ptr += BoundaryDataIncY;
  }
  in1Ptr += BoundaryDataIncZ;
  }
}


// --------------------------------------------------------------------------------------------------------------------------
//  Running Segmentation 
// --------------------------------------------------------------------------------------------------------------------------

//----------------------------------------------------------------------------
// Transferes the data wrom the working extent (that is only the area defined by the SegmentationBounday)
// to the Extent of the output - if SliceNum > 0 then it will just print out one slice

template <class TIn, class TOut>
static void vtkImageEMLocalSegmenter_TransfereDataToOutputExtension(vtkImageEMLocalSegmenter *self,TIn* inputExtension_Vector, TOut* outputExtension_Ptr, 
                                                                    int outputExtent_Inc[3], int SliceNum) {
  // -----------------------------------------------------
  // 1.) Setup Parameteres 
  // -----------------------------------------------------
  // Get increments to march through data
  int outputExtension[6];
  memcpy(outputExtension,self->GetExtent(), sizeof(int)*6);

  int SegmentationBoundaryMin[3];
  memcpy(SegmentationBoundaryMin,self->GetSegmentationBoundaryMin(), sizeof(int)*3);

  int DimensionX = self->GetDimensionX();
  int DimensionY = self->GetDimensionY();
  int DimensionZ = self->GetDimensionZ();

  if (SliceNum > 0) {
  outputExtension[4] += SliceNum-1 + SegmentationBoundaryMin[2] -1;
  outputExtension[5] = outputExtension[4];
  SegmentationBoundaryMin[2] = 1;
  DimensionZ = 1;
  }

  int idxR, idxY, idxZ;

  // We assume output is 1 scalar 
  TOut* outputExtensionStart_Ptr = outputExtension_Ptr; 

  int LengthOfXDim = outputExtension[1] - outputExtension[0] +1  + outputExtent_Inc[1];
  int LengthOfYDim = LengthOfXDim*(outputExtension[3] - outputExtension[2] + 1) +  outputExtent_Inc[2];  
  // Location of the first voxel 
  outputExtensionStart_Ptr += (SegmentationBoundaryMin[0] - 1) + (SegmentationBoundaryMin[1] - 1) * LengthOfXDim
    + LengthOfYDim *(SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - DimensionX;
  int BoundaryDataIncZ = LengthOfYDim - DimensionY *LengthOfXDim;

  // -----------------------------------------------------
  // 2.) Set output to zero 
  // -----------------------------------------------------

  // Initializa output by setting it to zero
  for (idxZ = outputExtension[4]; idxZ <= outputExtension[5]; idxZ++) {
  for (idxY = outputExtension[2]; idxY <= outputExtension[3]; idxY++) {
  memset(outputExtension_Ptr,0,sizeof(TOut)*(outputExtension[1] - outputExtension[0] + 1));
  outputExtension_Ptr +=  LengthOfXDim;
  }
  }
  outputExtension_Ptr = outputExtensionStart_Ptr ;  


  // -----------------------------------------------------
  // 3.) Define output in working Extension 
  // -----------------------------------------------------
  int index = 0;
  // 3.) Write result in outPtr
  for (idxZ = 0; idxZ < DimensionZ; idxZ++) {
  for (idxY = 0; idxY < DimensionY; idxY++) {
  for (idxR = 0; idxR < DimensionX; idxR++) {
  *outputExtension_Ptr = (TOut) inputExtension_Vector[index];
  index ++; 
  outputExtension_Ptr++;
  }
  outputExtension_Ptr +=  BoundaryDataIncY;
  }
  outputExtension_Ptr +=  BoundaryDataIncZ;
  }
}

//------------------------------------------------------------------------------
int vtkImageEMLocalSegmenter::CheckInputImage(vtkImageData * inData,int DataTypeOrig, vtkFloatingPointType DataSpacingOrig[3], int num) {
  vtkFloatingPointType DataSpacingNew[3];

  int inExt[6];
  if (inData == NULL) {
  vtkEMAddErrorMessage("CheckInputImage: Input "<< num << " must be specified.");
  return 1;
  }

  // Check for Data Type if correct : Remember ProbabilityData all has to be of the same data Type
  int DataTypeNew = inData->GetScalarType();
  if (DataTypeOrig != DataTypeNew) 
    vtkEMAddErrorMessage("CheckInputImage: Input "<< num << " has wrong data type ("<< inData->GetScalarType() << ") instead of " 
                         << DataTypeOrig << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT );

  // Check if it is one of the registered types
  if ((DataTypeNew != VTK_DOUBLE) && (DataTypeNew != VTK_FLOAT) && 
      (DataTypeNew != VTK_LONG) && (DataTypeNew != VTK_UNSIGNED_LONG) && (DataTypeNew != VTK_INT) && (DataTypeNew != VTK_UNSIGNED_INT) && 
      (DataTypeNew != VTK_SHORT) && (DataTypeNew != VTK_UNSIGNED_SHORT) && (DataTypeNew != VTK_CHAR) && (DataTypeNew != VTK_UNSIGNED_CHAR)) vtkEMAddErrorMessage("Input "<< num << " has not known data type.");

  // Check for dimenstion of InData
  // Could be easily fixed if needed 
  inData->GetWholeExtent(inExt);
  if ((inExt[1] != this->Extent[1]) || (inExt[0] != this->Extent[0]) || (inExt[3] != this->Extent[3]) || (inExt[2] != this->Extent[2]) || (inExt[5] != this->Extent[5]) || (inExt[4] != this->Extent[4])) 
    vtkEMAddErrorMessage("CheckInputImage: Extension of Input Image " << num << ", " << inExt[0] << "," << inExt[1] << "," << inExt[2] << "," << inExt[3] << "," << inExt[4] << "," << inExt[5] 
                         << "is not alligned with output image "  << this->Extent[0] << "," << this->Extent[1] << "," << this->Extent[2] << "," << this->Extent[3] << "," << this->Extent[4] << " " << this->Extent[5]);

  if (inData->GetNumberOfScalarComponents() != 1) 
    vtkEMAddErrorMessage("CheckInputImage: This filter assumes input to filter is defined with one scalar component. " << num  << " has " << inData->GetNumberOfScalarComponents() 
                         << " Can be easily changed !");

  inData->GetSpacing(DataSpacingNew);
  if ((DataSpacingOrig[0] !=  DataSpacingNew[0]) || (DataSpacingOrig[1] !=  DataSpacingNew[1]) || (DataSpacingOrig[2] !=  DataSpacingNew[2])) 
    vtkEMAddErrorMessage("CheckInputImage: Data Spacing of input images is unequal" );
  // Kilian Check for orientation has to be done in TCL !!!!
  return this->GetErrorFlag();
}


template <class T>  
void vtkImageEMLocalSegment_RunEMAlgorithm(vtkImageEMLocalSegmenter *self, T** ProbDataPtr, int NumTotalTypeCLASS, int ImageProd,  float **InputVector, short *ROI, 
                                           char *LevelName, float GlobalRegInvRotation[9], float GlobalRegInvTranslation[3], int RegistrationType, 
                                           EMTriVolume& iv_m, EMVolume *r_m, short *SegmentationResult, int DataType, int &SegmentLevelSucessfullFlag) {
  // Initialize Values
  float **w_m            = new float*[NumTotalTypeCLASS];
  for (int i=0; i< NumTotalTypeCLASS; i++) w_m[i] = new float[ImageProd];

  EMLocalAlgorithm<T> Algorithm(self, ProbDataPtr, InputVector, ROI, w_m, LevelName, GlobalRegInvRotation, GlobalRegInvTranslation, RegistrationType, 
                                DataType,SegmentLevelSucessfullFlag);

  // Run Algorithm
  if (SegmentLevelSucessfullFlag) Algorithm.RunAlgorithm(iv_m,r_m,SegmentLevelSucessfullFlag);

  // Determine Labelmap
  if (SegmentLevelSucessfullFlag) Algorithm.DetermineLabelMap(SegmentationResult);

  // Propagate Errormessages
  if (Algorithm.GetErrorFlag()) vtkEMJustAddErrorMessageSelf(Algorithm.GetErrorMessages());
  if (Algorithm.GetWarningFlag()) vtkEMJustAddWarningMessageSelf(Algorithm.GetWarningMessages());

  // Clean up 
  for (int i=0; i<NumTotalTypeCLASS; i++) delete[] w_m[i]; 
  delete []w_m;
} 


//------------------------------------------------------------------------------
// Needed to define hierarchies! => this will be done at a later point int time at vtkImageEMLocalSuperClass
// I did this design to multi thread it later
// If you start it always set ROI == NULL
int vtkImageEMLocalSegmenter::HierarchicalSegmentation(vtkImageEMLocalSuperClass* head, float** InputVector,short *ROI, short *OutputVector, EMTriVolume & iv_m, 
                                                       EMVolume *r_m,char* LevelName, float GlobalRegInvRotation[9], float GlobalRegInvTranslation[3]) {
  cout << "Start vtkImageEMLocalSegmenter::HierarchicalSegmentation"<< endl;  
  // Nothing to segment
  if (head->GetNumClasses() ==0) {
  if (ROI == NULL) memset(OutputVector,0, sizeof(short)*this->ImageProd);
  return 0;
  }

  // ---------------------------------------------------------------
  // 1. Define Variables
  // ---------------------------------------------------------------
  cout <<"====================================== Segmenting Level " << LevelName << " ==========================================" << endl;

  char      *NewLevelName = new char[strlen(LevelName)+5];
  void      **ClassList = head->GetClassList();
  classType *ClassListType = head->GetClassListType();
  int       NumClasses = head->GetNumClasses();
  int       NumTotalTypeCLASS = head->GetTotalNumberOfClasses(false); // This includes only classes from Type CLASSES  
  int       SegmentLevelSucessfullFlag;
  int       RegistrationType = head->GetRegistrationType();
  
  if (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) {
  if (!this->RegistrationInterpolationType) {
  RegistrationType = EMSEGMENT_REGISTRATION_DISABLED ;
  vtkEMAddWarningMessage("RegistrationType is set but RegistrationInterpolationType == 0 => deactivated registration !");
  }
  

  if (RegistrationType > EMSEGMENT_REGISTRATION_GLOBAL_ONLY  && NumClasses <= 2) {
  RegistrationType = EMSEGMENT_REGISTRATION_GLOBAL_ONLY ;
  vtkEMAddWarningMessage("RegistrationType also involves structure specific segmentation but only 2 classes are defined in this level => RegistrationType is set to global only !");
  }

  if (RegistrationType !=  EMSEGMENT_REGISTRATION_CLASS_ONLY && (RegistrationType !=  EMSEGMENT_REGISTRATION_APPLY) && strcmp(LevelName,"1")) {
  // Kilian: I am running experiment doing shape estimation and registration at the same time 
  // - may be the note below is correct - still want to try it out 
  // RegistrationType = EMSEGMENT_REGISTRATION_CLASS_ONLY;
  // vtkEMAddWarningMessage("Using global registration bejond the first level does not make sense - please change before running software again");
  // exit(1),
  }

  if (RegistrationType == EMSEGMENT_REGISTRATION_SIMULTANEOUS && head->GetGenerateBackgroundProbability()) { 
  vtkEMAddWarningMessage("If RegistrationType == 3, GenerateBackgroundProbability should not be activated because within the registration " 
                         << "costfunction the background calculation and the global registration parameter depend on each other!");
  }
  }

  cout << "Registration Type: ";
  switch (RegistrationType)  {
  case EMSEGMENT_REGISTRATION_DISABLED     : cout << "Disabled" << endl; break;
  case EMSEGMENT_REGISTRATION_APPLY        : cout << "Apply" << endl; break;
  case EMSEGMENT_REGISTRATION_GLOBAL_ONLY  : cout << "Global only " << endl; break;
  case EMSEGMENT_REGISTRATION_CLASS_ONLY   : cout << "Class only " << endl; break;
  case EMSEGMENT_REGISTRATION_SIMULTANEOUS : cout << "Simultaneously " << endl; break;
  case EMSEGMENT_REGISTRATION_SEQUENTIAL   : cout << "Sequential " << endl; break;
  default : 
    vtkEMAddErrorMessage("Unknown Registration Type " << RegistrationType) ;
    return 0;
  } 
  cout << "GenerateBackgroundProbability: " << (head->GetGenerateBackgroundProbability() ? "On" : "Off" ) << endl;


  // The follwoing division is done for multi threading purposes -> even though it is currently not implemented 
  // you would also have to make a copy of iv_m and r_m . I currently do not do it because it takes to much space
  // Also first install zoom function before doing it !
  // I will do it sometime
  // Note: ROI is Region of Interest => Read Only ! OutputVecotr is Write only !  
  // It needs to be a class of vtkImageEMLocalSegmenter => therefore we cannot use outPtr instead of OutputVactor 
  short *SegmentationResult = new short[this->ImageProd],
    *ROIPtr             = ROI, 
    *SegResultPtr       = SegmentationResult,
    *OutputVectorPtr    = OutputVector;

  memset(SegmentationResult,0,sizeof(short)*this->ImageProd);



  void  **ProbDataPtr    = new void*[NumTotalTypeCLASS];

  // Depending if we do the registration we have to consider boundary condition or not 
  head->GetProbDataPtrList(ProbDataPtr,0,!RegistrationType);

  // This is retroactive - it gives the warning here that the superclass ignores prob data even though that was important on the 
  // last level segmentation where the super class was involved - this is just simpler to program
  if ((head->GetProbDataWeight() == 0.0) && (this->GetHeadClass() != head)) 
    {
    for (int i=0; i<NumTotalTypeCLASS; i++) 
      {
      if (ProbDataPtr[i]) 
        {
        i = NumTotalTypeCLASS;
        vtkEMAddWarningMessage("Super Class segmented on Level " << LevelName  << " has ProbDataWeight == 0.0, but there are sub classes that have probability maps defined\n          => Probability Maps will be ignored!");
        } 
      }
    }
  
  this->activeSuperClass = head;
  // ---------------------------------------------------------------
  // 2. Segment Subject
  // ---------------------------------------------------------------
 
  // If ProbDataScalarType == 2 => non of the structures have a spatial distribution defined
  int ProbDataScalarType = (head->GetProbDataScalarType() > -1 ? head->GetProbDataScalarType() : VTK_CHAR) ;
  // cout << "Probability data is of type " << vtkImageScalarTypeNameMacro(ProbDataScalarType) << endl;
  
  switch (ProbDataScalarType) 
    {
    vtkTemplateMacro(vtkImageEMLocalSegment_RunEMAlgorithm(this, (VTK_TT**) ProbDataPtr, NumTotalTypeCLASS, this->ImageProd, InputVector, ROI, LevelName, 
                                                           GlobalRegInvRotation, GlobalRegInvTranslation, RegistrationType, iv_m, r_m, SegmentationResult, 
                                                           ProbDataScalarType,SegmentLevelSucessfullFlag )); 
    default :
      vtkEMAddErrorMessage("vtkImageEMLocalSegmenter::HierarchicalSegmentation: Unknown ScalarType " << ProbDataScalarType);
      SegmentLevelSucessfullFlag = 0;
    }  
  // ---------------------------------------------------------------
  // Update Global Registration Vectors
  // ---------------------------------------------------------------
  if (RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) {
  float NewGlobalRegInvRotation[9];
  float NewGlobalRegInvTranslation[3];
  if (EMLocalAlgorithm_RegistrationMatrix(head->GetRegistrationTranslation(), head->GetRegistrationRotation(), head->GetRegistrationScale(), 
                                          GlobalRegInvRotation, GlobalRegInvTranslation, NewGlobalRegInvRotation, NewGlobalRegInvTranslation,(this->GetDimensionZ() > 1 ? 0 : 1))) {
  vtkEMAddErrorMessage(" SetParameters: Cannot invert rotation matrix defined by the pararmeters");
  SegmentLevelSucessfullFlag = 0;
  }
  memcpy(GlobalRegInvRotation, NewGlobalRegInvRotation,sizeof(float)*9);
  memcpy(GlobalRegInvTranslation, NewGlobalRegInvTranslation,sizeof(float)*3);
  } 
  
  // ---------------------------------------------------------------
  // 4. Segment Subclasses
  // ---------------------------------------------------------------
  if (SegmentLevelSucessfullFlag) { 
  // Super Class Labels have to be unique (except HeadClass => not defined !) and not equal to zero
  // Transfere segmentation results to OutputVector  
  for (int i=0; i < this->ImageProd; i++) {
  // Remeber ROI is initiated with NULL -> only on second level it gets extended 
  if ((ROIPtr == NULL) || (*ROIPtr++ == head->GetLabel())) *OutputVectorPtr = *SegResultPtr; 
  OutputVectorPtr++;
  SegResultPtr++; 
  }
  
  OutputVectorPtr = OutputVector;SegResultPtr = SegmentationResult; ROIPtr = ROI;

  // 4.) Run it for all sub Superclasses
  for (int i=0; i <NumClasses; i++) {
  // need to save results
  if (ClassListType[i] == SUPERCLASS &&  SegmentLevelSucessfullFlag) {
  sprintf(NewLevelName,"%s-%d",LevelName,i);
  // we should really create a copy of iv_m and r_m !! otherwise things can go wrong here
  if ((RegistrationType > EMSEGMENT_REGISTRATION_DISABLED) &&  (((vtkImageEMLocalSuperClass*) ClassList[i])->GetRegistrationType() == EMSEGMENT_REGISTRATION_DISABLED)) 
    vtkEMAddWarningMessage("SuperClass had registration enabled , but child had it disabled . Thus, the registration parameters wont be transfered to the next segmentation level");
  SegmentLevelSucessfullFlag = this->HierarchicalSegmentation((vtkImageEMLocalSuperClass*) ClassList[i],InputVector,SegmentationResult,OutputVector,iv_m,r_m,NewLevelName, 
                                                              GlobalRegInvRotation, GlobalRegInvTranslation);
  } 
  }
  }
  delete []SegmentationResult;
  delete []NewLevelName;
  delete []ProbDataPtr;
  cout << "End vtkImageEMLocalSegmenter::HierachicalSegmentation"<< endl; 
  return SegmentLevelSucessfullFlag;
}


//----------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
template <class TOut>
static void vtkImageEMLocalSegmenterExecute(vtkImageEMLocalSegmenter *self,float **InputVector,vtkImageData *outData, TOut *outPtr,int outExt[6])
{
  // -----------------------------------------------------
  // 1.) Setup  Hierarchical Segmentation
  // -----------------------------------------------------
  int NumInputImages = self->GetNumInputImages();
  short *OutputVector = new short[self->GetImageProd()];
  char LevelName[3];
  int DimensionX = self->GetDimensionX();
  int DimensionY = self->GetDimensionY();
  int DimensionZ = self->GetDimensionZ();

  EMTriVolume iv_m(NumInputImages,DimensionZ,DimensionY,DimensionX); // weighted inverse covariances 
  EMVolume *r_m  = new EMVolume[NumInputImages]; // weighted residuals
  for (int i=0; i < NumInputImages; i++) r_m[i].Resize(DimensionZ,DimensionY,DimensionX);
  // Print information
  cout << "Multi Threading is " ;
  if (self->GetDisableMultiThreading()) cout << "disabled." << endl;
  else cout << "working (" << vtkMultiThreader::GetGlobalDefaultNumberOfThreads() << " cpus)" << endl;

  if ( (DimensionX != (outExt[1] - outExt[0] +1)) ||(DimensionY != (outExt[3] - outExt[2] +1)) ||(DimensionZ != (outExt[5] - outExt[4] +1)))  
    cout << "Segmentation Boundary is activated (" <<DimensionX  <<"," << DimensionY << "," << DimensionZ <<") !" << endl;    

  cout << "Print intermediate result to " << self->GetPrintDir() << endl;
  if (self->GetRegistrationInterpolationType()) 
    cout << "Registration Interpolation Type: " << ( self->GetRegistrationInterpolationType() ==  EMSEGMENT_REGISTRATION_INTERPOLATION_LINEAR ? "Linear" : "Nearest Neighbour") << endl;

  // Label All SuperClasses 
    {
    int TotalNumClasses = self->GetHeadClass()->GetTotalNumberOfClasses(1);
    short *LabelList = new short[TotalNumClasses];
    memset(LabelList,0,sizeof(short)*TotalNumClasses);
    // Get all existing labels
    int index = self->GetHeadClass()->GetAllLabels(LabelList,0,TotalNumClasses);
    // Otherwise no classes defined
    assert(index);
    // Label all super classes
    self->GetHeadClass()->LabelAllSuperClasses(LabelList,index, TotalNumClasses);

    delete[] LabelList;
    }

    // self->GetHeadClass()->DefineAllLogInvCovParamters();
    // self->GetHeadClass()->Print("");  
    sprintf(LevelName,"1");

    float GlobalRegInvRotation[9];
    GlobalRegInvRotation[0] = GlobalRegInvRotation[4] = GlobalRegInvRotation[8] = 1.0;
    GlobalRegInvRotation[1] = GlobalRegInvRotation[2] = GlobalRegInvRotation[3] = GlobalRegInvRotation[5] = GlobalRegInvRotation[6] = GlobalRegInvRotation[7] = 0.0;
    float GlobalRegInvTranslation[3];
    GlobalRegInvTranslation[0] = GlobalRegInvTranslation[1] = GlobalRegInvTranslation[2] = 0.0;

    // -----------------------------------------------------
    // 2.) Run  Hierarchical Segmentation
    // -----------------------------------------------------
    if (self->HierarchicalSegmentation(self->GetHeadClass(),InputVector, NULL, OutputVector,iv_m,r_m,LevelName,GlobalRegInvRotation,GlobalRegInvTranslation) == 0) {
    memset(OutputVector,0,sizeof(short)*self->GetImageProd());
    }

    // -----------------------------------------------------
    // 3.) Define   int outIncX, outIncY, outIncZ;
    // -----------------------------------------------------
    vtkIdType outIncX, outIncY, outIncZ;
    outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
    int outInc[3] = {outIncX, outIncY, outIncZ};
    vtkImageEMLocalSegmenter_TransfereDataToOutputExtension(self,OutputVector,outPtr,outInc,0);

    delete[] OutputVector;
    delete[] r_m;

    cout << "End vtkImageEMLocalSegmenterExecute "<< endl;
}



//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageEMLocalSegmenter::ExecuteData(vtkDataObject *)
{
  //std::cerr << "###vtkImageEMLocalSegmenter::ExecuteData" << std::endl;
  //this->PrintSelf(vtkstd::cerr, 0);

  cout << "Local Version" << endl;
  void *outPtr;
  int idx1, i;
  vtkNotUsed(int NumProbMap = 0;);
  vtkNotUsed(int FirstProbMapDef = -1;);
  // vtk4 - to find out more about the next couple of lines look at vtkImageEMGenericClass.cxx   
  vtkImageData **inData  = (vtkImageData **) this->GetInputs();
  vtkImageData *outData = this->GetOutput();
  // Magically the extent corresponds with the input extent !
  outData->GetWholeExtent(this->Extent);

  outData->SetExtent(this->Extent); 
  outData->AllocateScalars(); 
  // vtk4
  // -----------------------------------------------------
  // Define and Check General Parameters
  // -----------------------------------------------------
  vtkDebugMacro(<< "Execute: inData = " << inData << ", outData = " << outData);

  this->ResetErrorMessage();
  this->ResetWarningMessage();

  // this->NumberOfInputs = Actual number of inputs defined
  if (this->NumInputImages > this->NumberOfInputs) {
  vtkEMAddErrorMessage( "NumOfInputs ("<< this->NumberOfInputs << ") is greater than the number of Input images defined ("<<this->NumberOfInputs<<")!");
  return;
  } 

  if (this->NumberOfTrainingSamples < 1) {
  vtkEMAddErrorMessage( "Number of Training Samples taken for the probability map has to be defined first!");
  return;
  }

  if (outData == NULL) {
  vtkEMAddErrorMessage( "Output must be specified.");
  return;
  }

  // Did we define a superclass
  if (!this->HeadClass) {
  vtkEMAddErrorMessage( "No Head Class is defined !");
  return;
  } 

  this->HeadClass->Update();
  if (this->HeadClass->GetErrorFlag()) {
  vtkEMAddErrorMessage( "The following Error's occured during the class definition:" << endl << this->HeadClass->GetErrorMessages());
  return;
  }

  if (this->HeadClass->GetWarningFlag()) 
    vtkEMAddWarningMessage( "The following Warning's occured during the class definition:" << endl << this->HeadClass->GetWarningMessages());
  
  // Check if everything coresponds to each other
  if (!inData[0]) {
  vtkEMAddErrorMessage( "First image input is not defined !");
  return;
  }
  if ((this->HeadClass->GetProbDataScalarType() > -1) && (this->HeadClass->GetProbDataScalarType() !=  inData[0]->GetScalarType())) {
  vtkEMAddErrorMessage( "Scalar Type of Probability maps defined in classes does not correspond to scalar type of input image1");
  return;
  }
  if (this->HeadClass->GetNumInputImages() != this->NumInputImages) {
  vtkEMAddErrorMessage("Number of Input images of classes differes from number of images defined for this filter!");
  return;
  }
 
  // -----------------------------------------------------
  // Define Image Parameters
  // -----------------------------------------------------
  // this->GetInput(StartInputImages)  = first intensity input
  this->ImageProd = this->GetDimensionX() * this->GetDimensionY() * this->GetDimensionZ();

  // -----------------------------------------------------
  // Checking dimension with start and end slice 
  // -----------------------------------------------------
  // Making sure values are set correctly
    { 
    int* BoundaryMin = this->GetSegmentationBoundaryMin();
    int* BoundaryMax = this->GetSegmentationBoundaryMax();
    for (i=0; i < 3; i++) {
    int Dimension = this->Extent[i*2+1] - this->Extent[i*2] + 1;  
    if ((BoundaryMin[i] > Dimension) || (BoundaryMin[i] < 1)) {
    vtkEMAddErrorMessage( "SegmentationBoundaryMin[" <<i<<"]=" << BoundaryMin[i] << " is not defined correctly ! Should not be smaller than "<<  Dimension << " and larger 0 !");
    return;
    }
    if ((BoundaryMax[i] > Dimension) || (BoundaryMax[i] < BoundaryMin[i]) ) {
    vtkEMAddErrorMessage( "SegmentationBoundaryMax[" <<i<<"]=" << BoundaryMax[i] << " is not defined correctly ! Should not be larger than "<<  Dimension 
                          << " and not smaller than SegmentationBoundaryMin (" << BoundaryMin[i] << ") !");
    return;
    }
    }
    }
    // -----------------------------------------------------
    // Read Input Images
    // -----------------------------------------------------
    float **InputVector = new float*[this->ImageProd];
    for(idx1 = 0; idx1 <this->ImageProd; idx1++) InputVector[idx1] = new float[this->NumInputImages];
 
    for (idx1 = 0; idx1 < this->NumInputImages ; idx1++){  
    if (this->CheckInputImage(inData[idx1],this->GetInput(0)->GetScalarType(), this->GetInput(0)->GetSpacing(), idx1+1)) return;
    switch (this->GetInput(idx1)->GetScalarType()) {
    vtkTemplateMacro6(vtkImageEMLocalSegmenterReadInputChannel,this, inData[idx1], (VTK_TT *)(inData[idx1]->GetScalarPointerForExtent(this->Extent)),this->Extent,InputVector,idx1);
    default:
      vtkEMAddErrorMessage( "Execute: Unknown ScalarType");
      return;
    } 
    }

    // -----------------------------------------------------
    // Read in Debugging Data 
    // -----------------------------------------------------
    // For Debugging:
    if (EM_DEBUG && (idx1 < this->NumberOfInputs)) {
    this->DebugImage = new short*[this->NumberOfInputs - idx1];
    i = 0;
    while (idx1 < this->NumberOfInputs) {
    cout << "Loading EMDEBUG Volume ("<< idx1 << ") into EMAlgorithm .....................";
    this->DebugImage[i] = (short*) inData[idx1]->GetScalarPointerForExtent(this->Extent);
    idx1 ++;
    i++;
    cout << "Finshed" << endl;
    }
    } else {
    this->DebugImage = NULL; 
    }

#if (EMVERBOSE)
      {
      cout << "Kilian - For Debugging purposes " << endl; 
      vtkIndent indent;
      this->PrintSelf(cout,indent); 
      }
#endif

    // -----------------------------------------------------
    // Execute Segmentation Algorithmm
    // -----------------------------------------------------
    outPtr = outData->GetScalarPointerForExtent(outData->GetExtent());
    switch (this->GetOutput()->GetScalarType()) {
    vtkTemplateMacro5(vtkImageEMLocalSegmenterExecute, this, InputVector, outData, (VTK_TT*)outPtr,this->Extent);
    default:
      vtkEMAddErrorMessage("Execute: Unknown ScalarType");
      return;
    }
    for(idx1 = 0; idx1 <this->ImageProd; idx1++) delete[] InputVector[idx1];
    delete[] InputVector;
}
