/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

// MRML includes
#include "vtkMRMLTableViewNode.h"

#include "vtkMRMLScene.h"
#include "vtkMRMLTableNode.h"

// VTK includes
#include <vtkCommand.h> // for vtkCommand::ModifiedEvent
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char* vtkMRMLTableViewNode::TableNodeReferenceRole = "table";
const char* vtkMRMLTableViewNode::TableNodeReferenceMRMLAttributeName = "tableNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLTableViewNode);

//----------------------------------------------------------------------------
vtkMRMLTableViewNode::vtkMRMLTableViewNode()
{
  this->AddNodeReferenceRole(this->GetTableNodeReferenceRole(),
                             this->GetTableNodeReferenceMRMLAttributeName());
}

//----------------------------------------------------------------------------
vtkMRMLTableViewNode::~vtkMRMLTableViewNode() = default;

//----------------------------------------------------------------------------
const char* vtkMRMLTableViewNode::GetNodeTagName()
{
  return "TableView";
}


//----------------------------------------------------------------------------
void vtkMRMLTableViewNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  of << " doPropagateTableSelection=\"" << (int)this->DoPropagateTableSelection << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLTableViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  Superclass::ReadXMLAttributes(atts);
  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if(!strcmp (attName, "doPropagateTableSelection" ))
      {
      this->SetDoPropagateTableSelection(atoi(attValue)?true:false);
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node\"s attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, SliceID
void vtkMRMLTableViewNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();
  Superclass::Copy(anode);
  vtkMRMLTableViewNode *node = vtkMRMLTableViewNode::SafeDownCast(anode);
  if (node)
    {
    this->SetDoPropagateTableSelection (node->GetDoPropagateTableSelection());
    }
  else
    {
    vtkErrorMacro("vtkMRMLTableViewNode::Copy failed: invalid input node");
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLTableViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
  os << indent << "DoPropagateTableSelection: " << this->DoPropagateTableSelection << "\n";
}

//----------------------------------------------------------------------------
void vtkMRMLTableViewNode::SetTableNodeID(const char* tableNodeId)
{
  this->SetNodeReferenceID(this->GetTableNodeReferenceRole(), tableNodeId);
}

//----------------------------------------------------------------------------
const char* vtkMRMLTableViewNode::GetTableNodeID()
{
  return this->GetNodeReferenceID(this->GetTableNodeReferenceRole());
}

//----------------------------------------------------------------------------
vtkMRMLTableNode* vtkMRMLTableViewNode::GetTableNode()
{
  return vtkMRMLTableNode::SafeDownCast(this->GetNodeReference(this->GetTableNodeReferenceRole()));
}

//----------------------------------------------------------------------------
const char* vtkMRMLTableViewNode::GetTableNodeReferenceRole()
{
  return vtkMRMLTableViewNode::TableNodeReferenceRole;
}

//----------------------------------------------------------------------------
const char* vtkMRMLTableViewNode::GetTableNodeReferenceMRMLAttributeName()
{
  return vtkMRMLTableViewNode::TableNodeReferenceMRMLAttributeName;
}
