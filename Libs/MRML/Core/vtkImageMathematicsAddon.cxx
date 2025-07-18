/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

#include "vtkImageMathematicsAddon.h"

// VTK includes
#include <vtkAlgorithmOutput.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkPointData.h>
#include <vtkStreamingDemandDrivenPipeline.h>

vtkStandardNewMacro(vtkImageMathematicsAddon);

//----------------------------------------------------------------------------
vtkImageMathematicsAddon::vtkImageMathematicsAddon() {}

//----------------------------------------------------------------------------
vtkImageMathematicsAddon::~vtkImageMathematicsAddon() {}

//----------------------------------------------------------------------------
void vtkImageMathematicsAddon::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//------------------------------------------------------------------------------
// The output extent is the intersection.
int vtkImageMathematicsAddon::RequestInformation(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  if (this->Operation != VTK_MULTIPLYBYSCALEDRANGE)
  {
    return this->Superclass::RequestInformation(request, inputVector, outputVector);
  }

  // get the info objects
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation* inInfo;

  int c, idx;
  int ext[6], unionExt[6];

  // Initialize the union.
  inInfo = inputVector[0]->GetInformationObject(0);
  inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), unionExt);

  for (c = 0; c < this->GetNumberOfInputConnections(0); ++c)
  {
    inInfo = inputVector[0]->GetInformationObject(c);
    inInfo->Get(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), ext);
    for (idx = 0; idx < 3; ++idx)
    {
      if (unionExt[idx * 2] > ext[idx * 2])
      {
        unionExt[idx * 2] = ext[idx * 2];
      }
      if (unionExt[idx * 2 + 1] < ext[idx * 2 + 1])
      {
        unionExt[idx * 2 + 1] = ext[idx * 2 + 1];
      }
    }
  }

  outInfo->Set(vtkStreamingDemandDrivenPipeline::WHOLE_EXTENT(), unionExt, 6);
  return 1;
}

//------------------------------------------------------------------------------
template <class T>
void vtkImageMathematicsAddonInitOutput(vtkImageData* inData, T* inPtr, vtkImageData* vtkNotUsed(outData), T* outPtr, int ext[6])
{
  int idxY, idxZ;
  int maxY, maxZ;
  vtkIdType outIncY, outIncZ;
  int rowLength;
  int typeSize;
  T *outPtrZ, *outPtrY;
  T *inPtrZ, *inPtrY;

  // This method needs to copy scalars from input to output for the update-extent.
  vtkDataArray* const inArray = inData->GetPointData()->GetScalars();
  typeSize = vtkDataArray::GetDataTypeSize(inArray->GetDataType());
  outPtrZ = outPtr;
  inPtrZ = inPtr;
  // Get increments to march through data
  vtkIdType increments[3];
  increments[0] = inArray->GetNumberOfComponents();
  increments[1] = increments[0] * (ext[1] - ext[0] + 1);
  increments[2] = increments[1] * (ext[3] - ext[2] + 1);
  outIncY = increments[1];
  outIncZ = increments[2];

  // Find the region to loop over
  rowLength = (ext[1] - ext[0] + 1) * inArray->GetNumberOfComponents();
  rowLength *= typeSize;
  maxY = ext[3] - ext[2];
  maxZ = ext[5] - ext[4];

  // Loop through input pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
  {
    outPtrY = outPtrZ;
    inPtrY = inPtrZ;
    for (idxY = 0; idxY <= maxY; idxY++)
    {
      memcpy(outPtrY, inPtrY, rowLength);
      outPtrY += outIncY;
      inPtrY += outIncY;
    }
    outPtrZ += outIncZ;
    inPtrZ += outIncZ;
  }
}

//------------------------------------------------------------------------------
// This templated function executes the filter for any type of data.
// Handles the two input operations
template <class T>
void vtkImageMathematicsAddonExecute2(vtkImageMathematicsAddon* self, vtkImageData* inData, T* inPtr, vtkImageData* outData, T* outPtr, int outExt[6], int id)
{
  int idxR, idxY, idxZ;
  int maxY, maxZ;
  vtkIdType inIncX, inIncY, inIncZ;
  vtkIdType outIncX, outIncY, outIncZ;
  int rowLength;
  unsigned long count = 0;
  unsigned long target;
  const int op = self->GetOperation();

  double normalizeScalarRange[2] = { 0.0, 1.0 };
  self->GetRange(normalizeScalarRange);
  const double normalizeMagnitude = normalizeScalarRange[1] - normalizeScalarRange[0];

  // find the region to loop over
  rowLength = (outExt[1] - outExt[0] + 1) * inData->GetNumberOfScalarComponents();

  maxY = outExt[3] - outExt[2];
  maxZ = outExt[5] - outExt[4];
  target = static_cast<unsigned long>((maxZ + 1) * (maxY + 1) / 50.0);
  target++;

  // Get increments to march through data
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);
  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);

  if (op != VTK_MULTIPLYBYSCALEDRANGE)
  {
    vtkErrorWithObjectMacro(self, << "Execute: Unknown operation: " << op);
    return;
  }

  if (normalizeMagnitude == 0.0)
  {
    vtkErrorWithObjectMacro(self, << "Execute: normalizeScalarRange width is zero");
    return;
  }

  // Loop through output pixels
  for (idxZ = 0; idxZ <= maxZ; idxZ++)
  {
    for (idxY = 0; !self->AbortExecute && idxY <= maxY; idxY++)
    {
      if (!id)
      {
        if (!(count % target))
        {
          self->UpdateProgress(count / (50.0 * target));
        }
        count++;
      }
      for (idxR = 0; idxR < rowLength; idxR++)
      {
        // The formula interpolates between the input value (*inPtr) and the lower bound of the scalar range
        // (normalizeScalarRange[0]), weighted by the normalized value.
        const float normalizedValue = static_cast<float>((*outPtr - normalizeScalarRange[0]) / normalizeMagnitude);
        *outPtr = normalizedValue * (*inPtr) + (1.0 - normalizedValue) * normalizeScalarRange[0];
        outPtr++;
        inPtr++;
      }
      outPtr += outIncY;
      inPtr += inIncY;
    }
    outPtr += outIncZ;
    inPtr += inIncZ;
  }
}

//------------------------------------------------------------------------------
// This method is passed an input and output data, and executes the filter
// algorithm to fill the output from the inputs.
// It just executes a switch statement to call the correct function for
// the data types.
void vtkImageMathematicsAddon::ThreadedRequestData(vtkInformation* request,
                                                   vtkInformationVector** inputVector,
                                                   vtkInformationVector* outputVector,
                                                   vtkImageData*** inData,
                                                   vtkImageData** outData,
                                                   int outExt[6],
                                                   int id)
{
  if (this->Operation != VTK_MULTIPLYBYSCALEDRANGE)
  {
    Superclass::ThreadedRequestData(request, inputVector, outputVector, inData, outData, outExt, id);
    return;
  }

  void* inPtr1;
  void* outPtr;

  outPtr = outData[0]->GetScalarPointerForExtent(outExt);

  for (int idx1 = 0; idx1 < this->GetNumberOfInputConnections(0); ++idx1)
  {
    inPtr1 = inData[0][idx1]->GetScalarPointerForExtent(outExt);
    if (idx1 == 0)
    {
      switch (inData[0][idx1]->GetScalarType())
      {
        vtkTemplateMacro(vtkImageMathematicsAddonInitOutput(inData[0][idx1], static_cast<VTK_TT*>(inPtr1), outData[0], static_cast<VTK_TT*>(outPtr), outExt));
        default: vtkErrorMacro(<< "InitOutput: Unknown ScalarType"); return;
      }
    }
    else
    {
      switch (inData[0][idx1]->GetScalarType())
      {
        vtkTemplateMacro(vtkImageMathematicsAddonExecute2(this, inData[0][idx1], static_cast<VTK_TT*>(inPtr1), outData[0], static_cast<VTK_TT*>(outPtr), outExt, id));
        default: vtkErrorMacro(<< "Execute: Unknown ScalarType"); return;
      }
    }
  }
}
