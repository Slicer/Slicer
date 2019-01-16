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

#ifndef __vtkStreamingVolumeCodec_h
#define __vtkStreamingVolumeCodec_h

// vtkAddon includes
#include "vtkAddon.h"
#include "vtkAddonSetGet.h"
#include "vtkStreamingVolumeFrame.h"

// VTK includes
#include <vtkImageData.h>
#include <vtkObject.h>
#include <vtkUnsignedCharArray.h>

// STD includes
#include <map>

#ifndef vtkCodecNewMacro
#define vtkCodecNewMacro(newClass) \
vtkStandardNewMacro(newClass); \
vtkStreamingVolumeCodec* newClass::CreateCodecInstance() \
{ \
return newClass::New(); \
}
#endif

/// \brief VTK object for representing a volume compression codec (normally a video compression codec)
class VTK_ADDON_EXPORT vtkStreamingVolumeCodec : public vtkObject
{
public:
  vtkTypeMacro(vtkStreamingVolumeCodec, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Returns the FourCC code representing the codec
  /// See https://www.fourcc.org/codecs.php for an incomplete list
  virtual std::string GetFourCC() = 0;

  /// Creates an instance of the codec
  // This can be overridden using vtkCodecNewMacro(className);
  virtual vtkStreamingVolumeCodec* CreateCodecInstance() = 0;

  /// Decode compressed frame data and stores it in the imagedata
  /// Handles the decoding of additional previous frames if required
  /// \param frame Input frame containing the compressed frame data
  /// \param outputImageData Output image which will store the uncompressed image
  /// Returns true if the frame is decoded successfully
  virtual bool DecodeFrame(vtkStreamingVolumeFrame* frame, vtkImageData* outputImageData);

  /// Encode the image data and store it in the frame
  /// \param inputImageData Input image containing the uncompressed image
  /// \param outputStreamingFrame Output frame that will be used to store the compressed frame
  /// \param forceKeyFrame If the codec supports it, attempt to encode the image as a keyframe
  /// Returns true if the image is encoded successfully
  virtual bool EncodeImageData(vtkImageData* inputImageData, vtkStreamingVolumeFrame* outputStreamingFrame, bool forceKeyFrame=false);

  /// Read this codec's information from a string representation
  /// Format is "ParameterName1:ParameterValue1;ParameterName2;ParameterValue2;ParameterNameN:ParameterValueN"
  /// \sa GetParametersAsString()
  virtual void SetParametersFromString(std::string parameterString);

  /// Write this codec's information to a string representation
  /// Format is "ParameterName1:ParameterValue1;ParameterName2;ParameterValue2;ParameterNameN:ParameterValueN"
  /// \sa SetParametersFromString()
  virtual std::string GetParametersAsString();

  enum
  {
    ParameterModifiedEvent = 18003 ///< Event invoked when a codec parameter is changed
  };

  /// Returns a list of availiable parameter names for the codec
  vtkGetStdVectorMacro(AvailiableParameterNames, std::vector<std::string>);

  /// Get parameter description as a string
  /// \param parameterName String containing the name of the parameter
  /// Returns the description of the parameter as a string, and returns an empty string if the parameter name is invalid
  virtual std::string GetParameterDescription(std::string parameterName) = 0;

  /// Set a parameter for the codec
  /// \param parameterName String containing the name of the parameter
  /// \param parameterValue Value of the specified parameter
  /// Returns true if the parameter is successfully set
  virtual bool SetParameter(std::string parameterName, std::string parameterValue);

  /// Get a parameter for the codec
  /// \param parameterName String containing the name of the parameter
  /// \param parameterValue Value of the specified parameter
  /// Returns true if the parameter was found
  virtual bool GetParameter(std::string parameterName, std::string& parameterValue);

  /// Sets all of the specified parameters in the codec
  /// \param parameters Map containing the parameters and values to be set
  virtual void SetParameters(std::map<std::string, std::string> parameters);

  /// Returns a list of the human readable names of the supported parameter presets
  std::vector<std::string> GetParameterPresetNames() const;

  /// Get the string representing the preset value given the name of the parameter
  /// \param presetName String containing the name of the preset
  /// Returns a string representing the preset value
  std::string GetParameterPresetValue(const std::string& presetName) const;

  /// Get the human readable preset name given the parameter value string
  /// \param presetValue String representing the preset value
  /// Returns the human readable name of the preset value
  std::string GetParameterPresetName(const std::string& presetValue) const;

  /// Get the number of parameter presets
  int GetNumberOfParameterPresets() const { return this->ParameterPresets.size(); };

  struct ParameterPreset
  {
    /// Displayable human-readable name
    /// (for example "maximum compression").
    std::string Name;
    /// Machine-readable code, identifier, and/or parameter list that the codec can interpret
    /// (for example "ZLIB_9", referring to zlib compression with compression factor 9).
    std::string Value;
  };
  // Get a list of all supported parameter presets for the codec
  vtkGetStdVectorMacro(ParameterPresets, const std::vector<ParameterPreset>);

  /// Set the current parameters of the codec based on the specified preset value.
  /// The method must be overridden in child classes that support presets.
  /// \param presetValue String representing the preset value
  /// Returns true on success.
  virtual bool SetParametersFromPresetValue(const std::string& presetValue);

  /// Get the default preset parameter value
  /// The human readable name of the parameter can be retreived using GetParameterPresetName()
  /// \sa GetParameterPresetName()
  vtkGetMacro(DefaultParameterPresetValue, std::string);

protected:

  /// Updates parameter values for the codec
  /// \param parameterName String containing the name of the parameter
  /// \param parameterValue Value of the specified parameter
  /// Returns true if the parameter was found and updated successfully
  virtual bool UpdateParameterInternal(std::string parameterName, std::string parameterValue) = 0;

  /// Decode a frame and store its contents in a vtkImageData
  /// This function performs the actual decoding for a single frame and should be implemented in all non abstract subclasses
  /// \param inputFame Frame object containing the compressed data to be decoded
  /// \param outputImageData Image data object that will be used to store the output image
  /// \param saveDecodedImage If true, writes the decoded image to the frame. If false, the decoded results are discarded
  /// Returns true if the frame is decoded successfully
  virtual bool DecodeFrameInternal(vtkStreamingVolumeFrame* inputFrame, vtkImageData* outputImageData, bool saveDecodedImage = true) = 0;

  /// Decode a vtkImageData and store its contents in a frame
  /// This function performs the actual encoding for a single frame and should be implemented in all non abstract subclasses
  /// \param inputImageData Image data object containing the uncompressed data to be encoded
  /// \param outputFrame Frame object that will be used to store the compressed data
  /// \param forceKeyFrame When true, attempt to encode the image as a keyframe if the codec supports it
  /// Returns true if the image is encoded successfully
  virtual bool EncodeImageDataInternal(vtkImageData* inputImageData, vtkStreamingVolumeFrame* outputFrame, bool forceKeyFrame) = 0;

protected:
  vtkStreamingVolumeCodec();
  ~vtkStreamingVolumeCodec();

private:
  vtkStreamingVolumeCodec(const vtkStreamingVolumeCodec&);
  void operator=(const vtkStreamingVolumeCodec&);

protected:
  std::vector<std::string>                  AvailiableParameterNames;
  vtkSmartPointer<vtkStreamingVolumeFrame>  LastDecodedFrame;
  std::map<std::string, std::string>        Parameters;
  std::vector<ParameterPreset>              ParameterPresets;
  std::string                               DefaultParameterPresetValue;
};

#endif
