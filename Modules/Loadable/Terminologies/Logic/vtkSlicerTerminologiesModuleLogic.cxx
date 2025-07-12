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

#include "vtkSlicerTerminologyEntry.h"
#include "vtkSlicerTerminologyCategory.h"
#include "vtkSlicerTerminologyType.h"

// MRML includes
#include <vtkMRMLColorNode.h>
#include <vtkMRMLScene.h>

// Slicer includes
#include "vtkLoggingMacros.h"

// vtkSegmentationCore includes
#include <vtkSegment.h>

// VTK includes
#include <vtkCollection.h>
#include <vtkDirectory.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkVariant.h>
#include <vtksys/SystemTools.hxx>

// STD includes
#include <algorithm>

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filereadstream.h"

static rapidjson::Value JSON_EMPTY_VALUE;
static std::string REGION_CONTEXT_SCHEMA =
  "https://raw.githubusercontent.com/qiicr/dcmqi/master/doc/anatomic-context-schema.json#";
static std::string REGION_CONTEXT_SCHEMA_1 =
  "https://raw.githubusercontent.com/qiicr/dcmqi/master/doc/schemas/anatomic-context-schema.json#";
static std::string TERMINOLOGY_CONTEXT_SCHEMA =
  "https://raw.githubusercontent.com/qiicr/dcmqi/master/doc/segment-context-schema.json#";
static std::string TERMINOLOGY_CONTEXT_SCHEMA_1 =
  "https://raw.githubusercontent.com/qiicr/dcmqi/master/doc/schemas/segment-context-schema.json#";

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologiesModuleLogic);

//---------------------------------------------------------------------------
class vtkSlicerTerminologiesModuleLogic::vtkInternal
{
public:
  // rapidjson document object cannot be stored in an STL map (there are build errors
  // on Linux and Mac), therefore we store a simple pointer and create/delete
  // the document object manually
  typedef std::map<std::string, rapidjson::Document*> TerminologyMap;
  vtkInternal();
  ~vtkInternal();

  /// Utility function to get code in Json array
  /// \param foundIndex Output parameter for index of found object in input array. -1 if not found
  /// \return Json object if found, otherwise null Json object
  rapidjson::Value& GetCodeInArray(CodeIdentifier codeId, rapidjson::Value& jsonArray, int& foundIndex);

  /// Get root Json value for the terminology with given name
  rapidjson::Value& GetTerminologyRootByName(std::string terminologyName);

  /// Get category array Json value for a given terminology
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetCategoryArrayInTerminology(std::string terminologyName);
  /// Get category Json object from terminology with given category name
  /// \return Null Json value on failure, the category Json object otherwise
  rapidjson::Value& GetCategoryInTerminology(std::string terminologyName, CodeIdentifier categoryId);
  /// Populate \sa vtkSlicerTerminologyCategory from Json terminology
  bool PopulateTerminologyCategoryFromJson(rapidjson::Value& categoryObject, vtkSlicerTerminologyCategory* category);

  /// Get type array Json value for a given terminology and category
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetTypeArrayInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId);
  /// Get type Json object from a terminology category with given type name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetTypeInTerminologyCategory(std::string terminologyName,
                                                 CodeIdentifier categoryId,
                                                 CodeIdentifier typeId);
  /// Populate \sa vtkSlicerTerminologyType from Json terminology
  bool PopulateTerminologyTypeFromJson(rapidjson::Value& typeObject, vtkSlicerTerminologyType* type);

  /// Get type modifier array Json value for a given terminology, category, and type
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetTypeModifierArrayInTerminologyType(std::string terminologyName,
                                                          CodeIdentifier categoryId,
                                                          CodeIdentifier typeId);
  /// Get type modifier Json object from a terminology, category, and type with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetTypeModifierInTerminologyType(std::string terminologyName,
                                                     CodeIdentifier categoryId,
                                                     CodeIdentifier typeId,
                                                     CodeIdentifier modifierId);

  /// Get root Json value for the region context with given name
  rapidjson::Value& GetRegionContextRootByName(std::string regionContextName);

  /// Get region array Json value for a given region context
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetRegionArrayInRegionContext(std::string regionContextName);
  /// Get type Json object from an region context with given region name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetRegionInRegionContext(std::string regionContextName, CodeIdentifier regionId);
  /// Populate \sa vtkSlicerTerminologyType from Json region region
  bool PopulateRegionFromJson(rapidjson::Value& regionObject, vtkSlicerTerminologyType* region);

  /// Get region modifier array Json value for a given region context and region
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetRegionModifierArrayInRegion(std::string regionContextName, CodeIdentifier regionId);
  /// Get type modifier Json object from an region context and region with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetRegionModifierInRegion(std::string regionContextName,
                                              CodeIdentifier regionId,
                                              CodeIdentifier modifierId);

  /// Convert a segmentation descriptor Json structure to a terminology context one
  /// \param descriptorDoc Input segmentation descriptor json document
  /// \param convertedDoc Output terminology context json document
  /// \return Success flag
  bool ConvertSegmentationDescriptorToTerminologyContext(rapidjson::Document& descriptorDoc,
                                                         rapidjson::Document& convertedDoc,
                                                         std::string contextName);
  /// Convert a segmentation descriptor Json structure to an region context one
  /// \param descriptorDoc Input segmentation descriptor json document
  /// \param convertedDoc Output region context json document
  /// \return Success flag
  bool ConvertSegmentationDescriptorToRegionContext(rapidjson::Document& descriptorDoc,
                                                    rapidjson::Document& convertedDoc,
                                                    std::string contextName);
  /// Copy basic identifier members from an identifier object into a Json object
  /// Note: Allocator specifies the owner of the created object, so it is important to set the allocator
  ///       of the document where the object will be added
  /// \param code Json object into which the code information is added a members
  void GetJsonCodeFromIdentifier(rapidjson::Value& code,
                                 CodeIdentifier identifier,
                                 rapidjson::Document::AllocatorType& allocator);

  /// Utility function for safe (memory-leak-free) setting of a document pointer in map
  static void SetDocumentInTerminologyMap(TerminologyMap& terminologyMap,
                                          const std::string& name,
                                          rapidjson::Document* doc)
  {
    if (terminologyMap.find(name) != terminologyMap.end())
    {
      if (doc == terminologyMap[name])
      {
        // The two objects are the same, there is nothing to do
        return;
      }
      // Make sure the previous document object is deleted
      delete terminologyMap[name];
    }
    // Set new document object
    terminologyMap[name] = doc;
  }

public:
  /// Loaded terminologies. Key is the context name, value is the root item.
  TerminologyMap LoadedTerminologies;

  /// Loaded region contexts. Key is the context name, value is the root item.
  TerminologyMap LoadedRegionContexts;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkInternal::vtkInternal() = default;

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkInternal::~vtkInternal()
{
  for (TerminologyMap::iterator termIt = this->LoadedTerminologies.begin(); termIt != this->LoadedTerminologies.end();
       ++termIt)
  {
    delete termIt->second;
  }
  for (TerminologyMap::iterator anIt = this->LoadedRegionContexts.begin(); anIt != this->LoadedRegionContexts.end();
       ++anIt)
  {
    delete anIt->second;
  }
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCodeInArray(CodeIdentifier codeId,
                                                                                 rapidjson::Value& jsonArray,
                                                                                 int& foundIndex)
{
  if (!jsonArray.IsArray())
  {
    return JSON_EMPTY_VALUE;
  }

  // Traverse array and try to find the object with given identifier
  rapidjson::SizeType index = 0;
  while (index < jsonArray.Size())
  {
    rapidjson::Value& currentObject = jsonArray[index];
    if (currentObject.IsObject())
    {
      rapidjson::Value& codingSchemeDesignator = currentObject["CodingSchemeDesignator"];
      rapidjson::Value& codeValue = currentObject["CodeValue"];
      if (codingSchemeDesignator.IsString()
          && !codeId.CodingSchemeDesignator.compare(codingSchemeDesignator.GetString()) && codeValue.IsString()
          && !codeId.CodeValue.compare(codeValue.GetString()))
      {
        foundIndex = index;
        return currentObject;
      }
    }
    ++index;
  }

  // Not found
  foundIndex = -1;
  return JSON_EMPTY_VALUE;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTerminologyRootByName(std::string terminologyName)
{
  TerminologyMap::iterator termIt = this->LoadedTerminologies.find(terminologyName);
  if (termIt != this->LoadedTerminologies.end() && termIt->second != nullptr)
  {
    return *(termIt->second);
  }

  return JSON_EMPTY_VALUE;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryArrayInTerminology(
  std::string terminologyName)
{
  if (terminologyName.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& root = this->GetTerminologyRootByName(terminologyName);
  if (root.IsNull())
  {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find terminology root for context name '"
                           << terminologyName << "'");
    return JSON_EMPTY_VALUE;
  }

  rapidjson::Value& segmentationCodes = root["SegmentationCodes"];
  if (segmentationCodes.IsNull())
  {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find SegmentationCodes member in terminology '"
                           << terminologyName << "'");
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& categoryArray = segmentationCodes["Category"];
  if (!categoryArray.IsArray())
  {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find Category array member in terminology '"
                           << terminologyName << "'");
    return JSON_EMPTY_VALUE;
  }

  return categoryArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryInTerminology(std::string terminologyName,
                                                                                           CodeIdentifier categoryId)
{
  if (categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& categoryArray = this->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
  {
    vtkGenericWarningMacro("GetCategoryInTerminology: Failed to find category array in terminology '" << terminologyName
                                                                                                      << "'");
    return JSON_EMPTY_VALUE;
  }

  int index = -1;
  return this->GetCodeInArray(categoryId, categoryArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyCategoryFromJson(
  rapidjson::Value& categoryObject,
  vtkSlicerTerminologyCategory* category)
{
  if (!categoryObject.IsObject() || !category)
  {
    return false;
  }

  rapidjson::Value::MemberIterator codeMeaning = categoryObject.FindMember("CodeMeaning"); // e.g. "Tissue" (mandatory)
  rapidjson::Value::MemberIterator codingScheme =
    categoryObject.FindMember("CodingSchemeDesignator"); // e.g. "SCT" (mandatory)
  rapidjson::Value::MemberIterator SNOMEDCTConceptID =
    categoryObject.FindMember("SNOMEDCTConceptID");                                              // e.g. "85756007"
  rapidjson::Value::MemberIterator UMLSConceptUID = categoryObject.FindMember("UMLSConceptUID"); // e.g. "C0040300"
  rapidjson::Value::MemberIterator cid = categoryObject.FindMember("cid");                       // e.g. "7051"
  rapidjson::Value::MemberIterator codeValue = categoryObject.FindMember("CodeValue"); // e.g. "85756007" (mandatory)
  rapidjson::Value::MemberIterator contextGroupName =
    categoryObject.FindMember("contextGroupName"); // e.g. "Segmentation Property Categories"
  rapidjson::Value::MemberIterator showAnatomy = categoryObject.FindMember("showAnatomy");
  if (codingScheme == categoryObject.MemberEnd() || codeValue == categoryObject.MemberEnd()
      || codeMeaning == categoryObject.MemberEnd())
  {
    vtkGenericWarningMacro("PopulateTerminologyCategoryFromJson: Unable to access mandatory category member");
    return false;
  }

  category->SetCodeMeaning(codeMeaning->value.GetString());
  category->SetCodingSchemeDesignator(codingScheme->value.GetString());
  category->SetSNOMEDCTConceptID(SNOMEDCTConceptID != categoryObject.MemberEnd() ? SNOMEDCTConceptID->value.GetString()
                                                                                 : nullptr);
  category->SetUMLSConceptUID(UMLSConceptUID != categoryObject.MemberEnd() ? UMLSConceptUID->value.GetString()
                                                                           : nullptr);
  category->SetCid(cid != categoryObject.MemberEnd() ? cid->value.GetString() : nullptr);
  category->SetCodeValue(codeValue->value.GetString());
  category->SetContextGroupName(contextGroupName != categoryObject.MemberEnd() ? contextGroupName->value.GetString()
                                                                               : nullptr);
  if (showAnatomy == categoryObject.MemberEnd())
  {
    category->SetShowAnatomy(true); // Default
  }
  else
  {
    if (showAnatomy->value.IsString())
    {
      std::string showAnatomyStr = showAnatomy->value.GetString();
      std::transform(showAnatomyStr.begin(),
                     showAnatomyStr.end(),
                     showAnatomyStr.begin(),
                     ::tolower); // Make it lowercase for case-insensitive comparison
      category->SetShowAnatomy(showAnatomyStr.compare("true") ? false : true);
    }
    else if (showAnatomy->value.IsBool())
    {
      category->SetShowAnatomy(showAnatomy->value.GetBool());
    }
    else
    {
      category->SetShowAnatomy(true); // Default
    }
  }
  return true;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeArrayInTerminologyCategory(
  std::string terminologyName,
  CodeIdentifier categoryId)
{
  if (categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& categoryObject = this->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.IsNull())
  {
    // Category not found
    return JSON_EMPTY_VALUE;
  }

  rapidjson::Value& typeArray = categoryObject["Type"];
  if (!typeArray.IsArray())
  {
    vtkGenericWarningMacro("GetTypeArrayInTerminologyCategory: Failed to find Type array member in category '"
                           << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
  }

  return typeArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeInTerminologyCategory(
  std::string terminologyName,
  CodeIdentifier categoryId,
  CodeIdentifier typeId)
{
  if (typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& typeArray = this->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.IsNull())
  {
    // Type array not found for category
    return JSON_EMPTY_VALUE;
  }

  int index = -1;
  return this->GetCodeInArray(typeId, typeArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyTypeFromJson(rapidjson::Value& typeObject,
                                                                                     vtkSlicerTerminologyType* type)
{
  if (!typeObject.IsObject() || !type)
  {
    return false;
  }
  rapidjson::Value::MemberIterator recommendedDisplayRGBValue = typeObject.FindMember("recommendedDisplayRGBValue");
  // e.g. "Artery" (mandatory)
  rapidjson::Value::MemberIterator codeMeaning = typeObject.FindMember("CodeMeaning");
  // e.g. "SCT" (mandatory)
  rapidjson::Value::MemberIterator codingScheme = typeObject.FindMember("CodingSchemeDesignator");
  // e.g. "artery"
  rapidjson::Value::MemberIterator slicerLabel = typeObject.FindMember("3dSlicerLabel");
  // e.g. "85756007"
  rapidjson::Value::MemberIterator SNOMEDCTConceptID = typeObject.FindMember("SNOMEDCTConceptID");
  // e.g. "C0040300"
  rapidjson::Value::MemberIterator UMLSConceptUID = typeObject.FindMember("UMLSConceptUID");
  // e.g. "7051"
  rapidjson::Value::MemberIterator cid = typeObject.FindMember("cid");
  // e.g. "85756007" (mandatory)
  rapidjson::Value::MemberIterator codeValue = typeObject.FindMember("CodeValue");
  // e.g. "Segmentation Property Categories"
  rapidjson::Value::MemberIterator contextGroupName = typeObject.FindMember("contextGroupName");
  // Modifier array, containing modifiers of this type, e.g. "Left"
  rapidjson::Value::MemberIterator modifier = typeObject.FindMember("Modifier");
  if (codingScheme == typeObject.MemberEnd() || codeValue == typeObject.MemberEnd()
      || codeMeaning == typeObject.MemberEnd())
  {
    vtkGenericWarningMacro("PopulateTerminologyTypeFromJson: Unable to access mandatory type member");
    return false;
  }

  type->SetCodeMeaning(codeMeaning->value.GetString());
  type->SetCodingSchemeDesignator(codingScheme->value.GetString());
  type->SetSlicerLabel(slicerLabel != typeObject.MemberEnd() ? slicerLabel->value.GetString() : nullptr);
  type->SetSNOMEDCTConceptID(SNOMEDCTConceptID != typeObject.MemberEnd() ? SNOMEDCTConceptID->value.GetString()
                                                                         : nullptr);
  type->SetUMLSConceptUID(UMLSConceptUID != typeObject.MemberEnd() ? UMLSConceptUID->value.GetString() : nullptr);
  type->SetCid(cid != typeObject.MemberEnd() ? cid->value.GetString() : nullptr);
  type->SetCodeValue(codeValue->value.GetString());
  type->SetContextGroupName(contextGroupName != typeObject.MemberEnd() ? contextGroupName->value.GetString() : nullptr);

  if (recommendedDisplayRGBValue != typeObject.MemberEnd() && (recommendedDisplayRGBValue->value).IsArray()
      && (recommendedDisplayRGBValue->value).Size() == 3)
  {
    if (recommendedDisplayRGBValue->value[0].IsString())
    {
      type->SetRecommendedDisplayRGBValue( // Note: Casting directly to unsigned char fails
        (unsigned char)vtkVariant(recommendedDisplayRGBValue->value[0].GetString()).ToInt(),
        (unsigned char)vtkVariant(recommendedDisplayRGBValue->value[1].GetString()).ToInt(),
        (unsigned char)vtkVariant(recommendedDisplayRGBValue->value[2].GetString()).ToInt());
    }
    else if (recommendedDisplayRGBValue->value[0].IsInt())
    {
      type->SetRecommendedDisplayRGBValue((unsigned char)recommendedDisplayRGBValue->value[0].GetInt(),
                                          (unsigned char)recommendedDisplayRGBValue->value[1].GetInt(),
                                          (unsigned char)recommendedDisplayRGBValue->value[2].GetInt());
    }
    else
    {
      vtkGenericWarningMacro("PopulateTerminologyTypeFromJson: Unsupported data type for recommendedDisplayRGBValue");
    }
  }
  else
  {
    type->SetRecommendedDisplayRGBValue(vtkSlicerTerminologyType::INVALID_COLOR[0],
                                        vtkSlicerTerminologyType::INVALID_COLOR[1],
                                        vtkSlicerTerminologyType::INVALID_COLOR[2]); // 'Invalid' gray
  }

  type->SetHasModifiers((modifier->value).IsArray());

  return true;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeModifierArrayInTerminologyType(
  std::string terminologyName,
  CodeIdentifier categoryId,
  CodeIdentifier typeId)
{
  if (typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& categoryObject = this->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.IsNull())
  {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find category '"
                           << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
  }

  rapidjson::Value& typeObject = this->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId);
  if (typeObject.IsNull())
  {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find type '"
                           << typeId.CodeMeaning << "' in category '" << categoryId.CodeMeaning << "' in terminology '"
                           << terminologyName << "'");
    return JSON_EMPTY_VALUE;
  }

  rapidjson::Value::MemberIterator typeModifierIt = typeObject.FindMember("Modifier");
  if (typeModifierIt == typeObject.MemberEnd() || !typeModifierIt->value.IsArray())
  {
    return JSON_EMPTY_VALUE;
  }

  return typeModifierIt->value;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeModifierInTerminologyType(
  std::string terminologyName,
  CodeIdentifier categoryId,
  CodeIdentifier typeId,
  CodeIdentifier modifierId)
{
  if (modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& typeModifierArray =
    this->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.IsNull())
  {
    return JSON_EMPTY_VALUE;
  }

  int index = -1;
  return this->GetCodeInArray(modifierId, typeModifierArray, index);
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionContextRootByName(
  std::string regionContextName)
{
  TerminologyMap::iterator anIt = this->LoadedRegionContexts.find(regionContextName);
  if (anIt != this->LoadedRegionContexts.end() && anIt->second != nullptr)
  {
    return *(anIt->second);
  }

  return JSON_EMPTY_VALUE;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionArrayInRegionContext(
  std::string regionContextName)
{
  if (regionContextName.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& root = this->GetRegionContextRootByName(regionContextName);
  if (root.IsNull())
  {
    vtkGenericWarningMacro("GetRegionArrayInRegionContext: Failed to find region context root for context name '"
                           << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }

  rapidjson::Value& regionCodes = root["AnatomicCodes"];
  if (regionCodes.IsNull())
  {
    vtkGenericWarningMacro("GetRegionArrayInRegionContext: Failed to find AnatomicCodes member in region context '"
                           << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& regionArray = regionCodes["AnatomicRegion"];
  if (!regionArray.IsArray())
  {
    vtkGenericWarningMacro(
      "GetRegionArrayInRegionContext: Failed to find AnatomicRegion array member in region context '"
      << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }

  return regionArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionInRegionContext(
  std::string regionContextName,
  CodeIdentifier regionId)
{
  if (regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& regionArray = this->GetRegionArrayInRegionContext(regionContextName);
  if (regionArray.IsNull())
  {
    vtkGenericWarningMacro("GetRegionInRegionContext: Failed to find region array for region context '"
                           << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }

  int index = -1;
  return this->GetCodeInArray(regionId, regionArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateRegionFromJson(rapidjson::Value& regionObject,
                                                                            vtkSlicerTerminologyType* region)
{
  return this->PopulateTerminologyTypeFromJson(regionObject, region);
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierArrayInRegion(
  std::string regionContextName,
  CodeIdentifier regionId)
{
  if (regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& regionObject = this->GetRegionInRegionContext(regionContextName, regionId);
  if (regionObject.IsNull())
  {
    vtkGenericWarningMacro("GetRegionModifierArrayInRegion: Failed to find region '"
                           << regionId.CodeMeaning << "' in region context '" << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }

  rapidjson::Value& regionModifierArray = regionObject["Modifier"];
  if (!regionModifierArray.IsArray())
  {
    vtkGenericWarningMacro("GetRegionModifierArrayInRegion: Failed to find Modifier array member in region '"
                           << regionId.CodeMeaning << "' in region context '" << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }

  return regionModifierArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierInRegion(
  std::string regionContextName,
  CodeIdentifier regionId,
  CodeIdentifier modifierId)
{
  if (modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
  {
    return JSON_EMPTY_VALUE;
  }
  rapidjson::Value& regionModifierArray = this->GetRegionModifierArrayInRegion(regionContextName, regionId);
  if (regionModifierArray.IsNull())
  {
    vtkGenericWarningMacro("GetRegionModifierInRegion: Failed to find region modifier array for region '"
                           << regionId.CodeMeaning << "' in region context '" << regionContextName << "'");
    return JSON_EMPTY_VALUE;
  }

  int index = -1;
  return this->GetCodeInArray(modifierId, regionModifierArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::ConvertSegmentationDescriptorToTerminologyContext(
  rapidjson::Document& descriptorDoc,
  rapidjson::Document& convertedDoc,
  std::string contextName)
{
  if (!descriptorDoc.IsObject() || contextName.empty())
  {
    return false;
  }

  // Get segment attributes
  rapidjson::Value& segmentAttributesArray = descriptorDoc["segmentAttributes"];
  if (!segmentAttributesArray.IsArray())
  {
    vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Invalid segmentAttributes member");
    return false;
  }

  rapidjson::Document::AllocatorType& allocator = convertedDoc.GetAllocator();

  // Use terminology with context name if exists
  rapidjson::Value segmentationCodes;
  rapidjson::Value categoryArray;

  if (convertedDoc.IsObject() && convertedDoc.HasMember("SegmentationCodes"))
  {
    segmentationCodes = convertedDoc["SegmentationCodes"];
    categoryArray = segmentationCodes["Category"];
  }
  else
  {
    // If terminology was not found in the map, then initialize it and its members
    convertedDoc.SetObject();
    convertedDoc.AddMember("SegmentationCategoryTypeContextName", rapidjson::StringRef(contextName.c_str()), allocator);
    segmentationCodes.SetObject();
    categoryArray.SetArray();
  }

  // Parse segment attributes
  bool entryAdded = false;
  rapidjson::SizeType index = 0;
  while (index < segmentAttributesArray.Size())
  {
    rapidjson::Value& segmentAttributes = segmentAttributesArray[index];
    if (!segmentAttributes.IsArray())
    {
      ++index;
      continue;
    }
    // Note: "The reason for the inner list is that we have one single schema both for input and output. When we provide
    // input metafile,
    //       we can have multiple input files, and each file can have multiple labels, that is why we need to have list
    //       of lists"
    segmentAttributes = segmentAttributes[0]; // Enter "innerList"
    if (!segmentAttributes.HasMember("SegmentedPropertyCategoryCodeSequence")
        || !segmentAttributes.HasMember("SegmentedPropertyTypeCodeSequence")
        || !segmentAttributes.HasMember("recommendedDisplayRGBValue"))
    {
      vtkGenericWarningMacro(
        "ConvertSegmentationDescriptorToTerminologyContext: Invalid segment terminology entry at index " << index);
      ++index;
      continue;
    }
    rapidjson::Value& segmentCategory = segmentAttributes["SegmentedPropertyCategoryCodeSequence"];
    rapidjson::Value& segmentType = segmentAttributes["SegmentedPropertyTypeCodeSequence"];
    rapidjson::Value& segmentRecommendedDisplayRGBValue = segmentAttributes["recommendedDisplayRGBValue"];

    // Get type array if category already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier categoryId(segmentCategory["CodingSchemeDesignator"].GetString(),
                                                                 segmentCategory["CodeValue"].GetString(),
                                                                 segmentCategory["CodeMeaning"].GetString());
    int foundCategoryIndex = -1;
    rapidjson::Value category(this->GetCodeInArray(categoryId, categoryArray, foundCategoryIndex), allocator);
    rapidjson::Value typeArray;
    if (category.IsObject() && category.HasMember("Type"))
    {
      typeArray = category["Type"];
      if (!typeArray.IsArray())
      {
        vtkGenericWarningMacro(
          "ConvertSegmentationDescriptorToTerminologyContext: Failed to find Type array in category '"
          << categoryId.CodeMeaning);
        ++index;
        continue;
      }
    }
    else
    {
      // If category was not found in context, then initialize it as object and initialize empty type array
      category.SetObject();
      typeArray.SetArray();
    }

    // Get type from type array, create empty type if not found
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeId(segmentType["CodingSchemeDesignator"].GetString(),
                                                             segmentType["CodeValue"].GetString(),
                                                             segmentType["CodeMeaning"].GetString());
    int foundTypeIndex = -1;
    rapidjson::Value type(this->GetCodeInArray(typeId, typeArray, foundTypeIndex), allocator);
    rapidjson::Value typeModifierArray;
    if (type.IsObject())
    {
      if (type.HasMember("Modifier"))
      {
        typeModifierArray = type["Modifier"];
      }
    }
    else
    {
      // If type was not found in category, then initialize it as object and initialize empty modifier array
      type.SetObject();
      typeModifierArray.SetArray();
    }

    // Make a copy of the recommended color array so that it's allocated using the allocator of
    // convertedDoc and not descriptorDoc (which will be deleted soon after exiting this function)
    if (!segmentRecommendedDisplayRGBValue.IsArray() || segmentRecommendedDisplayRGBValue.Size() != 3
        || !segmentRecommendedDisplayRGBValue[0].IsInt())
    {
      vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Invalid recommended color in type '"
                             << typeId.CodeMeaning);
      ++index;
      continue;
    }
    rapidjson::Value recommendedDisplayRGBValueArray(rapidjson::kArrayType);
    recommendedDisplayRGBValueArray.PushBack(segmentRecommendedDisplayRGBValue[0].GetInt(), allocator);
    recommendedDisplayRGBValueArray.PushBack(segmentRecommendedDisplayRGBValue[1].GetInt(), allocator);
    recommendedDisplayRGBValueArray.PushBack(segmentRecommendedDisplayRGBValue[2].GetInt(), allocator);

    // Add modifier if specified in descriptor and does not yet exist in terminology
    if (segmentAttributes.HasMember("SegmentedPropertyTypeModifierCodeSequence"))
    {
      rapidjson::Value& segmentTypeModifier = segmentAttributes["SegmentedPropertyTypeModifierCodeSequence"];
      vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeModifierId(
        segmentTypeModifier["CodingSchemeDesignator"].GetString(),
        segmentTypeModifier["CodeValue"].GetString(),
        segmentTypeModifier["CodeMeaning"].GetString());
      int foundTypeModifierIndex = -1;
      rapidjson::Value typeModifier(this->GetCodeInArray(typeModifierId, typeModifierArray, foundTypeModifierIndex),
                                    allocator);
      // Modifier already exists, nothing to do
      if (typeModifier.IsObject())
      {
        ++index;
        continue;
      }
      else
      {
        typeModifier.SetObject();
      }

      // Create and populate modifier
      this->GetJsonCodeFromIdentifier(typeModifier, typeModifierId, allocator);
      if (typeModifier.HasMember("recommendedDisplayRGBValue"))
      {
        typeModifier.EraseMember("recommendedDisplayRGBValue");
      }
      typeModifier.AddMember(
        "recommendedDisplayRGBValue", recommendedDisplayRGBValueArray, allocator); // Add color to type modifier

      // Set modifier to type
      typeModifierArray.PushBack(typeModifier, allocator);
      if (type.HasMember("Modifier"))
      {
        type.EraseMember("Modifier");
      }
      type.AddMember("Modifier", typeModifierArray, allocator);
    }
    else
    {
      // Add color to type if there is no modifier
      if (type.HasMember("recommendedDisplayRGBValue"))
      {
        type.EraseMember("recommendedDisplayRGBValue");
      }
      type.AddMember("recommendedDisplayRGBValue", recommendedDisplayRGBValueArray, allocator);
    }

    // Add type if has not been added yet
    this->GetJsonCodeFromIdentifier(type, typeId, allocator);
    if (foundTypeIndex == -1)
    {
      typeArray.PushBack(type, allocator);
    }
    else
    {
      typeArray[foundTypeIndex] = type;
    }

    // Set type array category
    if (category.HasMember("Type"))
    {
      category.EraseMember("Type");
    }
    category.AddMember("Type", typeArray, allocator);

    // Add category if has not been added yet
    this->GetJsonCodeFromIdentifier(category, categoryId, allocator);
    if (foundCategoryIndex == -1)
    {
      categoryArray.PushBack(category, allocator);
    }
    else
    {
      categoryArray[foundCategoryIndex] = category;
    }

    entryAdded = true;
    ++index;
  } // For all segments

  // Set objects back to terminology Json object
  if (entryAdded)
  {
    if (segmentationCodes.HasMember("Category"))
    {
      segmentationCodes.EraseMember("Category");
    }
    segmentationCodes.AddMember("Category", categoryArray, allocator);

    if (convertedDoc.HasMember("SegmentationCodes"))
    {
      convertedDoc.EraseMember("SegmentationCodes");
    }
    convertedDoc.AddMember("SegmentationCodes", segmentationCodes, allocator);

    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::ConvertSegmentationDescriptorToRegionContext(
  rapidjson::Document& descriptorDoc,
  rapidjson::Document& convertedDoc,
  std::string contextName)
{
  if (!descriptorDoc.IsObject() || contextName.empty())
  {
    return false;
  }

  // Get segment attributes
  rapidjson::Value& segmentAttributesArray = descriptorDoc["segmentAttributes"];
  if (!segmentAttributesArray.IsArray())
  {
    vtkGenericWarningMacro("ConvertSegmentationDescriptorToRegionContext: Invalid segmentAttributes member");
    return false;
  }

  rapidjson::Document::AllocatorType& allocator = convertedDoc.GetAllocator();

  // Use terminology with context name if exists
  rapidjson::Value regionCodes;
  rapidjson::Value regionArray;
  if (convertedDoc.IsObject() && convertedDoc.HasMember("AnatomicCodes"))
  {
    regionCodes = convertedDoc["AnatomicCodes"];
    regionArray = regionCodes["AnatomicRegion"];
  }
  else
  {
    // If region context was not found in the map, then initialize it and its members
    convertedDoc.SetObject();
    convertedDoc.AddMember("AnatomicContextName", rapidjson::StringRef(contextName.c_str()), allocator);
    regionCodes.SetObject();
    regionArray.SetArray();
  }

  // Parse segment attributes
  bool entryAdded = false;
  rapidjson::SizeType index = 0;
  while (index < segmentAttributesArray.Size())
  {
    rapidjson::Value& segmentAttributes = segmentAttributesArray[index];
    if (!segmentAttributes.IsArray())
    {
      ++index;
      continue;
    }
    // Note: "The reason for the inner list is that we have one single schema both for input and output. When we provide
    // input metafile,
    //       we can have multiple input files, and each file can have multiple labels, that is why we need to have list
    //       of lists"
    segmentAttributes = segmentAttributes[0]; // Enter "innerList"
    if (!segmentAttributes.HasMember("AnatomicRegionSequence"))
    {
      // Region context is optional in the descriptor file
      ++index;
      continue;
    }
    rapidjson::Value& segmentRegion = segmentAttributes["AnatomicRegionSequence"];

    // Get region modifier array if region already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionId(segmentRegion["CodingSchemeDesignator"].GetString(),
                                                               segmentRegion["CodeValue"].GetString(),
                                                               segmentRegion["CodeMeaning"].GetString());
    int foundRegionIndex = -1;
    rapidjson::Value region(this->GetCodeInArray(regionId, regionArray, foundRegionIndex), allocator);
    rapidjson::Value regionModifierArray;
    if (region.IsObject())
    {
      if (region.HasMember("Modifier"))
      {
        regionModifierArray = region["Modifier"];
      }
    }
    else
    {
      // If region was not found in context, then initialize it as object and initialize empty modifier array
      region.SetObject();
      regionModifierArray.SetArray();
    }

    // Add modifier if specified in descriptor and does not yet exist in region context
    if (segmentAttributes.HasMember("AnatomicRegionModifierSequence"))
    {
      rapidjson::Value& segmentRegionModifier = segmentAttributes["AnatomicRegionModifierSequence"];
      vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionModifierId(
        segmentRegionModifier["CodingSchemeDesignator"].GetString(),
        segmentRegionModifier["CodeValue"].GetString(),
        segmentRegionModifier["CodeMeaning"].GetString());
      int foundRegionModifierIndex = -1;
      rapidjson::Value regionModifier(
        this->GetCodeInArray(regionModifierId, regionModifierArray, foundRegionModifierIndex), allocator);
      // Modifier already exists, nothing to do
      if (regionModifier.IsObject())
      {
        ++index;
        continue;
      }

      // Create and populate modifier
      regionModifier.SetObject();
      this->GetJsonCodeFromIdentifier(regionModifier, regionModifierId, allocator);

      // Set modifier to region
      regionModifierArray.PushBack(regionModifier, allocator);
      if (region.HasMember("Modifier"))
      {
        region.EraseMember("Modifier");
      }
      region.AddMember("Modifier", regionModifierArray, allocator);
    }

    // Add region if has not been added yet, overwrite otherwise
    this->GetJsonCodeFromIdentifier(region, regionId, allocator);
    if (foundRegionIndex == -1)
    {
      regionArray.PushBack(region, allocator);
    }
    else
    {
      regionArray[foundRegionIndex] = region;
    }

    entryAdded = true;
    ++index;
  }

  // Set objects back to region context Json object
  if (entryAdded)
  {
    if (regionCodes.HasMember("AnatomicRegion"))
    {
      regionCodes.EraseMember("AnatomicRegion");
    }
    regionCodes.AddMember("AnatomicRegion", regionArray, allocator);

    if (convertedDoc.HasMember("AnatomicCodes"))
    {
      convertedDoc.EraseMember("AnatomicCodes");
    }
    convertedDoc.AddMember("AnatomicCodes", regionCodes, allocator);

    return true;
  }

  return false;
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::vtkInternal::GetJsonCodeFromIdentifier(
  rapidjson::Value& code,
  CodeIdentifier identifier,
  rapidjson::Document::AllocatorType& allocator)
{
  code.AddMember("CodingSchemeDesignator",
                 rapidjson::Value().SetString(identifier.CodingSchemeDesignator.c_str(), allocator),
                 allocator);
  code.AddMember("CodeValue", rapidjson::Value().SetString(identifier.CodeValue.c_str(), allocator), allocator);
  code.AddMember("CodeMeaning", rapidjson::Value().SetString(identifier.CodeMeaning.c_str(), allocator), allocator);
}

//---------------------------------------------------------------------------
// vtkSlicerTerminologiesModuleLogic methods

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkSlicerTerminologiesModuleLogic()
{
  this->Internal = new vtkInternal();
}

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::~vtkSlicerTerminologiesModuleLogic()
{
  delete this->Internal;
  this->Internal = nullptr;

  this->SetUserContextsPath(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetMRMLSceneInternal(vtkMRMLScene* newScene)
{
  Superclass::SetMRMLSceneInternal(newScene);

  // Load default terminologies and region contexts
  // Note: Do it here not in the constructor so that the module shared directory is properly initialized
  bool wasModifying = this->GetDisableModifiedEvent();
  this->SetDisableModifiedEvent(true);
  this->LoadDefaultTerminologies();
  this->LoadDefaultRegionContexts();
  this->LoadUserContexts();
  this->SetDisableModifiedEvent(wasModifying);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadContextFromFile(std::string filePath)
{
  rapidjson::Document* jsonRoot = new rapidjson::Document;

  FILE* fp = fopen(filePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadContextFromFile: Failed to load context from file '" << filePath);
    delete jsonRoot;
    return false;
  }
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (jsonRoot->ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadContextFromFile: Failed to load context from file '" << filePath);
    fclose(fp);
    delete jsonRoot;
    return false;
  }

  // Load document based on schema
  rapidjson::Value::MemberIterator schemaIt = jsonRoot->FindMember("@schema");
  if (schemaIt == jsonRoot->MemberEnd())
  {
    vtkErrorMacro("LoadContextFromFile: File " << filePath << " does not contain schema information");
    fclose(fp);
    delete jsonRoot;
    return false;
  }
  std::string schema = (*jsonRoot)["@schema"].GetString();
  if (!schema.compare(TERMINOLOGY_CONTEXT_SCHEMA) || !schema.compare(TERMINOLOGY_CONTEXT_SCHEMA_1))
  {
    // Store terminology
    std::string contextName = (*jsonRoot)["SegmentationCategoryTypeContextName"].GetString();
    vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
      this->Internal->LoadedTerminologies, contextName, jsonRoot);
    vtkDebugMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  }
  else if (!schema.compare(REGION_CONTEXT_SCHEMA) || !schema.compare(REGION_CONTEXT_SCHEMA_1))
  {
    // Store region context
    std::string contextName = (*jsonRoot)["AnatomicContextName"].GetString();
    vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
      this->Internal->LoadedRegionContexts, contextName, jsonRoot);
    vtkDebugMacro("Region context named '" << contextName << "' successfully loaded from file " << filePath);
  }
  else
  {
    vtkErrorMacro("LoadContextFromFile: File "
                  << filePath << " is neither a terminology nor region context file according to its schema");
    fclose(fp);
    delete jsonRoot;
    return false;
  }

  fclose(fp);
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromFile(std::string filePath)
{
  rapidjson::Document* terminologyRoot = new rapidjson::Document;

  FILE* fp = fopen(filePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadTerminologyFromFile: Failed to load terminology from file '" << filePath << "'");
    delete terminologyRoot;
    return "";
  }
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (terminologyRoot->ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadTerminologyFromFile: Failed to load terminology from file '" << filePath << "'");
    fclose(fp);
    delete terminologyRoot;
    return "";
  }

  // Check schema
  rapidjson::Value::MemberIterator schemaIt = terminologyRoot->FindMember("@schema");
  if (schemaIt == terminologyRoot->MemberEnd())
  {
    vtkErrorMacro("LoadTerminologyFromFile: File " << filePath << " does not contain schema information");
    fclose(fp);
    delete terminologyRoot;
    return "";
  }
  std::string schema = (*terminologyRoot)["@schema"].GetString();
  if (schema.compare(TERMINOLOGY_CONTEXT_SCHEMA) && schema.compare(TERMINOLOGY_CONTEXT_SCHEMA_1))
  {
    vtkErrorMacro("LoadTerminologyFromFile: File " << filePath
                                                   << " is not a terminology context file according to its schema");
    fclose(fp);
    delete terminologyRoot;
    return "";
  }

  // Store terminology
  std::string contextName = (*terminologyRoot)["SegmentationCategoryTypeContextName"].GetString();
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedTerminologies, contextName, terminologyRoot);

  vtkDebugMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  fclose(fp);
  this->Modified();
  return contextName;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromSegmentDescriptorFile(std::string contextName,
                                                                                 std::string filePath)
{
  FILE* fp = fopen(filePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadTerminologyFromSegmentDescriptorFile: Failed to load terminology from file '" << filePath);
    return false;
  }

  rapidjson::Document descriptorDoc;
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (descriptorDoc.ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadTerminologyFromSegmentDescriptorFile: Failed to load terminology from file '" << filePath);
    fclose(fp);
    return false;
  }

  // Convert the loaded descriptor json file into terminology dictionary context json format
  rapidjson::Document* convertedDoc = nullptr;
  vtkInternal::TerminologyMap::iterator termIt = this->Internal->LoadedTerminologies.find(contextName);
  if (termIt != this->Internal->LoadedTerminologies.end() && termIt->second != nullptr)
  {
    convertedDoc = termIt->second;
  }
  else
  {
    convertedDoc = new rapidjson::Document;
  }

  bool success =
    this->Internal->ConvertSegmentationDescriptorToTerminologyContext(descriptorDoc, *convertedDoc, contextName);
  if (!success)
  {
    vtkErrorMacro("LoadTerminologyFromSegmentDescriptorFile: Failed to parse descriptor file '" << filePath);
    fclose(fp);
    return false;
  }

  // Store terminology
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedTerminologies, contextName, convertedDoc);

  vtkDebugMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  fclose(fp);
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::LoadDefaultTerminologies()
{
  std::string success("");
  success = this->LoadTerminologyFromFile(this->GetModuleShareDirectory()
                                          + "/SegmentationCategoryTypeModifier-SlicerGeneralAnatomy.term.json");
  if (success.empty())
  {
    vtkErrorMacro(
      "LoadDefaultTerminologies: Failed to load terminology 'SegmentationCategoryTypeModifier-SlicerGeneralAnatomy'");
  }
  success = this->LoadTerminologyFromFile(this->GetModuleShareDirectory()
                                          + "/SegmentationCategoryTypeModifier-DICOM-Master.term.json");
  if (success.empty())
  {
    vtkErrorMacro(
      "LoadDefaultTerminologies: Failed to load terminology 'SegmentationCategoryTypeModifier-DICOM-Master'");
  }
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::LoadRegionContextFromFile(std::string filePath)
{
  rapidjson::Document* regionContextRoot = new rapidjson::Document;

  FILE* fp = fopen(filePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadRegionContextFromFile: Failed to load region context from file " << filePath);
    delete regionContextRoot;
    return "";
  }

  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (regionContextRoot->ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadRegionContextFromFile: Failed to load region context from file " << filePath);
    fclose(fp);
    delete regionContextRoot;
    return "";
  }

  // Check schema
  rapidjson::Value::MemberIterator schemaIt = regionContextRoot->FindMember("@schema");
  if (schemaIt == regionContextRoot->MemberEnd())
  {
    vtkErrorMacro("LoadRegionContextFromFile: File " << filePath << " does not contain schema information");
    fclose(fp);
    delete regionContextRoot;
    return "";
  }
  std::string schema = (*regionContextRoot)["@schema"].GetString();
  if (schema.compare(REGION_CONTEXT_SCHEMA) && schema.compare(REGION_CONTEXT_SCHEMA_1))
  {
    vtkErrorMacro("LoadRegionContextFromFile: File " << filePath
                                                     << " is not an region context file according to its schema");
    fclose(fp);
    delete regionContextRoot;
    return "";
  }

  // Store region context
  std::string contextName = (*regionContextRoot)["AnatomicContextName"].GetString();
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedRegionContexts, contextName, regionContextRoot);

  vtkDebugMacro("REgion context named '" << contextName << "' successfully loaded from file " << filePath);
  fclose(fp);
  this->Modified();
  return contextName;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadRegionContextFromSegmentDescriptorFile(std::string contextName,
                                                                                   std::string filePath)
{
  FILE* fp = fopen(filePath.c_str(), "r");
  if (!fp)
  {
    vtkErrorMacro("LoadRegionContextFromSegmentDescriptorFile: Failed to load terminology from file '" << filePath);
    return false;
  }

  rapidjson::Document descriptorDoc;
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (descriptorDoc.ParseStream(fs).HasParseError())
  {
    vtkErrorMacro("LoadRegionContextFromSegmentDescriptorFile: Failed to load terminology from file '" << filePath);
    fclose(fp);
    return false;
  }

  // Convert the loaded descriptor json file into region context json format
  rapidjson::Document* convertedDoc = nullptr;
  vtkInternal::TerminologyMap::iterator anIt = this->Internal->LoadedRegionContexts.find(contextName);
  if (anIt != this->Internal->LoadedRegionContexts.end() && anIt->second != nullptr)
  {
    convertedDoc = anIt->second;
  }
  else
  {
    convertedDoc = new rapidjson::Document;
  }

  bool success =
    this->Internal->ConvertSegmentationDescriptorToRegionContext(descriptorDoc, *convertedDoc, contextName);
  if (!success)
  {
    // Region context is optional in descriptor file
    fclose(fp);
    return false;
  }

  // Store region context
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedRegionContexts, contextName, convertedDoc);

  vtkDebugMacro("Region context named '" << contextName << "' successfully loaded from file " << filePath);
  fclose(fp);
  this->Modified();
  return true;
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::LoadDefaultRegionContexts()
{
  std::string success("");
  success = this->LoadRegionContextFromFile(this->GetModuleShareDirectory()
                                            + "/AnatomicRegionAndModifier-DICOM-Master.term.json");
  if (success.empty())
  {
    vtkErrorMacro("LoadDefaultRegionContexts: Failed to load region context 'AnatomicRegionAndModifier-DICOM-Master'");
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::LoadUserContexts()
{
  if (!this->UserContextsPath || !vtksys::SystemTools::FileExists(this->UserContextsPath, false))
  {
    // user contexts path is not set or the folder does not exist
    return;
  }

  // Try to load all json files in the user settings directory
  vtkSmartPointer<vtkDirectory> userSettingsDir = vtkSmartPointer<vtkDirectory>::New();
  userSettingsDir->Open(this->UserContextsPath);
  vtkStringArray* files = userSettingsDir->GetFiles();
  for (int index = 0; index < files->GetNumberOfValues(); ++index)
  {
    std::string fileName = files->GetValue(index);

    // Only load json files
    if (userSettingsDir->FileIsDirectory(fileName.c_str()) || fileName.size() < 5
        || fileName.substr(fileName.size() - 5).compare(".json"))
    {
      continue;
    }

    // Try loading file
    std::string jsonFilePath = std::string(this->UserContextsPath) + "/" + fileName;
    if (!this->LoadContextFromFile(jsonFilePath))
    {
      vtkErrorMacro("LoadUserContexts: Failed to load terminology from file " << files->GetValue(index));
    }
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedTerminologyNames(std::vector<std::string>& terminologyNames)
{
  terminologyNames.clear();

  vtkSlicerTerminologiesModuleLogic::vtkInternal::TerminologyMap::iterator termIt;
  for (termIt = this->Internal->LoadedTerminologies.begin(); termIt != this->Internal->LoadedTerminologies.end();
       ++termIt)
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
  for (std::vector<std::string>::iterator termIt = terminologyNamesVector.begin();
       termIt != terminologyNamesVector.end();
       ++termIt)
  {
    terminologyNames->InsertNextValue(termIt->c_str());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedRegionContextNames(std::vector<std::string>& regionContextNames)
{
  regionContextNames.clear();

  vtkSlicerTerminologiesModuleLogic::vtkInternal::TerminologyMap::iterator anIt;
  for (anIt = this->Internal->LoadedRegionContexts.begin(); anIt != this->Internal->LoadedRegionContexts.end(); ++anIt)
  {
    regionContextNames.push_back(anIt->first);
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::GetLoadedRegionContextNames(vtkStringArray* regionContextNames)
{
  if (!regionContextNames)
  {
    return;
  }
  regionContextNames->Initialize();

  std::vector<std::string> regionContextNamesVector;
  this->GetLoadedRegionContextNames(regionContextNamesVector);
  for (std::vector<std::string>::iterator anIt = regionContextNamesVector.begin();
       anIt != regionContextNamesVector.end();
       ++anIt)
  {
    regionContextNames->InsertNextValue(anIt->c_str());
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetCategoriesInTerminology(std::string terminologyName,
                                                                   std::vector<CodeIdentifier>& categories)
{
  return this->FindCategoriesInTerminology(terminologyName, categories, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindCategoriesInTerminology(std::string terminologyName,
                                                                    std::vector<CodeIdentifier>& categories,
                                                                    std::string search /*=""*/)
{
  categories.clear();

  rapidjson::Value& categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
  {
    vtkErrorMacro("FindCategoriesInTerminology: Failed to find category array in terminology '" << terminologyName
                                                                                                << "'");
    return false;
  }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse categories
  rapidjson::SizeType index = 0;
  while (index < categoryArray.Size())
  {
    rapidjson::Value& category = categoryArray[index];
    if (category.IsObject())
    {
      rapidjson::Value& categoryName = category["CodeMeaning"];
      rapidjson::Value& categoryCodingSchemeDesignator = category["CodingSchemeDesignator"];
      rapidjson::Value& categoryCodeValue = category["CodeValue"];
      if (categoryName.IsString() && categoryCodingSchemeDesignator.IsString() && categoryCodeValue.IsString())
      {
        // Add category to list if search string is empty or is contained by the current category name
        std::string categoryNameStr = categoryName.GetString();
        std::string categoryNameLowerCase(categoryNameStr);
        std::transform(
          categoryNameLowerCase.begin(), categoryNameLowerCase.end(), categoryNameLowerCase.begin(), ::tolower);
        if (search.empty() || categoryNameLowerCase.find(search) != std::string::npos)
        {
          CodeIdentifier categoryId(
            categoryCodingSchemeDesignator.GetString(), categoryCodeValue.GetString(), categoryNameStr);
          categories.push_back(categoryId);
        }
      }
      else
      {
        vtkErrorMacro("FindCategoriesInTerminology: Invalid category '"
                      << categoryName.GetString() << "' in terminology '" << terminologyName << "'");
      }
    }
    ++index;
  }

  return true;
}

//---------------------------------------------------------------------------
int vtkSlicerTerminologiesModuleLogic::GetNumberOfCategoriesInTerminology(std::string terminologyName)
{
  rapidjson::Value& categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
  {
    vtkErrorMacro("GetNumberOfCategoriesInTerminology: Failed to find category array in terminology '"
                  << terminologyName << "'");
    return 0;
  }
  return categoryArray.Size();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetNthCategoryInTerminology(std::string terminologyName,
                                                                    int categoryIndex,
                                                                    vtkSlicerTerminologyCategory* category)
{
  rapidjson::Value& categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
  {
    vtkErrorMacro("GetNthCategoryInTerminology failed: Failed to find category array in terminology '"
                  << terminologyName << "'");
    return false;
  }
  if (!category)
  {
    vtkErrorMacro("GetNthCategoryInTerminology failed: category is invalid");
    return false;
  }

  if (categoryIndex < 0 || categoryIndex >= static_cast<int>(categoryArray.Size()))
  {
    vtkErrorMacro("GetNthCategoryInTerminology failed: category index of "
                  << categoryIndex << " is out of range"
                  << " (number of categories: " << categoryArray.Size() << ")");
    return false;
  }

  rapidjson::Value& categoryObject = categoryArray[categoryIndex];
  if (categoryObject.IsNull())
  {
    vtkErrorMacro("GetCategoryInTerminology: Failed to find category in terminology '" << terminologyName << "'");
    return false;
  }

  // Category found
  return this->Internal->PopulateTerminologyCategoryFromJson(categoryObject, category);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetCategoryInTerminology(std::string terminologyName,
                                                                 CodeIdentifier categoryId,
                                                                 vtkSlicerTerminologyCategory* category)
{
  if (!category || categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
  {
    return false;
  }

  rapidjson::Value& categoryObject = this->Internal->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.IsNull())
  {
    return false;
  }

  // Category found
  return this->Internal->PopulateTerminologyCategoryFromJson(categoryObject, category);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypesInTerminologyCategory(std::string terminologyName,
                                                                      CodeIdentifier categoryId,
                                                                      std::vector<CodeIdentifier>& types)
{
  return this->FindTypesInTerminologyCategory(terminologyName, categoryId, types, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindTypesInTerminologyCategory(
  std::string terminologyName,
  CodeIdentifier categoryId,
  std::vector<CodeIdentifier>& types,
  std::string search,
  std::vector<vtkSmartPointer<vtkSlicerTerminologyType>>* typeObjects /*=nullptr*/)
{
  types.clear();

  rapidjson::Value& typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.IsNull())
  {
    vtkErrorMacro("FindTypesInTerminologyCategory: Failed to find type array in category '"
                  << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
  }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse types
  rapidjson::SizeType index = 0;
  while (index < typeArray.Size())
  {
    rapidjson::Value& type = typeArray[index];
    if (type.IsObject())
    {
      rapidjson::Value& typeName = type["CodeMeaning"];
      rapidjson::Value& typeCodingSchemeDesignator = type["CodingSchemeDesignator"];
      rapidjson::Value& typeCodeValue = type["CodeValue"];
      if (typeName.IsString() && typeCodingSchemeDesignator.IsString() && typeCodeValue.IsString())
      {
        // Add type to list if search string is empty or is contained by the current type name
        std::string typeNameStr = typeName.GetString();
        std::string typeNameLowerCase(typeNameStr);
        std::transform(typeNameLowerCase.begin(), typeNameLowerCase.end(), typeNameLowerCase.begin(), ::tolower);
        if (search.empty() || typeNameLowerCase.find(search) != std::string::npos)
        {
          CodeIdentifier typeId(typeCodingSchemeDesignator.GetString(), typeCodeValue.GetString(), typeNameStr);
          types.push_back(typeId);
          if (typeObjects)
          {
            vtkSmartPointer<vtkSlicerTerminologyType> typeObject = vtkSmartPointer<vtkSlicerTerminologyType>::New();
            this->Internal->PopulateTerminologyTypeFromJson(type, typeObject);
            typeObjects->push_back(typeObject);
          }
        }
      }
      else
      {
        vtkErrorMacro("FindTypesInTerminologyCategory: Invalid type '" << typeName.GetString() << "in category '"
                                                                       << categoryId.CodeMeaning << "' in terminology '"
                                                                       << terminologyName << "'");
      }
    }
    ++index;
  }

  return true;
}

//---------------------------------------------------------------------------
int vtkSlicerTerminologiesModuleLogic::GetNumberOfTypesInTerminologyCategory(std::string terminologyName,
                                                                             vtkSlicerTerminologyCategory* category)
{
  CodeIdentifier categoryId = this->GetCodeIdentifierFromCodedEntry(category);
  rapidjson::Value& typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.IsNull())
  {
    vtkErrorMacro("GetNumberOfTypesInTerminologyCategory: Failed to find type array in category '"
                  << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return 0;
  }
  return typeArray.Size();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetNthTypeInTerminologyCategory(std::string terminologyName,
                                                                        vtkSlicerTerminologyCategory* category,
                                                                        int typeIndex,
                                                                        vtkSlicerTerminologyType* type)
{
  CodeIdentifier categoryId = this->GetCodeIdentifierFromCodedEntry(category);
  rapidjson::Value& typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.IsNull())
  {
    vtkErrorMacro("GetNthTypeInTerminologyCategory: Failed to find type array in category '"
                  << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
  }
  if (!type)
  {
    vtkErrorMacro("GetNthTypeInTerminologyCategory failed: invalid type");
    return false;
  }

  if (typeIndex < 0 || typeIndex >= static_cast<int>(typeArray.Size()))
  {
    vtkErrorMacro("GetNthTypeInTerminologyCategory failed: typeIndex of "
                  << typeIndex << " is out of range (number of types: " << typeArray.Size() << ")");
    return false;
  }
  rapidjson::Value& typeObject = typeArray[typeIndex];
  if (!typeObject.IsObject())
  {
    vtkErrorMacro("GetNthTypeInTerminologyCategory failed: invalid type");
    return false;
  }
  // Type found
  return this->Internal->PopulateTerminologyTypeFromJson(typeObject, type);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeInTerminologyCategory(std::string terminologyName,
                                                                     CodeIdentifier categoryId,
                                                                     CodeIdentifier typeId,
                                                                     vtkSlicerTerminologyType* type)
{
  if (!type || typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
  {
    return false;
  }

  rapidjson::Value& typeObject = this->Internal->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId);
  if (typeObject.IsNull())
  {
    // Not found. This is not an error, as the type may not exist in the terminology and this method can be used to
    // check for existence.
    return false;
  }

  // Type found
  return this->Internal->PopulateTerminologyTypeFromJson(typeObject, type);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifiersInTerminologyType(std::string terminologyName,
                                                                          CodeIdentifier categoryId,
                                                                          CodeIdentifier typeId,
                                                                          std::vector<CodeIdentifier>& typeModifiers)
{
  typeModifiers.clear();

  rapidjson::Value& typeModifierArray =
    this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.IsNull())
  {
    vtkErrorMacro("GetTypeModifiersInTerminologyType: Failed to find type modifier array member in type '"
                  << typeId.CodeMeaning << "' in category " << categoryId.CodeMeaning << "' in terminology '"
                  << terminologyName << "'");
    return false;
  }

  // Collect type modifiers
  rapidjson::SizeType index = 0;
  while (index < typeModifierArray.Size())
  {
    rapidjson::Value& typeModifier = typeModifierArray[index];
    if (typeModifier.IsObject())
    {
      rapidjson::Value& typeModifierName = typeModifier["CodeMeaning"];
      rapidjson::Value& typeModifierCodingSchemeDesignator = typeModifier["CodingSchemeDesignator"];
      rapidjson::Value& typeModifierCodeValue = typeModifier["CodeValue"];
      if (typeModifierName.IsString() && typeModifierCodingSchemeDesignator.IsString()
          && typeModifierCodeValue.IsString())
      {
        CodeIdentifier typeModifierId(typeModifierCodingSchemeDesignator.GetString(),
                                      typeModifierCodeValue.GetString(),
                                      typeModifierName.GetString());
        typeModifiers.push_back(typeModifierId);
      }
    }
    ++index;
  } // For each type modifier index

  return true;
}

//---------------------------------------------------------------------------
int vtkSlicerTerminologiesModuleLogic::GetNumberOfTypeModifiersInTerminologyType(std::string terminologyName,
                                                                                 vtkSlicerTerminologyCategory* category,
                                                                                 vtkSlicerTerminologyType* type)
{
  CodeIdentifier categoryId = this->GetCodeIdentifierFromCodedEntry(category);
  CodeIdentifier typeId = this->GetCodeIdentifierFromCodedEntry(type);
  rapidjson::Value& typeModifierArray =
    this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.IsNull())
  {
    vtkErrorMacro("GetNumberOfTypeModifiersInTerminologyType: Failed to find type modifier array for type '"
                  << typeId.CodeMeaning << "' in category '" << categoryId.CodeMeaning << "' in terminology '"
                  << terminologyName << "'");
    return 0;
  }
  return typeModifierArray.Size();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetNthTypeModifierInTerminologyType(std::string terminologyName,
                                                                            vtkSlicerTerminologyCategory* category,
                                                                            vtkSlicerTerminologyType* type,
                                                                            int typeModifierIndex,
                                                                            vtkSlicerTerminologyType* typeModifier)
{
  CodeIdentifier categoryId = this->GetCodeIdentifierFromCodedEntry(category);
  CodeIdentifier typeId = this->GetCodeIdentifierFromCodedEntry(type);
  rapidjson::Value& typeModifierArray =
    this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.IsNull())
  {
    vtkErrorMacro("GetNthTypeModifierInTerminologyType: Failed to find type modifier array for type '"
                  << typeId.CodeMeaning << "' in category '" << categoryId.CodeMeaning << "' in terminology '"
                  << terminologyName << "'");
    return 0;
  }
  if (!typeModifier)
  {
    vtkErrorMacro("GetNthTypeModifierInTerminologyType failed: typeModifier is invalid");
    return false;
  }
  if (typeModifierIndex < 0 || typeModifierIndex >= static_cast<int>(typeModifierArray.Size()))
  {
    vtkErrorMacro("GetNthTypeModifierInTerminologyType failed: type modifier index of "
                  << typeModifierIndex << " is out of range"
                  << " (number of type modifiers: " << typeModifierArray.Size() << ")");
    return false;
  }

  rapidjson::Value& typeModifierObject = typeModifierArray[typeModifierIndex];
  if (typeModifierObject.IsNull())
  {
    vtkErrorMacro("GetNthTypeModifierInTerminologyType: Failed to find type modifier in terminology '"
                  << terminologyName << "'");
    return false;
  }

  // Type modifier found
  return this->Internal->PopulateTerminologyTypeFromJson(typeModifierObject, typeModifier);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifierInTerminologyType(std::string terminologyName,
                                                                         CodeIdentifier categoryId,
                                                                         CodeIdentifier typeId,
                                                                         CodeIdentifier modifierId,
                                                                         vtkSlicerTerminologyType* typeModifier)
{
  if (!typeModifier || modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
  {
    return false;
  }

  rapidjson::Value& typeModifierObject =
    this->Internal->GetTypeModifierInTerminologyType(terminologyName, categoryId, typeId, modifierId);
  if (typeModifierObject.IsNull())
  {
    vtkErrorMacro("GetTypeModifierInTerminologyType: Failed to find type modifier '"
                  << modifierId.CodeMeaning << "' in type '" << typeId.CodeMeaning << "' in category '"
                  << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
  }

  // Type modifier with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(typeModifierObject, typeModifier);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionsInRegionContext(std::string regionContextName,
                                                                  std::vector<CodeIdentifier>& regions)
{
  return this->FindRegionsInRegionContext(regionContextName, regions, "");
}

//---------------------------------------------------------------------------
int vtkSlicerTerminologiesModuleLogic::GetNumberOfRegionsInRegionContext(std::string regionContextName)
{
  std::vector<CodeIdentifier> regions;
  if (!this->GetRegionsInRegionContext(regionContextName, regions))
  {
    return 0;
  }
  return regions.size();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetNthRegionInRegionContext(std::string regionContextName,
                                                                    int regionIndex,
                                                                    vtkSlicerTerminologyType* regionObject)
{
  if (!regionObject)
  {
    vtkErrorMacro("GetNthRegionInRegionContext failed: regionObject is invalid)");
    return false;
  }
  std::vector<CodeIdentifier> regions;
  if (!this->GetRegionsInRegionContext(regionContextName, regions))
  {
    return false;
  }
  if (regionIndex < 0 || regionIndex >= regions.size())
  {
    vtkErrorMacro("GetNthRegionInRegionContext failed: region index of " << regionIndex << " is out of range"
                                                                         << " (number of regions: " << regions.size()
                                                                         << ")");
    return false;
  }

  return this->GetRegionInRegionContext(regionContextName, regions[regionIndex], regionObject);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindRegionsInRegionContext(std::string regionContextName,
                                                                   std::vector<CodeIdentifier>& regions,
                                                                   std::string search)
{
  regions.clear();

  rapidjson::Value& regionArray = this->Internal->GetRegionArrayInRegionContext(regionContextName);
  if (regionArray.IsNull())
  {
    vtkErrorMacro("FindRegionsInRegionContext: Failed to find region array member in region context '"
                  << regionContextName << "'");
    return false;
  }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse regions
  rapidjson::SizeType index = 0;
  while (index < regionArray.Size())
  {
    rapidjson::Value& region = regionArray[index];
    if (region.IsObject())
    {
      rapidjson::Value& regionName = region["CodeMeaning"];
      rapidjson::Value& regionCodingSchemeDesignator = region["CodingSchemeDesignator"];
      rapidjson::Value& regionCodeValue = region["CodeValue"];
      if (regionName.IsString() && regionCodingSchemeDesignator.IsString() && regionCodeValue.IsString())
      {
        // Add region name to list if search string is empty or is contained by the current region name
        std::string regionNameStr = regionName.GetString();
        std::string regionNameLowerCase(regionNameStr);
        std::transform(regionNameLowerCase.begin(), regionNameLowerCase.end(), regionNameLowerCase.begin(), ::tolower);
        if (search.empty() || regionNameLowerCase.find(search) != std::string::npos)
        {
          CodeIdentifier regionId(regionCodingSchemeDesignator.GetString(), regionCodeValue.GetString(), regionNameStr);
          regions.push_back(regionId);
        }
      }
      else
      {
        vtkErrorMacro("FindRegionsInRegionContext: Invalid region '"
                      << regionName.GetString() << "' in region context '" << regionContextName << "'");
      }
    }
    ++index;
  }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionInRegionContext(std::string regionContextName,
                                                                 CodeIdentifier regionId,
                                                                 vtkSlicerTerminologyType* region)
{
  if (!region || regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
  {
    return false;
  }

  rapidjson::Value& regionObject = this->Internal->GetRegionInRegionContext(regionContextName, regionId);
  if (regionObject.IsNull())
  {
    vtkErrorMacro("GetRegionInRegionContext: Failed to find region '" << regionId.CodeMeaning << "' in region context '"
                                                                      << regionContextName << "'");
    return false;
  }

  // Region with specified name found
  return this->Internal->PopulateRegionFromJson(regionObject, region);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifiersInRegion(std::string regionContextName,
                                                                   CodeIdentifier regionId,
                                                                   std::vector<CodeIdentifier>& regionModifiers)
{
  regionModifiers.clear();

  rapidjson::Value& regionModifierArray = this->Internal->GetRegionModifierArrayInRegion(regionContextName, regionId);
  if (regionModifierArray.IsNull())
  {
    vtkErrorMacro("GetRegionModifiersInRegion: Failed to find Region Modifier array member in region '"
                  << regionId.CodeMeaning << "' in region context '" << regionContextName << "'");
    return false;
  }

  // Collect region modifiers
  rapidjson::SizeType index = 0;
  while (index < regionModifierArray.Size())
  {
    rapidjson::Value& regionModifier = regionModifierArray[index];
    if (regionModifier.IsObject())
    {
      rapidjson::Value& regionModifierName = regionModifier["CodeMeaning"];
      rapidjson::Value& regionModifierCodingSchemeDesignator = regionModifier["CodingSchemeDesignator"];
      rapidjson::Value& regionModifierCodeValue = regionModifier["CodeValue"];
      if (regionModifierName.IsString() && regionModifierCodingSchemeDesignator.IsString()
          && regionModifierCodeValue.IsString())
      {
        CodeIdentifier regionModifierId(regionModifierCodingSchemeDesignator.GetString(),
                                        regionModifierCodeValue.GetString(),
                                        regionModifierName.GetString());
        regionModifiers.push_back(regionModifierId);
      }
    }
    ++index;
  } // For each region index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifierInRegion(std::string regionContextName,
                                                                  CodeIdentifier regionId,
                                                                  CodeIdentifier modifierId,
                                                                  vtkSlicerTerminologyType* regionModifier)
{
  if (!regionModifier || modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
  {
    return false;
  }

  rapidjson::Value& regionModifierObject =
    this->Internal->GetRegionModifierInRegion(regionContextName, regionId, modifierId);
  if (regionModifierObject.IsNull())
  {
    vtkErrorMacro("GetRegionModifierInRegion: Failed to find region modifier '"
                  << modifierId.CodeMeaning << "' in region '" << regionId.CodeMeaning << "' in region context '"
                  << regionContextName << "'");
    return false;
  }

  // Region modifier with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(regionModifierObject, regionModifier);
}

//---------------------------------------------------------------------------
int vtkSlicerTerminologiesModuleLogic::GetNumberOfRegionModifierInRegion(std::string regionContextName,
                                                                         vtkSlicerTerminologyType* regionObject)
{
  if (!regionObject)
  {
    vtkErrorMacro("GetNumberOfRegionModifierInRegion failed: regionObject is invalid)");
    return 0;
  }
  CodeIdentifier regionId(
    regionObject->GetCodingSchemeDesignator(), regionObject->GetCodeValue(), regionObject->GetCodeMeaning());
  std::vector<CodeIdentifier> regionModifiers;
  if (!this->GetRegionModifiersInRegion(regionContextName, regionId, regionModifiers))
  {
    return 0;
  }
  return regionModifiers.size();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetNthRegionModifierInRegion(std::string regionContextName,
                                                                     vtkSlicerTerminologyType* regionObject,
                                                                     int regionModifierIndex,
                                                                     vtkSlicerTerminologyType* regionModifier)
{
  if (!regionObject)
  {
    vtkErrorMacro("GetNthRegionInRegionContext failed: regionObject is invalid)");
    return false;
  }
  CodeIdentifier regionId(
    regionObject->GetCodingSchemeDesignator(), regionObject->GetCodeValue(), regionObject->GetCodeMeaning());
  std::vector<CodeIdentifier> regionModifiers;
  if (!this->GetRegionModifiersInRegion(regionContextName, regionId, regionModifiers))
  {
    return false;
  }

  if (regionModifierIndex < 0 || regionModifierIndex >= regionModifiers.size())
  {
    vtkErrorMacro("GetNthRegionModifierInRegion failed: regionModifier index of "
                  << regionModifierIndex << " is out of range"
                  << " (number of regionModifiers: " << regionModifiers.size() << ")");
    return false;
  }
  CodeIdentifier modifierId = regionModifiers[regionModifierIndex];
  return this->GetRegionModifierInRegion(regionContextName, regionId, modifierId, regionModifier);
}

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::CodeIdentifier vtkSlicerTerminologiesModuleLogic::GetCodeIdentifierFromCodedEntry(
  vtkCodedEntry* entry)
{
  if (!entry)
  {
    return CodeIdentifier("", "", "");
  }
  CodeIdentifier id((entry->GetCodingSchemeDesignator() ? entry->GetCodingSchemeDesignator() : ""),
                    (entry->GetCodeValue() ? entry->GetCodeValue() : ""),
                    (entry->GetCodeMeaning() ? entry->GetCodeMeaning() : ""));
  return id;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(vtkSlicerTerminologyEntry* entry)
{
  if (!entry || !entry->GetTerminologyContextName())
  {
    vtkGenericWarningMacro("vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry: Invalid terminology given");
    return "";
  }

  // Serialized terminology entry consists of the following: terminologyContextName, category (codingScheme,
  // codeValue, codeMeaning triple), type, typeModifier, regionContextName, region, regionModifier
  std::string serializedEntry;
  serializedEntry += std::string(entry->GetTerminologyContextName()) + "~";
  serializedEntry += std::string(entry->GetCategoryObject() && entry->GetCategoryObject()->GetCodingSchemeDesignator()
                                   ? entry->GetCategoryObject()->GetCodingSchemeDesignator()
                                   : "")
                     + "^"
                     + std::string(entry->GetCategoryObject() && entry->GetCategoryObject()->GetCodeValue()
                                     ? entry->GetCategoryObject()->GetCodeValue()
                                     : "")
                     + "^"
                     + std::string(entry->GetCategoryObject() && entry->GetCategoryObject()->GetCodeMeaning()
                                     ? entry->GetCategoryObject()->GetCodeMeaning()
                                     : "")
                     + "~";
  serializedEntry += std::string(entry->GetTypeObject() && entry->GetTypeObject()->GetCodingSchemeDesignator()
                                   ? entry->GetTypeObject()->GetCodingSchemeDesignator()
                                   : "")
                     + "^"
                     + std::string(entry->GetTypeObject() && entry->GetTypeObject()->GetCodeValue()
                                     ? entry->GetTypeObject()->GetCodeValue()
                                     : "")
                     + "^"
                     + std::string(entry->GetTypeObject() && entry->GetTypeObject()->GetCodeMeaning()
                                     ? entry->GetTypeObject()->GetCodeMeaning()
                                     : "")
                     + "~";
  serializedEntry +=
    std::string(entry->GetTypeModifierObject() && entry->GetTypeModifierObject()->GetCodingSchemeDesignator()
                  ? entry->GetTypeModifierObject()->GetCodingSchemeDesignator()
                  : "")
    + "^"
    + std::string(entry->GetTypeModifierObject() && entry->GetTypeModifierObject()->GetCodeValue()
                    ? entry->GetTypeModifierObject()->GetCodeValue()
                    : "")
    + "^"
    + std::string(entry->GetTypeModifierObject() && entry->GetTypeModifierObject()->GetCodeMeaning()
                    ? entry->GetTypeModifierObject()->GetCodeMeaning()
                    : "")
    + "~";

  serializedEntry += std::string(entry->GetRegionContextName() ? entry->GetRegionContextName() : "") + "~";
  serializedEntry += std::string(entry->GetRegionObject() && entry->GetRegionObject()->GetCodingSchemeDesignator()
                                   ? entry->GetRegionObject()->GetCodingSchemeDesignator()
                                   : "")
                     + "^"
                     + std::string(entry->GetRegionObject() && entry->GetRegionObject()->GetCodeValue()
                                     ? entry->GetRegionObject()->GetCodeValue()
                                     : "")
                     + "^"
                     + std::string(entry->GetRegionObject() && entry->GetRegionObject()->GetCodeMeaning()
                                     ? entry->GetRegionObject()->GetCodeMeaning()
                                     : "")
                     + "~";
  serializedEntry +=
    std::string(entry->GetRegionModifierObject() && entry->GetRegionModifierObject()->GetCodingSchemeDesignator()
                  ? entry->GetRegionModifierObject()->GetCodingSchemeDesignator()
                  : "")
    + "^"
    + std::string(entry->GetRegionModifierObject() && entry->GetRegionModifierObject()->GetCodeValue()
                    ? entry->GetRegionModifierObject()->GetCodeValue()
                    : "")
    + "^"
    + std::string(entry->GetRegionModifierObject() && entry->GetRegionModifierObject()->GetCodeMeaning()
                    ? entry->GetRegionModifierObject()->GetCodeMeaning()
                    : "");

  return serializedEntry;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(std::string terminologyContextName,
                                                                         std::string categoryValue,
                                                                         std::string categorySchemeDesignator,
                                                                         std::string categoryMeaning,
                                                                         std::string typeValue,
                                                                         std::string typeSchemeDesignator,
                                                                         std::string typeMeaning,
                                                                         std::string modifierValue,
                                                                         std::string modifierSchemeDesignator,
                                                                         std::string modifierMeaning,
                                                                         std::string regionContextName,
                                                                         std::string regionValue,
                                                                         std::string regionSchemeDesignator,
                                                                         std::string regionMeaning,
                                                                         std::string regionModifierValue,
                                                                         std::string regionModifierSchemeDesignator,
                                                                         std::string regionModifierMeaning)
{
  std::string serializedEntry("");
  serializedEntry += terminologyContextName + "~";
  serializedEntry += categorySchemeDesignator + "^" + categoryValue + "^" + categoryMeaning + "~";
  serializedEntry += typeSchemeDesignator + "^" + typeValue + "^" + typeMeaning + "~";
  serializedEntry += modifierSchemeDesignator + "^" + modifierValue + "^" + modifierMeaning + "~";

  serializedEntry += regionContextName + "~";
  serializedEntry += regionSchemeDesignator + "^" + regionValue + "^" + regionMeaning + "~";
  serializedEntry += regionModifierSchemeDesignator + "^" + regionModifierValue + "^" + regionModifierMeaning;

  return serializedEntry;
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::DeserializeTerminologyEntry(std::string serializedEntry,
                                                                    vtkSlicerTerminologyEntry* entry)
{
  // Note: This implementation could be consolidated with vtkMRMLColorNode::SetTerminologyFromString(int ind,
  // std::string terminologyString)
  if (!entry)
  {
    vtkGenericWarningMacro("vtkSlicerTerminologiesModuleLogic::DeserializeTerminologyEntry: Invalid terminology given");
    return false;
  }

  // Clear terminology entry object
  entry->SetTerminologyContextName(nullptr);
  entry->SetRegionContextName(nullptr);

  if (!entry->GetCategoryObject() || !entry->GetTypeObject() || !entry->GetTypeModifierObject()
      || !entry->GetRegionObject() || !entry->GetRegionModifierObject())
  {
    vtkErrorWithObjectMacro(entry, "DeserializeTerminologyEntry: Invalid terminology entry given");
    return false;
  }

  // Serialized terminology entry consists of the following: terminologyContextName, category (codingScheme,
  // codeValue, codeMeaning triple), type, typeModifier, regionContextName, region, regionModifier
  std::vector<std::string> entryComponents;
  vtksys::SystemTools::Split(serializedEntry, entryComponents, '~');
  if (entryComponents.size() != 7)
  {
    return false;
  }
  if (!entryComponents[1].compare("^^"))
  {
    // Empty category (none selection)
    return false;
  }

  std::string terminologyName(entryComponents[0]);
  entry->SetTerminologyContextName(terminologyName.empty() ? nullptr : terminologyName.c_str());

  // Category
  std::vector<std::string> categoryIds = vtksys::SystemTools::SplitString(entryComponents[1], '^');
  if (categoryIds.size() != 3)
  {
    vtkErrorWithObjectMacro(entry, "DeserializeTerminologyEntry: Invalid category component");
    return false;
  }
  vtkSlicerTerminologiesModuleLogic::CodeIdentifier categoryId(categoryIds[0], categoryIds[1], categoryIds[2]);
  entry->GetCategoryObject()->SetCodingSchemeDesignator(categoryIds[0].c_str());
  entry->GetCategoryObject()->SetCodeValue(categoryIds[1].c_str());
  entry->GetCategoryObject()->SetCodeMeaning(categoryIds[2].c_str());

  // Type
  std::vector<std::string> typeIds = vtksys::SystemTools::SplitString(entryComponents[2], '^');

  if (typeIds.size() != 3)
  {
    vtkErrorWithObjectMacro(entry, "DeserializeTerminologyEntry: Invalid type component");
    return false;
  }
  entry->GetTypeObject()->SetCodingSchemeDesignator(typeIds[0].c_str());
  entry->GetTypeObject()->SetCodeValue(typeIds[1].c_str());
  entry->GetTypeObject()->SetCodeMeaning(typeIds[2].c_str());

  // Type modifier (optional)
  std::vector<std::string> typeModifierIds = vtksys::SystemTools::SplitString(entryComponents[3], '^');
  entry->GetTypeModifierObject()->Initialize();
  if (typeModifierIds.size() == 3)
  {
    entry->GetTypeModifierObject()->SetCodingSchemeDesignator(typeModifierIds[0].c_str());
    entry->GetTypeModifierObject()->SetCodeValue(typeModifierIds[1].c_str());
    entry->GetTypeModifierObject()->SetCodeMeaning(typeModifierIds[2].c_str());
  }

  // Region context name (optional)
  std::string regionContextName = entryComponents[4];
  entry->SetRegionContextName(regionContextName.empty() ? nullptr : regionContextName.c_str());

  // Region (optional)
  std::vector<std::string> regionIds = vtksys::SystemTools::SplitString(entryComponents[5], '^');
  entry->GetRegionObject()->Initialize();
  if (regionIds.size() == 3)
  {
    entry->GetRegionObject()->SetCodingSchemeDesignator(regionIds[0].c_str());
    entry->GetRegionObject()->SetCodeValue(regionIds[1].c_str());
    entry->GetRegionObject()->SetCodeMeaning(regionIds[2].c_str());

    // Region modifier (optional)
    std::vector<std::string> regionModifierIds = vtksys::SystemTools::SplitString(entryComponents[6], '^');
    entry->GetRegionModifierObject()->Initialize();
    if (regionModifierIds.size() == 3)
    {
      entry->GetRegionModifierObject()->SetCodingSchemeDesignator(regionModifierIds[0].c_str());
      entry->GetRegionModifierObject()->SetCodeValue(regionModifierIds[1].c_str());
      entry->GetRegionModifierObject()->SetCodeMeaning(regionModifierIds[2].c_str());
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::UpdateEntryFromLoadedTerminologies(
  vtkSlicerTerminologyEntry* entry,
  std::vector<std::string> preferredTerminologyNames,
  std::vector<std::string> preferredRegionContextNames)
{
  if (!entry)
  {
    return false;
  }

  if (!entry->GetCategoryObject() || !entry->GetTypeObject() || !entry->GetTypeModifierObject()
      || !entry->GetRegionObject() || !entry->GetRegionModifierObject())
  {
    vtkErrorWithObjectMacro(entry, "UpdateEntryFromLoadedTerminologies: Invalid terminology entry given");
    return false;
  }

  // Category, type, type modifier

  CodeIdentifier categoryId = GetCodeIdentifierFromCodedEntry(entry->GetCategoryObject());
  CodeIdentifier typeId = GetCodeIdentifierFromCodedEntry(entry->GetTypeObject());
  CodeIdentifier typeModifierId = GetCodeIdentifierFromCodedEntry(entry->GetTypeModifierObject());
  if (categoryId.IsValid() && typeId.IsValid())
  {
    // Create list of preferred terminology names: the list starts with the entry's terminologyName
    // followed by all the other loaded terminologies.
    std::string terminologyName = (entry->GetTerminologyContextName() ? entry->GetTerminologyContextName() : "");
    if (preferredTerminologyNames.empty())
    {
      this->GetLoadedTerminologyNames(preferredTerminologyNames);
    }
    std::vector<std::string>::iterator ptnIt =
      std::find(preferredTerminologyNames.begin(), preferredTerminologyNames.end(), terminologyName);
    if (ptnIt != preferredTerminologyNames.end())
    {
      preferredTerminologyNames.erase(ptnIt);
      preferredTerminologyNames.insert(preferredTerminologyNames.begin(), terminologyName);
    }

    // Look for the type in each terminology context
    for (std::string terminologyName : preferredTerminologyNames)
    {
      vtkNew<vtkSlicerTerminologyCategory> categoryObject;
      if (!this->GetCategoryInTerminology(terminologyName, categoryId, categoryObject))
      {
        continue;
      }
      vtkNew<vtkSlicerTerminologyType> typeObject;
      if (!this->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId, typeObject))
      {
        continue;
      }
      bool found = false;
      if (!typeModifierId.IsValid())
      {
        // Type without a modifier
        found = true;
      }
      else
      {
        // Type with a modifier
        vtkNew<vtkSlicerTerminologyType> typeModifierObject;
        if (this->GetTypeModifierInTerminologyType(
              terminologyName, categoryId, typeId, typeModifierId, typeModifierObject))
        {
          found = true;
          entry->GetTypeModifierObject()->Copy(typeModifierObject);
        }
      }
      if (found)
      {
        entry->SetTerminologyContextName(terminologyName.c_str());
        entry->GetCategoryObject()->Copy(categoryObject);
        entry->GetTypeObject()->Copy(typeObject);
        break;
      }
    }
  }

  // Region, region modifier

  CodeIdentifier regionId = GetCodeIdentifierFromCodedEntry(entry->GetRegionObject());
  if (regionId.IsValid())
  {
    // Create list of preferred region context names: the list starts with the entry's region context name
    // followed by all the other loaded region context names.
    std::string regionContextName = (entry->GetRegionContextName() ? entry->GetRegionContextName() : "");
    if (preferredRegionContextNames.empty())
    {
      this->GetLoadedRegionContextNames(preferredRegionContextNames);
    }
    std::vector<std::string>::iterator pacIt =
      std::find(preferredRegionContextNames.begin(), preferredRegionContextNames.end(), regionContextName);
    if (pacIt != preferredRegionContextNames.end())
    {
      preferredRegionContextNames.erase(pacIt);
      preferredRegionContextNames.insert(preferredRegionContextNames.begin(), regionContextName);
    }

    // Look for the type in each terminology context
    for (std::string regionContextName : preferredRegionContextNames)
    {
      vtkNew<vtkSlicerTerminologyType> regionObject;
      if (!this->GetRegionInRegionContext(regionContextName, regionId, regionObject))
      {
        continue;
      }
      bool found = false;
      CodeIdentifier regionModifierId = GetCodeIdentifierFromCodedEntry(entry->GetRegionModifierObject());
      if (!regionModifierId.IsValid())
      {
        // Region without a modifier
        found = true;
      }
      else
      {
        // Region with a modifier
        vtkNew<vtkSlicerTerminologyType> regionModifierObject;
        if (this->GetRegionModifierInRegion(regionContextName, regionId, regionModifierId, regionModifierObject))
        {
          found = true;
          entry->GetRegionModifierObject()->Copy(regionModifierObject);
        }
      }
      if (found)
      {
        entry->SetRegionContextName(regionContextName.c_str());
        entry->GetRegionObject()->Copy(regionObject);
        break;
      }
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::GetInfoStringFromTerminologyEntry(vtkSlicerTerminologyEntry* entry)
{
  if (!entry)
  {
    return "Invalid terminology";
  }
  if (!entry->GetTerminologyContextName() || !entry->GetCategoryObject() || !entry->GetCategoryObject()->GetCodeValue())
  {
    return "No terminology information";
  }

  std::string terminologyStr("Terminology:");
  terminologyStr = terminologyStr + std::string("\n  Context: ") + std::string(entry->GetTerminologyContextName());

  if (entry->GetCategoryObject() && entry->GetCategoryObject()->GetCodeValue())
  {
    terminologyStr =
      terminologyStr + std::string("\n  Category: ") + std::string(entry->GetCategoryObject()->GetCodeMeaning());
  }
  else
  {
    terminologyStr = terminologyStr + std::string("\n  Category: NONE");
  }
  if (entry->GetTypeObject() && entry->GetTypeObject()->GetCodeValue())
  {
    terminologyStr = terminologyStr + std::string("\n  Type: ") + std::string(entry->GetTypeObject()->GetCodeMeaning());
  }
  else
  {
    terminologyStr = terminologyStr + std::string("\n  Type: NONE");
  }
  if (entry->GetTypeModifierObject() && entry->GetTypeModifierObject()->GetCodeValue())
  {
    terminologyStr =
      terminologyStr + std::string("\n    Modifier: ") + std::string(entry->GetTypeModifierObject()->GetCodeMeaning());
  }

  // If region is not selected, then do not show region context name either
  if (entry->GetRegionContextName() && entry->GetRegionObject() && entry->GetRegionObject()->GetCodeValue())
  {
    terminologyStr = terminologyStr + std::string("\n  Region context: ") + std::string(entry->GetRegionContextName());
    terminologyStr =
      terminologyStr + std::string("\n  Region: ") + std::string(entry->GetRegionObject()->GetCodeMeaning());

    if (entry->GetRegionModifierObject() && entry->GetRegionModifierObject()->GetCodeValue())
    {
      terminologyStr = terminologyStr + std::string("\n    Modifier: ")
                       + std::string(entry->GetRegionModifierObject()->GetCodeMeaning());
    }
  }

  return terminologyStr;
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindTypeInTerminologyBy3dSlicerLabel(std::string terminologyName,
                                                                             std::string slicerLabel,
                                                                             vtkSlicerTerminologyEntry* entry)
{
  if (!entry)
  {
    vtkErrorMacro("FindTypeInTerminologyBy3dSlicerLabel: Invalid output terminology entry");
    return false;
  }
  if (slicerLabel.empty())
  {
    vtkErrorMacro("FindTypeInTerminologyBy3dSlicerLabel: Empty 3dSlicerLabel attribute to look for");
    return false;
  }

  rapidjson::Value& categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
  {
    vtkErrorMacro("FindTypeInTerminologyBy3dSlicerLabel: Failed to find terminology '" << terminologyName << "'");
    return false;
  }

  bool found = false;
  CodeIdentifier foundCategoryId;
  CodeIdentifier foundTypeId;
  CodeIdentifier foundTypeModifierId;

  // Traverse categories
  rapidjson::SizeType categoryIndex = 0;
  while (categoryIndex < categoryArray.Size())
  {
    rapidjson::Value& category = categoryArray[categoryIndex];
    if (category.IsObject())
    {
      rapidjson::Value& categoryName = category["CodeMeaning"];
      rapidjson::Value& categoryCodingSchemeDesignator = category["CodingSchemeDesignator"];
      rapidjson::Value& categoryCodeValue = category["CodeValue"];
      if (categoryName.IsString() && categoryCodingSchemeDesignator.IsString() && categoryCodeValue.IsString())
      {
        CodeIdentifier categoryId(
          categoryCodingSchemeDesignator.GetString(), categoryCodeValue.GetString(), categoryName.GetString());
        rapidjson::Value& typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
        if (typeArray.IsNull())
        {
          vtkErrorMacro("FindTypeInTerminologyBy3dSlicerLabel: Failed to find category '"
                        << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
          ++categoryIndex;
          continue;
        }

        // Traverse types
        rapidjson::SizeType typeIndex = 0;
        while (typeIndex < typeArray.Size())
        {
          rapidjson::Value& type = typeArray[typeIndex];
          if (type.IsObject())
          {
            rapidjson::Value& typeName = type["CodeMeaning"];
            rapidjson::Value& typeCodingSchemeDesignator = type["CodingSchemeDesignator"];
            rapidjson::Value& typeCodeValue = type["CodeValue"];
            if (typeName.IsString() && typeCodingSchemeDesignator.IsString() && typeCodeValue.IsString())
            {
              CodeIdentifier typeId(
                typeCodingSchemeDesignator.GetString(), typeCodeValue.GetString(), typeName.GetString());
              rapidjson::Value::MemberIterator currentSlicerLabelIt = type.FindMember("3dSlicerLabel");
              if (currentSlicerLabelIt != type.MemberEnd()
                  && !slicerLabel.compare(currentSlicerLabelIt->value.GetString()))
              {
                // 3dSlicerLabel found in type
                found = true;
                foundCategoryId = categoryId;
                foundTypeId = typeId;
                break;
              }

              rapidjson::Value& typeModifierArray =
                this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
              if (typeModifierArray.IsNull())
              {
                ++typeIndex;
                continue;
              }

              // Traverse type modifiers
              rapidjson::SizeType typeModifierIndex = 0;
              while (typeModifierIndex < typeModifierArray.Size())
              {
                rapidjson::Value& typeModifier = typeModifierArray[typeModifierIndex];
                if (typeModifier.IsObject())
                {
                  rapidjson::Value& typeModifierName = typeModifier["CodeMeaning"];
                  rapidjson::Value& typeModifierCodingSchemeDesignator = typeModifier["CodingSchemeDesignator"];
                  rapidjson::Value& typeModifierCodeValue = typeModifier["CodeValue"];
                  if (typeModifierName.IsString() && typeModifierCodingSchemeDesignator.IsString()
                      && typeModifierCodeValue.IsString())
                  {
                    CodeIdentifier typeModifierId(typeModifierCodingSchemeDesignator.GetString(),
                                                  typeModifierCodeValue.GetString(),
                                                  typeModifierName.GetString());
                    rapidjson::Value::MemberIterator currentSlicerLabelIt = typeModifier.FindMember("3dSlicerLabel");
                    if (currentSlicerLabelIt != typeModifier.MemberEnd()
                        && !slicerLabel.compare(currentSlicerLabelIt->value.GetString()))
                    {
                      // 3dSlicerLabel found in type modifier
                      found = true;
                      foundCategoryId = categoryId;
                      foundTypeId = typeId;
                      foundTypeModifierId = typeModifierId;
                      break;
                    }
                  }
                }
                ++typeModifierIndex;
              } // For all type modifiers

              if (found)
              {
                break;
              }
            }
            else
            {
              vtkErrorMacro("FindTypeInTerminologyBy3dSlicerLabel: Invalid type '"
                            << typeName.GetString() << "in category '" << categoryId.CodeMeaning << "' in terminology '"
                            << terminologyName << "'");
            }
          }
          ++typeIndex;
        } // For all types

        if (found)
        {
          break;
        }
      }
      else
      {
        vtkErrorMacro("FindTypeInTerminologyBy3dSlicerLabel: Invalid category '"
                      << categoryName.GetString() << "' in terminology '" << terminologyName << "'");
      }
    }
    ++categoryIndex;
  } // For all categories

  if (found)
  {
    entry->SetTerminologyContextName(terminologyName.c_str());

    vtkSmartPointer<vtkSlicerTerminologyCategory> category = vtkSmartPointer<vtkSlicerTerminologyCategory>::New();
    this->GetCategoryInTerminology(terminologyName, foundCategoryId, category);
    entry->GetCategoryObject()->Copy(category);

    vtkSmartPointer<vtkSlicerTerminologyType> type = vtkSmartPointer<vtkSlicerTerminologyType>::New();
    this->GetTypeInTerminologyCategory(terminologyName, foundCategoryId, foundTypeId, type);
    entry->GetTypeObject()->Copy(type);

    if (!foundTypeModifierId.CodeValue.empty())
    {
      vtkSmartPointer<vtkSlicerTerminologyType> typeModifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
      this->GetTypeModifierInTerminologyType(
        terminologyName, foundCategoryId, foundTypeId, foundTypeModifierId, typeModifier);
      entry->GetTypeModifierObject()->Copy(typeModifier);
    }

    return true;
  }

  return false;
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindFirstColorNodeOrTerminology(
  vtkSlicerTerminologyEntry* entry,
  std::vector<std::string> preferredTerminologyNames,
  std::string& foundTerminologyName,
  std::string& foundColorNodeID,
  int& foundColorIndex)
{
  if (!entry)
  {
    vtkErrorMacro("FindFirstColorNodeOrTerminology: Invalid terminology entry");
    return false;
  }
  if (!entry->GetTerminologyContextName()
      && (!entry->GetCategoryObject() || !entry->GetCategoryObject()->GetCodeValue()))
  {
    // neither context name nor category is specified, this is an empty terminology
    return false;
  }

  // Add current terminology or color node name to the beginning of the preferred list
  std::string currentTerminologyName = entry->GetTerminologyContextName() ? entry->GetTerminologyContextName() : "";
  if (!currentTerminologyName.empty())
  {
    auto foundCurrentTerminologyNameIt =
      std::find(preferredTerminologyNames.begin(), preferredTerminologyNames.end(), currentTerminologyName);
    if (foundCurrentTerminologyNameIt != preferredTerminologyNames.end())
    {
      // current terminology name is already in the preferred list, remove it so that we can insert it at the beginning
      preferredTerminologyNames.erase(foundCurrentTerminologyNameIt);
    }
    preferredTerminologyNames.insert(preferredTerminologyNames.begin(), currentTerminologyName);
  }

  std::string categoryScheme;
  std::string categoryValue;
  vtkSlicerTerminologyCategory* categoryObject = entry->GetCategoryObject();
  if (categoryObject && categoryObject->GetCodingSchemeDesignator() && categoryObject->GetCodeValue())
  {
    categoryScheme = categoryObject->GetCodingSchemeDesignator();
    categoryValue = categoryObject->GetCodeValue();
  }
  std::string typeScheme;
  std::string typeValue;
  vtkSlicerTerminologyType* typeObject = entry->GetTypeObject();
  if (typeObject && typeObject->GetCodingSchemeDesignator() && typeObject->GetCodeValue())
  {
    typeScheme = typeObject->GetCodingSchemeDesignator();
    typeValue = typeObject->GetCodeValue();
  }
  std::string typeModifierScheme;
  std::string typeModifierValue;
  vtkSlicerTerminologyType* typeModifierObject = entry->GetTypeModifierObject();
  if (typeModifierObject && typeModifierObject->GetCodingSchemeDesignator() && typeModifierObject->GetCodeValue())
  {
    typeModifierScheme = typeModifierObject->GetCodingSchemeDesignator();
    typeModifierValue = typeModifierObject->GetCodeValue();
  }
  vtkSlicerTerminologyType* regionObject = entry->GetRegionObject();
  std::string regionScheme;
  std::string regionValue;
  if (regionObject && regionObject->GetCodingSchemeDesignator() && regionObject->GetCodeValue())
  {
    regionScheme = regionObject->GetCodingSchemeDesignator();
    regionValue = regionObject->GetCodeValue();
  }
  vtkSlicerTerminologyType* regionModifierObject = entry->GetRegionModifierObject();
  std::string regionModifierScheme;
  std::string regionModifierValue;
  if (regionModifierObject && regionModifierObject->GetCodingSchemeDesignator() && regionModifierObject->GetCodeValue())
  {
    regionModifierScheme = regionModifierObject->GetCodingSchemeDesignator();
    regionModifierValue = regionModifierObject->GetCodeValue();
  }

  return this->FindFirstColorNodeOrTerminology(categoryScheme,
                                               categoryValue,
                                               typeScheme,
                                               typeValue,
                                               typeModifierScheme,
                                               typeModifierValue,
                                               regionScheme,
                                               regionValue,
                                               regionModifierScheme,
                                               regionModifierValue,
                                               preferredTerminologyNames,
                                               foundTerminologyName,
                                               foundColorNodeID,
                                               foundColorIndex);
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindFirstColorNodeOrTerminology(
  std::string categoryCodingSchemeDesignator,
  std::string categoryCodeValue,
  std::string typeCodingSchemeDesignator,
  std::string typeCodeValue,
  std::string typeModifierCodingSchemeDesignator,
  std::string typeModifierCodeValue,
  std::string regionCodingSchemeDesignator,
  std::string regionCodeValue,
  std::string regionModifierCodingSchemeDesignator,
  std::string regionModifierCodeValue,
  std::vector<std::string> preferredTerminologyNames,
  std::string& foundTerminologyName,
  std::string& foundColorNodeID,
  int& foundColorIndex)
{
  std::vector<std::string> foundColorTableNodeIds;
  vtkNew<vtkIntArray> foundColorIndices;
  vtkNew<vtkIntArray> foundPreferredColorNodeIndices;
  std::vector<std::string> foundTerminologyNames;
  vtkNew<vtkIntArray> foundTerminologyIndices;

  if (!categoryCodingSchemeDesignator.empty() && !categoryCodeValue.empty())
  {
    foundColorTableNodeIds = this->FindColorNodes(categoryCodingSchemeDesignator,
                                                  categoryCodeValue,
                                                  typeCodingSchemeDesignator,
                                                  typeCodeValue,
                                                  typeModifierCodingSchemeDesignator,
                                                  typeModifierCodeValue,
                                                  regionCodingSchemeDesignator,
                                                  regionCodeValue,
                                                  regionModifierCodingSchemeDesignator,
                                                  regionModifierCodeValue,
                                                  preferredTerminologyNames,
                                                  foundColorIndices,
                                                  foundPreferredColorNodeIndices);
    foundTerminologyNames = this->FindTerminologyNames(categoryCodingSchemeDesignator,
                                                       categoryCodeValue,
                                                       typeCodingSchemeDesignator,
                                                       typeCodeValue,
                                                       typeModifierCodingSchemeDesignator,
                                                       typeModifierCodeValue,
                                                       preferredTerminologyNames,
                                                       nullptr,
                                                       foundTerminologyIndices);

    if (!preferredTerminologyNames.empty() && (foundColorTableNodeIds.empty() && foundTerminologyNames.empty()))
    {
      // Preferred terminologies do not contain the item, try to get first terminology containing it among all loaded
      // contexts
      foundColorTableNodeIds = this->FindColorNodes(categoryCodingSchemeDesignator,
                                                    categoryCodeValue,
                                                    typeCodingSchemeDesignator,
                                                    typeCodeValue,
                                                    typeModifierCodingSchemeDesignator,
                                                    typeModifierCodeValue,
                                                    regionCodingSchemeDesignator,
                                                    regionCodeValue,
                                                    regionModifierCodingSchemeDesignator,
                                                    regionModifierCodeValue,
                                                    std::vector<std::string>(),
                                                    foundColorIndices,
                                                    foundPreferredColorNodeIndices);
      foundTerminologyNames = this->FindTerminologyNames(categoryCodingSchemeDesignator,
                                                         categoryCodeValue,
                                                         typeCodingSchemeDesignator,
                                                         typeCodeValue,
                                                         typeModifierCodingSchemeDesignator,
                                                         typeModifierCodeValue,
                                                         std::vector<std::string>(),
                                                         nullptr,
                                                         foundTerminologyIndices);
    }
  }

  if (!foundColorTableNodeIds.empty() && !foundTerminologyNames.empty())
  {
    // Both color node and terminology contain the item, select the one that is first in the preferred list
    if (foundPreferredColorNodeIndices->GetNumberOfValues() > 0 && foundTerminologyIndices->GetNumberOfValues() > 0)
    {
      if (foundPreferredColorNodeIndices->GetValue(0) < foundTerminologyIndices->GetValue(0))
      {
        foundTerminologyNames.clear();
      }
      else
      {
        foundColorTableNodeIds.clear();
      }
    }
  }

  if (!foundTerminologyNames.empty())
  {
    foundColorNodeID.clear();
    foundColorIndex = -1;
    foundTerminologyName = foundTerminologyNames.front();
    return true;
  }
  else if (!foundColorTableNodeIds.empty())
  {
    std::string firstColorNodeID = foundColorTableNodeIds.front();
    if (!this->GetMRMLScene())
    {
      vtkErrorMacro("FindFirstColorNodeOrTerminology failed: invalid scene");
      return false;
    }
    vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(this->GetMRMLScene()->GetNodeByID(firstColorNodeID));
    if (!colorNode)
    {
      vtkErrorMacro("FindFirstColorNodeOrTerminology failed: Failed to find color node by ID " << firstColorNodeID);
      return false;
    }
    foundColorNodeID = firstColorNodeID;
    foundColorIndex = (foundColorIndices->GetNumberOfValues() > 0) ? foundColorIndices->GetValue(0) : -1;
    foundTerminologyName.clear();
    return true;
  }
  else
  {
    return false;
  }
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerTerminologiesModuleLogic::FindTerminologyNames(
  std::string categoryCodingSchemeDesignator,
  std::string categoryCodeValue,
  std::string typeCodingSchemeDesignator,
  std::string typeCodeValue,
  std::string typeModifierCodingSchemeDesignator,
  std::string typeModifierCodeValue,
  std::vector<std::string> preferredTerminologyNames,
  vtkCollection* foundEntries /*=nullptr*/,
  vtkIntArray* foundPreferredTerminologyNameIndices /*=nullptr*/)
{
  if (foundEntries)
  {
    foundEntries->RemoveAllItems();
  }
  if (foundPreferredTerminologyNameIndices)
  {
    foundPreferredTerminologyNameIndices->Reset();
  }
  std::vector<std::string> foundTerminologyNames;
  if (categoryCodingSchemeDesignator.empty() || categoryCodeValue.empty())
  {
    vtkErrorMacro("FindTerminologyEntries: Category is not specified");
    return foundTerminologyNames;
  }
  CodeIdentifier categoryId(categoryCodingSchemeDesignator, categoryCodeValue);

  if (typeCodingSchemeDesignator.empty() || typeCodeValue.empty())
  {
    vtkErrorMacro("FindTerminologyEntries: Type is not specified");
    return foundTerminologyNames;
  }
  CodeIdentifier typeId(typeCodingSchemeDesignator, typeCodeValue);

  if (preferredTerminologyNames.empty())
  {
    // Terminology names are not specified, so search in all available terminologies
    this->GetLoadedTerminologyNames(preferredTerminologyNames);
  }

  // Find terminology entries in each preferred terminology
  int preferredTerminologyNameIndex = 0;
  for (int preferredTerminologyNameIndex = 0; preferredTerminologyNameIndex < preferredTerminologyNames.size();
       ++preferredTerminologyNameIndex)
  {
    std::string terminologyName = preferredTerminologyNames[preferredTerminologyNameIndex];
    if (!this->IsTerminologyContextLoaded(terminologyName))
    {
      continue; // It is possible that some preferred terminologies are not loaded in this session
    }
    vtkNew<vtkSlicerTerminologyType> typeObject;
    if (!this->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId, typeObject))
    {
      continue;
    }
    if (typeModifierCodingSchemeDesignator.empty() && typeModifierCodeValue.empty())
    {
      // Type without modifier
      foundTerminologyNames.push_back(terminologyName);
      if (foundEntries)
      {
        foundEntries->AddItem(typeObject);
      }
      if (foundPreferredTerminologyNameIndices)
      {
        foundPreferredTerminologyNameIndices->InsertNextValue(preferredTerminologyNameIndex);
      }
    }
    else
    {
      // Type with a modifier
      vtkNew<vtkSlicerTerminologyType> modifiedTypeObject;
      if (this->GetTypeModifierInTerminologyType(
            terminologyName,
            categoryId,
            typeId,
            CodeIdentifier(typeModifierCodingSchemeDesignator, typeModifierCodeValue),
            modifiedTypeObject))
      {
        foundTerminologyNames.push_back(terminologyName);
        if (foundEntries)
        {
          foundEntries->AddItem(typeObject);
        }
        if (foundPreferredTerminologyNameIndices)
        {
          foundPreferredTerminologyNameIndices->InsertNextValue(preferredTerminologyNameIndex);
        }
      }
    }
  }

  return foundTerminologyNames;
}

//-----------------------------------------------------------------------------
std::vector<std::string> vtkSlicerTerminologiesModuleLogic::FindRegionContextNames(
  std::string regionCodingSchemeDesignator,
  std::string regionCodeValue,
  std::string regionModifierCodingSchemeDesignator,
  std::string regionModifierCodeValue,
  std::vector<std::string> preferredRegionContextNames,
  vtkCollection* foundEntries /*=nullptr*/)
{
  std::vector<std::string> foundRegionContextNames;
  if (regionCodingSchemeDesignator.empty() || regionCodeValue.empty())
  {
    vtkErrorMacro("FindRegionContextNames: region is not specified");
    return foundRegionContextNames;
  }
  CodeIdentifier regionId(regionCodingSchemeDesignator, regionCodeValue);

  if (preferredRegionContextNames.empty())
  {
    // Region context names are not specified, so search in all available terminologies
    this->GetLoadedRegionContextNames(preferredRegionContextNames);
  }

  // Find terminology entries in each preferred region context
  for (std::string regionContextName : preferredRegionContextNames)
  {
    vtkNew<vtkSlicerTerminologyType> regionObject;
    if (!this->GetRegionInRegionContext(regionContextName, regionId, regionObject))
    {
      continue;
    }
    if (regionModifierCodingSchemeDesignator.empty() && regionModifierCodeValue.empty())
    {
      // Region without modifier
      foundRegionContextNames.push_back(regionContextName);
      if (foundEntries)
      {
        foundEntries->AddItem(regionObject);
      }
    }
    else
    {
      // Region with a modifier
      vtkNew<vtkSlicerTerminologyType> modifiedRegionObject;
      if (this->GetRegionModifierInRegion(regionContextName,
                                          regionId,
                                          CodeIdentifier(regionModifierCodingSchemeDesignator, regionModifierCodeValue),
                                          modifiedRegionObject))
      {
        foundRegionContextNames.push_back(regionContextName);
        if (foundEntries)
        {
          foundEntries->AddItem(regionObject);
        }
      }
    }
  }

  return foundRegionContextNames;
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::AreSegmentTerminologyEntriesEqual(vtkSegment* segment1, vtkSegment* segment2)
{
  if (!segment1 || !segment2)
  {
    vtkErrorMacro("AreSegmentTerminologyEntriesEqual: Invalid segment");
    return false;
  }

  std::string terminologyEntry1 = segment1->GetTerminology();
  std::string terminologyEntry2 = segment2->GetTerminology();

  return this->AreTerminologyEntriesEqual(terminologyEntry1, terminologyEntry2);
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::AreTerminologyEntriesEqual(std::string terminologyEntry1,
                                                                   std::string terminologyEntry2)
{
  if (terminologyEntry1.empty() && terminologyEntry2.empty())
  {
    return true;
  }

  vtkNew<vtkSlicerTerminologyEntry> entry1;
  if (!this->DeserializeTerminologyEntry(terminologyEntry1, entry1))
  {
    if (!terminologyEntry1.empty())
    {
      vtkErrorMacro("AreTerminologyEntriesEqual: Failed to deserialize terminology entry");
    }
    return false;
  }

  vtkNew<vtkSlicerTerminologyEntry> entry2;
  if (!this->DeserializeTerminologyEntry(terminologyEntry2, entry2))
  {
    if (!terminologyEntry2.empty())
    {
      vtkErrorMacro("AreTerminologyEntriesEqual: Failed to deserialize terminology entry");
    }
    return false;
  }

  return this->AreTerminologyEntriesEqual(entry1, entry2);
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::AreTerminologyEntriesEqual(vtkSlicerTerminologyEntry* entry1,
                                                                   vtkSlicerTerminologyEntry* entry2)
{
  if (!entry1 || !entry2)
  {
    // Return true if both are nullptr, false if only one is nullptr
    return entry1 == entry2;
  }

  return this->AreCodedEntriesEqual(entry1->GetCategoryObject(), entry2->GetCategoryObject())
         && this->AreCodedEntriesEqual(entry1->GetTypeObject(), entry2->GetTypeObject())
         && this->AreCodedEntriesEqual(entry1->GetTypeModifierObject(), entry2->GetTypeModifierObject())
         && this->AreCodedEntriesEqual(entry1->GetRegionObject(), entry2->GetRegionObject())
         && this->AreCodedEntriesEqual(entry1->GetRegionModifierObject(), entry2->GetRegionModifierObject());
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::AreCodedEntriesEqual(vtkCodedEntry* codedEntry1, vtkCodedEntry* codedEntry2)
{
  return vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(codedEntry1, codedEntry2);
}

//-----------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(vtkCodedEntry* codedEntry1,
                                                                  vtkCodedEntry* codedEntry2)
{
  if (!codedEntry1 || !codedEntry2)
  {
    // Return true if both are nullptr, false if only one is nullptr
    return codedEntry1 == codedEntry2;
  }

  const char* schemeDesignator1 = codedEntry1->GetCodingSchemeDesignator();
  bool schemeDesignator1Empty = (schemeDesignator1 == nullptr || strlen(schemeDesignator1) == 0);
  const char* schemeDesignator2 = codedEntry2->GetCodingSchemeDesignator();
  bool schemeDesignator2Empty = (schemeDesignator2 == nullptr || strlen(schemeDesignator2) == 0);
  if (!schemeDesignator1Empty && !schemeDesignator2Empty)
  {
    if (strcmp(schemeDesignator1, schemeDesignator2) != 0)
    {
      return false;
    }
  }
  else if (!schemeDesignator1Empty || !schemeDesignator2Empty)
  {
    // One is nullptr/empty, the other is not
    return false;
  }

  const char* codeValue1 = codedEntry1->GetCodeValue();
  bool codeValue1Empty = (codeValue1 == nullptr || strlen(codeValue1) == 0);
  const char* codeValue2 = codedEntry2->GetCodeValue();
  bool codeValue2Empty = (codeValue2 == nullptr || strlen(codeValue2) == 0);
  if (!codeValue1Empty && !codeValue2Empty)
  {
    if (strcmp(codeValue1, codeValue2) != 0)
    {
      return false;
    }
  }
  else if (!codeValue1Empty || !codeValue2Empty)
  {
    // One is nullptr/empty, the other is not
    return false;
  }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::IsTerminologyContextLoaded(std::string terminologyName)
{
  if (terminologyName.empty())
  {
    return false;
  }
  rapidjson::Value& root = this->Internal->GetTerminologyRootByName(terminologyName);
  return !root.IsNull();
}

//---------------------------------------------------------------------------
int vtkSlicerTerminologiesModuleLogic::GetColorIndexByTerminology(vtkMRMLColorNode* colorNode,
                                                                  const std::string& terminology,
                                                                  bool ignoreContextName /* =true */)
{
  if (colorNode == nullptr)
  {
    vtkGenericWarningMacro(
      "vtkSlicerTerminologiesModuleLogic::GetColorIndexByTerminology: need a valid colorNode as argument");
    return -1;
  }

  vtkNew<vtkSlicerTerminologyEntry> entry1;
  if (!vtkSlicerTerminologiesModuleLogic::DeserializeTerminologyEntry(terminology, entry1))
  {
    return -1;
  }

  for (int colorIdx = 0; colorIdx < colorNode->GetNumberOfColors(); ++colorIdx)
  {
    if (!colorNode->GetColorDefined(colorIdx))
    {
      continue;
    }
    std::string currentTerminologyStr = colorNode->GetTerminologyAsString(colorIdx);
    if (currentTerminologyStr.empty())
    {
      continue;
    }
    vtkNew<vtkSlicerTerminologyEntry> entry2;
    if (!vtkSlicerTerminologiesModuleLogic::DeserializeTerminologyEntry(currentTerminologyStr, entry2))
    {
      continue;
    }

    if (vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(entry1->GetCategoryObject(),
                                                                     entry2->GetCategoryObject())
        && vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(entry1->GetTypeObject(),
                                                                        entry2->GetTypeObject())
        && vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(entry1->GetTypeModifierObject(),
                                                                        entry2->GetTypeModifierObject())
        && vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(entry1->GetRegionObject(),
                                                                        entry2->GetRegionObject())
        && vtkSlicerTerminologiesModuleLogic::AreCodedEntriesExactMatch(entry1->GetRegionModifierObject(),
                                                                        entry2->GetRegionModifierObject()))
    {
      // found a match
      if (!ignoreContextName)
      {
        // need to check context name as well
        if (strcmp(entry1->GetTerminologyContextName(), entry2->GetTerminologyContextName()) != 0)
        {
          // Terminology context name does not match
          continue;
        }
        if (entry1->GetRegionObject() && strcmp(entry1->GetRegionContextName(), entry2->GetRegionContextName()) != 0)
        {
          // Region context name does not match
          continue;
        }
      }
      return colorIdx;
    }
  }

  return -1; // Not found
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkSlicerTerminologiesModuleLogic::GetCompatibleColorNodeIDs()
{
  std::vector<std::string> compatibleColorNodeIDs;
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetCompatibleColorNodeIDs: Invalid MRML scene");
    return compatibleColorNodeIDs;
  }
  std::vector<vtkMRMLNode*> colorNodes;
  scene->GetNodesByClass("vtkMRMLColorNode", colorNodes);
  for (vtkMRMLNode* node : colorNodes)
  {
    vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(node);
    if (colorNode && colorNode->GetContainsTerminology())
    {
      compatibleColorNodeIDs.push_back(colorNode->GetID());
    }
  }
  return compatibleColorNodeIDs;
}

//---------------------------------------------------------------------------
vtkMRMLColorNode* vtkSlicerTerminologiesModuleLogic::GetFirstCompatibleColorNodeByName(std::string name)
{
  std::vector<std::string> compatibleColorNodeIDs;
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("GetFirstCompatibleColorNodeByName: Invalid MRML scene");
    return nullptr;
  }
  std::vector<vtkMRMLNode*> colorNodes;
  scene->GetNodesByClass("vtkMRMLColorNode", colorNodes);
  for (vtkMRMLNode* node : colorNodes)
  {
    vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(node);
    if (colorNode && colorNode->GetContainsTerminology())
    {
      if (colorNode->GetName())
      {
        if (name == colorNode->GetName())
        {
          // found color node that has matching name
          return colorNode;
        }
      }
    }
  }
  return nullptr;
}

//---------------------------------------------------------------------------
std::vector<std::string> vtkSlicerTerminologiesModuleLogic::FindColorNodes(
  std::string categoryCodingSchemeDesignator,
  std::string categoryCodeValue,
  std::string typeCodingSchemeDesignator,
  std::string typeCodeValue,
  std::string typeModifierCodingSchemeDesignator,
  std::string typeModifierCodeValue,
  std::string regionCodingSchemeDesignator,
  std::string regionCodeValue,
  std::string regionModifierCodingSchemeDesignator,
  std::string regionModifierCodeValue,
  std::vector<std::string> preferredColorNodeNames,
  vtkIntArray* foundColorIndices /*=nullptr*/,
  vtkIntArray* foundPreferredColorNodeIndices /*=nullptr*/
)
{
  if (foundColorIndices)
  {
    foundColorIndices->Initialize();
  }
  if (foundPreferredColorNodeIndices)
  {
    foundPreferredColorNodeIndices->Initialize();
  }
  std::vector<std::string> foundColorNodeIDs;
  // Find candidate color nodes
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
  {
    vtkErrorMacro("FindColorTableNodes: Invalid MRML scene");
    return foundColorNodeIDs;
  }
  std::vector<std::string> compatibleColorNodeIDs;
  if (preferredColorNodeNames.empty())
  {
    compatibleColorNodeIDs = vtkSlicerTerminologiesModuleLogic::GetCompatibleColorNodeIDs();
  }
  else
  {
    for (const std::string& preferredColorNodeName : preferredColorNodeNames)
    {
      vtkSmartPointer<vtkCollection> preferredColorNodeCandidates = vtkSmartPointer<vtkCollection>::Take(
        scene->GetNodesByClassByName("vtkMRMLColorNode", preferredColorNodeName.c_str()));
      for (int i = 0; i < preferredColorNodeCandidates->GetNumberOfItems(); ++i)
      {
        vtkMRMLColorNode* colorNode = vtkMRMLColorNode::SafeDownCast(preferredColorNodeCandidates->GetItemAsObject(i));
        if (colorNode && colorNode->GetContainsTerminology())
        {
          compatibleColorNodeIDs.push_back(colorNode->GetID());
        }
      }
    }
  }
  // Check if we can find the item in the table
  std::string terminologyStr = vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(
    "", // terminologyContextName: we don't know it, so we set it to empty by default
    categoryCodeValue,
    categoryCodingSchemeDesignator,
    "",
    typeCodeValue,
    typeCodingSchemeDesignator,
    "",
    typeModifierCodeValue,
    typeModifierCodingSchemeDesignator,
    "",
    "", // regionContextName: we don't know it, so we set it to empty by default
    regionCodeValue,
    regionCodingSchemeDesignator,
    "",
    regionModifierCodeValue,
    regionModifierCodingSchemeDesignator,
    "");
  for (std::string& compatibleColorNodeID : compatibleColorNodeIDs)
  {
    vtkMRMLColorNode* compatibleColorNode = vtkMRMLColorNode::SafeDownCast(scene->GetNodeByID(compatibleColorNodeID));
    if (!compatibleColorNode)
    {
      continue;
    }
    int indexInColorTable = vtkSlicerTerminologiesModuleLogic::GetColorIndexByTerminology(
      compatibleColorNode, terminologyStr, /* ignoreContextName= */ true);
    if (indexInColorTable > -1)
    {
      foundColorNodeIDs.push_back(compatibleColorNodeID);
      if (foundColorIndices)
      {
        foundColorIndices->InsertNextValue(indexInColorTable);
      }
      if (foundPreferredColorNodeIndices)
      {
        std::string colorNodeName = (compatibleColorNode->GetName() ? compatibleColorNode->GetName() : "");
        auto foundPreferredColorNodeIDIt =
          std::find(preferredColorNodeNames.begin(), preferredColorNodeNames.end(), colorNodeName);
        if (foundPreferredColorNodeIDIt != preferredColorNodeNames.end())
        {
          foundPreferredColorNodeIndices->InsertNextValue(
            std::distance(preferredColorNodeNames.begin(), foundPreferredColorNodeIDIt));
        }
        else
        {
          foundPreferredColorNodeIndices->InsertNextValue(-1);
        }
      }
    }
  }
  return foundColorNodeIDs;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTerminologyEntry(vtkMRMLNode* node, vtkSlicerTerminologyEntry* entry)
{
  if (!node)
  {
    vtkGenericWarningMacro("GetTerminologyEntry: Invalid node");
    return false;
  }
  if (!entry)
  {
    vtkGenericWarningMacro("GetTerminologyEntry: Invalid output entry");
    return false;
  }
  const char* terminology = node->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAttributeName());
  if (!terminology)
  {
    return false;
  }
  return vtkSlicerTerminologiesModuleLogic::DeserializeTerminologyEntry(terminology, entry);
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAsString(vtkMRMLNode* node)
{
  if (!node)
  {
    vtkGenericWarningMacro("GetTerminologyEntryAsString: Invalid node");
    return std::string();
  }
  const char* terminology = node->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAttributeName());
  return terminology ? terminology : "";
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetTerminologyEntryAsString(vtkMRMLNode* node, std::string entryStr)
{
  if (!node)
  {
    vtkGenericWarningMacro("SetTerminologyEntryAsString: Invalid node");
    return;
  }
  if (entryStr.empty())
  {
    node->RemoveAttribute(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAttributeName());
  }
  else
  {
    node->SetAttribute(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAttributeName(), entryStr.c_str());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetTerminologyEntry(vtkMRMLNode* node, vtkSlicerTerminologyEntry* entry)
{
  if (!node)
  {
    vtkGenericWarningMacro("SetTerminologyEntry: Invalid node");
    return;
  }
  std::string terminologyStr;
  if (entry)
  {
    terminologyStr = vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(entry);
  }
  if (terminologyStr.empty())
  {
    node->RemoveAttribute(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAttributeName());
  }
  else
  {
    node->SetAttribute(vtkSlicerTerminologiesModuleLogic::GetTerminologyEntryAttributeName(), terminologyStr.c_str());
  }
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntry(vtkMRMLNode* node, vtkSlicerTerminologyEntry* entry)
{
  if (!node)
  {
    vtkGenericWarningMacro("GetDefaultTerminologyEntry: Invalid node");
    return false;
  }
  if (!entry)
  {
    vtkGenericWarningMacro("GetDefaultTerminologyEntry: Invalid output entry");
    return false;
  }
  const char* terminology =
    node->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAttributeName());
  if (!terminology)
  {
    return false;
  }
  return vtkSlicerTerminologiesModuleLogic::DeserializeTerminologyEntry(terminology, entry);
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAsString(vtkMRMLNode* node)
{
  if (!node)
  {
    vtkGenericWarningMacro("GetDefaultTerminologyEntryAsString: Invalid node");
    return std::string();
  }
  const char* terminology =
    node->GetAttribute(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAttributeName());
  return terminology ? terminology : "";
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetDefaultTerminologyEntryAsString(vtkMRMLNode* node, std::string entryStr)
{
  if (!node)
  {
    vtkGenericWarningMacro("SetDefaultTerminologyEntryAsString: Invalid node");
    return;
  }
  if (entryStr.empty())
  {
    node->RemoveAttribute(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAttributeName());
  }
  else
  {
    node->SetAttribute(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAttributeName(), entryStr.c_str());
  }
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::SetDefaultTerminologyEntry(vtkMRMLNode* node, vtkSlicerTerminologyEntry* entry)
{
  if (!node)
  {
    vtkGenericWarningMacro("SetDefaultTerminologyEntry: Invalid node");
    return;
  }
  std::string terminologyStr;
  if (entry)
  {
    terminologyStr = vtkSlicerTerminologiesModuleLogic::SerializeTerminologyEntry(entry);
  }
  if (terminologyStr.empty())
  {
    node->RemoveAttribute(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAttributeName());
  }
  else
  {
    node->SetAttribute(vtkSlicerTerminologiesModuleLogic::GetDefaultTerminologyEntryAttributeName(),
                       terminologyStr.c_str());
  }
}

std::string vtkSlicerTerminologiesModuleLogic::LoadAnatomicContextFromFile(std::string filePath)
{
  vtkWarningMacro("LoadAnatomicContextFromFile is deprecated. Use LoadRegionContextFromFile instead.");
  return this->LoadRegionContextFromFile(filePath);
}

bool vtkSlicerTerminologiesModuleLogic::LoadAnatomicContextFromSegmentDescriptorFile(std::string contextName,
                                                                                     std::string filePath)
{
  vtkWarningMacro("LoadAnatomicContextFromSegmentDescriptorFile is deprecated. Use "
                  "LoadRegionContextFromSegmentDescriptorFile instead.");
  return this->LoadRegionContextFromSegmentDescriptorFile(contextName, filePath);
}

void vtkSlicerTerminologiesModuleLogic::GetLoadedAnatomicContextNames(std::vector<std::string>& anatomicContextNames)
{
  vtkWarningMacro("GetLoadedAnatomicContextNames is deprecated. Use GetLoadedRegionContextNames instead.");
  this->GetLoadedRegionContextNames(anatomicContextNames);
}

void vtkSlicerTerminologiesModuleLogic::GetLoadedAnatomicContextNames(vtkStringArray* anatomicContextNames)
{
  vtkWarningMacro("GetLoadedAnatomicContextNames is deprecated. Use GetLoadedRegionContextNames instead.");
  this->GetLoadedRegionContextNames(anatomicContextNames);
}

std::vector<std::string> vtkSlicerTerminologiesModuleLogic::FindAnatomicContextNames(
  std::string anatomicRegionCodingSchemeDesignator,
  std::string anatomicRegionCodeValue,
  std::string anatomicRegionModifierCodingSchemeDesignator,
  std::string anatomicRegionModifierCodeValue,
  std::vector<std::string> preferredAnatomicContextNames,
  vtkCollection* foundEntries)
{
  vtkWarningMacro("FindAnatomicContextNames is deprecated. Use FindRegionContextNames instead.");
  return this->FindRegionContextNames(anatomicRegionCodingSchemeDesignator,
                                      anatomicRegionCodeValue,
                                      anatomicRegionModifierCodingSchemeDesignator,
                                      anatomicRegionModifierCodeValue,
                                      preferredAnatomicContextNames,
                                      foundEntries);
}

bool vtkSlicerTerminologiesModuleLogic::GetRegionsInAnatomicContext(std::string anatomicContextName,
                                                                    std::vector<CodeIdentifier>& regions)
{
  vtkWarningMacro("GetRegionsInAnatomicContext is deprecated. Use GetRegionsInRegionContext instead.");
  return this->GetRegionsInRegionContext(anatomicContextName, regions);
}

int vtkSlicerTerminologiesModuleLogic::GetNumberOfRegionsInAnatomicContext(std::string anatomicContextName)
{
  vtkWarningMacro("GetNumberOfRegionsInAnatomicContext is deprecated. Use GetNumberOfRegionsInRegionContext instead.");
  return this->GetNumberOfRegionsInRegionContext(anatomicContextName);
}

bool vtkSlicerTerminologiesModuleLogic::GetNthRegionInAnatomicContext(std::string anatomicContextName,
                                                                      int regionIndex,
                                                                      vtkSlicerTerminologyType* regionObject)
{
  vtkWarningMacro("GetNthRegionInAnatomicContext is deprecated. Use GetNthRegionInRegionContext instead.");
  return this->GetNthRegionInRegionContext(anatomicContextName, regionIndex, regionObject);
}

bool vtkSlicerTerminologiesModuleLogic::FindRegionsInAnatomicContext(std::string anatomicContextName,
                                                                     std::vector<CodeIdentifier>& regions,
                                                                     std::string search)
{
  vtkWarningMacro("FindRegionsInAnatomicContext is deprecated. Use FindRegionsInRegionContext instead.");
  return this->FindRegionsInRegionContext(anatomicContextName, regions, search);
}

bool vtkSlicerTerminologiesModuleLogic::GetRegionInAnatomicContext(std::string anatomicContextName,
                                                                   CodeIdentifier regionId,
                                                                   vtkSlicerTerminologyType* regionObject)
{
  vtkWarningMacro("GetRegionInAnatomicContext is deprecated. Use GetRegionInRegionContext instead.");
  return this->GetRegionInRegionContext(anatomicContextName, regionId, regionObject);
}

bool vtkSlicerTerminologiesModuleLogic::GetRegionModifiersInAnatomicRegion(std::string anatomicContextName,
                                                                           CodeIdentifier regionId,
                                                                           std::vector<CodeIdentifier>& regionModifiers)
{
  vtkWarningMacro("GetRegionModifiersInAnatomicRegion is deprecated. Use GetRegionModifiersInRegion instead.");
  return this->GetRegionModifiersInRegion(anatomicContextName, regionId, regionModifiers);
}

int vtkSlicerTerminologiesModuleLogic::GetNumberOfRegionModifierInAnatomicRegion(std::string anatomicContextName,
                                                                                 vtkSlicerTerminologyType* regionObject)
{
  vtkWarningMacro(
    "GetNumberOfRegionModifierInAnatomicRegion is deprecated. Use GetNumberOfRegionModifierInRegion instead.");
  return this->GetNumberOfRegionModifierInRegion(anatomicContextName, regionObject);
}

bool vtkSlicerTerminologiesModuleLogic::GetNthRegionModifierInAnatomicRegion(std::string anatomicContextName,
                                                                             vtkSlicerTerminologyType* regionObject,
                                                                             int regionModifierIndex,
                                                                             vtkSlicerTerminologyType* regionModifier)
{
  vtkWarningMacro("GetNthRegionModifierInAnatomicRegion is deprecated. Use GetNthRegionModifierInRegion instead.");
  return this->GetNthRegionModifierInRegion(anatomicContextName, regionObject, regionModifierIndex, regionModifier);
}

vtkSlicerTerminologiesModuleLogic::CodeIdentifier
vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory(vtkSlicerTerminologyCategory* category)
{
  vtkGenericWarningMacro("vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyCategory is deprecated."
                         " Use GetCodeIdentifierFromCodedEntry instead.");
  return vtkSlicerTerminologiesModuleLogic::GetCodeIdentifierFromCodedEntry(category);
}

vtkSlicerTerminologiesModuleLogic::CodeIdentifier vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType(
  vtkSlicerTerminologyType* type)
{
  vtkGenericWarningMacro("vtkSlicerTerminologiesModuleLogic::CodeIdentifierFromTerminologyType is deprecated."
                         " Use GetCodeIdentifierFromCodedEntry instead.");
  return vtkSlicerTerminologiesModuleLogic::GetCodeIdentifierFromCodedEntry(type);
}
