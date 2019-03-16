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

  This file was originally developed by Andras Lasso (PerkLab, Queen's University).

==============================================================================*/

#ifndef __vtkMRMLCrosshairDisplayableManager3D_h
#define __vtkMRMLCrosshairDisplayableManager3D_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerExport.h"

/// \brief Displayable manager for the crosshair on 3D views
///
/// Responsible for any display of the crosshair on 3D views.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCrosshairDisplayableManager3D :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{
public:
  static vtkMRMLCrosshairDisplayableManager3D* New();
  vtkTypeMacro(vtkMRMLCrosshairDisplayableManager3D,
    vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) override;

protected:
  vtkMRMLCrosshairDisplayableManager3D();
  ~vtkMRMLCrosshairDisplayableManager3D() override;

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  void Create() override;

  /// Method to perform additional initialization
  void AdditionalInitializeStep() override;

private:
  vtkMRMLCrosshairDisplayableManager3D(const vtkMRMLCrosshairDisplayableManager3D&) = delete;
  void operator=(const vtkMRMLCrosshairDisplayableManager3D&) = delete;

  void UnobserveMRMLScene() override;
  void ObserveMRMLScene() override;
  void UpdateFromMRMLScene() override;
  void OnMRMLNodeModified(vtkMRMLNode* node) override;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
