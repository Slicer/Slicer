/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMAtlasClass.cxx,v $
  Date:      $Date: 2006/12/15 19:11:06 $
  Version:   $Revision: 1.2 $

=========================================================================auto=*/
#include "vtkImageEMAtlasClass.h"
#include "vtkImageEMGeneral.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

//------------------------------------------------------------------------
vtkImageEMAtlasClass* vtkImageEMAtlasClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMAtlasClass");
  if(ret)
  {
    return (vtkImageEMAtlasClass*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMAtlasClass;

}


//----------------------------------------------------------------------------
vtkImageEMAtlasClass::vtkImageEMAtlasClass() { 
  this->LogMu               = NULL;
  this->LogCovariance       = NULL;

  this->ProbDataPtr         = NULL;
  this->ProbDataIncY        = -1; 
  this->ProbDataIncZ        = -1; 
  this->ProbDataWeight      = 0.0;
  
  this->ReferenceStandardPtr= NULL;

  // Printing functions 
  this->PrintQuality        = 0;

  // Important to set it - otherwise set to 1 by default  
  // I disabled it bc if no image data is attached to this class 
  // => NumberOfInputs = 0 => Update is disabled.
  // this->vtkProcessObject::SetNumberOfInputs(0); 

}


//------------------------------------------------------------------------------
void vtkImageEMAtlasClass::DeleteClassVariables(){
  if (this->LogCovariance) {
    for (int y=0; y < this->NumInputImages; y++) {
      delete[] this->LogCovariance[y];
    }
    delete[] this->LogCovariance;
  }
  
  if (this->LogMu) delete[] this->LogMu;
  
  this->LogCovariance       = NULL;
  this->LogMu               = NULL;

  this->ProbDataPtr         = NULL;
  this->ReferenceStandardPtr= NULL;
}

//----------------------------------------------------------------------------
void vtkImageEMAtlasClass::PrintSelf(ostream& os,vtkIndent indent) { 
  int x,y;
  os << indent << "------------------------------------------ CLASS ----------------------------------------------" << endl;
  this->vtkImageEMGenericClass::PrintSelf(os,indent);  
  os << indent << "ProbDataPtr:             " << this->ProbDataPtr << endl;
  os << indent << "ProbDataIncY:            " << this->ProbDataIncY << endl;
  os << indent << "ProbDataIncZ:            " << this->ProbDataIncZ << endl;

  os << indent << "LogMu:                   ";
  for (x= 0 ; x < this->NumInputImages; x ++) os << this->LogMu[x] << " ";
  os<< endl;

  os << indent << "LogCovariance:           ";
  for (y= 0 ; y < this->NumInputImages; y ++) {
     for (x= 0; x < this->NumInputImages; x++)  os << this->LogCovariance[y][x] << " " ;
     if ( y < (this->NumInputImages-1)) os<< "| ";
  }
  os<< endl;

  os << indent << "ReferenceStandardPtr:    ";
  if (this->ReferenceStandardPtr) {
     os << this->ReferenceStandardPtr << endl;
  } else {os << "(None)" << endl;}

  os << indent << "PrintQuality:            " << this->PrintQuality << endl;  
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasClass::SetNumInputImages(int number) {
  int OldNumber = this->NumInputImages;
  this->vtkImageEMGenericClass::SetNumInputImages(number);
  if (OldNumber == number) return;
  this->DeleteClassVariables();

  if (number > 0 ) {
    // Create new space for variables
    this->LogMu               = new double[number];
    this->LogCovariance       = new double*[number];
    int z;
    for (z=0; z < number; z++) {
      this->LogCovariance[z] = new double[number];
    }
    for (z= 0; z < number; z++) {
    this->LogMu[z] = -1;
    memset(this->LogCovariance[z], 0,number*sizeof(double));
    }
  } 
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasClass::SetLogMu(double mu, int x){
  if ((x<0) || (x >= this->NumInputImages) || (mu < 0)) {
    vtkEMAddErrorMessage("Error:vtkImageEMAtlasClass::SetLogMu: Incorrect input");
    return;
  }
  this->LogMu[x] = mu;
}

//------------------------------------------------------------------------------
void vtkImageEMAtlasClass::SetLogCovariance(double value, int y, int x){
  if ((y<0) || (y >= this->NumInputImages) || (x<0) || (x >= this->NumInputImages)) {
    vtkEMAddErrorMessage("Error:vtkImageEMAtlasClass::SetLogCovariance: Incorrect input");
    return;
  }
  this->LogCovariance[y][x] = value;
}

//----------------------------------------------------------------------------
void vtkImageEMAtlasClass::SetPrintQuality(int init) { 
    if ( (init < 0)  || (init > EMSEGMENT_NUM_OF_QUALITY_MEASURE)) {
      vtkEMAddErrorMessage("The parameter for PrintQuality has be between 0 and "<< EMSEGMENT_NUM_OF_QUALITY_MEASURE - 1 << "!");
      return;
    }
    this->PrintQuality = init;
}

//----------------------------------------------------------------------------
template <class T>
static void vtkImageEMAtlasClassAssignProbDataPointer(vtkImageEMAtlasClass *self, T *inProbDataPtr, int jump) {
  self->SetProbDataPtr((void*) (inProbDataPtr + jump));
}

//----------------------------------------------------------------------------
// Definition of  DataTypeIndex 
// 1   = probability map

int vtkImageEMAtlasClass::CheckAndAssignImageData(vtkImageData *inData, int outExt[6]) {
  vtkIdType DataIncY, DataIncZ, blub;
 
  // -----------------------------------------------------
  // 1.) Check Probability Maps
  // -----------------------------------------------------
  
  int DataTypeIndex = 1; 
  if (this->CheckInputImage(inData, inData->GetScalarType(), DataTypeIndex,outExt)) {
    vtkEMAddErrorMessage("CheckAndAssignImageData: Error occured at class with Label " <<(int) this->Label);      
    return 0;
  } 
  inData->GetContinuousIncrements(outExt, blub, DataIncY, DataIncZ);
        
  int LengthOfXDim = outExt[1] - outExt[0] + 1 + DataIncY;
  int LengthOfYDim = LengthOfXDim*(outExt[3] - outExt[2] + 1) + DataIncZ;  
  int jump = (this->SegmentationBoundaryMin[0] - 1) + (this->SegmentationBoundaryMin[1] - 1) * LengthOfXDim
         + LengthOfYDim *(this->SegmentationBoundaryMin[2] - 1);

  int BoundaryDataIncY = LengthOfXDim - this->DataDim[0];
  int BoundaryDataIncZ = LengthOfYDim - this->DataDim[1] *LengthOfXDim;

  // =========================================
  // Assign Probability Data
  switch (inData->GetScalarType()) {
      vtkTemplateMacro3(vtkImageEMAtlasClassAssignProbDataPointer, this, (VTK_TT*) inData->GetScalarPointerForExtent(outExt), jump);
  default:
      vtkEMAddErrorMessage("Execute: Unknown ScalarType");
      return 0;
  }
  this->ProbDataIncY = BoundaryDataIncY;
  this->ProbDataIncZ = BoundaryDataIncZ;
  this->ProbDataScalarType  = inData->GetScalarType();

  return 1; 
}


//------------------------------------------------------------------------------
int vtkImageEMAtlasClass::CheckInputImage(vtkImageData * inData,int DataTypeOrig, int num, int outExt[6]) {
  // Check if InData is defined 
  int inExt[6];
// #if EM_VTK_OLD_SETTINGS
//   float DataSpacingNew[3];
// #else 
   vtkFloatingPointType DataSpacingNew[3];
// #endif

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
  if ((inExt[1] != outExt[1]) || (inExt[0] != outExt[0]) || (inExt[3] != outExt[3]) || (inExt[2] != outExt[2]) || (inExt[5] != outExt[5]) || (inExt[4] != outExt[4])) {
    vtkEMAddErrorMessage("CheckInputImage: Extension of Input Image " << num << ", " << inExt[0] << "," << inExt[1] << "," << inExt[2] << "," << inExt[3] << "," << inExt[4] << "," << inExt[5] 
                  << "is not alligned with output image "  << outExt[0] << "," << outExt[1] << "," << outExt[2] << "," << outExt[3] << "," << outExt[4] << " " << outExt[5]);
    return 1;
  }
  if (inData->GetNumberOfScalarComponents() != 1) { 
    vtkEMAddErrorMessage("CheckInputImage: This filter assumes input to filter is defined with one scalar component. Input Image "<< num  << " has " 
                          << inData->GetNumberOfScalarComponents() << " Can be easily changed !");
    return 1;
  }
  inData->GetSpacing(DataSpacingNew);
  if ((this->DataSpacing[0] !=  float(DataSpacingNew[0])) || (this->DataSpacing[1] !=  float(DataSpacingNew[1])) || (this->DataSpacing[2] !=  float(DataSpacingNew[2]))) {
    vtkEMAddErrorMessage("CheckInputImage: Data Spacing of input images is unequal ! Orig: " 
                         << this->DataSpacing[0] << " " << this->DataSpacing[1] << " " << this->DataSpacing[2] << " New: "  
                         << DataSpacingNew[0]    << " " << DataSpacingNew[1]    << " " << DataSpacingNew[2]);
    return 1;
  }
  // Kilian Check for orientation has to be done in TCL !!!!
  return 0;
}

//----------------------------------------------------------------------------
// This method is passed a input and output datas, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the datas data types.
void vtkImageEMAtlasClass::ExecuteData(vtkDataObject *)
{
  // cout << "Start vtkImageEMAtlasClass::ExecuteData " << endl;
   vtkDebugMacro(<<"ExecuteData()");

   // ==================================================
   // Initial Check Parameters
   this->ResetWarningMessage();

   if (this->GetErrorFlag()) 
     vtkEMAddWarningMessage("The error flag for this module was set with the following messages (the error messages will be reset now:\n"<<this->GetErrorMessages());

   this->ResetErrorMessage();
   this->vtkImageEMGenericClass::ExecuteData(NULL);
   if (this->GetErrorFlag()) return;

   if (this->Label < 0) {
     vtkEMAddErrorMessage("Not all labels are defined for the classes");
     return ;
   } 
   for (int j = 0; j < this->NumInputImages; j++) {
     // Check Mu
     if (this->LogMu[j] < 0) {
       vtkEMAddErrorMessage("Mu[" << j <<"] = " << this->LogMu[j] << " for class with label " << this->Label <<" must be greater than 0!");
       return;
     } 
     for (int k=j+1; k <   NumInputImages; k++) {
       if (this->LogCovariance[j][k] != this->LogCovariance[k][j]) {
     vtkEMAddErrorMessage("Covariance must be symetric for class with label " << this->Label);
     return;
       }
     }
   }

   // ==================================================
   // Define Parameters

   // First input (input[0]) is a fake 
   int NumberOfRealInputData = this->vtkProcessObject::GetNumberOfInputs() -1;

   // No inputs defined we do not need to do here anything
   if (NumberOfRealInputData == 0) {
     if (this->ProbDataWeight > 0.0)  vtkEMAddErrorMessage("ProbDataWeight > 0.0 and no Probability Map defined !" );
     return;

   }  
   // Redefine ImageRelatedClass Parameters   
   vtkImageData **inData  = (vtkImageData **) this->GetInputs();

   // Update all Input Data
   // That does not help 
   // for (int i = 1; i <=  NumberOfRealInputData; i++) if (inData[i]) inData[i]->Update();  
 
   int FirstData = 1;

   while (FirstData <= NumberOfRealInputData && !inData[FirstData])  FirstData++;
   if (FirstData > NumberOfRealInputData) {
    // This error should not be possible 
    vtkEMAddErrorMessage("No image data defined as input even though vtkProcessObject::GetNumberOfInputs > 0 !");
    return;
   }
 
   this->DataDim[0] = ( this->SegmentationBoundaryMax[0] - this->SegmentationBoundaryMin[0] +1)*inData[FirstData]->GetNumberOfScalarComponents();
   this->DataDim[1] = this->SegmentationBoundaryMax[1] - this->SegmentationBoundaryMin[1] + 1; // outExt[3/2] = Relative Maximum/Minimum Y index  
   this->DataDim[2] = this->SegmentationBoundaryMax[2] - this->SegmentationBoundaryMin[2] + 1;

   int Ext[6];
   inData[FirstData]->GetWholeExtent(Ext);
  
   //!!!bcd!!! int XDim= Ext[1] - Ext[0] + 1;
   //!!!bcd!!! int YDim= Ext[3] - Ext[2] + 1;

   if (!(this->DataDim[0] * this->DataDim[1] * this->DataDim[2])) {
      vtkEMAddErrorMessage("Input has no points!" );
      return;
   }

   this->DataSpacing[0] = (float) inData[FirstData]->GetSpacing()[0];
   this->DataSpacing[1] = (float) inData[FirstData]->GetSpacing()[1];
   this->DataSpacing[2] = (float) inData[FirstData]->GetSpacing()[2];

   // ================================================== 
   // Load the images
   
   // Check and set Probability map 
   if (NumberOfRealInputData && inData[1]) {

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
       if (!this->CheckAndAssignImageData(inData[1], Ext)) return;
     }
   } else {
     if (this->ProbDataWeight > 0.0) {
       vtkEMAddErrorMessage("ProbDataWeight > 0.0 but no Probability Map defined !" );
       return;
     } else {
       vtkEMAddWarningMessage("No probability map is defined for class with Label " << this->Label);
     }
   }
}

