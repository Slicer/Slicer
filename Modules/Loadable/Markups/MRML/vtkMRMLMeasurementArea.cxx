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
#include "vtkMRMLMeasurementArea.h"
#include "vtkMRMLUnitNode.h"

// Markups includes
#include "vtkMRMLMarkupsClosedCurveNode.h"

vtkStandardNewMacro(vtkMRMLMeasurementArea);

//----------------------------------------------------------------------------
vtkMRMLMeasurementArea::vtkMRMLMeasurementArea()
{
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementArea::~vtkMRMLMeasurementArea() = default;

//----------------------------------------------------------------------------
void vtkMRMLMeasurementArea::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementArea::Compute()
{
  this->ClearValue();
  if (!this->InputMRMLNode)
    {
    this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
    return;
    }

  vtkMRMLMarkupsClosedCurveNode* curveNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(this->InputMRMLNode);
  if (!curveNode)
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputMRMLNode->GetClassName());
    this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
    return;
    }
  if (curveNode->GetNumberOfDefinedControlPoints(true) < 3)
      {
      vtkDebugMacro("Compute: Curve nodes must have more than one control points ("
        << curveNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
      return;
      }
  double area = vtkMRMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(curveNode);

  // We derive area unit from length unit, but it may be better to introduce
  // an area unit node to be able to specify more human-friendly format.
  vtkMRMLUnitNode* lengthUnitNode = curveNode->GetUnitNode("length");
  std::string printFormat = "%-#4.4gmm2";
  std::string unit = "mm2";
  if (lengthUnitNode)
    {
    // Derive area unit from length unit.
    // This could be made a feature of unit nodes.
    if (lengthUnitNode->GetDisplayOffset() != 0.0)
      {
      vtkErrorMacro("vtkMRMLMeasurementArea::Compute error: length unit display offset is non-zero, computation");
      this->LastComputationResult = vtkMRMLMeasurement::InternalError;
      return;
      }
    area = lengthUnitNode->GetDisplayCoefficient() * lengthUnitNode->GetDisplayCoefficient() * area;
    unit = lengthUnitNode->GetSuffix() ? lengthUnitNode->GetSuffix() : "";

    std::stringstream strstream;
    strstream << lengthUnitNode->GetPrefix() << "%."
      << 2 * lengthUnitNode->GetPrecision() << "g" // precision is 2x as area is length^2
      << lengthUnitNode->GetSuffix() << "2"; // square
    strstream >> printFormat;
    }

  this->LastComputationResult = vtkMRMLMeasurement::OK;
  this->SetValue(area);
  this->SetUnits(unit.c_str());
  this->SetPrintFormat(printFormat.c_str());
}
