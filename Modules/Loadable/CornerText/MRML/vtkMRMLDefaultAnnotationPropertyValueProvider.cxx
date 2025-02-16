/*==============================================================================

  Program: 3D Slicer

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/


#include "vtkMRMLDefaultAnnotationPropertyValueProvider.h"

// MRML includes
#include <vtkMRMLApplicationLogic.h>
#include <vtkMRMLScene.h>
#include <vtkMRMLSliceCompositeNode.h>
#include <vtkMRMLSliceLayerLogic.h>
#include <vtkMRMLSliceLogic.h>
#include <vtkMRMLSliceNode.h>
#include <vtkMRMLVolumeNode.h>

// VTK includes
#include <vtkNew.h>

// STD includes
#include <cassert>
#include <sstream>

//---------------------------------------------------------------------------
vtkStandardNewMacro(vtkMRMLDefaultAnnotationPropertyValueProvider);

//---------------------------------------------------------------------------
// vtkMRMLDefaultAnnotationPropertyValueProvider methods

//---------------------------------------------------------------------------
void vtkMRMLDefaultAnnotationPropertyValueProvider::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "vtkMRMLDefaultAnnotationPropertyValueProvider: " << this->GetClassName() << "\n";
}

//---------------------------------------------------------------------------
bool vtkMRMLDefaultAnnotationPropertyValueProvider::CanProvideValueForPropertyName(const std::string& propertyName)
{
  return this->RegisteredProperties.count(propertyName);
}

//---------------------------------------------------------------------------
std::string
vtkMRMLDefaultAnnotationPropertyValueProvider::GetValueForPropertyName(
    const std::string& propertyName,
    const XMLTagAttributes& attributes,
    vtkMRMLSliceNode* sliceNode)
{
  vtkMRMLSliceLogic* sliceLogic = this->GetAppLogic() ? this->GetAppLogic()->GetSliceLogic(sliceNode) : nullptr;
  if (!sliceLogic)
  {
    return "";
  }

  std::string output = "";

  if (propertyName == "VolumeName")
  {
    vtkMRMLVolumeNode* volumeNode = nullptr;
    const int role = this->GetRoleValueAsInteger(attributes);
    switch (role)
    {
      case ROLE_FOREGROUND:
        {
          volumeNode = sliceLogic->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerForeground);
          break;
        }
      case ROLE_BACKGROUND:
        {
          volumeNode = sliceLogic->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerBackground);
          break;
        }
      case ROLE_LABEL:
        {
          volumeNode = sliceLogic->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerLabel);
          break;
        }
      default:
        {
          break;
        }
    }
    if (volumeNode)
    {
      output = volumeNode->GetName();
      if (role == ROLE_FOREGROUND && sliceLogic->GetNthLayerVolumeNode(vtkMRMLSliceLogic::LayerBackground))
      {
        double foregroundOpacity = sliceLogic->GetSliceCompositeNode()->GetForegroundOpacity();
        std::ostringstream stream;
        stream << " (" << static_cast<int>(foregroundOpacity * 100) << "%)";
        output += stream.str();
      }
    }
  }
  else if (propertyName == "SlabReconstructionThickness" && sliceNode->GetSlabReconstructionEnabled())
  {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(2) << sliceNode->GetSlabReconstructionThickness();
    output = stream.str();
  }
  else if (propertyName == "SlabReconstructionType" && sliceNode->GetSlabReconstructionEnabled())
  {
    output = sliceNode->GetSlabReconstructionTypeAsString(sliceNode->GetSlabReconstructionType());
  }

  if (output != "" && attributes.count("prefix"))
  {
    output = attributes.at("prefix") + output;
  }

  return output;
}

//---------------------------------------------------------------------------
std::unordered_set<std::string>
vtkMRMLDefaultAnnotationPropertyValueProvider::GetSupportedProperties()
{
  return this->RegisteredProperties;
}

//---------------------------------------------------------------------------
void vtkMRMLDefaultAnnotationPropertyValueProvider::SetAppLogic(vtkMRMLApplicationLogic* appLogic)
{
  this->AppLogic = appLogic;
}
