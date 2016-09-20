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

  /// Field names for 2D display parameters
  static const char* GetScalarRangeFieldName() {return "ScalarRange";};
  static const char* GetThresholdValueFieldName() {return "ThresholdValue";};
  static const char* GetInterpolationTypeFieldName() {return "InterpolationType";};

public:
  static vtkMRMLSegmentationsDisplayableManager2D* New();
  vtkTypeMacro(vtkMRMLSegmentationsDisplayableManager2D, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

  /// Assemble and return info string to display in Data probe for a given viewer XYZ position.
  /// \return Invalid string by default, meaning no information to display.
  virtual std::string GetDataProbeInfoStringForPosition(double xyz[3]);

protected:
  virtual void UnobserveMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);

  /// Update Actors based on transforms in the scene
  virtual void UpdateFromMRML();

  virtual void OnMRMLSceneStartClose();
  virtual void OnMRMLSceneEndClose();

  virtual void OnMRMLSceneEndBatchProcess();

  /// Initialize the displayable manager based on its associated vtkMRMLSliceNode
  virtual void Create();

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
