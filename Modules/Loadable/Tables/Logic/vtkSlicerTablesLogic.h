/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright 2015 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Andras Lasso (PerkLab, Queen's
  University) and Kevin Wang (Princess Margaret Hospital, Toronto) and was
  supported through OCAIRO and the Applied Cancer Research Unit program of
  Cancer Care Ontario.

==============================================================================*/

#ifndef __vtkSlicerTablesLogic_h
#define __vtkSlicerTablesLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// Tables includes
#include "vtkSlicerTablesModuleLogicExport.h"

class vtkAbstractArray;
class vtkMRMLTableNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
/// \brief Slicer logic class for double array manipulation
/// This class manages the logic associated with reading, saving,
/// and changing propertied of the double array nodes
class VTK_SLICER_TABLES_MODULE_LOGIC_EXPORT vtkSlicerTablesLogic
  : public vtkSlicerModuleLogic
{
public:

  static vtkSlicerTablesLogic *New();
  vtkTypeMacro(vtkSlicerTablesLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Loads a table from filename.
  /// If findSchema is true then the method looks for a schema file (for example, basefilename.schema.csv)
  /// and if a schema file is found then it is used.
  vtkMRMLTableNode* AddTable(const char* fileName, const char* name = nullptr, bool findSchema = true, const char* password = nullptr);

  /// Returns ID of the layout that is similar to current layout but also contains a table view
  static int GetLayoutWithTable(int currentLayout);

protected:
  vtkSlicerTablesLogic();
  ~vtkSlicerTablesLogic() override;

private:
  vtkSlicerTablesLogic(const vtkSlicerTablesLogic&) = delete;
  void operator=(const vtkSlicerTablesLogic&) = delete;
};

#endif
