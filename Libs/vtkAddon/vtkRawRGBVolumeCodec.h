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

#ifndef __vtkRawRGBVolumeCodec_h
#define __vtkRawRGBVolumeCodec_h

// vtkAddon includes
#include "vtkStreamingVolumeCodec.h"

/// \brief Codec for storing imagedata in an 24-bit RGB format (8-bit color depth, no compression)
class VTK_ADDON_EXPORT vtkRawRGBVolumeCodec : public vtkStreamingVolumeCodec
{
public:
  static vtkRawRGBVolumeCodec *New();
  vtkStreamingVolumeCodec* CreateCodecInstance() override;
  vtkTypeMacro(vtkRawRGBVolumeCodec, vtkStreamingVolumeCodec);

  void PrintSelf(ostream& os, vtkIndent indent) override;

  // FourCC code representing 24-bit RGB using 8 bits per color
  std::string GetFourCC() override { return "RV24"; };

protected:
  vtkRawRGBVolumeCodec();
  ~vtkRawRGBVolumeCodec() override;

  /// Decode the compressed frame to an image
  bool DecodeFrameInternal(vtkStreamingVolumeFrame* inputFrame, vtkImageData* outputImageData, bool saveDecodedImage = true) override;

  /// Encode the image to a compressed frame
  bool EncodeImageDataInternal(vtkImageData* outputImageData, vtkStreamingVolumeFrame* inputFrame, bool forceKeyFrame) override;

  /// Update the codec parameters
  /// There are no parameters to update within this codec
  bool UpdateParameterInternal(std::string vtkNotUsed(parameterValue), std::string vtkNotUsed(parameterName)) override { return false; };

  /// Return the codec parameter description
  /// There are no parameters to update within this codec
  std::string GetParameterDescription(std::string vtkNotUsed(parameterName)) override { return ""; };

private:
  vtkRawRGBVolumeCodec(const vtkRawRGBVolumeCodec&) = delete;
  void operator=(const vtkRawRGBVolumeCodec&) = delete;
};

#endif
