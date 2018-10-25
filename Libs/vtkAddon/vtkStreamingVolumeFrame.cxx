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
#include "vtkStreamingVolumeFrame.h"

// VTK includes
#include <vtkObjectFactory.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkStreamingVolumeFrame);

//---------------------------------------------------------------------------
vtkStreamingVolumeFrame::vtkStreamingVolumeFrame()
  : FrameData(NULL)
  , FrameType(vtkStreamingVolumeFrame::PFrame)
  , NumberOfComponents(3)
  , PreviousFrame(NULL)
  , VTKScalarType(VTK_UNSIGNED_CHAR)
{
  this->Dimensions[0] = 0;
  this->Dimensions[1] = 0;
  this->Dimensions[2] = 0;
}

//---------------------------------------------------------------------------
vtkStreamingVolumeFrame::~vtkStreamingVolumeFrame()
{
}

//---------------------------------------------------------------------------
void vtkStreamingVolumeFrame::SetFrameData(vtkUnsignedCharArray* frameData)
{
  this->FrameData = frameData;
  this->Modified();
};

//---------------------------------------------------------------------------
void vtkStreamingVolumeFrame::SetPreviousFrame(vtkStreamingVolumeFrame* previousFrame)
{
  this->PreviousFrame = previousFrame;
  this->Modified();
};

//---------------------------------------------------------------------------
void vtkStreamingVolumeFrame::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << "CodecFourCC: " << CodecFourCC << "\n";
  os << "FrameType: " << this->FrameType << "\n";
  os << "Dimensions: [" << this->Dimensions[0] << this->Dimensions[1] << this->Dimensions[2] << "]\n";
  os << "NumberOfComponents: " << this->NumberOfComponents << "\n";
  os << "VTKScalarType: " << this->VTKScalarType << "\n";
  os << "CurrentFrame: " << this->FrameData << "\n";
  os << "PreviousFrame: " << this->PreviousFrame << "\n";
}
