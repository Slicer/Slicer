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
  virtual vtkStreamingVolumeCodec* CreateCodecInstance() VTK_OVERRIDE;
  vtkTypeMacro(vtkRawRGBVolumeCodec, vtkStreamingVolumeCodec);

  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  // FourCC code representing 24-bit RGB using 8 bits per color
  virtual std::string GetFourCC() VTK_OVERRIDE { return "RV24"; };

protected:
  vtkRawRGBVolumeCodec();
  ~vtkRawRGBVolumeCodec();

  /// Decode the compressed frame to an image
  virtual bool DecodeFrameInternal(vtkStreamingVolumeFrame* inputFrame, vtkImageData* outputImageData, bool saveDecodedImage = true) VTK_OVERRIDE;

  /// Encode the image to a compressed frame
  virtual bool EncodeImageDataInternal(vtkImageData* outputImageData, vtkStreamingVolumeFrame* inputFrame, bool forceKeyFrame) VTK_OVERRIDE;

  /// Update the codec parameters
  /// There are no parameters to update within this codec
  virtual bool UpdateParameterInternal(std::string vtkNotUsed(parameterValue), std::string vtkNotUsed(parameterName)) VTK_OVERRIDE { return false; };

  /// Return the codec parameter description
  /// There are no parameters to update within this codec
  virtual std::string GetParameterDescription(std::string vtkNotUsed(parameterName)) { return ""; };

private:
  vtkRawRGBVolumeCodec(const vtkRawRGBVolumeCodec&);
  void operator=(const vtkRawRGBVolumeCodec&);
};

#endif
