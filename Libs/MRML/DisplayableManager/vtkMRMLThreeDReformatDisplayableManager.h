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

  This file was originally developed by Michael Jeulin-L, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#ifndef __vtkMRMLThreeDReformatDisplayableManager_h
#define __vtkMRMLThreeDReformatDisplayableManager_h

// MRMLDisplayableManager includes
#include "vtkMRMLAbstractThreeDViewDisplayableManager.h"
#include "vtkMRMLDisplayableManagerWin32Header.h"

/// \brief Displayable manager for ImplicitPlaneWidget2 in 3D views.
///
/// Responsible for any display based on the reformat widgets.
class VTK_MRML_DISPLAYABLEMANAGER_EXPORT vtkMRMLThreeDReformatDisplayableManager :
  public vtkMRMLAbstractThreeDViewDisplayableManager
{

public:
  static vtkMRMLThreeDReformatDisplayableManager* New();
  vtkTypeMacro(vtkMRMLThreeDReformatDisplayableManager,
                       vtkMRMLAbstractThreeDViewDisplayableManager);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkMRMLThreeDReformatDisplayableManager();
  virtual ~vtkMRMLThreeDReformatDisplayableManager();

  /// Initialize the displayable manager based on its associated
  /// vtkMRMLSliceNode
  virtual void Create();

  /// WidgetCallback is a static function to relay modified events from the Logic
  virtual void ProcessWidgetsEvents(vtkObject *caller, unsigned long event, void *callData);

  virtual void UnobserveMRMLScene();
  virtual void UpdateFromMRMLScene();
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node);
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node);
  virtual void OnMRMLNodeModified(vtkMRMLNode* node);

private:
  vtkMRMLThreeDReformatDisplayableManager(const vtkMRMLThreeDReformatDisplayableManager&);// Not implemented
  void operator=(const vtkMRMLThreeDReformatDisplayableManager&);                         // Not Implemented

  class vtkInternal;
  vtkInternal* Internal;
};

#endif // vtkMRMLThreeDReformatDisplayableManager_h
