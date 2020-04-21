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
#include "vtkSegmentationConverter.h"
#include "vtkSegmentationConverterFactory.h"
#include "vtkOrientedImageData.h"
#include "vtkOrientedImageDataResample.h"
#include "vtkSegmentationConverterRule.h"

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkMatrix4x4.h>
#include <vtkNumberToString.h>
#include <vtkImageData.h>
#include <vtkTransform.h>
#include <vtkVariant.h>

// STD includes
#include <sstream>

//----------------------------------------------------------------------------
static const std::string SERIALIZED_GEOMETRY_SEPARATOR = ";";
static const std::string SERIALIZATION_SEPARATOR = "&";
static const std::string SERIALIZATION_SEPARATOR_INNER = "|";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSegmentationConverter);

//----------------------------------------------------------------------------
vtkSegmentationConverter::vtkSegmentationConverter()
{
  // Get default converter rules from factory
  vtkSegmentationConverterFactory::GetInstance()->CopyConverterRules(this->ConverterRules);
  this->RebuildRulesGraph();
}

//----------------------------------------------------------------------------
vtkSegmentationConverter::~vtkSegmentationConverter() = default;

//----------------------------------------------------------------------------
void vtkSegmentationConverter::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);

  ConverterRulesListType::iterator ruleIt;
  for (ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    vtkSegmentationConverterRule* rule = (*ruleIt);
    os << indent << "Rule:   " << (rule->GetSourceRepresentationName() ? rule->GetSourceRepresentationName() : "NULL") << " -> " << (rule->GetTargetRepresentationName() ? rule->GetTargetRepresentationName() : "NULL") << "\n";
    vtkSegmentationConverterRule::ConversionParameterListType::iterator paramIt;
    for (paramIt = rule->ConversionParameters.begin(); paramIt != rule->ConversionParameters.end(); ++paramIt)
      {
      os << indent << "  Parameter:   " << paramIt->first << " = " << paramIt->second.first << " (" << paramIt->second.second << ")\n";
      }
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::DeepCopy(vtkSegmentationConverter* aConverter)
{
  if (!aConverter)
    {
    return;
    }

  // Copy all conversion parameters
  ConverterRulesListType::iterator ruleIt;
  for (ruleIt = aConverter->ConverterRules.begin(); ruleIt != aConverter->ConverterRules.end(); ++ruleIt)
    {
    vtkSegmentationConverterRule::ConversionParameterListType::iterator paramIt;
    for (paramIt = (*ruleIt)->ConversionParameters.begin(); paramIt != (*ruleIt)->ConversionParameters.end(); ++paramIt)
      {
      this->SetConversionParameter(paramIt->first, paramIt->second.first);
      }
    }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::SerializeImageGeometry(vtkOrientedImageData* orientedImageData)
{
  if (!orientedImageData)
    {
    return "";
    }

  vtkSmartPointer<vtkMatrix4x4> geometryMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  orientedImageData->GetImageToWorldMatrix(geometryMatrix);

  return vtkSegmentationConverter::SerializeImageGeometry(geometryMatrix, orientedImageData);
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::SerializeImageGeometry(vtkMatrix4x4* geometryMatrix, vtkImageData* imageData)
{
  if (!imageData)
    {
    return "";
    }

  int extent[6] = {0,-1,0,-1,0,-1};
  imageData->GetExtent(extent);

  return vtkSegmentationConverter::SerializeImageGeometry(geometryMatrix, extent);
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::SerializeImageGeometry(vtkMatrix4x4* geometryMatrix, int extent[6])
{
  if (!geometryMatrix)
    {
    return "";
    }

  vtkNumberToString numberToString;
  std::stringstream geometryStream;
  for (int i=0; i<4; i++)
    {
    for (int j=0; j<4; j++)
      {
      geometryStream << numberToString(geometryMatrix->GetElement(i, j)) << SERIALIZED_GEOMETRY_SEPARATOR;
      }
    }

  for (int i=0; i<6; i++)
    {
    geometryStream << extent[i] << SERIALIZED_GEOMETRY_SEPARATOR;
    }

  return geometryStream.str();
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverter::DeserializeImageGeometry(std::string geometryString, vtkMatrix4x4* geometryMatrix, int extent[6])
{
  if (!geometryMatrix || geometryString.empty())
    {
    return false;
    }

  size_t separatorPosition = geometryString.find(SERIALIZED_GEOMETRY_SEPARATOR);
  for (int i=0; i<4; i++)
    {
    for (int j=0; j<4; j++)
      {
      if (separatorPosition == std::string::npos)
        {
        vtkErrorWithObjectMacro(geometryMatrix, "DeserializeImageGeometry: Unable to parse serialized image geometry! Error at matrix element " << i << "," << j);
        return false;
        }

      double element = vtkVariant(geometryString.substr(0, separatorPosition)).ToDouble();
      geometryMatrix->SetElement(i,j,element);

      geometryString = geometryString.substr(separatorPosition + SERIALIZED_GEOMETRY_SEPARATOR.size());
      separatorPosition = geometryString.find(SERIALIZED_GEOMETRY_SEPARATOR);
      }
    }

  // Parse image extent
  for (int i=0; i<6; i++)
    {
    if (separatorPosition == std::string::npos)
      {
      vtkErrorWithObjectMacro(geometryMatrix, "DeserializeImageGeometry: Unable to parse serialized image geometry! Error at extent index " << i);
      return false;
      }

    extent[i] = vtkVariant(geometryString.substr(0, separatorPosition)).ToInt();
    geometryString = geometryString.substr(separatorPosition + SERIALIZED_GEOMETRY_SEPARATOR.size());
    separatorPosition = geometryString.find(SERIALIZED_GEOMETRY_SEPARATOR);
    }

  return true;
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverter::DeserializeImageGeometry(
    std::string geometryString,
    vtkOrientedImageData* orientedImageData,
    bool allocateScalars/*=true*/,
    int scalarType/*=VTK_VOID*/,
    int numberOfScalarsComponents/*=-1*/)
{
  if (!orientedImageData)
    {
    vtkGenericWarningMacro("DeserializeImageGeometry failed: invalid orientedImageData");
    return false;
    }

  // Parse image geometry matrix in geometry string
  vtkSmartPointer<vtkMatrix4x4> geometryMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  int extent[6] = {0,-1,0,-1,0,-1};
  if (!vtkSegmentationConverter::DeserializeImageGeometry(geometryString, geometryMatrix, extent))
    {
    vtkErrorWithObjectMacro(orientedImageData, "DeserializeImageGeometry: Failed to de-serialize geometry string " << geometryString);
    return false;
    }

  orientedImageData->SetGeometryFromImageToWorldMatrix(geometryMatrix);
  orientedImageData->SetExtent(extent);

  if (allocateScalars)
    {
    int allocateScalarType = ((scalarType == VTK_VOID) ? orientedImageData->GetScalarType() : scalarType);
    int allocateNumberOfScalarsComponents = ((numberOfScalarsComponents < 0) ? orientedImageData->GetNumberOfScalarComponents() : numberOfScalarsComponents);
    orientedImageData->AllocateScalars(allocateScalarType, allocateNumberOfScalarsComponents);
    }

  return true;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::SetConversionParameters(vtkSegmentationConverterRule::ConversionParameterListType parameters)
{
  vtkSegmentationConverterRule::ConversionParameterListType::iterator paramIt;
  for (paramIt = parameters.begin(); paramIt != parameters.end(); ++paramIt)
    {
    this->SetConversionParameter(paramIt->first, paramIt->second.first);
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::SetConversionParameter(const std::string& name, const std::string& value, const std::string& description/*=""*/)
{
  // Cannot set parameter if any property contains a separator character
  if ( name.find(SERIALIZATION_SEPARATOR) != std::string::npos || name.find(SERIALIZATION_SEPARATOR_INNER) != std::string::npos
    || value.find(SERIALIZATION_SEPARATOR) != std::string::npos || value.find(SERIALIZATION_SEPARATOR_INNER) != std::string::npos
    || description.find(SERIALIZATION_SEPARATOR) != std::string::npos || description.find(SERIALIZATION_SEPARATOR_INNER) != std::string::npos )
    {
    vtkErrorMacro("SetConversionParameter: Conversion parameter '" << name << " name, value, or description contains a separator character so it cannot be set!");
    return;
    }

  // Set conversion parameter to each converter having that parameter
  bool parameterFound = false;
  ConverterRulesListType::iterator ruleIt;
  for (ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    if ((*ruleIt)->HasConversionParameter(name))
      {
      (*ruleIt)->SetConversionParameter(name,value,description);
      parameterFound = true;
      }
    }

  if (!parameterFound)
    {
    vtkErrorMacro("SetConversionParameter: Conversion parameter '" << name << "' not found in converter rules!");
    }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::GetConversionParameter(const std::string& name)
{
  ConverterRulesListType::iterator ruleIt;
  for (ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    if ((*ruleIt)->HasConversionParameter(name))
      {
      return (*ruleIt)->GetConversionParameter(name);
      }
    }

  vtkErrorMacro("GetConversionParameter: Conversion parameter '" << name << "' not found in converter rules!");
  return "";
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::GetConversionParameterDescription(const std::string& name)
{
  ConverterRulesListType::iterator ruleIt;
  for (ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    if ((*ruleIt)->HasConversionParameter(name))
      {
      return (*ruleIt)->GetConversionParameterDescription(name);
      }
    }

  vtkErrorMacro("GetConversionParameterDescription: Conversion parameter '" << name << "' not found in converter rules!");
  return "";
}

//----------------------------------------------------------------------------
vtkSegmentationConverter::ConversionPathType vtkSegmentationConverter::GetCheapestPath(const ConversionPathAndCostListType &pathsCosts)
{
  unsigned int cheapestPathCost = vtkSegmentationConverterRule::GetConversionInfiniteCost();
  unsigned int cheapestPathNumberOfConversions = 0;
  ConversionPathType cheapestPath;
  for (ConversionPathAndCostListType::const_iterator pathIt = pathsCosts.begin(); pathIt != pathsCosts.end(); ++pathIt)
    {
    if (pathIt->second <= cheapestPathCost)
      {
      size_t numberOfConversions = pathIt->first.size();
      if (pathIt->second == cheapestPathCost)
        {
        // If the path cost is exactly the same then compare the number of conversions too
        if (numberOfConversions > cheapestPathNumberOfConversions)
          {
          // Cost is the same but this path contains more conversions, so it is not cheaper
          continue;
          }
        }
      cheapestPathCost = pathIt->second;
      cheapestPathNumberOfConversions = (unsigned int)numberOfConversions;
      cheapestPath = pathIt->first;
      }
    }
  return cheapestPath;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetPossibleConversions(const std::string& sourceRepresentationName, const std::string& targetRepresentationName, ConversionPathAndCostListType &pathsCosts)
{
  pathsCosts.clear();
  std::set<std::string> skipRepresentations;
  this->FindPath(sourceRepresentationName, targetRepresentationName, pathsCosts, skipRepresentations);
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::FindPath(const std::string& sourceRepresentationName, const std::string& targetRepresentationName, ConversionPathAndCostListType &pathsCosts, std::set<std::string>& skipRepresentations)
{
  if (sourceRepresentationName == targetRepresentationName)
    {
    vtkErrorMacro("FindPath failed: source and target representation names are the same - "<<sourceRepresentationName);
    return;
    }
  // try to find a path through all the available rules from this representation
  RulesListType& rulesFromSourceRepresentation = this->RulesGraph[sourceRepresentationName];
  if (rulesFromSourceRepresentation.empty())
    {
    // dead end, no more rules from here
    return;
    }

  // Get all the paths from here to the target
  ConversionPathAndCostListType pathsCostsFromHere;
  skipRepresentations.insert(sourceRepresentationName);
  for (RulesListType::iterator representationRuleIt=rulesFromSourceRepresentation.begin(); representationRuleIt!=rulesFromSourceRepresentation.end(); ++representationRuleIt)
    {
    if (skipRepresentations.find((*representationRuleIt)->GetTargetRepresentationName()) != skipRepresentations.end())
      {
      // representation has to be ignored
      continue;
      }
    const std::string& thisRuleTargetRepresentationName = (*representationRuleIt)->GetTargetRepresentationName();
    if (thisRuleTargetRepresentationName==targetRepresentationName)
      {
      // this rule leads to target directly
      ConversionPathAndCostType pathCostFromHere;
      pathCostFromHere.first.push_back(*representationRuleIt); // path
      pathCostFromHere.second = (*representationRuleIt)->GetConversionCost(); // cost
      pathsCostsFromHere.push_back(pathCostFromHere);
      }
    else
      {
      // this rule may lead to the target indirectly
      ConversionPathAndCostListType pathsCostsFromNext;
      this->FindPath(thisRuleTargetRepresentationName, targetRepresentationName, pathsCostsFromNext, skipRepresentations);
      if (!pathsCostsFromNext.empty())
        {
        for (ConversionPathAndCostListType::iterator pathCostFromNextIt = pathsCostsFromNext.begin(); pathCostFromNextIt != pathsCostsFromNext.end(); ++pathCostFromNextIt)
          {
          ConversionPathAndCostType pathCostFromHere;
          pathCostFromHere.first.push_back(*representationRuleIt); // path
          pathCostFromHere.second = (*representationRuleIt)->GetConversionCost(); // cost
          pathCostFromHere.first.insert(pathCostFromHere.first.end(), pathCostFromNextIt->first.begin(), pathCostFromNextIt->first.end()); // path append
          pathCostFromHere.second += pathCostFromNextIt->second; // cost
          pathsCostsFromHere.push_back(pathCostFromHere);
          }
        }
      }
    }
  // skipRepresentations is reused, so make sure we restore the original contents
  skipRepresentations.erase(sourceRepresentationName);

  if (pathsCostsFromHere.empty())
    {
    // no paths from here to the target
    return;
    }

  if (pathsCosts.empty())
    {
    // pathsCosts to here is empty, so we are at the starting point
    pathsCosts = pathsCostsFromHere;
    return;
    }

  if (pathsCostsFromHere.size()==1)
    {
    // Special case: there is just one possible continuation from here
    // just append the only possible continuation here to all known paths to here and return
    for (ConversionPathAndCostListType::iterator pathsCostsIt = pathsCosts.begin(); pathsCostsIt != pathsCosts.end(); ++pathsCostsIt)
      {
      pathsCostsIt->first.insert(pathsCostsIt->first.end(),pathsCostsFromHere[0].first.begin(),pathsCostsFromHere[0].first.end());
      pathsCostsIt->second += pathsCostsFromHere[0].second;
      }
    return;
    }

  // There are multiple possible continuations from here, append that to all known paths to here (we'll have a full combination of all the paths to here and from here)
  ConversionPathAndCostListType pathsCostsToHere = pathsCosts; // first save all the possible paths to here
  for (ConversionPathAndCostListType::iterator pathCostsFromHereIt = pathsCostsFromHere.begin(); pathCostsFromHereIt != pathsCostsFromHere.end(); ++pathCostsFromHereIt)
    {
    if (pathCostsFromHereIt == pathsCostsFromHere.begin())
      {
      // first path from here, just append it to the current pathsCosts
      for (ConversionPathAndCostListType::iterator pathsCostsToHereIt = pathsCosts.begin(); pathsCostsToHereIt != pathsCosts.end(); ++pathsCostsToHereIt)
        {
        pathsCostsToHereIt->first.insert(pathsCostsToHereIt->first.end(), pathCostsFromHereIt->first.begin(), pathCostsFromHereIt->first.end());
        pathsCostsToHereIt->second += pathCostsFromHereIt->second;
        }
      }
    else
      {
      // additional path from here, append it to a copy of the original pathsCosts (pathsCosts to here)
      for (ConversionPathAndCostListType::iterator pathsCostsToHereIt = pathsCostsToHere.begin(); pathsCostsToHereIt != pathsCostsToHere.end(); ++pathsCostsToHereIt)
        {
        ConversionPathAndCostType pathCost;
        pathCost.first = pathsCostsToHereIt->first; // path
        pathCost.second = pathsCostsToHereIt->second; // cost
        pathCost.first.insert(pathCost.first.end(), pathCostsFromHereIt->first.begin(), pathCostsFromHereIt->first.end()); // path append
        pathCost.second += pathCostsFromHereIt->second; // cost
        pathsCosts.push_back(pathCost);
        }
      }
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::RebuildRulesGraph()
{
  this->RulesGraph.clear();
  for (ConverterRulesListType::iterator ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    this->RulesGraph[ruleIt->GetPointer()->GetSourceRepresentationName()].push_back(ruleIt->GetPointer());
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetAvailableRepresentationNames(std::set<std::string>& representationNames)
{
  representationNames.clear();
  for (ConverterRulesListType::iterator ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    representationNames.insert(std::string((*ruleIt)->GetSourceRepresentationName()));
    representationNames.insert(std::string((*ruleIt)->GetTargetRepresentationName()));
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetConversionParametersForPath(
    vtkSegmentationConverterRule::ConversionParameterListType& conversionParameters,
    const ConversionPathType& path)
{
  conversionParameters.clear();
  ConversionPathType::const_iterator ruleIt;
  for (ruleIt = path.begin(); ruleIt != path.end(); ++ruleIt)
    {
    (*ruleIt)->GetRuleConversionParameters(conversionParameters);
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetAllConversionParameters(vtkSegmentationConverterRule::ConversionParameterListType& conversionParameters)
{
  conversionParameters.clear();
  for (ConverterRulesListType::iterator ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
    {
    (*ruleIt)->GetRuleConversionParameters(conversionParameters);
    }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::SerializeAllConversionParameters()
{
  std::stringstream ssParameters;
  vtkSegmentationConverterRule::ConversionParameterListType parameters;
  this->GetAllConversionParameters(parameters);

  vtkSegmentationConverterRule::ConversionParameterListType::iterator paramIt;
  for (paramIt = parameters.begin(); paramIt != parameters.end(); ++paramIt)
    {
    ssParameters << paramIt->first << SERIALIZATION_SEPARATOR_INNER << paramIt->second.first << SERIALIZATION_SEPARATOR_INNER << paramIt->second.second << SERIALIZATION_SEPARATOR;
    }

  return ssParameters.str();
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::DeserializeConversionParameters(std::string conversionParametersString)
{
  size_t separatorPosition = conversionParametersString.find(SERIALIZATION_SEPARATOR);
  while (separatorPosition != std::string::npos)
    {
    std::string parameterString = conversionParametersString.substr(0, separatorPosition);

    size_t innerSeparatorPosition = parameterString.find(SERIALIZATION_SEPARATOR_INNER);
    std::string name = parameterString.substr(0, innerSeparatorPosition);
    parameterString = parameterString.substr(innerSeparatorPosition+1);
    innerSeparatorPosition = parameterString.find(SERIALIZATION_SEPARATOR_INNER);
    std::string value = parameterString.substr(0, innerSeparatorPosition);
    parameterString = parameterString.substr(innerSeparatorPosition+1);
    innerSeparatorPosition = parameterString.find(SERIALIZATION_SEPARATOR_INNER);
    std::string description = parameterString.substr(0, innerSeparatorPosition);
    this->SetConversionParameter(name, value, description);

    conversionParametersString = conversionParametersString.substr(separatorPosition+1);
    separatorPosition = conversionParametersString.find(SERIALIZATION_SEPARATOR);
    }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::ApplyTransformOnReferenceImageGeometry(vtkAbstractTransform* transform)
{
  if (!transform)
    {
    return;
    }
  // Get current reference geometry parameter
  std::string geometryString = this->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
  if (geometryString.empty())
    {
    vtkDebugMacro("ApplyTransformOnReferenceImageGeometry: Reference image geometry conversion parameter is empty");
    return;
    }

  // Deserialize parameter string into oriented image data
  vtkSmartPointer<vtkOrientedImageData> geometryImage = vtkSmartPointer<vtkOrientedImageData>::New();
  if (!vtkSegmentationConverter::DeserializeImageGeometry(geometryString, geometryImage))
    {
    vtkErrorMacro("ApplyTransformOnReferenceImageGeometry: Failed to get reference image geometry");
    return;
    }

  // Transform geometry image using input transform (geometry only, so the non-linear transform is not applied to the volume)
  vtkOrientedImageDataResample::TransformOrientedImage(geometryImage, transform, true);

  // Set reference image geometry parameter from oriented image data
  std::string newGeometryString = vtkSegmentationConverter::SerializeImageGeometry(geometryImage);
  if (newGeometryString.empty())
    {
    vtkErrorMacro("ApplyTransformOnReferenceImageGeometry: Failed to serialize new image geometry");
    return;
    }
  this->SetConversionParameter(
    vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), newGeometryString );
}
