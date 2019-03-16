/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/


#ifndef __vtkSlicerUnitsLogic_h
#define __vtkSlicerUnitsLogic_h

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// MRML includes
class vtkMRMLUnitNode;

#include "vtkSlicerUnitsModuleLogicExport.h"

// STD includes
#include <map>

/// \brief Slicer logic for unit manipulation.
///
/// This class manages the logic associated with the units. It allows to create
/// a new unit easily. The logic is in charge of calling a modify on the
/// the selection node every time a current unit is modified so the listeners
/// can update themselves.
class VTK_SLICER_UNITS_MODULE_LOGIC_EXPORT vtkSlicerUnitsLogic
  : public vtkMRMLAbstractLogic
{
public:
  static vtkSlicerUnitsLogic *New();
  typedef vtkSlicerUnitsLogic Self;
  vtkTypeMacro(vtkSlicerUnitsLogic, vtkMRMLAbstractLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Add unit node to the scene.
  /// Returns nullptr if the logic has no scene.
  vtkMRMLUnitNode* AddUnitNode(const char* name,
    const char* quantity = "length",
    const char* prefix = "",
    const char* suffix = "",
    int precision = 3,
    double min = -10000.,
    double max = 10000.);

  /// Change the default unit for the corresponding quantity
  void SetDefaultUnit(const char* quantity, const char* id);

  /// Get the scene with preset unit nodes in it.
  vtkMRMLScene* GetUnitsScene() const;

  /// \brief Get the coefficient associated with the given SI prefix \a name.
  ///
  /// Returns the coefficient for all prefix names defined in the [International
  /// System of Units (SI)](https://en.wikipedia.org/wiki/Metric_prefix#List_of_SI_prefixes) reported in the table below.
  ///
  /// Otherwise, returns 1 if an empty name or an unknown prefix name is given.
  ///
  /// | Prefix | Coefficient                         |
  /// |--------|-------------------------------------|
  /// | yotta  | 10^24  (1000000000000000000000000)  |
  /// | zetta  | 10^21  (1000000000000000000000)     |
  /// | exa    | 10^18  (1000000000000000000)        |
  /// | peta   | 10^15  (1000000000000000)           |
  /// | tera   | 10^12  (1000000000000)              |
  /// | giga   | 10^9   (1000000000)                 |
  /// | mega   | 10^6   (1000000)                    |
  /// | kilo   | 10^3   (1000)                       |
  /// | hecto  | 10^2   (100)                        |
  /// | deca   | 10^1   (10)                         |
  /// |        | 10^0   (1)                          |
  /// | deci   | 10^-1  (0.1)                        |
  /// | centi  | 10^-2  (0.01)                       |
  /// | milli  | 10^-3  (0.001)                      |
  /// | micro  | 10^-6  (0.000001)                   |
  /// | nano   | 10^-9  (0.000000001)                |
  /// | pico   | 10^-12 (0.000000000001)             |
  /// | femto  | 10^-15 (0.000000000000001)          |
  /// | atto   | 10^-18 (0.000000000000000001)       |
  /// | zepto  | 10^-21 (0.000000000000000000001)    |
  /// | yocto  | 10^-24 (0.000000000000000000000001) |
  ///
  /// Source https://en.wikipedia.org/wiki/Metric_prefix#List_of_SI_prefixes
  ///
  /// \sa AddUnitNodeToScene()
  static double GetSIPrefixCoefficient(const char* prefix);

  /// \brief Get the coefficient to transform a value and display it.
  ///
  /// This function is used to conveniently compute the display coefficient
  /// expected by AddUnitNodeToScene().
  ///
  /// The display coefficient is used to transform quantity values to a given
  /// unit.
  ///
  /// By default, value for a given quantity are assumed to have no prefix. For
  /// example, this means that `length` values are in `meter`, `time` values are
  /// in `second`. In that case, the coefficient to transform quantity values to
  /// a given unit can be computed using GetDisplayCoefficient() specifying only
  /// the \a displayPrefix parameter.
  ///
  /// If the quantity values are associated with a specific unit, the \a valuePrefix
  /// parameter should be provided. For example, in Slicer, since `length` values
  /// are assumed to be in millimeter the display coefficient should be computed
  /// specifying `milli` as \a valuePrefix.
  ///
  /// \a prefix and \a basePrefix can be any value documented in GetSIPrefixCoefficient().
  ///
  /// \sa GetSIPrefixCoefficient()
  /// \sa AddUnitNodeToScene()
  /// \sa AddDefaultsUnits(), AddBuiltInUnits()
  static double GetDisplayCoefficient(const char* displayPrefix, const char* valuePrefix = "");

protected:
  vtkSlicerUnitsLogic();
  ~vtkSlicerUnitsLogic() override;

  /// Reimplemented to initialize the scene with unit nodes.
  void ObserveMRMLScene() override;
  /// Reimplemented to save the selection node unit nodes.
  /// \sa SaveDefaultUnits(), RestoreDefaultUnits()
  void OnMRMLSceneStartBatchProcess() override;
  /// Reimplemented to restore the selection node unit nodes.
  /// \sa SaveDefaultUnits(), RestoreDefaultUnits()
  void OnMRMLNodeModified(vtkMRMLNode* modifiedNode) override;

  /// Add the built in units in the units logic scene.
  virtual void AddDefaultsUnits();

  /// Add the default units in the application scene
  virtual void AddBuiltInUnits(vtkMRMLScene* scene);

  /// Overloaded to add the defaults units in the application scene.
  void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;

  /// Register MRML Node classes to Scene.
  /// Gets called automatically when the MRMLScene is attached to this
  /// logic class.
  void RegisterNodes() override;
  virtual void RegisterNodesInternal(vtkMRMLScene* scene);

  /// \brief Add a unit node to the given scene.
  ///
  /// A unit node is defined by the following properties:
  ///
  /// * Name: The property describes the unit itself. For example,
  /// the name of a `length` unit can be `Millimeter`, `Meter` or
  /// `Centimeter`. Setting the node's name also sets the node's singleton tag.
  ///
  /// * Quantity: This property describes what types of unit. For example the
  /// quantity of `second` and `day` is `time`. This property is a Units node
  /// attribute so it can be easily observed by the GUI.
  ///
  /// * Prefix and Suffix: Abbreviation/text displayed respectively before and
  /// after the unit.
  ///
  /// * Precision: This property describes the number of digit used after the
  /// comma. For example a precision of 2 gives 12.00 and -13.61.
  ///
  /// * Min and Max: Range of value allowed for the unit. For example, the
  /// minimum for the Kelvin value would be 0.
  ///
  /// * DisplayCoeff: Coefficient multiplied to the value to display it with
  /// the appropriate unit. The display coefficient corresponds to the inverse
  /// of the SI prefix coefficient associated with the unit.
  ///
  /// * DisplayOffset: Offset added to the value being displayed.
  ///
  /// \sa GetDisplayCoefficient()
  vtkMRMLUnitNode* AddUnitNodeToScene(vtkMRMLScene* scene,
    const char* name,
    const char* quantity = "length",
    const char* prefix = "",
    const char* suffix = "",
    int precision = 3,
    double min = -10000.,
    double max = 10000.,
    double displayCoeff = 1.0,
    double displayOffset = 0.0);

  /// Save the default units referenced in the selection node singleton.
  /// \sa RestoreDefaultUnits()
  void SaveDefaultUnits();

  /// Restore the saved default units referenced in the selection node
  /// singleton.
  /// \sa SaveDefaultUnits()
  void RestoreDefaultUnits();

  // Variables
  vtkMRMLScene* UnitsScene;
private:
  vtkSlicerUnitsLogic(const vtkSlicerUnitsLogic&) = delete;
  void operator=(const vtkSlicerUnitsLogic&) = delete;

  /// This variable contains the units of the singleton before the last scene
  /// batch process.
  /// \sa SaveDefaultUnits(), RestoreDefaultUnits()
  std::map<std::string, std::string> CachedDefaultUnits;
  /// This variable is on when restoring the default units with
  /// CachedDefaultUnits on the selection node.
  /// \sa SaveDefaultUnits(), RestoreDefaultUnits()
  bool RestoringDefaultUnits;
};

#endif
