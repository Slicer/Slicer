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

  This file was originally developed by Davide Punzo, punzodavide@hotmail.it,
  and development was supported in part by CI3.

==============================================================================*/

#ifndef __vtkMRMLThreeDSliceEdgeDisplayableManager_h
#define __vtkMRMLThreeDSliceEdgeDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

/// \brief Displayable manager for showing slice edges in 3D views.
///
/// Responsible for any display based on the reformat widgets.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLThreeDSliceEdgeDisplayableManager
  : public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLThreeDSliceEdgeDisplayableManager* New();
  vtkTypeMacro(vtkMRMLThreeDSliceEdgeDisplayableManager, vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkMRMLThreeDSliceEdgeDisplayableManager();
  ~vtkMRMLThreeDSliceEdgeDisplayableManager() override;

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  void Create() override;

  void UnobserveMRMLScene() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  void OnMRMLNodeModified(vtkMRMLNode* node) override;
  void OnMRMLViewNodeModifiedEvent() override;

private:
  vtkMRMLThreeDSliceEdgeDisplayableManager(const vtkMRMLThreeDSliceEdgeDisplayableManager&) = delete;
  void operator=(const vtkMRMLThreeDSliceEdgeDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal* Internal;
};

#endif
