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
#include <vtkIntArray.h> //TODO: Scene events?
#include <vtkStringArray.h>
#include <vtkVariant.h>

// JSON includes
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

  /// Get root Json value for the terminology with given name
  Json::Value GetTerminologyRootByName(std::string terminologyName);

  /// Get category array Json value for a given terminology
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetCategoryArrayInTerminology(std::string terminologyName);
  /// Get category Json object from terminology with given category name
  /// \return Null Json value on failure, the category Json object otherwise
  Json::Value GetCategoryInTerminology(std::string terminologyName, std::string categoryName);
  /// Populate \sa vtkSlicerTerminologyCategory from Json terminology
  bool PopulateTerminologyCategoryFromJson(Json::Value categoryObject, vtkSlicerTerminologyCategory* category);

  /// Get type array Json value for a given terminology and category
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetTypeArrayInTerminologyCategory(std::string terminologyName, std::string categoryName);
  /// Get type Json object from a terminology category with given type name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetTypeInTerminologyCategory(std::string terminologyName, std::string categoryName, std::string typeName);
  /// Populate \sa vtkSlicerTerminologyType from Json terminology
  bool PopulateTerminologyTypeFromJson(Json::Value typeObject, vtkSlicerTerminologyType* type);

  /// Get type modifier array Json value for a given terminology, category, and type
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetTypeModifierArrayInTerminologyType(std::string terminologyName, std::string categoryName, std::string typeName);
  /// Get type modifier Json object from a terminology, category, and type with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetTypeModifierInTerminologyType(std::string terminologyName, std::string categoryName, std::string typeName, std::string modifierName);

  /// Get root Json value for the anatomic context with given name
  Json::Value GetAnatomicContextRootByName(std::string anatomicContextName);

  /// Get region array Json value for a given anatomic context
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetRegionArrayInAnatomicContext(std::string anatomicContextName);
  /// Get type Json object from an anatomic context with given region name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetRegionInAnatomicContext(std::string anatomicContextName, std::string regionName);
  /// Populate \sa vtkSlicerTerminologyType from Json anatomic region
  bool PopulateRegionFromJson(Json::Value anatomicRegionObject, vtkSlicerTerminologyType* region);

  /// Get region modifier array Json value for a given anatomic context and region
  /// \return Null Json value on failure, the array object otherwise
  Json::Value GetRegionModifierArrayInRegion(std::string anatomicContextName, std::string regionName);
  /// Get type modifier Json object from an anatomic context and region with given modifier name
  /// \return Null Json value on failure, the type Json object otherwise
  Json::Value GetRegionModifierInRegion(std::string anatomicContextName, std::string regionName, std::string modifierName);

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
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetCategoryInTerminology(std::string terminologyName, std::string categoryName)
{
  if (categoryName.empty())
    {
    return Json::Value();
    }
  Json::Value categoryArray = this->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.isNull())
    {
    vtkGenericWarningMacro("GetCategoryInTerminology: Failed to find category array in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  // Traverse categories and try to find the one with the given name
  Json::ArrayIndex index = 0;
  while (categoryArray.isValidIndex(index))
    {
    Json::Value category = categoryArray[index];
    if (category.isObject())
      {
      Json::Value categoryCodeMeaning = category["codeMeaning"];
      if (categoryCodeMeaning.isString() && !categoryName.compare(categoryCodeMeaning.asString()))
        {
        return category;
        }
      }
    ++index;
    }

  // No category with the specified name was found
  return Json::Value();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyCategoryFromJson(Json::Value categoryObject, vtkSlicerTerminologyCategory* category)
{
  if (!categoryObject.isObject() || !category)
    {
    return false;
    }

  Json::Value codeMeaning = categoryObject["codeMeaning"];             // e.g. "Tissue" (mandatory)
  Json::Value codingScheme = categoryObject["codingScheme"];           // e.g. "SRT"
  Json::Value SNOMEDCTConceptID = categoryObject["SNOMEDCTConceptID"]; // e.g. "85756007"
  Json::Value UMLSConceptUID = categoryObject["UMLSConceptUID"];       // e.g. "C0040300"
  Json::Value cid = categoryObject["cid"];                             // e.g. "7051"
  Json::Value codeValue = categoryObject["codeValue"];                 // e.g. "T-D0050"
  Json::Value contextGroupName = categoryObject["contextGroupName"];   // e.g. "Segmentation Property Categories" (mandatory)
  Json::Value showAnatomy = categoryObject["showAnatomy"];
  if (!codeMeaning.isString() || !contextGroupName.isString())
    {
    vtkGenericWarningMacro("PopulateTerminologyCategoryFromJson: Unable to access mandatory category member");
    return false;
    }

  category->SetCodeMeaning(codeMeaning.asCString());
  category->SetCodingScheme(codingScheme.isString() ? codingScheme.asCString() : NULL);
  category->SetSNOMEDCTConceptID(SNOMEDCTConceptID.isString() ? SNOMEDCTConceptID.asCString() : NULL);
  category->SetUMLSConceptUID(UMLSConceptUID.isString()? UMLSConceptUID.asCString() : NULL);
  category->SetCid(cid.isString() ? cid.asCString() : NULL);
  category->SetCodeValue(codeValue.isString() ? codeValue.asCString() : NULL);
  category->SetContextGroupName(contextGroupName.asCString());
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
    category->SetShowAnatomy(false); // Default
    }

  return true;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeArrayInTerminologyCategory(std::string terminologyName, std::string categoryName)
{
  if (categoryName.empty())
    {
    return Json::Value();
    }
  Json::Value categoryObject = this->GetCategoryInTerminology(terminologyName, categoryName);
  if (categoryObject.isNull())
    {
    vtkGenericWarningMacro("GetTypeArrayInTerminologyCategory: Failed to find category '" << categoryName << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  Json::Value typeArray = categoryObject["Type"];
  if (!typeArray.isArray())
    {
    vtkGenericWarningMacro("GetTypeArrayInTerminologyCategory: Failed to find Type array member in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  return typeArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetTypeInTerminologyCategory(
  std::string terminologyName, std::string categoryName, std::string typeName)
{
  if (typeName.empty())
    {
    return Json::Value();
    }
  Json::Value typeArray = this->GetTypeArrayInTerminologyCategory(terminologyName, categoryName);
  if (typeArray.isNull())
    {
    vtkGenericWarningMacro("GetTypeInTerminologyCategory: Failed to find type array for category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  // Traverse types and try to find the one with the given name
  Json::ArrayIndex index = 0;
  while (typeArray.isValidIndex(index))
    {
    Json::Value type = typeArray[index];
    if (type.isObject())
      {
      Json::Value typeCodeMeaning = type["codeMeaning"];
      if (typeCodeMeaning.isString() && !typeName.compare(typeCodeMeaning.asString()))
        {
        return type;
        }
      }
    ++index;
    }

  // No type with the specified name was found
  return Json::Value();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateTerminologyTypeFromJson(Json::Value typeObject, vtkSlicerTerminologyType* type)
{
  if (!typeObject.isObject() || !type)
    {
    return false;
    }

  Json::Value recommendedDisplayRGBValue = typeObject["recommendedDisplayRGBValue"];
  Json::Value codeMeaning = typeObject["codeMeaning"];             // e.g. "Artery" (mandatory)
  Json::Value codingScheme = typeObject["codingScheme"];           // e.g. "SRT"
  Json::Value slicerLabel = typeObject["3dSlicerLabel"];           // e.g. "artery"
  Json::Value SNOMEDCTConceptID = typeObject["SNOMEDCTConceptID"]; // e.g. "85756007"
  Json::Value UMLSConceptUID = typeObject["UMLSConceptUID"];       // e.g. "C0040300"
  Json::Value cid = typeObject["cid"];                             // e.g. "7051"
  Json::Value codeValue = typeObject["codeValue"];                 // e.g. "T-D0050"
  Json::Value contextGroupName = typeObject["contextGroupName"];   // e.g. "Segmentation Property Categories" (mandatory)
  Json::Value modifier = typeObject["Modifier"];                   // Modifier array, containing modifiers of this type, e.g. "Left"
  if (!codeMeaning.isString() || !contextGroupName.isString())
    {
    vtkGenericWarningMacro("PopulateTerminologyTypeFromJson: Unable to access mandatory type member");
    return false;
    }

  type->SetCodeMeaning(codeMeaning.asCString());
  type->SetCodingScheme(codingScheme.isString() ? codingScheme.asCString() : NULL);
  type->SetSlicerLabel(slicerLabel.isString() ? slicerLabel.asCString() : NULL);
  type->SetSNOMEDCTConceptID(SNOMEDCTConceptID.isString() ? SNOMEDCTConceptID.asCString() : NULL);
  type->SetUMLSConceptUID(UMLSConceptUID.isString()? UMLSConceptUID.asCString() : NULL);
  type->SetCid(cid.isString() ? cid.asCString() : NULL);
  type->SetCodeValue(codeValue.isString() ? codeValue.asCString() : NULL);
  type->SetContextGroupName(contextGroupName.asCString());
  type->SetContextGroupName(contextGroupName.asCString());

  if ( recommendedDisplayRGBValue.isArray() && recommendedDisplayRGBValue.isValidIndex(2) &&
       recommendedDisplayRGBValue[0].isString() )
    {
    type->SetRecommendedDisplayRGBValue( // Note: Casting directly to unsigned char fails
      (unsigned char)vtkVariant(recommendedDisplayRGBValue[0].asString()).ToInt(),
      (unsigned char)vtkVariant(recommendedDisplayRGBValue[1].asString()).ToInt(),
      (unsigned char)vtkVariant(recommendedDisplayRGBValue[2].asString()).ToInt() );
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
  std::string terminologyName, std::string categoryName, std::string typeName)
{
  if (typeName.empty())
    {
    return Json::Value();
    }
  Json::Value categoryObject = this->GetCategoryInTerminology(terminologyName, categoryName);
  if (categoryObject.isNull())
    {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find category '" <<
      categoryName << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  Json::Value typeObject = this->GetTypeInTerminologyCategory(terminologyName, categoryName, typeName);
  if (typeObject.isNull())
    {
    vtkGenericWarningMacro("GetTypeModifierArrayInTerminologyType: Failed to find type '" << typeName << "' in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
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
  std::string terminologyName, std::string categoryName, std::string typeName, std::string modifierName)
{
  if (modifierName.empty())
    {
    return Json::Value();
    }
  Json::Value typeModifierArray = this->GetTypeModifierArrayInTerminologyType(terminologyName, categoryName, typeName);
  if (typeModifierArray.isNull())
    {
    vtkGenericWarningMacro("GetTypeModifierInTerminologyType: Failed to find type modifier array for type '" << typeName << "' in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
    return Json::Value();
    }

  // Traverse type modifiers and try to find the one with the given name
  Json::ArrayIndex index = 0;
  while (typeModifierArray.isValidIndex(index))
    {
    Json::Value typeModifier = typeModifierArray[index];
    if (typeModifier.isObject())
      {
      Json::Value typeModifierCodeMeaning = typeModifier["codeMeaning"];
      if (typeModifierCodeMeaning.isString() && !modifierName.compare(typeModifierCodeMeaning.asString()))
        {
        return typeModifier;
        }
      }
    ++index;
    }

  // No type modifier with the specified name was found
  return Json::Value();
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
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionInAnatomicContext(std::string anatomicContextName, std::string regionName)
{
  if (regionName.empty())
    {
    return Json::Value();
    }
  Json::Value regionArray = this->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.isNull())
    {
    vtkGenericWarningMacro("GetRegionInAnatomicContext: Failed to find region array for anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  // Traverse regions and try to find the one with the given name
  Json::ArrayIndex index = 0;
  while (regionArray.isValidIndex(index))
    {
    Json::Value region = regionArray[index];
    if (region.isObject())
      {
      Json::Value regionCodeMeaning = region["codeMeaning"];
      if (regionCodeMeaning.isString() && !regionName.compare(regionCodeMeaning.asString()))
        {
        return region;
        }
      }
    ++index;
    }

  // No region with the specified name was found
  return Json::Value();
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::vtkInternal::PopulateRegionFromJson(Json::Value anatomicRegionObject, vtkSlicerTerminologyType* region)
{
  return this->PopulateTerminologyTypeFromJson(anatomicRegionObject, region);
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierArrayInRegion(std::string anatomicContextName, std::string regionName)
{
  if (regionName.empty())
    {
    return Json::Value();
    }
  Json::Value regionObject = this->GetRegionInAnatomicContext(anatomicContextName, regionName);
  if (regionObject.isNull())
    {
    vtkGenericWarningMacro("GetRegionModifierArrayInAnatomicRegion: Failed to find region '" <<
      regionName << "' in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  Json::Value regionModifierArray = regionObject["Modifier"];
  if (!regionModifierArray.isArray())
    {
    vtkGenericWarningMacro("GetRegionModifierArrayInAnatomicRegion: Failed to find Modifier array member in region '" << regionName
      << "' in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  return regionModifierArray;
}

//---------------------------------------------------------------------------
Json::Value vtkSlicerTerminologiesModuleLogic::vtkInternal::GetRegionModifierInRegion(
  std::string anatomicContextName, std::string regionName, std::string modifierName )
{
  if (modifierName.empty())
    {
    return Json::Value();
    }
  Json::Value regionModifierArray = this->GetRegionModifierArrayInRegion(anatomicContextName, regionName);
  if (regionModifierArray.isNull())
    {
    vtkGenericWarningMacro("GetRegionModifierInRegion: Failed to find region modifier array for region '" << regionName
      << "' in anatomic context '" << anatomicContextName << "'");
    return Json::Value();
    }

  // Traverse region modifiers and try to find the one with the given name
  Json::ArrayIndex index = 0;
  while (regionModifierArray.isValidIndex(index))
    {
    Json::Value regionModifier = regionModifierArray[index];
    if (regionModifier.isObject())
      {
      Json::Value typeModifierCodeMeaning = regionModifier["codeMeaning"];
      if (typeModifierCodeMeaning.isString() && !modifierName.compare(typeModifierCodeMeaning.asString()))
        {
        return regionModifier;
        }
      }
    ++index;
    }

  // No region modifier with the specified name was found
  return Json::Value();
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
  vtkSmartPointer<vtkIntArray> events = vtkSmartPointer<vtkIntArray>::New();
  events->InsertNextValue(vtkMRMLScene::EndCloseEvent); //TODO: Needed?
  this->SetAndObserveMRMLSceneEvents(newScene, events.GetPointer());

  // Load default terminologies and anatomical contexts
  // Note: Do it here not in the constructor so that the module shared directory is properly initialized
  this->LoadDefaultTerminologies();
  this->LoadDefaultAnatomicContexts();
}

//---------------------------------------------------------------------------
void vtkSlicerTerminologiesModuleLogic::OnMRMLSceneEndClose() //TODO: Needed?
{
  if (!this->GetMRMLScene())
    {
    vtkErrorMacro("OnMRMLSceneEndClose: Invalid MRML scene!");
    return;
    }
}

//---------------------------------------------------------------------------
std::string vtkSlicerTerminologiesModuleLogic::LoadTerminologyFromFile(std::string filePath)
{
  std::ifstream terminologyStream(filePath.c_str(), std::ios_base::binary);

  std::string contextName("");
  Json::Value terminologyRoot;

  try
    {
    terminologyStream >> terminologyRoot;
    contextName = terminologyRoot["SegmentationCategoryTypeContextName"].asString();
    }
  catch (std::exception &e)
    {
    vtkErrorMacro("LoadDefaultTerminologies: Failed to load default terminology - exception: " << e.what());
    return "";
    }

  // Store terminology
  this->Internal->LoadedTerminologies[contextName] = terminologyRoot;

  vtkInfoMacro("Terminology named '" << contextName << "' successfully loaded from file " << filePath);
  return contextName;
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

  // Store terminology
  this->Internal->LoadedAnatomicContexts[contextName] = anatomicContextRoot;

  vtkInfoMacro("Anatomic context named '" << contextName << "' successfully loaded from file " << filePath);
  return contextName;
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
bool vtkSlicerTerminologiesModuleLogic::GetCategoriesInTerminology(std::string terminologyName, vtkCollection* categoryCollection)
{
  if (!categoryCollection)
    {
    return false;
    }
  categoryCollection->RemoveAllItems();

  Json::Value categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.isNull())
    {
    vtkErrorMacro("GetCategoriesInTerminology: Failed to find Category array in terminology '" << terminologyName << "'");
    return false;
    }

  // Collect categories
  Json::ArrayIndex index = 0;
  while (categoryArray.isValidIndex(index))
    {
    Json::Value category = categoryArray[index];
    if (category.isObject())
      {
      vtkSmartPointer<vtkSlicerTerminologyCategory> currentCategory = vtkSmartPointer<vtkSlicerTerminologyCategory>::New();
      if (!this->Internal->PopulateTerminologyCategoryFromJson(category, currentCategory))
        {
        vtkErrorMacro("GetCategoriesInTerminology: Unable to parse a category in terminology '" << terminologyName << "'");
        continue;
        }

      categoryCollection->AddItem(currentCategory);
      }
    ++index;
    } // For each category index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindCategoryNamesInTerminology(std::string terminologyName, vtkStringArray* categoryNames, std::string search/*=""*/)
{
  if (!categoryNames)
    {
    return false;
    }
  categoryNames->Initialize();

  Json::Value categoryArray = this->Internal->GetCategoryArrayInTerminology(terminologyName);
  if (categoryArray.isNull())
    {
    vtkErrorMacro("FindCategoryNamesInTerminology: Failed to find Category array in terminology '" << terminologyName << "'");
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
      Json::Value categoryName = category["codeMeaning"];
      if (categoryName.isString())
        {
        // Add category name to list if search string is empty or is contained by the current category name
        std::string categoryNameStr = categoryName.asString();
        std::string categoryNameLowerCase(categoryNameStr);
        std::transform(categoryNameLowerCase.begin(), categoryNameLowerCase.end(), categoryNameLowerCase.begin(), ::tolower);
        if (search.empty() || categoryNameLowerCase.find(search) != std::string::npos)
          {
          categoryNames->InsertNextValue(categoryNameStr);
          }
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetCategoryNamesInTerminology(std::string terminologyName, vtkStringArray* categoryNames)
{
  return this->FindCategoryNamesInTerminology(terminologyName, categoryNames, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetCategoryInTerminology(std::string terminologyName, std::string categoryName, vtkSlicerTerminologyCategory* category)
{
  if (!category || categoryName.empty())
    {
    return false;
    }

  Json::Value categoryObject = this->Internal->GetCategoryInTerminology(terminologyName, categoryName);
  if (categoryObject.isNull())
    {
    vtkErrorMacro("GetCategoryInTerminology: Failed to find category '" << categoryName << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Category with specified name found
  return this->Internal->PopulateTerminologyCategoryFromJson(categoryObject, category);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypesInTerminologyCategory(std::string terminologyName, std::string categoryName, vtkCollection* typeCollection)
{
  if (!typeCollection || categoryName.empty())
    {
    return false;
    }
  typeCollection->RemoveAllItems();

  Json::Value typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryName);
  if (typeArray.isNull())
    {
    vtkErrorMacro("GetTypesInTerminologyCategory: Failed to find Type array member in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Collect types
  Json::ArrayIndex index = 0;
  while (typeArray.isValidIndex(index))
    {
    Json::Value type = typeArray[index];
    if (type.isObject())
      {
      vtkSmartPointer<vtkSlicerTerminologyType> currentType = vtkSmartPointer<vtkSlicerTerminologyType>::New();
      if (!this->Internal->PopulateTerminologyTypeFromJson(type, currentType))
        {
        vtkErrorMacro("GetTypesInTerminologyCategory: Unable to parse a type in category '" << categoryName << "' in terminology '" << terminologyName << "'");
        continue;
        }

      typeCollection->AddItem(currentType);
      }
    ++index;
    } // For each type index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeNamesInTerminologyCategory(std::string terminologyName, std::string categoryName, vtkStringArray* typeNames)
{
  return this->FindTypeNamesInTerminologyCategory(terminologyName, categoryName, typeNames, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindTypeNamesInTerminologyCategory(
  std::string terminologyName, std::string categoryName, vtkStringArray* typeNames, std::string search)
{
  if (!typeNames)
    {
    return false;
    }
  typeNames->Initialize();

  Json::Value typeArray = this->Internal->GetTypeArrayInTerminologyCategory(terminologyName, categoryName);
  if (typeArray.isNull())
    {
    vtkErrorMacro("FindTypeNamesInTerminologyCategory: Failed to find Type array member in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
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
      Json::Value typeName = type["codeMeaning"];
      if (typeName.isString())
        {
        // Add type name to list if search string is empty or is contained by the current type name
        std::string typeNameStr = typeName.asString();
        std::string typeNameLowerCase(typeNameStr);
        std::transform(typeNameLowerCase.begin(), typeNameLowerCase.end(), typeNameLowerCase.begin(), ::tolower);
        if (search.empty() || typeNameLowerCase.find(search) != std::string::npos)
          {
          typeNames->InsertNextValue(typeNameStr);
          }
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeInTerminologyCategory(
  std::string terminologyName, std::string categoryName, std::string typeName, vtkSlicerTerminologyType* type)
{
  if (!type || typeName.empty())
    {
    return false;
    }

  Json::Value typeObject = this->Internal->GetTypeInTerminologyCategory(terminologyName, categoryName, typeName);
  if (typeObject.isNull())
    {
    vtkErrorMacro("GetTypeInTerminologyCategory: Failed to find type '" << typeName << "' in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Type with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(typeObject, type);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifiersInTerminologyType(
  std::string terminologyName, std::string categoryName, std::string typeName, vtkCollection* typeModifierCollection)
{
  if (!typeModifierCollection)
    {
    return false;
    }
  typeModifierCollection->RemoveAllItems();

  Json::Value typeModifierArray = this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryName, typeName);
  if (typeModifierArray.isNull())
    {
    vtkErrorMacro("GetTypeModifiersInTerminologyType: Failed to find Type Modifier array member in type '" << typeName << "' in category "
      << categoryName << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Collect type modifiers
  Json::ArrayIndex index = 0;
  while (typeModifierArray.isValidIndex(index))
    {
    Json::Value typeModifier = typeModifierArray[index];
    if (typeModifier.isObject())
      {
      vtkSmartPointer<vtkSlicerTerminologyType> currentTypeModifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
      if (!this->Internal->PopulateTerminologyTypeFromJson(typeModifier, currentTypeModifier))
        {
        vtkErrorMacro("GetTypeModifiersInTerminologyType: Unable to parse a type modifier in type '" << typeName << "' in category '"
          << categoryName << "' in terminology '" << terminologyName << "'");
        continue;
        }

      typeModifierCollection->AddItem(currentTypeModifier);
      }
    ++index;
    } // For each type index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifierNamesInTerminologyType(
  std::string terminologyName, std::string categoryName, std::string typeName, vtkStringArray* typeModifierNames)
{
  if (!typeModifierNames)
    {
    return false;
    }
  typeModifierNames->Initialize();

  Json::Value typeModifierArray = this->Internal->GetTypeModifierArrayInTerminologyType(terminologyName, categoryName, typeName);
  if (typeModifierArray.isNull())
    {
    vtkErrorMacro("GetTypeModifierNamesInTerminologyType: Failed to find Type Modifier array member in type '" << typeName << "' in category "
      << categoryName << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Traverse type modifiers
  Json::ArrayIndex index = 0;
  while (typeModifierArray.isValidIndex(index))
    {
    Json::Value typeModifier = typeModifierArray[index];
    if (typeModifier.isObject())
      {
      Json::Value typeNameObject = typeModifier["codeMeaning"];
      if (typeNameObject.isString())
        {
        typeModifierNames->InsertNextValue(typeNameObject.asString());
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetTypeModifierInTerminologyType(
std::string terminologyName, std::string categoryName, std::string typeName, std::string modifierName, vtkSlicerTerminologyType* typeModifier)
{
  if (!typeModifier || modifierName.empty())
    {
    return false;
    }

  Json::Value typeModifierObject = this->Internal->GetTypeModifierInTerminologyType(terminologyName, categoryName, typeName, modifierName);
  if (typeModifierObject.isNull())
    {
    vtkErrorMacro("GetTypeModifierInTerminologyType: Failed to find type modifier '" << modifierName << "' in type '" << typeName << "' in category '"
      << categoryName << "' in terminology '" << terminologyName << "'");
    return false;
    }

  // Type modifier with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(typeModifierObject, typeModifier);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionsInAnatomicContext(std::string anatomicContextName, vtkCollection* regionCollection)
{
  if (!regionCollection)
    {
    return false;
    }
  regionCollection->RemoveAllItems();

  Json::Value regionArray = this->Internal->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.isNull())
    {
    vtkErrorMacro("GetRegionsInAnatomicContext: Failed to find Region array in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Collect regions
  Json::ArrayIndex index = 0;
  while (regionArray.isValidIndex(index))
    {
    Json::Value region = regionArray[index];
    if (region.isObject())
      {
      vtkSmartPointer<vtkSlicerTerminologyType> currentRegion = vtkSmartPointer<vtkSlicerTerminologyType>::New();
      if (!this->Internal->PopulateRegionFromJson(region, currentRegion))
        {
        vtkErrorMacro("GetRegionsInAnatomicContext: Unable to parse a region in anatomic context '" << anatomicContextName << "'");
        continue;
        }

      regionCollection->AddItem(currentRegion);
      }
    ++index;
    } // For each region index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionNamesInAnatomicContext(std::string anatomicContextName, vtkStringArray* regionNames)
{
  return this->FindRegionNamesInAnatomicContext(anatomicContextName, regionNames, "");
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::FindRegionNamesInAnatomicContext(std::string anatomicContextName, vtkStringArray* regionNames, std::string search)
{
  if (!regionNames)
    {
    return false;
    }
  regionNames->Initialize();

  Json::Value regionArray = this->Internal->GetRegionArrayInAnatomicContext(anatomicContextName);
  if (regionArray.isNull())
    {
    vtkErrorMacro("FindRegionNamesInAnatomicContext: Failed to find Region array member in anatomic context '" << anatomicContextName << "'");
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
      Json::Value regionName = region["codeMeaning"];
      if (regionName.isString())
        {
        // Add region name to list if search string is empty or is contained by the current region name
        std::string regionNameStr = regionName.asString();
        std::string regionNameLowerCase(regionNameStr);
        std::transform(regionNameLowerCase.begin(), regionNameLowerCase.end(), regionNameLowerCase.begin(), ::tolower);
        if (search.empty() || regionNameLowerCase.find(search) != std::string::npos)
          {
          regionNames->InsertNextValue(regionNameStr);
          }
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionInAnatomicContext(std::string anatomicContextName, std::string regionName, vtkSlicerTerminologyType* region)
{
  if (!region || regionName.empty())
    {
    return false;
    }

  Json::Value regionObject = this->Internal->GetRegionInAnatomicContext(anatomicContextName, regionName);
  if (regionObject.isNull())
    {
    vtkErrorMacro("GetRegionInAnatomicContext: Failed to find region '" << regionName << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Region with specified name found
  return this->Internal->PopulateRegionFromJson(regionObject, region);
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifiersInAnatomicRegion(
  std::string anatomicContextName, std::string regionName, vtkCollection* regionModifierCollection )
{
  if (!regionModifierCollection || regionName.empty())
    {
    return false;
    }
  regionModifierCollection->RemoveAllItems();

  Json::Value regionModifierArray = this->Internal->GetRegionModifierArrayInRegion(anatomicContextName, regionName);
  if (regionModifierArray.isNull())
    {
    vtkErrorMacro("GetRegionModifiersInRegion: Failed to find Region Modifier array member in region '" << regionName
      << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Collect region modifiers
  Json::ArrayIndex index = 0;
  while (regionModifierArray.isValidIndex(index))
    {
    Json::Value regionModifier = regionModifierArray[index];
    if (regionModifier.isObject())
      {
      vtkSmartPointer<vtkSlicerTerminologyType> currentRegionModifier = vtkSmartPointer<vtkSlicerTerminologyType>::New();
      if (!this->Internal->PopulateRegionFromJson(regionModifier, currentRegionModifier))
        {
        vtkErrorMacro("GetRegionModifiersInRegion: Unable to parse a region modifier in region '" << regionName
          << "' in anatomic context '" << anatomicContextName << "'");
        continue;
        }

      regionModifierCollection->AddItem(currentRegionModifier);
      }
    ++index;
    } // For each region index

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifierNamesInAnatomicRegion(
  std::string anatomicContextName, std::string regionName, vtkStringArray* regionModifierNames )
{
  if (!regionModifierNames)
    {
    return false;
    }
  regionModifierNames->Initialize();

  Json::Value regionModifierArray = this->Internal->GetRegionModifierArrayInRegion(anatomicContextName, regionName);
  if (regionModifierArray.isNull())
    {
    vtkErrorMacro("GetRegionModifierNamesInAnatomicRegion: Failed to find Region Modifier array member in region '" << regionName
      << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Traverse region modifiers
  Json::ArrayIndex index = 0;
  while (regionModifierArray.isValidIndex(index))
    {
    Json::Value regionModifier = regionModifierArray[index];
    if (regionModifier.isObject())
      {
      Json::Value regionNameObject = regionModifier["codeMeaning"];
      if (regionNameObject.isString())
        {
        regionModifierNames->InsertNextValue(regionNameObject.asString());
        }
      }
    ++index;
    }

  return true;
}

//---------------------------------------------------------------------------
bool vtkSlicerTerminologiesModuleLogic::GetRegionModifierInAnatomicRegion(std::string anatomicContextName,
    std::string regionName, std::string modifierName, vtkSlicerTerminologyType* regionModifier)
{
  if (!regionModifier || modifierName.empty())
    {
    return false;
    }

  Json::Value regionModifierObject = this->Internal->GetRegionModifierInRegion(anatomicContextName, regionName, modifierName);
  if (regionModifierObject.isNull())
    {
    vtkErrorMacro("GetRegionModifierInAnatomicRegion: Failed to find region modifier '" << modifierName << "' in region '" << regionName
      << "' in anatomic context '" << anatomicContextName << "'");
    return false;
    }

  // Region modifier with specified name found
  return this->Internal->PopulateTerminologyTypeFromJson(regionModifierObject, regionModifier);
}
