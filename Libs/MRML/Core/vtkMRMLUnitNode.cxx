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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// MRML includes
#include "vtkMRMLNode.h"
#include "vtkMRMLScene.h"
#include "vtkMRMLUnitNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

//------------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLUnitNode);

//----------------------------------------------------------------------------
vtkMRMLUnitNode::vtkMRMLUnitNode()
{
  this->Prefix = 0;
  this->Suffix = 0;
  this->Precision = 3;
  this->MinimumValue = VTK_DOUBLE_MIN;
  this->MaximumValue = VTK_DOUBLE_MAX;

   this->SetQuantity("");
}

//----------------------------------------------------------------------------
vtkMRMLUnitNode::~vtkMRMLUnitNode()
{
  if (this->Prefix)
    {
    delete [] this->Prefix;
    }
  if (this->Suffix)
    {
    delete [] this->Suffix;
    }
}

//----------------------------------------------------------------------------
void vtkMRMLUnitNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes not equal to their defaults
  this->Superclass::WriteXML(of, nIndent);

  vtkIndent indent(nIndent);
  of << indent << " Quantity=\""
    << (this->GetQuantity() ? this->GetQuantity() : "") << "\"";
  of << indent << " Prefix=\"" << (this->Prefix ? this->Prefix : "") << "\"";
  of << indent << " Suffix=\"" << (this->Suffix ? this->Suffix : "") << "\"";
  of << indent << " Precision=\"" << this->Precision << "\"";
  of << indent << " MinimumValue=\"" << this->MinimumValue << "\"";
  of << indent << " MaximumValue=\"" << this->MaximumValue << "\"";
}

namespace
{
//----------------------------------------------------------------------------
template <typename T> T StringToNumber(const char* num)
{
  std::stringstream ss;
  ss << num;
  T result;
  return ss >> result ? result : 0;
}

//----------------------------------------------------------------------------
int StringToInt(const char* str)
{
  return StringToNumber<int>(str);
}

//----------------------------------------------------------------------------
double StringToDouble(const char* str)
{
  return StringToNumber<double>(str);
}
}// end namespace

//----------------------------------------------------------------------------
void vtkMRMLUnitNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();
  this->Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;
  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);

    if (!strcmp(attName, "Quantity"))
      {
      this->SetQuantity(attValue);
      }
    else if (!strcmp(attName, "Prefix"))
      {
      this->SetPrefix(attValue);
      }
    else if (!strcmp(attName, "Suffix"))
      {
      this->SetSuffix(attValue);
      }
    else if (!strcmp(attName, "Precision"))
      {
      int precision = StringToInt(attValue);
      this->SetPrecision(precision);
      }
    else if (!strcmp(attName, "MinimumValue"))
      {
      double min = StringToDouble(attValue);
      this->SetMinimumValue(min);
      }
    else if (!strcmp(attName, "MaximumValue"))
      {
      double max = StringToDouble(attValue);
      this->SetMaximumValue(max);
      }
    }
  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
const char* vtkMRMLUnitNode::GetQuantity()
{
  return this->GetAttribute("Quantity");
}

//----------------------------------------------------------------------------
void vtkMRMLUnitNode::SetQuantity(const char* name)
{
  // Quantity uses attributes internally so it can be easily access by the GUI
  // (qMRMLComboBox for example).
  this->SetAttribute("Quantity", name);
}

//----------------------------------------------------------------------------
std::string vtkMRMLUnitNode::WrapValueWithPrefix(const std::string& value) const
{
  std::string wrappedString = "";
  if (this->Prefix)
    {
    wrappedString = std::string(this->Prefix) + " ";
    }
  return wrappedString + value;
}

//----------------------------------------------------------------------------
std::string vtkMRMLUnitNode::WrapValueWithSuffix(const std::string& value) const
{
  std::string wrappedString = "";
  if (this->Suffix)
    {
    wrappedString = " " + std::string(this->Suffix);
    }
  return value + wrappedString;
}

//----------------------------------------------------------------------------
std::string vtkMRMLUnitNode::WrapValueWithPrefixAndSuffix(const std::string& value) const
{
  return this->WrapValueWithPrefix(this->WrapValueWithSuffix(value));
}

//----------------------------------------------------------------------------
void vtkMRMLUnitNode::SetName(const char* name)
{
  this->Superclass::SetName(name);
  this->SetSingletonTag(name);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLUnitNode::Copy(vtkMRMLNode *anode)
{
  vtkMRMLUnitNode *node = vtkMRMLUnitNode::SafeDownCast(anode);
  if (!node)
    {
    return;
    }

  int disabledModify = this->StartModify();
  this->Superclass::Copy(anode);

  this->SetQuantity(node->GetQuantity());
  this->SetPrefix(node->GetPrefix());
  this->SetSuffix(node->GetSuffix());
  this->SetPrecision(node->GetPrecision());
  this->SetMinimumValue(node->GetMinimumValue());
  this->SetMaximumValue(node->GetMaximumValue());

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLUnitNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);

  os << indent << "Quantity: " <<
    (this->GetQuantity() ? this->GetQuantity() : "(none)") << "\n";
  os << indent << "Prefix: " <<
    (this->Prefix ? this->Prefix : "(none)") << "\n";
  os << indent << "Suffix: " <<
    (this->Suffix ? this->Suffix : "(none)") << "\n";
  os << indent << "Precision: " << this->Precision << "\n";
  os << indent << "MinimumValue: " << this->MinimumValue << "\n";
  os << indent << "MaximumValue: " << this->MaximumValue << "\n";
}
