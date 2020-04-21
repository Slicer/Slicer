/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// MRML includes
#include "vtkMRMLAbstractLayoutNode.h"

//----------------------------------------------------------------------------
vtkMRMLAbstractLayoutNode::vtkMRMLAbstractLayoutNode()
{
  this->HideFromEditors = 1;
}

//----------------------------------------------------------------------------
vtkMRMLAbstractLayoutNode::~vtkMRMLAbstractLayoutNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLAbstractLayoutNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLayoutNode::ReadXMLAttributes(const char** atts)
{
  //int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  //this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, LabelText, ID
void vtkMRMLAbstractLayoutNode::Copy(vtkMRMLNode *anode)
{
  //int disabledModify = this->StartModify();

  Superclass::Copy(anode);

  //this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractLayoutNode::PrintSelf(ostream& vtkNotUsed(os), vtkIndent vtkNotUsed(indent))
{
}
