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

// MRML includes
#include "vtkMRMLAbstractViewNode.h"
#include "vtkMRMLScene.h"

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
vtkMRMLAbstractViewNode::vtkMRMLAbstractViewNode()
{
  this->LayoutLabel = NULL;
  this->Active = 0;
  this->Visibility = 1;

  double black[3] = {0.,0.,0.};
  memcpy(this->BackgroundColor, black, 3 * sizeof(double));
  memcpy(this->BackgroundColor2, black, 3 * sizeof(double));

  this->SetLayoutLabel("1");
  this->SetHideFromEditors(0);
 }

//----------------------------------------------------------------------------
vtkMRMLAbstractViewNode::~vtkMRMLAbstractViewNode()
{
  if ( this->LayoutLabel )
    {
    delete [] this->LayoutLabel;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults

  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);

  if (this->GetLayoutLabel())
    {
    of << indent << " layoutLabel=\"" << this->GetLayoutLabel() << "\"";
    }
  if (this->GetLayoutName())
    {
    of << indent << " layoutName=\"" << this->GetLayoutName() << "\"";
    }

  of << indent << " active=\"" << (this->Active ? "true" : "false") << "\"";
  of << indent << " visibility=\"" << (this->Visibility ? "true" : "false") << "\"";

  // background color
  of << indent << " backgroundColor=\"" << this->BackgroundColor[0] << " "
     << this->BackgroundColor[1] << " " << this->BackgroundColor[2] << "\"";

  of << indent << " backgroundColor2=\"" << this->BackgroundColor2[0] << " "
     << this->BackgroundColor2[1] << " " << this->BackgroundColor2[2] << "\"";
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
  this->SetBackgroundColor ( node->GetBackgroundColor ( ) );
  this->SetBackgroundColor2 ( node->GetBackgroundColor2 ( ) );
  // Important: do not use SetActive or RemoveActiveFlagInScene will be called
  this->Active = node->GetActive();
  this->Visibility = node->GetVisibility();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::Reset()
{
  // The LayoutName is preserved by vtkMRMLNode::Reset, however the layout
  // label (typically associated with the layoutName) is not preserved
  // automatically.
  // This require a custom behavior implemented here.
  std::string layoutLabel = this->GetLayoutLabel() ? this->GetLayoutLabel() : "";
  this->Superclass::Reset();
  this->DisableModifiedEventOn();
  this->SetLayoutLabel(layoutLabel.c_str());
  this->DisableModifiedEventOff();
}

//----------------------------------------------------------------------------
void vtkMRMLAbstractViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  os << indent << "LayoutLabel: " << (this->LayoutLabel ? this->LayoutLabel : "(null)") << std::endl;
  os << indent << "Active:        " << this->Active << "\n";
  os << indent << "Visibility:        " << this->Visibility << "\n";
  os << indent << "BackgroundColor:       " << this->BackgroundColor[0] << " "
     << this->BackgroundColor[1] << " "
     << this->BackgroundColor[2] <<"\n";
  os << indent << "BackgroundColor2:       " << this->BackgroundColor2[0] << " "
     << this->BackgroundColor2[1] << " "
     << this->BackgroundColor2[2] <<"\n";
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
