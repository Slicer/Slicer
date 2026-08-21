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

  This file was originally developed by Kyle Sunderland, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, Cancer
  Care Ontario, OpenAnatomy, and Brigham and Women's Hospital through NIH grant R01MH112748.

==============================================================================*/

// MRML includes
#include "vtkMRMLMarkupsPlaneDisplayNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLMarkupsPlaneDisplayNode);

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneDisplayNode::vtkMRMLMarkupsPlaneDisplayNode()
{
  this->TypeDisplayName = vtkMRMLTr("vtkMRMLMarkupsPlaneDisplayNode", "Markups Plane Display");

  this->HandlesInteractive = true;
  this->TranslationHandleVisibility = false;
  this->RotationHandleVisibility = false;
  this->ScaleHandleVisibility = true;
  this->CanDisplayScaleHandles = true;
}

//----------------------------------------------------------------------------
vtkMRMLMarkupsPlaneDisplayNode::~vtkMRMLMarkupsPlaneDisplayNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneDisplayNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);

  // Store plane-normal arrow display settings with the display node so scene
  // and MRB reloads preserve the Markups UI state.
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(normalVisibility, NormalVisibility);
  vtkMRMLWriteXMLFloatMacro(normalOpacity, NormalOpacity);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneDisplayNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(normalVisibility, NormalVisibility);
  vtkMRMLReadXMLFloatMacro(normalOpacity, NormalOpacity);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy /*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(NormalVisibility);
  vtkMRMLCopyFloatMacro(NormalOpacity);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLMarkupsPlaneDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(NormalVisibility);
  vtkMRMLPrintFloatMacro(NormalOpacity);
  vtkMRMLPrintEndMacro();
}
