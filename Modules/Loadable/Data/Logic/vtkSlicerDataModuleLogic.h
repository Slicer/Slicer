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

// .NAME vtkSlicerDataModuleLogic - slicer logic class for data module

#ifndef __vtkSlicerDataModuleLogic_h
#define __vtkSlicerDataModuleLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// STD includes
#include <cstdlib>

#include "vtkSlicerDataModuleLogicExport.h"

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_DATA_LOGIC_EXPORT vtkSlicerDataModuleLogic :
  public vtkSlicerModuleLogic
{
public:
  static vtkSlicerDataModuleLogic *New();
  vtkTypeMacro(vtkSlicerDataModuleLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent);

public:
  vtkSetMacro( SceneChanged, bool );
  vtkGetMacro( SceneChanged, bool );
  vtkBooleanMacro( SceneChanged, bool );

protected:
  vtkSlicerDataModuleLogic();
  virtual ~vtkSlicerDataModuleLogic();

  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes();

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene);
  virtual void UpdateFromMRMLScene();

private:
  vtkSlicerDataModuleLogic(const vtkSlicerDataModuleLogic&); // Not implemented
  void operator=(const vtkSlicerDataModuleLogic&);               // Not implemented

protected:
  /// Flag indicating if the scene has recently changed (update of the module GUI if needed)
  bool SceneChanged;

};

#endif
