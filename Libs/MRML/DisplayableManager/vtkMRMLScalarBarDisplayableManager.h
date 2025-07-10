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
class vtkMRMLWindowLevelWidget;

/// \brief Displayable manager for window/level adjustment of volumes.
///
/// This displayable manager only manages window/level adjustment events, does not display
/// "scalar bar" (color legend). Its name was given because originally it was intended for
/// displaying color legend as well, but later a dedicated displayable manager was added for that purpose
/// (that can be used in any view types, for any displayable node types).
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLScalarBarDisplayableManager
  : public vtkMRMLAbstractSliceViewDisplayableManager
{
public:
  static vtkMRMLScalarBarDisplayableManager* New();
  vtkTypeMacro(vtkMRMLScalarBarDisplayableManager, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  bool CanProcessInteractionEvent(vtkMRMLInteractionEventData* eventData, double& closestDistance2) override;
  bool ProcessInteractionEvent(vtkMRMLInteractionEventData* eventData) override;

  void SetAdjustForegroundWindowLevelEnabled(bool enabled);
  bool GetAdjustForegroundWindowLevelEnabled();
  void SetAdjustBackgroundWindowLevelEnabled(bool enabled);
  bool GetAdjustBackgroundWindowLevelEnabled();

  vtkMRMLWindowLevelWidget* GetWindowLevelWidget();

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
  vtkInternal* Internal;
};

#endif
