/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLMeasurementVolume.h"
#include "vtkMRMLUnitNode.h"

// Markups includes
#include "vtkMRMLMarkupsROINode.h"

vtkStandardNewMacro(vtkMRMLMeasurementVolume);

//----------------------------------------------------------------------------
vtkMRMLMeasurementVolume::vtkMRMLMeasurementVolume()
{
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementVolume::~vtkMRMLMeasurementVolume() = default;

//----------------------------------------------------------------------------
void vtkMRMLMeasurementVolume::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementVolume::Compute()
{
  if (!this->InputMRMLNode)
    {
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  vtkMRMLMarkupsROINode* roiNode = vtkMRMLMarkupsROINode::SafeDownCast(this->InputMRMLNode);
  if (!roiNode)
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputMRMLNode->GetClassName());
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  double size[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSize(size);
  double volume = size[0] * size[1] * size[2];

  vtkMRMLUnitNode* unitNode = roiNode->GetUnitNode("volume");
  std::string printFormat;
  std::string unit;
  if (unitNode)
    {
    if (unitNode->GetSuffix())
      {
      unit = unitNode->GetSuffix();
      }
    volume = unitNode->GetDisplayValueFromValue(volume);
    printFormat = unitNode->GetDisplayStringFormat();
    }
  else
    {
    unit = "cm3"; // mm3 would be too small for most clinical values
    volume *= 0.001; // length unit is mm by default, so display coefficient for cm3 is 0.001
    printFormat = "%-#4.4gcm3";
    }

  this->SetValue(volume, unit, printFormat, vtkMRMLMeasurement::OK);
}
