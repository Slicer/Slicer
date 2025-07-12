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
vtkSegmentationConverterRule::~vtkSegmentationConverterRule() {}

//----------------------------------------------------------------------------
void vtkSegmentationConverterRule::PrintSelf(ostream& os, vtkIndent indent)
{
  os << indent << "Name: " << this->GetName() << "\n";
  os << indent << "SourceRepresentationName: "
     << (this->GetSourceRepresentationName() ? this->GetSourceRepresentationName() : "(none)") << "\n";
  os << indent << "TargetRepresentationName: "
     << (this->GetTargetRepresentationName() ? this->GetTargetRepresentationName() : "(none)") << "\n";
  if (this->ConversionParameters->GetNumberOfParameters() > 0)
  {
    os << indent << "ConversionParameters:\n";
    this->ConversionParameters->PrintSelf(os, indent.GetNextIndent());
  }
  Superclass::PrintSelf(os, indent);
}

//----------------------------------------------------------------------------
vtkSegmentationConverterRule* vtkSegmentationConverterRule::Clone()
{
  vtkSegmentationConverterRule* clone = this->CreateRuleInstance();
  clone->ConversionParameters->DeepCopy(this->ConversionParameters);
  return clone;
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverterRule::CreateTargetRepresentation(vtkSegment* segment)
{
  // Get target representation
  vtkSmartPointer<vtkDataObject> targetRepresentation = segment->GetRepresentation(this->GetTargetRepresentationName());

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
void vtkSegmentationConverterRule::GetRuleConversionParameters(
  vtkSegmentationConversionParameters* conversionParameters)
{
  // Copy rule conversion parameters into aggregated path parameters
  if (!conversionParameters)
  {
    vtkErrorMacro("GetRuleConversionParameters failed: invalid conversionParameters");
    return;
  }
  int numberOfParameters = this->ConversionParameters->GetNumberOfParameters();
  for (int parameterIndex = 0; parameterIndex < numberOfParameters; parameterIndex++)
  {
    conversionParameters->CopyParameter(this->ConversionParameters, parameterIndex);
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverterRule::SetConversionParameter(const std::string& name,
                                                          const std::string& value,
                                                          const std::string& description /*=""*/)
{
  this->ConversionParameters->SetValue(name, value);
  if (!description.empty())
  {
    this->ConversionParameters->SetDescription(name, description);
  }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverterRule::GetConversionParameter(const std::string& name)
{
  return this->ConversionParameters->GetValue(name);
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverterRule::GetConversionParameterDescription(const std::string& name)
{
  return this->ConversionParameters->GetDescription(name);
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverterRule::HasConversionParameter(const std::string& name)
{
  return (this->ConversionParameters->GetIndexFromName(name) >= 0);
}
