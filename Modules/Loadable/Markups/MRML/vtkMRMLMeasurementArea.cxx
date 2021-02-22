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
#include "vtkMRMLMarkupsPlaneNode.h"

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
  if (!this->InputMRMLNode)
    {
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  // We derive area unit from length unit, but it may be better to introduce
  // an area unit node to be able to specify more human-friendly format.
  vtkMRMLUnitNode* unitNode = nullptr;
  double area = 0.0;

  vtkMRMLMarkupsClosedCurveNode* curveNode = vtkMRMLMarkupsClosedCurveNode::SafeDownCast(this->InputMRMLNode);
  vtkMRMLMarkupsPlaneNode* planeNode = vtkMRMLMarkupsPlaneNode::SafeDownCast(this->InputMRMLNode);
  if (curveNode)
    {
    if (curveNode->GetNumberOfDefinedControlPoints(true) < 3)
      {
      vtkDebugMacro("Compute: Curve nodes must have more than one control points ("
        << curveNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
      return;
      }
    area = vtkMRMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(curveNode);
    unitNode = curveNode->GetUnitNode("area");
    }
  else if (planeNode)
    {
    double bounds[6] = { 0.0 };
    planeNode->GetPlaneBounds(bounds);
    area = (bounds[1] - bounds[0]) * (bounds[3] - bounds[2]);
    unitNode = planeNode->GetUnitNode("area");
    }
  else
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputMRMLNode->GetClassName());
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  std::string printFormat;
  std::string unit;
  if (unitNode)
    {
    if (unitNode->GetSuffix())
      {
      unit = unitNode->GetSuffix();
      }
    area = unitNode->GetDisplayValueFromValue(area);
    printFormat = unitNode->GetDisplayStringFormat();
    }
  else
    {
    unit = "cm2"; // mm2 would be too small for most clinical values
    area *= 0.01; // length unit is mm by default, so display coefficient for cm2 is 0.01
    printFormat = "%-#4.4gcm2";
    }

  this->SetValue(area, unit, printFormat, vtkMRMLMeasurement::OK);
}
