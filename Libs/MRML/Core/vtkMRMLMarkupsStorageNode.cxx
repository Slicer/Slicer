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

#include "vtkMRMLMarkupsStorageNode.h"

#include "vtkMRMLScene.h"

#include "vtkObjectFactory.h"
#include "vtkStringArray.h"

#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::vtkMRMLMarkupsStorageNode()
{
  this->CoordinateSystem = vtkMRMLMarkupsStorageNode::LPS;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsStorageNode::~vtkMRMLMarkupsStorageNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(CoordinateSystem);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(coordinateSystem, CoordinateSystem);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::Copy(vtkMRMLNode* anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(CoordinateSystem);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::CanReadInReferenceNode(vtkMRMLNode* refNode)
{
  return refNode->IsA("vtkMRMLMarkupsNode");
}

//----------------------------------------------------------------------------
std::string vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString()
{
  return vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(this->CoordinateSystem);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseRASOn()
{
  this->SetCoordinateSystem(vtkMRMLStorageNode::CoordinateSystemRAS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseRAS()
{
  return (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemRAS);
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsStorageNode::UseLPSOn()
{
  this->SetCoordinateSystem(vtkMRMLStorageNode::CoordinateSystemLPS);
}

//----------------------------------------------------------------------------
bool vtkMRMLMarkupsStorageNode::GetUseLPS()
{
  return (this->GetCoordinateSystem() == vtkMRMLStorageNode::CoordinateSystemLPS);
}

//---------------------------------------------------------------------------
const char* vtkMRMLMarkupsStorageNode::GetCoordinateSystemAsString(int id)
{
  return vtkMRMLStorageNode::GetCoordinateSystemTypeAsString(id);
}

//-----------------------------------------------------------
int vtkMRMLMarkupsStorageNode::GetCoordinateSystemFromString(const char* name)
{
  // For backward-compatibility with old scenes (magic number was used instead of string)
  if (strcmp(name, "0") == 0)
  {
    return vtkMRMLStorageNode::CoordinateSystemRAS;
  }
  else if (strcmp(name, "1") == 0)
  {
    return vtkMRMLStorageNode::CoordinateSystemLPS;
  }

  // Current method, store coordinate system as string
  return vtkMRMLStorageNode::GetCoordinateSystemTypeFromString(name);
}
