/*==============================================================================

  Program: 3D Slicer

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

#include "vtkMRMLColorLegendDisplayNode.h"

// MRML includes
#include <vtkMRMLScalarVolumeDisplayNode.h>
#include <vtkMRMLModelDisplayNode.h>
#include <vtkMRMLDisplayableNode.h>
#include <vtkMRMLProceduralColorNode.h>
#include <vtkMRMLColorTableNode.h>
#include <vtkMRMLScene.h>

// VTK includes
#include <vtkCallbackCommand.h>
#include <vtkTextProperty.h>

// STD includes
#include <cstring>
#include <sstream>

namespace
{
const char* PRIMARY_DISPLAY_NODE_REFERENCE_ROLE = "primaryDisplay";
}

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLColorLegendDisplayNode);

//-----------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode::vtkMRMLColorLegendDisplayNode()
{
  this->LabelFormat = this->GetDefaultNumericLabelFormat();

  vtkNew<vtkTextProperty> titleTextProperty;
  titleTextProperty->SetBackgroundOpacity(0.0);
  titleTextProperty->SetFontSize(12);
  titleTextProperty->ShadowOn();
  vtkSetAndObserveMRMLObjectMacro(this->TitleTextProperty, titleTextProperty);

  vtkNew<vtkTextProperty> labelTextProperty;
  labelTextProperty->SetBackgroundOpacity(0.0);
  labelTextProperty->SetFontSize(12);
  // Used fixed spaced font family (Courier) so that decimal point in numbers
  // appear at the same position in right-aligned numbers.
  labelTextProperty->SetFontFamilyToCourier();
  labelTextProperty->ShadowOn();
  vtkSetAndObserveMRMLObjectMacro(this->LabelTextProperty, labelTextProperty);

  // Observe primary display node events (like change of scalar range)
  vtkNew<vtkIntArray> primaryDisplayNodeEvents;
  primaryDisplayNodeEvents->InsertNextValue(vtkCommand::ModifiedEvent);
  primaryDisplayNodeEvents->InsertNextValue(vtkMRMLNode::ReferencedNodeModifiedEvent);
  primaryDisplayNodeEvents->InsertNextValue(vtkMRMLNode::ReferenceModifiedEvent);
  this->AddNodeReferenceRole(PRIMARY_DISPLAY_NODE_REFERENCE_ROLE, nullptr, primaryDisplayNodeEvents);

  this->Visibility2DOn();
  this->Visibility3DOn();
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorLegendDisplayNode::GetDefaultNumericLabelFormat()
{
  return "%.1f";
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorLegendDisplayNode::GetDefaultTextLabelFormat()
{
  return "%s";
}

//-----------------------------------------------------------------------------
vtkMRMLColorLegendDisplayNode::~vtkMRMLColorLegendDisplayNode()
{
  this->SetAndObservePrimaryDisplayNode(nullptr);
  vtkSetAndObserveMRMLObjectMacro(this->TitleTextProperty, nullptr);
  vtkSetAndObserveMRMLObjectMacro(this->LabelTextProperty, nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(Orientation);
  vtkMRMLPrintVectorMacro(Position, double, 2);
  vtkMRMLPrintVectorMacro(Size, double, 2);
  vtkMRMLPrintStdStringMacro(TitleText);
  vtkMRMLPrintStdStringMacro(TitleTextPropertyAsString);
  vtkMRMLPrintStdStringMacro(LabelTextPropertyAsString);
  vtkMRMLPrintStdStringMacro(LabelFormat);
  vtkMRMLPrintIntMacro(MaxNumberOfColors);
  vtkMRMLPrintIntMacro(NumberOfLabels);
  vtkMRMLPrintBooleanMacro(UseColorNamesForLabels);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(orientation, Orientation);
  vtkMRMLWriteXMLVectorMacro(position, Position, double, 2);
  vtkMRMLWriteXMLVectorMacro(size, Size, double, 2);
  vtkMRMLWriteXMLStdStringMacro(titleText, TitleText);
  vtkMRMLWriteXMLStdStringMacro(titleTextProperty, TitleTextPropertyAsString);
  vtkMRMLWriteXMLStdStringMacro(labelTextProperty, LabelTextPropertyAsString);
  vtkMRMLWriteXMLStdStringMacro(labelFormat, LabelFormat);
  vtkMRMLWriteXMLIntMacro(maxNumberOfColors, MaxNumberOfColors);
  vtkMRMLWriteXMLIntMacro(numberOfLabels, NumberOfLabels);
  vtkMRMLWriteXMLBooleanMacro(useColorNamesForLabels, UseColorNamesForLabels);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::ReadXMLAttributes(const char** atts)
{
  // Read all MRML node attributes
  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(orientation, Orientation);
  vtkMRMLReadXMLVectorMacro(position, Position, double, 2);
  vtkMRMLReadXMLVectorMacro(size, Size, double, 2);
  vtkMRMLReadXMLStdStringMacro(titleText, TitleText);
  vtkMRMLReadXMLStdStringMacro(titleTextProperty, TitleTextPropertyFromString);
  vtkMRMLReadXMLStdStringMacro(labelTextProperty, LabelTextPropertyFromString);
  vtkMRMLReadXMLStdStringMacro(labelFormat, LabelFormat);
  vtkMRMLReadXMLIntMacro(maxNumberOfColors, MaxNumberOfColors);
  vtkMRMLReadXMLIntMacro(numberOfLabels, NumberOfLabels);
  vtkMRMLReadXMLBooleanMacro(useColorNamesForLabels, UseColorNamesForLabels);
  vtkMRMLReadXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  this->Superclass::CopyContent(anode, deepCopy);

  vtkMRMLColorLegendDisplayNode* node = vtkMRMLColorLegendDisplayNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(Orientation);
  vtkMRMLCopyVectorMacro(Position, double, 2);
  vtkMRMLCopyVectorMacro(Size, double, 2);
  vtkMRMLCopyStdStringMacro(TitleText);
  // The name is misleading, this ShallowCopy method actually creates a deep copy
  this->TitleTextProperty->ShallowCopy(this->SafeDownCast(copySourceNode)->GetTitleTextProperty());
  this->LabelTextProperty->ShallowCopy(this->SafeDownCast(copySourceNode)->GetLabelTextProperty());
  vtkMRMLCopyStdStringMacro(LabelFormat);
  vtkMRMLCopyIntMacro(MaxNumberOfColors);
  vtkMRMLCopyIntMacro(NumberOfLabels);
  vtkMRMLCopyBooleanMacro(UseColorNamesForLabels);
  vtkMRMLCopyEndMacro();
}

//---------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::SetOrientation(int id)
{
  switch (id)
    {
    case 0:
      this->SetOrientation(vtkMRMLColorLegendDisplayNode::Horizontal);
      break;
    case 1:
    default:
      this->SetOrientation(vtkMRMLColorLegendDisplayNode::Vertical);
      break;
    }
}

//---------------------------------------------------------------------------
const char* vtkMRMLColorLegendDisplayNode::GetOrientationAsString(int id)
{
  switch (id)
    {
    case vtkMRMLColorLegendDisplayNode::Horizontal:
      return "Horizontal";
    case vtkMRMLColorLegendDisplayNode::Vertical:
    default:
      return "Vertical";
    }
}

//---------------------------------------------------------------------------
int vtkMRMLColorLegendDisplayNode::GetOrientationFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i = 0; i < vtkMRMLColorLegendDisplayNode::Orientation_Last; i++)
    {
    if (std::strcmp(name, vtkMRMLColorLegendDisplayNode::GetOrientationAsString(i)) == 0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
vtkMRMLDisplayNode* vtkMRMLColorLegendDisplayNode::GetPrimaryDisplayNode()
{
  return vtkMRMLDisplayNode::SafeDownCast(this->GetNodeReference(PRIMARY_DISPLAY_NODE_REFERENCE_ROLE));
}

//----------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::SetAndObservePrimaryDisplayNode(vtkMRMLDisplayNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("vtkMRMLColorLegendDisplayNode: Cannot set reference, the referenced and referencing node are not in the same scene");
    return;
    }

  this->SetAndObserveNodeReferenceID(PRIMARY_DISPLAY_NODE_REFERENCE_ROLE, (node ? node->GetID() : nullptr));
}

//----------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::ProcessMRMLEvents(vtkObject *caller, unsigned long eventID, void *callData)
{
  Superclass::ProcessMRMLEvents(caller, eventID, callData);

  // Propagate primary display node and text properties modification events
  if (eventID == vtkCommand::ModifiedEvent
    && (caller == this->TitleTextProperty || caller == this->LabelTextProperty
     || caller == this->GetPrimaryDisplayNode()))
    {
    this->Modified();
    }
}

//---------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::SetTitleTextPropertyFromString(std::string textPropertyString)
{
  if (textPropertyString.empty())
    {
    vtkErrorMacro("SetTitleTextPropertyFromString: Invalid text property string");
    return;
    }

  std::string currentTextPropertyString = vtkMRMLDisplayNode::GetTextPropertyAsString(this->TitleTextProperty);
  if (textPropertyString == currentTextPropertyString)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->UpdateTextPropertyFromString(textPropertyString, this->TitleTextProperty);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorLegendDisplayNode::GetTitleTextPropertyAsString()
{
  return vtkMRMLDisplayNode::GetTextPropertyAsString(this->TitleTextProperty);
}

//---------------------------------------------------------------------------
void vtkMRMLColorLegendDisplayNode::SetLabelTextPropertyFromString(std::string textPropertyString)
{
  if (textPropertyString.empty())
    {
    vtkErrorMacro("SetLabelTextPropertyFromString: Invalid text property string");
    return;
    }

  std::string currentTextPropertyString = vtkMRMLDisplayNode::GetTextPropertyAsString(this->LabelTextProperty);
  if (textPropertyString == currentTextPropertyString)
    {
    return;
    }

  MRMLNodeModifyBlocker blocker(this);
  this->UpdateTextPropertyFromString(textPropertyString, this->LabelTextProperty);
  this->Modified();
}

//---------------------------------------------------------------------------
std::string vtkMRMLColorLegendDisplayNode::GetLabelTextPropertyAsString()
{
  return vtkMRMLDisplayNode::GetTextPropertyAsString(this->LabelTextProperty);
}
