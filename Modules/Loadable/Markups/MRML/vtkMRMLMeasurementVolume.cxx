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
  // Default unit is cm3 because mm3 would be too small for most clinical values.
  // Accordingly, since length unit is mm by default, display coefficient for cm3 is 0.001.
  this->SetUnits("cm3");
  this->SetDisplayCoefficient(0.001);
  this->SetPrintFormat("%-#4.4g%s");
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
  roiNode->GetSizeWorld(size);
  double volume = size[0] * size[1] * size[2];
  double eps = 1.e-6;
  if (volume < eps)
    {
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  this->SetValue(volume, "volume");
}
