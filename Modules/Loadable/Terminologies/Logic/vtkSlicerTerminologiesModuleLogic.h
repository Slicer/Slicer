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

#ifndef __vtkSlicerTerminologiesModuleLogic_h
#define __vtkSlicerTerminologiesModuleLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

#include "vtkSlicerTerminologiesModuleLogicExport.h"

#include <vtkVector.h>

class vtkStringArray;
class vtkSlicerTerminologyCategory;
class vtkSlicerTerminologyType;

/// \ingroup Slicer_QtModules_Terminologies
class VTK_SLICER_TERMINOLOGIES_LOGIC_EXPORT vtkSlicerTerminologiesModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerTerminologiesModuleLogic *New();
  vtkTypeMacro(vtkSlicerTerminologiesModuleLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Information needed to uniquely identify a terminology code
  class CodeIdentifier
    {
    public:
      CodeIdentifier(std::string codingSchemeDesignator, std::string codeValue, std::string codeMeaning)
        {
        this->CodingSchemeDesignator = codingSchemeDesignator;
        this->CodeValue = codeValue;
        this->CodeMeaning = codeMeaning;
        };
      std::string CodingSchemeDesignator;
      std::string CodeValue;
      std::string CodeMeaning; // Human readable name (not required for ID)
    };

  /// Load terminology dictionary from JSON terminology context file into \sa LoadedTerminologies.
  /// \param filePath File containing the terminology to load
  /// \return Context name (SegmentationCategoryTypeContextName) of the loaded terminology. Empty string on failure.
  std::string LoadTerminologyFromFile(std::string filePath);
  /// Load anatomic context dictionaries from JSON into \sa LoadedAnatomicContexts
  /// \param filePath File containing the anatomic context to load
  /// \return Context name (AnatomicContextName) of the loaded anatomic context. Empty string on failure.
  std::string LoadAnatomicContextFromFile(std::string filePath);
  /// Load terminology dictionary from segmentation descriptor JSON file into \sa LoadedTerminologies.
  /// \param Terminology context name (the descriptor file does not contain information about that)
  /// \param filePath File containing the terminology to load
  /// \return Success flag
  bool LoadTerminologyFromSegmentDescriptorFile(std::string contextName, std::string filePath);
  /// Load anatomic context dictionary from segmentation descriptor JSON file into \sa LoadedAnatomicContexts.
  /// See also \sa LoadTerminologyFromSegmentDescriptorFile
  bool LoadAnatomicContextFromSegmentDescriptorFile(std::string contextName, std::string filePath);

  /// Get context names of loaded terminologies
  void GetLoadedTerminologyNames(std::vector<std::string> &terminologyNames);
  /// Python accessor variant of \sa GetLoadedTerminologyNames
  void GetLoadedTerminologyNames(vtkStringArray* terminologyNames);
  /// Get context names of loaded anatomic contexts
  void GetLoadedAnatomicContextNames(std::vector<std::string> &anatomicContextNames);
  /// Python accessor variant of \sa GetLoadedAnatomicContextNames
  void GetLoadedAnatomicContextNames(vtkStringArray* anatomicContextNames);

  /// Get terminology categories from a terminology as collection of \sa vtkSlicerTerminologyCategory container objects
  /// Note: Separate function from \sa FindCategoryNamesInTerminology for python compatibility
  /// \param categories Output argument containing all the \sa vtkSlicerTerminologyCategory objects created
  ///   from the categories found in the given terminology
  /// \return Success flag
  bool GetCategoriesInTerminology(std::string terminologyName, std::vector<CodeIdentifier>& categories);
  /// Find category names (codeMeaning) in terminology containing a given string
  /// \param categories Output argument containing all the \sa vtkSlicerTerminologyCategory objects created
  ///   from the categories found in the given terminology
  /// \return Success flag
  bool FindCategoriesInTerminology(std::string terminologyName, std::vector<CodeIdentifier>& categories, std::string search);
  /// Get a category with given name from a terminology
  /// \param category Output argument containing the details of the found category if any (if return value is true)
  /// \return Success flag
  bool GetCategoryInTerminology(std::string terminologyName, CodeIdentifier categoryId, vtkSlicerTerminologyCategory* categoryObject);

  /// Get terminology types from a terminology category as collection of \sa vtkSlicerTerminologyType container objects
  /// Note: Separate function from \sa FindTypeNamesInTerminologyCategory for python compatibility
  /// \param typeCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the types found in the given terminology category
  /// \return Success flag
  bool GetTypesInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId, std::vector<CodeIdentifier>& types);
  /// Get all type names (codeMeaning) in a terminology category
  /// \param typeCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the types found in the given terminology category
  /// \return Success flag
  bool FindTypesInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId, std::vector<CodeIdentifier>& types, std::string search);
  /// Get a type with given name from a terminology category
  /// \param type Output argument containing the details of the found type if any (if return value is true)
  /// \return Success flag
  bool GetTypeInTerminologyCategory(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, vtkSlicerTerminologyType* typeObject);

  /// Get terminology type modifiers from a terminology type as collection of \sa vtkSlicerTerminologyType container objects
  /// \param typeModifierCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the type modifiers found in the given terminology type
  /// \return Success flag
  bool GetTypeModifiersInTerminologyType(std::string terminologyName, CodeIdentifier categoryId, CodeIdentifier typeId, std::vector<CodeIdentifier>& typeModifiers);
  /// Get a type modifier with given name from a terminology type
  /// \param typeModifier Output argument containing the details of the found type modifier if any (if return value is true)
  /// \return Success flag
  bool GetTypeModifierInTerminologyType(std::string terminologyName,
    CodeIdentifier categoryId, CodeIdentifier typeId, CodeIdentifier modifierId, vtkSlicerTerminologyType* typeModifier);

  /// Get anatomic regions from an anatomic context as collection of \sa vtkSlicerTerminologyType container objects
  /// Note: Separate function from \sa FindRegionNamesInAnatomicContext for python compatibility
  /// \param regionCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the regions found in the given anatomic context
  /// \return Success flag
  bool GetRegionsInAnatomicContext(std::string anatomicContextName, std::vector<CodeIdentifier>& regions);
  /// Get all region names (codeMeaning) in an anatomic context
  /// Note: Separate function from \sa FindRegionNamesInAnatomicContext for python compatibility
  /// \return Success flag
  bool FindRegionsInAnatomicContext(std::string anatomicContextName, std::vector<CodeIdentifier>& regions, std::string search);
  /// Get a region with given name from an anatomic context
  /// \param region Output argument containing the details of the found region if any (if return value is true)
  /// \return Success flag
  bool GetRegionInAnatomicContext(std::string anatomicContextName, CodeIdentifier regionId, vtkSlicerTerminologyType* regionObject);

  /// Get region modifiers from an anatomic region as collection of \sa vtkSlicerTerminologyType container objects
  /// \param regionModifierCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the region modifiers found in the given anatomic region
  /// \return Success flag
  bool GetRegionModifiersInAnatomicRegion(std::string anatomicContextName, CodeIdentifier regionId, std::vector<CodeIdentifier>& regionModifiers);
  /// Get a region modifier with given name from an anatomic region
  /// \param regionModifier Output argument containing the details of the found region modifier if any (if return value is true)
  /// \return Success flag
  bool GetRegionModifierInAnatomicRegion(std::string anatomicContextName,
    CodeIdentifier regionId, CodeIdentifier modifierId, vtkSlicerTerminologyType* regionModifier);

  /// Convert terminology category object to code identifier
  static CodeIdentifier CodeIdentifierFromTerminologyCategory(vtkSlicerTerminologyCategory* category);
  /// Convert terminology type object to code identifier
  static CodeIdentifier CodeIdentifierFromTerminologyType(vtkSlicerTerminologyType* type);

public:
  vtkGetStringMacro(UserTerminologiesPath);
  vtkSetStringMacro(UserTerminologiesPath);

protected:
  vtkSlicerTerminologiesModuleLogic();
  virtual ~vtkSlicerTerminologiesModuleLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Load default terminology dictionaries from JSON into \sa LoadedTerminologies
  void LoadDefaultTerminologies();
  /// Load default anatomic context dictionaries from JSON into \sa LoadedAnatomicContexts
  void LoadDefaultAnatomicContexts();
  /// Load terminologies from the user settings directory \sa UserTerminologiesPath
  void LoadUserTerminologies();

protected:
  /// The path from which the json files are automatically loaded on startup
  char* UserTerminologiesPath;

private:
  vtkSlicerTerminologiesModuleLogic(const vtkSlicerTerminologiesModuleLogic&); // Not implemented
  void operator=(const vtkSlicerTerminologiesModuleLogic&);              // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
