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
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// Segmentations includes
#include "vtkSegmentationConverterRule.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// SegmentationCore includes
#include <vtkSegment.h>

//----------------------------------------------------------------------------
vtkSegmentationConverterRule::vtkSegmentationConverterRule() = default;

//----------------------------------------------------------------------------
vtkSegmentationConverterRule::~vtkSegmentationConverterRule()
{
  this->ConversionParameters.clear();
}

//----------------------------------------------------------------------------
vtkSegmentationConverterRule* vtkSegmentationConverterRule::Clone()
{
  vtkSegmentationConverterRule* clone = this->CreateRuleInstance();
  clone->ConversionParameters = this->ConversionParameters;
  return clone;
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverterRule::CreateTargetRepresentation(vtkSegment* segment)
{
  // Get target representation
  vtkSmartPointer<vtkDataObject> targetRepresentation = segment->GetRepresentation(
    this->GetTargetRepresentationName());

  // Create an empty target representation if it does not exist, or if we want to replace the target
  if (!targetRepresentation.GetPointer() || this->ReplaceTargetRepresentation)
    {
    targetRepresentation = vtkSmartPointer<vtkDataObject>::Take(
      this->ConstructRepresentationObjectByRepresentation(this->GetTargetRepresentationName()));
    segment->AddRepresentation(this->GetTargetRepresentationName(), targetRepresentation);
    }
  return true;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterRule::GetRuleConversionParameters(ConversionParameterListType& conversionParameters)
{
  // Copy rule conversion parameters into aggregated path parameters
  ConversionParameterListType::iterator paramIt;
  for (paramIt = this->ConversionParameters.begin(); paramIt != this->ConversionParameters.end(); ++paramIt)
    {
    conversionParameters[paramIt->first] = paramIt->second;
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterRule::SetConversionParameter(const std::string& name, const std::string& value, const std::string& description/*=""*/)
{
  this->ConversionParameters[name].first = value;

  if (!description.empty())
    {
    this->ConversionParameters[name].second = description;
    }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverterRule::GetConversionParameter(const std::string& name)
{
  return this->ConversionParameters[name].first;
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverterRule::GetConversionParameterDescription(const std::string& name)
{
  return this->ConversionParameters[name].second;
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverterRule::HasConversionParameter(const std::string& name)
{
  return (this->ConversionParameters.count(name) > 0);
}
