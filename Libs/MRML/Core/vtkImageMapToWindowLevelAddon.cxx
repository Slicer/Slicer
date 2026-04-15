#include "vtkImageMapToWindowLevelAddon.h"

#include "vtkDataArray.h"
#include "vtkImageData.h"
#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkPointData.h"
#include "vtkScalarsToColors.h"

#include <cmath>

vtkStandardNewMacro(vtkImageMapToWindowLevelAddon);

// Constructor sets default values
vtkImageMapToWindowLevelAddon::vtkImageMapToWindowLevelAddon()
{
  this->MappingMode = WindowMappingMode::Linear;
}

vtkImageMapToWindowLevelAddon::~vtkImageMapToWindowLevelAddon() = default;

//------------------------------------------------------------------------------
// This method checks to see if we can simply reference the input data
int vtkImageMapToWindowLevelAddon::RequestData(vtkInformation* request, vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* outInfo = outputVector->GetInformationObject(0);
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);

  vtkImageData* outData = vtkImageData::SafeDownCast(outInfo->Get(vtkDataObject::DATA_OBJECT()));
  vtkImageData* inData = vtkImageData::SafeDownCast(inInfo->Get(vtkDataObject::DATA_OBJECT()));

  // If LookupTable is null and window / level produces no change,
  // then just pass the data
  if (this->LookupTable == nullptr && this->MappingMode == WindowMappingMode::Linear
      && (inData->GetScalarType() == VTK_UNSIGNED_CHAR && this->Window == 255 && this->Level == 127.5))
  {
    vtkDebugMacro("ExecuteData: LookupTable not set, "
                  "Window / Level at default, "
                  "passing input to output.");

    outData->SetExtent(inData->GetExtent());
    outData->GetPointData()->PassData(inData->GetPointData());
    this->DataWasPassed = 1;
  }
  else
  // normal behaviour - skip up a level since we don't want to
  // call the superclasses ExecuteData - it would pass the data if there
  // is no lookup table even if there is a window / level - wrong
  // behavior.
  {
    if (this->DataWasPassed)
    {
      outData->GetPointData()->SetScalars(nullptr);
      this->DataWasPassed = 0;
    }

    // NOLINTNEXTLINE(bugprone-parent-virtual-call)
    return this->vtkThreadedImageAlgorithm::RequestData(request, inputVector, outputVector);
  }

  return 1;
}

//------------------------------------------------------------------------------
int vtkImageMapToWindowLevelAddon::RequestInformation(vtkInformation* vtkNotUsed(request), vtkInformationVector** inputVector, vtkInformationVector* outputVector)
{
  vtkInformation* inInfo = inputVector[0]->GetInformationObject(0);
  vtkInformation* outInfo = outputVector->GetInformationObject(0);

  vtkInformation* inScalarInfo = vtkDataObject::GetActiveFieldInformation(inInfo, vtkDataObject::FIELD_ASSOCIATION_POINTS, vtkDataSetAttributes::SCALARS);
  if (!inScalarInfo)
  {
    vtkErrorMacro("Missing scalar field on input information!");
    return 0;
  }

  // If LookupTable is null and window / level produces no change,
  // then the data will be passed
  if (this->LookupTable == nullptr && (inScalarInfo->Get(vtkDataObject::FIELD_ARRAY_TYPE()) == VTK_UNSIGNED_CHAR && this->Window == 255 && this->Level == 127.5))
  {
    if (inScalarInfo->Get(vtkDataObject::FIELD_ARRAY_TYPE()) != VTK_UNSIGNED_CHAR)
    {
      vtkErrorMacro("ExecuteInformation: No LookupTable was set and input data is not VTK_UNSIGNED_CHAR!");
    }
    else
    {
      // no lookup table, pass the input if it was VTK_UNSIGNED_CHAR
      vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, inScalarInfo->Get(vtkDataObject::FIELD_NUMBER_OF_COMPONENTS()));
    }
  }
  else // the lookup table was set or window / level produces a change
  {
    int numComponents = 4;
    switch (this->OutputFormat)
    {
      case VTK_RGBA: numComponents = 4; break;
      case VTK_RGB: numComponents = 3; break;
      case VTK_LUMINANCE_ALPHA: numComponents = 2; break;
      case VTK_LUMINANCE: numComponents = 1; break;
      default: vtkErrorMacro("ExecuteInformation: Unrecognized color format."); break;
    }
    vtkDataObject::SetPointDataActiveScalarInfo(outInfo, VTK_UNSIGNED_CHAR, numComponents);
  }

  return 1;
}

//------------------------------------------------------------------------------
// This non-templated function executes the filter for any type of data.
template <class T>
void vtkImageMapToWindowLevelAddonExecute(vtkImageMapToWindowLevelAddon* self, vtkImageData* inData, T* inPtr, vtkImageData* outData, unsigned char* outPtr, int outExt[6], int id)
{
  int idxX, idxY, idxZ;
  int extX, extY, extZ;
  vtkIdType inIncX, inIncY, inIncZ;
  vtkIdType outIncX, outIncY, outIncZ;
  unsigned long count = 0;
  unsigned long target;
  int dataType = inData->GetScalarType();
  int numberOfComponents, numberOfOutputComponents, outputFormat;
  int rowLength;
  vtkScalarsToColors* lookupTable = self->GetLookupTable();
  unsigned char* outPtr1;
  T* inPtr1;
  unsigned char* optr;
  T* iptr;

  double window = self->GetWindow();
  double level = self->GetLevel();
  T lower = static_cast<T>(level - fabs(window) / 2.0);
  T upper = lower + static_cast<T>(fabs(window));

  unsigned char result_val;
  auto mappingMode = self->GetMappingMode();
  double windowFactor;

  // find the region to loop over
  extX = outExt[1] - outExt[0] + 1;
  extY = outExt[3] - outExt[2] + 1;
  extZ = outExt[5] - outExt[4] + 1;

  target = static_cast<unsigned long>(extZ * extY / 50.0);
  target++;

  // Get increments to march through data
  inData->GetContinuousIncrements(outExt, inIncX, inIncY, inIncZ);

  outData->GetContinuousIncrements(outExt, outIncX, outIncY, outIncZ);
  numberOfComponents = inData->GetNumberOfScalarComponents();
  numberOfOutputComponents = outData->GetNumberOfScalarComponents();
  outputFormat = self->GetOutputFormat();

  rowLength = extX * numberOfComponents;

  // Loop through output pixels
  outPtr1 = outPtr;
  inPtr1 = inPtr + self->GetActiveComponent();
  for (idxZ = 0; idxZ < extZ; idxZ++)
  {
    for (idxY = 0; !self->AbortExecute && idxY < extY; idxY++)
    {
      if (!id)
      {
        if (!(count % target))
        {
          self->UpdateProgress(count / (50.0 * target));
        }
        count++;
      }

      iptr = inPtr1;
      optr = outPtr1;

      if (lookupTable)
      {
        lookupTable->MapScalarsThroughTable2(inPtr1, outPtr1, dataType, extX, numberOfComponents, outputFormat);
        for (idxX = 0; idxX < extX; idxX++)
        {
          windowFactor = vtkImageMapToWindowLevelAddon::mapScalarToWindow(*iptr, lower, upper, mappingMode);
          *optr = static_cast<unsigned char>(*optr * windowFactor);
          switch (outputFormat)
          {
            case VTK_RGBA:
              *(optr + 1) = static_cast<unsigned char>(*(optr + 1) * windowFactor);
              *(optr + 2) = static_cast<unsigned char>(*(optr + 2) * windowFactor);
              *(optr + 3) = 255;
              break;
            case VTK_RGB:
              *(optr + 1) = static_cast<unsigned char>(*(optr + 1) * windowFactor);
              *(optr + 2) = static_cast<unsigned char>(*(optr + 2) * windowFactor);
              break;
            case VTK_LUMINANCE_ALPHA: *(optr + 1) = 255; break;
          }
          iptr += numberOfComponents;
          optr += numberOfOutputComponents;
        }
      }
      else
      {
        for (idxX = 0; idxX < extX; idxX++)
        {
          result_val = static_cast<unsigned char>(255.0 * vtkImageMapToWindowLevelAddon::mapScalarToWindow(*iptr, lower, upper, mappingMode));
          *optr = result_val;
          switch (outputFormat)
          {
            case VTK_RGBA:
              *(optr + 1) = result_val;
              *(optr + 2) = result_val;
              *(optr + 3) = 255;
              break;
            case VTK_RGB:
              *(optr + 1) = result_val;
              *(optr + 2) = result_val;
              break;
            case VTK_LUMINANCE_ALPHA: *(optr + 1) = 255; break;
          }
          iptr += numberOfComponents;
          optr += numberOfOutputComponents;
        }
      }
      outPtr1 += outIncY + extX * numberOfOutputComponents;
      inPtr1 += inIncY + rowLength;
    }
    outPtr1 += outIncZ;
    inPtr1 += inIncZ;
  }
}

//------------------------------------------------------------------------------
// This method is passed a input and output data, and executes the filter
// algorithm to fill the output from the input.

void vtkImageMapToWindowLevelAddon::ThreadedRequestData(vtkInformation* vtkNotUsed(request),
                                                        vtkInformationVector** vtkNotUsed(inputVector),
                                                        vtkInformationVector* vtkNotUsed(outputVector),
                                                        vtkImageData*** inData,
                                                        vtkImageData** outData,
                                                        int outExt[6],
                                                        int id)
{
  void* inPtr = inData[0][0]->GetScalarPointerForExtent(outExt);
  void* outPtr = outData[0]->GetScalarPointerForExtent(outExt);

  switch (inData[0][0]->GetScalarType())
  {
    vtkTemplateMacro(vtkImageMapToWindowLevelAddonExecute(this, inData[0][0], static_cast<VTK_TT*>(inPtr), outData[0], static_cast<unsigned char*>(outPtr), outExt, id));
    default: vtkErrorMacro(<< "Execute: Unknown ScalarType"); return;
  }
}

void vtkImageMapToWindowLevelAddon::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "WindowMappingMode: " << this->MappingMode << endl;
}

// --------------------------------------------------------------------------
vtkImageMapToWindowLevelAddon::WindowMappingMode vtkImageMapToWindowLevelAddon::GetMappingMode() const
{
  return this->MappingMode;
}

// --------------------------------------------------------------------------
void vtkImageMapToWindowLevelAddon::SetMappingMode(vtkImageMapToWindowLevelAddon::WindowMappingMode mappingMode)
{
  this->MappingMode = mappingMode;
}
