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

==============================================================================*/

// SegmentationCore includes
#include "vtkSegmentationConversionPath.h"
#include "vtkSegmentationConverterRule.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentationConversionPath);
vtkStandardNewMacro(vtkSegmentationConversionPaths);

//----------------------------------------------------------------------------
vtkSegmentationConversionPath::vtkSegmentationConversionPath() {}

//----------------------------------------------------------------------------
vtkSegmentationConversionPath::~vtkSegmentationConversionPath() {}

//----------------------------------------------------------------------------
void vtkSegmentationConversionPath::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "Cost: " << this->GetCost() << "\n";
  for (const vtkSmartPointer<vtkSegmentationConverterRule>& rule : this->Rules)
  {
    os << indent << "Rule:\n";
    rule->PrintSelf(os, indent.GetNextIndent());
  }
}

//----------------------------------------------------------------------------
int vtkSegmentationConversionPath::GetNumberOfRules()
{
  return this->Rules.size();
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionPath::RemoveAllRules()
{
  this->Rules.clear();
}

//----------------------------------------------------------------------------
vtkSegmentationConverterRule* vtkSegmentationConversionPath::GetRule(int index)
{
  if (index < 0 || index >= this->GetNumberOfRules())
  {
    vtkErrorMacro("GetRule failed: invalid index: " << index);
    return nullptr;
  }
  return this->Rules[index].GetPointer();
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionPath::RemoveRule(int index)
{
  if (index < 0 || index >= this->GetNumberOfRules())
  {
    vtkErrorMacro("RemoveRule failed: invalid index: " << index);
    return;
  }
  this->Rules.erase(this->Rules.begin() + index);
}

//----------------------------------------------------------------------------
int vtkSegmentationConversionPath::AddRule(vtkSegmentationConverterRule* rule)
{
  this->Rules.push_back(rule);
  return this->GetNumberOfRules() - 1;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionPath::AddRules(vtkSegmentationConversionPath* path)
{
  int numberOfRules = path->GetNumberOfRules();
  for (int ruleIndex = 0; ruleIndex < numberOfRules; ++ruleIndex)
  {
    this->AddRule(path->GetRule(ruleIndex));
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionPath::Copy(vtkSegmentationConversionPath* source)
{
  if (!source)
  {
    vtkErrorMacro("DeepCopy failed: invalid source object");
    return;
  }
  this->Rules = source->Rules;
}

//----------------------------------------------------------------------------
unsigned int vtkSegmentationConversionPath::GetCost()
{
  unsigned int cost = 0;
  for (const vtkSmartPointer<vtkSegmentationConverterRule>& rule : this->Rules)
  {
    cost += rule->GetConversionCost();
  }
  return cost;
}

//----------------------------------------------------------------------------
void vtkSegmentationConversionPaths::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkSegmentationConversionPath* path = nullptr;
  vtkCollectionSimpleIterator it;
  for (this->InitTraversal(it); (path = this->GetNextPath(it));)
  {
    os << indent << "Path:\n";
    path->PrintSelf(os, indent.GetNextIndent());
  }
}

//----------------------------------------------------------------------------
vtkSegmentationConversionPath* vtkSegmentationConversionPaths::GetNextPath(vtkCollectionSimpleIterator& cookie)
{
  return vtkSegmentationConversionPath::SafeDownCast(this->GetNextItemAsObject(cookie));
}
