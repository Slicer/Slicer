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
#include "vtkMRMLMarkupsDisplayNode.h"
#include "vtkMRMLMarkupsLineNode.h"
#include "vtkMatrix4x4.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLUnitNode.h"
#include "vtkTransform.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsLineNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode::vtkMRMLMarkupsLineNode()
{
  this->MaximumNumberOfControlPoints = 2;
  this->RequiredNumberOfControlPoints = 2;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode::~vtkMRMLMarkupsLineNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
double vtkMRMLMarkupsLineNode::GetLineLengthWorld()
{
  double p1[3] = { 0.0 };
  double p2[3] = { 0.0 };
  this->GetNthControlPointPositionWorld(0, p1);
  this->GetNthControlPointPositionWorld(1, p2);
  double length = sqrt(vtkMath::Distance2BetweenPoints(p1, p2));
  return length;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::UpdateMeasurementsInternal()
{
  this->RemoveAllMeasurements();
  if (this->GetNumberOfDefinedControlPoints(true) == 2)
    {
    double length = this->GetLineLengthWorld();
    std::string printFormat;
    std::string unit = "mm";
    vtkMRMLUnitNode* unitNode = GetUnitNode("length");
    if (unitNode)
      {
      if (unitNode->GetSuffix())
        {
        unit = unitNode->GetSuffix();
        }
      length = unitNode->GetDisplayValueFromValue(length);
      printFormat = unitNode->GetDisplayStringFormat();
      }
    this->SetNthMeasurement(0, "length", length, unit, printFormat);
    }
  this->WriteMeasurementsToDescription();
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::UpdateInteractionHandleToWorldMatrix()
{
  Superclass::UpdateInteractionHandleToWorldMatrix();
  if (this->GetNumberOfControlPoints() < 2)
    {
    return;
    }

  double handleX_World[4] = { 1.0, 0.0, 0.0, 0.0 };
  this->InteractionHandleToWorldMatrix->MultiplyPoint(handleX_World, handleX_World);

  double point0_World[3];
  this->GetNthControlPointPositionWorld(0, point0_World);
  double point1_World[3];
  this->GetNthControlPointPositionWorld(1, point1_World);

  double vectorPoint0ToPoint1_World[4] = { 0.0 };
  vtkMath::Subtract(point1_World, point0_World, vectorPoint0ToPoint1_World);

  double angle = vtkMath::DegreesFromRadians(vtkMath::AngleBetweenVectors(vectorPoint0ToPoint1_World, handleX_World));
  double epsilon = 1e-5;
  if (angle < epsilon)
    {
    return;
    }

  double rotationVector_Local[3] = { 0.0 };
  vtkMath::Cross(handleX_World, vectorPoint0ToPoint1_World, rotationVector_Local);

  double origin_World[4] = { 0.0, 0.0, 0.0, 1.0 };
  this->InteractionHandleToWorldMatrix->MultiplyPoint(origin_World, origin_World);

  vtkNew<vtkTransform> handleToWorldMatrix;
  handleToWorldMatrix->PostMultiply();
  handleToWorldMatrix->Concatenate(this->InteractionHandleToWorldMatrix);
  handleToWorldMatrix->Translate(-origin_World[0], -origin_World[1], -origin_World[2]);
  handleToWorldMatrix->RotateWXYZ(angle, rotationVector_Local);
  handleToWorldMatrix->Translate(origin_World);
  this->InteractionHandleToWorldMatrix->DeepCopy(handleToWorldMatrix->GetMatrix());
}
