/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See Doc/copyright/copyright.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkImageEMGenericClass.cxx,v $
  Date:      $Date: 2006/12/08 23:28:24 $
  Version:   $Revision: 1.1 $

=========================================================================auto=*/
#include "vtkImageEMGenericClass.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"

//------------------------------------------------------------------------------
// Define Procedures for ProtocolMessage
//------------------------------------------------------------------------------
char* ProtocolMessages::GetMessages() {
  if (this->Message) return this->Message->str();
  else return NULL;
}

// I did not know how to differently flush the stream;
void ProtocolMessages::ResetParameters() {
  this->DeleteMessage();
  this->Message = new vtkOStrStreamWrapper;
}

void ProtocolMessages::AddMessage(const char* os) {
  vtkOStreamWrapper::EndlType endl;
  vtkOStreamWrapper::UseEndl(endl);  
  this->Message->rdbuf()->freeze(0);                   
  (*this->Message) << os << "\n";             
  this->Flag =  1;
}

void ProtocolMessages::DeleteMessage() {    
  if (this->Message) {
    this->Message->rdbuf()->freeze(0);
    delete this->Message;
  }
  // I did not know how to differently flush the stream;
  this->Flag = 0;
}

//------------------------------------------------------------------------
vtkImageEMGenericClass* vtkImageEMGenericClass::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageEMGenericClass");
  if(ret)
  {
    return (vtkImageEMGenericClass*)ret;
  }
  // If the factory was unable to create the object, then create it here.
  return new vtkImageEMGenericClass;
}

//----------------------------------------------------------------------------
vtkImageEMGenericClass::vtkImageEMGenericClass()
{
    this->Label               = -1;
    this->TissueProbability   = -1;
    this->NumInputImages      = 0;  
    this->InputChannelWeights = NULL;
    this->ProbDataWeight      = 0.0;

    this->ProbDataScalarType = -1; // This means no prob data is defined

    memset(this->DataDim,0,sizeof(int)*3);
    memset(this->DataSpacing,0,sizeof(float)*3);

    for (int i=0; i < 3; i++) {
      this->SegmentationBoundaryMin[i] = -1;    // Lower bound of the boundary box where the image gets segments.
      this->SegmentationBoundaryMax[i] = -1;    // Upper bound of the boundary box where the image gets segments.
    }

    // Set the filter so that ther is no output
    // Do not remove output otherwise the filter does not call the updat function  
    // this->vtkSource::RemoveOutput(this->Outputs[0]);

    // This is Just Created so the Update pipeline works properly 
    vtkImageData* UpdateBlubber = vtkImageData::New();
    UpdateBlubber->SetExtent(0,0,0,0,0,0);
    UpdateBlubber->AllocateScalars();
    this->SetInput(0,UpdateBlubber);
    UpdateBlubber->Delete();
    this->PrintWeights        = 0;
 }

//----------------------------------------------------------------------------
void vtkImageEMGenericClass::PrintSelf(ostream& os,vtkIndent indent) {
  // this->vtkImageMultipleInputFilter::PrintSelf(os,indent);
  os << indent << "Label:                   " << this->Label << endl;
  os << indent << "NumInputImages:          " << this->NumInputImages << endl;
  os << indent << "Tissue Probability:      " << this->TissueProbability <<endl;
  os << indent << "ProbDataWeight:          " << this->ProbDataWeight << endl;
  os << indent << "ProbDataScalarType:      " << this->ProbDataScalarType << endl;
  os << indent << "InputChannelWeights:     ";
  for (int x= 0 ; x < this->NumInputImages; x ++) os << this->InputChannelWeights[x] << " ";
  os<< endl;
  os << indent << "DataSpacing:             " << this->DataSpacing[0] << " " << this->DataSpacing[1] << " " << this->DataSpacing[2] << endl;
  os << indent << "DataDim:                 " << this->DataDim[0] << " " << this->DataDim[1] << " " << this->DataDim[2] << endl;
  os << indent << "SegmentationBoundaryMin: " << this->SegmentationBoundaryMin[0] << " " << this->SegmentationBoundaryMin[1] << " " << this->SegmentationBoundaryMin[2] << endl;
  os << indent << "SegmentationBoundaryMax: " << this->SegmentationBoundaryMax[0] << " " << this->SegmentationBoundaryMax[1] << " " << this->SegmentationBoundaryMax[2] << endl;
  os << indent << "ErrorFlag:               " << this->GetErrorFlag() << endl;
  if (this->GetErrorFlag()) {
    os << indent << "ErrorMessage:          " << endl;
    os << indent << this->GetErrorMessages();
  }
  os << indent << "WarningFlag:             " << this->GetWarningFlag() << endl;
  if (this->GetWarningFlag()) {
    os << indent << "WarningMessage:        " << endl;
    os << indent << this->GetWarningMessages();
  }
  os << indent << "PrintWeights:             " << this->PrintWeights << endl;
}

//----------------------------------------------------------------------------
void vtkImageEMGenericClass::DeleteVariables() {
    if (this->InputChannelWeights) delete[] this->InputChannelWeights; 
    this->InputChannelWeights = NULL;
}

//----------------------------------------------------------------------------
void vtkImageEMGenericClass::SetInputChannelWeights(float val, int x){
  if ((x<0) || (x >= this->NumInputImages) || (val < 0) || (val > 1.0)) {
    vtkEMAddErrorMessage("Error:EMClass::SetInputChannelWeights: Incorrect input");
    return;
  }
  this->InputChannelWeights[x] = val;
}

//----------------------------------------------------------------------------
void vtkImageEMGenericClass::SetNumInputImages(int number) {
    if (this->NumInputImages == number) return;
    this->DeleteVariables();
    if (number > 0 ) {
       this->InputChannelWeights = new float[number];
       for (int z= 0; z < number; z++) this->InputChannelWeights[z] = 1.0;
    }
    this->NumInputImages = number;  
}

//----------------------------------------------------------------------------
void  vtkImageEMGenericClass::ExecuteData(vtkDataObject*) {
    // Check All Values that are defined here 
    // std::cerr << "vtkImageEMGenericClass::ExecuteData" << endl;

    // This must be performed in the subclasses  
    // this->ResetErrorMessage();

    // This is needed otherwise the Time Stamp function is not properly updated and an Update will always eexecute the following command !
    { 
        int outExt[6];
        // The next Function sets the  outData->WholeExtent properly - why do not ask me 
        vtkNotUsed(vtkImageData **inData  = (vtkImageData **) this->GetInputs(););
    vtkImageData *outData = this->GetOutput();
    outData->GetWholeExtent(outExt);
        // Now set the extent like the whole extent 
    outData->SetExtent(outExt); 
        // Allocate memory 
    outData->AllocateScalars(); 
    }
    if (this->TissueProbability < 0) {
    vtkEMAddErrorMessage("TissueProbability for class "<< this->Label <<" is not defined ");
    return;
    } 
    
    for (int i = 0; i < this->NumInputImages; i++) {
      if (this->InputChannelWeights[i] < 0 || this->InputChannelWeights[i] > 1) {
    vtkEMAddErrorMessage("InputChannelWeights for class "<< this->Label <<" and input "<<i <<" (" << this->InputChannelWeights[i] << ") is not defined correctly");
    return;
      }
    }

    if (this->ProbDataWeight < 0 || this->ProbDataWeight > 1) {
      vtkEMAddErrorMessage("ProbDataWeight for class "<< this->Label <<" (" << this->ProbDataWeight <<") is not defined correctly");
      return;
    }
    // Do not check labels bc super classes have labeles automatically assigned to them 
 }

char* vtkImageEMGenericClass::GetErrorMessages() {
  return this->ErrorMessage.GetMessages(); 
}


int vtkImageEMGenericClass::GetErrorFlag() {
   return  this->ErrorMessage.GetFlag();
}

void vtkImageEMGenericClass::ResetErrorMessage() {
  this->ErrorMessage.ResetParameters();
}
