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

#ifndef __vtkSlicerMarkupsReader_h
#define __vtkSlicerMarkupsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerMarkupsModuleLogicExport.h"

class vtkSlicerMarkupsLogic;

/// Reader of markups files (.mrk.json, .json, .fcsv).
///
/// Properties: "fileName", "name".
class VTK_SLICER_MARKUPS_MODULE_LOGIC_EXPORT vtkSlicerMarkupsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerMarkupsReader* New();
  vtkTypeMacro(vtkSlicerMarkupsReader, vtkMRMLFileReader);

  void SetMarkupsLogic(vtkSlicerMarkupsLogic* logic);
  vtkSlicerMarkupsLogic* GetMarkupsLogic();

  /// For generic file extensions (.json) the file content is inspected.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerMarkupsReader();
  ~vtkSlicerMarkupsReader() override;

  vtkWeakPointer<vtkSlicerMarkupsLogic> MarkupsLogic;

private:
  vtkSlicerMarkupsReader(const vtkSlicerMarkupsReader&) = delete;
  void operator=(const vtkSlicerMarkupsReader&) = delete;
};

#endif
