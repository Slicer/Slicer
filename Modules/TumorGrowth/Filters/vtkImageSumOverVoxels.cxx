/*=auto=========================================================================

(c) Copyright 2003 Massachusetts Institute of Technology (MIT) All Rights Reserved.

This software ("3D Slicer") is provided by The Brigham and Women's 
Hospital, Inc. on behalf of the copyright holders and contributors.
Permission is hereby granted, without payment, to copy, modify, display 
and distribute this software and its documentation, if any, for  
research purposes only, provided that (1) the above copyright notice and 
the following four paragraphs appear on all copies of this software, and 
(2) that source code to any modifications to this software be made 
publicly available under terms no more restrictive than those in this 
License Agreement. Use of this software constitutes acceptance of these 
terms and conditions.

3D Slicer Software has not been reviewed or approved by the Food and 
Drug Administration, and is for non-clinical, IRB-approved Research Use 
Only.  In no event shall data or images generated through the use of 3D 
Slicer Software be used in the provision of patient care.

IN NO EVENT SHALL THE COPYRIGHT HOLDERS AND CONTRIBUTORS BE LIABLE TO 
ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL 
DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, 
EVEN IF THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE BEEN ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

THE COPYRIGHT HOLDERS AND CONTRIBUTORS SPECIFICALLY DISCLAIM ANY EXPRESS 
OR IMPLIED WARRANTIES INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND 
NON-INFRINGEMENT.

THE SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS 
IS." THE COPYRIGHT HOLDERS AND CONTRIBUTORS HAVE NO OBLIGATION TO 
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.


=========================================================================auto=*/
#include "vtkImageSumOverVoxels.h"
#include "vtkObjectFactory.h"
#include "vtkImageData.h"
#include <cassert>

//----------------------------------------------------------------------------
void vtkImageSumOverVoxels::ComputeInputUpdateExtent(int inExt[6], int outExt[6])
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
  if (!((inInc[0] == inInc[1] == inInc[2] == 0) && (outInc[0] == outInc[1] == outInc[2] == 0))) {
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








