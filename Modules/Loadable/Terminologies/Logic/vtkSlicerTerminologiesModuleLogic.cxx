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
#include <vtksys/SystemTools.hxx>
#include <vtkDirectory.h>

// STD includes
#include <algorithm>

#include "rapidjson/document.h"     // rapidjson's DOM-style API
#include "rapidjson/prettywriter.h" // for stringify JSON
#include "rapidjson/filereadstream.h"

static rapidjson::Value JSON_EMPTY_VALUE;

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerTerminologiesModuleLogic);

//---------------------------------------------------------------------------
class vtkSlicerTerminologiesModuleLogic::vtkInternal
{
public:
  // rapidjson document object cannot be stored in an STL map (there are build errors
  // on linux and mac), therefore we store a simple pointer and create/delete
  // the document object manually
  typedef std::map<std::string, rapidjson::Document* > TerminologyMap;
  vtkInternal();
  ~vtkInternal();

  /// Utility function to get code in Json array
  /// \param foundIndex Output parameter for index of found object in input array. -1 if not found
  /// \return Json object if found, otherwise null Json object
  rapidjson::Value& GetCodeInArray(CodeIdentifier codeId, rapidjson::Value& jsonArray, int &foundIndex);

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
  rapidjson::Value& GetTypeInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId);
  /// Populate \sa vtkSlicerTerminologyType from Json terminology
  bool PopulateTerminologyTypeFromJson(rapidjson::Value& typeObject, vtkSlicerTerminologyType* type);

  /// Get type modifier array Json value for a given terminology, category, and type
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetTypeModifierArrayInTerminologyType(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId);
  /// Get type modifier Json object from a terminology, category, and type with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetTypeModifierInTerminologyType(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, CodeIdentifier modifierId);

  /// Get root Json value for the anatomic context with given name
  rapidjson::Value& GetAnatomicContextRootByName(std::string anatomicContextName);

  /// Get region array Json value for a given anatomic context
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetRegionArrayInAnatomicContext(std::string anatomicContextName);
  /// Get type Json object from an anatomic context with given region name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetRegionInAnatomicContext(std::string anatomicContextName, CodeIdentifier regionId);
  /// Populate \sa vtkSlicerTerminologyType from Json anatomic region
  bool PopulateRegionFromJson(rapidjson::Value& anatomicRegionObject, vtkSlicerTerminologyType* region);

  /// Get region modifier array Json value for a given anatomic context and region
  /// \return Null Json value on failure, the array object otherwise
  rapidjson::Value& GetRegionModifierArrayInRegion(std::string anatomicContextName, CodeIdentifier regionId);
  /// Get type modifier Json object from an anatomic context and region with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  rapidjson::Value& GetRegionModifierInRegion(std::string anatomicContextName, CodeIdentifier regionId, CodeIdentifier modifierId);

  /// Convert a segmentation descriptor Json structure to a terminology context one
  /// \param descriptorDoc Input segmentation descriptor json document
  /// \param convertedDoc Output terminology context json document
  /// \return Success flag
  bool ConvertSegmentationDescriptorToTerminologyContext(rapidjson::Document& descriptorDoc, rapidjson::Document& convertedDoc, std::string contextName);
  /// Convert a segmentation descriptor Json structure to an anatomic context one
  /// \param descriptorDoc Input segmentation descriptor json document
  /// \param convertedDoc Output anatomic context json document
  /// \return Success flag
  bool ConvertSegmentationDescriptorToAnatomicContext(rapidjson::Document& descriptorDoc, rapidjson::Document& convertedDoc, std::string contextName);
  /// Copy basic identifier members from an identifier object into a Json object
  /// Note: Allocator specifies the owner of the created object, so it is important to set the allocator
  ///       of the document where the object will be added
  /// \param code Json object into which the code information is added a members
  void GetJsonCodeFromIdentifier(rapidjson::Value& code, CodeIdentifier idenfifier, rapidjson::Document::AllocatorType& allocator);

  /// Utility function for safe (memory-leak-free) setting of a document pointer in map
  static void SetDocumentInTerminologyMap(TerminologyMap& terminologyMap, const std::string& name, rapidjson::Document* doc)
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

  /// Loaded anatomical region contexts. Key is the context name, value is the root item.
  TerminologyMap LoadedAnatomicContexts;
};

//---------------------------------------------------------------------------
// vtkInternal methods

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkInternal::vtkInternal()
{
}

//---------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkInternal::~vtkInternal()
{
  for (TerminologyMap::iterator termIt = this->LoadedTerminologies.begin();
    termIt != this->LoadedTerminologies.end(); ++termIt)
    {
    delete termIt->second;
    }
  for (TerminologyMap::iterator anIt = this->LoadedAnatomicContexts.begin();
    anIt != this->LoadedAnatomicContexts.end(); ++anIt)
    {
    delete anIt->second;
    }
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCodeInArray(CodeIdentifier codeId, rapidjson::Value &jsonArray, int &foundIndex)
{
  if (!jsonArray.IsArray())
    {
    return JSON_EMPTY_VALUE;
    }

  // Traverse array and try to find the object with given identifier
  rapidjson::SizeType index = 0;
  while (index<jsonArray.Size())
    {
    rapidjson::Value& currentObject = jsonArray[index];
    if (currentObject.IsObject())
      {
      rapidjson::Value& codingSchemeDesignator = currentObject["CodingSchemeDesignator"];
      rapidjson::Value& codeValue = currentObject["CodeValue"];
      if ( codingSchemeDesignator.IsString() && !codeId.CodingSchemeDesignator.compare(codingSchemeDesignator.GetString())
        && codeValue.IsString() && !codeId.CodeValue.compare(codeValue.GetString()) )
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
  if (termIt != this->LoadedTerminologies.end() && termIt->second != NULL)
    {
    return *(termIt->second);
    }

  return JSON_EMPTY_VALUE;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryArrayInTerminology(std::string terminologyName)
{
  if (terminologyName.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& root = this->GetTerminologyRootByName(terminologyName);
  if (root.IsNull())
    {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find terminology root for context name '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  rapidjson::Value& segmentationCodes = root["SegmentationCodes"];
  if (segmentationCodes.IsNull())
    {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find SegmentationCodes member in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& categoryArray = segmentationCodes["Category"];
  if (!categoryArray.IsArray())
    {
    vtkGenericWarningMacro("GetCategoryArrayInTerminology: Failed to find Category array member in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  return categoryArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryInTerminology(std::string terminologyName, CodeIdentifier categoryId)
{
  if (categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& categoryArray = this->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
    {
    vtkGenericWarningMacro("GetCategoryInTerminology: Failed to find category array in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  int index = -1;
  return this->GetCodeInArray(categoryId, categoryArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyCategoryFromJson(
  rapidjson::Value& categoryObject, vtkSlicerTerminologyCategory* category)
{
  if (!categoryObject.IsObject() || !category)
    {
    return false;
    }

  rapidjson::Value::MemberIterator codeMeaning = categoryObject.FindMember("CodeMeaning");             // e.g. "Tissue" (mandatory)
  rapidjson::Value::MemberIterator codingScheme = categoryObject.FindMember("CodingSchemeDesignator"); // e.g. "SRT" (mandatory)
  rapidjson::Value::MemberIterator SNOMEDCTConceptID = categoryObject.FindMember("SNOMEDCTConceptID"); // e.g. "85756007"
  rapidjson::Value::MemberIterator UMLSConceptUID = categoryObject.FindMember("UMLSConceptUID");       // e.g. "C0040300"
  rapidjson::Value::MemberIterator cid = categoryObject.FindMember("cid");                             // e.g. "7051"
  rapidjson::Value::MemberIterator codeValue = categoryObject.FindMember("CodeValue");                 // e.g. "T-D0050" (mandatory)
  rapidjson::Value::MemberIterator contextGroupName = categoryObject.FindMember("contextGroupName");   // e.g. "Segmentation Property Categories"
  rapidjson::Value::MemberIterator showAnatomy = categoryObject.FindMember("showAnatomy");
  if (codingScheme == categoryObject.MemberEnd() || codeValue == categoryObject.MemberEnd() || codeMeaning == categoryObject.MemberEnd())
    {
    vtkGenericWarningMacro("PopulateTerminologyCategoryFromJson: Unable to access mandatory category member");
    return false;
    }

  category->SetCodeMeaning(codeMeaning->value.GetString());
  category->SetCodingScheme(codingScheme->value.GetString());
  category->SetSNOMEDCTConceptID(SNOMEDCTConceptID != categoryObject.MemberEnd() ? SNOMEDCTConceptID->value.GetString() : NULL);
  category->SetUMLSConceptUID(UMLSConceptUID != categoryObject.MemberEnd() ? UMLSConceptUID->value.GetString() : NULL);
  category->SetCid(cid != categoryObject.MemberEnd() ? cid->value.GetString() : NULL);
  category->SetCodeValue(codeValue->value.GetString());
  category->SetContextGroupName(contextGroupName != categoryObject.MemberEnd() ? contextGroupName->value.GetString() : NULL);
  if (showAnatomy == categoryObject.MemberEnd())
    {
    category->SetShowAnatomy(true); // Default
    }
  else
    {
    if (showAnatomy->value.IsString())
      {
      std::string showAnatomyStr = showAnatomy->value.GetString();
      std::transform(showAnatomyStr.begin(), showAnatomyStr.end(), showAnatomyStr.begin(), ::tolower); // Make it lowercase for case-insensitive comparison
      category->SetShowAnatomy( showAnatomyStr.compare("true") ? false : true );
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
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeArrayInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId)
{
  if (categoryId.CodingSchemeDesignator.empty() || categoryId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& categoryObject = this->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.IsNull())
    {
    vtkGenericWarningMacro("GetTypeArrayInTerminologyCategory: Failed to find category '" << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
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
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId)
{
  if (typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& typeArray = this->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.IsNull())
    {
    vtkGenericWarningMacro("GetTypeInTerminologyCategory: Failed to find type array for category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  int index = -1;
  return this->GetCodeInArray(typeId, typeArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyTypeFromJson(rapidjson::Value& typeObject, vtkSlicerTerminologyType* type)
{
  if (!typeObject.IsObject() || !type)
    {
    return false;
    }
  rapidjson::Value::MemberIterator recommendedDisplayRGBValue = typeObject.FindMember("recommendedDisplayRGBValue");
  // e.g. "Artery" (mandatory)
  rapidjson::Value::MemberIterator codeMeaning = typeObject.FindMember("CodeMeaning");
  // e.g. "SRT" (mandatory)
  rapidjson::Value::MemberIterator codingScheme = typeObject.FindMember("CodingSchemeDesignator");
  // e.g. "artery"
  rapidjson::Value::MemberIterator slicerLabel = typeObject.FindMember("3dSlicerLabel");
  // e.g. "85756007"
  rapidjson::Value::MemberIterator SNOMEDCTConceptID = typeObject.FindMember("SNOMEDCTConceptID");
  // e.g. "C0040300"
  rapidjson::Value::MemberIterator UMLSConceptUID = typeObject.FindMember("UMLSConceptUID");
  // e.g. "7051"
  rapidjson::Value::MemberIterator cid = typeObject.FindMember("cid");
  // e.g. "T-D0050" (mandatory)
  rapidjson::Value::MemberIterator codeValue = typeObject.FindMember("CodeValue");
  // e.g. "Segmentation Property Categories"
  rapidjson::Value::MemberIterator contextGroupName = typeObject.FindMember("contextGroupName");
  // Modifier array, containing modifiers of this type, e.g. "Left"
  rapidjson::Value::MemberIterator modifier = typeObject.FindMember("Modifier");
  if (codingScheme == typeObject.MemberEnd() || codeValue == typeObject.MemberEnd() || codeMeaning == typeObject.MemberEnd())
    {
    vtkGenericWarningMacro("PopulateTerminologyTypeFromJson: Unable to access mandatory type member");
    return false;
    }

  type->SetCodeMeaning(codeMeaning->value.GetString());
  type->SetCodingScheme(codingScheme->value.GetString());
  type->SetSlicerLabel(slicerLabel != typeObject.MemberEnd() ? slicerLabel->value.GetString() : NULL);
  type->SetSNOMEDCTConceptID(SNOMEDCTConceptID != typeObject.MemberEnd() ? SNOMEDCTConceptID->value.GetString() : NULL);
  type->SetUMLSConceptUID(UMLSConceptUID != typeObject.MemberEnd() ? UMLSConceptUID->value.GetString() : NULL);
  type->SetCid(cid != typeObject.MemberEnd() ? cid->value.GetString() : NULL);
  type->SetCodeValue(codeValue->value.GetString());
  type->SetContextGroupName(contextGroupName != typeObject.MemberEnd() ? contextGroupName->value.GetString() : NULL);

  if (recommendedDisplayRGBValue != typeObject.MemberEnd()
    && (recommendedDisplayRGBValue->value).IsArray() && (recommendedDisplayRGBValue->value).Size() == 3)
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
      type->SetRecommendedDisplayRGBValue(
        (unsigned char)recommendedDisplayRGBValue->value[0].GetInt(),
        (unsigned char)recommendedDisplayRGBValue->value[1].GetInt(),
        (unsigned char)recommendedDisplayRGBValue->value[2].GetInt() );
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

  type->SetHasModifiers((modifier->value).IsArray());

  return true;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeModifierArrayInTerminologyType(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId)
{
  if (typeId.CodingSchemeDesignator.empty() || typeId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& categoryObject = this->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.IsNull())
    {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find category '" <<
      categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  rapidjson::Value& typeObject = this->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId);
  if (typeObject.IsNull())
    {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find type '" << typeId.CodeMeaning << "' in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  rapidjson::Value& typeModifierArray = typeObject["Modifier"];
  if (!typeModifierArray.IsArray())
    {
    return JSON_EMPTY_VALUE;
    }

  return typeModifierArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeModifierInTerminologyType(
  std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, CodeIdentifier modifierId)
{
  if (modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& typeModifierArray = this->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.IsNull())
    {
    vtkGenericWarningMacro("GetTypeModifierInTerminologyType: Failed to find type modifier array for type '" << typeId.CodeMeaning << "' in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return JSON_EMPTY_VALUE;
    }

  int index = -1;
  return this->GetCodeInArray(modifierId, typeModifierArray, index);
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetAnatomicContextRootByName(std::string anatomicContextName)
{
  TerminologyMap::iterator anIt = this->LoadedAnatomicContexts.find(anatomicContextName);
  if (anIt != this->LoadedAnatomicContexts.end() && anIt->second != NULL)
    {
    return *(anIt->second);
    }

  return JSON_EMPTY_VALUE;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionArrayInAnatomicContext(std::string anatomicContextName)
{
  if (anatomicContextName.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& root = this->GetAnatomicContextRootByName(anatomicContextName);
  if (root.IsNull())
    {
    vtkGenericWarningMacro("GetRegionArrayInAnatomicContext: Failed to find anatomic context root for context name '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }

  rapidjson::Value& anatomicCodes = root["AnatomicCodes"];
  if (anatomicCodes.IsNull())
    {
    vtkGenericWarningMacro("GetRegionArrayInAnatomicContext: Failed to find AnatomicCodes member in anatomic context '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& anatomicRegionArray = anatomicCodes["AnatomicRegion"];
  if (!anatomicRegionArray.IsArray())
    {
    vtkGenericWarningMacro("GetRegionArrayInAnatomicContext: Failed to find AnatomicRegion array member in anatomic context '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }

  return anatomicRegionArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionInAnatomicContext(std::string anatomicContextName, CodeIdentifier regionId)
{
  if (regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& regionArray = this->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.IsNull())
    {
    vtkGenericWarningMacro("GetRegionInAnatomicContext: Failed to find region array for anatomic context '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }

  int index = -1;
  return this->GetCodeInArray(regionId, regionArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateRegionFromJson(rapidjson::Value& anatomicRegionObject, vtkSlicerTerminologyType* region)
{
  return this->PopulateTerminologyTypeFromJson(anatomicRegionObject, region);
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierArrayInRegion(std::string anatomicContextName, CodeIdentifier regionId)
{
  if (regionId.CodingSchemeDesignator.empty() || regionId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& regionObject = this->GetRegionInAnatomicContext(anatomicContextName, regionId);
  if (regionObject.IsNull())
    {
    vtkGenericWarningMacro("GetRegionModifierArrayInAnatomicRegion: Failed to find region '" <<
      regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }

  rapidjson::Value& regionModifierArray = regionObject["Modifier"];
  if (!regionModifierArray.IsArray())
    {
    vtkGenericWarningMacro("GetRegionModifierArrayInAnatomicRegion: Failed to find Modifier array member in region '" <<
      regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }

  return regionModifierArray;
}

//---------------------------------------------------------------------------
rapidjson::Value& vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierInRegion(
  std::string anatomicContextName, CodeIdentifier regionId, CodeIdentifier modifierId )
{
  if (modifierId.CodingSchemeDesignator.empty() || modifierId.CodeValue.empty())
    {
    return JSON_EMPTY_VALUE;
    }
  rapidjson::Value& regionModifierArray = this->GetRegionModifierArrayInRegion(anatomicContextName, regionId);
  if (regionModifierArray.IsNull())
    {
    vtkGenericWarningMacro("GetRegionModifierInRegion: Failed to find region modifier array for region '" <<
      regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return JSON_EMPTY_VALUE;
    }

  int index = -1;
  return this->GetCodeInArray(modifierId, regionModifierArray, index);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::ConvertSegmentationDescriptorToTerminologyContext(
  rapidjson::Document& descriptorDoc, rapidjson::Document& convertedDoc, std::string contextName )
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
    // Note: "The reason for the inner list is that we have one single schema both for input and output. When we provide input metafile,
    //       we can have multiple input files, and each file can have multiple labels, that is why we need to have list of lists"
    segmentAttributes = segmentAttributes[0]; // Enter "innerList"
    if ( !segmentAttributes.HasMember("SegmentedPropertyCategoryCodeSequence")
      || !segmentAttributes.HasMember("SegmentedPropertyTypeCodeSequence")
      || !segmentAttributes.HasMember("recommendedDisplayRGBValue") )
      {
      vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Invalid segment terminology entry at index " << index);
      ++index;
      continue;
      }
    rapidjson::Value& segmentCategory = segmentAttributes["SegmentedPropertyCategoryCodeSequence"];
    rapidjson::Value& segmentType = segmentAttributes["SegmentedPropertyTypeCodeSequence"];
    rapidjson::Value& segmentRecommendedDisplayRGBValue = segmentAttributes["recommendedDisplayRGBValue"];

    // Get type array if category already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier categoryId(
      segmentCategory["CodingSchemeDesignator"].GetString(), segmentCategory["CodeValue"].GetString(), segmentCategory["CodeMeaning"].GetString() );
    int foundCategoryIndex = -1;
    rapidjson::Value category(this->GetCodeInArray(categoryId, categoryArray, foundCategoryIndex), allocator);
    rapidjson::Value typeArray;
    if (category.IsObject() && category.HasMember("Type"))
      {
      typeArray = category["Type"];
      if (!typeArray.IsArray())
        {
        vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Failed to find Type array in category '" << categoryId.CodeMeaning);
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
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier typeId(
      segmentType["CodingSchemeDesignator"].GetString(), segmentType["CodeValue"].GetString(), segmentType["CodeMeaning"].GetString() );
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
    if (!segmentRecommendedDisplayRGBValue.IsArray() || segmentRecommendedDisplayRGBValue.Size() != 3 || !segmentRecommendedDisplayRGBValue[0].IsInt())
      {
      vtkGenericWarningMacro("ConvertSegmentationDescriptorToTerminologyContext: Invalid recommended color in type '" << typeId.CodeMeaning);
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
        segmentTypeModifier["CodeMeaning"].GetString() );
      int foundTypeModifierIndex = -1;
      rapidjson::Value typeModifier(this->GetCodeInArray(typeModifierId, typeModifierArray, foundTypeModifierIndex), allocator);
      // Modifier already exists, nothing to do
      if (typeModifier.IsObject())
        {
        ++index;
        continue;
        }

      // Create and populate modifier
      this->GetJsonCodeFromIdentifier(typeModifier, typeModifierId, allocator);
      if (typeModifier.HasMember("recommendedDisplayRGBValue"))
        {
        typeModifier.EraseMember("recommendedDisplayRGBValue");
        }
      typeModifier.AddMember("recommendedDisplayRGBValue", recommendedDisplayRGBValueArray, allocator); // Add color to type modifier

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
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::ConvertSegmentationDescriptorToAnatomicContext(
  rapidjson::Document& descriptorDoc, rapidjson::Document& convertedDoc, std::string contextName)
{
  if (!descriptorDoc.IsObject() || contextName.empty())
    {
    return false;
    }

  // Get segment attributes
  rapidjson::Value& segmentAttributesArray = descriptorDoc["segmentAttributes"];
  if (!segmentAttributesArray.IsArray())
    {
    vtkGenericWarningMacro("ConvertSegmentationDescriptorToAnatomicContext: Invalid segmentAttributes member");
    return false;
    }

  rapidjson::Document::AllocatorType& allocator = convertedDoc.GetAllocator();

  // Use terminology with context name if exists
  rapidjson::Value anatomicCodes;
  rapidjson::Value regionArray;
  if (convertedDoc.IsObject() && convertedDoc.HasMember("AnatomicCodes"))
    {
    anatomicCodes = convertedDoc["AnatomicCodes"];
    regionArray = anatomicCodes["AnatomicRegion"];
    }
  else
    {
    // If anatomic context was not found in the map, then initialize it and its members
    convertedDoc.SetObject();
    convertedDoc.AddMember("AnatomicContextName", rapidjson::StringRef(contextName.c_str()), allocator);
    anatomicCodes.SetObject();
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
    // Note: "The reason for the inner list is that we have one single schema both for input and output. When we provide input metafile,
    //       we can have multiple input files, and each file can have multiple labels, that is why we need to have list of lists"
    segmentAttributes = segmentAttributes[0]; // Enter "innerList"
    if (!segmentAttributes.HasMember("AnatomicRegionSequence"))
      {
      // Anatomic context is optional in the descriptor file
      ++index;
      continue;
      }
    rapidjson::Value& segmentRegion = segmentAttributes["AnatomicRegionSequence"];

    // Get region modifier array if region already exists, create empty otherwise
    vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionId(
      segmentRegion["CodingSchemeDesignator"].GetString(), segmentRegion["CodeValue"].GetString(), segmentRegion["CodeMeaning"].GetString() );
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

    // Add modifier if specified in descriptor and does not yet exist in anatomic context
    if (segmentAttributes.HasMember("AnatomicRegionModifierSequence"))
      {
      rapidjson::Value& segmentRegionModifier = segmentAttributes["AnatomicRegionModifierSequence"];
      vtkSlicerTerminologiesModuleLogic::CodeIdentifier regionModifierId(
        segmentRegionModifier["CodingSchemeDesignator"].GetString(),
        segmentRegionModifier["CodeValue"].GetString(),
        segmentRegionModifier["CodeMeaning"].GetString() );
      int foundRegionModifierIndex = -1;
      rapidjson::Value regionModifier(this->GetCodeInArray(regionModifierId, regionModifierArray, foundRegionModifierIndex), allocator);
      // Modifier already exists, nothing to do
      if (regionModifier.IsObject())
        {
        ++index;
        continue;
        }

      // Create and populate modifier
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

  // Set objects back to anatomic context Json object
  if (entryAdded)
    {
    if (anatomicCodes.HasMember("AnatomicRegion"))
      {
      anatomicCodes.EraseMember("AnatomicRegion");
      }
    anatomicCodes.AddMember("AnatomicRegion", regionArray, allocator);

    if (convertedDoc.HasMember("AnatomicCodes"))
      {
      convertedDoc.EraseMember("AnatomicCodes");
      }
    convertedDoc.AddMember("AnatomicCodes", anatomicCodes, allocator);

    return true;
    }

  return false;
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::vtkInternal::GetJsonCodeFromIdentifier(
  rapidjson::Value& code, CodeIdentifier identifier, rapidjson::Document::AllocatorType& allocator)
{
  code.AddMember("CodingSchemeDesignator", rapidjson::Value().SetString(identifier.CodingSchemeDesignator.c_str(), allocator), allocator);
  code.AddMember("CodeValue", rapidjson::Value().SetString(identifier.CodeValue.c_str(), allocator), allocator);
  code.AddMember("CodeMeaning", rapidjson::Value().SetString(identifier.CodeMeaning.c_str(), allocator), allocator);
}


//---------------------------------------------------------------------------
// vtkSlicerTerminologiesModuleLogic methods

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::vtkSlicerTerminologiesModuleLogic()
  : UserTerminologiesPath(NULL)
{
  this->Internal = new vtkInternal();
}

//----------------------------------------------------------------------------
vtkSlicerTerminologiesModuleLogic::~vtkSlicerTerminologiesModuleLogic()
{
  delete this->Internal;
  this->Internal = NULL;

  this->SetUserTerminologiesPath(NULL);
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

  // Load default terminologies and anatomical contexts
  // Note: Do it here not in the constructor so that the module shared directory is properly initialized
  bool wasModifying = this->GetDisableModifiedEvent();
  this->SetDisableModifiedEvent(true);
  this->LoadDefaultTerminologies();
  this->LoadDefaultAnatomicContexts();
  this->LoadUserTerminologies();
  this->SetDisableModifiedEvent(wasModifying);
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromFile(std::string filePath)
{
  std::string contextName;

  rapidjson::Document* terminologyRoot = new rapidjson::Document;

  FILE *fp = fopen(filePath.c_str(), "r");
  if (!fp)
    {
    vtkErrorMacro("LoadTerminologyFromFile: Failed to load terminology from file '" << filePath);
    return "";
    }
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (terminologyRoot->ParseStream(fs).HasParseError())
    {
    vtkErrorMacro("LoadTerminologyFromFile: Failed to load terminology from file '" << filePath);
    fclose(fp);
    return "";
    }

  // Store terminology
  contextName = (*terminologyRoot)["SegmentationCategoryTypeContextName"].GetString();
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedTerminologies, contextName, terminologyRoot);
  fclose(fp);
  vtkInfoMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  this->Modified();
  return contextName;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromSegmentDescriptorFile(std::string contextName, std::string filePath)
{
  FILE *fp = fopen(filePath.c_str(), "r");
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
  rapidjson::Document* convertedDoc = NULL;
  vtkInternal::TerminologyMap::iterator termIt = this->Internal->LoadedTerminologies.find(contextName);
  if (termIt != this->Internal->LoadedTerminologies.end() && termIt->second != NULL)
    {
    convertedDoc = termIt->second;
    }
  else
    {
    convertedDoc = new rapidjson::Document;
    }

  bool success = this->Internal->ConvertSegmentationDescriptorToTerminologyContext(descriptorDoc, *convertedDoc, contextName);
  if (!success)
    {
    vtkErrorMacro("LoadTerminologyFromSegmentDescriptorFile: Failed to parse descriptor file '" << filePath);
    fclose(fp);
    return false;
    }

  // Store terminology
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedTerminologies, contextName, convertedDoc );

  fclose(fp);
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
  std::string contextName;
  rapidjson::Document* anatomicContextRoot = new rapidjson::Document;

  FILE *fp = fopen(filePath.c_str(), "r");
  if (!fp)
    {
    vtkErrorMacro("LoadAnatomicContextFromFile: Failed to load terminology from file '" << filePath);
    return "";
    }

  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (anatomicContextRoot->ParseStream(fs).HasParseError())
    {
    vtkErrorMacro("LoadAnatomicContextFromFile: Failed to load terminology from file '" << filePath);
    fclose(fp);
    return "";
    }

  // Store anatomic context
  contextName = (*anatomicContextRoot)["AnatomicContextName"].GetString();
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedAnatomicContexts, contextName, anatomicContextRoot);

  fclose(fp);
  vtkInfoMacro("Anatomic context named '" << contextName << "' successfully loaded from file " << filePath);
  return contextName;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::LoadAnatomicContextFromSegmentDescriptorFile(std::string contextName, std::string filePath)
{
  FILE *fp = fopen(filePath.c_str(), "r");
  if (!fp)
    {
    vtkErrorMacro("LoadAnatomicContextFromSegmentDescriptorFile: Failed to load terminology from file '" << filePath);
    return false;
    }

  rapidjson::Document descriptorDoc;
  char buffer[4096];
  rapidjson::FileReadStream fs(fp, buffer, sizeof(buffer));
  if (descriptorDoc.ParseStream(fs).HasParseError())
    {
    vtkErrorMacro("LoadAnatomicContextFromSegmentDescriptorFile: Failed to load terminology from file '" << filePath);
    fclose(fp);
    return false;
    }

  // Convert the loaded descriptor json file into anatomic context json format
  rapidjson::Document* convertedDoc = NULL;
  vtkInternal::TerminologyMap::iterator anIt = this->Internal->LoadedAnatomicContexts.find(contextName);
  if (anIt != this->Internal->LoadedAnatomicContexts.end() && anIt->second != NULL)
    {
    convertedDoc = anIt->second;
    }
  else
    {
    convertedDoc = new rapidjson::Document;
    }

  bool success = this->Internal->ConvertSegmentationDescriptorToAnatomicContext(descriptorDoc, *convertedDoc, contextName);
  if (!success)
    {
    // Anatomic context is optional in descriptor file
    fclose(fp);
    return false;
    }

  // Store anatomic context
  vtkSlicerTerminologiesModuleLogic::vtkInternal::SetDocumentInTerminologyMap(
    this->Internal->LoadedAnatomicContexts, contextName, convertedDoc );

  fclose(fp);
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
void vtkSlicerTerminologiesModuleLogic::LoadUserTerminologies()
{
  if (!this->UserTerminologiesPath)
    {
    vtkErrorMacro("LoadUserTerminologies: User settings directory '"
      << (this->UserTerminologiesPath ? this->UserTerminologiesPath : "None") << "' does not exist");
    return;
    }
  if (!vtksys::SystemTools::FileExists(this->UserTerminologiesPath, false))
    {
    return;
    }

  // Try to load all json files in the user settings directory
  vtkSmartPointer<vtkDirectory> userSettingsDir = vtkSmartPointer<vtkDirectory>::New();
  userSettingsDir->Open(this->UserTerminologiesPath);
  //vtkSmartPointer<vtkStringArray> files = vtkSmartPointer<vtkStringArray>::Take(userSettingsDir->GetFiles());
  vtkStringArray* files = userSettingsDir->GetFiles();
  for (int index=0; index<files->GetNumberOfValues(); ++index)
    {
    std::string fileName = files->GetValue(index);

    // Only load json files
    if ( userSettingsDir->FileIsDirectory(fileName.c_str())
      || fileName.size() < 5 || fileName.substr(fileName.size()-5).compare(".json") )
      {
      continue;
      }

    // Try loading file
    std::string jsonFilePath = std::string(this->UserTerminologiesPath) + "/" + fileName;
    std::string success = this->LoadTerminologyFromFile(jsonFilePath);
    if (success.empty())
      {
      vtkErrorMacro("LoadUserTerminologies: Failed to load terminology from file " << files->GetValue(index));
      }
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

  rapidjson::Value& categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.IsNull())
    {
    vtkErrorMacro("FindCategoriesInTerminology: Failed to find category array in terminology '" << terminologyName << "'");
    return false;
    }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse categories
  rapidjson::SizeType index = 0;
  while (index<categoryArray.Size())
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
        std::transform(categoryNameLowerCase.begin(), categoryNameLowerCase.end(), categoryNameLowerCase.begin(), ::tolower);
        if (search.empty() || categoryNameLowerCase.find(search) != std::string::npos)
          {
          CodeIdentifier categoryId(categoryCodingSchemeDesignator.GetString(), categoryCodeValue.GetString(), categoryNameStr);
          categories.push_back(categoryId);
          }
        }
      else
        {
        vtkErrorMacro("FindCategoriesInTerminology: Invalid category '" << categoryName.GetString() << "' in terminology '" << terminologyName << "'");
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

  rapidjson::Value& categoryObject = this->Internal->GetCategoryInTerminology(terminologyName, categoryId);
  if (categoryObject.IsNull())
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

  rapidjson::Value& typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryId);
  if (typeArray.IsNull())
    {
    vtkErrorMacro("FindTypesInTerminologyCategory: Failed to find Type array member in category '"
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse types
  rapidjson::SizeType index = 0;
  while (index<typeArray.Size())
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
          }
        }
      else
        {
        vtkErrorMacro("FindTypesInTerminologyCategory: Invalid type '" << typeName.GetString() << "in category '"
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

  rapidjson::Value& typeObject = this->Internal->GetTypeInTerminologyCategory(terminologyName, categoryId, typeId);
  if (typeObject.IsNull())
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

  rapidjson::Value& typeModifierArray = this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryId, typeId);
  if (typeModifierArray.IsNull())
    {
    vtkErrorMacro("GetTypeModifiersInTerminologyType: Failed to find Type Modifier array member in type '" << typeId.CodeMeaning << "' in category "
      << categoryId.CodeMeaning << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Collect type modifiers
  rapidjson::SizeType index = 0;
  while (index<typeModifierArray.Size())
    {
    rapidjson::Value& typeModifier = typeModifierArray[index];
    if (typeModifier.IsObject())
      {
      rapidjson::Value& typeModifierName = typeModifier["CodeMeaning"];
      rapidjson::Value& typeModifierCodingSchemeDesignator = typeModifier["CodingSchemeDesignator"];
      rapidjson::Value& typeModifierCodeValue = typeModifier["CodeValue"];
      if (typeModifierName.IsString() && typeModifierCodingSchemeDesignator.IsString() && typeModifierCodeValue.IsString())
        {
        CodeIdentifier typeModifierId(typeModifierCodingSchemeDesignator.GetString(),
          typeModifierCodeValue.GetString(), typeModifierName.GetString());
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

  rapidjson::Value& typeModifierObject = this->Internal->GetTypeModifierInTerminologyType(terminologyName, categoryId, typeId, modifierId);
  if (typeModifierObject.IsNull())
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

  rapidjson::Value& regionArray = this->Internal->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.IsNull())
    {
    vtkErrorMacro("FindRegionsInAnatomicContext: Failed to find region array member in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Make lowercase for case-insensitive comparison
  std::transform(search.begin(), search.end(), search.begin(), ::tolower);

  // Traverse regions
  rapidjson::SizeType index = 0;
  while (index<regionArray.Size())
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
        vtkErrorMacro("FindRegionsInAnatomicContext: Invalid region '" << regionName.GetString()
          << "' in anatomic context '" << anatomicContextName << "'");
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

  rapidjson::Value& regionObject = this->Internal->GetRegionInAnatomicContext(anatomicContextName, regionId);
  if (regionObject.IsNull())
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

  rapidjson::Value& regionModifierArray = this->Internal->GetRegionModifierArrayInRegion(anatomicContextName, regionId);
  if (regionModifierArray.IsNull())
    {
    vtkErrorMacro("GetRegionModifiersInRegion: Failed to find Region Modifier array member in region '"
      << regionId.CodeMeaning << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Collect region modifiers
  rapidjson::SizeType index = 0;
  while (index<regionModifierArray.Size())
    {
    rapidjson::Value& regionModifier = regionModifierArray[index];
    if (regionModifier.IsObject())
      {
      rapidjson::Value& regionModifierName = regionModifier["CodeMeaning"];
      rapidjson::Value& regionModifierCodingSchemeDesignator = regionModifier["CodingSchemeDesignator"];
      rapidjson::Value& regionModifierCodeValue = regionModifier["CodeValue"];
      if (regionModifierName.IsString() && regionModifierCodingSchemeDesignator.IsString() && regionModifierCodeValue.IsString())
        {
        CodeIdentifier regionModifierId(regionModifierCodingSchemeDesignator.GetString(),
          regionModifierCodeValue.GetString(), regionModifierName.GetString());
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

  rapidjson::Value& regionModifierObject = this->Internal->GetRegionModifierInRegion(anatomicContextName, regionId, modifierId);
  if (regionModifierObject.IsNull())
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
