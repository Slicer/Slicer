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

  This file was originally developed by Andras Lasso, PerkLab, Queen's University.

==============================================================================*/

#ifndef __vtkMRMLRulerDisplayableManager_h
#define __vtkMRMLRulerDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

// STL includes
#include <vector>

/// \brief Displayable manager that displays orienatation marker in a slice or 3D view
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLRulerDisplayableManager
  : public vtkMRMLAbstractDisplayableManager
{
  friend class vtkRulerRendererUpdateObserver;

public:
  static vtkMRMLRulerDisplayableManager* New();
  vtkTypeMacro(vtkMRMLRulerDisplayableManager,vtkMRMLAbstractDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Adds a scale preset to the list of possible scales.
  /// Length: The preset with Length closest to the actual length of the ruler will be used.
  /// If a preset exists with the same length then it will be updated with the new parameters.
  /// Number of major divisions: how many parts the ruler will be divided to by major ticks.
  /// Number of minor divisions: how many parts one division will be divided to by minor ticks.
  /// Displayed unit name: unit name displayed in the label.
  /// Displayed scale: 1 [actual unit] = displayedScale [displayed unit] (e.g., 1 mm = 0.1 cm)
  /// Example: (world coordinate is mm)
  /// 1.0 length, 5 major, 2 minor, mm, 1.0 scale
  /// 5.0         5        1        mm  1.0
  /// 10.0        5        2        cm  0.1
  void AddRulerScalePreset(double length, int numberOfMajorDivisons, int numberOfMinorDivisions, const std::string& displayedUnitName, double displayedScale);
  /// Remove all scale presets.
  void RemoveAllRulerScalePresets();

protected:

  vtkMRMLRulerDisplayableManager();
  ~vtkMRMLRulerDisplayableManager() override;

  /// Observe the View node and initialize the renderer accordingly.
  void Create() override;

  /// Called each time the view node is modified.
  /// Internally update the renderer from the view node.
  /// \sa UpdateFromMRMLViewNode()
  void OnMRMLDisplayableNodeModifiedEvent(vtkObject* caller) override;

  /// Update the renderer from the view node properties.
  void UpdateFromViewNode();

  /// Update the renderer based on the master renderer (the one that the orientation marker follows)
  void UpdateFromRenderer();

  struct RulerScalePreset
  {
    double Length; // actual length of the ruler (in world coordinates)
    int NumberOfMajorDivisions; // number of major divisions of the ruler
    int NumberOfMinorDivisions; // number of divisions of each major division
    std::string DisplayedUnitName; // label that will be shown after the scaled length
    double DisplayedScale; // displayed length is Length*Scale
  };

  // List of ruler presets, ordered based on length.
  std::vector<RulerScalePreset> RulerScalePresets;

private:

  vtkMRMLRulerDisplayableManager(const vtkMRMLRulerDisplayableManager&) = delete;
  void operator=(const vtkMRMLRulerDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
