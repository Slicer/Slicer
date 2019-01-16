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

#ifndef __vtkStreamingVolumeFrame_h
#define __vtkStreamingVolumeFrame_h

// VTK includes
#include <vtkObject.h>
#include <vtkUnsignedCharArray.h>

// vtkAddon includes
#include "vtkAddon.h"

/// \brief VTK object containing a single compressed frame
class VTK_ADDON_EXPORT vtkStreamingVolumeFrame : public vtkObject
{
public:

  static vtkStreamingVolumeFrame* New();
  vtkTypeMacro(vtkStreamingVolumeFrame, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Enum for frame type
  /// For more information on frame types see: https://en.wikipedia.org/wiki/Video_compression_picture_types
  enum
  {
    IFrame, ///< Uninterpolated keyframe
    PFrame, ///< Frame interpolated from previous frames
    BFrame, ///< Frame interpolated from previous and forward frames
  };

  /// Reflects the type of the frame (I-Frame, P-Frame, B-Frame)
  vtkSetMacro(FrameType, int);
  vtkGetMacro(FrameType, int);

  /// Pointer to the contents of the frame in a compressed codec format
  void SetFrameData(vtkUnsignedCharArray* frameData);
  vtkUnsignedCharArray* GetFrameData() { return this->FrameData; };

  /// Pointer to the last frame that must be decoded before this one
  /// The pointer of each frame to the previous frame forms a linked list back to the originating keyframe
  /// this ensures that each frame provides access the information neccesary to be able to decode it.
  /// PreviousFrame does not refer to the frame that should be displayed before the this frame,
  /// but the frame that should be decoded immediately before this frame
  void SetPreviousFrame(vtkStreamingVolumeFrame* previousFrame);
  vtkStreamingVolumeFrame* GetPreviousFrame() { return this->PreviousFrame; };

  /// Dimensions of the decoded frame
  vtkSetVector3Macro(Dimensions, int);
  vtkGetVector3Macro(Dimensions, int);

  /// Number of components for the decoded image
  vtkSetMacro(NumberOfComponents, int);
  vtkGetMacro(NumberOfComponents, int);

  /// VTK scalar type of the decoded image
  vtkSetMacro(VTKScalarType, int);
  vtkGetMacro(VTKScalarType, int);

  /// FourCC of the codec for the frame
  vtkSetMacro(CodecFourCC, std::string);
  vtkGetMacro(CodecFourCC, std::string);

  /// Returns true if the frame is a "Keyframe", aka "I-Frame"
  bool IsKeyFrame() { return this->FrameType == IFrame; };

protected:
  int                                         Dimensions[3];
  std::string                                 CodecFourCC;
  vtkSmartPointer<vtkUnsignedCharArray>       FrameData;
  int                                         FrameType;
  int                                         NumberOfComponents;
  vtkSmartPointer<vtkStreamingVolumeFrame>    PreviousFrame;
  int                                         VTKScalarType;

protected:
  vtkStreamingVolumeFrame();
  ~vtkStreamingVolumeFrame();

private:
  vtkStreamingVolumeFrame(const vtkStreamingVolumeFrame&);
  void operator=(const vtkStreamingVolumeFrame&);

};
#endif
