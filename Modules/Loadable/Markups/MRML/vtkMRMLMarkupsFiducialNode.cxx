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
#include "vtkMRMLMarkupsFiducialNode.h"
#include "vtkMRMLMarkupsFiducialStorageNode.h"
#include "vtkMRMLScene.h"

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsFiducialNode);


//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode::vtkMRMLMarkupsFiducialNode()
{

}

//----------------------------------------------------------------------------
vtkMRMLMarkupsFiducialNode::~vtkMRMLMarkupsFiducialNode()
{

}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of,nIndent);

  vtkIndent indent(nIndent);

  //of << indent << " locked=\"" << this->Locked << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->RemoveAllMarkups();

  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;

  int fidID = 0;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    // backward compatibility reading of annotation fiducials
    if (!strcmp(attName, "ctrlPtsCoord"))
      {
      std::string valStr(attValue);
      std::stringstream ss;
      double x, y, z;
      ss << valStr;
      ss >> x;
      ss >> y;
      ss >> z;
      fidID = this->AddFiducial(x,y,z);
      }
    else if (!strcmp(attName, "ctrlPtsSelected"))
      {
      std::stringstream ss;
      int selected;
      ss << attValue;
      ss >> selected;
      this->SetNthFiducialSelected(fidID, (selected == 1 ? true : false));
      }
    else if (!strcmp(attName, "ctrlPtsVisible"))
      {
      std::stringstream ss;
      int visible;
      ss << attValue;
      ss >> visible;
      this->SetNthFiducialVisibility(fidID, (visible == 1 ? true : false));
      }
    else if (!strcmp(attName, "ctrlPtsNumberingScheme"))
      {
      // ignore
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::Copy(vtkMRMLNode *anode)
{
  Superclass::Copy(anode);
}


//-----------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::UpdateScene(vtkMRMLScene *scene)
{
  Superclass::UpdateScene(scene);
}

//---------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::ProcessMRMLEvents ( vtkObject *caller,
                                           unsigned long event,
                                           void *callData )
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}

//-------------------------------------------------------------------------
vtkMRMLStorageNode* vtkMRMLMarkupsFiducialNode::CreateDefaultStorageNode()
{
  return vtkMRMLStorageNode::SafeDownCast(vtkMRMLMarkupsFiducialStorageNode::New());
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::CreateDefaultDisplayNodes()
{
  if (this->GetDisplayNode() != NULL &&
      vtkMRMLMarkupsDisplayNode::SafeDownCast(this->GetDisplayNode()) != NULL)
    {
    // display node already exists
    return;
    }
  if (this->GetScene()==NULL)
    {
    vtkErrorMacro("vtkMRMLMarkupsFiducialNode::CreateDefaultDisplayNodes failed: scene is invalid");
    return;
    }
  vtkNew<vtkMRMLMarkupsDisplayNode> dispNode;
  this->GetScene()->AddNode(dispNode.GetPointer());
  this->SetAndObserveDisplayNodeID(dispNode->GetID());
}

//-------------------------------------------------------------------------
vtkMRMLMarkupsDisplayNode *vtkMRMLMarkupsFiducialNode::GetMarkupsDisplayNode()
{
  vtkMRMLDisplayNode *displayNode = this->GetDisplayNode();
  if (displayNode &&
      displayNode->IsA("vtkMRMLMarkupsDisplayNode"))
    {
    return vtkMRMLMarkupsDisplayNode::SafeDownCast(displayNode);
    }
  return NULL;
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
  vtkVector3d point;
  point.SetX(x);
  point.SetY(y);
  point.SetZ(z);
  return this->AddPointToNewMarkup(point, label);
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
  vtkVector3d point= this->GetMarkupPointVector(n, 0);
  pos[0] = point.GetX();
  pos[1] = point.GetY();
  pos[2] = point.GetZ();
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialPositionFromArray(int n, double pos[3])
{
  this->SetMarkupPoint(n, 0, pos[0], pos[1], pos[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialPosition(int n, double x, double y, double z)
{
  this->SetMarkupPoint(n, 0, x, y, z);
}

//-------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialNode::GetNthFiducialSelected(int n)
{
  return this->GetNthMarkupSelected(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialSelected(int n, bool flag)
{
  this->SetNthMarkupSelected(n, flag);
}

//-------------------------------------------------------------------------
bool vtkMRMLMarkupsFiducialNode::GetNthFiducialVisibility(int n)
{
  return this->GetNthMarkupVisibility(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialVisibility(int n, bool flag)
{
  this->SetNthMarkupVisibility(n, flag);
}

//-------------------------------------------------------------------------
std::string vtkMRMLMarkupsFiducialNode::GetNthFiducialLabel(int n)
{
  return this->GetNthMarkupLabel(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialLabel(int n, std::string label)
{
  this->SetNthMarkupLabel(n, label);
}

//-------------------------------------------------------------------------
std::string vtkMRMLMarkupsFiducialNode::GetNthFiducialAssociatedNodeID(int n)
{
  return this->GetNthMarkupAssociatedNodeID(n);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialAssociatedNodeID(int n, const char* id)
{
  this->SetNthMarkupAssociatedNodeID(n, std::string(id));
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::SetNthFiducialWorldCoordinates(int n, double coords[4])
{
  this->SetMarkupPointWorld(n, 0, coords[0], coords[1], coords[2]);
}

//-------------------------------------------------------------------------
void vtkMRMLMarkupsFiducialNode::GetNthFiducialWorldCoordinates(int n, double coords[4])
{
  this->GetMarkupPointWorld(n, 0, coords);
}
