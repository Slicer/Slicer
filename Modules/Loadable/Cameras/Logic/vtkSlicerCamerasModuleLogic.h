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

  This file was originally developed by Julien Finet, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// .NAME vtkSlicerCamerasModuleLogic - Logic class for Cameras module

#ifndef __vtkSlicerCamerasModuleLogic_h
#define __vtkSlicerCamerasModuleLogic_h

// Slicer includes
#include "vtkSlicerCamerasModuleLogicExport.h"
#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLCameraNode;
class vtkMRMLViewNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
///
/// Properties of the scene-to-import camera nodes are always
/// copied into the existing nodes having the same name. This is done
/// when a camera node is about to be added to the scene.
///
class VTK_SLICER_CAMERAS_LOGIC_EXPORT vtkSlicerCamerasModuleLogic
  : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerCamerasModuleLogic *New();
  vtkTypeMacro(vtkSlicerCamerasModuleLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Scan the scene and search for the active camera that is used
  /// in the view.
  vtkMRMLCameraNode* GetViewActiveCameraNode(vtkMRMLViewNode* view);

protected:
  vtkSlicerCamerasModuleLogic();
  ~vtkSlicerCamerasModuleLogic() override;

private:
  vtkSlicerCamerasModuleLogic(const vtkSlicerCamerasModuleLogic&) = delete;
  void operator=(const vtkSlicerCamerasModuleLogic&) = delete;
};

#endif
