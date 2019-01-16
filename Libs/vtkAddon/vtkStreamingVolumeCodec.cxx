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
#include "vtkStreamingVolumeCodec.h"

// STD includes
#include <deque>
#include <sstream>
#include <string>

// VTK includes
#include <vtkObjectFactory.h>

// vtksys includes
#include <vtksys/SystemTools.hxx>

//---------------------------------------------------------------------------
vtkStreamingVolumeCodec::vtkStreamingVolumeCodec()
  : LastDecodedFrame(NULL)
{
}

//---------------------------------------------------------------------------
vtkStreamingVolumeCodec::~vtkStreamingVolumeCodec()
{
}

//---------------------------------------------------------------------------
bool vtkStreamingVolumeCodec::DecodeFrame(vtkStreamingVolumeFrame* streamingFrame, vtkImageData* outputImageData)
{
  if (!streamingFrame || !outputImageData)
    {
    vtkErrorMacro("Invalid arguments!");
    return false;
    }

  vtkStreamingVolumeFrame* currentFrame = streamingFrame;

  std::deque<vtkStreamingVolumeFrame*> frames;
  frames.push_back(currentFrame);

  // Decode previous frames if the following is true:
  // - Current frame is not a keyframe
  // - The frame that was previously decoded is not the same as the frame preceding the current one
  while (currentFrame && !currentFrame->IsKeyFrame() &&
         currentFrame->GetPreviousFrame() != this->LastDecodedFrame)
    {
    currentFrame = currentFrame->GetPreviousFrame();
    frames.push_back(currentFrame);
    }

  while (!frames.empty())
    {
    vtkStreamingVolumeFrame* frame = frames.back();
    if (frame)
      {
      // Decode the required frames
      // Only the final frame needs to be saved to the image
      bool saveDecodedImage = frames.size() == 1;
      if (!this->DecodeFrameInternal(frame, outputImageData, saveDecodedImage))
        {
        vtkErrorMacro("Could not decode frame!");
        return false;
        }
      }
    frames.pop_back();
    }

  this->LastDecodedFrame = streamingFrame;
  return true;
}

//---------------------------------------------------------------------------
bool vtkStreamingVolumeCodec::EncodeImageData(vtkImageData* inputImageData, vtkStreamingVolumeFrame* outputStreamingFrame, bool forceKeyFrame/*=false*/)
{
  if (!inputImageData || !outputStreamingFrame)
    {
    vtkErrorMacro("Invalid arguments!");
    return false;
    }

  if (!this->EncodeImageDataInternal(inputImageData, outputStreamingFrame, forceKeyFrame))
    {
    vtkErrorMacro("Could not encode frame!");
    return false;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkStreamingVolumeCodec::SetParameter(std::string parameterName, std::string parameterValue)
{
  this->Parameters[parameterName] = parameterValue;
  if (!this->UpdateParameterInternal(parameterName, parameterValue))
    {
    return false;
    }
  this->InvokeEvent(vtkStreamingVolumeCodec::ParameterModifiedEvent);
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
bool vtkStreamingVolumeCodec::GetParameter(std::string parameterName, std::string& parameterValue)
{
  std::map<std::string, std::string>::iterator parameterIt = this->Parameters.find(parameterName);
  if (parameterIt == this->Parameters.end())
    {
    return false;
    }

  parameterValue = (*parameterIt).second;
  return true;
}

//---------------------------------------------------------------------------
void vtkStreamingVolumeCodec::SetParameters(std::map<std::string, std::string> parameters)
{
  std::map<std::string, std::string>::iterator parameterIt;
  for (parameterIt = parameters.begin(); parameterIt != parameters.end(); ++parameterIt)
    {
    std::string parameterName = parameterIt->first;
    std::string parameterValue = parameterIt->second;
    this->SetParameter(parameterName, parameterValue);
    }
}

//----------------------------------------------------------------------------
void vtkStreamingVolumeCodec::SetParametersFromString(std::string parameterString)
{
  std::stringstream parameters(parameterString);
  std::string parameter;
  while (std::getline(parameters, parameter, ';'))
    {
    int colonIndex = parameter.find(':');
    std::string parameterName = parameter.substr(0, colonIndex);
    vtksys::SystemTools::ReplaceString(parameterName, "%3A", ":");
    vtksys::SystemTools::ReplaceString(parameterName, "%3B", ";");
    vtksys::SystemTools::ReplaceString(parameterName, "%25", "%");

    std::string parameterValue = parameter.substr(colonIndex + 1);
    vtksys::SystemTools::ReplaceString(parameterValue, "%3A", ":");
    vtksys::SystemTools::ReplaceString(parameterValue, "%3B", ";");
    vtksys::SystemTools::ReplaceString(parameterValue, "%25", "%");
    this->SetParameter(parameterName.c_str(), parameterValue.c_str());
    }
}

//----------------------------------------------------------------------------
std::string vtkStreamingVolumeCodec::GetParametersAsString()
{
  std::stringstream parameterSS;
  if (this->Parameters.size())
    {
    std::map<std::string, std::string>::const_iterator it;
    std::map<std::string, std::string>::const_iterator begin = this->Parameters.begin();
    std::map<std::string, std::string>::const_iterator end = this->Parameters.end();
    for (it = begin; it != end; ++it)
      {
      if (it != begin)
        {
        parameterSS << ';';
        }

      std::string parameterName = it->first;
      vtksys::SystemTools::ReplaceString(parameterName, "%", "%25");
      vtksys::SystemTools::ReplaceString(parameterName, ";", "%3B");
      vtksys::SystemTools::ReplaceString(parameterName, ":", "%3A");

      std::string parameterValue = it->second;
      vtksys::SystemTools::ReplaceString(parameterValue, "%", "%25");
      vtksys::SystemTools::ReplaceString(parameterValue, ";", "%3B");
      vtksys::SystemTools::ReplaceString(parameterValue, ":", "%3A");
      parameterSS << parameterName << ':' << parameterValue;
      }
    }
  return parameterSS.str();
}

//---------------------------------------------------------------------------
void vtkStreamingVolumeCodec::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  os << indent << "Codec FourCC:\t" << this->GetFourCC() << std::endl;
  std::map<std::string, std::string>::iterator codecParameterIt;
  for (codecParameterIt = this->Parameters.begin(); codecParameterIt != this->Parameters.end(); ++codecParameterIt)
    {
    os << indent << codecParameterIt->first << "=\"" << codecParameterIt->second << "\"";
    }
}

//------------------------------------------------------------------------------
std::vector<std::string> vtkStreamingVolumeCodec::GetParameterPresetNames() const
{
  std::vector<std::string> names;
  std::vector<ParameterPreset>::const_iterator presetIt;
  for (presetIt = this->ParameterPresets.begin(); presetIt != this->ParameterPresets.end(); ++presetIt)
    {
    names.push_back(presetIt->Name);
    }
  return names;
}

//------------------------------------------------------------------------------
std::string vtkStreamingVolumeCodec::GetParameterPresetValue(const std::string& presetName) const
{
  std::vector<ParameterPreset>::const_iterator presetIt;
  for (presetIt = this->ParameterPresets.begin(); presetIt != this->ParameterPresets.end(); ++presetIt)
    {
    if (presetIt->Name == presetName)
      {
      break;
      }
    }
  if (presetIt == this->ParameterPresets.end())
    {
    return "";
    }
  return presetIt->Value;
}

//------------------------------------------------------------------------------
std::string vtkStreamingVolumeCodec::GetParameterPresetName(const std::string& parameter) const
{
  std::vector<ParameterPreset>::const_iterator presetIt;
  for (presetIt = this->ParameterPresets.begin(); presetIt != this->ParameterPresets.end(); ++presetIt)
    {
    if (presetIt->Value == parameter)
      {
      break;
      }
    }
  if (presetIt == this->ParameterPresets.end())
    {
    return "";
    }
  return presetIt->Name;
}

//------------------------------------------------------------------------------
bool vtkStreamingVolumeCodec::SetParametersFromPresetValue(const std::string& presetValue)
{
  if (presetValue.empty())
    {
    // no change requested, nothing to do
    return true;
    }
  vtkWarningMacro("vtkStreamingVolumeCodec::SetParametersFromPresetValue failed: not implemented");
  return false;
}
