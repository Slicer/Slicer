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

  This file was originally developed by Johan Andruejol, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

// .NAME vtkSlicerUnitsLogic - Slicer logic for unit manipulation
// .SECTION Description
// This class manages the logic associated with the units. It allows to create
// a new unit easily. The logic is in charge of calling a modify on the
// the selection node every time a current unit is modified so the listeners
// can update themselves.

#ifndef __vtkSlicerUnitsLogic_h
#define __vtkSlicerUnitsLogic_h

// Slicer includes
#include "vtkMRMLAbstractLogic.h"

// MRML includes
class vtkMRMLUnitNode;

#include "vtkSlicerUnitsModuleLogicExport.h"

class VTK_SLICER_UNITS_MODULE_LOGIC_EXPORT vtkSlicerUnitsLogic
  : public vtkMRMLAbstractLogic
{
public:
  static vtkSlicerUnitsLogic *New();
  vtkTypeMacro(vtkSlicerUnitsLogic, vtkMRMLAbstractLogic);
  virtual void PrintSelf(ostream& os, vtkIndent indent);

  //
  // Add unit node to the scene.
  // Returns NULL if the logic has no scene.
  vtkMRMLUnitNode* AddUnitNode(const char* name,
    const char* quantity = "length",
    const char* prefix = "",
    const char* suffix = "",
    int precision = 3,
    double min = -10000,
    double max = 10000);

  //
  // Change the default unit for the corresponding quantity
  void SetDefaultUnit(const char* quantity, const char* id);

  //
  // Get the scene with preset unit nodes in it.
  vtkMRMLScene* GetUnitsScene() const;

protected:
  vtkSlicerUnitsLogic();
  virtual ~vtkSlicerUnitsLogic();

  // Add the built in units in the units logic scene.
  virtual void AddDefaultsUnits();

  // Add the default units in the application scene
  virtual void AddBuiltInUnits(vtkMRMLScene* scene);

  // Overloaded to add the defaults units in the application scene.
  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);

  /// Register MRML Node classes to Scene.
  /// Gets called automatically when the MRMLScene is attached to this
  /// logic class.
  virtual void RegisterNodes();
  virtual void RegisterNodesInternal(vtkMRMLScene* scene);

  // Add a unit node to the given secne
  vtkMRMLUnitNode* AddUnitNodeToScene(vtkMRMLScene* scene,
    const char* name,
    const char* quantity = "length",
    const char* prefix = "",
    const char* suffix = "",
    int precision = 3,
    double min = -10000,
    double max = 10000);

  // Variables
  vtkMRMLScene* UnitsScene;
private:
  vtkSlicerUnitsLogic(const vtkSlicerUnitsLogic&); // Not implemented
  void operator=(const vtkSlicerUnitsLogic&); // Not implemented
};

#endif
