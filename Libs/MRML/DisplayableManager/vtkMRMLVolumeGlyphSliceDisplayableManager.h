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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLVolumeGlyphSliceDisplayableManager_h
#define __vtkMRMLVolumeGlyphSliceDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

/// \brief Displayable manager for slice (2D) views.
///
/// Responsible for any display on Slice views that is not the slice themselves
/// nor the annotations.
/// Currently support only glyph display for Diffusion Tensor volumes.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLVolumeGlyphSliceDisplayableManager
  : public vtkMRMLAbstractSliceViewDisplayableManager
{
public:
  static vtkMRMLVolumeGlyphSliceDisplayableManager* New();
  vtkTypeMacro(vtkMRMLVolumeGlyphSliceDisplayableManager,
                       vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:

  vtkMRMLVolumeGlyphSliceDisplayableManager();
  ~vtkMRMLVolumeGlyphSliceDisplayableManager() override;

  void UnobserveMRMLScene() override;
  void UpdateFromMRMLScene() override;
  void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;
  void OnMRMLSceneStartClose() override;

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  void Create() override;

private:

  vtkMRMLVolumeGlyphSliceDisplayableManager(const vtkMRMLVolumeGlyphSliceDisplayableManager&) = delete;
  void operator=(const vtkMRMLVolumeGlyphSliceDisplayableManager&) = delete;

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;
};

#endif
