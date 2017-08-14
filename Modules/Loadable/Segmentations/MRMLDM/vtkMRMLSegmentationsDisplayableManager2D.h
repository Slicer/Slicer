/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

#ifndef __vtkMRMLSegmentationsDisplayableManager2D_h
#define __vtkMRMLSegmentationsDisplayableManager2D_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"

#include "vtkSlicerSegmentationsModuleMRMLDisplayableManagerExport.h"

/// \brief Displayable manager for showing segmentations in slice (2D) views.
///
/// Displays segmentations in slice viewers as labelmaps or contour lines
///
class VTK_SLICER_SEGMENTATIONS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLSegmentationsDisplayableManager2D
  : public vtkMRMLAbstractSliceViewDisplayableManager
{

public:
  static vtkMRMLSegmentationsDisplayableManager2D* New();
  vtkTypeMacro(vtkMRMLSegmentationsDisplayableManager2D, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent) VTK_OVERRIDE;

  /// Assemble and return info string to display in Data probe for a given viewer XYZ position.
  /// \return Invalid string by default, meaning no information to display.
  virtual std::string GetDataProbeInfoStringForPosition(double xyz[3]) VTK_OVERRIDE;

protected:
  virtual void UnobserveMRMLScene() VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) VTK_OVERRIDE;
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) VTK_OVERRIDE;

  /// Update Actors based on transforms in the scene
  virtual void UpdateFromMRML() VTK_OVERRIDE;

  virtual void OnMRMLSceneStartClose() VTK_OVERRIDE;
  virtual void OnMRMLSceneEndClose() VTK_OVERRIDE;

  virtual void OnMRMLSceneEndBatchProcess() VTK_OVERRIDE;

  /// Initialize the displayable manager based on its associated vtkMRMLSliceNode
  virtual void Create() VTK_OVERRIDE;

protected:
  vtkMRMLSegmentationsDisplayableManager2D();
  virtual ~vtkMRMLSegmentationsDisplayableManager2D();

private:
  vtkMRMLSegmentationsDisplayableManager2D(const vtkMRMLSegmentationsDisplayableManager2D&);// Not implemented
  void operator=(const vtkMRMLSegmentationsDisplayableManager2D&);                     // Not Implemented

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;
};

#endif
