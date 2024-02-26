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
  Superclass::PrintSelf(os, indent);
  os << indent << "Rules:\n";
  int ruleIndex = 0;
  for (auto rule : this->ConverterRules)
  {
    os << indent.GetNextIndent() << "Rule[" << ruleIndex++ << "]:\n";
    rule->PrintSelf(os, indent.GetNextIndent().GetNextIndent());
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
  for (auto& rule : aConverter->ConverterRules)
  {
    int numberOfParameters = rule->ConversionParameters->GetNumberOfParameters();
    for (int parameterIndex = 0; parameterIndex < numberOfParameters; parameterIndex++)
    {
      this->SetConversionParameter(rule->ConversionParameters->GetName(parameterIndex),
                                   rule->ConversionParameters->GetValue(parameterIndex));
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

  int extent[6] = { 0, -1, 0, -1, 0, -1 };
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
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      geometryStream << numberToString.Convert(geometryMatrix->GetElement(i, j)) << SERIALIZED_GEOMETRY_SEPARATOR;
    }
  }

  for (int i = 0; i < 6; i++)
  {
    geometryStream << extent[i] << SERIALIZED_GEOMETRY_SEPARATOR;
  }

  return geometryStream.str();
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverter::DeserializeImageGeometry(std::string geometryString,
                                                        vtkMatrix4x4* geometryMatrix,
                                                        int extent[6])
{
  if (!geometryMatrix || geometryString.empty())
  {
    return false;
  }

  size_t separatorPosition = geometryString.find(SERIALIZED_GEOMETRY_SEPARATOR);
  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (separatorPosition == std::string::npos)
      {
        vtkErrorWithObjectMacro(
          geometryMatrix,
          "DeserializeImageGeometry: Unable to parse serialized image geometry! Error at matrix element " << i << ","
                                                                                                          << j);
        return false;
      }

      double element = vtkVariant(geometryString.substr(0, separatorPosition)).ToDouble();
      geometryMatrix->SetElement(i, j, element);

      geometryString = geometryString.substr(separatorPosition + SERIALIZED_GEOMETRY_SEPARATOR.size());
      separatorPosition = geometryString.find(SERIALIZED_GEOMETRY_SEPARATOR);
    }
  }

  // Parse image extent
  for (int i = 0; i < 6; i++)
  {
    if (separatorPosition == std::string::npos)
    {
      vtkErrorWithObjectMacro(
        geometryMatrix,
        "DeserializeImageGeometry: Unable to parse serialized image geometry! Error at extent index " << i);
      return false;
    }

    extent[i] = vtkVariant(geometryString.substr(0, separatorPosition)).ToInt();
    geometryString = geometryString.substr(separatorPosition + SERIALIZED_GEOMETRY_SEPARATOR.size());
    separatorPosition = geometryString.find(SERIALIZED_GEOMETRY_SEPARATOR);
  }

  return true;
}

//----------------------------------------------------------------------------
bool vtkSegmentationConverter::DeserializeImageGeometry(std::string geometryString,
                                                        vtkOrientedImageData* orientedImageData,
                                                        bool allocateScalars /*=true*/,
                                                        int scalarType /*=VTK_VOID*/,
                                                        int numberOfScalarsComponents /*=-1*/)
{
  if (!orientedImageData)
  {
    vtkGenericWarningMacro("DeserializeImageGeometry failed: invalid orientedImageData");
    return false;
  }

  // Parse image geometry matrix in geometry string
  vtkSmartPointer<vtkMatrix4x4> geometryMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
  int extent[6] = { 0, -1, 0, -1, 0, -1 };
  if (!vtkSegmentationConverter::DeserializeImageGeometry(geometryString, geometryMatrix, extent))
  {
    vtkErrorWithObjectMacro(orientedImageData,
                            "DeserializeImageGeometry: Failed to de-serialize geometry string " << geometryString);
    return false;
  }

  orientedImageData->SetGeometryFromImageToWorldMatrix(geometryMatrix);
  orientedImageData->SetExtent(extent);

  if (allocateScalars)
  {
    int allocateScalarType = ((scalarType == VTK_VOID) ? orientedImageData->GetScalarType() : scalarType);
    int allocateNumberOfScalarsComponents =
      ((numberOfScalarsComponents < 0) ? orientedImageData->GetNumberOfScalarComponents() : numberOfScalarsComponents);
    orientedImageData->AllocateScalars(allocateScalarType, allocateNumberOfScalarsComponents);
  }

  return true;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::SetConversionParameters(vtkSegmentationConversionParameters* parameters)
{
  int numberOfParameters = parameters->GetNumberOfParameters();
  for (int parameterIndex = 0; parameterIndex < numberOfParameters; parameterIndex++)
  {
    this->SetConversionParameter(parameters->GetName(parameterIndex), parameters->GetValue(parameterIndex));
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::SetConversionParameter(const std::string& name,
                                                      const std::string& value,
                                                      const std::string& description /*=""*/)
{
  // Cannot set parameter if any property contains a separator character
  if (name.find(SERIALIZATION_SEPARATOR) != std::string::npos
      || name.find(SERIALIZATION_SEPARATOR_INNER) != std::string::npos
      || value.find(SERIALIZATION_SEPARATOR) != std::string::npos
      || value.find(SERIALIZATION_SEPARATOR_INNER) != std::string::npos
      || description.find(SERIALIZATION_SEPARATOR) != std::string::npos
      || description.find(SERIALIZATION_SEPARATOR_INNER) != std::string::npos)
  {
    vtkErrorMacro("SetConversionParameter: Conversion parameter '"
                  << name << " name, value, or description contains a separator character so it cannot be set!");
    return;
  }

  // Set conversion parameter to each converter having that parameter
  bool parameterFound = false;
  ConverterRulesListType::iterator ruleIt;
  for (ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end(); ++ruleIt)
  {
    if ((*ruleIt)->HasConversionParameter(name))
    {
      (*ruleIt)->SetConversionParameter(name, value, description);
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

  vtkErrorMacro("GetConversionParameterDescription: Conversion parameter '" << name
                                                                            << "' not found in converter rules!");
  return "";
}

//----------------------------------------------------------------------------
vtkSegmentationConversionPath* vtkSegmentationConverter::GetCheapestPath(vtkSegmentationConversionPaths* paths)
{
  unsigned int cheapestPathCost = VTK_UNSIGNED_INT_MAX;
  unsigned int cheapestPathNumberOfConversions = 0;
  vtkSegmentationConversionPath* cheapestPath = nullptr;
  vtkSegmentationConversionPath* path = nullptr;
  vtkCollectionSimpleIterator it;
  for (paths->InitTraversal(it); (path = paths->GetNextPath(it));)
  {
    if (path->GetCost() <= cheapestPathCost)
    {
      size_t numberOfConversions = path->GetNumberOfRules();
      if (path->GetCost() == cheapestPathCost)
      {
        // If the path cost is exactly the same then compare the number of conversions too
        if (numberOfConversions > cheapestPathNumberOfConversions)
        {
          // Cost is the same but this path contains more conversions, so it is not cheaper
          continue;
        }
      }
      cheapestPathCost = path->GetCost();
      cheapestPathNumberOfConversions = (unsigned int)numberOfConversions;
      cheapestPath = path;
    }
  }
  return cheapestPath;
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetPossibleConversions(const std::string& sourceRepresentationName,
                                                      const std::string& targetRepresentationName,
                                                      vtkSegmentationConversionPaths* paths)
{
  paths->RemoveAllItems();
  vtkNew<vtkStringArray> skipRepresentations;
  this->FindPath(sourceRepresentationName, targetRepresentationName, paths, skipRepresentations);
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::FindPath(const std::string& sourceRepresentationName,
                                        const std::string& targetRepresentationName,
                                        vtkSegmentationConversionPaths* pathsToSource,
                                        vtkStringArray* skipRepresentations)
{
  if (sourceRepresentationName == targetRepresentationName)
  {
    vtkErrorMacro("FindPath failed: source and target representation names are the same - "
                  << sourceRepresentationName);
    return;
  }
  // try to find a path through all the available rules from this representation
  RulesListType& rulesFromSourceRepresentation = this->RulesGraph[sourceRepresentationName];
  if (rulesFromSourceRepresentation.empty())
  {
    // dead end, no more rules from source
    return;
  }

  // Get all the paths from source to the target
  vtkNew<vtkSegmentationConversionPaths> pathsFromSource;

  // skipRepresentations is reused, so we don't modify it
  vtkNew<vtkStringArray> skipRepresentationsNew;
  skipRepresentationsNew->DeepCopy(skipRepresentations);
  skipRepresentationsNew->InsertNextValue(sourceRepresentationName);

  for (RulesListType::iterator representationRuleIt = rulesFromSourceRepresentation.begin();
       representationRuleIt != rulesFromSourceRepresentation.end();
       ++representationRuleIt)
  {
    if (skipRepresentationsNew->LookupValue((*representationRuleIt)->GetTargetRepresentationName()) >= 0)
    {
      // representation has to be ignored
      continue;
    }
    const std::string& thisRuleTargetRepresentationName = (*representationRuleIt)->GetTargetRepresentationName();
    if (thisRuleTargetRepresentationName == targetRepresentationName)
    {
      // this rule leads to target directly
      vtkNew<vtkSegmentationConversionPath> pathFromSource;
      pathFromSource->AddRule(*representationRuleIt);
      pathsFromSource->AddPath(pathFromSource);
    }
    else
    {
      // this rule may lead to the target indirectly
      vtkNew<vtkSegmentationConversionPaths> pathsFromNext;
      this->FindPath(thisRuleTargetRepresentationName, targetRepresentationName, pathsFromNext, skipRepresentationsNew);
      if (pathsFromNext->GetNumberOfPaths() > 0)
      {
        vtkSegmentationConversionPath* pathFromNext = nullptr;
        vtkCollectionSimpleIterator it;
        for (pathsFromNext->InitTraversal(it); (pathFromNext = pathsFromNext->GetNextPath(it));)
        {
          vtkNew<vtkSegmentationConversionPath> pathFromSource;
          pathFromSource->AddRule(*representationRuleIt);
          pathFromSource->AddRules(pathFromNext);
          pathsFromSource->AddPath(pathFromSource);
        }
      }
    }
  }

  if (pathsFromSource->GetNumberOfPaths() == 0)
  {
    // no paths from source to the target
    return;
  }

  if (pathsToSource->GetNumberOfPaths() == 0)
  {
    // pathsToSource to source is empty, so we are at the starting point
    pathsToSource->AddPaths(pathsFromSource);
    return;
  }

  if (pathsFromSource->GetNumberOfPaths() == 1)
  {
    // Special case: there is just one possible continuation from source
    // just append the only possible continuation here to all known paths to the source and return
    vtkSegmentationConversionPath* pathToSource = nullptr;
    vtkCollectionSimpleIterator it;
    for (pathsToSource->InitTraversal(it); (pathToSource = pathsToSource->GetNextPath(it));)
    {
      pathToSource->AddRules(pathsFromSource->GetPath(0));
    }
    return;
  }

  // There are multiple possible continuations from source, append that to all known paths
  // to source (we'll have a full combination of all the paths to source and from source)
  vtkNew<vtkSegmentationConversionPaths> pathsToSourceOriginal;
  pathsToSourceOriginal->AddPaths(pathsToSource); // first save all the possible paths to source
  pathsToSource->RemoveAllItems();
  vtkSegmentationConversionPath* pathFromSource = nullptr;
  vtkCollectionSimpleIterator itFromSource;
  for (pathsFromSource->InitTraversal(itFromSource); (pathFromSource = pathsFromSource->GetNextPath(itFromSource));)
  {
    // append each path from source to a copy of the original pathsToSource
    vtkSegmentationConversionPath* pathToSource = nullptr;
    vtkCollectionSimpleIterator itToSource;
    for (pathsToSourceOriginal->InitTraversal(itToSource);
         (pathToSource = pathsToSourceOriginal->GetNextPath(itToSource));)
    {
      vtkNew<vtkSegmentationConversionPath> path;
      path->AddRules(pathToSource);
      path->AddRules(pathFromSource);
      pathsToSource->AddPath(path);
    }
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::RebuildRulesGraph()
{
  this->RulesGraph.clear();
  for (ConverterRulesListType::iterator ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end();
       ++ruleIt)
  {
    this->RulesGraph[ruleIt->GetPointer()->GetSourceRepresentationName()].push_back(ruleIt->GetPointer());
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetAvailableRepresentationNames(std::set<std::string>& representationNames)
{
  representationNames.clear();
  for (ConverterRulesListType::iterator ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end();
       ++ruleIt)
  {
    representationNames.insert(std::string((*ruleIt)->GetSourceRepresentationName()));
    representationNames.insert(std::string((*ruleIt)->GetTargetRepresentationName()));
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetConversionParametersForPath(vtkSegmentationConversionParameters* conversionParameters,
                                                              vtkSegmentationConversionPath* path)
{
  conversionParameters->RemoveAllParameters();
  int numberOfRules = path->GetNumberOfRules();
  for (int ruleIndex = 0; ruleIndex < numberOfRules; ++ruleIndex)
  {
    path->GetRule(ruleIndex)->GetRuleConversionParameters(conversionParameters);
  }
}

//----------------------------------------------------------------------------
void vtkSegmentationConverter::GetAllConversionParameters(vtkSegmentationConversionParameters* conversionParameters)
{
  conversionParameters->RemoveAllParameters();
  for (ConverterRulesListType::iterator ruleIt = this->ConverterRules.begin(); ruleIt != this->ConverterRules.end();
       ++ruleIt)
  {
    (*ruleIt)->GetRuleConversionParameters(conversionParameters);
  }
}

//----------------------------------------------------------------------------
std::string vtkSegmentationConverter::SerializeAllConversionParameters()
{
  std::stringstream ssParameters;
  vtkNew<vtkSegmentationConversionParameters> parameters;
  this->GetAllConversionParameters(parameters);

  int numberOfParameters = parameters->GetNumberOfParameters();
  for (int parameterIndex = 0; parameterIndex < numberOfParameters; parameterIndex++)
  {
    ssParameters << parameters->GetName(parameterIndex) << SERIALIZATION_SEPARATOR_INNER
                 << parameters->GetValue(parameterIndex) << SERIALIZATION_SEPARATOR_INNER
                 << parameters->GetDescription(parameterIndex) << SERIALIZATION_SEPARATOR;
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
    parameterString = parameterString.substr(innerSeparatorPosition + 1);
    innerSeparatorPosition = parameterString.find(SERIALIZATION_SEPARATOR_INNER);
    std::string value = parameterString.substr(0, innerSeparatorPosition);
    parameterString = parameterString.substr(innerSeparatorPosition + 1);
    innerSeparatorPosition = parameterString.find(SERIALIZATION_SEPARATOR_INNER);
    std::string description = parameterString.substr(0, innerSeparatorPosition);
    this->SetConversionParameter(name, value, description);

    conversionParametersString = conversionParametersString.substr(separatorPosition + 1);
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
  std::string geometryString =
    this->GetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName());
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

  // Transform geometry image using input transform (geometry only, so the non-linear transform is not applied to the
  // volume)
  vtkOrientedImageDataResample::TransformOrientedImage(geometryImage, transform, true);

  // Set reference image geometry parameter from oriented image data
  std::string newGeometryString = vtkSegmentationConverter::SerializeImageGeometry(geometryImage);
  if (newGeometryString.empty())
  {
    vtkErrorMacro("ApplyTransformOnReferenceImageGeometry: Failed to serialize new image geometry");
    return;
  }
  this->SetConversionParameter(vtkSegmentationConverter::GetReferenceImageGeometryParameterName(), newGeometryString);
}
