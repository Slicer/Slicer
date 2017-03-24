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
#include "vtkMRMLModelNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <sstream>

const char* vtkMRMLAbstractViewNode::OrientationMarkerHumanModelReferenceRole = "OrientationMarkerHumanModel";
const int vtkMRMLAbstractViewNode::AxisLabelsCount = 6;
static const char* DEFAULT_AXIS_LABELS[vtkMRMLAbstractViewNode::AxisLabelsCount] = {"L", "R", "P", "A", "I", "S"};

//----------------------------------------------------------------------------
vtkMRMLAbstractViewNode::vtkMRMLAbstractViewNode()
{
  this->LayoutLabel = NULL;
  this->ViewGroup = 0;
  this->Active = 0;
  this->Visibility = 1;

  double black[3] = {0.,0.,0.};
  memcpy(this->BackgroundColor, black, 3 * sizeof(double));
  memcpy(this->BackgroundColor2, black, 3 * sizeof(double));

  this->SetLayoutLabel("1");
  this->SetHideFromEditors(0);

  this->OrientationMarkerEnabled = false;
  this->OrientationMarkerType = OrientationMarkerTypeNone;
  this->OrientationMarkerSize = OrientationMarkerSizeMedium;

  this->RulerEnabled = false;
  this->RulerType = RulerTypeNone;

  this->AxisLabels = vtkSmartPointer<vtkStringArray>::New();
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->AxisLabels->InsertNextValue(DEFAULT_AXIS_LABELS[i]);
    }
 }

//----------------------------------------------------------------------------
vtkMRMLAbstractViewNode::~vtkMRMLAbstractViewNode()
{
  this->SetLayoutLabel(NULL);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  this->Superclass::WriteXML(of, nIndent);

  if (this->GetLayoutLabel())
    {
    of << " layoutLabel=\"" << this->GetLayoutLabel() << "\"";
    }
  if (this->GetLayoutName())
    {
    of << " layoutName=\"" << this->GetLayoutName() << "\"";
    }
  if (this->GetViewGroup() > 0)
    {
    of << " viewGroup=\"" << this->GetViewGroup() << "\"";
    }

  of << " active=\"" << (this->Active ? "true" : "false") << "\"";
  of << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";

  // background color
  of << " backgroundColor=\"" << this->BackgroundColor[0] << " "
     << this->BackgroundColor[1] << " " << this->BackgroundColor[2] << "\"";

  of << " backgroundColor2=\"" << this->BackgroundColor2[0] << " "
     << this->BackgroundColor2[1] << " " << this->BackgroundColor2[2] << "\"";

  if (this->OrientationMarkerEnabled)
    {
    of << " orientationMarkerType=\"" << this->GetOrientationMarkerTypeAsString(this->OrientationMarkerType) << "\"";
    of << " orientationMarkerSize=\"" << this->GetOrientationMarkerSizeAsString(this->OrientationMarkerSize) << "\"";
    }

  if (this->RulerEnabled)
    {
    of << " rulerType=\"" << this->GetRulerTypeAsString(this->RulerType) << "\"";
    }

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
  bool isBackgroundColor2Set = false;
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if (!strcmp(attName, "layoutLabel"))
      {
      this->SetLayoutLabel( attValue );
      }
    else if (!strcmp(attName, "layoutName"))
      {
      this->SetLayoutName( attValue );
      }
    else if (!strcmp(attName, "viewGroup"))
      {
      std::stringstream ss;
      ss << attValue;
      int val;
      ss >> val;
      this->SetViewGroup(val);
      }
    else if (!strcmp(attName, "backgroundColor"))
      {
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->BackgroundColor[0] = val;
      ss << attValue;
      ss >> val;
      this->BackgroundColor[1] = val;
      ss << attValue;
      ss >> val;
      this->BackgroundColor[2] = val;
      }
    else if (!strcmp(attName, "backgroundColor2"))
      {
      isBackgroundColor2Set = true;
      std::stringstream ss;
      ss << attValue;
      double val;
      ss >> val;
      this->BackgroundColor2[0] = val;
      ss << attValue;
      ss >> val;
      this->BackgroundColor2[1] = val;
      ss << attValue;
      ss >> val;
      this->BackgroundColor2[2] = val;
      }
    else if (!strcmp(attName, "active"))
      {
      if (!strcmp(attValue,"true"))
        {
        this->Active = 1;
        }
      else
        {
        this->Active = 0;
        }
      }
    else if (!strcmp(attName, "orientationMarkerType") && this->OrientationMarkerEnabled)
      {
      int id = this->GetOrientationMarkerTypeFromString(attValue);
      if (id<0)
        {
        vtkWarningMacro("Invalid orientationMarkerType: "<<(attValue?attValue:"(none)"));
        }
      else
        {
        this->OrientationMarkerType = id;
        }
      }
    else if (!strcmp(attName, "orientationMarkerSize") && this->OrientationMarkerEnabled)
      {
      int id = this->GetOrientationMarkerSizeFromString(attValue);
      if (id<0)
        {
        vtkWarningMacro("Invalid orientationMarkerSize: "<<(attValue?attValue:"(none)"));
        }
      else
        {
        this->OrientationMarkerSize = id;
        }
      }
    else if (!strcmp(attName, "rulerType") && this->RulerEnabled)
      {
      int id = this->GetRulerTypeFromString(attValue);
      if (id<0)
        {
        vtkWarningMacro("Invalid rulerType: "<<(attValue?attValue:"(none)"));
        }
      else
        {
        this->RulerType = id;
        }
      }
    else if (!strcmp(attName, "AxisLabels"))
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
//    else if (!strcmp(attName, "visibility"))
//      {
//      if (!strcmp(attValue,"true"))
//        {
//        this->Visibility = 1;
//        }
//      else
//        {
//        this->Visibility = 0;
//        }
//      }
    }
#if MRML_SUPPORT_VERSION < 0x040000
  if (!isBackgroundColor2Set)
    {
    this->BackgroundColor2[0] = this->BackgroundColor[0];
    this->BackgroundColor2[1] = this->BackgroundColor[1];
    this->BackgroundColor2[2] = this->BackgroundColor[2];
    }
#endif
#if MRML_SUPPORT_VERSION < 0x040200
  // vtkMRMLAbstractViewNodes where not singletons before 4.2
  if (!this->GetLayoutName() || strlen(this->GetLayoutName()))
    {
    const char* layoutName = 0;
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
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLAbstractViewNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  Superclass::Copy(anode);
  vtkMRMLAbstractViewNode *node = (vtkMRMLAbstractViewNode *) anode;

  this->SetLayoutLabel(node->GetLayoutLabel());
  this->SetViewGroup(node->GetViewGroup());
  this->SetBackgroundColor ( node->GetBackgroundColor ( ) );
  this->SetBackgroundColor2 ( node->GetBackgroundColor2 ( ) );
  // Important: do not use SetActive or RemoveActiveFlagInScene will be called
  this->Active = node->GetActive();
  this->Visibility = node->GetVisibility();

  if (this->OrientationMarkerEnabled)
    {
    this->OrientationMarkerType = node->OrientationMarkerType;
    this->OrientationMarkerSize = node->OrientationMarkerSize;
    }

  if (this->RulerEnabled)
    {
    this->RulerType = node->RulerType;
    }

  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    this->SetAxisLabel(i,node->GetAxisLabel(i));
    }

  this->EndModify(disabledModify);
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

  os << indent << "LayoutLabel: " << (this->LayoutLabel ? this->LayoutLabel : "(null)") << std::endl;
  os << indent << "ViewGroup: " << this->ViewGroup << std::endl;
  os << indent << "Active:        " << this->Active << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "BackgroundColor:       " << this->BackgroundColor[0] << " "
     << this->BackgroundColor[1] << " "
     << this->BackgroundColor[2] <<"\n";
  os << indent << "BackgroundColor2:       " << this->BackgroundColor2[0] << " "
     << this->BackgroundColor2[1] << " "
     << this->BackgroundColor2[2] <<"\n";

  if (this->OrientationMarkerEnabled)
    {
    os << indent << "Orientation marker type: " << this->GetOrientationMarkerTypeAsString(this->OrientationMarkerType) << "\n";
    os << indent << "Orientation marker size: " << this->GetOrientationMarkerSizeAsString(this->OrientationMarkerSize) << "\n";
    }

  if (this->RulerEnabled)
    {
    os << indent << "Ruler type: " << this->GetRulerTypeAsString(this->RulerType) << "\n";
    }

  os << indent << " AxisLabels: ";
  for (int i=0; i<vtkMRMLAbstractViewNode::AxisLabelsCount; i++)
    {
    os << (i>0?";":"") << this->GetAxisLabel(i);
    }
  os << "\n";

}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::RemoveActiveFlagInScene()
{
  if (this->Scene == NULL)
    {
    return;
    }

  vtkMRMLAbstractViewNode *node = NULL;
  int nnodes = this->Scene->GetNumberOfNodesByClass("vtkMRMLAbstractViewNode");
  for (int n=0; n<nnodes; n++)
    {
    node = vtkMRMLAbstractViewNode::SafeDownCast (
       this->Scene->GetNthNodeByClass(n, "vtkMRMLAbstractViewNode"));
    if (node != this)
      {
      node->SetActive(0);
      }
    }
}

//------------------------------------------------------------------------------
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
  if (name == NULL)
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
  if (name == NULL)
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
  if (name == NULL)
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
    return NULL;
    }
  return this->GetNodeReferenceID(OrientationMarkerHumanModelReferenceRole);
}

//-----------------------------------------------------------
vtkMRMLModelNode* vtkMRMLAbstractViewNode::GetOrientationMarkerHumanModelNode()
{
  if (!this->OrientationMarkerEnabled)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::GetOrientationMarkerHumanModel failed: orientation marker is disabled");
    return NULL;
    }
  return vtkMRMLModelNode::SafeDownCast(this->GetNodeReference(OrientationMarkerHumanModelReferenceRole));
}

//-----------------------------------------------------------
const char* vtkMRMLAbstractViewNode::GetAxisLabel(int labelIndex)
{
  if (labelIndex<0 || labelIndex>=vtkMRMLAbstractViewNode::AxisLabelsCount)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::GetAxisLabel labelIndex=" << labelIndex << " argument is invalid. Valid range: 0<=labelIndex<" << vtkMRMLAbstractViewNode::AxisLabelsCount);
    return "";
    }
  return this->AxisLabels->GetValue(labelIndex);
}

//-----------------------------------------------------------
void vtkMRMLAbstractViewNode::SetAxisLabel(int labelIndex, const char* label)
{
  if (labelIndex<0 || labelIndex>=vtkMRMLAbstractViewNode::AxisLabelsCount)
    {
    vtkErrorMacro("vtkMRMLAbstractViewNode::SetAxisLabel labelIndex="<<labelIndex<<" argument is invalid. Valid range: 0<=labelIndex<" <<vtkMRMLAbstractViewNode::AxisLabelsCount);
    return;
    }
  if (label==NULL)
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
