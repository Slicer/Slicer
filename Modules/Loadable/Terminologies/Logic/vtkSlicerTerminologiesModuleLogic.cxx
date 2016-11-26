/*==============================================================================

  Program: 3D Slicer

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

// Terminologies includes
#include "vtkSlicerTerminologiesModuleLogic.h"

#include "vtkSlicerTerminologyCategory.h"
#include "vtkSlicerTerminologyType.h"

// MRMLLogic includes
#include <vtkMRMLScene.h>

// Slicer includes
#include "vtkLoggingMacros.h"

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkObjectFactory.h>
#include <vtkStringArray.h>
#include <vtkVariant.h>

// STD includes
#include <algorithm>

// JSON includes (requires Slicer_BUILD_PARAMETERSERIALIZER_SUPPORT)
#include <json/json.h>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologiesModuleLogic);

//---------------------------------------------------------------------------
class vtkSlicerTerminologiesModuleLogic::vtkInternal
{
public:
  typedef std::map<std::string, Json::Value> TerminologyMap;
  vtkInternal(vtkSlicerTerminologiesModuleLogic* external);
  ~vtkInternal();

  /// Utility function to get code in Json array
  /// \param foundIndex Output parameter for index of found object in input array. -1 if not found
  /// \return Json object if found, otherwise null Json object
  Json::Value GetCodeInArray(CodeIdentifier codeId, Json::Value jsonArray, int &foundIndex);
  
  /// Get root Json value for the terminology with given name
  Json::Value GetTerminologyRootByName(std::string terminologyName);

  /// Get category array Json value for a given terminology
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetCategoryArrayInTerminology(std::string terminologyName);
  /// Get category Json object from terminology with given category name
  /// \return Null Json value on failure, the category Json object otherwise
  Json::Value GetCategoryInTerminology(std::string terminologyName, CodeIdentifier categoryId);
  /// Populate \sa vtkSlicerTerminologyCategory from Json terminology
  bool PopulateTerminologyCategoryFromJson(Json::Value categoryObject, vtkSlicerTerminologyCategory* category);

  /// Get type array Json value for a given terminology and category
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetTypeArrayInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId);
  /// Get type Json object from a terminology category with given type name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetTypeInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId);
  /// Populate \sa vtkSlicerTerminologyType from Json terminology
  bool PopulateTerminologyTypeFromJson(Json::Value typeObject, vtkSlicerTerminologyType* type);

  /// Get type modifier array Json value for a given terminology, category, and type
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetTypeModifierArrayInTerminologyType(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId);
  /// Get type modifier Json object from a terminology, category, and type with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetTypeModifierInTerminologyType(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, CodeIdentifier modifierId);

  /// Get root Json value for the anatomic context with given name
  Json::Value GetAnatomicContextRootByName(std::string anatomicContextName);

  /// Get region array Json value for a given anatomic context
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetRegionArrayInAnatomicContext(std::string anatomicContextName);
  /// Get type Json object from an anatomic context with given region name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetRegionInAnatomicContext(std::string anatomicContextName, CodeIdentifier regionId);
  /// Populate \sa vtkSlicerTerminologyType from Json anatomic region
  bool PopulateRegionFromJson(Json::Value anatomicRegionObject, vtkSlicerTerminologyType* region);

  /// Get region modifier array Json value for a given anatomic context and region
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetRegionModifierArrayInRegion(std::string anatomicContextName, CodeIdentifier regionId);
  /// Get type modifier Json object from an anatomic context and region with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetRegionModifierInRegion(std::string anatomicContextName, CodeIdentifier regionId, CodeIdentifier modifierId);

  /// Convert a segmentation descriptor Json structure to a terminology context one
  /// \return Terminology context Json structure, Null Json value on failure
  Json::Value ConvertSegmentationDescriptorToTerminologyContext(Json::Value descriptorRoot, std::string contextName);
  /// Convert a segmentation descriptor Json structure to an anatomic context one
  /// \return Anatomic context Json structure, Null Json value on failure
  Json::Value ConvertSegmentationDescriptorToAnatomicContext(Json::Value descriptorRoot, std::string contextName);
  /// Copy basic identifier members from an identifier object into a Json object
  /// \return The code object with the identifiers set
  Json::Value GetJsonCodeFromIdentifier(Json::Value code, CodeIdentifier idenfifier);

public:
  /// Loaded terminologies. Key is the context name, value is the root item.
  TerminologyMap LoadedTerminologies;

  /// Loaded anatomical region contexts. Key is the context name, value is the root item.
  TerminologyMap LoadedAnatomicContexts;

private:
  vtkSlicerTerminologiesModuleLogic* External;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkInternal::vtkInternal(vtkSlicerTerminologiesModuleLogic* external)
: External(external)
{
}

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkInternal::~vtkInternal()
{
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCodeInArray(CodeIdentifier codeId, Json::Value jsonArray, int &foundIndex)
{
  if (!jsonArray.isArray())
    {
    return Json::Value();
    }

  // Traverse array and try to find the object with given identifier
  Json::ArrayIndex index = 0;
  while (jsonArray.isValidIndex(index))
    {
    Json::Value currentObject = jsonArray[index];
    if (currentObject.isObject())
      {
      Json::Value codingSchemeDesignator = currentObject["CodingSchemeDesignator"];
      Json::Value codeValue = currentObject["CodeValue"];
      if ( codingSchemeDesignator.isString() && !codeId.CodingSchemeDesignator.compare(codingSchemeDesignator.asString())
        && codeValue.isString() && !codeId.CodeValue.compare(codeValue.asString()) )
        {
        foundIndex = index;
        return currentObject;
        }
      }
    ++index;
    }

  // Not found
  foundIndex = -1;
  return Json::Value();
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTerminologyRootByName(std::string terminologyName)
{
  TerminologyMap::iterator termIt = this->LoadedTerminologies.find(terminologyName);
  if (termIt != this->LoadedTerminologies.end())
    {
    return termIt->second;
    }

  return Json::Value();
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryArrayInTerminology(std::string terminologyName)
{
  if (terminologyName.empty())
    {
    return Json::Value();
    }
  Json::Value root = this->GetTerminologyRootByName(terminologyName);
  if (root.isNull())
    {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find terminology root for context name '" << terminologyName << "'");
    return Json::Value();
    }

  Json::Value segmentationCodes = root["SegmentationCodes"];
  if (segmentationCodes.isNull())
    {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find SegmentationCodes member in terminology '" << terminologyName << "'");
    return Json::Value();
    }
  Json::Value categoryArray = segmentationCodes["Category"];
  if (!categoryArray.isArray())
    {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find Category array member in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  return categoryArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryInTerminology(std::string terminologyName, CodeIdentifier categoryId)
{
  if (categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value categoryArray = this->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.isNull())
    {
    vtkGenericWarningMacro("GetCategoryInTerminology: Failed to find category array in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  int index = -1;
  return this->GetCodeInArray(categoryId, categoryArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyCategoryFromJson(Json::Value categoryObject, vtkSlicerTerminologyCategory* category)
{
  if (!categoryObject.isObject() || !category)
    {
    return false;
    }

  Json::Value codeMeaning = categoryObject["CodeMeaning"];             // e.g. "Tissue" (mandatory)
  Json::Value codingScheme = categoryObject["CodingSchemeDesignator"]; // e.g. "SRT" (mandatory)
  Json::Value SNOMEDCTConceptID = categoryObject["SNOMEDCTConceptID"]; // e.g. "85756007"
  Json::Value UMLSConceptUID = categoryObject["UMLSConceptUID"];       // e.g. "C0040300"
  Json::Value cid = categoryObject["cid"];                             // e.g. "7051"
  Json::Value codeValue = categoryObject["CodeValue"];                 // e.g. "T-D0050" (mandatory)
  Json::Value contextGroupName = categoryObject["contextGroupName"];   // e.g. "Segmentation Property Categories"
  Json::Value showAnatomy = categoryObject["showAnatomy"];
  if (!codingScheme.isString() || !codeValue.isString() || !codeMeaning.isString())
    {
    vtkGenericWarningMacro("PopulateTerminologyCategoryFromJson: Unable to access mandatory category member");
    return false;
    }

  category->SetCodeMeaning(codeMeaning.asCString());
  category->SetCodingScheme(codingScheme.asCString());
  category->SetSNOMEDCTConceptID(SNOMEDCTConceptID.isString() ? SNOMEDCTConceptID.asCString() : NULL);
  category->SetUMLSConceptUID(UMLSConceptUID.isString()? UMLSConceptUID.asCString() : NULL);
  category->SetCid(cid.isString() ? cid.asCString() : NULL);
  category->SetCodeValue(codeValue.asCString());
  category->SetContextGroupName(contextGroupName.isString() ? contextGroupName.asCString() : NULL);
  if (showAnatomy.isString())
    {
    std::string showAnatomyStr = showAnatomy.asString();
    std::transform(showAnatomyStr.begin(), showAnatomyStr.end(), showAnatomyStr.begin(), ::tolower); // Make it lowercase for case-insensitive comparison
    category->SetShowAnatomy( showAnatomyStr.compare("true") ? false : true );
    }
  else if (showAnatomy.isBool())
    {
    category->SetShowAnatomy(showAnatomy.asBool());
    }
  else
    {
    category->SetShowAnatomy(true); // Default
    }

  return true;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeArrayInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId)
{
  if (categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value categoryObject = this->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.isNull())
    {
    vtkGenericWarningMacro("GetTypeArrayInTerminologyCategory: Failed to find category '" << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  Json::Value typeArray = categoryObject["Type"];
  if (!typeArray.isArray())
    {
    vtkGenericWarningMacro("GetTypeArrayInTerminologyCategory: Failed to find Type array member in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  return typeArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeInTerminologyCategory(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId)
{
  if (typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value typeArray = this->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.isNull())
    {
    vtkGenericWarningMacro("GetTypeInTerminologyCategory: Failed to find type array for category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  int index = -1;
  return this->GetCodeInArray(typeId, typeArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyTypeFromJson(Json::Value typeObject, vtkSlicerTerminologyType* type)
{
  if (!typeObject.isObject() || !type)
    {
    return false;
    }

  Json::Value recommendedDisplayRGBValue = typeObject["recommendedDisplayRGBValue"];
  Json::Value codeMeaning = typeObject["CodeMeaning"];             // e.g. "Artery" (mandatory)
  Json::Value codingScheme = typeObject["CodingSchemeDesignator"]; // e.g. "SRT" (mandatory)
  Json::Value slicerLabel = typeObject["3dSlicerLabel"];           // e.g. "artery"
  Json::Value SNOMEDCTConceptID = typeObject["SNOMEDCTConceptID"]; // e.g. "85756007"
  Json::Value UMLSConceptUID = typeObject["UMLSConceptUID"];       // e.g. "C0040300"
  Json::Value cid = typeObject["cid"];                             // e.g. "7051"
  Json::Value codeValue = typeObject["CodeValue"];                 // e.g. "T-D0050" (mandatory)
  Json::Value contextGroupName = typeObject["contextGroupName"];   // e.g. "Segmentation Property Categories"
  Json::Value modifier = typeObject["Modifier"];                   // Modifier array, containing modifiers of this type, e.g. "Left"
  if (!codingScheme.isString() || !codeValue.isString() || !codeMeaning.isString())
    {
    vtkGenericWarningMacro("PopulateTerminologyTypeFromJson: Unable to access mandatory type member");
    return false;
    }

  type->SetCodeMeaning(codeMeaning.asCString());
  type->SetCodingScheme(codingScheme.asCString());
  type->SetSlicerLabel(slicerLabel.isString() ? slicerLabel.asCString() : NULL);
  type->SetSNOMEDCTConceptID(SNOMEDCTConceptID.isString() ? SNOMEDCTConceptID.asCString() : NULL);
  type->SetUMLSConceptUID(UMLSConceptUID.isString()? UMLSConceptUID.asCString() : NULL);
  type->SetCid(cid.isString() ? cid.asCString() : NULL);
  type->SetCodeValue(codeValue.asCString());
  type->SetContextGroupName(contextGroupName.isString() ? contextGroupName.asCString() : NULL);

  if (recommendedDisplayRGBValue.isArray() && recommendedDisplayRGBValue.size() == 3)
    {
    if (recommendedDisplayRGBValue[0].isString())
      {
      type->SetRecommendedDisplayRGBValue( // Note: Casting directly to unsigned char fails
        (unsigned char)vtkVariant(recommendedDisplayRGBValue[0].asString()).ToInt(),
        (unsigned char)vtkVariant(recommendedDisplayRGBValue[1].asString()).ToInt(),
        (unsigned char)vtkVariant(recommendedDisplayRGBValue[2].asString()).ToInt() );
      }
    else if (recommendedDisplayRGBValue[0].isInt())
      {
      type->SetRecommendedDisplayRGBValue(
        (unsigned char)recommendedDisplayRGBValue[0].asInt(),
        (unsigned char)recommendedDisplayRGBValue[1].asInt(),
        (unsigned char)recommendedDisplayRGBValue[2].asInt() );
      }
    else
      {
      vtkGenericWarningMacro("PopulateTerminologyTypeFromJson: Unsupported data type for recommendedDisplayRGBValue");
      }
    }
  else
    {
    type->SetRecommendedDisplayRGBValue(127,127,127); // 'Invalid' gray
    }

  type->SetHasModifiers(modifier.isArray());

  return true;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeModifierArrayInTerminologyType(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId)
{
  if (typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value categoryObject = this->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.isNull())
    {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find category '" <<
      categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  Json::Value typeObject = this->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId);
  if (typeObject.isNull())
    {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find type '" << typeId.CodeMeaning << "' in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  Json::Value typeModifierArray = typeObject["Modifier"];
  if (!typeModifierArray.isArray())
    {
    return Json::Value();
    }

  return typeModifierArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeModifierInTerminologyType(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, CodeIdentifier modifierId)
{
  if (modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value typeModifierArray = this->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.isNull())
    {
    vtkGenericWarningMacro("GetTypeModifierInTerminologyType: Failed to find type modifier array for type '" << typeId.CodeMeaning << "' in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  int index = -1;
  return this->GetCodeInArray(modifierId, typeModifierArray, index);
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetAnatomicContextRootByName(std::string anatomicContextName)
{
  TerminologyMap::iterator anIt = this->LoadedAnatomicContexts.find(anatomicContextName);
  if (anIt != this->LoadedAnatomicContexts.end())
    {
    return anIt->second;
    }

  return Json::Value();
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionArrayInAnatomicContext(std::string anatomicContextName)
{
  if (anatomicContextName.empty())
    {
    return Json::Value();
    }
  Json::Value root = this->GetAnatomicContextRootByName(anatomicContextName);
  if (root.isNull())
    {
    vtkGenericWarningMacro("GetRegionArrayInAnatomicContext: Failed to find anatomic context root for context name '" << anatomicContextName << "'");
    return Json::Value();
    }

  Json::Value anatomicCodes = root["AnatomicCodes"];
  if (anatomicCodes.isNull())
    {
    vtkGenericWarningMacro("GetRegionArrayInAnatomicContext: Failed to find AnatomicCodes member in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }
  Json::Value anatomicRegionArray = anatomicCodes["AnatomicRegion"];
  if (!anatomicRegionArray.isArray())
    {
    vtkGenericWarningMacro("GetRegionArrayInAnatomicContext: Failed to find AnatomicRegion array member in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  return anatomicRegionArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionInAnatomicContext(std::string anatomicContextName, CodeIdentifier regionId)
{
  if (regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value regionArray = this->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.isNull())
    {
    vtkGenericWarningMacro("GetRegionInAnatomicContext: Failed to find region array for anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  int index = -1;
  return this->GetCodeInArray(regionId, regionArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateRegionFromJson(Json::Value anatomicRegionObject, vtkSlicerTerminologyType* region)
{
  return this->PopulateTerminologyTypeFromJson(anatomicRegionObject, region);
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierArrayInRegion(std::string anatomicContextName, CodeIdentifier regionId)
{
  if (regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value regionObject = this->GetRegionInAnatomicContext(anatomicContextName, regionId);
  if (regionObject.isNull())
    {
    vtkGenericWarningMacro("GetRegionModifierArrayInAnatomicRegion: Failed to find region '" <<
      regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  Json::Value regionModifierArray = regionObject["Modifier"];
  if (!regionModifierArray.isArray())
    {
    vtkGenericWarningMacro("GetRegionModifierArrayInAnatomicRegion: Failed to find Modifier array member in region '" <<
      regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  return regionModifierArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierInRegion(
  std::string anatomicContextName, CodeIdentifier regionId, CodeIdentifier modifierId )
{
  if (modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
    {
    return Json::Value();
    }
  Json::Value regionModifierArray = this->GetRegionModifierArrayInRegion(anatomicContextName, regionId);
  if (regionModifierArray.isNull())
    {
    vtkGenericWarningMacro("GetRegionModifierInRegion: Failed to find region modifier array for region '" <<
      regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  int index = -1;
  return this->GetCodeInArray(modifierId, regionModifierArray, index);
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::ConvertSegmentationDescriptorToTerminologyContext(Json::Value descriptorRoot, std::string contextName)
{
  if (descriptorRoot.isNull() || contextName.empty())
    {
    return Json::Value();
    }

  // Get segment attributes
  Json::Value segmentAttributesArray = descriptorRoot["segmentAttributes"];
  if (!segmentAttributesArray.isArray())
    {
    vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Invalid segmentAttributes member");
    return Json::Value();
    }

  // Use terminology with context name if exists
  Json::Value terminologyRoot = this->GetTerminologyRootByName(contextName);
  Json::Value segmentationCodes;
  Json::Value categoryArray = Json::Value(Json::arrayValue);
  if (!terminologyRoot.isNull())
    {
    segmentationCodes = terminologyRoot["SegmentationCodes"];
    categoryArray = this->GetCategoryArrayInTerminology(contextName);
    }
  else
    {
    terminologyRoot["SegmentationCategoryTypeContextName"] = contextName;
    }

  // Parse segment attributes
  bool entryAdded = false;
  Json::ArrayIndex index = 0;
  while (segmentAttributesArray.isValidIndex(index))
    {
    Json::Value segmentAttributes = segmentAttributesArray[index];
    if (!segmentAttributes.isArray())
      {
      ++index;
      continue;
      }
    // Note: "The reason for the inner list is that we have one single schema both for input and output. When we provide input metafile,
    //       we can have multiple input files, and each file can have multiple labels, that is why we need to have list of lists"
    segmentAttributes = segmentAttributes[0]; // Enter "innerList"
    Json::Value segmentCategory = segmentAttributes["SegmentedPropertyCategoryCodeSequence"];
    Json::Value segmentType = segmentAttributes["SegmentedPropertyTypeCodeSequence"];
    Json::Value segmentTypeModifier = segmentAttributes["SegmentedPropertyTypeModifierCodeSequence"];
    Json::Value segmentRecommendedDisplayRGBValue = segmentAttributes["recommendedDisplayRGBValue"];
    if (!segmentCategory.isObject() || !segmentType.isObject())
      {
      vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Invalid segment terminology entry at index " << index);
      ++index;
      continue;
      }

    // Get type array if category already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier categoryId(
      segmentCategory["CodingSchemeDesignator"].asString(), segmentCategory["CodeValue"].asString(), segmentCategory["CodeMeaning"].asString() );
    int foundCategoryIndex = -1;
    Json::Value category = this->GetCodeInArray(categoryId, categoryArray, foundCategoryIndex);
    Json::Value typeArray = Json::Value(Json::arrayValue);
    if (!category.isNull())
      {
      typeArray = category["Type"];
      if (!typeArray.isArray())
        {
        vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Failed to find Type array in category '" << categoryId.CodeMeaning);
        ++index;
        continue;
        }
      }

    // Get type modifier array if type already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeId(
      segmentType["CodingSchemeDesignator"].asString(), segmentType["CodeValue"].asString(), segmentType["CodeMeaning"].asString() );
    int foundTypeIndex = -1;
    Json::Value type = this->GetCodeInArray(typeId, typeArray, foundTypeIndex);
    Json::Value typeModifierArray = Json::Value(Json::arrayValue);
    if (!type.isNull())
      {
      typeModifierArray = type["Modifier"];
      }

    // Add modifier if specified in descriptor and does not yet exist in terminology
    if (segmentTypeModifier.isObject())
      {
      vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeModifierId(
        segmentTypeModifier["CodingSchemeDesignator"].asString(), segmentTypeModifier["CodeValue"].asString(), segmentTypeModifier["CodeMeaning"].asString() );
      int foundTypeModifierIndex = -1;
      Json::Value typeModifier = this->GetCodeInArray(typeModifierId, typeModifierArray, foundTypeModifierIndex);
      // Modifier already exists, nothing to do
      if (typeModifier.isObject())
        {
        ++index;
        continue;
        }

      // Create and populate modifier
      typeModifier = this->GetJsonCodeFromIdentifier(typeModifier, typeModifierId);
      typeModifier["recommendedDisplayRGBValue"] = segmentRecommendedDisplayRGBValue; // Add color to type modifier

      // Set modifier to type
      typeModifierArray.append(typeModifier);
      type["Modifier"] = typeModifierArray;
      }
    else
      {
      // Add color to type if there is no modifier
      type["recommendedDisplayRGBValue"] = segmentRecommendedDisplayRGBValue;
      }

    // Add type if has not been added yet, overwrite otherwise
    type = this->GetJsonCodeFromIdentifier(type, typeId);
    if (foundTypeIndex == -1)
      {
      typeArray.append(type);
      }
    else
      {
      typeArray[foundTypeIndex] = type;
      }

    // Set type array back to category
    category["Type"] = typeArray;

    // Add category if has not been added yet, overwrite otherwise
    category = this->GetJsonCodeFromIdentifier(category, categoryId);
    if (foundCategoryIndex == -1)
      {
      categoryArray.append(category);
      }
    else
      {
      categoryArray[foundCategoryIndex] = category;
      }

    entryAdded = true;
    ++index;
    }

  // Set objects back to terminology Json object
  if (entryAdded)
    {
    segmentationCodes["Category"] = categoryArray;
    terminologyRoot["SegmentationCodes"] = segmentationCodes;

    return terminologyRoot;
    }

  return Json::Value(); // Return invalid if no entry was added from descriptor
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::ConvertSegmentationDescriptorToAnatomicContext(Json::Value descriptorRoot, std::string contextName)
{
  if (descriptorRoot.isNull() || contextName.empty())
    {
    return Json::Value();
    }

  // Get segment attributes
  Json::Value segmentAttributesArray = descriptorRoot["segmentAttributes"];
  if (!segmentAttributesArray.isArray())
    {
    vtkGenericWarningMacro("ConvertSegmentationDescriptorToAnatomicContext: Invalid segmentAttributes member");
    return Json::Value();
    }

  // Use terminology with context name if exists
  Json::Value anatomicContextRoot = this->GetAnatomicContextRootByName(contextName);
  Json::Value anatomicCodes;
  Json::Value regionArray = Json::Value(Json::arrayValue);
  if (!anatomicContextRoot.isNull())
    {
    anatomicCodes = anatomicContextRoot["AnatomicCodes"];
    regionArray = this->GetRegionArrayInAnatomicContext(contextName);
    }
  else
    {
    anatomicContextRoot["AnatomicContextName"] = contextName;
    }

  // Parse segment attributes
  bool entryAdded = false;
  Json::ArrayIndex index = 0;
  while (segmentAttributesArray.isValidIndex(index))
    {
    Json::Value segmentAttributes = segmentAttributesArray[index];
    if (!segmentAttributes.isArray())
      {
      ++index;
      continue;
      }
    // Note: "The reason for the inner list is that we have one single schema both for input and output. When we provide input metafile,
    //       we can have multiple input files, and each file can have multiple labels, that is why we need to have list of lists"
    segmentAttributes = segmentAttributes[0]; // Enter "innerList"
    Json::Value segmentRegion = segmentAttributes["AnatomicRegionCodeSequence"];
    Json::Value segmentRegionModifier = segmentAttributes["AnatomicRegionModifierCodeSequence"];
    if (!segmentRegion.isObject())
      {
      // Anatomic context is optional in the descriptor file
      ++index;
      continue;
      }

    // Get region modifier array if region already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionId(
      segmentRegion["CodingSchemeDesignator"].asString(), segmentRegion["CodeValue"].asString(), segmentRegion["CodeMeaning"].asString() );
    int foundRegionIndex = -1;
    Json::Value region = this->GetCodeInArray(regionId, regionArray, foundRegionIndex);
    Json::Value regionModifierArray = Json::Value(Json::arrayValue);
    if (!region.isNull())
      {
      regionModifierArray = region["Modifier"];
      }

    // Add modifier if specified in descriptor and does not yet exist in anatomic context
    if (segmentRegionModifier.isObject())
      {
      vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionModifierId(
        segmentRegionModifier["CodingSchemeDesignator"].asString(), segmentRegionModifier["CodeValue"].asString(), segmentRegionModifier["CodeMeaning"].asString() );
      int foundRegionModifierIndex = -1;
      Json::Value regionModifier = this->GetCodeInArray(regionModifierId, regionModifierArray, foundRegionModifierIndex);
      // Modifier already exists, nothing to do
      if (regionModifier.isObject())
        {
        ++index;
        continue;
        }

      // Create and populate modifier
      regionModifier = this->GetJsonCodeFromIdentifier(regionModifier, regionModifierId);

      // Set modifier to region
      regionModifierArray.append(regionModifier);
      region["Modifier"] = regionModifierArray;
      }

    // Add region if has not been added yet, overwrite otherwise
    region = this->GetJsonCodeFromIdentifier(region, regionId);
    if (foundRegionIndex == -1)
      {
      regionArray.append(region);
      }
    else
      {
      regionArray[foundRegionIndex] = region;
      }

    entryAdded = true;
    ++index;
    }

  // Set objects back to anatomic context Json object
  if (entryAdded)
    {
    anatomicCodes["AnatomicRegion"] = regionArray;
    anatomicContextRoot["AnatomicCodes"] = anatomicCodes;

    return anatomicContextRoot;
    }

  return Json::Value(); // Return invalid if no entry was added from descriptor
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetJsonCodeFromIdentifier(Json::Value code, CodeIdentifier idenfifier)
{
  code["CodingSchemeDesignator"] = idenfifier.CodingSchemeDesignator;
  code["CodeValue"] = idenfifier.CodeValue;
  code["CodeMeaning"] = idenfifier.CodeMeaning;
  return code;
}


//---------------------------------------------------------------------------
// vtkSlicerTerminologiesModuleLogic methods

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkSlicerTerminologiesModuleLogic()
{
  this->Internal = new vtkInternal(this);
}

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::~vtkSlicerTerminologiesModuleLogic()
{
  delete this->Internal;
  this->Internal = NULL;
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  // Load default terminologies and anatomical contexts
  // Note: Do it here not in the constructor so that the module shared directory is properly initialized
  bool wasModifying = this->GetDisableModifiedEvent();
  this->SetDisableModifiedEvent(true);
  this->LoadDefaultTerminologies();
  this->LoadDefaultAnatomicContexts();
  this->SetDisableModifiedEvent(wasModifying);
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromFile(std::string filePath)
{
  std::ifstream terminologyStream(filePath.c_str(), std::ios_base::binary);

  std::string contextName("");
  Json::Value terminologyRoot;

  // Load JSON file
  try
    {
    terminologyStream >> terminologyRoot;
    contextName = terminologyRoot["SegmentationCategoryTypeContextName"].asString();
    }
  catch (std::exception &e)
    {
    vtkErrorMacro("LoadTerminologyFromFile: Failed to load terminology from file '" << filePath << "' - exception: " << e.what());
    return "";
    }

  // Store terminology
  this->Internal->LoadedTerminologies[contextName] = terminologyRoot;

  vtkInfoMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  this->Modified();
  return contextName;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromSegmentDescriptorFile(std::string contextName, std::string filePath)
{
  std::ifstream descriptorStream(filePath.c_str(), std::ios_base::binary);
  Json::Value descriptorRoot;

  // Load JSON file
  try
    {
    descriptorStream >> descriptorRoot;
    }
  catch (std::exception &e)
    {
    vtkErrorMacro("LoadTerminologyFromSegmentDescriptorFile: Failed to load descriptor file '" << filePath << "' - exception: " << e.what());
    return false;
    }

  Json::Value terminologyRoot = this->Internal->ConvertSegmentationDescriptorToTerminologyContext(descriptorRoot, contextName);
  if (terminologyRoot.isNull())
    {
    vtkErrorMacro("LoadTerminologyFromSegmentDescriptorFile: Failed to parse descriptor file '" << filePath);
    return false;
    }

  // Store terminology
  this->Internal->LoadedTerminologies[contextName] = terminologyRoot;

  vtkInfoMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::LoadDefaultTerminologies()
{
  std::string success("");
  success = this->LoadTerminologyFromFile(this->GetModuleShareDirectory() + "/SegmentationCategoryTypeModifier-SlicerGeneralAnatomy.json");
  if (success.empty())
    {
    vtkErrorMacro("LoadDefaultTerminologies: Failed to load terminology 'SegmentationCategoryTypeModifier-SlicerGeneralAnatomy'");
    }
  success = this->LoadTerminologyFromFile(this->GetModuleShareDirectory() + "/SegmentationCategoryTypeModifier-DICOM-Master.json");
  if (success.empty())
    {
    vtkErrorMacro("LoadDefaultTerminologies: Failed to load terminology 'SegmentationCategoryTypeModifier-DICOM-Master'");
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::LoadAnatomicContextFromFile(std::string filePath)
{
  std::ifstream anatomicContextStream(filePath.c_str(), std::ios_base::binary);

  std::string contextName("");
  Json::Value anatomicContextRoot;

  try
    {
    anatomicContextStream >> anatomicContextRoot;
    contextName = anatomicContextRoot["AnatomicContextName"].asString();
    }
  catch (std::exception &e)
    {
    vtkErrorMacro("LoadAnatomicContextFromFile: Failed to load default terminology - exception: " << e.what());
    return "";
    }

  // Store anatomic context
  this->Internal->LoadedAnatomicContexts[contextName] = anatomicContextRoot;

  vtkInfoMacro("Anatomic context named '" << contextName << "' successfully loaded from file " << filePath);
  return contextName;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadAnatomicContextFromSegmentDescriptorFile(std::string contextName, std::string filePath)
{
  std::ifstream descriptorStream(filePath.c_str(), std::ios_base::binary);
  Json::Value descriptorRoot;

  // Load JSON file
  try
    {
    descriptorStream >> descriptorRoot;
    }
  catch (std::exception &e)
    {
    vtkErrorMacro("LoadAnatomicContextFromSegmentDescriptorFile: Failed to load descriptor file '" << filePath << "' - exception: " << e.what());
    return false;
    }

  Json::Value anatomicContextRoot = this->Internal->ConvertSegmentationDescriptorToAnatomicContext(descriptorRoot, contextName);
  if (anatomicContextRoot.isNull())
    {
    // Anatomic context is optional in descriptor file
    return false;
    }

  // Store anatomic context
  this->Internal->LoadedAnatomicContexts[contextName] = anatomicContextRoot;

  vtkInfoMacro("Anatomic context named '" << contextName << "' successfully loaded from file " << filePath);
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::LoadDefaultAnatomicContexts()
{
  std::string success("");
  success = this->LoadAnatomicContextFromFile(this->GetModuleShareDirectory() + "/AnatomicRegionModifier-Master.json");
  if (success.empty())
    {
    vtkErrorMacro("LoadDefaultAnatomicContexts: Failed to load anatomical region context 'AnatomicRegionModifier-Master'");
    }
}


//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedTerminologyNames(std::vector<std::string> &terminologyNames)
{
  terminologyNames.clear();

  vtkSlicerTerminologiesModuleLogic::vtkInternal::TerminologyMap::iterator termIt;
  for (termIt=this->Internal->LoadedTerminologies.begin(); termIt!=this->Internal->LoadedTerminologies.end(); ++termIt)
    {
    terminologyNames.push_back(termIt->first);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedTerminologyNames(vtkStringArray* terminologyNames)
{
  if (!terminologyNames)
    {
    return;
    }
  terminologyNames->Initialize();

  std::vector<std::string> terminologyNamesVector;
  this->GetLoadedTerminologyNames(terminologyNamesVector);
  for (std::vector<std::string>::iterator termIt = terminologyNamesVector.begin(); termIt != terminologyNamesVector.end(); ++termIt)
    {
    terminologyNames->InsertNextValue(termIt->c_str());
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedAnatomicContextNames(std::vector<std::string> &anatomicContextNames)
{
  anatomicContextNames.clear();

  vtkSlicerTerminologiesModuleLogic::vtkInternal::TerminologyMap::iterator anIt;
  for (anIt=this->Internal->LoadedAnatomicContexts.begin(); anIt!=this->Internal->LoadedAnatomicContexts.end(); ++anIt)
    {
    anatomicContextNames.push_back(anIt->first);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedAnatomicContextNames(vtkStringArray* anatomicContextNames)
{
  if (!anatomicContextNames)
    {
    return;
    }
  anatomicContextNames->Initialize();

  std::vector<std::string> anatomicContextNamesVector;
  this->GetLoadedAnatomicContextNames(anatomicContextNamesVector);
  for (std::vector<std::string>::iterator anIt = anatomicContextNamesVector.begin(); anIt != anatomicContextNamesVector.end(); ++anIt)
    {
    anatomicContextNames->InsertNextValue(anIt->c_str());
    }
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetCategoriesInTerminology(std::string terminologyName, std::vector<CodeIdentifier>& categories)
{
  return this->FindCategoriesInTerminology(terminologyName, categories, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindCategoriesInTerminology(std::string terminologyName, std::vector<CodeIdentifier>& categories, std::string search/*=""*/)
{
  categories.clear();

  Json::Value categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.isNull())
    {
    vtkErrorMacro("FindCategoriesInTerminology: Failed to find category array in terminology '" << terminologyName << "'");
    return false;
    }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse categories
  Json::ArrayIndex index = 0;
  while (categoryArray.isValidIndex(index))
    {
    Json::Value category = categoryArray[index];
    if (category.isObject())
      {
      Json::Value categoryName = category["CodeMeaning"];
      Json::Value categoryCodingSchemeDesignator = category["CodingSchemeDesignator"];
      Json::Value categoryCodeValue = category["CodeValue"];
      if (categoryName.isString() && categoryCodingSchemeDesignator.isString() && categoryCodeValue.isString())
        {
        // Add category to list if search string is empty or is contained by the current category name
        std::string categoryNameStr = categoryName.asString();
        std::string categoryNameLowerCase(categoryNameStr);
        std::transform(categoryNameLowerCase.begin(), categoryNameLowerCase.end(), categoryNameLowerCase.begin(), ::tolower);
        if (search.empty() || categoryNameLowerCase.find(search) != std::string::npos)
          {
          CodeIdentifier categoryId(categoryCodingSchemeDesignator.asString(), categoryCodeValue.asString(), categoryNameStr);
          categories.push_back(categoryId);
          }
        }
      else
        {
        vtkErrorMacro("FindCategoriesInTerminology: Invalid category '" << categoryName.asString() << "' in terminology '" << terminologyName << "'");
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetCategoryInTerminology(std::string terminologyName, CodeIdentifier categoryId, vtkSlicerTerminologyCategory* category)
{
  if (!category || categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
    {
    return false;
    }

  Json::Value categoryObject = this->Internal->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.isNull())
    {
    vtkErrorMacro("GetCategoryInTerminology: Failed to find category '" << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Category found
  return this->Internal->PopulateTerminologyCategoryFromJson(categoryObject, category);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypesInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId, std::vector<CodeIdentifier>& types)
{
  return this->FindTypesInTerminologyCategory(terminologyName, categoryId, types, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindTypesInTerminologyCategory(
  std::string terminologyName, CodeIdentifier categoryId, std::vector<CodeIdentifier>& types, std::string search)
{
  types.clear();

  Json::Value typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.isNull())
    {
    vtkErrorMacro("FindTypesInTerminologyCategory: Failed to find Type array member in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse types
  Json::ArrayIndex index = 0;
  while (typeArray.isValidIndex(index))
    {
    Json::Value type = typeArray[index];
    if (type.isObject())
      {
      Json::Value typeName = type["CodeMeaning"];
      Json::Value typeCodingSchemeDesignator = type["CodingSchemeDesignator"];
      Json::Value typeCodeValue = type["CodeValue"];
      if (typeName.isString() && typeCodingSchemeDesignator.isString() && typeCodeValue.isString())
        {
        // Add type to list if search string is empty or is contained by the current type name
        std::string typeNameStr = typeName.asString();
        std::string typeNameLowerCase(typeNameStr);
        std::transform(typeNameLowerCase.begin(), typeNameLowerCase.end(), typeNameLowerCase.begin(), ::tolower);
        if (search.empty() || typeNameLowerCase.find(search) != std::string::npos)
          {
          CodeIdentifier typeId(typeCodingSchemeDesignator.asString(), typeCodeValue.asString(), typeNameStr);
          types.push_back(typeId);
          }
        }
      else
        {
        vtkErrorMacro("FindTypesInTerminologyCategory: Invalid type '" << typeName.asString() << "in category '"
          << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeInTerminologyCategory(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, vtkSlicerTerminologyType* type)
{
  if (!type || typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
    {
    return false;
    }

  Json::Value typeObject = this->Internal->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId);
  if (typeObject.isNull())
    {
    vtkErrorMacro("GetTypeInTerminologyCategory: Failed to find type '" << typeId.CodeMeaning << "' in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Type found
  return this->Internal->PopulateTerminologyTypeFromJson(typeObject, type);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifiersInTerminologyType(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, std::vector<CodeIdentifier>& typeModifiers)
{
  typeModifiers.clear();

  Json::Value typeModifierArray = this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.isNull())
    {
    vtkErrorMacro("GetTypeModifiersInTerminologyType: Failed to find Type Modifier array member in type '" << typeId.CodeMeaning << "' in category "
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Collect type modifiers
  Json::ArrayIndex index = 0;
  while (typeModifierArray.isValidIndex(index))
    {
    Json::Value typeModifier = typeModifierArray[index];
    if (typeModifier.isObject())
      {
      Json::Value typeModifierName = typeModifier["CodeMeaning"];
      Json::Value typeModifierCodingSchemeDesignator = typeModifier["CodingSchemeDesignator"];
      Json::Value typeModifierCodeValue = typeModifier["CodeValue"];
      if (typeModifierName.isString() && typeModifierCodingSchemeDesignator.isString() && typeModifierCodeValue.isString())
        {
          CodeIdentifier typeModifierId(typeModifierCodingSchemeDesignator.asString(), typeModifierCodeValue.asString(), typeModifierName.asString());
          typeModifiers.push_back(typeModifierId);
        }
      }
    ++index;
    } // For each type index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifierInTerminologyType(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, CodeIdentifier modifierId, vtkSlicerTerminologyType* typeModifier)
{
  if (!typeModifier || modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
    {
    return false;
    }

  Json::Value typeModifierObject = this->Internal->GetTypeModifierInTerminologyType(terminologyName, categoryId, typeId, modifierId);
  if (typeModifierObject.isNull())
    {
    vtkErrorMacro("GetTypeModifierInTerminologyType: Failed to find type modifier '" << modifierId.CodeMeaning << "' in type '"
      << typeId.CodeMeaning << "' in category '" << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Type modifier with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(typeModifierObject, typeModifier);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionsInAnatomicContext(std::string anatomicContextName, std::vector<CodeIdentifier>& regions)
{
  return this->FindRegionsInAnatomicContext(anatomicContextName, regions, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindRegionsInAnatomicContext(std::string anatomicContextName, std::vector<CodeIdentifier>& regions, std::string search)
{
  regions.clear();

  Json::Value regionArray = this->Internal->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.isNull())
    {
    vtkErrorMacro("FindRegionsInAnatomicContext: Failed to find region array member in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse regions
  Json::ArrayIndex index = 0;
  while (regionArray.isValidIndex(index))
    {
    Json::Value region = regionArray[index];
    if (region.isObject())
      {
      Json::Value regionName = region["CodeMeaning"];
      Json::Value regionCodingSchemeDesignator = region["CodingSchemeDesignator"];
      Json::Value regionCodeValue = region["CodeValue"];
      if (regionName.isString() && regionCodingSchemeDesignator.isString() && regionCodeValue.isString())
        {
        // Add region name to list if search string is empty or is contained by the current region name
        std::string regionNameStr = regionName.asString();
        std::string regionNameLowerCase(regionNameStr);
        std::transform(regionNameLowerCase.begin(), regionNameLowerCase.end(), regionNameLowerCase.begin(), ::tolower);
        if (search.empty() || regionNameLowerCase.find(search) != std::string::npos)
          {
          CodeIdentifier regionId(regionCodingSchemeDesignator.asString(), regionCodeValue.asString(), regionNameStr);
          regions.push_back(regionId);
          }
        }
      else
        {
        vtkErrorMacro("FindRegionsInAnatomicContext: Invalid region '" << regionName.asString() << "' in anatomic context '" << anatomicContextName << "'");
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionInAnatomicContext(std::string anatomicContextName, CodeIdentifier regionId, vtkSlicerTerminologyType* region)
{
  if (!region || regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
    {
    return false;
    }

  Json::Value regionObject = this->Internal->GetRegionInAnatomicContext(anatomicContextName, regionId);
  if (regionObject.isNull())
    {
    vtkErrorMacro("GetRegionInAnatomicContext: Failed to find region '" << regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Region with specified name found
  return this->Internal->PopulateRegionFromJson(regionObject, region);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifiersInAnatomicRegion(
  std::string anatomicContextName, CodeIdentifier regionId, std::vector<CodeIdentifier>& regionModifiers )
{
  regionModifiers.clear();

  Json::Value regionModifierArray = this->Internal->GetRegionModifierArrayInRegion(anatomicContextName, regionId);
  if (regionModifierArray.isNull())
    {
    vtkErrorMacro("GetRegionModifiersInRegion: Failed to find Region Modifier array member in region '"
      << regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Collect region modifiers
  Json::ArrayIndex index = 0;
  while (regionModifierArray.isValidIndex(index))
    {
    Json::Value regionModifier = regionModifierArray[index];
    if (regionModifier.isObject())
      {
      Json::Value regionModifierName = regionModifier["CodeMeaning"];
      Json::Value regionModifierCodingSchemeDesignator = regionModifier["CodingSchemeDesignator"];
      Json::Value regionModifierCodeValue = regionModifier["CodeValue"];
      if (regionModifierName.isString() && regionModifierCodingSchemeDesignator.isString() && regionModifierCodeValue.isString())
        {
          CodeIdentifier regionModifierId(regionModifierCodingSchemeDesignator.asString(), regionModifierCodeValue.asString(), regionModifierName.asString());
          regionModifiers.push_back(regionModifierId);
        }
      }
    ++index;
    } // For each region index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifierInAnatomicRegion(std::string anatomicContextName,
    CodeIdentifier regionId, CodeIdentifier modifierId, vtkSlicerTerminologyType* regionModifier)
{
  if (!regionModifier || modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
    {
    return false;
    }

  Json::Value regionModifierObject = this->Internal->GetRegionModifierInRegion(anatomicContextName, regionId, modifierId);
  if (regionModifierObject.isNull())
    {
    vtkErrorMacro("GetRegionModifierInAnatomicRegion: Failed to find region modifier '" << modifierId.CodeMeaning
      << "' in region '" << regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Region modifier with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(regionModifierObject, regionModifier);
}

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::CodeIdentifier vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory(vtkSlicerTerminologyCategory* category)
{
  if (!category)
    {
    return CodeIdentifier("","","");
    }
  CodeIdentifier id(
    (category->GetCodingScheme()?category->GetCodingScheme():""),
    (category->GetCodeValue()?category->GetCodeValue():""),
    (category->GetCodeMeaning()?category->GetCodeMeaning():"") );
  return id;
}

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::CodeIdentifier vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType(vtkSlicerTerminologyType* type)
{
  if (!type)
    {
    return CodeIdentifier("","","");
    }
  CodeIdentifier id(
    (type->GetCodingScheme()?type->GetCodingScheme():""),
    (type->GetCodeValue()?type->GetCodeValue():""),
    (type->GetCodeMeaning()?type->GetCodeMeaning():"") );
  return id;
}
