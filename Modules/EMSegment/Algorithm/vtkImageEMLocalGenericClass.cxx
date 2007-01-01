/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMLocalGenericClass.cxx,v $
  Date:      $Date: 2006/12/15 19:11:06 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
#include "vtkImageEMLocalGenericClass.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

//------------------------------------------------------------------------
vtkImageEMLocalGenericClass* vtkImageEMLocalGenericClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMLocalGenericClass");
  if(ret)
  {
    return (vtkImageEMLocalGenericClass*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMLocalGenericClass;
}

//----------------------------------------------------------------------------
vtkImageEMLocalGenericClass::vtkImageEMLocalGenericClass()
{
  this->ProbImageData       = NULL;

  memset(this->RegistrationTranslation,0,3*sizeof(double));
  memset(this->RegistrationRotation,0,3*sizeof(double));
  for (int i =0; i < 3; i++) RegistrationScale[i]= 1.0;
  this->PrintRegistrationParameters = 0;
  this->PrintRegistrationSimularityMeasure = 0 ; 

  this->RegistrationInvCovariance[0] =   this->RegistrationInvCovariance[1] =   this->RegistrationInvCovariance[2] =   this->RegistrationInvCovariance[3] 
                                     =   this->RegistrationInvCovariance[4] =   this->RegistrationInvCovariance[5] = 1.0;
  this->RegistrationInvCovariance[6] =   this->RegistrationInvCovariance[7] =   this->RegistrationInvCovariance[8] = 100; 

  memset(this->Extent,0,sizeof(int)*6);

  this->RegistrationClassSpecificRegistrationFlag =0;

  this->ExcludeFromIncompleteEStepFlag =0;
}

void vtkImageEMLocalGenericClass::SetRegistrationCovariance(double Init[9]) {
  for (int i = 0; i < 9;i ++) 
    if (Init[i] <= 0.0) {
      vtkEMAddErrorMessage("Error:vtkImageEMLocalGenericClass::SetRegistrationCovariance Input has to be greater 0 (" << i <<"th input : " << Init[i] << ")!");
    }
    else this->RegistrationInvCovariance[i] = 1.0/(Init[i]*Init[i]);
}


//----------------------------------------------------------------------------
void vtkImageEMLocalGenericClass::PrintSelf(ostream& os,vtkIndent indent) {
  // this->vtkImageMultipleInputFilter::PrintSelf(os,indent);
  this->vtkImageEMGenericClass::PrintSelf(os,indent);
  os << indent << "ProbImageData:           " << endl; 
  if (this->ProbImageData) this->ProbImageData->PrintSelf(os,indent.GetNextIndent());

  os << indent << "RegistrationTranslation:            " << this->RegistrationTranslation[0] << ", " << this->RegistrationTranslation[1] << ", " << this->RegistrationTranslation[2] << "\n" ;
  os << indent << "RegistrationRotation:               " << this->RegistrationRotation[0] << ", " << this->RegistrationRotation[1] << ", " << this->RegistrationRotation[2] << "\n" ;
  os << indent << "RegistrationScale:                  " << this->RegistrationScale[0] << ", " << this->RegistrationScale[1] << ", " << this->RegistrationScale[2] << "\n" ;
  os << indent << "PrintRegistrationParameters:        " << this->PrintRegistrationParameters << "\n" ;
  os << indent << "PrintRegistrationSimularityMeasure: " << this->PrintRegistrationSimularityMeasure << "\n" ;
  os << indent << "RegistrationClassSpecificRegistrationFlag: " << this->RegistrationClassSpecificRegistrationFlag << "\n" ;
  os << indent << "ExcludeFromIncompleteEStepFlag:     " << this->ExcludeFromIncompleteEStepFlag <<"\n" ;
}

//----------------------------------------------------------------------------
// Generic vtkImageData Classes
//----------------------------------------------------------------------------
// SegmentationBoundary condition are taken into account (BoundaryType == 1) or not (BoundaryType == 0);
void* vtkImageEMLocalGenericClass::GetDataPtr(vtkImageData* ImageData,int BoundaryType) {
  if (!ImageData) return NULL;

  int extent[6];
  ImageData->GetWholeExtent(extent);
  //vtkIndent indent;
  //ImageData->PrintSelf(cout, indent);
  if (BoundaryType) {
    vtkIdType DataIncX, DataIncY, DataIncZ;
    ImageData->GetContinuousIncrements(extent, DataIncX, DataIncY, DataIncZ);
  
    int LengthOfXDim = extent[1] - extent[0] + 1 + DataIncY;
    int LengthOfYDim = LengthOfXDim*(extent[3] - extent[2] + 1) + DataIncZ;  

    int jump = (this->SegmentationBoundaryMin[0] - 1) + (this->SegmentationBoundaryMin[1] - 1) * LengthOfXDim
               + LengthOfYDim *(this->SegmentationBoundaryMin[2] - 1);

    switch (ImageData->GetScalarType()) { 
      case VTK_DOUBLE:         return  (void*) (((double*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_FLOAT:          return  (void*) (((float*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_LONG:           return  (void*) (((long*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_UNSIGNED_LONG:  return  (void*) (((unsigned long*)ImageData->GetScalarPointerForExtent(extent)) + jump); 
      case VTK_INT:            return  (void*) (((int*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_UNSIGNED_INT:   return  (void*) (((unsigned int*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_SHORT:          return  (void*) (((short*)ImageData->GetScalarPointerForExtent(extent)) + jump); 
      case VTK_UNSIGNED_SHORT: return  (void*) (((unsigned short*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_CHAR:           return  (void*) (((char*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      case VTK_UNSIGNED_CHAR:  return  (void*) (((unsigned char*)ImageData->GetScalarPointerForExtent(extent)) + jump);
      default:
    cout << "vtkImageEMLocalSegmenter::HierarchicalSegmentation Unknown ScalarType" << endl;
      return NULL;
    }
  } else {
    return ImageData->GetScalarPointerForExtent(extent);
  }
}

//----------------------------------------------------------------------------
// If IncType = 0 => Returns DataIncY (BoundaryType = 0) or BoundaryDataIncY (BoundaryType = 1)
// If IncType = 1 => Returns DataIncZ (BoundaryType = 0) or BoundaryDataIncZ (BoundaryType = 1)
int vtkImageEMLocalGenericClass::GetImageDataInc(vtkImageData* ImageData, int BoundaryType, int IncType)
{
  if (!ImageData) {
    return 0;
    }
  int extent[6];
  ImageData->GetWholeExtent(extent);
  vtkIdType DataIncX, DataIncY, DataIncZ;
  ImageData->GetContinuousIncrements(extent, DataIncX, DataIncY, DataIncZ);
  if (BoundaryType) {
    int LengthOfXDim = extent[1] - extent[0] + 1 + DataIncY;

    if (IncType) {
      int LengthOfYDim = LengthOfXDim*(extent[3] - extent[2] + 1) + DataIncZ;  
      return (LengthOfYDim - this->DataDim[1] *LengthOfXDim);
    } else {
      return (LengthOfXDim - this->DataDim[0]);
    }
  } else {
    if (IncType) return DataIncZ;
    else return DataIncY;
  }
}


//------------------------------------------------------------------------------
int vtkImageEMLocalGenericClass::CheckInputImage(vtkImageData * inData, int DataTypeOrig, int num) {
  // Check if InData is defined 
  int inExt[6];
  vtkFloatingPointType DataSpacingNew[3];

  if (inData == 0) {
    vtkEMAddErrorMessage("CheckInputImage: Input "<< num << " must be specified.");
    return 1;
  }

  // Check for Data Type if correct : Remember ProbabilityData all has to be of the same data Type
  if (DataTypeOrig != inData->GetScalarType()) {
    vtkEMAddErrorMessage("CheckInputImage: Input "<< num << " has wrong data type ("<< inData->GetScalarType() << ") instead of " 
          << DataTypeOrig << "! Note: VTK_FLOAT="<< VTK_FLOAT <<", VTK_SHORT=" << VTK_SHORT );
    return 1;
  }


  // Check for dimenstion of InData
  // Could be easily fixed if needed 
  inData->GetWholeExtent(inExt);
  if ((inExt[1] != this->Extent[1]) || (inExt[0] != this->Extent[0]) || (inExt[3] != this->Extent[3]) || (inExt[2] != this->Extent[2]) || (inExt[5] != this->Extent[5]) || (inExt[4] != this->Extent[4])) {
    vtkEMAddErrorMessage("CheckInputImage: Extension of Input Image " << num << ", " << inExt[0] << "," << inExt[1] << "," << inExt[2] << "," << inExt[3] << "," << inExt[4] << "," << inExt[5] 
                  << "is not alligned with output image "  << this->Extent[0] << "," << this->Extent[1] << "," << this->Extent[2] << "," << this->Extent[3] << "," << this->Extent[4] << " " << this->Extent[5]);
    return 1;
  }
  if (inData->GetNumberOfScalarComponents() != 1) { 
    vtkEMAddErrorMessage("CheckInputImage: This filter assumes input to filter is defined with one scalar component. Input Image "<< num  << " has " 
                          << inData->GetNumberOfScalarComponents() << " Can be easily changed !");
    return 1;
  }
  inData->GetSpacing(DataSpacingNew);
  if ((this->DataSpacing[0] !=  float(DataSpacingNew[0])) || (this->DataSpacing[1] !=  float(DataSpacingNew[1])) || (this->DataSpacing[2] !=  float(DataSpacingNew[2]))) {
    vtkEMAddErrorMessage("CheckInputImage: Data Spacing of input images is unequal" );
    cout << this->DataSpacing[0] << " " << DataSpacingNew[0] << " + " << this->DataSpacing[1] << " " << DataSpacingNew[1] << " + " << this->DataSpacing[2] << " " << DataSpacingNew[2] << endl;
    return 1;
  }
  // Kilian Check for orientation has to be done in TCL !!!!
  return 0;
}

// -----------------------------------------------------
// 1.) Check and Assign Probability Maps
// -----------------------------------------------------

int vtkImageEMLocalGenericClass::CheckAndAssignProbImageData(vtkImageData *inData) {
  if (this->CheckInputImage(inData, inData->GetScalarType(), 1)) return 0;
  this->ProbImageData  = inData;
  this->ProbDataScalarType  = inData->GetScalarType();
  return 1;
}



//----------------------------------------------------------------------------
void  vtkImageEMLocalGenericClass::ExecuteData(vtkDataObject*) {
    // Check All Values that are defined here 
    // cout << "vtkImageEMGenericClass::ExecuteData" << endl; 

    // -----------------------------------------------------
    // Check Generic Class Setting 
    this->vtkImageEMGenericClass::ExecuteData(NULL);

    // -----------------------------------------------------    
    // Define General ImageDataSettings
    // First input (input[0]) is a fake 
    int NumberOfRealInputData = this->vtkProcessObject::GetNumberOfInputs() -1;
    if (NumberOfRealInputData == 0) { return; }  

    vtkImageData **inData  = (vtkImageData **) this->GetInputs();
    int FirstData = 1;
    while (FirstData <= NumberOfRealInputData && !inData[FirstData])  FirstData++;
    if (FirstData > NumberOfRealInputData) {
      // This error should not be possible 
      vtkEMAddErrorMessage("No image data defined as input even though vtkProcessObject::GetNumberOfInputs > 0 !");
      return;
    }
    inData[FirstData]->GetWholeExtent(this->Extent);

    this->DataDim[0] = ( this->SegmentationBoundaryMax[0] - this->SegmentationBoundaryMin[0] +1)*inData[FirstData]->GetNumberOfScalarComponents();
    this->DataDim[1] = this->SegmentationBoundaryMax[1] - this->SegmentationBoundaryMin[1] + 1; // this->Extent[3/2] = Relative Maximum/Minimum Y index  
    this->DataDim[2] = this->SegmentationBoundaryMax[2] - this->SegmentationBoundaryMin[2] + 1;

    if (!(this->DataDim[0] * this->DataDim[1] * this->DataDim[2])) {
      vtkEMAddErrorMessage("Input has no points!" );
      return;
    }

    this->DataSpacing[0] = (float)inData[FirstData]->GetSpacing()[0];
    this->DataSpacing[1] = (float)inData[FirstData]->GetSpacing()[1];
    this->DataSpacing[2] = (float)inData[FirstData]->GetSpacing()[2];

    // ================================================== 
    // Load Probabiity Data 
    if (inData[1]) {   

#if (EMVERBOSE)
      {
        // If ProbDataPtr points to weired values then do an Update of the orignal source of the data before assinging it to this class 
        // e.g. vtkImageReader Blub ; Blub Update ; EMClass SetProbDataPtr [Blub GetOutput]
        cout << "------------------ ProbabilityData ------------------------- " << endl;
        vtkIndent indent;
        inData[1]->PrintSelf(cout,indent); 
      }
#endif
      if (this->ProbDataWeight > 0.0) {
        this->CheckAndAssignProbImageData(inData[1]);
      }
    }
}
