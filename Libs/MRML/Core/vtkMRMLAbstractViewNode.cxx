/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// VTK includes
#include <vtkStringArray.h>

// MRML includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLInteractionNode.h"
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <sstream>

const char* vtkMRMLAbstractViewNode::OrientationMarkerHumanModelReferenceRole = "OrientationMarkerHumanModel";
const char* vtkMRMLAbstractViewNode::ParentLayoutNodeReferenceRole = "ParentLayoutNodeRef";
const char* vtkMRMLAbstractViewNode::InteractionNodeReferenceRole = "InteractionNodeRef";
const int vtkMRMLAbstractViewNode::AxisLabelsCount = 6;
static const char* DEFAULT_AXIS_LABELS[vtkMRMLAbstractViewNode::AxisLabelsCount] = {"L", "R", "P", "A", "I", "S"};

//----------------------------------------------------------------------------
vtkMRMLAbstractViewNode::vtkMRMLAbstractViewNode()
{
  this->BackgroundColor[0] = 0.0;
  this->BackgroundColor[1] = 0.0;
  this->BackgroundColor[2] = 0.0;
  this->BackgroundColor2[0] = 0.0;
  this->BackgroundColor2[1] = 0.0;
  this->BackgroundColor2[2] = 0.0;

  this->LayoutColor[0] = vtkMRMLAbstractViewNode::GetThreeDViewBlueColor()[0];
  this->LayoutColor[1] = vtkMRMLAbstractViewNode::GetThreeDViewBlueColor()[1];
  this->LayoutColor[2] = vtkMRMLAbstractViewNode::GetThreeDViewBlueColor()[2];

  this->SetLayoutLabel("1");
  this->SetHideFromEditors(0);

  this->AxisLabels = vtkSmartPointer<vtkStringArray>::New();
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->AxisLabels->InsertNextValue(DEFAULT_AXIS_LABELS[i]);
    }
 }

//----------------------------------------------------------------------------
vtkMRMLAbstractViewNode::~vtkMRMLAbstractViewNode()
{
  this->SetLayoutLabel(nullptr);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLStringMacro(layoutLabel, LayoutLabel);
  vtkMRMLWriteXMLStringMacro(layoutName, LayoutName);
  if (this->GetViewGroup() > 0)
    {
    vtkMRMLWriteXMLBooleanMacro(viewGroup, ViewGroup);
    }
  vtkMRMLWriteXMLBooleanMacro(active, Active);
  vtkMRMLWriteXMLBooleanMacro(visibility, Visibility);
  vtkMRMLWriteXMLVectorMacro(backgroundColor, BackgroundColor, double, 3);
  vtkMRMLWriteXMLVectorMacro(backgroundColor2, BackgroundColor2, double, 3);
  vtkMRMLWriteXMLVectorMacro(layoutColor, LayoutColor, double, 3);
  if (this->OrientationMarkerEnabled)
    {
    vtkMRMLWriteXMLEnumMacro(orientationMarkerType, OrientationMarkerType);
    vtkMRMLWriteXMLEnumMacro(orientationMarkerSize, OrientationMarkerSize);
    }
  if (this->RulerEnabled)
    {
    vtkMRMLWriteXMLEnumMacro(rulerType, RulerType);
    }
  vtkMRMLWriteXMLEnumMacro(rulerColor, RulerColor);
  vtkMRMLWriteXMLEndMacro();

  of << " AxisLabels=\"";
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    of << (i>0?";":"") << this->GetAxisLabel(i);
    }
  of << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  const int backGroundColorInvalid = -1;
  this->BackgroundColor2[0] = backGroundColorInvalid;

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLStringMacro(layoutLabel, LayoutLabel);
  vtkMRMLReadXMLStringMacro(layoutName, LayoutName);
  vtkMRMLReadXMLBooleanMacro(viewGroup, ViewGroup);
  vtkMRMLReadXMLBooleanMacro(active, Active);

  // XXX Do not read 'visibility' attribute and default to 1 because:
  // (1) commit r21034 (STYLE: Add abstract class for all view nodes)
  // changed the default value for 'visibility' attribute from 1 to 0. This
  // means there are a lot of already saved scene where visibility attribute
  // value is saved as 0.
  // (2) support for visibility attribute by the layout manager has been
  // added.
  // XXX Support for 'visibility' attribute could be restored by updating
  // the mrml version. Scene with a newer version number would consider the
  // serialized attribute whereas older scene would not.
  //
  // vtkMRMLReadXMLBooleanMacro(visibility, Visibility)

  vtkMRMLReadXMLVectorMacro(backgroundColor, BackgroundColor, double, 3);
  vtkMRMLReadXMLVectorMacro(backgroundColor2, BackgroundColor2, double, 3);
  vtkMRMLReadXMLVectorMacro(layoutColor, LayoutColor, double, 3);
  vtkMRMLReadXMLEnumMacro(orientationMarkerType, OrientationMarkerType);
  vtkMRMLReadXMLEnumMacro(orientationMarkerSize, OrientationMarkerSize);
  vtkMRMLReadXMLEnumMacro(rulerType, RulerType);
  vtkMRMLReadXMLEnumMacro(rulerColor, RulerColor);
  vtkMRMLReadXMLEndMacro();

  const char* attName;
  const char* attValue;
  while (*atts != nullptr)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "AxisLabels"))
      {
      std::stringstream labels(attValue);
      std::string label;
      int labelIndex = 0;
      while (std::getline(labels, label, ';') && labelIndex<vtkMRMLAbstractViewNode::AxisLabelsCount)
        {
        this->SetAxisLabel(labelIndex, label.c_str());
        labelIndex++;
        }
      // If not all labels were defined set the missing ones to empty
      // to make sure all labels are consistently set.
      for (; labelIndex<vtkMRMLAbstractViewNode::AxisLabelsCount; labelIndex++)
        {
        this->SetAxisLabel(labelIndex, "");
        }
      }
    }
#if MRML_SUPPORT_VERSION < 0x040000
  if (this->BackgroundColor2[0] == backGroundColorInvalid)
    {
    // BackgroundColor2 has not been set
    this->BackgroundColor2[0] = this->BackgroundColor[0];
    this->BackgroundColor2[1] = this->BackgroundColor[1];
    this->BackgroundColor2[2] = this->BackgroundColor[2];
    }
#endif
#if MRML_SUPPORT_VERSION < 0x040200
  // vtkMRMLAbstractViewNodes where not singletons before 4.2
  if (!this->GetLayoutName() || strlen(this->GetLayoutName()))
    {
    const char* layoutName = nullptr;
    if (this->GetID() &&
        strncmp(this->GetID(), this->GetClassName(), strlen(this->GetClassName())) == 0)
      {
      layoutName = this->GetID() + strlen(this->GetClassName());
      }
    if (!layoutName || strlen(layoutName) == 0)
      {
      layoutName = "1";
      }
    this->SetLayoutName(layoutName);
    }
#endif
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::CopyContent(vtkMRMLNode* anode, bool deepCopy/*=true*/)
{
  MRMLNodeModifyBlocker blocker(this);
  Superclass::CopyContent(anode, deepCopy);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyStringMacro(LayoutLabel);
  vtkMRMLCopyIntMacro(ViewGroup);
  vtkMRMLCopyIntMacro(Active);
  vtkMRMLCopyIntMacro(Visibility);
  vtkMRMLCopyVectorMacro(BackgroundColor, double, 3);
  vtkMRMLCopyVectorMacro(BackgroundColor2, double, 3);
  vtkMRMLCopyVectorMacro(LayoutColor, double, 3);
  if (this->OrientationMarkerEnabled)
    {
    vtkMRMLCopyEnumMacro(OrientationMarkerType);
    vtkMRMLCopyEnumMacro(OrientationMarkerSize);
    }
  if (this->RulerEnabled)
    {
    vtkMRMLCopyEnumMacro(RulerType);
    }
  vtkMRMLCopyEnumMacro(RulerColor);
  vtkMRMLCopyEndMacro();

  vtkMRMLAbstractViewNode *node = vtkMRMLAbstractViewNode::SafeDownCast(anode);
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->SetAxisLabel(i,node->GetAxisLabel(i));
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::Reset(vtkMRMLNode* defaultNode)
{
  // The LayoutName is preserved by vtkMRMLNode::Reset, however the layout
  // label (typically associated with the layoutName) is not preserved
  // automatically.
  // This require a custom behavior implemented here.
  std::string layoutLabel = this->GetLayoutLabel() ? this->GetLayoutLabel() : "";
  int viewGroup = this->GetViewGroup();
  this->Superclass::Reset(defaultNode);
  this->DisableModifiedEventOn();
  this->SetLayoutLabel(layoutLabel.c_str());
  this->SetViewGroup(viewGroup);
  this->AxisLabels->Reset();
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->AxisLabels->InsertNextValue(DEFAULT_AXIS_LABELS[i]);
    }
  this->DisableModifiedEventOff();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintStringMacro(LayoutLabel);
  vtkMRMLPrintIntMacro(ViewGroup);
  vtkMRMLPrintIntMacro(Active);
  vtkMRMLPrintIntMacro(Visibility);
  vtkMRMLPrintVectorMacro(BackgroundColor, double, 3);
  vtkMRMLPrintVectorMacro(BackgroundColor2, double, 3);
  vtkMRMLPrintVectorMacro(LayoutColor, double, 3);
  if (this->OrientationMarkerEnabled)
    {
    vtkMRMLPrintEnumMacro(OrientationMarkerType);
    vtkMRMLPrintEnumMacro(OrientationMarkerSize);
    }
  if (this->RulerEnabled)
    {
    vtkMRMLPrintEnumMacro(RulerType);
    }
  vtkMRMLPrintEnumMacro(RulerColor);
  vtkMRMLPrintEndMacro();

  os << indent << " AxisLabels: ";
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    os << (i>0?";":"") << this->GetAxisLabel(i);
    }
  os << "\n";

}

//------------------------------------------------------------------------------
vtkMRMLInteractionNode* vtkMRMLAbstractViewNode::GetInteractionNode()
{
  vtkMRMLInteractionNode * interactionNode =
      vtkMRMLInteractionNode::SafeDownCast(this->GetNodeReference(this->InteractionNodeReferenceRole));
  if (this->GetScene() && !interactionNode)
    {
    interactionNode = vtkMRMLInteractionNode::SafeDownCast (
          this->GetScene()->GetNodeByID("vtkMRMLInteractionNodeSingleton"));
    }
  return interactionNode;
}

//------------------------------------------------------------------------------
bool vtkMRMLAbstractViewNode::SetInteractionNodeID(const char *interactionNodeId)
{
  if (!interactionNodeId)
    {
    return false;
    }
  this->SetNodeReferenceID(this->InteractionNodeReferenceRole, interactionNodeId);
  return true;
}

//------------------------------------------------------------------------------
bool vtkMRMLAbstractViewNode::SetInteractionNode(vtkMRMLNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return false;
    }
  return this->SetInteractionNodeID(node ? node->GetID() : nullptr);
}

int vtkMRMLAbstractViewNode::IsMappedInLayout()
{
  if (!this->GetAttribute("MappedInLayout"))
    {
    return 0;
    }
  return strcmp(this->GetAttribute("MappedInLayout"), "1") == 0;
}

//------------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::SetMappedInLayout(int value)
{
  if (this->IsMappedInLayout() == value)
    {
    return;
    }
  this->SetAttribute("MappedInLayout", value ? "1" : "0");
}

//------------------------------------------------------------------------------
bool vtkMRMLAbstractViewNode::IsViewVisibleInLayout()
{
  return (this->IsMappedInLayout() && this->GetVisibility());
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetOrientationMarkerTypeAsString(int id)
{
  switch (id)
    {
    case OrientationMarkerTypeNone: return "none";
    case OrientationMarkerTypeCube: return "cube";
    case OrientationMarkerTypeHuman: return "human";
    case OrientationMarkerTypeAxes: return "axes";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLAbstractViewNode::GetOrientationMarkerTypeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i=0; i<OrientationMarkerType_Last; i++)
    {
    if (strcmp(name, GetOrientationMarkerTypeAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetOrientationMarkerSizeAsString(int id)
{
  switch (id)
    {
    case OrientationMarkerSizeSmall: return "small";
    case OrientationMarkerSizeMedium: return "medium";
    case OrientationMarkerSizeLarge: return "large";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLAbstractViewNode::GetOrientationMarkerSizeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int i=0; i<OrientationMarkerSize_Last; i++)
    {
    if (strcmp(name, GetOrientationMarkerSizeAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetRulerTypeAsString(int id)
{
  switch (id)
    {
    case RulerTypeNone: return "none";
    case RulerTypeThin: return "thin";
    case RulerTypeThick: return "thick";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLAbstractViewNode::GetRulerTypeFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i=0; i<RulerType_Last; i++)
    {
    if (strcmp(name, GetRulerTypeAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetRulerColorAsString(int id)
{
  switch (id)
    {
    case RulerColorWhite: return "white";
    case RulerColorBlack: return "black";
    case RulerColorYellow: return "yellow";
    default:
      // invalid id
      return "";
    }
}

//-----------------------------------------------------------
int vtkMRMLAbstractViewNode::GetRulerColorFromString(const char* name)
{
  if (name == nullptr)
    {
    // invalid name
    return -1;
    }
  for (int i=0; i<RulerColor_Last; i++)
    {
    if (strcmp(name, GetRulerColorAsString(i))==0)
      {
      // found a matching name
      return i;
      }
    }
  // unknown name
  return -1;
}

//-----------------------------------------------------------
void vtkMRMLAbstractViewNode::SetOrientationMarkerHumanModelNodeID(const char* modelNodeId)
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::SetOrientationMarkerHumanModelID failed: orientation marker is disabled");
    return;
    }
  this->SetNodeReferenceID(OrientationMarkerHumanModelReferenceRole, modelNodeId);
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetOrientationMarkerHumanModelNodeID()
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::GetOrientationMarkerHumanModelID failed: orientation marker is disabled");
    return nullptr;
    }
  return this->GetNodeReferenceID(OrientationMarkerHumanModelReferenceRole);
}

//-----------------------------------------------------------
vtkMRMLModelNode* vtkMRMLAbstractViewNode::GetOrientationMarkerHumanModelNode()
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::GetOrientationMarkerHumanModel failed: orientation marker is disabled");
    return nullptr;
    }
  return vtkMRMLModelNode::SafeDownCast(this->GetNodeReference(OrientationMarkerHumanModelReferenceRole));
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetAxisLabel(int labelIndex)
{
  if (labelIndex<0 || labelIndex>=vtkMRMLAbstractViewNode::AxisLabelsCount)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::GetAxisLabel labelIndex=" << labelIndex << " argument is invalid. Valid range: 0<=labelIndex<"
      << vtkMRMLAbstractViewNode::AxisLabelsCount);
    return "";
    }
  return this->AxisLabels->GetValue(labelIndex);
}

//-----------------------------------------------------------
void vtkMRMLAbstractViewNode::SetAxisLabel(int labelIndex, const char* label)
{
  if (labelIndex<0 || labelIndex>=vtkMRMLAbstractViewNode::AxisLabelsCount)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::SetAxisLabel labelIndex=" << labelIndex << " argument is invalid. Valid range: 0<=labelIndex<"
      << vtkMRMLAbstractViewNode::AxisLabelsCount);
    return;
    }
  if (label==nullptr)
    {
    label = "";
    }
  if (this->AxisLabels->GetValue(labelIndex).compare(label)==0)
    {
    // no change
    return;
    }
  this->AxisLabels->SetValue(labelIndex, label);
  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLNode* vtkMRMLAbstractViewNode::GetParentLayoutNode()
{
  return this->GetNodeReference(this->ParentLayoutNodeReferenceRole);
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractViewNode::SetAndObserveParentLayoutNodeID(const char *layoutNodeId)
{
  if (!layoutNodeId)
    {
    return false;
    }

  this->SetAndObserveNodeReferenceID(this->ParentLayoutNodeReferenceRole, layoutNodeId);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLAbstractViewNode::SetAndObserveParentLayoutNode(vtkMRMLNode* node)
{
  if (node && this->Scene != node->GetScene())
    {
    vtkErrorMacro("Cannot set reference: the referenced and referencing node are not in the same scene");
    return false;
    }

  return this->SetAndObserveParentLayoutNodeID(node ? node->GetID() : nullptr);
}

//----------------------------------------------------------------------------
double* vtkMRMLAbstractViewNode::GetRedColor()
{
  // #F34A33
  static double redColor[3] = {243. / 255.,
                                74. / 255.,
                                51. / 255.};
  return redColor;
}

//----------------------------------------------------------------------------
double* vtkMRMLAbstractViewNode::GetYellowColor()
{
  // #EDD54C
  static double yellowColor[3] = {237. / 255.,
                                  213. / 255.,
                                   76. / 255.};
  return yellowColor;
}

//----------------------------------------------------------------------------
double* vtkMRMLAbstractViewNode::GetGreenColor()
{
  // #6EB04B
  static double greenColor[3] = {110. / 255.,
                                 176. / 255.,
                                  75. / 255.};
  return greenColor;
}

//----------------------------------------------------------------------------
double* vtkMRMLAbstractViewNode::GetCompareColor()
{
  // #E17012
  static double compareColor[3] = {225. / 255.,
                                   112. / 255.,
                                    18. / 255.};
  return compareColor;
}

//----------------------------------------------------------------------------
double* vtkMRMLAbstractViewNode::GetGrayColor()
{
  // #8C8C8C
  static double grayColor[3] = {140. / 255.,
                                140. / 255.,
                                140. / 255.};
  return grayColor;
}

//------------------------------------------------------------------------------
double* vtkMRMLAbstractViewNode::GetThreeDViewBlueColor()
{
  // #7483E9
  static double blueColor[3] = {116. / 255.,
                                131. / 255.,
                                233. / 255.};
  return blueColor;
}
