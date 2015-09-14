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
  vtkTypeMacro(vtkSlicerUnitsLogic, vtkMRMLAbstractLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  /// Add unit node to the scene.
  /// Returns NULL if the logic has no scene.
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

protected:
  vtkSlicerUnitsLogic();
  virtual ~vtkSlicerUnitsLogic();

  /// Reimplemented to initialize the scene with unit nodes.
  virtual void ObserveMRMLScene();
  /// Reimplemented to save the selection node unit nodes.
  /// \sa SaveDefaultUnits(), RestoreDefaultUnits()
  virtual void OnMRMLSceneStartBatchProcess();
  /// Reimplemented to restore the selection node unit nodes.
  /// \sa SaveDefaultUnits(), RestoreDefaultUnits()
  virtual void OnMRMLNodeModified(vtkMRMLNode* modifiedNode);

  /// Add the built in units in the units logic scene.
  virtual void AddDefaultsUnits();

  /// Add the default units in the application scene
  virtual void AddBuiltInUnits(vtkMRMLScene* scene);

  /// Overloaded to add the defaults units in the application scene.
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Register MRML Node classes to Scene.
  /// Gets called automatically when the MRMLScene is attached to this
  /// logic class.
  virtual void RegisterNodes();
  virtual void RegisterNodesInternal(vtkMRMLScene* scene);

  /// \brief Add a unit node to the given scene.
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
  vtkSlicerUnitsLogic(const vtkSlicerUnitsLogic&); // Not implemented
  void operator=(const vtkSlicerUnitsLogic&); // Not implemented

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
