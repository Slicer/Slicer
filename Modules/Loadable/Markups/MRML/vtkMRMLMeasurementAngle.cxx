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
#include "vtkMRMLMeasurementAngle.h"
#include "vtkMRMLUnitNode.h"

// Markups includes
#include "vtkMRMLMarkupsAngleNode.h"

vtkStandardNewMacro(vtkMRMLMeasurementAngle);

//----------------------------------------------------------------------------
vtkMRMLMeasurementAngle::vtkMRMLMeasurementAngle()
{
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementAngle::~vtkMRMLMeasurementAngle() = default;

//----------------------------------------------------------------------------
void vtkMRMLMeasurementAngle::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementAngle::Compute()
{
  if (!this->InputMRMLNode)
    {
    this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
    return;
    }

  vtkMRMLMarkupsAngleNode* angleNode = vtkMRMLMarkupsAngleNode::SafeDownCast(this->InputMRMLNode);
  vtkMRMLUnitNode* unitNode = nullptr;

  double angle = 0.0;
  if (angleNode)
    {
    if (angleNode->GetNumberOfDefinedControlPoints(true) != 3)
      {
      vtkDebugMacro("Compute: Angle nodes must have exactly three control points ("
        << angleNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
      return;
      }
    double p1[3] = { 0.0 };
    double c[3] = { 0.0 };
    double p2[3] = { 0.0 };
    angleNode->GetNthControlPointPositionWorld(0, p1);
    angleNode->GetNthControlPointPositionWorld(1, c);
    angleNode->GetNthControlPointPositionWorld(2, p2);

    if ( vtkMath::Distance2BetweenPoints(p1, c) < VTK_DBL_EPSILON
      || vtkMath::Distance2BetweenPoints(p2, c) < VTK_DBL_EPSILON )
      {
      vtkErrorMacro("Compute: Control points are too close to each other to compute angle reliably");
      this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
      return;
      }

    angle = angleNode->GetAngleDegrees();
    unitNode = angleNode->GetUnitNode("angle");
    }
  else
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputMRMLNode->GetClassName());
    this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
    return;
    }

  std::string printFormat;
  std::string unit = "deg";
  if (unitNode)
    {
    if (unitNode->GetSuffix())
      {
      unit = unitNode->GetSuffix();
      }
    angle = unitNode->GetDisplayValueFromValue(angle);
    printFormat = unitNode->GetDisplayStringFormat();
    }
  else
    {
    printFormat = "%3.1f%s";
    }

  this->SetValue(angle);
  this->SetUnits(unit.c_str());
  this->SetPrintFormat(printFormat.c_str());

  this->LastComputationResult = vtkMRMLMeasurement::OK;
}
