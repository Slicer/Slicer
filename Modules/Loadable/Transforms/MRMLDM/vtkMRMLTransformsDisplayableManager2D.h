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

==============================================================================*/

#ifndef __vtkMRMLTransformsDisplayableManager2D_h
#define __vtkMRMLTransformsDisplayableManager2D_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkSlicerTransformsModuleMRMLDisplayableManagerExport.h"

/// \brief Displayable manager for showing transforms in slice (2D) views.
///
/// Displays transforms in slice viewers as glyphs, deformed grid, or
/// contour lines
///
class VTK_SLICER_TRANSFORMS_MODULE_MRMLDISPLAYABLEMANAGER_EXPORT vtkMRMLTransformsDisplayableManager2D
  : public vtkMRMLAbstractSliceViewDisplayableManager
{

public:

  static vtkMRMLTransformsDisplayableManager2D* New();
  vtkTypeMacro(vtkMRMLTransformsDisplayableManager2D, vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:

  vtkMRMLTransformsDisplayableManager2D();
  virtual ~vtkMRMLTransformsDisplayableManager2D();

  virtual void UnobserveMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData);

  /// Update Actors based on transforms in the scene
  virtual void UpdateFromMRML();

  virtual void OnMRMLSceneStartClose();
  virtual void OnMRMLSceneEndClose();

  virtual void OnMRMLSceneEndBatchProcess();

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  virtual void Create();

private:

  vtkMRMLTransformsDisplayableManager2D(const vtkMRMLTransformsDisplayableManager2D&);// Not implemented
  void operator=(const vtkMRMLTransformsDisplayableManager2D&);                     // Not Implemented

  class vtkInternal;
  vtkInternal * Internal;
  friend class vtkInternal;
};

#endif
