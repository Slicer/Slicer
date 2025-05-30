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
  and was supported in part through NIH grant R01 HL153166.

==============================================================================*/

// MRML includes
#include <vtkImplicitInvertableBoolean.h>
#include "vtkMRMLClipNode.h"
#include "vtkMRMLI18N.h"
#include <vtkMRMLMessageCollection.h>
#include <vtkMRMLTransformableNode.h>
#include <vtkMRMLSliceNode.h>

// VTK includes
#include <vtkImplicitBoolean.h>
#include <vtkImplicitFunctionCollection.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPlanes.h>

// vtkAddon includes
#include <vtkCapPolyData.h>

// STD includes
#include <sstream>

const char* vtkMRMLClipNode::ClippingNodeReferenceRole = "clipping";
const char* vtkMRMLClipNode::ClippingNodeReferenceRef = "clippingRef";
const char* vtkMRMLClipNode::ClippingNodeStatePropertyName = "clippingState";
int DEFAULT_CLIPPING_STATE = vtkMRMLClipNode::ClipPositiveSpace;

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLClipNode);

//----------------------------------------------------------------------------
vtkMRMLClipNode::vtkMRMLClipNode()
{
  this->HideFromEditors = true;
  this->ClippingMethod = vtkMRMLClipNode::Straight;

  this->ImplicitFunction = vtkSmartPointer<vtkImplicitBoolean>::New();

  this->AddNodeReferenceRole(ClippingNodeReferenceRole, ClippingNodeReferenceRef, nullptr, true);
}

//----------------------------------------------------------------------------
vtkMRMLClipNode::~vtkMRMLClipNode() = default;

//----------------------------------------------------------------------------
void vtkMRMLClipNode::WriteXML(ostream& of, int nIndent)
{
  Superclass::WriteXML(of, nIndent);
  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLIntMacro(clipType, ClipType);
  if (this->ClippingMethod != vtkMRMLClipNode::Straight)
  {
    vtkMRMLWriteXMLEnumMacro(clippingMethod, ClippingMethod);
  }
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::ReadXMLAttributes(const char** atts)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::ReadXMLAttributes(atts);
  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(clipType, ClipType);
  vtkMRMLReadXMLEnumMacro(clippingmethod, ClippingMethod);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);
  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(ClipType);
  vtkMRMLCopyEnumMacro(ClippingMethod);
  vtkMRMLCopyEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);
  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(ClipType);
  vtkMRMLPrintEnumMacro(ClippingMethod);
  vtkMRMLPrintEndMacro();
}

//-----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClippingMethodFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  if (!strcmp(name, "Straight"))
  {
    return Straight;
  }
  else if (!strcmp(name, "WholeCells")
    || !strcmp(name, "Whole Cells"))  // for backward compatibility
  {
    return WholeCells;
  }
  else if (!strcmp(name, "WholeCellsWithBoundary")
    || !strcmp(name, "Whole Cells With Boundary"))  // for backward compatibility
  {
    return WholeCellsWithBoundary;
  }
  // unknown name
  return -1;
}

//-----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetClippingMethodAsString(int type)
{
  switch (type)
  {
  case Straight: return "Straight";
  case WholeCells: return "WholeCells";
  case WholeCellsWithBoundary: return "WholeCellsWithBoundary";
  default:
    // invalid id
    return "";
  }
}

//-----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClippingStateFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  if (!strcmp(name, "ClipOff"))
  {
    return (int)ClipOff;
  }
  else if (!strcmp(name, "ClipPositiveSpace"))
  {
    return (int)ClipPositiveSpace;
  }
  else if (!strcmp(name, "ClipNegativeSpace"))
  {
    return (int)ClipNegativeSpace;
  }
  return -1;
}

//-----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetClippingStateAsString(int state)
{
  switch (state)
  {
  case ClipOff: return "ClipOff";
  case ClipPositiveSpace: return "ClipPositiveSpace";
  case ClipNegativeSpace: return "ClipNegativeSpace";
  default:
    // invalid state
    return "";
  }
}

//-----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClipTypeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  if (!strcmp(name, "ClipIntersection"))
  {
    return ClipIntersection;
  }
  else if (!strcmp(name, "ClipUnion"))
  {
    return ClipUnion;
  }
  else if (!strcmp(name, "0"))
  {
    return ClipIntersection; // For backwards compatibility
  }
  else if (!strcmp(name, "1"))
  {
    return ClipUnion; // For backwards compatibility
  }
  // unknown name
  return -1;
}

//-----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetClipTypeAsString(int type)
{
  switch (type)
  {
  case ClipIntersection: return "ClipIntersection";
  case ClipUnion: return "ClipUnion";
  default:
    // invalid type
    return "";
  }
}


//----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetClippingNodeReferenceRole() const
{
  return vtkMRMLClipNode::ClippingNodeReferenceRole;
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetAndObserveClippingNodeID(const char* clipNodeID)
{
  this->SetAndObserveNodeReferenceID(this->GetClippingNodeReferenceRole(), clipNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::AddAndObserveClippingNodeID(const char* clipNodeID)
{
  this->AddAndObserveNodeReferenceID(this->GetClippingNodeReferenceRole(), clipNodeID);
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::RemoveClippingNodeID(const char* clipNodeID)
{
  if (!clipNodeID)
  {
    return;
  }

  this->RemoveNthClippingNodeID(this->GetClippingNodeIndexFromID(clipNodeID));
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::RemoveNthClippingNodeID(int n)
{
  this->RemoveNthNodeReferenceID(this->GetClippingNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::RemoveAllClippingNodeIDs()
{
  this->RemoveNodeReferenceIDs(this->GetClippingNodeReferenceRole());
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetAndObserveNthClippingNodeID(int n, const char* clipNodeID)
{
  this->SetAndObserveNthNodeReferenceID(this->GetClippingNodeReferenceRole(), n, clipNodeID);
}

//----------------------------------------------------------------------------
bool vtkMRMLClipNode::HasClippingNodeID(const char* clipNodeID)
{
  return this->HasNodeReferenceID(this->GetClippingNodeReferenceRole(), clipNodeID);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetNumberOfClippingNodes()
{
  return this->GetNumberOfNodeReferences(this->GetClippingNodeReferenceRole());
}

//----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetNthClippingNodeID(int n)
{
  return this->GetNthNodeReferenceID(this->GetClippingNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClippingNodeIndexFromID(const char* clipNodeID)
{
  if (!clipNodeID)
  {
    return -1;
  }

  int numClipNodes = this->GetNumberOfNodeReferences(
    this->GetClippingNodeReferenceRole());

  for (int plotIndex = 0; plotIndex < numClipNodes; plotIndex++)
  {
    const char* id = this->GetNthNodeReferenceID(
      this->GetClippingNodeReferenceRole(), plotIndex);
    if (id && !strcmp(clipNodeID, id))
    {
      return plotIndex;
    }
  }

  return -1;
}

//----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetClippingNodeID()
{
  return this->GetNthClippingNodeID(0);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLClipNode::GetNthClippingNode(int n)
{
  return this->GetNthNodeReference(this->GetClippingNodeReferenceRole(), n);
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLClipNode::GetClippingNode()
{
  return this->GetNthClippingNode(0);
}

//----------------------------------------------------------------------------
const char* vtkMRMLClipNode::GetClippingNodeStatePropertyName()
{
  return vtkMRMLClipNode::ClippingNodeStatePropertyName;
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::OnNodeReferenceAdded(vtkMRMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceAdded(reference);
  if (strcmp(reference->GetReferenceRole(), this->ClippingNodeReferenceRole) == 0)
  {
    if (reference->GetProperty(this->GetClippingNodeStatePropertyName()).empty())
    {
      // Set the default clipping state to positive
      reference->SetProperty(this->GetClippingNodeStatePropertyName(), GetClippingStateAsString(DEFAULT_CLIPPING_STATE));
    }
    this->UpdateImplicitFunction();
    this->InvokeCustomModifiedEvent(vtkMRMLClipNode::ClipNodeModifiedEvent, reference->GetReferencedNode());
  }
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::OnNodeReferenceRemoved(vtkMRMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceRemoved(reference);
  if (strcmp(reference->GetReferenceRole(), this->ClippingNodeReferenceRole) == 0)
  {
    this->UpdateImplicitFunction();
    this->InvokeCustomModifiedEvent(vtkMRMLClipNode::ClipNodeModifiedEvent, reference->GetReferencedNode());
  }
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::OnNodeReferenceModified(vtkMRMLNodeReference* reference)
{
  this->Superclass::OnNodeReferenceModified(reference);
  if (strcmp(reference->GetReferenceRole(), this->ClippingNodeReferenceRole) == 0)
  {
    this->UpdateImplicitFunction();
    this->InvokeCustomModifiedEvent(vtkMRMLClipNode::ClipNodeModifiedEvent, reference->GetReferencedNode());
  }
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::UpdateImplicitFunction()
{
  vtkImplicitFunctionCollection* functions = this->ImplicitFunction->GetFunction();
  functions->RemoveAllItems();

  int numClipNodes = this->GetNumberOfClippingNodes();
  for (int n = 0; n < numClipNodes; n++)
  {
    vtkMRMLNode* clippingNode = this->GetNthClippingNode(n);

    int clippingState = this->GetNthClippingNodeState(n);
    if (clippingState == ClipOff)
    {
      continue;
    }

    vtkNew<vtkImplicitInvertableBoolean> invertableBoolean;
    if (clippingState == ClipNegativeSpace)
    {
      invertableBoolean->InvertOn();
    }
    this->ImplicitFunction->AddFunction(invertableBoolean);

    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(clippingNode);
    if (transformableNode && transformableNode->GetImplicitFunctionWorld())
    {
      invertableBoolean->AddFunction(transformableNode->GetImplicitFunctionWorld());
      continue;
    }

    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(clippingNode);
    if (sliceNode)
    {
      vtkImplicitFunction* implicitFunction = sliceNode->GetImplicitFunctionWorld();
      if (implicitFunction)
      {
        invertableBoolean->AddFunction(implicitFunction);
      }
      continue;
    }

    vtkMRMLClipNode* clipNode = vtkMRMLClipNode::SafeDownCast(clippingNode);
    if (clipNode)
    {
      vtkImplicitFunction* implicitFunction = clipNode->GetImplicitFunctionWorld();
      if (implicitFunction)
      {
        invertableBoolean->AddFunction(implicitFunction);
      }
      continue;
    }
  }

  this->ImplicitFunction->Modified();
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetClipType(int clipType)
{
  if (this->GetClipType() == clipType)
  {
    return;
  }

  switch (clipType)
  {
  case ClipIntersection:
    clipType = vtkImplicitBoolean::VTK_INTERSECTION;
    break;
  case ClipUnion:
    clipType = vtkImplicitBoolean::VTK_UNION;
    break;
  default:
    clipType = vtkImplicitBoolean::VTK_INTERSECTION;
  }
  this->ImplicitFunction->SetOperationType(clipType);
  this->Modified();
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClipType()
{
  int clipType = this->ImplicitFunction->GetOperationType();
  switch (clipType)
  {
  case vtkImplicitBoolean::VTK_INTERSECTION:
    return ClipIntersection;
  case vtkImplicitBoolean::VTK_UNION:
    return ClipUnion;
  default:
    break;
  }
  return ClipIntersection;
}

//----------------------------------------------------------------------------
vtkImplicitFunction* vtkMRMLClipNode::GetImplicitFunctionWorld()
{
  return this->ImplicitFunction;
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::ProcessMRMLEvents(vtkObject* caller, unsigned long event, void* callData)
{
  Superclass::ProcessMRMLEvents(caller, event, callData);
  if (vtkMRMLClipNode::SafeDownCast(caller)
    || vtkMRMLTransformableNode::SafeDownCast(caller)
    || vtkMRMLSliceNode::SafeDownCast(caller))
  {
    this->UpdateImplicitFunction();
    this->InvokeCustomModifiedEvent(vtkMRMLClipNode::ClipNodeModifiedEvent, caller);
  }
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::GetPlanes(vtkPlaneCollection* planes)
{
  vtkCapPolyData::GetPlanes(this->ImplicitFunction, planes);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClippingNodeIndex(const char* nodeID)
{
  int i = -1;
  NodeReferenceListType& references = this->NodeReferences[std::string(ClippingNodeReferenceRole)];
  for (vtkMRMLNodeReference* reference : references)
  {
    ++i;
    if (strcmp(reference->GetReferencedNodeID(), nodeID) == 0)
    {
      return i;
    }
  }

  return -1;
}

//----------------------------------------------------------------------------
int  vtkMRMLClipNode::GetClippingNodeState(vtkMRMLNode* node)
{
  if (!node)
  {
    return false;
  }
  return this->GetClippingNodeState(node->GetID());
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetClippingNodeState(const char* nodeID)
{
  if (!nodeID)
  {
    return ClipOff;
  }

  NodeReferenceListType& references = this->NodeReferences[std::string(ClippingNodeReferenceRole)];
  if (references.size() == 0)
  {
    return ClipOff;
  }

  int clipState = ClipOff;
  for (vtkMRMLNodeReference* nodeReference : references)
  {
    if (strcmp(nodeReference->GetReferencedNodeID(), nodeID) == 0)
    {
      std::string clipStateString = nodeReference->GetProperty(this->GetClippingNodeStatePropertyName());
      if (!clipStateString.empty())
      {
        clipState = GetClippingStateFromString(clipStateString.c_str());
      }
      break;
    }
  }
  return clipState;
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetNthClippingNodeState(int n)
{
  NodeReferenceListType references = this->NodeReferences[ClippingNodeReferenceRole];
  if (references.size() <= static_cast<size_t>(n))
  {
    return ClipOff;
  }

  std::string clipNodeStateString = references[n]->GetProperty(GetClippingNodeStatePropertyName());
  return GetClippingStateFromString(clipNodeStateString.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetClippingNodeState(vtkMRMLNode* node, int state)
{
  if (!node)
  {
    return;
  }
  return this->SetClippingNodeState(node->GetID(), state);
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetClippingNodeState(const char* nodeID, int state)
{
  if (!nodeID)
  {
    return;
  }

  MRMLNodeModifyBlocker blocker(this);

  int clipIndex = this->GetClippingNodeIndex(nodeID);
  if (clipIndex < 0)
  {
    // Clipping node hasn't been added as a reference yet.
    if (state == vtkMRMLClipNode::ClipOff)
    {
      return;
    }
    this->AddAndObserveClippingNodeID(nodeID);
    clipIndex = this->GetClippingNodeIndex(nodeID);
  }

  this->SetNthClippingNodeState(clipIndex, state);
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetNthClippingNodeState(int n, int state)
{
  NodeReferenceListType references = this->NodeReferences[ClippingNodeReferenceRole];
  if (references.size() <= static_cast<size_t>(n))
  {
    return;
  }

  std::string clipStateString = GetClippingStateAsString(state);
  if (references[n]->SetProperty(GetClippingNodeStatePropertyName(), clipStateString))
  {
    this->UpdateImplicitFunction();
    this->Modified();
  }
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::CopyReferences(vtkMRMLNode* node)
{
  Superclass::CopyReferences(node);
  this->UpdateImplicitFunction();
  this->InvokeCustomModifiedEvent(vtkMRMLClipNode::ClipNodeModifiedEvent, nullptr);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetSliceClipState(const char* nodeID)
{
  int nodeIndex = this->GetClippingNodeIndex(nodeID);
  if (nodeIndex < 0)
  {
    return ClipOff;
  }
  return this->GetNthClippingNodeState(nodeIndex);
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetSliceClipState(const char* nodeID, int state)
{
  if (!this->HasClippingNodeID(nodeID))
  {
    this->AddAndObserveClippingNodeID(nodeID);
  }

  int nodeIndex = this->GetClippingNodeIndex(nodeID);
  int oldState = this->GetNthClippingNodeState(nodeIndex);
  if (state == oldState)
  {
    return;
  }

  this->SetNthClippingNodeState(nodeIndex, state);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetRedSliceClipState()
{
  return this->GetSliceClipState("vtkMRMLSliceNodeRed");
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetRedSliceClipState(int state)
{
  this->SetSliceClipState("vtkMRMLSliceNodeRed", state);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetGreenSliceClipState()
{
  return this->GetSliceClipState("vtkMRMLSliceNodeGreen");
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetGreenSliceClipState(int state)
{
  this->SetSliceClipState("vtkMRMLSliceNodeGreen", state);
}

//----------------------------------------------------------------------------
int vtkMRMLClipNode::GetYellowSliceClipState()
{
  return this->GetSliceClipState("vtkMRMLSliceNodeYellow");
}

//----------------------------------------------------------------------------
void vtkMRMLClipNode::SetYellowSliceClipState(int state)
{
  this->SetSliceClipState("vtkMRMLSliceNodeYellow", state);
}

//----------------------------------------------------------------------------
bool vtkMRMLClipNode::GetClippingPlanes(vtkPlaneCollection* planeCollection, bool invert/*=false*/, vtkMRMLMessageCollection* messages/*=nullptr*/)
{
  bool clippingPlanesOnly = true;
  for (int n = 0; n < this->GetNumberOfClippingNodes(); ++n)
  {
    int clippingState = this->GetNthClippingNodeState(n);
    if (clippingState == ClipOff)
    {
      continue;
    }

    bool currentClippingNodePlanesOnly = true;

    vtkMRMLNode* clippingNode = this->GetNthClippingNode(n);
    bool currentInvert = this->GetNthClippingNodeState(n) == ClipNegativeSpace ? !invert : invert;

    vtkMRMLTransformableNode* transformableNode = vtkMRMLTransformableNode::SafeDownCast(clippingNode);
    if (transformableNode && transformableNode->GetImplicitFunctionWorld())
    {
      currentClippingNodePlanesOnly = this->GetClippingPlanesFromFunction(transformableNode->GetImplicitFunctionWorld(), planeCollection, currentInvert);
    }

    vtkMRMLSliceNode* sliceNode = vtkMRMLSliceNode::SafeDownCast(clippingNode);
    if (sliceNode)
    {
      vtkImplicitFunction* implicitFunction = sliceNode->GetImplicitFunctionWorld();
      if (implicitFunction)
      {
        currentClippingNodePlanesOnly = this->GetClippingPlanesFromFunction(sliceNode->GetImplicitFunctionWorld(), planeCollection, currentInvert);
      }
      continue;
    }

    vtkMRMLClipNode* clipNode = vtkMRMLClipNode::SafeDownCast(clippingNode);
    if (clipNode)
    {
      currentClippingNodePlanesOnly = clipNode->GetClippingPlanes(planeCollection, currentInvert, messages);
      continue;
    }

    if (messages && !clipNode && !currentClippingNodePlanesOnly)
    {
      std::string warningMessage = vtkMRMLTr("vtkMRMLClipNode",
        "%name (%id) cannot be represented using only planes");
      warningMessage.replace(warningMessage.find("%name"), 5, clippingNode->GetName());
      warningMessage.replace(warningMessage.find("%id"), 3, clippingNode->GetID());
      messages->AddMessage(vtkCommand::MessageEvent, warningMessage);
    }

    clippingPlanesOnly &= currentClippingNodePlanesOnly;
  }

  return clippingPlanesOnly;
}

//---------------------------------------------------------------------------
bool vtkMRMLClipNode::GetClippingPlanesFromFunction(vtkImplicitFunction* function, vtkPlaneCollection* planeCollection, bool invert/*=false*/)
{
  vtkImplicitBoolean* implicitBoolean = vtkImplicitBoolean::SafeDownCast(function);
  vtkImplicitInvertableBoolean* invertableBoolean = vtkImplicitInvertableBoolean::SafeDownCast(function);
  if (implicitBoolean)
  {
    bool planesOnly = true;
    vtkImplicitFunctionCollection* subFunctions = implicitBoolean->GetFunction();
    for (int i = 0; i < subFunctions->GetNumberOfItems(); ++i)
    {
      vtkImplicitFunction* subFunction = vtkImplicitFunction::SafeDownCast(subFunctions->GetItemAsObject(i));
      if (subFunction)
      {
        planesOnly &= vtkMRMLClipNode::GetClippingPlanesFromFunction(subFunction, planeCollection,
          invertableBoolean && invertableBoolean->GetInvert() ? !invert : invert);
      }
    }
    return planesOnly;
  }

  vtkPlane* implicitPlane = vtkPlane::SafeDownCast(function);
  if (implicitPlane)
  {
    vtkNew<vtkPlane> tempPlane;
    tempPlane->SetOrigin(implicitPlane->GetOrigin());
    tempPlane->SetNormal(implicitPlane->GetNormal());
    if (invert)
    {
      tempPlane->SetNormal(-tempPlane->GetNormal()[0], -tempPlane->GetNormal()[1], -tempPlane->GetNormal()[2]);
    }
    planeCollection->AddItem(tempPlane);
    return true;
  }

  vtkPlanes* planes = vtkPlanes::SafeDownCast(function);
  if (planes)
  {
    for (int i = 0; i < planes->GetNumberOfPlanes(); ++i)
    {
      vtkPlane* currentPlane = planes->GetPlane(i);
      if (currentPlane)
      {
        vtkNew<vtkPlane> tempPlane;
        tempPlane->SetOrigin(currentPlane->GetOrigin());
        tempPlane->SetNormal(currentPlane->GetNormal());
        if (invert)
        {
          tempPlane->SetNormal(-tempPlane->GetNormal()[0], -tempPlane->GetNormal()[1], -tempPlane->GetNormal()[2]);
        }
        planeCollection->AddItem(tempPlane);
      }
    }
    return true;
  }

  return false;
}
