/*==============================================================================

  Program: 3D Slicer

  Copyright(c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// GeneralizedReformat MRML includes
#include "vtkMRMLAbstractVolumeResampler.h"

// MRML includes
#include <vtkMRMLVolumeNode.h>

// MRMLLogic includes
#include <vtkMRMLApplicationLogic.h>

//----------------------------------------------------------------------------
void vtkMRMLAbstractVolumeResampler::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
std::string vtkMRMLAbstractVolumeResampler::GetParameterValue(const ResamplingParameters& parameters, const std::string& name)
{
  const ResamplingParameters::const_iterator parameterIt = parameters.find(name);
  if (parameterIt != parameters.end())
  {
    return parameterIt->second;
  }
  return std::string();
}

//----------------------------------------------------------------------------
double vtkMRMLAbstractVolumeResampler::GetParameterValueAsDouble(const ResamplingParameters& parameters, const std::string& name)
{
  const double value = vtkVariant(vtkMRMLAbstractVolumeResampler::GetParameterValue(parameters, name)).ToDouble();
  return value;
}

//----------------------------------------------------------------------------
int vtkMRMLAbstractVolumeResampler::GetParameterValueAsInt(const ResamplingParameters& parameters, const std::string& name)
{
  const int value = vtkVariant(vtkMRMLAbstractVolumeResampler::GetParameterValue(parameters, name)).ToInt();
  return value;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractVolumeResampler::SetParameterValue(ResamplingParameters& parameters, const std::string& name, const std::string& value)
{
  parameters[name] = value;
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractVolumeResampler::SetParameterValueAsDouble(ResamplingParameters& parameters, const std::string& name, double value)
{
  vtkMRMLAbstractVolumeResampler::SetParameterValue(parameters, name, vtkVariant(value).ToString());
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractVolumeResampler::SetParameterValueAsInt(ResamplingParameters& parameters, const std::string& name, int value)
{
  vtkMRMLAbstractVolumeResampler::SetParameterValue(parameters, name, vtkVariant(value).ToString());
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractVolumeResampler::SetMRMLApplicationLogic(vtkMRMLApplicationLogic* applicationLogic)
{
  this->MRMLApplicationLogic = applicationLogic;
}

//----------------------------------------------------------------------------
vtkMRMLApplicationLogic* vtkMRMLAbstractVolumeResampler::GetMRMLApplicationLogic() const
{
  return this->MRMLApplicationLogic;
}
