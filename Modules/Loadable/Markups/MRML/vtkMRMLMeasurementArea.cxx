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
  // Default unit is "cm2" because mm2 would be too small for most clinical values.
  // Accordingly, display coefficient is 0.01 because length unit is mm by default.
  this->SetUnits("cm2");
  this->SetDisplayCoefficient(0.01);
  this->SetPrintFormat("%-#4.4g%s");
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
    vtkPolyData* surfaceAreaMesh = this->GetMeshValue();
    if (!surfaceAreaMesh)
      {
      vtkNew<vtkPolyData> newMesh;
      this->SetMeshValue(newMesh);
      surfaceAreaMesh = this->GetMeshValue();
      }
    area = vtkMRMLMarkupsClosedCurveNode::GetClosedCurveSurfaceArea(curveNode, surfaceAreaMesh);
    }
  else if (planeNode)
    {
    double size_world[2] = { 0.0, 0.0 };
    planeNode->GetSizeWorld(size_world);
    area = size_world[0] * size_world[1];
    }
  else
    {
    vtkErrorMacro("Compute: Markup type not supported by this measurement: " << this->InputMRMLNode->GetClassName());
    this->ClearValue(vtkMRMLMeasurement::InsufficientInput);
    return;
    }

  this->SetValue(area, "area");
}
