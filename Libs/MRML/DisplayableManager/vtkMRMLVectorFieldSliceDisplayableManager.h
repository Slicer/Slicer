/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso and Franklin King at
  PerkLab, Queen's University and was supported through the Applied Cancer
  Research Unit program of Cancer Care Ontario with funds provided by the
  Ontario Ministry of Health and Long-Term Care.

  This file was further developed with funding from the Children's Hospital of
  Philadelphia Frontier Program.

==============================================================================*/

#ifndef __vtkMRMLVectorFieldSliceDisplayableManager_h
#define __vtkMRMLVectorFieldSliceDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

class vtkMRMLDisplayableNode;

/// \brief Displays vector fields in slice views.
///
/// Resampleable fields (volume, transform, volumetric mesh) are sampled in the slice plane; surface mesh points are
/// shown within the slice thickness. Glyphs are drawn flat in the slice, other modes are cut or projected to it.
///
/// \sa vtkMRMLVectorFieldDisplayableManager
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLVectorFieldSliceDisplayableManager : public vtkMRMLAbstractSliceViewDisplayableManager
{

public:
  static vtkMRMLVectorFieldSliceDisplayableManager* New();
  vtkTypeMacro(vtkMRMLVectorFieldSliceDisplayableManager, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  // DisplayableNode handling customizations
  void AddDisplayableNode(vtkMRMLDisplayableNode* displayableNode);
  void RemoveDisplayableNode(vtkMRMLDisplayableNode* displayableNode);

protected:
  vtkMRMLVectorFieldSliceDisplayableManager();
  ~vtkMRMLVectorFieldSliceDisplayableManager() override;

  void UnobserveMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

  void UpdateFromMRML() override;
  void OnMRMLSceneStartClose() override;
  void OnMRMLSceneEndClose() override;
  void OnMRMLSceneEndBatchProcess() override;

  /// Initialize the displayable manager based on its associated vtkMRMLSliceNode
  void Create() override;

  int AddingDisplayableNode;

private:
  vtkMRMLVectorFieldSliceDisplayableManager(const vtkMRMLVectorFieldSliceDisplayableManager&) = delete;
  void operator=(const vtkMRMLVectorFieldSliceDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
  friend class vtkInternal;
};

#endif
