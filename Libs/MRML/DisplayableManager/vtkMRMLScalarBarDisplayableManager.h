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

#ifndef __vtkMRMLScalarBarDisplayableManager_h
#define __vtkMRMLScalarBarDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLScalarBarNode;
class vtkMRMLScene;

/// \brief Displayable manager for the scalar bars and window/level adjustment.
///
/// This displayable manager for now just manages window/level adjustment events,
/// but in the future it will implement scalar bar display in both 2D and 3D views.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLScalarBarDisplayableManager :
  public vtkMRMLAbstractSliceViewDisplayableManager
{
public:
  static vtkMRMLScalarBarDisplayableManager* New();
  vtkTypeMacro(vtkMRMLScalarBarDisplayableManager,
                       vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double &closestDistance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  void SetAdjustForegroundWindowLevelEnabled(bool enabled);
  bool GetAdjustForegroundWindowLevelEnabled();
  void SetAdjustBackgroundWindowLevelEnabled(bool enabled);
  bool GetAdjustBackgroundWindowLevelEnabled();

protected:
  vtkMRMLScalarBarDisplayableManager();
  ~vtkMRMLScalarBarDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  void Create() override;

  /// Called when the SliceNode is modified. May cause ScalarBar to remap its position on screen.
  void OnMRMLSliceNodeModifiedEvent() override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkMRMLScalarBarDisplayableManager(const vtkMRMLScalarBarDisplayableManager&) = delete;
  void operator=(const vtkMRMLScalarBarDisplayableManager&) = delete;

  void UnobserveMRMLScene() override;
  void UpdateFromMRMLScene() override;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
