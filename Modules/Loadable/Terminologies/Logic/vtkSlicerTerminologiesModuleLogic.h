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

  /// Load terminology dictionary from JSON into \sa LoadedTerminologies
  /// \param filePath File containing the terminology to load
  /// \return Context name (SegmentationCategoryTypeContextName) of the loaded terminology. Empty string on failure.
  std::string LoadTerminologyFromFile(std::string filePath);
  /// Load anatomic context dictionaries from JSON into \sa LoadedAnatomicContexts
  /// \param filePath File containing the anatomic context to load
  /// \return Context name (AnatomicContextName) of the loaded anatomic context. Empty string on failure.
  std::string LoadAnatomicContextFromFile(std::string filePath);

  /// Get context names of loaded terminologies
  void GetLoadedTerminologyNames(std::vector<std::string> &terminologyNames);
  /// Python accessor variant of \sa GetLoadedTerminologyNames
  void GetLoadedTerminologyNames(vtkStringArray* terminologyNames);
  /// Get context names of loaded anatomic contexts
  void GetLoadedAnatomicContextNames(std::vector<std::string> &anatomicContextNames);
  /// Python accessor variant of \sa GetLoadedAnatomicContextNames
  void GetLoadedAnatomicContextNames(vtkStringArray* anatomicContextNames);

  /// Get terminology categories from a terminology as collection of \sa vtkSlicerTerminologyCategory container objects
  /// \param categoryCollection Output argument containing all the \sa vtkSlicerTerminologyCategory objects created
  ///   from the categories found in the given terminology
  /// \return Success flag
  bool GetCategoriesInTerminology(std::string terminologyName, vtkCollection* categoryCollection);
  /// Get all category names (codeMeaning) in terminology
  /// Note: Separate function from \sa FindCategoryNamesInTerminology for python compatibility
  /// \return Success flag
  bool GetCategoryNamesInTerminology(std::string terminologyName, vtkStringArray* categoryNames);
  /// Find category names (codeMeaning) in terminology containing a given string
  /// \return Success flag
  bool FindCategoryNamesInTerminology(std::string terminologyName, vtkStringArray* categoryNames, std::string search);
  /// Get a category with given name from a terminology
  /// \param category Output argument containing the details of the found category if any (if return value is true)
  /// \return Success flag
  bool GetCategoryInTerminology(std::string terminologyName, std::string categoryName, vtkSlicerTerminologyCategory* category);

  /// Get terminology types from a terminology category as collection of \sa vtkSlicerTerminologyType container objects
  /// \param typeCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the types found in the given terminology category
  /// \return Success flag
  bool GetTypesInTerminologyCategory(std::string terminologyName, std::string categoryName, vtkCollection* typeCollection);
  /// Get all type names (codeMeaning) in a terminology category
  /// Note: Separate function from \sa FindTypeNamesInTerminologyCategory for python compatibility
  /// \return Success flag
  bool GetTypeNamesInTerminologyCategory(std::string terminologyName, std::string categoryName, vtkStringArray* typeNames);
  /// Find type names (codeMeaning) in a terminology category containing a given string
  /// \return Success flag
  bool FindTypeNamesInTerminologyCategory(std::string terminologyName, std::string categoryName, vtkStringArray* typeNames, std::string search);
  /// Get a type with given name from a terminology category
  /// \param type Output argument containing the details of the found type if any (if return value is true)
  /// \return Success flag
  bool GetTypeInTerminologyCategory(std::string terminologyName, std::string categoryName, std::string typeName, vtkSlicerTerminologyType* type);

  /// Get terminology type modifiers from a terminology type as collection of \sa vtkSlicerTerminologyType container objects
  /// \param typeModifierCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the type modifiers found in the given terminology type
  /// \return Success flag
  bool GetTypeModifiersInTerminologyType(std::string terminologyName, std::string categoryName, std::string typeName, vtkCollection* typeModifierCollection);
  /// Get all type modifier names (codeMeaning) in a terminology type
  /// \return Success flag
  bool GetTypeModifierNamesInTerminologyType(std::string terminologyName, std::string categoryName, std::string typeName, vtkStringArray* typeModifierNames);
  /// Get a type modifier with given name from a terminology type
  /// \param typeModifier Output argument containing the details of the found type modifier if any (if return value is true)
  /// \return Success flag
  bool GetTypeModifierInTerminologyType(std::string terminologyName,
    std::string categoryName, std::string typeName, std::string modifierName, vtkSlicerTerminologyType* typeModifier);

  /// Get anatomic regions from an anatomic context as collection of \sa vtkSlicerTerminologyType container objects
  /// \param regionCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the regions found in the given anatomic context
  /// \return Success flag
  bool GetRegionsInAnatomicContext(std::string anatomicContextName, vtkCollection* regionCollection);
  /// Get all region names (codeMeaning) in an anatomic context
  /// Note: Separate function from \sa FindRegionNamesInAnatomicContext for python compatibility
  /// \return Success flag
  bool GetRegionNamesInAnatomicContext(std::string anatomicContextName, vtkStringArray* regionNames);
  /// Find region names (codeMeaning) in an anatomic context containing a given string
  /// \return Success flag
  bool FindRegionNamesInAnatomicContext(std::string anatomicContextName, vtkStringArray* regionNames, std::string search);
  /// Get a region with given name from an anatomic context
  /// \param region Output argument containing the details of the found region if any (if return value is true)
  /// \return Success flag
  bool GetRegionInAnatomicContext(std::string anatomicContextName, std::string regionName, vtkSlicerTerminologyType* region);

  /// Get region modifiers from an anatomic region as collection of \sa vtkSlicerTerminologyType container objects
  /// \param regionModifierCollection Output argument containing all the \sa vtkSlicerTerminologyType objects created
  ///   from the region modifiers found in the given anatomic region
  /// \return Success flag
  bool GetRegionModifiersInAnatomicRegion(std::string anatomicContextName, std::string regionName, vtkCollection* regionModifierCollection);
  /// Get all region modifier names (codeMeaning) in an anatomic regions
  /// \return Success flag
  bool GetRegionModifierNamesInAnatomicRegion(std::string anatomicContextName, std::string regionName, vtkStringArray* regionModifierNames);
  /// Get a region modifier with given name from an anatomic region
  /// \param regionModifier Output argument containing the details of the found region modifier if any (if return value is true)
  /// \return Success flag
  bool GetRegionModifierInAnatomicRegion(std::string anatomicContextName,
    std::string regionName, std::string modifierName, vtkSlicerTerminologyType* regionModifier);

protected:
  vtkSlicerTerminologiesModuleLogic();
  virtual ~vtkSlicerTerminologiesModuleLogic();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  virtual void OnMRMLSceneEndClose();

  /// Load default terminology dictionaries from JSON into \sa LoadedTerminologies
  void LoadDefaultTerminologies();
  /// Load default anatomic context dictionaries from JSON into \sa LoadedAnatomicContexts
  void LoadDefaultAnatomicContexts();

private:
  vtkSlicerTerminologiesModuleLogic(const vtkSlicerTerminologiesModuleLogic&); // Not implemented
  void operator=(const vtkSlicerTerminologiesModuleLogic&);              // Not implemented

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
