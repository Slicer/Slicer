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
#include "vtkMRMLMeasurementLength.h"
#include "vtkMRMLUnitNode.h"

// Markups includes
#include "vtkMRMLMarkupsCurveNode.h"
#include "vtkMRMLMarkupsLineNode.h"

vtkStandardNewMacro(vtkMRMLMeasurementLength);

//----------------------------------------------------------------------------
vtkMRMLMeasurementLength::vtkMRMLMeasurementLength()
{
  this->SetUnits("mm");
  this->SetPrintFormat("%-#4.4g%s");
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementLength::~vtkMRMLMeasurementLength() = default;

//----------------------------------------------------------------------------
void vtkMRMLMeasurementLength::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementLength::Compute()
{
  if (!this->InputMRMLNode)
    {
    this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
    return;
    }

  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->InputMRMLNode);
  vtkMRMLMarkupsLineNode* lineNode = vtkMRMLMarkupsLineNode::SafeDownCast(this->InputMRMLNode);

  double length = 0.0;
  if (curveNode)
    {
    if (curveNode->GetNumberOfDefinedControlPoints(true) < 2)
      {
      vtkDebugMacro("Compute: Curve nodes must have more than one control points ("
        << curveNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
      return;
      }
    length = curveNode->GetCurveLengthWorld();
    }
  else if (lineNode)
    {
    if (lineNode->GetNumberOfDefinedControlPoints(true) < 2)
      {
      vtkDebugMacro("Compute: Line nodes must have exactly two control points ("
        << lineNode->GetNumberOfDefinedControlPoints(true) << " found)");
      this->LastComputationResult = vtkMRMLMeasurement::InsufficientInput;
      return;
      }
    length = lineNode->GetLineLengthWorld();
    }
  else
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputMRMLNode->GetClassName());
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  this->SetValue(length, "length");
}
