/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Ebatinca S.L., Las Palmas de Gran Canaria, Spain

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// MRML includes
#include "vtkMRMLSliceDisplayNode.h"

// VTK includes

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLSliceDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLSliceDisplayNode::vtkMRMLSliceDisplayNode()
{
  // Set active component defaults for mouse (identified by empty string)
  this->ActiveComponents[GetDefaultContextName()] = ComponentInfo();
}

//-----------------------------------------------------------------------------
vtkMRMLSliceDisplayNode::~vtkMRMLSliceDisplayNode()
{
}

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintBooleanMacro(IntersectingSlicesInteractive);
  vtkMRMLPrintBooleanMacro(IntersectingSlicesTranslationEnabled);
  vtkMRMLPrintBooleanMacro(IntersectingSlicesRotationEnabled);
  vtkMRMLPrintIntMacro(IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkMRMLPrintIntMacro(IntersectingSlicesIntersectionMode);
  vtkMRMLPrintIntMacro(IntersectingSlicesLineThicknessMode);
  {
  os << indent << "ActiveComponents:";
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    os << indent << indent;
    if (it->first.empty())
      {
      os << "(default)";
      }
    else
      {
      os << it->first;
      }
    os << ": " << it->second.Type << ", " << it->second.Index;
    }
  os << "\n";
  }
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLBooleanMacro(intersectingSlicesInteractive, IntersectingSlicesInteractive);
  vtkMRMLWriteXMLBooleanMacro(intersectingSlicesTranslationEnabled, IntersectingSlicesTranslationEnabled);
  vtkMRMLWriteXMLBooleanMacro(intersectingSlicesRotationEnabled, IntersectingSlicesRotationEnabled);
  vtkMRMLWriteXMLIntMacro(intersectingSlicesInteractiveHandlesVisibilityMode, IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkMRMLWriteXMLIntMacro(intersectingSlicesIntersectionMode, IntersectingSlicesIntersectionMode);
  vtkMRMLWriteXMLIntMacro(intersectingSlicesIntersectionMode, IntersectingSlicesLineThicknessMode);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLBooleanMacro(intersectingSlicesInteractive, IntersectingSlicesInteractive);
  vtkMRMLReadXMLBooleanMacro(intersectingSlicesTranslationEnabled, IntersectingSlicesTranslationEnabled);
  vtkMRMLReadXMLBooleanMacro(intersectingSlicesRotationEnabled, IntersectingSlicesRotationEnabled);
  vtkMRMLReadXMLIntMacro(intersectingSlicesInteractiveHandlesVisibilityMode, IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkMRMLReadXMLIntMacro(intersectingSlicesIntersectionMode, IntersectingSlicesIntersectionMode);
  vtkMRMLReadXMLIntMacro(intersectingSlicesLineThicknessMode, IntersectingSlicesLineThicknessMode);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLSliceDisplayNode* node = vtkMRMLSliceDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyBooleanMacro(IntersectingSlicesInteractive);
  vtkMRMLCopyBooleanMacro(IntersectingSlicesTranslationEnabled);
  vtkMRMLCopyBooleanMacro(IntersectingSlicesRotationEnabled);
  vtkMRMLCopyIntMacro(IntersectingSlicesInteractiveHandlesVisibilityMode);
  vtkMRMLCopyIntMacro(IntersectingSlicesIntersectionMode);
  vtkMRMLCopyIntMacro(IntersectingSlicesLineThicknessMode);
  vtkMRMLCopyEndMacro();
}

//---------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::SetIntersectingSlicesInteractiveModeEnabled(
  IntersectingSlicesInteractiveMode mode, bool enabled)
{
  switch (mode)
    {
    case vtkMRMLSliceDisplayNode::ModeTranslation:
      this->SetIntersectingSlicesTranslationEnabled(enabled);
      break;
    case vtkMRMLSliceDisplayNode::ModeRotation:
      this->SetIntersectingSlicesRotationEnabled(enabled);
      break;
    default:
      vtkErrorMacro("Unknown mode");
      break;
    }
}

//---------------------------------------------------------------------------
bool vtkMRMLSliceDisplayNode::GetIntersectingSlicesInteractiveModeEnabled(
  IntersectingSlicesInteractiveMode mode)
{
  switch (mode)
    {
    case vtkMRMLSliceDisplayNode::ModeTranslation:
      return this->GetIntersectingSlicesTranslationEnabled();
    case vtkMRMLSliceDisplayNode::ModeRotation:
      return this->GetIntersectingSlicesRotationEnabled();
    default:
      vtkErrorMacro("Unknown mode");
    }
  return false;
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString()
  {
  return vtkMRMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString(this->IntersectingSlicesInteractiveHandlesVisibilityMode);
  }

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::SetIntersectingSlicesInteractiveHandlesVisibilityModeFromString(const char* handlesVisibilityModeString)
  {
  this->SetIntersectingSlicesInteractiveHandlesVisibilityMode(
    vtkMRMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeFromString(handlesVisibilityModeString));
  }

//-----------------------------------------------------------
int vtkMRMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeFromString(const char* name)
  {
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < HandlesVisibilityMode_Last; ii++)
    {
    if (strcmp(name, GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
  }

//---------------------------------------------------------------------------
const char* vtkMRMLSliceDisplayNode::GetIntersectingSlicesInteractiveHandlesVisibilityModeAsString(int id)
  {
  switch (id)
    {
    case NeverVisible: return "NeverVisible";
    case NearbyVisible: return "NearbyVisible";
    case AlwaysVisible: return "AlwaysVisible";
    //case FadingVisible: return "FadingVisible";
    default:
      // invalid id
      return "Invalid";
    }
  }

//----------------------------------------------------------------------------
const char* vtkMRMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeAsString()
  {
  return vtkMRMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeAsString(this->IntersectingSlicesIntersectionMode);
  }

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::SetIntersectingSlicesIntersectionModeFromString(const char* handlesVisibilityModeString)
  {
  this->SetIntersectingSlicesIntersectionMode(
    vtkMRMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeFromString(handlesVisibilityModeString));
  }

//-----------------------------------------------------------
int vtkMRMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeFromString(const char* name)
  {
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < IntersectionMode_Last; ii++)
    {
    if (strcmp(name, GetIntersectingSlicesIntersectionModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
  }

//---------------------------------------------------------------------------
const char* vtkMRMLSliceDisplayNode::GetIntersectingSlicesIntersectionModeAsString(int id)
{
  switch (id)
    {
    case SkipLineCrossings: return "SkipLineCrossings";
    case FullLines: return "FullLines";
    default:
      // invalid id
      return "Invalid";
    }
}

//----------------------------------------------------------------------------
const char* vtkMRMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeAsString()
  {
  return vtkMRMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeAsString(this->IntersectingSlicesLineThicknessMode);
  }

//----------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::SetIntersectingSlicesLineThicknessModeFromString(const char* handlesVisibilityModeString)
  {
  this->SetIntersectingSlicesLineThicknessMode(
    vtkMRMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeFromString(handlesVisibilityModeString));
  }

//-----------------------------------------------------------
int vtkMRMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeFromString(const char* name)
  {
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int ii = 0; ii < LineThicknessMode_Last; ii++)
    {
    if (strcmp(name, GetIntersectingSlicesLineThicknessModeAsString(ii)) == 0)
      {
      // found a matching name
      return ii;
      }
    }
  // unknown name
  return -1;
  }

//---------------------------------------------------------------------------
const char* vtkMRMLSliceDisplayNode::GetIntersectingSlicesLineThicknessModeAsString(int id)
{
  switch (id)
    {
    case FineLines: return "FineLines";
    case MediumLines: return "MediumLines";
    case ThickLines: return "ThickLines";
    default:
      // invalid id
      return "Invalid";
    }
}

//---------------------------------------------------------------------------
int vtkMRMLSliceDisplayNode::GetActiveComponentType(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentType: No interaction context with identifier '" << context << "' was found");
    return ComponentNone;
    }

  return this->ActiveComponents[context].Type;
}

//---------------------------------------------------------------------------
int vtkMRMLSliceDisplayNode::GetActiveComponentIndex(std::string context/*=GetDefaultContextName()*/)
{
  if (this->ActiveComponents.find(context) == this->ActiveComponents.end())
    {
    vtkErrorMacro("GetActiveComponentIndex: No interaction context with identifier '" << context << "' was found");
    return -1;
    }

  return this->ActiveComponents[context].Index;
}

//---------------------------------------------------------------------------
void vtkMRMLSliceDisplayNode::SetActiveComponent(int componentType, int componentIndex, std::string context/*=GetDefaultContextName()*/)
{
  if ( this->ActiveComponents.find(context) != this->ActiveComponents.end()
    && this->ActiveComponents[context].Type == componentType
    && this->ActiveComponents[context].Index == componentIndex )
    {
    // no change
    return;
    }
  this->ActiveComponents[context].Index = componentIndex;
  this->ActiveComponents[context].Type = componentType;
  this->Modified();
}

//---------------------------------------------------------------------------
bool vtkMRMLSliceDisplayNode::HasActiveComponent()
{
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type != ComponentNone)
      {
      return true;
      }
    }
  return false;
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkMRMLSliceDisplayNode::GetActiveComponentInteractionContexts()
{
  std::vector<std::string> interactionContextVector;
  for (std::map<std::string, ComponentInfo>::iterator it = this->ActiveComponents.begin(); it != this->ActiveComponents.end(); ++it)
    {
    if (it->second.Type != ComponentNone)
      {
      interactionContextVector.push_back(it->first);
      }
    }
  return interactionContextVector;
}
