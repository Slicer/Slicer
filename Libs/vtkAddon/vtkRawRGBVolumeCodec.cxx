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
and was supported through CANARIE's Research Software Program, and Cancer
Care Ontario.

==============================================================================*/

// vtkAddon includes
#include "vtkRawRGBVolumeCodec.h"

vtkCodecNewMacro(vtkRawRGBVolumeCodec);

//---------------------------------------------------------------------------
vtkRawRGBVolumeCodec::vtkRawRGBVolumeCodec()
{
}

//---------------------------------------------------------------------------
vtkRawRGBVolumeCodec::~vtkRawRGBVolumeCodec()
{
}

//---------------------------------------------------------------------------
bool vtkRawRGBVolumeCodec::DecodeFrameInternal(vtkStreamingVolumeFrame* inputFrame, vtkImageData* outputImageData, bool vtkNotUsed(saveDecodedImage))
{
  if (!inputFrame || !outputImageData)
    {
    vtkErrorMacro("Incorrect arguments!");
    return false;
    }

  if (inputFrame->GetVTKScalarType() != VTK_UNSIGNED_CHAR || inputFrame->GetNumberOfComponents() != 3)
    {
    vtkErrorMacro("Codec only supports encoding and decoding of 8-bit color images");
    }

  int frameDimensions[3] = { 0,0,0 };
  inputFrame->GetDimensions(frameDimensions);

  int imageDimensions[3] = { 0,0,0 };
  outputImageData->GetDimensions(imageDimensions);

  unsigned int numberOfVoxels = frameDimensions[0] * frameDimensions[1] * frameDimensions[2];
  if (numberOfVoxels == 0)
    {
    vtkErrorMacro("Cannot decode frame, number of voxels is zero");
    return false;
    }

  for (int i = 0; i < 3; ++i)
    {
    if (frameDimensions[i] != imageDimensions[i])
      {
      vtkErrorMacro("Cannot decode frame, voxel size does not match image");
      return false;
      }
    }

  vtkUnsignedCharArray* frameData = inputFrame->GetFrameData();
  void* framePointer = frameData->GetPointer(0);
  void* imagePointer = outputImageData->GetScalarPointer();

  unsigned int numberOfBytes = frameDimensions[0] * frameDimensions[1] * frameDimensions[2] * inputFrame->GetNumberOfComponents();
  memcpy(imagePointer, framePointer, numberOfBytes);

  return true;
}

//---------------------------------------------------------------------------
bool vtkRawRGBVolumeCodec::EncodeImageDataInternal(vtkImageData* inputImageData, vtkStreamingVolumeFrame* outputFrame, bool vtkNotUsed(forceKeyFrame))
{
  if (!inputImageData || !outputFrame)
    {
    vtkErrorMacro("Incorrect arguments!");
    return false;
    }

  if (inputImageData->GetScalarType() != VTK_UNSIGNED_CHAR || inputImageData->GetNumberOfScalarComponents() != 3)
    {
    vtkErrorMacro("Codec only supports encoding and decoding of 8-bit color images");
    }

  void* imagePointer = inputImageData->GetScalarPointer();

  int dimensions[3] = { 0,0,0 };
  inputImageData->GetDimensions(dimensions);
  unsigned int numberOfVoxels = dimensions[0] * dimensions[1] * dimensions[2];
  if (numberOfVoxels == 0)
    {
    vtkErrorMacro("Cannot decode frame, number of voxels is zero");
    return false;
    }

  unsigned int numberOfBytes = numberOfVoxels * inputImageData->GetNumberOfScalarComponents();

  vtkSmartPointer<vtkUnsignedCharArray> frameData = vtkSmartPointer<vtkUnsignedCharArray>::New();
  frameData->Allocate(numberOfBytes);

  void* framePointer = frameData->GetPointer(0);
  memcpy(framePointer, imagePointer, numberOfBytes);

  outputFrame->SetFrameData(frameData);
  outputFrame->SetVTKScalarType(VTK_UNSIGNED_CHAR);
  outputFrame->SetDimensions(dimensions);
  outputFrame->SetNumberOfComponents(inputImageData->GetNumberOfScalarComponents());
  outputFrame->SetFrameType(vtkStreamingVolumeFrame::IFrame);
  outputFrame->SetCodecFourCC(this->GetFourCC());
  outputFrame->SetPreviousFrame(NULL);

  return true;
}

//---------------------------------------------------------------------------
void vtkRawRGBVolumeCodec::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
}
