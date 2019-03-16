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

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through the Applied Cancer Research Unit program of Cancer Care
  Ontario with funds provided by the Ontario Ministry of Health and Long-Term Care

==============================================================================*/

// .NAME vtkSlicerViewControllersLogic - slicer logic class for SliceViewControllers module

#ifndef __vtkSlicerViewControllersLogic_h
#define __vtkSlicerViewControllersLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerViewControllersModuleLogicExport.h"

class vtkMRMLPlotViewNode;
class vtkMRMLSliceNode;
class vtkMRMLViewNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_VIEWCONTROLLERS_MODULE_LOGIC_EXPORT vtkSlicerViewControllersLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerViewControllersLogic *New();
  vtkTypeMacro(vtkSlicerViewControllersLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

public:

  /// Retrieves the default slice view node from the scene.
  /// The returned node can be changed to customize the contents of
  /// new view nodes. ResetAllViewNodesToDefault() can be called to
  /// update all existing view nodes to the new default settings immediately.
  vtkMRMLSliceNode* GetDefaultSliceViewNode();

  /// Retrieves the default 3D view node from the scene.
  /// The returned node can be changed to customize the contents of
  /// new view nodes. ResetAllViewNodesToDefault() can be called to
  /// update all existing view nodes to the new default settings immediately.
  vtkMRMLViewNode* GetDefaultThreeDViewNode();

  /// Retrieves the default plot view node from the scene.
  /// The returned node can be changed to customize the contents of
  /// new view nodes. ResetAllViewNodesToDefault() can be called to
  /// update all existing view nodes to the new default settings immediately.
  vtkMRMLPlotViewNode* GetDefaultPlotViewNode();

  /// Reset all existing slice and 3D view nodes to default settings.
  void ResetAllViewNodesToDefault();

protected:
  vtkSlicerViewControllersLogic();
  ~vtkSlicerViewControllersLogic() override;

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  void RegisterNodes() override;

private:
  vtkSlicerViewControllersLogic(const vtkSlicerViewControllersLogic&) = delete;
  void operator=(const vtkSlicerViewControllersLogic&) = delete;

};

#endif
