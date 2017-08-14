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
#include "vtkMRMLDisplayableManagerWin32Header.h"

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
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

protected:
  vtkMRMLCrosshairDisplayableManager3D();
  virtual ~vtkMRMLCrosshairDisplayableManager3D();

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  virtual void Create() VTK_OVERRIDE;

  /// Method to perform additional initialization
  virtual void AdditionalInitializeStep() VTK_OVERRIDE;

private:
  vtkMRMLCrosshairDisplayableManager3D(const vtkMRMLCrosshairDisplayableManager3D&);// Not implemented
  void operator=(const vtkMRMLCrosshairDisplayableManager3D&);                     // Not Implemented

  virtual void UnobserveMRMLScene() VTK_OVERRIDE;
  virtual void ObserveMRMLScene() VTK_OVERRIDE;
  virtual void UpdateFromMRMLScene() VTK_OVERRIDE;
  virtual void OnMRMLNodeModified(vtkMRMLNode* node) VTK_OVERRIDE;

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
