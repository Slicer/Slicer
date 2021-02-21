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

  /*
  if (curveNode->GetNumberOfDefinedControlPoints(true) < 3)
      {
      vtkDebugMacro("Compute: Curve nodes must have more than one control points ("
        << curveNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
      return;
      }
      */

  double size[3] = { 0.0, 0.0, 0.0 };
  roiNode->GetSize(size);
  double volume = size[0] * size[1] * size[2];

  // We derive volume unit from length unit, but it may be better to introduce
  // an volume unit node to be able to specify more human-friendly format.
  vtkMRMLUnitNode* lengthUnitNode = roiNode->GetUnitNode("length");
  std::string printFormat = "%-#4.4gmm3";
  std::string unit = "mm3";
  if (lengthUnitNode)
    {
    // Derive Volume unit from length unit.
    // This could be made a feature of unit nodes.
    if (lengthUnitNode->GetDisplayOffset() != 0.0)
      {
      vtkErrorMacro("vtkMRMLMeasurementVolume::Compute error: length unit display offset is non-zero, computation");
      this->LastComputationResult = vtkMRMLMeasurement::InternalError;
      this->ClearValue(vtkMRMLMeasurement::InternalError);
      return;
      }
    volume *= lengthUnitNode->GetDisplayCoefficient() * lengthUnitNode->GetDisplayCoefficient() * lengthUnitNode->GetDisplayCoefficient();
    unit = lengthUnitNode->GetSuffix() ? lengthUnitNode->GetSuffix() : "";

    std::stringstream strstream;
    strstream << lengthUnitNode->GetPrefix() << "%."
      << 3 * lengthUnitNode->GetPrecision() << "g" // precision is 2x as volume is length^3
      << lengthUnitNode->GetSuffix() << "3"; // square
    strstream >> printFormat;
    }

  this->LastComputationResult = vtkMRMLMeasurement::OK;
  this->SetValue(volume);
  this->SetUnits(unit.c_str());
  this->SetPrintFormat(printFormat.c_str());
}
