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
#include "vtkMRMLMarkupsAngleNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsAngleNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsAngleNode::vtkMRMLMarkupsAngleNode()
{
  this->MaximumNumberOfControlPoints = 3;
  this->RequiredNumberOfControlPoints = 3;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsAngleNode::~vtkMRMLMarkupsAngleNode()
= default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsAngleNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsAngleNode::ReadXMLAttributes(const char** atts)
{
  Superclass::ReadXMLAttributes(atts);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsAngleNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsAngleNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsAngleNode::UpdateMeasurements()
{
  Superclass::UpdateMeasurements();
  if (this->GetNumberOfControlPoints() < 3)
    {
    return;
    }

  double p1[3] = { 0.0 };
  double c[3] = { 0.0 };
  double p2[3] = { 0.0 };
  this->GetNthControlPointPositionWorld(0, p1);
  this->GetNthControlPointPositionWorld(1, c);
  this->GetNthControlPointPositionWorld(2, p2);

  // Compute the angle (only if necessary since we don't want
  // fluctuations in angle value as the camera moves, etc.)
  if (((fabs(p1[0] - c[0]) <= VTK_DBL_EPSILON) &&
    (fabs(p1[1] - c[1]) <= VTK_DBL_EPSILON) &&
    (fabs(p1[2] - c[2]) <= VTK_DBL_EPSILON)) ||
    ((fabs(p2[0] - c[0]) <= VTK_DBL_EPSILON) &&
    (fabs(p2[1] - c[1]) <= VTK_DBL_EPSILON) &&
      (fabs(p2[2] - c[2]) <= VTK_DBL_EPSILON)))
    {
    return;
    }

  double vector1[3] = { p1[0] - c[0], p1[1] - c[1], p1[2] - c[2] };
  double vector2[3] = { p2[0] - c[0], p2[1] - c[1], p2[2] - c[2] };
  vtkMath::Normalize(vector1);
  vtkMath::Normalize(vector2);
  double angle = vtkMath::DegreesFromRadians(acos(vtkMath::Dot(vector1, vector2)));

  this->SetNthMeasurement(0, "angle", angle, "deg");
}
