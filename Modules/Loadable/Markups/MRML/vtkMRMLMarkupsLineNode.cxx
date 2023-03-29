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
#include "vtkMRMLMeasurementLength.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLUnitNode.h"

// VTK includes
#include <vtkCollection.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkTransform.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsLineNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsLineNode::vtkMRMLMarkupsLineNode()
{
  this->MaximumNumberOfControlPoints = 2;
  this->RequiredNumberOfControlPoints = 2;

  // Setup measurements calculated for this markup type
  vtkNew<vtkMRMLMeasurementLength> lengthMeasurement;
  lengthMeasurement->SetName("length");
  lengthMeasurement->SetInputMRMLNode(this);
  this->Measurements->AddItem(lengthMeasurement);
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

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetLineStartPosition(double position[3])
{
  this->GetNthControlPointPosition(0, position);
  return this->GetNthControlPointPositionStatus(0) != PositionUndefined;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetLineEndPosition(double position[3])
{
  this->GetNthControlPointPosition(1, position);
  return this->GetNthControlPointPositionStatus(1) != PositionUndefined;
}

//---------------------------------------------------------------------------
vtkVector3d vtkMRMLMarkupsLineNode::GetLineStartPosition()
{
  return vtkVector3d(this->GetNthControlPointPosition(0));
}

//---------------------------------------------------------------------------
vtkVector3d vtkMRMLMarkupsLineNode::GetLineEndPosition()
{
  return vtkVector3d(this->GetNthControlPointPosition(1));
}


//---------------------------------------------------------------------------
vtkVector3d vtkMRMLMarkupsLineNode::GetLineStartPositionWorld()
{
  double position[3] = {0.0, 0.0, 0.0};
  this->GetNthControlPointPositionWorld(0, position);
  return vtkVector3d(position);
}

//---------------------------------------------------------------------------
vtkVector3d vtkMRMLMarkupsLineNode::GetLineEndPositionWorld()
{
  double position[3] = { 0.0, 0.0, 0.0 };
  this->GetNthControlPointPositionWorld(1, position);
  return vtkVector3d(position);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetLineStartPositionWorld(double position[3])
{
  this->GetNthControlPointPositionWorld(0, position);
  return this->GetNthControlPointPositionStatus(0) != PositionUndefined;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetLineEndPositionWorld(double position[3])
{
  this->GetNthControlPointPositionWorld(1, position);
  return this->GetNthControlPointPositionStatus(1) != PositionUndefined;
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::SetLineStartPosition(double position[3])
{
  if (this->GetNumberOfControlPoints() < 1)
    {
    this->AddControlPoint(position);
    }
  else
    {
    this->SetNthControlPointPosition(0, position);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::SetLineEndPosition(double position[3])
{
  if (this->GetNumberOfControlPoints() == 0)
    {
    // First control point has not been added yet.
    // Add it now, with undefined position.
    ControlPoint* controlPoint = new ControlPoint;
    this->AddControlPoint(controlPoint);
    }
  if (this->GetNumberOfControlPoints() < 2)
    {
    this->AddControlPoint(position);
    }
  else
    {
    this->SetNthControlPointPosition(1, position);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::SetLineStartPositionWorld(double position[3])
{
  if (this->GetNumberOfControlPoints() < 1)
    {
    this->AddControlPointWorld(position);
    }
  else
    {
    this->SetNthControlPointPositionWorld(0, position);
    }
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsLineNode::SetLineEndPositionWorld(double position[3])
{
  if (this->GetNumberOfControlPoints() == 0)
    {
    // First control point has not been added yet.
    // Add it now, with undefined position.
    ControlPoint* controlPoint = new ControlPoint;
    this->AddControlPoint(controlPoint);
    }
  if (this->GetNumberOfControlPoints() < 2)
    {
    this->AddControlPointWorld(position);
    }
  else
    {
    this->SetNthControlPointPositionWorld(1, position);
    }
}


// Deprecated methods

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetPosition1(double position[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::GetPosition1 method is deprecated. Use GetLineStartPosition method instead.");
  return this->GetLineStartPosition(position);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetPosition2(double position[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::GetPosition2 method is deprecated. Use GetLineEndPosition method instead.");
  return this->GetLineEndPosition(position);
}

//---------------------------------------------------------------------------
double* vtkMRMLMarkupsLineNode::GetPosition1()
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::GetPosition1 method is deprecated. Use GetLineStartPosition method instead.");
  if (this->GetNthControlPointPositionStatus(0) == PositionUndefined)
    {
    return nullptr;
    }
  return this->GetNthControlPointPosition(0);
}

//---------------------------------------------------------------------------
double* vtkMRMLMarkupsLineNode::GetPosition2()
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::GetPosition2 method is deprecated. Use GetLineEndPosition method instead.");
  if (this->GetNthControlPointPositionStatus(1) == PositionUndefined)
    {
    return nullptr;
    }
  return this->GetNthControlPointPosition(1);
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::SetPosition1(double nC1, double nC2, double nC3)
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPosition1 method is deprecated. Use SetLineStartPosition method instead.");
  double newControl[3] = { nC1, nC2, nC3 };
  this->SetLineStartPosition(newControl);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::SetPosition2(double nC1, double nC2, double nC3)
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPosition2 method is deprecated. Use SetLineEndPosition method instead.");
  double newControl[3] = { nC1, nC2, nC3 };
  this->SetLineEndPosition(newControl);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::SetPosition1(double newControl[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPosition1 method is deprecated. Use SetLineStartPosition method instead.");
  this->SetLineStartPosition(newControl);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::SetPosition2(double newControl[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPosition2 method is deprecated. Use SetLineEndPosition method instead.");
  this->SetLineEndPosition(newControl);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetPositionWorldCoordinates1(double position[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::GetPositionWorldCoordinates1 method is deprecated. Use GetLineStartPositionWorld method instead.");
  if (this->GetNthControlPointPositionStatus(0) == PositionUndefined)
    {
    return false;
    }
  this->GetNthControlPointPosition(0, position);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::GetPositionWorldCoordinates2(double position[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPositionWorldCoordinates2 method is deprecated. Use GetLineEndPositionWorld method instead.");
  if (this->GetNthControlPointPositionStatus(1) == PositionUndefined)
    {
    return false;
    }
  this->GetNthControlPointPosition(1, position);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::SetPositionWorldCoordinates1(double newControl[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPositionWorldCoordinates1 method is deprecated. Use SetLineStartPositionWorld method instead.");
  this->SetLineStartPositionWorld(newControl);
  return true;
}

//---------------------------------------------------------------------------
bool vtkMRMLMarkupsLineNode::SetPositionWorldCoordinates2(double newControl[3])
{
  vtkWarningMacro("vtkMRMLMarkupsLineNode::SetPositionWorldCoordinates2 method is deprecated. Use SetLineEndPositionWorld method instead.");
  this->SetLineEndPositionWorld(newControl);
  return true;
}
