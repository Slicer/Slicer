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
#include "vtkMRMLMarkupsFiducialDisplayNode.h"
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkBoundingBox.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode::vtkMRMLMarkupsFiducialNode() = default;

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode::~vtkMRMLMarkupsFiducialNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(maximumNumberOfControlPoints, MaximumNumberOfControlPoints);
  vtkMRMLWriteXMLIntMacro(requiredNumberOfControlPoints, RequiredNumberOfControlPoints);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);

  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLIntMacro(maximumNumberOfControlPoints, MaximumNumberOfControlPoints);
  vtkMRMLReadXMLIntMacro(requiredNumberOfControlPoints, RequiredNumberOfControlPoints);
  vtkMRMLReadXMLEndMacro();

  // In scenes created by Slicer version version 4.13.0 revision 30287 (built 2021-10-05).
  // The value used to represent unlimited control points has been changed to -1.
  if (this->MaximumNumberOfControlPoints == 0)
    {
    this->MaximumNumberOfControlPoints = -1;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyIntMacro(MaximumNumberOfControlPoints);
  vtkMRMLCopyIntMacro(RequiredNumberOfControlPoints);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialNode::AddFiducial(double x, double y, double z)
{
  return this->AddFiducial(x, y, z, std::string());
}

//-------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialNode::AddFiducial(double x, double y, double z,
                                            std::string label)
{
  vtkWarningMacro("AddFiducial method is deprecated, please use AddControlPoint instead");
  vtkVector3d point;
  point.Set(x, y, z);
  return this->AddControlPoint(point, label);
}

//-------------------------------------------------------------------------

//-------------------------------------------------------------------------
int vtkMRMLMarkupsFiducialNode::AddFiducialFromArray(double pos[3], std::string label)
{
  return this->AddFiducial(pos[0], pos[1], pos[2], label);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::GetNthFiducialPosition(int n, double pos[3])
{
  vtkWarningMacro("GetNthFiducialPosition method is deprecated, please use GetNthControlPointPositionVector instead");
  vtkVector3d point= this->GetNthControlPointPositionVector(n);
  pos[0] = point.GetX();
  pos[1] = point.GetY();
  pos[2] = point.GetZ();
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != nullptr &&
    vtkMRMLMarkupsDisplayNode::SafeDownCast(this->GetDisplayNode()) != nullptr)
    {
    // display node already exists
    return;
    }
  if (this->GetScene() == nullptr)
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkMRMLMarkupsFiducialDisplayNode* dispNode = vtkMRMLMarkupsFiducialDisplayNode::SafeDownCast(
    this->GetScene()->AddNewNodeByClass("vtkMRMLMarkupsFiducialDisplayNode"));
  if (!dispNode)
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialNode::CreateDefaultDisplayNodes failed: unable to create vtkMRMLMarkupsFiducialDisplayNode");
    return;
    }
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}
