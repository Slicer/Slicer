/*=========================================================================

  Program:   Slicer
  Language:  C++
  Module:    $HeadURL: http://svn.slicer.org/Slicer4/trunk/Applications/GUI/Slicer3.cxx $
  Date:      $Date: 2009-04-15 06:29:13 -0400 (Wed, 15 Apr 2009) $
  Version:   $Revision: 9206 $

  Copyright (c) Brigham and Women's Hospital (BWH) All Rights Reserved.

  See License.txt or http://www.slicer.org/copyright/copyright.txt for details.

==========================================================================*/
#include "vtkImageSumOverVoxels.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include <cassert>

//----------------------------------------------------------------------------
void vtkImageSumOverVoxels::ComputeInputUpdateExtent(int inExt[6], int vtkNotUsed(outExt)[6])
{
  this->GetInput()->GetWholeExtent(inExt);
}
 
//------------------------------------------------------------------------------
vtkImageSumOverVoxels* vtkImageSumOverVoxels::New()
{
  // First try to create the object from the vtkObjectFactory
  vtkObject* ret = vtkObjectFactory::CreateInstance("vtkImageSumOverVoxels");
  if(ret) return (vtkImageSumOverVoxels*)ret;
  // If the factory was unable to create the object, then create it here.
  return new vtkImageSumOverVoxels;
}


//----------------------------------------------------------------------------
void vtkImageSumOverVoxels::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}


template <class T> 
void VolumeFlipXAxis(T *invec, T *outvec, int XSize, int YSize, int ZSize, int XYSize) {
     for (int z = 0 ; z < ZSize; z++) {
       FlipXAxis(invec, outvec, XSize, YSize, XYSize);
       invec  += XYSize;
       outvec += XYSize;
     } 

}
//----------------------------------------------------------------------------
void vtkImageSumOverVoxels::ExecuteInformation(vtkImageData *inData, vtkImageData *outData) 
{
  // Currently filter is setup so input and output have the same dimension 
  // The code itself does not require it - they can have different dimension 
  // so just update it if you need it - just did not have time for it  
  outData->SetOrigin(inData->GetOrigin());
  outData->SetNumberOfScalarComponents(1);
  outData->SetWholeExtent(inData->GetWholeExtent());
  outData->SetSpacing(inData->GetSpacing());
  outData->SetScalarType(inData->GetScalarType());
}



template <class T>  
void AddVoxels(T* Ptr, vtkIdType inInc[3], int Dim[3], double & result) {
  result = 0.0;
  for (int z = 0; z < Dim[2]; z++) {
    for (int y = 0; y < Dim[1]; y++) {
      double resultSlice = 0.0;
      for (int x = 0; x < Dim[0]; x++) resultSlice += *Ptr++;
      result += resultSlice;
      Ptr += inInc[1];
    }
    Ptr += inInc[2];
  }   
} 

// --------------------------------------------------------------------------------------------------------------------------
// Currently does nothing here 
// this is baisically the VTK version of resamplerMain.c
void vtkImageSumOverVoxels::ExecuteData(vtkDataObject *) {
  void *inPtr;
  void *outPtr;

  int inExt[6];
  vtkIdType inInc[3];
  int outExt[6];
  vtkIdType outInc[3];

  this->ComputeInputUpdateExtent(inExt,outExt);
 // vtk4
  vtkImageData *inData  = this->GetInput();
  vtkImageData *outData = this->GetOutput();

  outData->SetExtent(this->GetInput()->GetWholeExtent());
  outData->AllocateScalars();
  outData->GetWholeExtent(outExt);

  // vtk4
  vtkDebugMacro(<< "Execute: inData = " << inData << ", outData = " << outData);
 
  if (inData == NULL) {
    vtkErrorMacro("Input " << 0 << " must be specified.");
    return;
  }
  if (inData->GetNumberOfScalarComponents() != 1) {
     vtkErrorMacro("Number Of Scalar Componentsfor Input has to be 1.");
     return;
  }
  inData->GetContinuousIncrements(inExt, inInc[0], inInc[1], inInc[2]);

  outData->GetContinuousIncrements(outExt, outInc[0], outInc[1], outInc[2]);
  if (!((inInc[0] == 0) && (inInc[1] == 0) && (inInc[2] == 0) && (outInc[0] == 0) && (outInc[1] == 0) && (outInc[2] == 0))) {
     vtkErrorMacro("Increments for input and output have to be 0!");
     return;
  }

  int Dim[3] = {inExt[1] - inExt[0] + 1, inExt[3] - inExt[2] + 1, inExt[5] - inExt[4] + 1};
  if (!(Dim[0]*Dim[1]*Dim[2])) {
    vtkErrorMacro("Execute: No voxels in the volumes !");
    return ;
  }

  inPtr = inData->GetScalarPointerForExtent(inExt);
  outPtr = outData->GetScalarPointerForExtent(outExt);

  // cout << inInc[0] << " " << inInc[1] << " " << inInc[2] << endl; 
  switch (inData->GetScalarType()) {
    vtkTemplateMacro(AddVoxels((VTK_TT*) inPtr, inInc, Dim, this->VoxelSum));
  default: 
      vtkErrorMacro("Execute: Unknown ScalarType");
      return;
  }
}








