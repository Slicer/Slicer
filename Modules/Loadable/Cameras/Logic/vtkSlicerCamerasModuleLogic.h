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
class VTK_SLICER_CAMERAS_LOGIC_EXPORT vtkSlicerCamerasModuleLogic
  : public vtkSlicerModuleLogic
{
public:
  static vtkSlicerCamerasModuleLogic *New();
  vtkTypeMacro(vtkSlicerCamerasModuleLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

public:
  /// Scan the scene and search for the active camera that is used
  /// in the view.
  vtkMRMLCameraNode* GetViewActiveCameraNode(vtkMRMLViewNode* view);

protected:
  vtkSlicerCamerasModuleLogic();
  virtual ~vtkSlicerCamerasModuleLogic();

  //virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  //virtual void UpdateFromMRMLScene();

private:
  vtkSlicerCamerasModuleLogic(const vtkSlicerCamerasModuleLogic&); // Not implemented
  void operator=(const vtkSlicerCamerasModuleLogic&);              // Not implemented

protected:

};

#endif
