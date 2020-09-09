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

vtkStandardNewMacro(vtkMRMLMeasurementLength);

//----------------------------------------------------------------------------
vtkMRMLMeasurementLength::vtkMRMLMeasurementLength()
{
}

//----------------------------------------------------------------------------
vtkMRMLMeasurementLength::~vtkMRMLMeasurementLength() = default;

//----------------------------------------------------------------------------
void vtkMRMLMeasurementLength::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLMeasurementLength::Execute()
{
  if (!this->InputMRMLNode)
    {
    return;
    }
  vtkMRMLMarkupsCurveNode* curveNode = vtkMRMLMarkupsCurveNode::SafeDownCast(this->InputMRMLNode);

  double length = curveNode->GetCurveLengthWorld();

  std::string printFormat;
  std::string unit = "mm";
  vtkMRMLUnitNode* unitNode = curveNode->GetUnitNode("length");
  if (unitNode)
    {
    if (unitNode->GetSuffix())
      {
      unit = unitNode->GetSuffix();
      }
    length = unitNode->GetDisplayValueFromValue(length);
    printFormat = unitNode->GetDisplayStringFormat();
    }

  this->SetValue(length);
  this->SetUnits(unit.c_str());
  this->SetPrintFormat(printFormat.c_str());
}
