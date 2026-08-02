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

==============================================================================*/

#ifndef vtkMRMLAccuratePicker_h
#define vtkMRMLAccuratePicker_h

// MRMLDisplayableManager includes
#include "vtkMRMLDisplayableManagerExport.h"

// VTK includes
#include <vtkCellPicker.h>
#include <vtkSmartPointer.h>

// STD includes
#include <map>

class vtkAbstractCellLocator;
class vtkPolyData;
class vtkRenderer;

/// \brief A cell picker that stays fast when large surfaces are shown.
///
/// vtkCellPicker tests every cell of every pickable surface unless a spatial
/// locator is registered for that surface's data. Over a large mesh -- for
/// example a segmentation closed surface with millions of cells -- an
/// un-indexed pick costs O(number of cells), tens to hundreds of milliseconds,
/// which makes per-mouse-move picking (markup dragging, hover read-out) janky.
///
/// vtkMRMLAccuratePicker registers and caches a vtkStaticCellLocator for each
/// large, pickable surface in the renderer before every pick, rebuilding a
/// locator only when its surface changes and dropping it when the surface is no
/// longer shown. Picks then become indexed queries. Everything else behaves
/// exactly like vtkCellPicker (same tolerance, picked position, and normal), so
/// it is a drop-in replacement.
///
/// A single instance is meant to be shared per view: vtkMRMLThreeDViewInteractorStyle
/// owns one and exposes it through vtkMRMLInteractionEventData::GetAccuratePicker(),
/// so every widget in the view -- and anything else that needs quick localization
/// in world coordinates -- reuses one picker and one set of locators.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLAccuratePicker : public vtkCellPicker
{
public:
  static vtkMRMLAccuratePicker* New();
  vtkTypeMacro(vtkMRMLAccuratePicker, vtkCellPicker);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Minimum number of cells for a surface to be indexed with a locator.
  /// Smaller surfaces are already cheap to pick by brute force, and building a
  /// locator for them would cost more than it saves. Defaults to 10000.
  vtkSetMacro(MinimumCellCountToIndex, vtkIdType);
  vtkGetMacro(MinimumCellCountToIndex, vtkIdType);

  using vtkCellPicker::Pick;

  /// Refresh the cell locators for the renderer's large surfaces, then pick as
  /// vtkCellPicker does.
  int Pick(double selectionX, double selectionY, double selectionZ, vtkRenderer* renderer) override;

protected:
  vtkMRMLAccuratePicker();
  ~vtkMRMLAccuratePicker() override;

  /// Register a cached locator with this picker for each large, pickable
  /// surface currently shown in the renderer (building or rebuilding it only
  /// when the surface changes) and drop locators for surfaces no longer shown.
  void UpdateLocators(vtkRenderer* renderer);

  struct CachedLocator
  {
    vtkSmartPointer<vtkAbstractCellLocator> Locator;
    vtkMTimeType BuildMTime{ 0 };
  };
  std::map<vtkPolyData*, CachedLocator> Locators;

  vtkIdType MinimumCellCountToIndex{ 10000 };

private:
  vtkMRMLAccuratePicker(const vtkMRMLAccuratePicker&) = delete;
  void operator=(const vtkMRMLAccuratePicker&) = delete;
};

#endif
