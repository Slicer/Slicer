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

==============================================================================*/

#ifndef __vtkSlicerTablesReader_h
#define __vtkSlicerTablesReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerTablesModuleLogicExport.h"

// VTK includes
#include <vtkCommand.h>

class vtkSlicerTablesLogic;

/// Reader of table files (text and SQLite database).
///
/// Properties: "fileName", "name", "password" (for SQLite databases).
///
/// If a SQLite database cannot be opened without a password and no password is specified in the properties
/// then PasswordRequestedEvent is invoked. The call data is a pointer to a std::string, which the
/// observer (for example, a password dialog) may set to the password.
class VTK_SLICER_TABLES_MODULE_LOGIC_EXPORT vtkSlicerTablesReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerTablesReader* New();
  vtkTypeMacro(vtkSlicerTablesReader, vtkMRMLFileReader);

  enum
  {
    PasswordRequestedEvent = vtkCommand::UserEvent + 181
  };

  void SetTablesLogic(vtkSlicerTablesLogic* logic);
  vtkSlicerTablesLogic* GetTablesLogic();

  /// .txt file is more likely a simple text file than a table, therefore lower confidence is returned.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerTablesReader();
  ~vtkSlicerTablesReader() override;

  vtkWeakPointer<vtkSlicerTablesLogic> TablesLogic;

private:
  vtkSlicerTablesReader(const vtkSlicerTablesReader&) = delete;
  void operator=(const vtkSlicerTablesReader&) = delete;
};

#endif
