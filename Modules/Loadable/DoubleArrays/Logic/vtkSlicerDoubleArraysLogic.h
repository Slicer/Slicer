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

#ifndef __vtkSlicerDoubleArraysLogic_h
#define __vtkSlicerDoubleArraysLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes
class vtkMRMLDoubleArrayNode;

// DoubleArrays includes
#include "vtkSlicerDoubleArraysModuleLogicExport.h"

/// \ingroup Slicer_QtModules_ExtensionTemplate
/// \brief Slicer logic class for double array manipulation
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the double array nodes
class VTK_SLICER_DOUBLEARRAYS_MODULE_LOGIC_EXPORT vtkSlicerDoubleArraysLogic
  : public vtkSlicerModuleLogic
{
public:

  static vtkSlicerDoubleArraysLogic *New();
  vtkTypeMacro(vtkSlicerDoubleArraysLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  vtkMRMLDoubleArrayNode* AddDoubleArray(const char* fileName,
                                         const char* name = nullptr);
protected:
  vtkSlicerDoubleArraysLogic();
  ~vtkSlicerDoubleArraysLogic() override;

private:
  vtkSlicerDoubleArraysLogic(const vtkSlicerDoubleArraysLogic&) = delete;
  void operator=(const vtkSlicerDoubleArraysLogic&) = delete;
};

#endif
