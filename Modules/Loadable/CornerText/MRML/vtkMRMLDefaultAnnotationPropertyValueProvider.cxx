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
bool vtkMRMLDefaultAnnotationPropertyValueProvider::CanProvideValueForPropertyName(
    const std::string &propertyName)
{
  return registeredProperties.count(propertyName);
}

//---------------------------------------------------------------------------
std::string
vtkMRMLDefaultAnnotationPropertyValueProvider::GetValueForPropertyName(
    const std::string &propertyName, const XMLTagAttributes &attributes,
    vtkMRMLSliceNode *sliceNode)
{
  vtkMRMLSliceLogic *sliceLogic = this->GetAppLogic()->GetSliceLogic(sliceNode);

  if (!sliceLogic) { return ""; }

  std::string output = "";

  if (propertyName == "VolumeName")
  {
    vtkMRMLVolumeNode* volumeNode;
    const int layer = this->GetLayerValueAsInteger(attributes);
    switch (layer)
    {
      case LAYER_FOREGROUND:
        {
          output = (volumeNode = sliceLogic->GetForegroundLayer()->GetVolumeNode()) != nullptr
                     ? volumeNode->GetName()
                     : "";
          break;
        }
      case LAYER_BACKGROUND:
        {
          output = (volumeNode = sliceLogic->GetBackgroundLayer()->GetVolumeNode()) != nullptr
                     ? volumeNode->GetName()
                     : "";
          break;
        }
      case LAYER_LABEL:
        {
          output = (volumeNode = sliceLogic->GetLabelLayer()->GetVolumeNode()) != nullptr
                     ? volumeNode->GetName()
                     : "";
          break;
        }
      default:
        {
          break;
        }
    }
  }
  else if (propertyName == "SlabReconstructionThickness" &&
             sliceNode->GetSlabReconstructionEnabled())
  {
    output =
        (std::ostringstream() << std::fixed << std::setprecision(2)
                              << sliceNode->GetSlabReconstructionThickness())
            .str();
  }
  else if (propertyName == "SlabReconstructionType" &&
             sliceNode->GetSlabReconstructionEnabled())
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
  return this->registeredProperties;
}
