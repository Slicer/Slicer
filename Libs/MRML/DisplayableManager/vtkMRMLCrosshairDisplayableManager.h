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

#ifndef __vtkMRMLCrosshairDisplayableManager_h
#define __vtkMRMLCrosshairDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractSliceViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerWin32Header.h"

/// \brief Displayable manager for the crosshair on slice (2D) views
///
/// Responsible for any display of the crosshair on Slice views.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLCrosshairDisplayableManager :
  public vtkMRMLAbstractSliceViewDisplayableManager
{
public:
  static vtkMRMLCrosshairDisplayableManager* New();
  vtkTypeMacro(vtkMRMLCrosshairDisplayableManager,
                       vtkMRMLAbstractSliceViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkMRMLCrosshairDisplayableManager();
  virtual ~vtkMRMLCrosshairDisplayableManager();

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  virtual void Create();

  /// Override the default interaction modes under which this
  /// displayable manager gets events.
  virtual int ActiveInteractionModes();

  /// Called after interactor style event specified using
  /// AddInteractorStyleObservableEvent are invoked.
  /// \sa AddInteractorStyleObservableEvent RemoveInteractorStyleObservableEvent
  virtual void OnInteractorStyleEvent(int eventid);

  /// Called after interactor event specified using
  /// AddInteractorObservableEvent are invoked.
  /// \sa AddInteractorObservableEvent RemoveInteractorObservableEvent
  virtual void OnInteractorEvent(int eventid);

  /// Called when the SliceNode is modified. May cause Crosshair to remap its position on screen.
  virtual void OnMRMLSliceNodeModifiedEvent();

  /// Method to perform additional initialization
  virtual void AdditionalInitializeStep();

private:
  vtkMRMLCrosshairDisplayableManager(const vtkMRMLCrosshairDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLCrosshairDisplayableManager&);                     // Not Implemented

  virtual void UnobserveMRMLScene();
  virtual void ObserveMRMLScene();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLNodeModified(vtkMRMLNode* node);

  class vtkInternal;
  vtkInternal * Internal;
};

#endif
