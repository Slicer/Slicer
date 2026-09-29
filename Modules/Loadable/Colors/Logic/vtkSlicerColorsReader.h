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

#ifndef __vtkSlicerColorsReader_h
#define __vtkSlicerColorsReader_h

// Slicer includes
#include "vtkMRMLFileReader.h"

// VTK includes
#include <vtkWeakPointer.h>

#include "vtkSlicerColorsModuleLogicExport.h"

class vtkSlicerColorLogic;

/// Reader of color table files.
class VTK_SLICER_COLORS_MODULE_LOGIC_EXPORT vtkSlicerColorsReader : public vtkMRMLFileReader
{
public:
  static vtkSlicerColorsReader* New();
  vtkTypeMacro(vtkSlicerColorsReader, vtkMRMLFileReader);

  void SetColorLogic(vtkSlicerColorLogic* logic);
  vtkSlicerColorLogic* GetColorLogic();

  /// For generic file extensions (.txt) the file content is inspected.
  double CanLoadFileConfidence(const std::string& filePath) override;

  bool Load(vtkMRMLIOProperties* properties) override;

protected:
  vtkSlicerColorsReader();
  ~vtkSlicerColorsReader() override;

  vtkWeakPointer<vtkSlicerColorLogic> ColorLogic;

private:
  vtkSlicerColorsReader(const vtkSlicerColorsReader&) = delete;
  void operator=(const vtkSlicerColorsReader&) = delete;
};

#endif
